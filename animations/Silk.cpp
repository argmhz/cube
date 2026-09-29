#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <unistd.h>

// Slowly drifting closed curves with dim after-images. The low intensity
// halos and trails deliberately use the full 4-bit BAM range to make the
// motion feel soft on the physical cube.
class Silk : public Animation {
  struct Point {
    int x;
    int y;
    int z;
  };

  int speed = 50000;

  static int clampColor(float value) {
    return std::clamp(static_cast<int>(std::round(value)), 0, MAX_COLOR);
  }

  static Point curve(float u, float time) {
    const float breath = 2.55f + 0.22f * std::sin(time * 0.37f);
    const float fold = u * 2.0f + time * 0.43f;

    // A gently breathing loop whose vertical fold drifts through the ring.
    // Both frequencies are integral, so the curve closes without a seam.
    return {
        static_cast<int>(std::round(3.5f + breath * std::cos(u))),
        static_cast<int>(std::round(3.5f + breath * std::sin(u))),
        static_cast<int>(std::round(3.5f + 2.35f * std::sin(fold)))
    };
  }

  static void maxSet(Cube *cube, Point p, int red, int green, int blue) {
    if (!cube->inBounce(p.x, p.y, p.z)) {
      return;
    }

    Cube::Color old = cube->get(p.x, p.y, p.z);
    cube->set(p.x, p.y, p.z,
              std::max(old.red, red),
              std::max(old.green, green),
              std::max(old.blue, blue));
  }

  static void halo(Cube *cube, Point p, int red, int green, int blue) {
    const std::array<Point, 6> neighbours = {{{p.x - 1, p.y, p.z},
                                               {p.x + 1, p.y, p.z},
                                               {p.x, p.y - 1, p.z},
                                               {p.x, p.y + 1, p.z},
                                               {p.x, p.y, p.z - 1},
                                               {p.x, p.y, p.z + 1}}};

    for (Point neighbour : neighbours) {
      maxSet(cube, neighbour, red, green, blue);
    }
  }

  void drawRibbon(Cube *cube, float time, int light, float hueShift, bool glow) {
    constexpr int samples = 48;
    Point first = curve(0.0f, time);
    Point previous = first;

    for (int sample = 1; sample <= samples; ++sample) {
      const float u = sample * 2.0f * static_cast<float>(M_PI) / samples;
      Point current = curve(u, time);

      const float colorWave = 0.5f + 0.5f * std::sin(u + hueShift);
      const int red = clampColor(light * (0.10f + colorWave * 0.32f));
      const int green = clampColor(light * (0.55f + colorWave * 0.45f));
      const int blue = clampColor(light * (1.00f - colorWave * 0.18f));

      cube->line(previous.x, previous.y, previous.z,
                 current.x, current.y, current.z, red, green, blue);

      if (glow && sample % 2 == 0) {
        halo(cube, current,
             std::max(1, red / 5),
             std::max(1, green / 5),
             std::max(1, blue / 5));
      }

      previous = current;
    }

    // Explicitly close the loop after integer rounding.
    cube->line(previous.x, previous.y, previous.z,
               first.x, first.y, first.z, 1, light / 2, light);
  }

public:
  void onDataUpdate(json data) override {
    if (data["speed"].is_number_integer()) {
      speed = std::clamp(data["speed"].get<int>(), 20000, 250000);
    }
  }

  void draw(Cube *cube) override {
    float time = 0.0f;

    while (isRunning()) {
      cube->clear();

      // Oldest trail first. Increasing BAM values make four translucent-looking
      // layers while keeping the brightest curve calm rather than dazzling.
      drawRibbon(cube, time - 0.54f, 2, 1.7f, false);
      drawRibbon(cube, time - 0.36f, 4, 1.1f, false);
      drawRibbon(cube, time - 0.18f, 7, 0.6f, false);
      drawRibbon(cube, time, 13, 0.0f, true);

      cube->update();
      usleep(speed);

      time += 0.025f;
      if (time > 2.0f * static_cast<float>(M_PI) * 100.0f) {
        time = std::fmod(time, 2.0f * static_cast<float>(M_PI));
      }
    }
  }
};

extern "C" Animation *create() {
  return new Silk;
}

extern "C" void destroy(Animation *animation) {
  delete animation;
}
