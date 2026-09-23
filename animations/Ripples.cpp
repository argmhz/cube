#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <unistd.h>

using json = nlohmann::json;

// A rippling surface: each point's height follows a sine of its distance
// from the centre, so waves travel outward the way they do when something
// drops into water. `axis` picks which way the wave rises -- or all three
// at once, so three sets of ripples run through each other -- and
// `mirrors` adds a reflection at the opposite face.
class Ripples : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  static constexpr int AXIS_ALONG_X = 0;
  static constexpr int AXIS_ALONG_Y = 1;
  static constexpr int AXIS_ALONG_Z = 2;
  static constexpr int ALL_AXES = 3;

  int speed = 30000;
  int axis = AXIS_ALONG_Z;
  int mirrors = 1;
  float spread = 0.35f;
  float flow = 1.0f;
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
    if (data["axis"].is_number()) {
      axis = std::clamp(data["axis"].get<int>(), 0, 3);
    }
    if (data["mirrors"].is_number()) {
      mirrors = std::clamp(data["mirrors"].get<int>(), 1, 2);
    }
    if (data["spread"].is_number()) {
      spread = std::clamp(data["spread"].get<float>(), 0.0f, 1.0f);
    }
    if (data["flow"].is_number()) {
      flow = std::clamp(data["flow"].get<float>(), 0.0f, 4.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  void draw(Cube *cube) override {
    // Distance from each point of the near quadrant to the cube's centre,
    // scaled by 10 so the wave phase advances in whole steps. Only 4x4 is
    // computed because the other three quadrants are its mirror images.
    //
    // This replaces a hand-written 16-entry table that was inconsistent:
    // it rounded some entries and truncated others, and one pair -- (3,1)
    // as 26 against (1,3) as 25 -- broke the very mirror symmetry the code
    // claimed to rely on. Computing it shifts three of the sixteen points
    // by a fraction of a step out of a 128-step wave.
    float distance[4][4];
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        distance[x][y] = 10.0f * std::sqrt((3.5f - x) * (3.5f - x) + (3.5f - y) * (3.5f - y));
      }
    }

    // Started at zero rather than left uninitialised, which is what the
    // original did before reading it -- the wave used to open on whatever
    // phase happened to be on the stack.
    float phase = 0.0f;

    while (isRunning()) {
      const int copies = mirrors;
      const int activeAxis = axis;
      cube->clear();

      for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
          // Same curve the original fixed-point sine table produced, just
          // computed directly: this runs on a machine with an FPU, not the
          // 8-bit microcontroller the lookup table was written for.
          const float angle = (distance[x][y] + phase) * (static_cast<float>(M_PI) / 64.0f);
          const int height = static_cast<int>((196.0f + 181.0f * std::sin(angle)) / 49.0f);

          const Rgb hue = rainbow(height * spread * 0.125f + phase * flow * 0.0005f);
          const int red = toLevel(hue.r);
          const int green = toLevel(hue.g);
          const int blue = toLevel(hue.b);

          // `axis` picks which way is "up" for the wave; the other two axes
          // carry the surface. Drawing every orientation through the same
          // helper is what makes ALL_AXES simply mean running all three.
          const auto plot = [&](int heightAxis, int h, int u, int v) {
            switch (heightAxis) {
              case AXIS_ALONG_X: cube->set(h, u, v, red, green, blue); break;
              case AXIS_ALONG_Y: cube->set(u, h, v, red, green, blue); break;
              default: cube->set(u, v, h, red, green, blue); break;
            }
          };

          const auto surface = [&](int heightAxis) {
            const int quadrants[4][2] = {{x, y}, {7 - x, y}, {x, 7 - y}, {7 - x, 7 - y}};
            for (const auto &q : quadrants) {
              plot(heightAxis, height, q[0], q[1]);
              // ...and its reflection at the opposite face.
              if (copies >= 2) {
                plot(heightAxis, 7 - height, q[0], q[1]);
              }
            }
          };

          if (activeAxis == ALL_AXES) {
            surface(AXIS_ALONG_X);
            surface(AXIS_ALONG_Y);
            surface(AXIS_ALONG_Z);
          } else {
            surface(activeAxis);
          }
        }
      }

      cube->update();
      phase = std::fmod(phase + 4.0f, 100000.0f);
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Ripples;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
