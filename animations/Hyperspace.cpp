#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <random>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

// Flying through space, then jumping to hyperspace. Stars come at you out of
// the back of the cube and spread outwards as they pass, the way they would
// through a cockpit window. Every so often the engines spool up: the stars
// stretch into longer and longer streaks, a white flash marks the jump, and
// you are in a tunnel of blue-white streaks swirling along the walls. Another
// flash drops you back out, the streaks shrink back to points, and the calm
// starfield carries on.
class Hyperspace : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  struct Star {
    float angle;
    // Distance from the axis at the far end, and how much further out it
    // gets by the time it reaches you.
    float start;
    float spread;
    // 0 at the back of the cube, 1 at the front.
    float progress;
    Rgb colour;
  };

  enum Phase { CRUISE, CHARGE, HYPER, DROP };

  static constexpr int MODE_JUMPS = 0;
  static constexpr int MODE_CRUISE = 1;
  static constexpr int MODE_HYPER = 2;

  static constexpr float CENTRE = 3.5f;
  static constexpr float TWO_PI = 2.0f * static_cast<float>(M_PI);

  // Speeds in voxels per second along the flight direction.
  static constexpr float CRUISE_SPEED = 3.5f;
  static constexpr float HYPER_SPEED = 32.0f;
  // How long a streak is: the distance covered in this many seconds.
  static constexpr float STREAK_TIME = 0.14f;
  static constexpr float MAX_STREAK = 6.5f;

  static constexpr float CRUISE_TIME = 8.0f;
  static constexpr float CHARGE_TIME = 2.5f;
  static constexpr float HYPER_TIME = 5.0f;
  static constexpr float DROP_TIME = 1.5f;
  static constexpr float FLASH_TIME = 0.3f;

  std::atomic<int> mode{MODE_JUMPS};
  std::atomic<int> direction{0};
  std::atomic<float> density{1.0f};
  std::atomic<float> pace{1.0f};
  std::atomic<float> brightness{1.0f};

  std::mt19937 random{std::random_device{}()};
  std::vector<Star> stars;
  float buffer[8][8][8][3] = {};

  float uniform(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random);
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void onDataUpdate(json data) override {
    if (data["mode"].is_number()) {
      mode = std::clamp(data["mode"].get<int>(), 0, 2);
    }
    if (data["direction"].is_number()) {
      direction = std::clamp(data["direction"].get<int>(), 0, 1);
    }
    if (data["density"].is_number()) {
      density = std::clamp(data["density"].get<float>(), 0.3f, 1.5f);
    }
    if (data["pace"].is_number()) {
      pace = std::clamp(data["pace"].get<float>(), 0.25f, 3.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  // Out in normal space: points of light scattered round the middle, mostly
  // white with the odd blue or yellow one.
  Star cruiser(float progress) {
    const float tint = uniform(0.0f, 1.0f);
    const Rgb colour = tint < 0.2f ? Rgb{0.5f, 0.65f, 1.0f} : tint < 0.35f ? Rgb{1.0f, 0.85f, 0.45f} : Rgb{0.85f, 0.85f, 0.9f};
    return {uniform(0.0f, TWO_PI), uniform(0.0f, 1.6f), uniform(1.5f, 3.2f), progress, colour};
  }

  // In hyperspace: streaks hugging the walls of a tunnel.
  Star streak(float progress) {
    const float white = uniform(0.0f, 1.0f);
    return {uniform(0.0f, TWO_PI), uniform(2.4f, 3.3f), uniform(0.0f, 0.4f), progress,
            {0.3f + 0.6f * white, 0.55f + 0.4f * white, 1.0f}};
  }

  void fill(bool tunnel, int count) {
    stars.clear();
    for (int i = 0; i < count; i++) {
      stars.push_back(tunnel ? streak(uniform(0.0f, 1.0f)) : cruiser(uniform(0.0f, 1.0f)));
    }
  }

  // Where a star is at a given point of its flight, in cube coordinates
  // before any flip. Stars spread outwards faster the closer they get.
  static void place(const Star &s, float progress, float twist, float &x, float &y, float &z) {
    const float radius = s.start + s.spread * progress * progress;
    x = CENTRE + radius * std::cos(s.angle + twist * progress);
    y = CENTRE + radius * std::sin(s.angle + twist * progress);
    z = progress * 7.0f;
  }

  void light(int x, int y, int z, Rgb colour, float amount) {
    if (x < 0 || x > 7 || y < 0 || y > 7 || z < 0 || z > 7) {
      return;
    }
    float *v = buffer[x][y][z];
    v[0] = std::max(v[0], colour.r * amount);
    v[1] = std::max(v[1], colour.g * amount);
    v[2] = std::max(v[2], colour.b * amount);
  }

  void draw(Cube *cube) override {
    Phase phase = CRUISE;
    float inPhase = 0.0f;
    float flash = 0.0f;
    float swirl = 0.0f;
    int lastMode = -1;

    while (isRunning()) {
      const float dt = 0.033f * pace;
      const int currentMode = mode;
      const int count = static_cast<int>(std::lround((phase == HYPER ? 42.0f : 22.0f) * density));

      // Switching between modes starts over in the right state.
      if (currentMode != lastMode) {
        phase = currentMode == MODE_HYPER ? HYPER : CRUISE;
        inPhase = 0.0f;
        fill(phase == HYPER, count);
        lastMode = currentMode;
      }

      // --- the jump cycle ---
      inPhase += dt;
      float speed = CRUISE_SPEED;
      switch (phase) {
        case CRUISE:
          if (currentMode == MODE_JUMPS && inPhase > CRUISE_TIME) {
            phase = CHARGE;
            inPhase = 0.0f;
          }
          break;
        case CHARGE: {
          // The engines spool up: speed climbs ever faster, so the stars
          // stretch slowly at first and then all at once.
          const float t = inPhase / CHARGE_TIME;
          speed = CRUISE_SPEED + (HYPER_SPEED - CRUISE_SPEED) * t * t * t;
          if (inPhase > CHARGE_TIME) {
            phase = HYPER;
            inPhase = 0.0f;
            flash = 1.0f;
            fill(true, static_cast<int>(std::lround(42.0f * density)));
          }
          break;
        }
        case HYPER:
          speed = HYPER_SPEED;
          if (currentMode == MODE_JUMPS && inPhase > HYPER_TIME) {
            phase = DROP;
            inPhase = 0.0f;
            flash = 0.7f;
            fill(false, static_cast<int>(std::lround(22.0f * density)));
          }
          break;
        case DROP: {
          // Coming out, the streaks snap back to points almost at once.
          const float t = std::min(1.0f, inPhase / DROP_TIME);
          speed = CRUISE_SPEED + (HYPER_SPEED * 0.5f - CRUISE_SPEED) * (1.0f - t) * (1.0f - t);
          if (inPhase > DROP_TIME) {
            phase = CRUISE;
            inPhase = 0.0f;
          }
          break;
        }
      }
      const bool tunnel = phase == HYPER;
      // The tunnel slowly turns.
      if (tunnel) {
        swirl = std::fmod(swirl + 0.8f * dt, TWO_PI);
      }

      // --- move ---
      for (Star &s : stars) {
        s.progress += speed / 7.0f * dt;
        float x, y, z;
        place(s, s.progress, tunnel ? 0.6f : 0.0f, x, y, z);
        const bool gone = s.progress > 1.08f || x < -0.5f || x > 7.5f || y < -0.5f || y > 7.5f;
        if (gone) {
          s = tunnel ? streak(0.0f) : cruiser(0.0f);
          // Staggered, so the stars don't arrive in waves.
          s.progress = -uniform(0.0f, 0.3f);
        }
      }
      while (static_cast<int>(stars.size()) < count) {
        stars.push_back(tunnel ? streak(-uniform(0.0f, 0.3f)) : cruiser(-uniform(0.0f, 0.3f)));
      }
      if (static_cast<int>(stars.size()) > count) {
        stars.resize(count);
      }

      // --- draw ---
      std::fill(&buffer[0][0][0][0], &buffer[0][0][0][0] + 8 * 8 * 8 * 3, 0.0f);
      const float streakLength = std::min(MAX_STREAK, speed * STREAK_TIME);
      const bool flipped = direction == 1;
      for (const Star &s : stars) {
        if (s.progress < 0.0f) {
          continue;
        }
        // Head first, then the streak behind it back along its path, fading.
        for (float back = 0.0f; back <= streakLength; back += 0.35f) {
          const float p = s.progress - back / 7.0f;
          if (p < 0.0f) {
            break;
          }
          float x, y, z;
          place(s, p, tunnel ? 0.6f : 0.0f, x, y, z);
          // Stars fade in out of the distance rather than popping up.
          const float approach = std::min(1.0f, 0.35f + s.progress * 1.3f);
          const float tail = 1.0f - back / (streakLength + 0.6f);
          const int vz = static_cast<int>(std::lround(z));
          light(static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y)), flipped ? 7 - vz : vz, s.colour,
                approach * tail * (tunnel ? 0.6f + 0.4f * std::sin(swirl * 3.0f + s.angle * 5.0f) : 1.0f));
        }
      }

      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            float *v = buffer[x][y][z];
            // The jump flash washes over everything and fades.
            const float f = flash * flash;
            cube->set(x, y, z, toLevel(std::max(v[0], 0.85f * f)), toLevel(std::max(v[1], 0.9f * f)), toLevel(std::max(v[2], f)));
          }
        }
      }
      cube->update();
      flash = std::max(0.0f, flash - dt / FLASH_TIME);
      usleep(30000);
    }
  }
};
extern "C" Animation *create() {
  return new Hyperspace;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
