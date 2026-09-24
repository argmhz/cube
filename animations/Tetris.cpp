#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <random>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

// Tetris in three dimensions, played by the cube itself. The whole cube is
// the well: the seven classic pieces, in their classic colours, drop in
// lying flat, and a little AI turns and slides each one into place in the
// air before letting it fall. A faint shadow on the stack shows where it is
// headed. Fill a whole 8x8 layer and it flashes white and vanishes, with
// everything above coming down a level; stack up to the top and the game is
// over -- the stack fades away and a new game begins.
class Tetris : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  struct Cell {
    int x;
    int z;
  };

  using Shape = std::array<Cell, 4>;

  struct Placement {
    int rotation;
    int x;
    int z;
    int level;
    float score;
    bool valid;
  };

  enum Phase { ALIGN, DROP, CLEAR, GAME_OVER };

  static constexpr int PIECES = 7;
  // Grid value for an empty cell; otherwise it holds the piece's index.
  static constexpr int EMPTY = -1;

  // Seconds per step at pace 1.
  static constexpr float MOVE_STEP = 0.11f;
  static constexpr float DROP_STEP = 0.045f;
  static constexpr float CLEAR_TIME = 0.45f;
  static constexpr float GAME_OVER_TIME = 2.0f;

  static constexpr Shape SHAPES[PIECES] = {
    Shape{{{0, 0}, {1, 0}, {2, 0}, {3, 0}}},  // I
    Shape{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}},  // O
    Shape{{{0, 0}, {1, 0}, {2, 0}, {1, 1}}},  // T
    Shape{{{1, 0}, {2, 0}, {0, 1}, {1, 1}}},  // S
    Shape{{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},  // Z
    Shape{{{0, 0}, {0, 1}, {1, 1}, {2, 1}}},  // J
    Shape{{{2, 0}, {0, 1}, {1, 1}, {2, 1}}},  // L
  };

  static constexpr Rgb COLOURS[PIECES] = {
    {0.0f, 0.9f, 1.0f},   // I cyan
    {1.0f, 0.85f, 0.0f},  // O yellow
    {0.7f, 0.0f, 1.0f},   // T purple
    {0.0f, 1.0f, 0.0f},   // S green
    {1.0f, 0.0f, 0.0f},   // Z red
    {0.0f, 0.2f, 1.0f},   // J blue
    {1.0f, 0.45f, 0.0f},  // L orange
  };

  std::atomic<float> pace{1.0f};
  std::atomic<float> brightness{1.0f};
  std::atomic<bool> shadow{true};

  std::mt19937 random{std::random_device{}()};
  int grid[8][8][8];
  std::vector<int> bag;

  int piece = 0;
  int held = -1;
  int rotation = 0;
  int px = 0;
  int pz = 0;
  int py = 7;
  Placement goal = {};
  int cleared[8] = {};
  int clearedCount = 0;

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void onDataUpdate(json data) override {
    if (data["pace"].is_number()) {
      pace = std::clamp(data["pace"].get<float>(), 0.25f, 3.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
    if (data["shadow"].is_number()) {
      shadow = data["shadow"].get<int>() != 0;
    }
  }

  // A piece turned a quarter turn `turns` times, shifted back so its
  // corner is at the origin.
  static Shape turned(int kind, int turns) {
    Shape s = SHAPES[kind];
    for (int t = 0; t < turns; t++) {
      for (Cell &c : s) {
        c = {c.z, -c.x};
      }
    }
    int minX = 8, minZ = 8;
    for (const Cell &c : s) {
      minX = std::min(minX, c.x);
      minZ = std::min(minZ, c.z);
    }
    for (Cell &c : s) {
      c.x -= minX;
      c.z -= minZ;
    }
    return s;
  }

  // Moves a position just far enough that the whole piece is inside.
  static void keepInside(const Shape &s, int &x, int &z) {
    int width = 0, depth = 0;
    for (const Cell &c : s) {
      width = std::max(width, c.x + 1);
      depth = std::max(depth, c.z + 1);
    }
    x = std::clamp(x, 0, 8 - width);
    z = std::clamp(z, 0, 8 - depth);
  }

  static bool fits(const Shape &s, int x, int z) {
    for (const Cell &c : s) {
      if (x + c.x < 0 || x + c.x > 7 || z + c.z < 0 || z + c.z > 7) {
        return false;
      }
    }
    return true;
  }

  // How high the stack is in each column: one above its top block.
  void heights(int (&h)[8][8]) const {
    for (int x = 0; x < 8; x++) {
      for (int z = 0; z < 8; z++) {
        h[x][z] = 0;
        for (int y = 7; y >= 0; y--) {
          if (grid[x][y][z] != EMPTY) {
            h[x][z] = y + 1;
            break;
          }
        }
      }
    }
  }

  // The seven pieces come shuffled in sets, as in modern Tetris, so no
  // piece is ever left out for long.
  int nextPiece() {
    if (bag.empty()) {
      for (int i = 0; i < PIECES; i++) {
        bag.push_back(i);
      }
      std::shuffle(bag.begin(), bag.end(), random);
    }
    const int p = bag.back();
    bag.pop_back();
    return p;
  }

  // The AI: tries every way the piece can land and scores each one. A
  // piece lying flat comes to rest on the highest column under it, so the
  // columns it bridges over become holes -- those cost the most. After
  // that it wants to stay low, to fit snugly against walls and blocks, to
  // leave gaps that pieces can still fill exactly, and above all to
  // complete layers.
  Placement best(int kind) const {
    int h[8][8];
    heights(h);
    Placement top = {0, 0, 0, 0, -1e9f, false};

    const int turns = kind == 1 ? 1 : 4;
    for (int r = 0; r < turns; r++) {
      const Shape s = turned(kind, r);
      for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
          if (!fits(s, x, z)) {
            continue;
          }
          int level = 0;
          for (const Cell &c : s) {
            level = std::max(level, h[x + c.x][z + c.z]);
          }
          if (level > 7) {
            continue;
          }

          bool mine[8][8] = {};
          int holes = 0;
          for (const Cell &c : s) {
            holes += level - h[x + c.x][z + c.z];
            mine[x + c.x][z + c.z] = true;
          }
          const auto filled = [&](int cx, int cz) {
            if (cx < 0 || cx > 7 || cz < 0 || cz > 7) {
              return true;
            }
            return mine[cx][cz] || grid[cx][level][cz] != EMPTY;
          };

          int snug = 0;
          for (const Cell &c : s) {
            const int cx = x + c.x, cz = z + c.z;
            const Cell around[4] = {{cx - 1, cz}, {cx + 1, cz}, {cx, cz - 1}, {cx, cz + 1}};
            for (const Cell &n : around) {
              if ((n.x < 0 || n.x > 7 || n.z < 0 || n.z > 7 || !mine[n.x][n.z]) && filled(n.x, n.z)) {
                snug++;
              }
            }
          }

          // The gaps left in this layer: each area of connected empty
          // cells can only ever be filled by four-block pieces if its size
          // is a multiple of four, and areas smaller than a piece never can.
          // Cells under an overhang can't be reached, so they count as
          // filled here.
          bool seen[8][8] = {};
          int layerFull = 1;
          int badAreas = 0;
          int tinyAreas = 0;
          for (int cx = 0; cx < 8; cx++) {
            for (int cz = 0; cz < 8; cz++) {
              if (seen[cx][cz] || filled(cx, cz) || h[cx][cz] > level) {
                continue;
              }
              layerFull = 0;
              int size = 0;
              Cell stack[64];
              int pending = 0;
              stack[pending++] = {cx, cz};
              seen[cx][cz] = true;
              while (pending > 0) {
                const Cell c = stack[--pending];
                size++;
                const Cell around[4] = {{c.x - 1, c.z}, {c.x + 1, c.z}, {c.x, c.z - 1}, {c.x, c.z + 1}};
                for (const Cell &n : around) {
                  if (!filled(n.x, n.z) && h[n.x][n.z] <= level && !seen[n.x][n.z]) {
                    seen[n.x][n.z] = true;
                    stack[pending++] = n;
                  }
                }
              }
              if (size % 4 != 0) {
                badAreas++;
              }
              if (size < 4) {
                tinyAreas++;
              }
            }
          }
          if (layerFull) {
            // Every cell may be full because of overhangs rather than
            // blocks; only a layer with a block in every cell clears.
            for (int cx = 0; cx < 8 && layerFull; cx++) {
              for (int cz = 0; cz < 8 && layerFull; cz++) {
                layerFull = mine[cx][cz] || grid[cx][level][cz] != EMPTY;
              }
            }
          }

          const float score = -14.0f * holes - 6.0f * level + 0.8f * snug - 8.0f * badAreas - 12.0f * tinyAreas + 60.0f * layerFull;
          if (score > top.score) {
            top = {r, x, z, level, score, true};
          }
        }
      }
    }
    return top;
  }

  void newGame() {
    for (auto &plane : grid) {
      for (auto &row : plane) {
        for (int &cell : row) {
          cell = EMPTY;
        }
      }
    }
    bag.clear();
    held = -1;
  }

  // Picks the next piece -- or swaps in the held one when that lands
  // better -- and works out where it should go. False means nothing fits.
  bool spawn() {
    const int incoming = nextPiece();
    Placement here = best(incoming);
    if (held < 0) {
      held = nextPiece();
    }
    const Placement other = best(held);
    piece = incoming;
    if (other.valid && (!here.valid || other.score > here.score)) {
      piece = held;
      held = incoming;
      here = other;
    }
    if (!here.valid) {
      return false;
    }
    goal = here;
    rotation = 0;
    px = 3;
    pz = 3;
    keepInside(turned(piece, 0), px, pz);
    py = 7;
    return true;
  }

  void setVoxel(Cube *cube, int x, int y, int z, Rgb c, float amount) {
    cube->set(x, y, z, toLevel(c.r * amount), toLevel(c.g * amount), toLevel(c.b * amount));
  }

  void draw(Cube *cube) override {
    newGame();
    Phase phase = spawn() ? ALIGN : GAME_OVER;
    float clock = 0.0f;
    float inPhase = 0.0f;

    while (isRunning()) {
      const float dt = 0.033f * pace;
      clock += dt;
      inPhase += dt;

      switch (phase) {
        case ALIGN: {
          // One move at a time, as if someone were pressing the buttons:
          // turn first, then slide across, then along.
          if (clock < MOVE_STEP) {
            break;
          }
          clock = 0.0f;
          if (rotation != goal.rotation) {
            rotation = (rotation + 1) % 4;
            keepInside(turned(piece, rotation), px, pz);
          } else if (px != goal.x) {
            px += px < goal.x ? 1 : -1;
          } else if (pz != goal.z) {
            pz += pz < goal.z ? 1 : -1;
          } else {
            phase = DROP;
          }
          break;
        }

        case DROP:
          if (clock < DROP_STEP) {
            break;
          }
          clock = 0.0f;
          if (py > goal.level) {
            py--;
          } else {
            for (const Cell &c : turned(piece, rotation)) {
              grid[px + c.x][py][pz + c.z] = piece;
            }
            clearedCount = 0;
            for (int y = 0; y < 8; y++) {
              bool full = true;
              for (int x = 0; x < 8 && full; x++) {
                for (int z = 0; z < 8 && full; z++) {
                  full = grid[x][y][z] != EMPTY;
                }
              }
              if (full) {
                cleared[clearedCount++] = y;
              }
            }
            if (clearedCount > 0) {
              phase = CLEAR;
              inPhase = 0.0f;
            } else {
              phase = spawn() ? ALIGN : GAME_OVER;
              inPhase = 0.0f;
            }
          }
          break;

        case CLEAR:
          if (inPhase >= CLEAR_TIME) {
            // Remove the full layers from the top down, so the indices of
            // the ones still to go stay right.
            for (int i = clearedCount - 1; i >= 0; i--) {
              for (int y = cleared[i]; y < 7; y++) {
                for (int x = 0; x < 8; x++) {
                  for (int z = 0; z < 8; z++) {
                    grid[x][y][z] = grid[x][y + 1][z];
                  }
                }
              }
              for (int x = 0; x < 8; x++) {
                for (int z = 0; z < 8; z++) {
                  grid[x][7][z] = EMPTY;
                }
              }
            }
            clearedCount = 0;
            phase = spawn() ? ALIGN : GAME_OVER;
            inPhase = 0.0f;
          }
          break;

        case GAME_OVER:
          if (inPhase >= GAME_OVER_TIME) {
            newGame();
            phase = spawn() ? ALIGN : GAME_OVER;
            inPhase = 0.0f;
          }
          break;
      }

      // --- draw ---
      cube->clear();

      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            const int kind = grid[x][y][z];
            if (kind == EMPTY) {
              continue;
            }
            bool flashing = false;
            for (int i = 0; i < clearedCount; i++) {
              flashing = flashing || cleared[i] == y;
            }
            if (phase == CLEAR && flashing) {
              // Full layers blink white before they go.
              const bool on = static_cast<int>(inPhase / 0.075f) % 2 == 0;
              setVoxel(cube, x, y, z, on ? Rgb{1.0f, 1.0f, 1.0f} : COLOURS[kind], on ? 0.9f : 0.5f);
            } else {
              float fade = 0.5f;
              if (phase == GAME_OVER) {
                // Game over: the stack fades away a layer at a time, from
                // the top down.
                const float gone = (7 - y) / 8.0f * GAME_OVER_TIME * 0.7f + 0.3f;
                fade = 0.5f * std::clamp((gone - inPhase) / 0.3f, 0.0f, 1.0f);
              }
              if (fade > 0.01f) {
                setVoxel(cube, x, y, z, COLOURS[kind], fade);
              }
            }
          }
        }
      }

      if (phase == ALIGN || phase == DROP) {
        const Shape s = turned(piece, rotation);
        if (shadow && phase == ALIGN) {
          for (const Cell &c : turned(piece, goal.rotation)) {
            const int sy = goal.level;
            if (sy < py) {
              setVoxel(cube, goal.x + c.x, sy, goal.z + c.z, COLOURS[piece], 0.12f);
            }
          }
        }
        for (const Cell &c : s) {
          setVoxel(cube, px + c.x, py, pz + c.z, COLOURS[piece], 1.0f);
        }
      }

      cube->update();
      usleep(30000);
    }
  }
};
extern "C" Animation *create() {
  return new Tetris;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
