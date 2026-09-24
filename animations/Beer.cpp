#include <algorithm>
#include <cmath>

#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

using json = nlohmann::json;

// A pint glass that pours full, sits for a drink, then empties again --
// looping forever. The glass itself (walls + a narrower base, like a real
// pint glass's heel) is redrawn every frame from a height-dependent shape
// function rather than stored, so the only state that has to survive
// between frames is the liquid surface height and the high-water mark used
// for the foam "lacing" left clinging to the glass after a drink.
class Beer : public Animation {
  enum Phase { POUR, HOLD_FULL, DRINK, HOLD_EMPTY };

  static constexpr float MAX_LEVEL = 7.3f;
  static constexpr float POUR_RATE = 0.035f;
  static constexpr float DRINK_RATE = 0.05f;
  static constexpr int HOLD_FULL_FRAMES = 90;
  static constexpr int HOLD_EMPTY_FRAMES = 40;

  // Mutated live from cube-client: speed is the same per-frame usleep every
  // other animation uses (lower = faster), and chugRequested is a one-shot
  // flag set by the "Chug!" button and consumed by draw() on its next loop.
  // Plain, unlocked shared state -- same tradeoff ColorWheel.cpp and
  // Wipeout.cpp already make for their own onDataUpdate() fields.
  int speed = 40000;
  bool chugRequested = false;

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      speed = std::clamp(data["speed"].get<int>(), 5000, 150000);
    }
    if (data["chug"].is_number()) {
      chugRequested = true;
    }
  }

  // How many extra cells on each side are "outside the glass" at a given
  // height -- narrower near the bottom (a heel), full width for the rest of
  // the body. Returns 0/1 so the interior is 4x4 at the base, 6x6 above it.
  static int taper(int y) {
    return y < 2 ? 1 : 0;
  }

  static bool isWall(int x, int z, int y) {
    const int t = taper(y);
    return x < 1 + t || x > 6 - t || z < 1 + t || z > 6 - t;
  }

  // Cheap spatial hash used wherever a pattern needs to look randomly
  // speckled but stay stable frame-to-frame (or drift slowly with `t`),
  // rather than flickering with a fresh rand() every frame.
  static int speckle(int x, int y, int z, int t, int modulus) {
    return (x * 7 + z * 11 + y * 13 + t) % modulus;
  }

  void draw(Cube *cube) override {
    Phase phase = HOLD_EMPTY;
    float level = 0.0f;
    float lacingHigh = 0.0f;
    int phaseFrame = 0;
    int t = 0;

    // What's currently lit, so each frame only writes the voxels that
    // actually changed instead of clearing and redrawing all 512 every
    // time. Most of the glass (walls, the still beer body) is identical
    // frame to frame -- only the surface band, the odd bubble and the
    // foam texture actually move. Keeping each frame's write burst small
    // also matters for `bin/simulator`'s recording: it samples this same
    // buffer from another thread with no locking, so a long redraw is
    // exactly what shows up as a torn, flickering frame in playback.
    Cube::Color previous[8][8][8] = {};
    bool chugging = false;

    while (isRunning()) {
      // A press jumps straight into (or stays in) DRINK and drains at
      // several times the normal rate -- works mid-pour or mid-hold too,
      // since nobody waits for a full glass to start chugging. Ignored
      // with nothing in the glass; consumed either way so a stray press
      // while empty doesn't carry over into the next pour.
      if (chugRequested) {
        chugRequested = false;
        if (level > 0.0f) {
          phase = DRINK;
          lacingHigh = std::max(lacingHigh, level);
          chugging = true;
        }
      }

      switch (phase) {
        case POUR:
          level += POUR_RATE;
          if (level >= MAX_LEVEL) {
            level = MAX_LEVEL;
            lacingHigh = level;
            phase = HOLD_FULL;
            phaseFrame = 0;
          }
          break;
        case HOLD_FULL:
          phaseFrame++;
          if (phaseFrame > HOLD_FULL_FRAMES) {
            phase = DRINK;
          }
          break;
        case DRINK:
          level -= chugging ? DRINK_RATE * 4.0f : DRINK_RATE;
          if (level <= 0.0f) {
            level = 0.0f;
            phase = HOLD_EMPTY;
            phaseFrame = 0;
            chugging = false;
          }
          break;
        case HOLD_EMPTY:
          phaseFrame++;
          if (phaseFrame > HOLD_EMPTY_FRAMES) {
            phase = POUR;
            lacingHigh = 0.0f;
          }
          break;
      }

      // Fresh, foamy head while pouring; it settles down to a thin cap
      // while the glass sits, then rides the surface down while drinking.
      float foamThickness = 1.3f;
      if (phase == HOLD_FULL) {
        foamThickness = std::max(0.4f, 1.3f - phaseFrame * 0.012f);
      } else if (phase == DRINK) {
        foamThickness = chugging ? 0.25f : 0.5f;
      } else if (phase == HOLD_EMPTY) {
        foamThickness = 0.0f;
      }

      for (int z = 0; z < 8; z++) {
        for (int x = 0; x < 8; x++) {
          for (int y = 0; y < 8; y++) {
            int r = 0, g = 0, b = 0;

            if (isWall(x, z, y)) {
              // A broken ring of dried foam clinging above the current
              // surface, up to the highest point the beer reached this
              // round -- wiped clean once the level rises back past it.
              if (phase != POUR && y > level && y <= lacingHigh && speckle(x, y, z, 0, 3) == 0) {
                r = 6; g = 6; b = 4;
              } else {
                r = 3; g = 4; b = 6;
              }
            } else if (y <= level) {
              if (y == 0 && level <= 0.0f) {
                r = 3; g = 4; b = 6;
              } else if (y > level - foamThickness) {
                // Speckle the foam so it reads as bubbly rather than a flat
                // white block; the pattern drifts slowly over time.
                const bool fleck = speckle(x, y, z, t / 3, 10) < 2;
                const int shade = fleck ? 11 : 15;
                r = shade; g = shade; b = shade - 3;
              } else {
                // Rising carbonation: the odd voxel flashes a lighter gold.
                if (rand() % 140 == 0) {
                  r = 15; g = 13; b = 4;
                } else {
                  r = 15; g = 8; b = 0;
                }
              }
            } else if (phase == POUR && y < level + 1.4f && speckle(x, y, z, t / 2, 30) == 0) {
              // Loose splash droplets thrown up just above the surface
              // while actively pouring.
              r = 15; g = 15; b = 12;
            }

            Cube::Color &prev = previous[x][y][z];
            if (prev.red != r || prev.green != g || prev.blue != b) {
              cube->set(x, y, z, r, g, b);
              prev.red = r;
              prev.green = g;
              prev.blue = b;
            }
          }
        }
      }

      cube->update();
      t++;
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Beer;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
