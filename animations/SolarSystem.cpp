#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <unistd.h>

using json = nlohmann::json;

// A miniature solar system: a flickering sun in the middle, three planets on
// slightly tilted orbits -- the middle one with a moon -- and a comet on a
// long, stretched orbit whose tail always points away from the sun. The
// planets leave fading trails that sketch out their orbits, and a few stars
// twinkle in the corners.
//
// The motion follows Kepler: planets further out go slower (angular speed
// falls with distance to the power 1.5), and the comet whips round the sun
// and crawls at the far end of its orbit.
class SolarSystem : public Animation {
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

  struct Planet {
    float radius;
    // Orbit tilt, and which way the tilt leans.
    float tilt;
    float node;
    Rgb colour;
    float angle;
  };

  struct Star {
    int x;
    int y;
    int z;
    float phase;
    float rate;
  };

  static constexpr float CENTRE = 3.5f;
  static constexpr float TWO_PI = 2.0f * static_cast<float>(M_PI);
  // Angular speed, in radians per second, of a planet at radius 1.
  static constexpr float ORBIT_SPEED = 2.9f;

  static constexpr float COMET_AXIS = 3.2f;
  static constexpr float COMET_ECCENTRICITY = 0.65f;
  static constexpr float COMET_PERIOD = 20.0f;

  static constexpr float MOON_RADIUS = 1.1f;
  static constexpr float MOON_SPEED = 4.0f;

  static constexpr int STAR_COUNT = 7;

  int speed = 30000;
  float pace = 1.0f;
  float trails = 0.6f;
  float brightness = 1.0f;

  std::mt19937 random{std::random_device{}()};

  Planet planets[3] = {
    {1.9f, 0.10f, 0.0f, {0.6f, 0.45f, 0.3f}, 0.0f},
    {2.6f, 0.25f, 2.0f, {0.05f, 0.35f, 1.0f}, 2.1f},
    {3.35f, 0.18f, 4.2f, {1.0f, 0.12f, 0.0f}, 4.0f},
  };
  // Which planet has the moon.
  static constexpr int EARTH = 1;
  float moonAngle = 0.0f;
  float cometAngle = 2.5f;

  Star stars[STAR_COUNT] = {};
  float corona[8][8][8] = {};

  float trail[8][8][8][3] = {};
  float frame[8][8][8][3] = {};

  float uniform(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random);
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  static bool voxel(Vec p, int &x, int &y, int &z) {
    x = static_cast<int>(std::lround(p.x));
    y = static_cast<int>(std::lround(p.y));
    z = static_cast<int>(std::lround(p.z));
    return x >= 0 && x <= 7 && y >= 0 && y <= 7 && z >= 0 && z <= 7;
  }

  // Lights a voxel, keeping whichever is brighter where things overlap.
  static void light(float (&buffer)[8][8][8][3], Vec p, Rgb colour) {
    int x, y, z;
    if (!voxel(p, x, y, z)) {
      return;
    }
    float *v = buffer[x][y][z];
    v[0] = std::max(v[0], colour.r);
    v[1] = std::max(v[1], colour.g);
    v[2] = std::max(v[2], colour.b);
  }

  static Rgb scaled(Rgb c, float amount) {
    return {c.r * amount, c.g * amount, c.b * amount};
  }

  // A point on a circle round the sun, in the horizontal plane and then
  // tipped by `tilt` about an axis pointing towards `node`.
  static Vec onOrbit(float radius, float angle, float tilt, float node) {
    const float px = radius * std::cos(angle);
    const float pz = radius * std::sin(angle);
    // Rotate about the in-plane axis (cos node, 0, sin node).
    const float ax = std::cos(node);
    const float az = std::sin(node);
    const float along = px * ax + pz * az;
    const float across = -px * az + pz * ax;
    const float lifted = across * std::sin(tilt);
    const float kept = across * std::cos(tilt);
    return {along * ax - kept * az, lifted, along * az + kept * ax};
  }

