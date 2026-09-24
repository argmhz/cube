#include <algorithm>
#include <array>
#include <cmath>

#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

// A stylized, low-res take on the Monster Energy claw logo: three tapered
// claw marks, tallest at the outer edges with a dip in the middle so the
// negative space still reads as an "M". Drawn as an 8x8 bitmap on a thin
// billboard that spins around the cube's vertical axis -- the same
// per-voxel "sampled sign" technique Ticker.cpp uses for its BILLBOARD
// mode, just without the scroll/text layout machinery.
class Moner : public Animation {
  static constexpr float THICKNESS = 1.6f;
  static constexpr float SPIN = 0.35f;
  static constexpr int SPEED = 60000;

  // [y][x], y=0 is the bottom row, x=0 is the left column.
  static constexpr std::array<std::array<int, 8>, 8> LOGO = {{
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 0, 0, 1, 1, 0, 1, 0},
    {1, 0, 0, 1, 1, 0, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 1, 0},
  }};

  void draw(Cube *cube) override {
    float angle = 0.0f;

    while (isRunning()) {
      const float cosAngle = std::cos(angle);
      const float sinAngle = std::sin(angle);

      cube->clear();

      for (int z = 0; z < 8; z++) {
        for (int x = 0; x < 8; x++) {
          const float px = x - 3.5f;
          const float pz = z - 3.5f;

          // Split the position into a distance along the sign and one out
          // through its face, so spinning the sign is just a change of
          // angle rather than a different drawing routine.
          const float along = px * cosAngle + pz * sinAngle;
          const float distance = std::fabs(pz * cosAngle - px * sinAngle);
          if (distance > THICKNESS * 0.5f) continue;

          const float offset = along + 3.5f;
          if (offset < -0.5f || offset > 7.5f) continue;
          const int column = std::clamp(static_cast<int>(std::lround(offset)), 0, 7);

          for (int y = 0; y < 8; y++) {
            if (!LOGO[y][column]) continue;
            cube->set(x, y, z, 0, 15, 2);
          }
        }
      }

      cube->update();
      angle = std::fmod(angle + SPIN * 0.05f, 2.0f * static_cast<float>(M_PI));
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
