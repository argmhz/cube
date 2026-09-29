#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"

#include <algorithm>
#include <cmath>
#include <unistd.h>

class ChromaticBloom : public Animation {
  int speed = 55000;

  static float wrap(float value) {
    value -= std::floor(value);
    return value;
  }

  // A compact HSV rainbow with saturation fixed at 100%. Brightness is a
  // native 0..15 BAM level, not an approximation made by skipping frames.
  static Cube::Color rainbow(float hue, int brightness) {
    hue = wrap(hue) * 6.0f;
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

      for (int z = 0; z < 8; ++z) {
        for (int y = 0; y < 8; ++y) {
          for (int x = 0; x < 8; ++x) {
            const float dx = x - 3.5f;
            const float dy = y - 3.5f;
            const float dz = z - 3.5f;
            const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

            // Two waves moving in opposite directions interfere to form
            // slowly opening petals rather than a uniformly filled sphere.
            const float outward = std::sin(distance * 2.35f - time);
            const float inward = std::sin(distance * 1.45f + time * 0.61f +
                                          std::atan2(dy, dx) * 2.0f);
            const float petal = 0.58f * outward + 0.42f * inward;
            const float shaped = std::max(0.0f, petal - 0.03f) / 0.97f;
            const int light = std::clamp(
                static_cast<int>(std::round(std::pow(shaped, 1.45f) * 15.0f)),
                0, MAX_COLOR);

            // Keeping level 1 dark leaves air between the petals. Levels
            // 2..15 create the soft BAM falloff at every colored edge.
            if (light < 2) {
              continue;
            }

            const float azimuth = std::atan2(dy, dx) / (2.0f * M_PI);
            const float elevation = dz / 14.0f;
            const float hue = azimuth + elevation + time * 0.018f;
            cube->set(x, y, z, rainbow(hue, light));
          }
        }
      }

      // A quiet white heartbeat anchors the otherwise saturated volume.
      const int heart = 5 + static_cast<int>(
          std::round(3.0f * (0.5f + 0.5f * std::sin(time * 0.73f))));
      cube->set(3, 3, 3, heart, heart, heart);
      cube->set(4, 4, 4, heart, heart, heart);

      cube->update();
      usleep(speed);

      time += 0.055f;
      if (time > 200.0f * static_cast<float>(M_PI)) {
        time = std::fmod(time, 2.0f * static_cast<float>(M_PI));
      }
    }
  }
};

extern "C" Animation *create() {
  return new ChromaticBloom;
}

extern "C" void destroy(Animation *animation) {
  delete animation;
}
