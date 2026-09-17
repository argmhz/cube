#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"

#include <algorithm>
#include <cmath>
#include <unistd.h>

class Aurora : public Animation {
  struct Star {
    int x;
    int y;
    int z;
    float phase;
  };

  static int brightness(float value) {
    return std::clamp(static_cast<int>(std::round(value)), 0, MAX_COLOR);
  }

  void drawCurtain(Cube *cube, float time, float offset, bool violet) {
    for (int y = 0; y < 8; ++y) {
      for (int z = 0; z < 8; ++z) {
        const float wave =
            std::sin(time + y * 0.72f + z * 0.20f + offset) * 1.35f +
            std::sin(time * 0.47f - z * 0.83f + offset) * 0.65f;
        const int center = static_cast<int>(std::round(3.5f + wave));

        // A bright ribbon with a dim voxel on either side gives the curtain
        // visible volume instead of making it look like a flat surface.
        for (int glow = -1; glow <= 1; ++glow) {
          const int x = center + glow;
          if (!cube->inBounce(x, y, z)) {
            continue;
          }

          const float edgeFade = 0.38f + 0.62f * std::sin((z + 1) * M_PI / 9.0f);
          const float shimmer = 0.78f + 0.22f * std::sin(time * 1.8f + y + z * 0.6f);
          const float glowFade = glow == 0 ? 1.0f : 0.22f;
          const float light = edgeFade * shimmer * glowFade;

          int red = violet ? brightness(9.0f * light) : brightness(1.5f * light);
          int green = violet ? brightness(3.0f * light) : brightness(15.0f * light);
          int blue = violet ? brightness(15.0f * light) : brightness(10.0f * light);

          Cube::Color current = cube->get(x, y, z);
          cube->set(x, y, z,
                    std::max(current.red, red),
                    std::max(current.green, green),
                    std::max(current.blue, blue));
        }
      }
    }
  }

public:
  void draw(Cube *cube) override {
    const Star stars[] = {
        {0, 0, 7, 0.2f}, {7, 1, 6, 1.7f}, {1, 6, 5, 3.1f},
        {6, 7, 7, 4.4f}, {0, 4, 3, 5.2f}, {7, 5, 2, 2.5f}};

    float time = 0.0f;
    while (isRunning()) {
      cube->clear();

      drawCurtain(cube, time, 0.0f, false);
      drawCurtain(cube, -time * 0.82f, M_PI, true);

      for (const Star &star : stars) {
        const float twinkle = (std::sin(time * 2.7f + star.phase) + 1.0f) * 0.5f;
        const int white = brightness(2.0f + twinkle * twinkle * 13.0f);
        cube->set(star.x, star.y, star.z, white, white, white);
      }

      cube->update();
      usleep(40000);
      time += 0.075f;

      if (time > 2.0f * M_PI) {
        time -= 2.0f * M_PI;
      }
    }
  }
};

extern "C" Animation *create() {
  return new Aurora;
}

extern "C" void destroy(Animation *animation) {
  delete animation;
}
