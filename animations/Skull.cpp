#include "../lib/core/Cube.h"
#include "../lib/core/Vec3.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>

using json = nlohmann::json;

// A skull spinning in place, hand-authored voxel by voxel (see
// tools/skull -- not part of this repo, the shape was designed and checked
// offline before being transcribed here) rather than generated from a
// formula: at 8x8x8, smooth ellipsoids just blur into a rounded blob with
// no recognisable features. The eye sockets are real 3D recesses (missing
// voxels), not a texture -- from the front they read as dark hollows, with
// a small ember glowing at the back of each one.
//
// Rotation follows Prism.cpp's approach: the clean, original point cloud is
// rotated fresh every frame with Vec3's rotateAroundAxis(), never the
// cube's own voxel buffer, so the shape never degrades from repeatedly
// re-rounding already-rounded positions.
class Skull : public Animation {
  int speed = 45000;
  // Extra tumble on top of the main spin -- 0 is a pure turntable rotation
  // around Y, higher values let the skull tip and roll as it turns.
  float wobble = 0.0f;
  float eyeGlow = 0.6f;

  static constexpr int POINT_COUNT = 170;
  static const Vec3 BASE_POINTS[POINT_COUNT];
  // The deepest empty voxel in each eye socket -- lit separately from the
  // bone itself so its brightness/colour can be tuned independently.
  static const Vec3 EYE_POINTS[2];

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["wobble"].is_number()) {
      wobble = std::clamp(data["wobble"].get<float>(), 0.0f, 1.0f);
    }
    if (data["eyeGlow"].is_number()) {
      eyeGlow = std::clamp(data["eyeGlow"].get<float>(), 0.0f, 1.0f);
    }
  }

  void draw(Cube *cube) override {
    const double CENTER = 3.5;

    double spin = 0.0;
    double tiltX = 0.0;
    double tiltZ = 0.0;
    double emberPhase = 0.0;

    while (isRunning()) {
      cube->clear();

      const double spinRad = spin * M_PI / 180.0;
      const double tiltXRad = tiltX * M_PI / 180.0;
      const double tiltZRad = tiltZ * M_PI / 180.0;

      for (int i = 0; i < POINT_COUNT; i++) {
        Vec3 p = BASE_POINTS[i];
        if (wobble > 0.0f) {
          p = rotateAroundAxis(p, AXIS_X, tiltXRad);
          p = rotateAroundAxis(p, AXIS_Z, tiltZRad);
        }
        p = rotateAroundAxis(p, AXIS_Y, spinRad);

        cube->set(
            static_cast<int>(std::round(p.x + CENTER)),
            static_cast<int>(std::round(p.y + CENTER)),
            static_cast<int>(std::round(p.z + CENTER)),
            14, 13, 11);
      }

      // A slow ember-flicker rather than a flat glow -- reads as something
      // alive in the dark sockets instead of a static coloured dot.
      const double flicker = 0.7 + 0.3 * std::sin(emberPhase);
      const int emberLevel = static_cast<int>(std::round(eyeGlow * flicker * MAX_COLOR));

      for (const Vec3 &base : EYE_POINTS) {
        Vec3 p = base;
        if (wobble > 0.0f) {
          p = rotateAroundAxis(p, AXIS_X, tiltXRad);
          p = rotateAroundAxis(p, AXIS_Z, tiltZRad);
        }
        p = rotateAroundAxis(p, AXIS_Y, spinRad);

        cube->set(
            static_cast<int>(std::round(p.x + CENTER)),
            static_cast<int>(std::round(p.y + CENTER)),
            static_cast<int>(std::round(p.z + CENTER)),
            emberLevel, static_cast<int>(emberLevel * 0.25), 0);
      }

      cube->update();

      spin = std::fmod(spin + 1.1, 360.0);
      if (wobble > 0.0f) {
        tiltX = std::fmod(tiltX + 0.6 * wobble, 360.0);
        tiltZ = std::fmod(tiltZ + 0.4 * wobble, 360.0);
      }
      emberPhase += 0.08;

      usleep(speed);
    }
  }
};

