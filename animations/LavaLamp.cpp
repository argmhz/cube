#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>

using json = nlohmann::json;

// Several soft, warm-coloured blobs bobbing slowly up and down like wax in
// a lava lamp. Each blob follows its own sine-wave bob (a sine's velocity
// is lowest at the extremes and highest through the middle, which is
// exactly the "dwell at the top and bottom, drift through the middle" look
// real lava lamp blobs have -- no easing curve needed, the sine already
// is one) with its own period and phase, so they drift in and out of sync
// rather than moving in lockstep.
class LavaLamp : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  struct Blob {
    float phaseX, phaseY, phaseZ;
    float speedX, speedY, speedZ;
    float baseRadius;
    float hueOffset;
    float pulsePhase;
    float pulseSpeed;
  };

  static constexpr int MAX_BLOBS = 6;
  Blob blobs[MAX_BLOBS];

  int speed = 45000;
  int blobCount = 4;
  float spread = 0.8f;
  float hue = 0.02f;        // warm red-orange by default
  float hueSpread = 0.06f;  // per-blob colour variation
  float brightness = 1.0f;
  float pulse = 0.15f;      // how much each blob "breathes"

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

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["blobs"].is_number()) {
      blobCount = std::clamp(data["blobs"].get<int>(), 1, MAX_BLOBS);
    }
    if (data["spread"].is_number()) {
      spread = std::clamp(data["spread"].get<float>(), 0.4f, 3.0f);
    }
    if (data["hue"].is_number()) {
      hue = std::clamp(data["hue"].get<float>(), 0.0f, 1.0f);
    }
    if (data["hueSpread"].is_number()) {
      hueSpread = std::clamp(data["hueSpread"].get<float>(), 0.0f, 0.3f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
    if (data["pulse"].is_number()) {
      pulse = std::clamp(data["pulse"].get<float>(), 0.0f, 0.6f);
    }
  }

  static float randf(float lo, float hi) {
    return lo + (static_cast<float>(rand() % 10000) / 10000.0f) * (hi - lo);
  }

  void draw(Cube *cube) override {
    for (int i = 0; i < MAX_BLOBS; i++) {
      Blob &b = blobs[i];
      b.phaseX = randf(0.0f, 6.28f);
      b.phaseY = randf(0.0f, 6.28f);
      b.phaseZ = randf(0.0f, 6.28f);
      // Vertical bob is deliberately much slower than the horizontal drift
      // -- that asymmetry is what reads as "rising through liquid" rather
      // than a blob just orbiting in place.
      b.speedY = randf(0.15f, 0.30f);
      b.speedX = randf(0.05f, 0.12f);
      b.speedZ = randf(0.05f, 0.12f);
      b.baseRadius = randf(1.1f, 1.7f);
      b.hueOffset = randf(-1.0f, 1.0f);
      b.pulsePhase = randf(0.0f, 6.28f);
      b.pulseSpeed = randf(0.4f, 0.8f);
    }

    float t = 0.0f;

    while (isRunning()) {
      const int active = std::clamp(blobCount, 1, MAX_BLOBS);

      float bx[MAX_BLOBS], by[MAX_BLOBS], bz[MAX_BLOBS], brad[MAX_BLOBS];
      Rgb bcol[MAX_BLOBS];
      for (int i = 0; i < active; i++) {
        Blob &b = blobs[i];
        bx[i] = 3.5f + 1.3f * std::sin(t * b.speedX + b.phaseX);
        by[i] = 3.5f + 2.6f * std::sin(t * b.speedY + b.phaseY);
        bz[i] = 3.5f + 1.3f * std::sin(t * b.speedZ + b.phaseZ);
        float wobble = 1.0f + pulse * std::sin(t * b.pulseSpeed + b.pulsePhase);
        brad[i] = spread * b.baseRadius * wobble;
        bcol[i] = rainbow(hue + hueSpread * b.hueOffset);
      }

      for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
          for (int x = 0; x < 8; x++) {
            float redSum = 0.0f, greenSum = 0.0f, blueSum = 0.0f, weight = 0.0f;

            for (int i = 0; i < active; i++) {
              float dx = x - bx[i], dy = y - by[i], dz = z - bz[i];
              float dist2 = dx * dx + dy * dy + dz * dz;
              float glow = std::exp(-dist2 / (brad[i] * brad[i]));
              redSum += bcol[i].r * glow;
              greenSum += bcol[i].g * glow;
              blueSum += bcol[i].b * glow;
              weight += glow;
            }

            if (weight < 0.004f) {
              cube->set(x, y, z, 0, 0, 0);
              continue;
            }

            // Blended colour where blobs overlap, brightness that climbs
            // toward full where several blobs merge -- reads as denser wax
            // rather than the two glows just adding up to a washed-out mess.
            float level = std::min(weight, 1.0f);
            cube->set(x, y, z,
                      toLevel(redSum / weight * level),
                      toLevel(greenSum / weight * level),
                      toLevel(blueSum / weight * level));
          }
        }
      }

      cube->update();
      t += 0.06f;
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new LavaLamp;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
