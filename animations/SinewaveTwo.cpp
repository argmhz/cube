#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>

using json = nlohmann::json;

// Nested square rings, one per shell of the cube. Each ring's position
// along x bounces between the two walls a step at a time, and since the
// rings start staggered they ripple through each other rather than moving
// as one. The colour changes every time the outermost ring reaches the far
// wall.
class SinewaveTwo : public Animation {
  // The original slept 8000us eight times per frame -- once per iteration
  // of the loop that copied the ring positions -- so this keeps the same
  // ~15fps pace now that the frame sleeps once.
  int speed = 64000;
  int shells = 4;

  int wave[8] = {};
  int direction[8] = {};
  int previous[8] = {};

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["shells"].is_number()) {
      shells = std::clamp(data["shells"].get<int>(), 1, 4);
    }
  }

  // One ring, `depth` layers in from the cube's surface: four runs of
  // voxels that meet at the corners to close the square. Clears where the
  // ring was last frame before drawing where it is now.
  void drawShell(Cube *c, int depth, int red, int green, int blue) {
    for (int addr = depth; addr <= 7 - depth; addr++) {
      const int was = previous[addr];
      c->set(was, addr, depth, 0, 0, 0);
      c->set(was, depth, addr, 0, 0, 0);
      c->set(was, 7 - addr, 7 - depth, 0, 0, 0);
      c->set(was, 7 - depth, 7 - addr, 0, 0, 0);

      const int now = wave[addr];
      c->set(now, addr, depth, red, green, blue);
      c->set(now, depth, addr, red, green, blue);
      c->set(now, 7 - addr, 7 - depth, red, green, blue);
      c->set(now, 7 - depth, 7 - addr, red, green, blue);
    }
  }

  void draw(Cube *c) override {
    int red = 0;
    int green = 0;
    int blue = 15;

    for (int i = 0; i < 8; i++) {
      wave[i] = i;
      direction[i] = 1;
      // Seeded to match wave[] rather than left uninitialised: the first
      // frame clears "where the ring was" before anything has been drawn
      // there, and used to read whatever happened to be on the stack.
      previous[i] = i;
    }

    while (isRunning()) {
      for (int i = 0; i < 8; i++) {
        if (wave[i] == 7) {
          direction[i] = -1;
        }
        if (wave[i] == 0) {
          direction[i] = 1;
        }
        wave[i] += direction[i];
      }

      // Two random channels lit and the third off, so the new colour is
      // always a vivid mix rather than an occasional muddy grey.
      if (wave[0] == 7) {
        switch (random(3)) {
          case 0:
            red = random(1, 16); green = random(1, 16); blue = 0;
            break;
          case 1:
            red = random(1, 16); green = 0; blue = random(1, 16);
            break;
          default:
            red = 0; green = random(1, 16); blue = random(1, 16);
            break;
        }
      }

      for (int depth = 0; depth < shells; depth++) {
        drawShell(c, depth, red, green, blue);
      }

      for (int i = 0; i < 8; i++) {
        previous[i] = wave[i];
      }

      c->update();
      usleep(speed);
    }
  }
};

extern "C" Animation *create() {
  return new SinewaveTwo;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