const Vec3 Skull::BASE_POINTS[Skull::POINT_COUNT] = {
    {-3.5,-0.5,-1.5}, {-3.5,-0.5,-0.5}, {-3.5,0.5,-1.5}, {-3.5,0.5,-0.5}, {-3.5,0.5,0.5}, {-2.5,-1.5,-1.5}, {-2.5,-1.5,-0.5}, {-2.5,-0.5,-2.5},
    {-2.5,-0.5,-1.5}, {-2.5,-0.5,-0.5}, {-2.5,0.5,-2.5}, {-2.5,0.5,-1.5}, {-2.5,0.5,-0.5}, {-2.5,0.5,0.5}, {-2.5,0.5,1.5}, {-2.5,1.5,-2.5},
    {-2.5,1.5,-1.5}, {-2.5,1.5,-0.5}, {-2.5,1.5,0.5}, {-2.5,1.5,1.5}, {-2.5,2.5,-1.5}, {-2.5,2.5,-0.5}, {-2.5,2.5,0.5}, {-2.5,2.5,1.5},
    {-1.5,-2.5,-1.5}, {-1.5,-2.5,-0.5}, {-1.5,-1.5,-2.5}, {-1.5,-1.5,-1.5}, {-1.5,-1.5,-0.5}, {-1.5,-1.5,0.5}, {-1.5,-0.5,-3.5}, {-1.5,-0.5,-2.5},
    {-1.5,-0.5,-0.5}, {-1.5,-0.5,0.5}, {-1.5,-0.5,1.5}, {-1.5,0.5,-0.5}, {-1.5,0.5,0.5}, {-1.5,0.5,1.5}, {-1.5,0.5,2.5}, {-1.5,1.5,-3.5},
    {-1.5,1.5,-2.5}, {-1.5,1.5,-1.5}, {-1.5,1.5,-0.5}, {-1.5,1.5,0.5}, {-1.5,1.5,1.5}, {-1.5,1.5,2.5}, {-1.5,2.5,-2.5}, {-1.5,2.5,-1.5},
    {-1.5,2.5,-0.5}, {-1.5,2.5,0.5}, {-1.5,2.5,1.5}, {-1.5,2.5,2.5}, {-1.5,3.5,-0.5}, {-1.5,3.5,0.5}, {-0.5,-3.5,-1.5}, {-0.5,-3.5,-0.5},
    {-0.5,-2.5,-2.5}, {-0.5,-2.5,-1.5}, {-0.5,-2.5,-0.5}, {-0.5,-2.5,0.5}, {-0.5,-1.5,-2.5}, {-0.5,-1.5,-1.5}, {-0.5,-1.5,-0.5}, {-0.5,-1.5,0.5},
    {-0.5,-0.5,-1.5}, {-0.5,-0.5,-0.5}, {-0.5,-0.5,0.5}, {-0.5,-0.5,1.5}, {-0.5,-0.5,2.5}, {-0.5,0.5,-3.5}, {-0.5,0.5,-2.5}, {-0.5,0.5,-1.5},
    {-0.5,0.5,-0.5}, {-0.5,0.5,0.5}, {-0.5,0.5,1.5}, {-0.5,0.5,2.5}, {-0.5,1.5,-3.5}, {-0.5,1.5,-2.5}, {-0.5,1.5,-1.5}, {-0.5,1.5,-0.5},
    {-0.5,1.5,0.5}, {-0.5,1.5,1.5}, {-0.5,1.5,2.5}, {-0.5,2.5,-2.5}, {-0.5,2.5,-1.5}, {-0.5,2.5,-0.5}, {-0.5,2.5,0.5}, {-0.5,2.5,1.5},
    {-0.5,2.5,2.5}, {-0.5,3.5,-1.5}, {-0.5,3.5,-0.5}, {-0.5,3.5,0.5}, {-0.5,3.5,1.5}, {0.5,-3.5,-1.5}, {0.5,-3.5,-0.5}, {0.5,-2.5,-2.5},
    {0.5,-2.5,-1.5}, {0.5,-2.5,-0.5}, {0.5,-2.5,0.5}, {0.5,-1.5,-2.5}, {0.5,-1.5,-1.5}, {0.5,-1.5,-0.5}, {0.5,-1.5,0.5}, {0.5,-0.5,-1.5},
    {0.5,-0.5,-0.5}, {0.5,-0.5,0.5}, {0.5,-0.5,1.5}, {0.5,-0.5,2.5}, {0.5,0.5,-3.5}, {0.5,0.5,-2.5}, {0.5,0.5,-1.5}, {0.5,0.5,-0.5},
    {0.5,0.5,0.5}, {0.5,0.5,1.5}, {0.5,0.5,2.5}, {0.5,1.5,-3.5}, {0.5,1.5,-2.5}, {0.5,1.5,-1.5}, {0.5,1.5,-0.5}, {0.5,1.5,0.5},
    {0.5,1.5,1.5}, {0.5,1.5,2.5}, {0.5,2.5,-2.5}, {0.5,2.5,-1.5}, {0.5,2.5,-0.5}, {0.5,2.5,0.5}, {0.5,2.5,1.5}, {0.5,2.5,2.5},
    {0.5,3.5,-1.5}, {0.5,3.5,-0.5}, {0.5,3.5,0.5}, {0.5,3.5,1.5}, {1.5,-2.5,-1.5}, {1.5,-2.5,-0.5}, {1.5,-1.5,-2.5}, {1.5,-1.5,-1.5},
    {1.5,-1.5,-0.5}, {1.5,-1.5,0.5}, {1.5,-0.5,-2.5}, {1.5,-0.5,-0.5}, {1.5,-0.5,0.5}, {1.5,-0.5,1.5}, {1.5,0.5,-0.5}, {1.5,0.5,0.5},
    {1.5,0.5,1.5}, {1.5,1.5,-2.5}, {1.5,1.5,-1.5}, {1.5,1.5,-0.5}, {1.5,1.5,0.5}, {1.5,1.5,1.5}, {1.5,2.5,-2.5}, {1.5,2.5,-1.5},
    {1.5,2.5,-0.5}, {1.5,2.5,0.5}, {1.5,2.5,1.5}, {1.5,3.5,-0.5}, {1.5,3.5,0.5}, {2.5,-1.5,-1.5}, {2.5,-1.5,-0.5}, {2.5,-0.5,-1.5},
    {2.5,-0.5,-0.5}, {2.5,0.5,-1.5}, {2.5,0.5,-0.5}, {2.5,0.5,0.5}, {2.5,1.5,-1.5}, {2.5,1.5,-0.5}, {2.5,1.5,0.5}, {2.5,2.5,-1.5},
    {2.5,2.5,-0.5}, {2.5,2.5,0.5},
};

const Vec3 Skull::EYE_POINTS[2] = {
    {-1.5, 0.5, -1.5},
    {1.5, 0.5, -1.5},
};

extern "C" Animation *create() {
  return new Skull;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