  static Vec offset(Vec p) {
    return {p.x + CENTRE, p.y + CENTRE, p.z + CENTRE};
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["pace"].is_number()) {
      pace = std::clamp(data["pace"].get<float>(), 0.0f, 4.0f);
    }
    if (data["trails"].is_number()) {
      trails = std::clamp(data["trails"].get<float>(), 0.0f, 1.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  void draw(Cube *cube) override {
    // Stars go in the corners, well clear of every orbit.
    for (Star &s : stars) {
      do {
        s.x = std::uniform_int_distribution<int>(0, 7)(random);
        s.y = std::uniform_int_distribution<int>(0, 7)(random);
        s.z = std::uniform_int_distribution<int>(0, 7)(random);
      } while (std::sqrt((s.x - CENTRE) * (s.x - CENTRE) + (s.y - CENTRE) * (s.y - CENTRE) + (s.z - CENTRE) * (s.z - CENTRE)) < 5.0f);
      s.phase = uniform(0.0f, TWO_PI);
      s.rate = uniform(1.0f, 3.0f);
    }

    // The comet's orbit lies along the cube's long diagonal, the only
    // direction with room for its far end.
    const Vec major = {0.57735f, 0.57735f, 0.57735f};
    const Vec minor = {0.70711f, -0.70711f, 0.0f};
    const float cometMeanMotion = TWO_PI / COMET_PERIOD;
    const float e = COMET_ECCENTRICITY;
    const float semiLatus = COMET_AXIS * (1.0f - e * e);

    float time = 0.0f;

    while (isRunning()) {
      const float dt = 0.033f * pace;
      const float fadeFactor = trails <= 0.0f ? 0.0f : 0.80f + 0.17f * trails;

      // --- move ---
      for (Planet &p : planets) {
        p.angle = std::fmod(p.angle + ORBIT_SPEED / std::pow(p.radius, 1.5f) * dt, TWO_PI);
      }
      moonAngle = std::fmod(moonAngle + MOON_SPEED * dt, TWO_PI);
      // Kepler's second law: sweeping out equal areas in equal times means
      // the angle advances with the inverse square of the distance.
      const float onePlusCos = 1.0f + e * std::cos(cometAngle);
      cometAngle = std::fmod(cometAngle + cometMeanMotion * onePlusCos * onePlusCos / std::pow(1.0f - e * e, 1.5f) * dt, TWO_PI);
      time += dt;

      // --- the sun ---
      std::fill(&frame[0][0][0][0], &frame[0][0][0][0] + 8 * 8 * 8 * 3, 0.0f);
      const float pulse = 0.88f + 0.12f * std::sin(time * 1.7f);
      for (int x = 3; x <= 4; x++) {
        for (int y = 3; y <= 4; y++) {
          for (int z = 3; z <= 4; z++) {
            light(frame, {float(x), float(y), float(z)}, scaled({1.0f, 0.75f, 0.1f}, pulse * uniform(0.9f, 1.0f)));
          }
        }
      }
      // The corona: the voxels touching the sun's faces, each flaring up and
      // dying back on its own.
      for (int x = 2; x <= 5; x++) {
        for (int y = 2; y <= 5; y++) {
          for (int z = 2; z <= 5; z++) {
            const int outside = (x < 3 || x > 4) + (y < 3 || y > 4) + (z < 3 || z > 4);
            if (outside != 1) {
              continue;
            }
            float &c = corona[x][y][z];
            if (uniform(0.0f, 1.0f) < 0.08f) {
              c = uniform(0.3f, 1.0f);
            }
            c *= 0.96f;
            light(frame, {float(x), float(y), float(z)}, scaled({0.7f, 0.18f, 0.0f}, c));
          }
        }
      }

      // --- trails fade, then planets, moon and comet are drawn ---
      for (auto &plane : trail) {
        for (auto &row : plane) {
          for (auto &cell : row) {
            for (float &channel : cell) {
              channel *= fadeFactor;
            }
          }
        }
      }

      for (const Planet &p : planets) {
        const Vec where = offset(onOrbit(p.radius, p.angle, p.tilt, p.node));
        light(trail, where, scaled(p.colour, 0.35f));
        light(frame, where, p.colour);
      }

      const Planet &earth = planets[EARTH];
      const Vec earthAt = onOrbit(earth.radius, earth.angle, earth.tilt, earth.node);
      const Vec moonOffset = onOrbit(MOON_RADIUS, moonAngle, earth.tilt, earth.node);
      light(frame, offset({earthAt.x + moonOffset.x, earthAt.y + moonOffset.y, earthAt.z + moonOffset.z}), {0.45f, 0.45f, 0.45f});
      // Drawn again so the planet wins when the moon lands on its voxel.
      light(frame, offset(earthAt), earth.colour);

      const float distance = semiLatus / (1.0f + e * std::cos(cometAngle));
      const float c = std::cos(cometAngle) * distance;
      const float s = std::sin(cometAngle) * distance;
      const Vec cometAt = {major.x * c + minor.x * s, major.y * c + minor.y * s, major.z * c + minor.z * s};
      // The tail streams straight away from the sun, longer the closer in
      // the comet is.
      const float tailLength = std::clamp(4.0f / distance, 0.8f, 3.0f);
      for (float k = 0.7f; k <= tailLength; k += 0.35f) {
        const float fade = 1.0f - k / (tailLength + 0.5f);
        light(frame, offset({cometAt.x * (1.0f + k / distance), cometAt.y * (1.0f + k / distance), cometAt.z * (1.0f + k / distance)}),
              scaled({0.35f, 0.6f, 1.0f}, 0.7f * fade));
      }
      light(frame, offset(cometAt), {0.9f, 0.9f, 1.0f});

      for (const Star &star : stars) {
        const float twinkle = 0.5f + 0.5f * std::sin(time * star.rate + star.phase);
        light(frame, {float(star.x), float(star.y), float(star.z)}, scaled({0.3f, 0.3f, 0.35f}, 0.2f + 0.8f * twinkle * twinkle));
      }

      // --- out to the cube ---
      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            const float *f = frame[x][y][z];
            const float *t = trail[x][y][z];
            cube->set(x, y, z, toLevel(std::max(f[0], t[0])), toLevel(std::max(f[1], t[1])), toLevel(std::max(f[2], t[2])));
          }
        }
      }
      cube->update();

      if (time > 10000.0f) {
        time -= 10000.0f;
      }
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new SolarSystem;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
