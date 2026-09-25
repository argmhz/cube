#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <unistd.h>

using json = nlohmann::json;

// A small glowing ball flying through the cube, leaving a trail that fades
// behind it. The ball lives at a fractional position and is drawn as a soft
// blob, so it glides between voxels instead of hopping from one to the next.
//
// `pattern` picks what steers it:
//   0 flow   -- follows a smooth, slowly changing 3D vector field
//   1 knot   -- traces a Lissajous knot that never quite repeats
//   2 orbit  -- circles the centre on a tilted, precessing ring
//   3 vector -- flies along the vector (dx, dy, dz), bouncing off the walls
class Comet : public Animation {
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

  static constexpr int PATTERN_FLOW = 0;
  static constexpr int PATTERN_KNOT = 1;
  static constexpr int PATTERN_ORBIT = 2;
  static constexpr int PATTERN_VECTOR = 3;

  // Everything is kept inside [MIN_POS, MAX_POS] so the blob's centre never
  // leaves the grid and the edge voxels still get lit when it passes them.
  // (Not LOW/HIGH: bcm2835.h defines those as macros.)
  static constexpr float MIN_POS = 0.0f;
  static constexpr float MAX_POS = 7.0f;
  static constexpr float CENTRE = 3.5f;

  int speed = 30000;
  int pattern = PATTERN_FLOW;
  float velocity = 0.25f;
  float trail = 0.85f;
  float size = 0.7f;
  float hue = 0.55f;
  float spread = 0.3f;
  float brightness = 1.0f;
  Vec direction = {1.0f, 0.6f, 0.35f};

  // Trail kept in floats so it decays smoothly; quantising to 0..15 every
  // frame would make it drop off in visible steps and die early.
  float glow[8][8][8][3] = {};

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

  static float length(Vec v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  }

