#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <unistd.h>

using json = nlohmann::json;

// A donut pulled into an oval and twisted: the tube is flat rather than
// round, and that flat cross-section turns as it goes round the ring, so the
// band shows its face in some places and its edge in others. The twist
// slowly rolls round the ring while the whole thing tumbles, and a rainbow
// runs round it lengthwise.
class Donut : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  struct Vec {
    float x;
    float y;
    float z;
  };

  // Points along the ring's centreline that each voxel is measured against.
  static constexpr int SAMPLES = 128;
  // Half-width and half-thickness of the flat tube, in voxels. Thinner than
  // about 0.7 and the band starts falling through the gaps between voxels.
  static constexpr float TUBE_WIDTH = 1.5f;
  static constexpr float TUBE_THICKNESS = 0.8f;

  int speed = 30000;
  float stretch = 1.6f;
  int twists = 2;
  float spin = 1.0f;
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
    if (data["stretch"].is_number()) {
      stretch = std::clamp(data["stretch"].get<float>(), 1.0f, 2.2f);
    }
    // Counted in half turns: the flat cross-section looks the same after
    // half a turn, so any whole number of them closes up seamlessly.
    if (data["twists"].is_number()) {
      twists = std::clamp(data["twists"].get<int>(), 0, 6);
    }
    if (data["spin"].is_number()) {
      spin = std::clamp(data["spin"].get<float>(), 0.0f, 3.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  void draw(Cube *cube) override {
    float time = 0.0f;

    while (isRunning()) {
      const float along = stretch;
      const int halfTurns = twists;

      // The ring is an ellipse, long way along x. Its area stays roughly the
      // same however far it is stretched, so it keeps fitting in the cube.
      const float longRadius = 2.5f * std::sqrt(along);
      const float shortRadius = 2.5f / std::sqrt(along);

      // Tumble: turn about the vertical axis and, more slowly, tip over.
      const float yaw = time * 0.9f;
      const float tilt = 0.6f + 0.5f * std::sin(time * 0.37f);
      const float cosYaw = std::cos(yaw), sinYaw = std::sin(yaw);
      const float cosTilt = std::cos(tilt), sinTilt = std::sin(tilt);
      const float roll = time * 1.3f;

      cube->clear();

      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            // Into the donut's own frame: undo the yaw, then the tilt.
            const Vec world = {x - 3.5f, y - 3.5f, z - 3.5f};
            const Vec yawed = {cosYaw * world.x + sinYaw * world.z, world.y, -sinYaw * world.x + cosYaw * world.z};
            const Vec p = {yawed.x, cosTilt * yawed.y + sinTilt * yawed.z, -sinTilt * yawed.y + cosTilt * yawed.z};

            // Nearest point on the centreline, which lies in the xz plane.
            float best = 1e9f;
            float angle = 0.0f;
            for (int i = 0; i < SAMPLES; i++) {
              const float a = i * (2.0f * static_cast<float>(M_PI) / SAMPLES);
              const float dx = p.x - longRadius * std::cos(a);
              const float dz = p.z - shortRadius * std::sin(a);
              const float d = dx * dx + p.y * p.y + dz * dz;
              if (d < best) {
                best = d;
                angle = a;
              }
            }
            if (best > TUBE_WIDTH * TUBE_WIDTH) {
              continue;
            }

            // Split the offset from the centreline into "outwards in the
            // ring's plane" and "up out of it".
            float outX = shortRadius * std::cos(angle);
            float outZ = longRadius * std::sin(angle);
            const float length = std::sqrt(outX * outX + outZ * outZ);
            outX /= length;
            outZ /= length;
            const float outwards = (p.x - longRadius * std::cos(angle)) * outX + (p.z - shortRadius * std::sin(angle)) * outZ;
            const float upwards = p.y;

            // Turn the cross-section by the twist at this point on the ring.
            const float twist = angle * halfTurns * 0.5f + roll;
            const float c = std::cos(twist), s = std::sin(twist);
            const float across = (c * outwards + s * upwards) / TUBE_WIDTH;
            const float through = (-s * outwards + c * upwards) / TUBE_THICKNESS;
            const float inside = across * across + through * through;
            if (inside > 1.0f) {
              continue;
            }

            // Rainbow round the ring, drifting; the band's middle is a
            // little brighter than its edges so the twist reads.
            const Rgb hue = rainbow(angle / (2.0f * static_cast<float>(M_PI)) + time * 0.05f);
            const float level = 0.55f + 0.45f * (1.0f - across * across);
            cube->set(x, y, z, toLevel(hue.r * level), toLevel(hue.g * level), toLevel(hue.b * level));
          }
        }
      }

      cube->update();
      // Wrapped at a whole cycle of every motion above would need their
      // common period; instead just keep it from growing past where float
      // steps become visible.
      time += 0.03f * spin;
      if (time > 5000.0f) {
        time -= 5000.0f;
      }
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Donut;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
