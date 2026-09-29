#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <unistd.h>

using json = nlohmann::json;

// One to three interlocking rings tumbling and drifting through the cube.
// Each ring is drawn as a soft volumetric tube -- brightness falls off with
// distance from the ring's centre line, so it reads as a glowing torus
// rather than a hard outline -- and the colour runs around the ring and
// cycles over time, so the whole palette drifts as the rings turn.
class Halo : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  // Defaults tuned against recordings from bin/simulator: a tighter tube and
  // a narrow colour span read as ribbons of light on an 8x8x8 grid, where a
  // wide glow and a full rainbow per ring just turn into confetti.
  int speed = 30000;
  int rings = 2;
  float radius = 2.4f;
  float thickness = 0.5f;
  float tumble = 0.5f;
  float spread = 0.3f;
  float flow = 1.0f;
  float brightness = 1.0f;
  float drift = 0.9f;
  float pulse = 0.2f;

  static float wrap(float value) {
    return value - std::floor(value);
  }

  // Fully saturated HSV->RGB in 0..1 floats. The conversion to the cube's
  // native 0..15 BAM levels happens once, at write-out, so the trail keeps
  // its precision while it decays.
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
      speed = std::clamp(data["speed"].get<int>(), 8000, 150000);
    }
    if (data["rings"].is_number()) {
      rings = std::clamp(data["rings"].get<int>(), 1, 3);
    }
    if (data["radius"].is_number()) {
      radius = std::clamp(data["radius"].get<float>(), 0.8f, 3.4f);
    }
    if (data["thickness"].is_number()) {
      thickness = std::clamp(data["thickness"].get<float>(), 0.3f, 2.5f);
    }
    if (data["tumble"].is_number()) {
      tumble = std::clamp(data["tumble"].get<float>(), 0.0f, 3.0f);
    }
    if (data["spread"].is_number()) {
      spread = std::clamp(data["spread"].get<float>(), 0.0f, 2.0f);
    }
    if (data["flow"].is_number()) {
      flow = std::clamp(data["flow"].get<float>(), 0.0f, 3.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.1f, 1.0f);
    }
    if (data["drift"].is_number()) {
      drift = std::clamp(data["drift"].get<float>(), 0.0f, 2.0f);
    }
    if (data["pulse"].is_number()) {
      pulse = std::clamp(data["pulse"].get<float>(), 0.0f, 0.7f);
    }
  }

  void draw(Cube *cube) override {
    float t = 0.0f;

    while (isRunning()) {
      // Snapshot the knobs that size loops/arrays, so a parameter arriving
      // mid-frame can't change them underneath us.
      const int ringCount = rings;
      const float tube = thickness;
      // Past this distance exp() returns less than half a BAM step, so the
      // voxel would round to black anyway -- skip the expensive maths.
      const float cutoff = 9.0f * tube * tube;

      const float pitch = t * 0.31f * tumble;
      const float yaw = t * 0.47f * tumble;
      const float cosPitch = std::cos(pitch), sinPitch = std::sin(pitch);
      const float cosYaw = std::cos(yaw), sinYaw = std::sin(yaw);

      float ringRadius[3];
      for (int i = 0; i < ringCount; i++) {
        // Offset phases so the rings breathe out of sync with each other.
        ringRadius[i] = radius * (1.0f + pulse * std::sin(t * 0.9f + i * 2.1f));
      }

      // The whole assembly wanders on a slow Lissajous path, so the rings
      // sweep through the volume instead of being pinned to the centre.
      const float driftX = drift * std::sin(t * 0.23f);
      const float driftY = drift * std::sin(t * 0.31f + 1.3f);
      const float driftZ = drift * std::sin(t * 0.19f + 2.7f);

      for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
          for (int x = 0; x < 8; x++) {
            const float px = x - 3.5f - driftX;
            const float py = y - 3.5f - driftY;
            const float pz = z - 3.5f - driftZ;

            // Tumble the sample point into the rings' own frame. Rotating
            // the 512 sample points is the same as rotating the rings, and
            // avoids having to re-voxelise geometry every frame.
            const float ty = py * cosPitch - pz * sinPitch;
            const float tz = py * sinPitch + pz * cosPitch;
            const float rx = px * cosYaw + tz * sinYaw;
            const float rz = tz * cosYaw - px * sinYaw;

            float red = 0.0f;
            float green = 0.0f;
            float blue = 0.0f;

            for (int i = 0; i < ringCount; i++) {
              // Cycling which axis is the ring's own axis puts each ring in
              // its own plane, so two or three of them interlock at right
              // angles without any extra trigonometry.
              float u, v, w;
              if (i == 0) {
                u = rx; v = ty; w = rz;
              } else if (i == 1) {
                u = rz; v = rx; w = ty;
              } else {
                u = ty; v = rz; w = rx;
              }

              // Distance from this voxel to the ring's centre line.
              const float offset = std::sqrt(u * u + v * v) - ringRadius[i];
              const float distance = offset * offset + w * w;
              if (distance > cutoff) {
                continue;
              }

              const float intensity = std::exp(-distance / (tube * tube));
              const float around = std::atan2(v, u) * 0.15915494f; // radians -> turns
              // Rings sit a small step apart on the colour wheel: enough to
              // tell them apart where they cross, close enough that two or
              // three of them still read as one palette rather than confetti.
              const Rgb hue = rainbow(around * spread + t * flow * 0.04f + i * 0.18f);

              // Rings add where they cross, so the intersections flare
              // brighter than either ring alone.
              red += hue.r * intensity;
              green += hue.g * intensity;
              blue += hue.b * intensity;
            }

            cube->set(x, y, z,
                      toLevel(std::min(red, 1.0f)),
                      toLevel(std::min(green, 1.0f)),
                      toLevel(std::min(blue, 1.0f)));
          }
        }
      }

      cube->update();
      t = std::fmod(t + 0.05f, 100000.0f);
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Halo;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
