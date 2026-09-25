#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

using json = nlohmann::json;

// Drops fall down the Y axis one row per frame (via cube->shift), fading
// to a dim trail as they pass, and flash into a bright puddle mark for one
// frame as they hit the floor.
class Rain : public Animation {
  // At most one drop per (x, z) column makes visual sense on an 8x8 floor
  // -- also doubles as the fixed bound for the stack array below, so a
  // client can't send a value that blows the stack.
  static const int MAX_DROPS = 64;

  int speed = 65000;
  int max_drops = 4;
  // Modulo divisor controlling how often a new drop spawns per column per
  // frame -- higher means rarer (see the roll against `target` below).
  int tens = 3;

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }

    if (data["max_drops"].is_number()) {
      int value = data["max_drops"].get<int>();
      if (value > 0 && value <= MAX_DROPS) {
        max_drops = value;
      }
    }

    if (data["tens"].is_number()) {
      int value = data["tens"].get<int>();
      // tens is a modulo divisor below -- 0 would be a division by zero.
      if (value > 0) {
        tens = value;
      }
    }
  }

  void draw(Cube *c) override {
    const int target = 1;

    while (isRunning()) {
      int prev[MAX_DROPS][3] = {};

      for (int i = 0; i < max_drops; i++) {
        if (prev[i][0] || prev[i][1] || prev[i][2]) {
          c->set(prev[i][0], prev[i][1], prev[i][2], 0, 0, 3);
        }

        prev[i][0] = 0;
        prev[i][1] = 0;
        prev[i][2] = 0;
      }

      for (int i = 0; i < max_drops; i++) {
        if ((rand() % tens) != target) {
          continue;
        }

        int x = rand() % 8;
        int z = rand() % 8;
        int y = 7;

        prev[i][0] = x;
        prev[i][1] = y;
        prev[i][2] = z;

        c->set(x, y, z, 0, 0, 15);
      }

      c->update();
      c->shift(AXIS_Y, -1);

      // Anything that just shifted into the floor row flashes bright for
      // one frame before the next shift carries it out of the cube.
      for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
          if (!c->isOff(x, 0, z)) {
            c->set(x, 0, z, 15, 0, 15);
          }
        }
      }

      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Rain;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
