#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <unistd.h>

using json = nlohmann::json;

// The rubber-sheet picture of gravity: a flat sheet stretched across the
// cube stands in for space, and a heavy sphere dropped onto it bends it
// into a well. The sphere lands, bounces on the sheet a few times and
// settles at the bottom of the dip; the impact sends ripples out to the
// edges. A small marble circles the well -- it isn't pulled by the sphere,
// it's just following the curve -- which is the point of the picture.
// Then the sphere lifts off, the sheet springs back flat, and it repeats.
//
// Y is up. The sheet rests at REST_HEIGHT; heavier `mass` sinks deeper.
class GravityWell : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  static constexpr float CENTRE = 3.5f;
  static constexpr float REST_HEIGHT = 5.0f;
  // Where the sphere starts and ends, above the top of the cube so it
  // enters and leaves out of sight.
  static constexpr float START_HEIGHT = 11.0f;
  static constexpr float GRAVITY = 9.0f;
  static constexpr float LIFT_SPEED = 3.0f;
  // Simulated seconds advanced per frame. Fixed, so `speed` changes the
  // playback rate rather than the physics.
  static constexpr float DT = 0.035f;

  enum Phase { FALLING, HOLDING, LIFTING, EMPTY };

  int speed = 30000;
  float mass = 2.5f;
  float size = 1.4f;
  float hold = 3.0f;
  int grid = 0;
  int orbit = 1;
  float brightness = 1.0f;

  // Current shape of the sheet, read by height().
  float depth = 0.0f;
  float rippleAmp = 0.0f;
  float rippleAge = 0.0f;

  static Rgb mix(Rgb a, Rgb b, float k) {
    return {a.r + (b.r - a.r) * k, a.g + (b.g - a.g) * k, a.b + (b.b - a.b) * k};
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      speed = std::clamp(data["speed"].get<int>(), 8000, 150000);
    }
    if (data["mass"].is_number()) {
      mass = std::clamp(data["mass"].get<float>(), 0.8f, 4.0f);
    }
    if (data["size"].is_number()) {
      size = std::clamp(data["size"].get<float>(), 0.8f, 2.2f);
    }
    if (data["hold"].is_number()) {
      hold = std::clamp(data["hold"].get<float>(), 0.5f, 20.0f);
    }
    if (data["grid"].is_number()) {
      grid = std::clamp(data["grid"].get<int>(), 0, 1);
    }
    if (data["orbit"].is_number()) {
      orbit = std::clamp(data["orbit"].get<int>(), 0, 1);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.1f, 1.0f);
    }
  }

  // How far below rest the sheet is at distance r from the centre, without
  // ripples. A softened 1/r: steep near the mass, flattening out towards
  // the edges, the way the textbook diagrams draw it. Scaled so the dip
  // directly under the sphere is exactly `depth`.
  float wellShape(float r) const {
    const float w = size * 1.1f + 1.0f;
    return depth * (w * w) / (r * r + w * w);
  }

  float height(float x, float z) const {
    const float dx = x - CENTRE, dz = z - CENTRE;
    const float r = std::sqrt(dx * dx + dz * dz);
    float h = REST_HEIGHT - wellShape(r);
    if (rippleAmp > 0.0f) {
      // Rings travelling outward from the impact, fading as they go. Faded
      // in over the sphere's own radius, so the ground it rests on stays
      // still.
      const float under = std::clamp(r / size, 0.0f, 1.0f);
      h += rippleAmp * std::exp(-rippleAge * 1.1f) * std::cos(2.0f * r - 6.0f * rippleAge) * under;
    }
    return h;
  }

  void draw(Cube *cube) override {
    Phase phase = FALLING;
    float phaseTime = 0.0f;
    float sphereY = START_HEIGHT;
    float sphereV = 0.0f;
    bool touching = false;
    float marbleAngle = 0.0f;

    while (isRunning()) {
      const float radius = size;
      // Spring constant chosen so the sheet balances the sphere's weight
      // exactly `mass` voxels down: a heavier object bends space further.
      const float stiffness = GRAVITY / mass;
      // Critically-ish damped: two or three visible bounces, then still.
      const float damping = 0.7f * std::sqrt(stiffness);

      phaseTime += DT;

      // How far the sphere pushes the sheet below rest. The sheet wraps
      // the sphere while they touch, so this is also the well's depth.
      const float press = std::max(0.0f, REST_HEIGHT - (sphereY - radius));

      switch (phase) {
        case FALLING:
        case HOLDING: {
          float accel = -GRAVITY;
          if (press > 0.0f) {
            accel += stiffness * press - damping * sphereV;
          }
          sphereV += accel * DT;
          sphereY += sphereV * DT;
          if (phase == FALLING && phaseTime > 1.5f && std::fabs(sphereV) < 0.08f && press > 0.0f) {
            phase = HOLDING;
            phaseTime = 0.0f;
          }
          // Give up waiting to settle if it takes unreasonably long.
          if (phase == FALLING && phaseTime > 12.0f) {
            phase = HOLDING;
            phaseTime = 0.0f;
          }
          if (phase == HOLDING && phaseTime > hold) {
            phase = LIFTING;
            phaseTime = 0.0f;
          }
          break;
        }
        case LIFTING:
          sphereV = LIFT_SPEED;
          sphereY += sphereV * DT;
          if (sphereY - radius > 8.0f) {
            phase = EMPTY;
            phaseTime = 0.0f;
          }
          break;
        case EMPTY:
          if (phaseTime > 1.8f) {
            phase = FALLING;
            phaseTime = 0.0f;
            sphereY = START_HEIGHT;
            sphereV = 0.0f;
          }
          break;
      }

      // Contact and release both shake the sheet.
      const bool nowTouching = REST_HEIGHT - (sphereY - radius) > 0.0f;
      if (nowTouching != touching) {
        rippleAmp = std::min(0.8f, 0.15f + std::fabs(sphereV) * 0.08f);
        rippleAge = 0.0f;
        touching = nowTouching;
      }
      rippleAge += DT;
      if (rippleAge > 6.0f) {
        rippleAmp = 0.0f;
      }

      // Keep the bottom of the well inside the cube, even when a heavy
      // sphere overshoots on its first bounce.
      const float floorLimit = REST_HEIGHT - 0.2f;
      depth = std::min(floorLimit, std::max(0.0f, REST_HEIGHT - (sphereY - radius)));

      cube->clear();

      // The sheet. Each voxel is lit by its distance to the surface; the
      // vertical gap is divided by the slope so the steep walls of the well
      // stay one voxel thick instead of breaking into floating dots.
      const Rgb flat = {0.0f, 0.25f, 0.7f};
      const Rgb deep = {0.8f, 0.0f, 0.9f};
      for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
          const float h = height(x, z);
          const float gx = height(x + 0.5f, z) - height(x - 0.5f, z);
          const float gz = height(x, z + 0.5f) - height(x, z - 0.5f);
          const float slope = std::sqrt(1.0f + gx * gx + gz * gz);
          const float sag = std::clamp((REST_HEIGHT - h) / 3.5f, 0.0f, 1.0f);
          const bool onLine = (x % 2 == 0) || (z % 2 == 0);
          const float lineScale = grid && !onLine ? 0.12f : 1.0f;
          const Rgb colour = mix(flat, deep, sag);

          for (int y = 0; y < 8; y++) {
            const float distance = std::fabs(y - h) / slope;
            const float intensity = std::clamp(1.0f - distance / 0.75f, 0.0f, 1.0f) * lineScale;
            if (intensity <= 0.0f) {
              continue;
            }
            // The sphere covers the part of the sheet it's sitting in.
            const float sx = x - CENTRE, sy = y - sphereY, sz = z - CENTRE;
            if (sx * sx + sy * sy + sz * sz < radius * radius) {
              continue;
            }
            cube->set(x, y, z, toLevel(colour.r * intensity), toLevel(colour.g * intensity),
                      toLevel(colour.b * intensity));
          }
        }
      }

      // The sphere: solid, with a soft edge, lit brighter from above.
      if (sphereY - radius < 8.0f) {
        for (int x = 0; x < 8; x++) {
          for (int y = 0; y < 8; y++) {
            for (int z = 0; z < 8; z++) {
              const float dx = x - CENTRE, dy = y - sphereY, dz = z - CENTRE;
              const float d = std::sqrt(dx * dx + dy * dy + dz * dz);
              const float edge = std::clamp(radius + 0.5f - d, 0.0f, 1.0f);
              if (edge <= 0.0f) {
                continue;
              }
              const float light = 0.55f + 0.45f * std::clamp(dy / radius, -1.0f, 1.0f);
              cube->set(x, y, z, toLevel(1.0f * edge * light), toLevel(0.55f * edge * light),
                        toLevel(0.05f * edge * light));
            }
          }
        }
      }

      // The marble: rides the sheet on a slightly elliptical path, faster
      // when the well is deep, like a coin in a charity funnel. Only shown
      // while there is a well to roll around in.
      if (orbit && depth > 0.4f) {
        marbleAngle += DT * (0.8f + 0.6f * depth);
        const float r = 2.7f + 0.4f * std::sin(marbleAngle * 0.7f);
        const float mx = CENTRE + r * std::cos(marbleAngle);
        const float mz = CENTRE + r * std::sin(marbleAngle);
        const float my = height(mx, mz) + 0.6f;
        for (int x = 0; x < 8; x++) {
          for (int y = 0; y < 8; y++) {
            for (int z = 0; z < 8; z++) {
              const float dx = x - mx, dy = y - my, dz = z - mz;
              const float glow = std::exp(-(dx * dx + dy * dy + dz * dz) * 2.2f);
              if (glow < 0.2f) {
                continue;
              }
              cube->set(x, y, z, toLevel(glow * 0.6f), toLevel(glow), toLevel(glow * 0.6f));
            }
          }
        }
      }

      cube->update();
      usleep(speed);
    }
  }
};

extern "C" Animation *create() {
  return new GravityWell;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