  static Vec normalised(Vec v) {
    const float len = length(v);
    if (len < 1e-5f) {
      return {1.0f, 0.0f, 0.0f};
    }
    return {v.x / len, v.y / len, v.z / len};
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      speed = std::clamp(data["speed"].get<int>(), 8000, 150000);
    }
    if (data["pattern"].is_number()) {
      pattern = std::clamp(data["pattern"].get<int>(), 0, 3);
    }
    if (data["velocity"].is_number()) {
      velocity = std::clamp(data["velocity"].get<float>(), 0.02f, 1.0f);
    }
    if (data["trail"].is_number()) {
      trail = std::clamp(data["trail"].get<float>(), 0.0f, 0.97f);
    }
    if (data["size"].is_number()) {
      size = std::clamp(data["size"].get<float>(), 0.35f, 2.0f);
    }
    if (data["hue"].is_number()) {
      hue = wrap(data["hue"].get<float>());
    }
    if (data["spread"].is_number()) {
      spread = std::clamp(data["spread"].get<float>(), 0.0f, 2.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.1f, 1.0f);
    }
    // Any of the three can be sent on its own; the others keep their value.
    Vec next = direction;
    if (data["dx"].is_number()) {
      next.x = data["dx"].get<float>();
    }
    if (data["dy"].is_number()) {
      next.y = data["dy"].get<float>();
    }
    if (data["dz"].is_number()) {
      next.z = data["dz"].get<float>();
    }
    if (length(next) > 1e-5f) {
      direction = next;
      pendingDirection = true;
    }
  }

  // Set when a new (dx, dy, dz) arrives, so the vector pattern picks it up
  // on the next frame instead of carrying on with the old bounced heading.
  bool pendingDirection = false;

  // A smooth, divergence-light field built from crossed sines: each
  // component depends on the other two axes, so the ball swirls instead of
  // sliding straight along an axis. Time shifts the phases, so the currents
  // slowly rearrange and the path never settles into a loop.
  static Vec field(Vec p, float t) {
    const float s = 0.55f;
    return {
      std::sin(p.y * s + t * 0.37f) + std::cos(p.z * s * 1.3f - t * 0.21f),
      std::sin(p.z * s * 1.1f + t * 0.29f) + std::cos(p.x * s - t * 0.33f),
      std::sin(p.x * s * 0.9f + t * 0.41f) + std::cos(p.y * s * 1.2f + t * 0.17f),
    };
  }

  // Pushes back from the walls, growing sharply over the last voxel and a
  // half, so the flow pattern turns before it hits a face rather than
  // sticking to it.
  static float wallPush(float value) {
    const float margin = 1.5f;
    if (value < MIN_POS + margin) {
      const float d = (MIN_POS + margin - value) / margin;
      return d * d * 3.0f;
    }
    if (value > MAX_POS - margin) {
      const float d = (value - (MAX_POS - margin)) / margin;
      return -d * d * 3.0f;
    }
    return 0.0f;
  }

  // Deposits the ball at `p` into the trail buffer as a soft blob. Uses max
  // rather than add, so a slow ball sitting in one spot doesn't burn out
  // to white.
  void splat(Vec p, Rgb colour) {
    const float radius = size;
    const float reach = radius * 2.2f;
    const int x0 = std::max(0, static_cast<int>(std::floor(p.x - reach)));
    const int x1 = std::min(7, static_cast<int>(std::ceil(p.x + reach)));
    const int y0 = std::max(0, static_cast<int>(std::floor(p.y - reach)));
    const int y1 = std::min(7, static_cast<int>(std::ceil(p.y + reach)));
    const int z0 = std::max(0, static_cast<int>(std::floor(p.z - reach)));
    const int z1 = std::min(7, static_cast<int>(std::ceil(p.z + reach)));
    const float falloff = 1.0f / (radius * radius);

    for (int x = x0; x <= x1; x++) {
      for (int y = y0; y <= y1; y++) {
        for (int z = z0; z <= z1; z++) {
          const float dx = x - p.x, dy = y - p.y, dz = z - p.z;
          const float intensity = std::exp(-(dx * dx + dy * dy + dz * dz) * falloff);
          float *cell = glow[x][y][z];
          cell[0] = std::max(cell[0], colour.r * intensity);
          cell[1] = std::max(cell[1], colour.g * intensity);
          cell[2] = std::max(cell[2], colour.b * intensity);
        }
      }
    }
  }

  void draw(Cube *cube) override {
    Vec pos = {CENTRE, CENTRE, CENTRE};
    Vec heading = normalised(direction);
    // The flow pattern steers its current velocity toward the field instead
    // of snapping to it, which is what keeps its turns round.
    Vec flowVelocity = {0.0f, 0.0f, 0.0f};
    int lastPattern = -1;
    float t = 0.0f;
    // Path parameter for the curve patterns, advanced by velocity so the
    // velocity knob means the same thing for every pattern.
    float along = 0.0f;

    while (isRunning()) {
      const int activePattern = pattern;
      const float step = velocity;

      if (activePattern != lastPattern || pendingDirection) {
        heading = normalised(direction);
        pendingDirection = false;
        lastPattern = activePattern;
      }

      Vec next = pos;
      switch (activePattern) {
        case PATTERN_KNOT: {
          // Frequencies 3:4:5 with offset phases: a knot that fills the
          // volume. Dividing by the typical speed along the curve keeps the
          // ball's pace roughly equal to `velocity` voxels per frame.
          along += step / 9.0f;
          next = {
            CENTRE + 3.3f * std::sin(3.0f * along + 0.5f),
            CENTRE + 3.3f * std::sin(4.0f * along + 1.7f),
            CENTRE + 3.3f * std::sin(5.0f * along),
          };
          break;
        }
        case PATTERN_ORBIT: {
          along += step / 3.0f;
          // A circle in the plane spanned by u and v, where the plane
          // itself slowly tips over, so successive laps sweep new ground.
          const float tilt = t * 0.07f;
          const float precess = t * 0.05f;
          const Vec u = {std::cos(precess), std::sin(precess), 0.0f};
          const Vec v = {
            -std::sin(precess) * std::cos(tilt),
            std::cos(precess) * std::cos(tilt),
            std::sin(tilt),
          };
          const float r = 3.0f;
          const float c = std::cos(along), s = std::sin(along);
          next = {
            CENTRE + r * (c * u.x + s * v.x),
            CENTRE + r * (c * u.y + s * v.y),
            CENTRE + r * (c * u.z + s * v.z),
          };
          break;
        }
        case PATTERN_VECTOR: {
          next = {pos.x + heading.x * step, pos.y + heading.y * step, pos.z + heading.z * step};
          // Mirror the heading on whichever face was crossed, and fold the
          // overshoot back in, so the bounce stays exact at any velocity.
          float *coords[3] = {&next.x, &next.y, &next.z};
          float *components[3] = {&heading.x, &heading.y, &heading.z};
          for (int i = 0; i < 3; i++) {
            if (*coords[i] < MIN_POS) {
              *coords[i] = 2.0f * MIN_POS - *coords[i];
              *components[i] = std::fabs(*components[i]);
            } else if (*coords[i] > MAX_POS) {
              *coords[i] = 2.0f * MAX_POS - *coords[i];
              *components[i] = -std::fabs(*components[i]);
            }
          }
          break;
        }
        default: {
          const Vec f = field(pos, t);
          const Vec want = {f.x + wallPush(pos.x), f.y + wallPush(pos.y), f.z + wallPush(pos.z)};
          const float turn = 0.12f;
          flowVelocity = {
            flowVelocity.x + (want.x - flowVelocity.x) * turn,
            flowVelocity.y + (want.y - flowVelocity.y) * turn,
            flowVelocity.z + (want.z - flowVelocity.z) * turn,
          };
          // Constant speed along whatever direction the field gives, so the
          // ball doesn't stall in the field's quiet spots.
          const Vec dir = normalised(flowVelocity);
          next = {pos.x + dir.x * step, pos.y + dir.y * step, pos.z + dir.z * step};
          break;
        }
      }
      next.x = std::clamp(next.x, MIN_POS, MAX_POS);
      next.y = std::clamp(next.y, MIN_POS, MAX_POS);
      next.z = std::clamp(next.z, MIN_POS, MAX_POS);

      // Fade what's already there -- that's the whole trail.
      const float decay = trail;
      for (auto &plane : glow) {
        for (auto &line : plane) {
          for (auto &cell : line) {
            cell[0] *= decay;
            cell[1] *= decay;
            cell[2] *= decay;
          }
        }
      }

      const Rgb colour = rainbow(hue + t * spread * 0.05f);

      // Fill in the stretch between last frame's position and this one, so
      // a fast ball leaves a continuous streak rather than a dotted line.
      // Moving from a far point (e.g. a pattern switch) is skipped, so it
      // doesn't paint a line straight across the cube.
      const Vec travel = {next.x - pos.x, next.y - pos.y, next.z - pos.z};
      const float distance = length(travel);
      if (distance < 2.5f) {
        const int substeps = std::max(1, static_cast<int>(std::ceil(distance / 0.25f)));
        for (int i = 1; i <= substeps; i++) {
          const float k = static_cast<float>(i) / substeps;
          splat({pos.x + travel.x * k, pos.y + travel.y * k, pos.z + travel.z * k}, colour);
        }
      } else {
        splat(next, colour);
      }
      pos = next;

      cube->clear();
      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            const float *cell = glow[x][y][z];
            const int red = toLevel(cell[0]);
            const int green = toLevel(cell[1]);
            const int blue = toLevel(cell[2]);
            if (red || green || blue) {
              cube->set(x, y, z, red, green, blue);
            }
          }
        }
      }
      cube->update();

      t += 0.1f;
      usleep(speed);
    }
  }
};

extern "C" Animation *create() {
  return new Comet;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
