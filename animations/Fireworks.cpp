#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <cmath>

using json = nlohmann::json;

class Fireworks : public Animation {
  static const int MAX_PARTICLES = 24;

  int speed = 10000;
  int numParticles = 7;

  // Compact HSV-to-RGB, saturation fixed at 100%, brightness a native
  // 0..15 BAM level (same approach as Nebula.cpp's rainbow()) -- gives each
  // burst its own randomly-picked colour instead of always plain white.
  static Cube::Color rainbow(float hue, int brightness) {
    hue -= std::floor(hue);
    hue *= 6.0f;
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

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["particles"].is_number()) {
      int value = data["particles"].get<int>();
      // particles sizes a stack array each launch -- keep it within sane
      // bounds instead of trusting a network client not to send 0/negative
      // (undefined behaviour) or something huge (stack overflow).
      if (value > 0 && value <= MAX_PARTICLES) {
        numParticles = value;
      }
    }
  }

  void draw(Cube *c) override {
    while (isRunning()) {
      float originX = 2 + rand() % 4;
      float originY = 2 + rand() % 4;
      float originZ = 5 + rand() % 2;

      Cube::Color color = rainbow(static_cast<float>(rand() % 360) / 360.0f, MAX_COLOR);

      // Ascend: a single spark climbs from the floor to the burst height.
      for (int z = 0; z < originZ; z++) {
        c->clear();
        c->set(originX, originY, z, color);
        c->update();
        usleep(600 + 500 * z);
      }

      // Burst: numParticles sparks fly outward from the origin and fall
      // under gravity. The tan() ramp keeps the burst tight for the first
      // few frames and lets it drift apart and drop faster as it ages.
      float particles[MAX_PARTICLES][6];
      for (int i = 0; i < numParticles; i++) {
        particles[i][0] = originX;
        particles[i][1] = originY;
        particles[i][2] = originZ;
        particles[i][3] = 1 - (rand() % 200) / 100.0f;
        particles[i][4] = 1 - (rand() % 200) / 100.0f;
        particles[i][5] = 1 - (rand() % 200) / 100.0f;
      }

      for (int frame = 0; frame < 25; frame++) {
        float slowrate = 1 + tan((frame + 0.1f) / 20) * 10;
        float gravity = tan((frame + 0.1f) / 20) / 2;

        c->clear();
        for (int i = 0; i < numParticles; i++) {
          particles[i][0] += particles[i][3] / slowrate;
          particles[i][1] += particles[i][4] / slowrate;
          particles[i][2] += particles[i][5] / slowrate;
          particles[i][2] -= gravity;

          c->set(particles[i][0], particles[i][1], particles[i][2], color);
        }
        c->update();
        usleep(speed);
      }
    }
  }
};

extern "C" Animation *create() {
  return new Fireworks;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
