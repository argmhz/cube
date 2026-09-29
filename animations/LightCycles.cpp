#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <cstdlib>

using json = nlohmann::json;

// Up to four players, each steering a cycle that leaves a permanent trail
// behind it -- crash into any trail (including your own) or a wall and
// you're out. Last cycle standing wins the round, then everything resets.
//
// There is no "join"/"leave" message anywhere in this protocol (see
// AnimationCommandHandler.h), so this deliberately doesn't invent one: all
// four slots are always alive from the start of a round. A slot nobody is
// pressing buttons for just keeps going straight in its spawn direction
// until it drives itself into a wall or its own trail -- which is exactly
// how "1 to 4 players" falls out for free, with no presence-tracking of
// any kind. cube-client's player-select card (see www/index.html) is what
// tags a browser's button presses with which slot they steer.
class LightCycles : public Animation {
  static constexpr int MAX_PLAYERS = 4;

  struct Rgb {
    int r, g, b;
  };

  struct Player {
    bool alive;
    int x, y, z;
    int dir;         // index 0..5 into AXIS_D*, applied at the top of each tick
    int pendingDir;  // last accepted button press, may differ from dir mid-tick
  };

  // 1=+X, 2=-X, 3=+Y, 4=-Y, 5=+Z, 6=-Z over the wire (matches Dot.cpp's
  // "com" convention) -- stored here 0-indexed.
  static constexpr int AXIS_DX[6] = {1, -1, 0, 0, 0, 0};
  static constexpr int AXIS_DY[6] = {0, 0, 1, -1, 0, 0};
  static constexpr int AXIS_DZ[6] = {0, 0, 0, 0, 1, -1};

  static constexpr Rgb PLAYER_COLOR[MAX_PLAYERS] = {
      {0, 15, 15},  // cyan
      {15, 0, 15},  // magenta
      {15, 8, 0},   // amber
      {0, 15, 0},   // green
  };

 public:
  // True if turning from `current` straight to `candidate` would send the
  // cycle back into the trail cell it just left. Public and static (no
  // Cube/Animation state needed) purely so tests/lightcycles_tests.cpp can
  // exercise it directly -- see AXIS_D* above for the 0..5 encoding.
  static bool isReversal(int current, int candidate) {
    return AXIS_DX[candidate] == -AXIS_DX[current] && AXIS_DY[candidate] == -AXIS_DY[current] &&
           AXIS_DZ[candidate] == -AXIS_DZ[current];
  }

 private:
  Player players[MAX_PLAYERS];
  int trailOwner[8][8][8] = {};  // 0 = empty, 1..4 = owning player

  int speed = 90000;

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["player"].is_number() && data["dir"].is_number()) {
      int p = data["player"].get<int>() - 1;
      int d = data["dir"].get<int>() - 1;
      if (p >= 0 && p < MAX_PLAYERS && d >= 0 && d < 6 && players[p].alive) {
        // Reject a direct reversal -- it would drive the cycle straight
        // into the trail cell it just left, which reads as an unfair,
        // accidental death rather than a real mistake.
        if (!isReversal(players[p].dir, d)) {
          players[p].pendingDir = d;
        }
      }
    }
  }

  void resetRound() {
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        for (int z = 0; z < 8; z++) {
          trailOwner[x][y][z] = 0;
        }
      }
    }

    // Four spawns, each on its own (y, z) line running along X. Distinct
    // (y, z) pairs mean no two unpiloted cycles' straight-line paths ever
    // cross -- they used to (two shared y=1,z=1 heading straight at each
    // other, the other two shared y=6,z=6 the same way), which crashed
    // most rounds within 2-3 ticks, before a single button press could
    // possibly land. A player can still steer into someone else's path on
    // purpose; the default paths themselves just no longer collide.
    static const int spawnX[MAX_PLAYERS] = {1, 6, 1, 6};
    static const int spawnY[MAX_PLAYERS] = {1, 2, 6, 5};
    static const int spawnZ[MAX_PLAYERS] = {1, 2, 6, 5};
    static const int spawnDir[MAX_PLAYERS] = {0, 1, 0, 1};  // +X, -X, +X, -X

    for (int p = 0; p < MAX_PLAYERS; p++) {
      players[p].alive = true;
      players[p].x = spawnX[p];
      players[p].y = spawnY[p];
      players[p].z = spawnZ[p];
      players[p].dir = spawnDir[p];
      players[p].pendingDir = spawnDir[p];
      trailOwner[players[p].x][players[p].y][players[p].z] = p + 1;
    }
  }

  // A few seconds of flashing before the next round -- winner's colour, or
  // plain white if the last two players crashed into each other on the
  // same tick and nobody is left standing.
  void announceRoundEnd(Cube *cube, int aliveCount, int winner) {
    const Rgb flash = aliveCount == 1 ? PLAYER_COLOR[winner] : Rgb{15, 15, 15};
    for (int i = 0; i < 4; i++) {
      cube->clear();
      cube->update();
      usleep(150000);
      cube->all(flash.r, flash.g, flash.b);
      cube->update();
      usleep(150000);
    }
    cube->clear();
    cube->update();
    usleep(400000);
  }

  void draw(Cube *cube) override {
    resetRound();

    while (isRunning()) {
      for (int p = 0; p < MAX_PLAYERS; p++) {
        if (!players[p].alive) {
          continue;
        }

        players[p].dir = players[p].pendingDir;
        const int nx = players[p].x + AXIS_DX[players[p].dir];
        const int ny = players[p].y + AXIS_DY[players[p].dir];
        const int nz = players[p].z + AXIS_DZ[players[p].dir];

        const bool outOfBounds = nx < 0 || nx > 7 || ny < 0 || ny > 7 || nz < 0 || nz > 7;
        if (outOfBounds || trailOwner[nx][ny][nz] != 0) {
          players[p].alive = false;
          continue;
        }

        players[p].x = nx;
        players[p].y = ny;
        players[p].z = nz;
        trailOwner[nx][ny][nz] = p + 1;
      }

      cube->clear();
      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            const int owner = trailOwner[x][y][z];
            if (owner == 0) {
              continue;
            }
            const Rgb &c = PLAYER_COLOR[owner - 1];
            const bool isHead = players[owner - 1].alive && players[owner - 1].x == x &&
                                 players[owner - 1].y == y && players[owner - 1].z == z;
            if (isHead) {
              cube->set(x, y, z, c.r, c.g, c.b);
            } else {
              // Dimmed trail so the live head still stands out.
              cube->set(x, y, z, c.r / 3, c.g / 3, c.b / 3);
            }
          }
        }
      }
      cube->update();

      int aliveCount = 0;
      int lastAlive = -1;
      for (int p = 0; p < MAX_PLAYERS; p++) {
        if (players[p].alive) {
          aliveCount++;
          lastAlive = p;
        }
      }

      if (aliveCount <= 1) {
        announceRoundEnd(cube, aliveCount, lastAlive);
        resetRound();
      }

      usleep(speed);
    }
  }
};

extern "C" Animation *create() {
  return new LightCycles;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
