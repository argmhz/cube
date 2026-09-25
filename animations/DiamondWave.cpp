#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>

using json = nlohmann::json;

// A pair of diamond-shaped cones (one opening up, one opening down) that
// sweep through the cube. On any horizontal slice, the cone's radius at
// that height traces a circle -- widest in the middle, narrowing to a
// point at top and bottom -- which is why it reads as a diamond/pyramid
// shape as it moves through the 8x8x8 grid.
class DiamondWave : public Animation {
  int speed = 30000;

  // Compact HSV-to-RGB, saturation fixed at 100%, brightness a native
  // 0..15 BAM level (same approach as Nebula.cpp's rainbow()).
  static Cube::Color rainbow(float hue, int brightness) {
    hue -= std::floor(hue);
    hue *= 6.0f;
    const int sector = static_cast<int>(std::floor(hue));
    const float fraction = hue - sector;
    const int rising = static_cast<int>(std::round(brightness * fraction));
    const int falling = brightness - rising;

    switch (sector % 6) {
      case 0: return {brightness, rising, 0};
      case 1: return {falling, brightness, 0};
      case 2: return {0, brightness, rising};
      case 3: return {0, falling, brightness};
      case 4: return {rising, 0, brightness};
      default: return {brightness, 0, falling};
    }
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
  }

  void drawLevel(Cube *cube, int i, Cube::Color color) {
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        float radius = std::sqrt((3.5f - x) * (3.5f - x) + (3.5f - y) * (3.5f - y));
        int top = static_cast<int>(i - 9.5f + radius);
        int bottom = static_cast<int>(i - 0.5f - radius);

        if (top > -1 && top < 8) {
          cube->set(x, y, top, color);
        }
        if (bottom > -1 && bottom < 8) {
          cube->set(x, y, bottom, color);
        }
      }
    }
  }

  void draw(Cube *cube) override {
    float hue = 0.0f;

    while (isRunning()) {
      // The colour shifts once per full up-and-down pass rather than every
      // frame, so a single sweep reads as one solid colour.
      Cube::Color color = rainbow(hue, MAX_COLOR);
      hue += 0.05f;

      for (int i = 0; i < 18; i++) {
        cube->clear();
        drawLevel(cube, i, color);
        cube->update();
        usleep(speed);
      }
      for (int i = 16; i > 0; i--) {
        cube->clear();
        drawLevel(cube, i, color);
        cube->update();
        usleep(speed);
      }
    }
  }
};
extern "C" Animation *create() {
  return new DiamondWave;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
