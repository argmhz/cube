#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/animation/Font.h"
#include "../lib/vendor/json.hpp"

using json = nlohmann::json;

// Scrolling text on a billboard that turns in place. The text is laid out
// once as a ribbon of 8-pixel-tall columns, and every frame each voxel asks
// the ribbon whether it should be lit. Asking per voxel rather than pushing
// pixels in at an edge is what lets the sign rotate to any angle, thicken
// into a solid slab, and wrap around with no seam.
class Ticker : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  // onDataUpdate runs on the connection thread while draw() is rendering,
  // so the text is handed over under a lock and only picked up between
  // frames, rather than being rebuilt underneath the renderer.
  std::mutex settingsLock;
  std::string text = "Topper 3D";
  int gap = 8;
  bool layoutChanged = true;

  int speed = 60000;
  float spin = 0.35f;
  float thickness = 2.0f;
  int palette = 1;
  float hue = 0.0f;
  float flow = 0.5f;
  float brightness = 1.0f;
  float wobble = 0.0f;

  std::vector<std::array<int, 8>> ribbon;

  static float wrap(float value) {
    return value - std::floor(value);
  }

  static Rgb rainbow(float h) {
    h = wrap(h) * 6.0f;
    const int sector = static_cast<int>(std::floor(h));
    const float fraction = h - sector;

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

  // Lays the text out as one column per pixel of width, followed by `gap`
  // blank columns. Reading it modulo its length is what makes the text run
  // round and round with an even space between repeats.
  void buildRibbon() {
    std::string local;
    int blanks;
    {
      std::lock_guard<std::mutex> lock(settingsLock);
      local = Font::renderable(text);
      blanks = gap;
      layoutChanged = false;
    }

    ribbon.clear();
    for (char chr : local) {
      const std::array<std::array<int, 8>, 8> glyph = Font::asArray(chr);
      for (int column = 0; column < 8; column++) {
        std::array<int, 8> strip{};
        for (int row = 0; row < 8; row++) {
          strip[row] = glyph[row][column];
        }
        ribbon.push_back(strip);
      }
    }

    for (int i = 0; i < blanks; i++) {
      ribbon.push_back(std::array<int, 8>{});
    }
    if (ribbon.empty()) {
      ribbon.push_back(std::array<int, 8>{});
    }
  }

  void onDataUpdate(json data) override {
    if (data["text"].is_string()) {
      std::lock_guard<std::mutex> lock(settingsLock);
      text = data["text"].get<std::string>();
      layoutChanged = true;
    }
    if (data["gap"].is_number()) {
      std::lock_guard<std::mutex> lock(settingsLock);
      gap = std::clamp(data["gap"].get<int>(), 0, 32);
      layoutChanged = true;
    }
    if (data["speed"].is_number()) {
      speed = std::clamp(data["speed"].get<int>(), 10000, 250000);
    }
    if (data["spin"].is_number()) {
      spin = std::clamp(data["spin"].get<float>(), 0.0f, 3.0f);
    }
    if (data["thickness"].is_number()) {
      thickness = std::clamp(data["thickness"].get<float>(), 1.0f, 8.0f);
    }
    if (data["palette"].is_number()) {
      palette = std::clamp(data["palette"].get<int>(), 0, 2);
    }
    if (data["hue"].is_number()) {
      hue = std::clamp(data["hue"].get<float>(), 0.0f, 1.0f);
    }
    if (data["flow"].is_number()) {
      flow = std::clamp(data["flow"].get<float>(), 0.0f, 4.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
    if (data["wobble"].is_number()) {
      wobble = std::clamp(data["wobble"].get<float>(), 0.0f, 3.0f);
    }
  }

  void draw(Cube *cube) override {
    buildRibbon();

    float scroll = 0.0f;
    float angle = 0.0f;
    float t = 0.0f;

    while (isRunning()) {
      if (layoutChanged) {
        buildRibbon();
        scroll = std::fmod(scroll, static_cast<float>(ribbon.size()));
      }

      const int width = static_cast<int>(ribbon.size());
      const float cosAngle = std::cos(angle);
      const float sinAngle = std::sin(angle);
      const float halfThickness = thickness * 0.5f;
      const float baseHue = hue + t * flow * 0.01f;

      cube->clear();

      for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
          for (int x = 0; x < 8; x++) {
            const float px = x - 3.5f;
            const float pz = z - 3.5f;

            // Split the voxel's position into a distance along the sign and
            // a distance out through its face, so turning the sign is just
            // a change of angle rather than a different drawing routine.
            const float along = px * cosAngle + pz * sinAngle;
            const float across = pz * cosAngle - px * sinAngle;
            if (std::fabs(across) > halfThickness) {
              continue;
            }

            float vertical = static_cast<float>(y);
            if (wobble > 0.0f) {
              vertical -= wobble * std::sin(along * 0.55f + t * 0.9f);
            }
            const int row = static_cast<int>(std::lround(vertical));
            if (row < 0 || row > 7) {
              continue;
            }

            int column = static_cast<int>(std::floor(along + 3.5f + scroll)) % width;
            if (column < 0) {
              column += width;
            }
            if (!ribbon[column][row]) {
              continue;
            }

            float pixelHue = baseHue;
            if (palette == 1) {
              pixelHue += column * 0.035f;
            } else if (palette == 2) {
              pixelHue += row * 0.09f;
            }

            const Rgb color = rainbow(pixelHue);
            cube->set(x, y, z, toLevel(color.r), toLevel(color.g), toLevel(color.b));
          }
        }
      }

      cube->update();

      scroll = std::fmod(scroll + 1.0f, static_cast<float>(width));
      angle = std::fmod(angle + spin * 0.05f, 2.0f * static_cast<float>(M_PI));
      t += 1.0f;
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Ticker;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
