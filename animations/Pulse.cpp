#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <unistd.h>

using json = nlohmann::json;

// Hollow shells that grow out of the cube's centre, one after another, each
// a step further round the colour wheel than the last -- like rings on water,
// but in three dimensions.
class Pulse : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  // Centre to corner of the 8x8x8 grid, measured from the middle at 3.5.
  static constexpr float MAX_RADIUS = 6.1f;
  // How far either side of a shell's surface a voxel still glows, in voxels.
  static constexpr float SHELL_HALF_WIDTH = 0.7f;

  int speed = 40000;
  float gap = 2.5f;
  float brightness = 1.0f;

  static float wrap(float value) {
    return value - std::floor(value);
  }

  static Rgb rainbow(float hue) {
    hue = wrap(hue) * 6.0f;
    const int sector = static_cast<int>(std::floor(hue));
    const float fraction = hue - sector;

    switch (sector % 6) {
      case 0: return {1.0f, fraction, 0.0f};
      case 1: return {1.0f - fraction, 1.0f, 0.0f};
      case 2: return {0.0f, 1.0f, fraction};
      case 3: return {0.0f, 1.0f - fraction, 1.0f};
      case 4: return {fraction, 0.0f, 1.0f};
      default: return {1.0f, 0.0f, 1.0f - fraction};
    }
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["gap"].is_number()) {
      gap = std::clamp(data["gap"].get<float>(), 1.5f, 6.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  void draw(Cube *cube) override {
    float distance[8][8][8];
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        for (int z = 0; z < 8; z++) {
          const float dx = x - 3.5f;
          const float dy = y - 3.5f;
          const float dz = z - 3.5f;
          distance[x][y][z] = std::sqrt(dx * dx + dy * dy + dz * dz);
        }
      }
    }

    float travelled = 0.0f;

    while (isRunning()) {
      const float spacing = gap;
      cube->clear();

      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            // How far behind the newest shell this voxel sits, counted in
            // shells: the whole part says which shell, the fraction how
            // close to its surface.
            const float behind = (travelled - distance[x][y][z]) / spacing;
            if (behind < -0.5f) {
              continue;
            }
            const float offset = wrap(behind);
            const float closeness = 1.0f - std::min(offset, 1.0f - offset) * spacing / SHELL_HALF_WIDTH;
            if (closeness <= 0.0f) {
              continue;
            }

            // Shells fade as they near the corners instead of popping out.
            const float fade = std::clamp(1.0f - distance[x][y][z] / MAX_RADIUS, 0.25f, 1.0f);
            // Each shell keeps its colour as it travels outwards.
            const Rgb hue = rainbow(std::lround(behind) * 0.13f);
            const float level = closeness * fade;
            cube->set(x, y, z, toLevel(hue.r * level), toLevel(hue.g * level), toLevel(hue.b * level));
          }
        }
      }

      cube->update();
      // Kept small so float precision holds. Stepping back a hundred whole
      // shells is invisible: the pattern repeats every shell, and a hundred
      // hue steps of 0.13 land back on the same colour.
      travelled += 0.25f;
      if (travelled > MAX_RADIUS + 200.0f * spacing) {
        travelled -= 100.0f * spacing;
      }
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Pulse;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
