#include <algorithm>
#include <array>
#include <cmath>

#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

// A small product-reveal loop: the Monster Energy claw logo sits up front
// as a flat sign, slides back into the cube, and then a plain white can
// (the "White Monster") rises up out of the floor in the space the sign
// left behind -- rather than the logo spinning in place.
class Moner : public Animation {
  enum Phase { RECEDE, RISE, HOLD, FALL, RETURN };

  static constexpr float THICKNESS = 1.6f;
  static constexpr float FRONT_Z = 1.0f;
  static constexpr float BACK_Z = 6.5f;
  static constexpr float SLIDE_RATE = 0.045f;
  static constexpr float CAN_RADIUS = 1.6f;
  static constexpr float CAN_MAX_HEIGHT = 7.3f;
  static constexpr float CAN_RATE = 0.07f;
  static constexpr int HOLD_FRAMES = 90;
  static constexpr int SPEED = 45000;

  // [y][x], y=0 is the bottom row, x=0 is the left column.
  static constexpr std::array<std::array<int, 8>, 8> LOGO = {{
    {1, 1, 0, 0, 0, 0, 1, 1},
    {1, 1, 0, 0, 0, 0, 1, 1},
    {1, 1, 0, 0, 0, 0, 1, 1},
    {1, 1, 0, 0, 0, 0, 1, 1},
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 1, 1, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 1, 1, 1},
  }};

  void draw(Cube *cube) override {
    Phase phase = RECEDE;
    float logoZ = FRONT_Z;
    float canHeight = 0.0f;
    int phaseFrame = 0;

    // What's currently lit, so each frame only writes voxels that actually
    // changed instead of clearing and redrawing the cube every time -- see
    // Beer.cpp for why (bin/simulator's recorder samples this buffer from
    // another thread with no locking; a big redraw is a torn frame waiting
    // to happen).
    Cube::Color previous[8][8][8] = {};

    while (isRunning()) {
      switch (phase) {
        case RECEDE:
          logoZ += SLIDE_RATE;
          if (logoZ >= BACK_Z) {
            logoZ = BACK_Z;
            phase = RISE;
          }
          break;
        case RISE:
          canHeight += CAN_RATE;
          if (canHeight >= CAN_MAX_HEIGHT) {
            canHeight = CAN_MAX_HEIGHT;
            phase = HOLD;
            phaseFrame = 0;
          }
          break;
        case HOLD:
          phaseFrame++;
          if (phaseFrame > HOLD_FRAMES) {
            phase = FALL;
          }
          break;
        case FALL:
          canHeight -= CAN_RATE;
          if (canHeight <= 0.0f) {
            canHeight = 0.0f;
            phase = RETURN;
          }
          break;
        case RETURN:
          logoZ -= SLIDE_RATE;
          if (logoZ <= FRONT_Z) {
            logoZ = FRONT_Z;
            phase = RECEDE;
          }
          break;
      }

      for (int z = 0; z < 8; z++) {
        for (int x = 0; x < 8; x++) {
          const float dx = x - 3.5f;
          const float dz = z - FRONT_Z;

          for (int y = 0; y < 8; y++) {
            int r = 0, g = 0, b = 0;

            // The can: a circle in x/z centred at the front, filled from
            // the bottom up to canHeight -- so it reads as rising up out
            // of the floor rather than fading in all at once. Its top two
            // rows are drawn narrower and metallic, like a can's lid, so
            // that detail only comes into view right at the end of the
            // rise, the same way the real lid is the last thing you'd see.
            const bool isRim = y >= 6;
            const float radius = isRim ? CAN_RADIUS - 0.4f : CAN_RADIUS;
            if (y < canHeight && dx * dx + dz * dz <= radius * radius) {
              if (isRim) {
                r = 9; g = 9; b = 10;
              } else if (y == 5) {
                r = 0; g = 15; b = 3;
              } else {
                r = 15; g = 15; b = 15;
              }
            } else if (std::fabs(z - logoZ) <= THICKNESS * 0.5f && LOGO[y][x]) {
              // The logo sign: a thin slab that slides along z.
              r = 0; g = 15; b = 2;
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
      usleep(SPEED);
    }
  }
};
extern "C" Animation *create() {
  return new Moner;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
