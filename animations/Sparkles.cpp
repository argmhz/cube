#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>

using json = nlohmann::json;

class Sparkles : public Animation {
  int speed = 100000;
  int numVoxels = 100;

  // Compact HSV-to-RGB, saturation fixed at 100%, brightness a native
  // 0..15 BAM level (same approach as Nebula.cpp's rainbow()) -- gives each
  // sparkle its own randomly-picked colour instead of plain white.
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
    if (data["voxels"].is_number()) {
      int value = data["voxels"].get<int>();
      if (value > 0 && value <= 400) {
        numVoxels = value;
      }
    }
  }

  void draw(Cube *cube) override {
    while (isRunning()) {
      for (int v = 0; v < numVoxels; v++) {
        Cube::Color color = rainbow(static_cast<float>(rand() % 360) / 360.0f, MAX_COLOR);
        cube->set(rand() % 8, rand() % 8, rand() % 8, color);
      }

      cube->update();
      usleep(speed);
      cube->clear();
    }
  }
};
extern "C" Animation *create() {
  return new Sparkles;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
