#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

// A campfire: crossed logs glow on the floor, flames lick up out of them and
// sway in a gentle breeze, and now and then a spark breaks loose and floats
// up until it burns out.
//
// The flames are a heat field, the classic way fire is done on pixel
// screens, just in three dimensions: the embers feed heat into the bottom
// layer, every layer above takes a blurred copy of the one below, and heat
// cools off on the way up -- faster away from the middle, so the fire
// narrows into a tip. Heat then maps to colour: red, orange, yellow, and
// nearly white at the hottest.
class Campfire : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  struct Spark {
    float x;
    float y;
    float z;
    float driftX;
    float driftZ;
    float rise;
    float age;
    float life;
  };

  static constexpr float CENTRE = 3.5f;
  // How far out from the middle the embers reach, in voxels.
  static constexpr float BED_RADIUS = 2.8f;

  int speed = 40000;
  float height = 1.0f;
  float sparks = 0.5f;
  float wind = 0.5f;
  float brightness = 1.0f;

  std::mt19937 random{std::random_device{}()};
  float heat[8][8][8] = {};
  // Each ember flickers on its own slow cycle rather than every frame, so
  // the base smoulders instead of fizzing.
  float ember[8][8] = {};
  std::vector<Spark> flying;

  float uniform(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random);
  }

  static float fromCentre(float x, float z) {
    return std::sqrt((x - CENTRE) * (x - CENTRE) + (z - CENTRE) * (z - CENTRE));
  }

  // Two logs crossed in an X on the floor.
  static bool isLog(int x, int z) {
    return (x == z || x == 7 - z) && fromCentre(x, z) < 3.6f;
  }

  static Rgb flame(float h) {
    return {
      std::clamp(h * 2.2f, 0.0f, 1.0f),
      std::clamp((h - 0.2f) * 1.4f, 0.0f, 1.0f) * 0.8f,
      std::clamp((h - 0.85f) * 2.0f, 0.0f, 1.0f) * 0.4f,
    };
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void set(Cube *cube, int x, int y, int z, Rgb colour) {
    cube->set(x, y, z, toLevel(colour.r), toLevel(colour.g), toLevel(colour.b));
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["height"].is_number()) {
      height = std::clamp(data["height"].get<float>(), 0.4f, 1.6f);
    }
    if (data["sparks"].is_number()) {
      sparks = std::clamp(data["sparks"].get<float>(), 0.0f, 1.0f);
    }
    if (data["wind"].is_number()) {
      wind = std::clamp(data["wind"].get<float>(), 0.0f, 1.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  void draw(Cube *cube) override {
    for (auto &row : ember) {
      for (float &e : row) {
        e = uniform(0.5f, 1.0f);
      }
    }
    float time = 0.0f;

    while (isRunning()) {
      const float dt = 0.04f;
      const float tall = height;
      const float breeze = wind;

      // --- embers: drift each towards a new random level now and then ---
      for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
          if (uniform(0.0f, 1.0f) < 0.15f) {
            ember[x][z] += (uniform(0.35f, 1.0f) - ember[x][z]) * 0.6f;
          }
          const float r = fromCentre(x, z);
          const float bed = std::clamp(1.0f - r / BED_RADIUS, 0.0f, 1.0f);
          // Allowed past 1 in the middle: that white-hot core is what lets the
          // flames climb before they cool.
          heat[x][0][z] = r < BED_RADIUS ? ember[x][z] * (0.4f + 1.1f * bed) : 0.0f;
        }
      }

      // --- flames: each layer is a cooled, blurred copy of the one below ---
      // The breeze leans the fire by drawing heat from slightly upwind, and
      // gusts come and go.
      const float gust = breeze * (0.5f + 0.5f * std::sin(time * 0.7f)) * (0.6f + 0.4f * std::sin(time * 2.3f));
      // Part of each voxel's heat comes from one step upwind; the stronger
      // the gust, the bigger that part, so the fire leans gradually.
      const float lean = std::min(gust * 1.2f, 0.9f);
      for (int y = 7; y >= 1; y--) {
        for (int x = 0; x < 8; x++) {
          for (int z = 0; z < 8; z++) {
            const auto below = [&](int bx, int bz) {
              bx = std::clamp(bx, 0, 7);
              bz = std::clamp(bz, 0, 7);
              return heat[bx][y - 1][bz];
            };
            const auto blur = [&](int sx) {
              return 0.52f * below(sx, z) +
                  0.12f * (below(sx - 1, z) + below(sx + 1, z) + below(sx, z - 1) + below(sx, z + 1));
            };
            const float blurred = (1.0f - lean) * blur(x) + lean * blur(x - 1);

            const float r = fromCentre(x, z);
            const float cooling = (0.03f + 0.02f * y + 0.035f * r * r / 4.0f) / tall * uniform(0.6f, 1.4f);
            heat[x][y][z] = std::max(0.0f, blurred - cooling);
          }
        }
      }

      // --- sparks ---
      if (uniform(0.0f, 1.0f) < sparks * 0.18f) {
        flying.push_back({uniform(2.5f, 4.5f), 2.0f, uniform(2.5f, 4.5f), uniform(-0.8f, 0.8f), uniform(-0.8f, 0.8f),
                          uniform(2.5f, 4.5f), 0.0f, uniform(0.9f, 1.8f)});
      }
      for (Spark &s : flying) {
        s.age += dt;
        s.y += s.rise * dt;
        // Sparks wander as they rise, and the breeze carries them too.
        s.x += (s.driftX + gust * 1.5f + uniform(-1.5f, 1.5f)) * dt;
        s.z += (s.driftZ + uniform(-1.5f, 1.5f)) * dt;
      }
      flying.erase(std::remove_if(flying.begin(), flying.end(), [](const Spark &s) {
        return s.age > s.life || s.y > 7.5f || s.x < -0.5f || s.x > 7.5f || s.z < -0.5f || s.z > 7.5f;
      }), flying.end());

      // --- draw ---
      cube->clear();

      for (int x = 0; x < 8; x++) {
        for (int z = 0; z < 8; z++) {
          // The floor: logs smouldering under the flames, the bed round them
          // a dim red glow.
          if (isLog(x, z)) {
            const float glow = 0.35f + 0.5f * ember[x][z];
            set(cube, x, 0, z, {0.9f * glow, 0.25f * glow * glow, 0.0f});
          } else if (heat[x][0][z] > 0.05f) {
            set(cube, x, 0, z, {0.5f * heat[x][0][z], 0.05f * heat[x][0][z], 0.0f});
          }
          for (int y = 1; y < 8; y++) {
            const float h = heat[x][y][z];
            if (h > 0.08f) {
              set(cube, x, y, z, flame(h));
            }
          }
        }
      }

      // Sparks cool from yellow through orange to a dying red.
      for (const Spark &s : flying) {
        const float left = 1.0f - s.age / s.life;
        const Rgb colour = {1.0f * std::sqrt(left), 0.75f * left * left, 0.0f};
        set(cube, std::clamp(static_cast<int>(std::lround(s.x)), 0, 7), std::clamp(static_cast<int>(std::lround(s.y)), 0, 7),
            std::clamp(static_cast<int>(std::lround(s.z)), 0, 7), colour);
      }

      cube->update();
      time = std::fmod(time + dt, 2.0f * static_cast<float>(M_PI) * 100.0f);
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Campfire;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
