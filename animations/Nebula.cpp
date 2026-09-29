#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"

#include <algorithm>
#include <cmath>
#include <unistd.h>

// A soft, glowing orb of light drifting through the cube on a lazy
// Lissajous path, its hue cycling slowly -- brightness falls off gently
// with distance from the orb's center (real 4-bit BAM fades, not a hard
// on/off sphere). Inspired by soft plasma-glow LED cube builds.
class Nebula : public Animation {
  int speed = 35000;
  // Gaussian falloff width. On an 8-wide cube even the un-widened glow
  // already reaches most voxels, so this is deliberately tight -- kept
  // tunable live (see onDataUpdate) since exactly how a given glow radius
  // reads depends on the physical LEDs/diffuser, not just the numbers.
  float spread = 1.6f;
  // Base glow added everywhere, 0..1 of full brightness. Defaults to 0 --
  // a real cube's LEDs make even a small ambient floor on all 512 voxels
  // read as a much stronger background wash than the same numbers look
  // like on a screen.
  float ambient = 0.0f;

  static float wrap(float value) {
    value -= std::floor(value);
    return value;
  }

  // Compact HSV-to-RGB with saturation fixed at 100%, brightness a native
  // 0..15 BAM level (same approach as ChromaticBloom.cpp's rainbow()).
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
      speed = std::clamp(data["speed"].get<int>(), 15000, 200000);
    }
    if (data["spread"].is_number()) {
      spread = std::clamp(data["spread"].get<float>(), 0.4f, 6.0f);
    }
    if (data["ambient"].is_number()) {
      ambient = std::clamp(data["ambient"].get<float>(), 0.0f, 0.3f);
    }
  }

  void draw(Cube *cube) override {
    const float CENTER = 3.5f;
    const float RADIUS = 2.4f; // how far the orb drifts from the center

    float t = 0.0f;

    while (isRunning()) {
      float ox = CENTER + RADIUS * std::sin(t * 0.6f);
      float oy = CENTER + RADIUS * std::sin(t * 0.44f + 2.1f);
      float oz = CENTER + RADIUS * std::sin(t * 0.81f + 4.6f);

      Cube::Color hue = rainbow(t * 0.03f, 15);

      for (int z = 0; z < 8; ++z) {
        for (int y = 0; y < 8; ++y) {
          for (int x = 0; x < 8; ++x) {
            float dx = x - ox, dy = y - oy, dz = z - oz;
            float dist2 = dx * dx + dy * dy + dz * dz;
            float glow = std::exp(-dist2 / spread);
            float level = glow + ambient * (1.0f - glow);

            int r = (int)std::round(hue.red   * level);
            int g = (int)std::round(hue.green * level);
            int b = (int)std::round(hue.blue  * level);
            cube->set(x, y, z, r, g, b);
          }
        }
      }

      cube->update();
      t = std::fmod(t + 0.05f, 100000.0f);
      usleep(speed);
    }
  }
};
extern "C" Animation * create() {
    return new Nebula;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
