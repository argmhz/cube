#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

// A classic side-scrolling platformer, seen from the front: a little runner
// in a red cap and blue overalls dashes along an endless course, jumping up
// to head-butt question blocks (a coin pops out), leaping over pipes and
// stomping the walking mushrooms. The course plays out in the middle layers;
// clouds and hills drift past at half speed on the back wall, so the cube's
// depth reads as parallax.
class Platformer : public Animation {
  struct Rgb {
    float r;
    float g;
    float b;
  };

  enum Kind { QUESTION, BRICK, PIPE, ENEMY };

  struct Thing {
    Kind kind;
    float x;
    int width;
    int height;
    // When it was hit or stomped, or below zero while untouched.
    float hit;
    // Whether the runner has already taken off to deal with it.
    bool planned;
  };

  // A jump is a parabola over ground distance rather than time, so the arc
  // stays the same shape whatever the running speed.
  struct Jump {
    bool active;
    float from;
    float base;
    float height;
    float distance;
  };

  // Screen column the runner's left edge stays at; the world scrolls past.
  static constexpr float RUNNER_COLUMN = 1.0f;
  static constexpr float RUN_SPEED = 6.0f;
  static constexpr float ENEMY_SPEED = 1.2f;
  // Question blocks hang at this row, just above where a jump can reach the
  // runner's head.
  static constexpr int BLOCK_ROW = 6;
  // Layers along z that the course is drawn in, and the back wall.
  static constexpr int PLAY_NEAR = 5;
  static constexpr int PLAY_FAR = 3;
  static constexpr int BACKDROP = 0;

  static constexpr Rgb CAP = {1.0f, 0.0f, 0.0f};
  static constexpr Rgb SKIN = {1.0f, 0.55f, 0.3f};
  static constexpr Rgb OVERALLS = {0.0f, 0.15f, 1.0f};
  static constexpr Rgb QUESTION_LIT = {1.0f, 0.65f, 0.0f};
  static constexpr Rgb USED = {0.4f, 0.18f, 0.05f};
  static constexpr Rgb BRICK_COLOUR = {0.75f, 0.25f, 0.0f};
  static constexpr Rgb COIN = {1.0f, 0.9f, 0.0f};
  static constexpr Rgb PIPE_BODY = {0.0f, 0.7f, 0.0f};
  static constexpr Rgb PIPE_RIM = {0.35f, 1.0f, 0.2f};
  static constexpr Rgb MUSHROOM_CAP = {0.6f, 0.2f, 0.02f};
  static constexpr Rgb MUSHROOM_FEET = {0.9f, 0.6f, 0.3f};
  static constexpr Rgb GROUND_LIGHT = {0.7f, 0.3f, 0.02f};
  static constexpr Rgb GROUND_DARK = {0.4f, 0.15f, 0.0f};
  static constexpr Rgb CLOUD = {0.45f, 0.45f, 0.55f};
  static constexpr Rgb HILL = {0.0f, 0.3f, 0.02f};

  int speed = 30000;
  float pace = 1.0f;
  float brightness = 1.0f;

  std::mt19937 random{std::random_device{}()};
  std::vector<Thing> things;
  float nextThing = 10.0f;

  float runner = 0.0f;
  float feet = 1.0f;
  Jump jump = {};

  float uniform(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random);
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
    if (data["pace"].is_number()) {
      pace = std::clamp(data["pace"].get<float>(), 0.25f, 3.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  // Lays out more course ahead of the camera, with room between features
  // for the runner to land from one jump before the next.
  void extendCourse(float camera) {
    while (nextThing < camera + 20.0f) {
      const float roll = uniform(0.0f, 1.0f);
      if (roll < 0.35f) {
        const bool bricks = uniform(0.0f, 1.0f) < 0.5f;
        if (bricks) {
          things.push_back({BRICK, nextThing - 1.0f, 1, 1, -1.0f, true});
          things.push_back({BRICK, nextThing + 2.0f, 1, 1, -1.0f, true});
        }
        things.push_back({QUESTION, nextThing, 2, 1, -1.0f, false});
      } else if (roll < 0.65f) {
        things.push_back({PIPE, nextThing, 2, uniform(0.0f, 1.0f) < 0.5f ? 2 : 3, -1.0f, false});
      } else {
        things.push_back({ENEMY, nextThing, 2, 2, -1.0f, false});
      }
      nextThing += uniform(11.0f, 16.0f);
    }
  }

  void leap(float base, float height, float distance) {
    jump = {true, runner, base, height, distance};
  }

  // Looks at the next feature coming up and takes off at the moment that
  // lands the jump where it needs to be.
  void plan() {
    for (Thing &t : things) {
      if (t.planned || t.x + t.width < runner) {
        continue;
      }
      const float gap = t.x - runner;
      switch (t.kind) {
        case QUESTION:
          // Peak -- head right under the block -- with the runner centred on it.
          if (gap <= 3.0f) {
            leap(1.0f, BLOCK_ROW - 4.0f, 6.0f);
            t.planned = true;
          }
          break;
        case PIPE: {
          // The arc must clear the rim for as long as runner and pipe
          // overlap, a quarter of the jump either side of the peak; at that
          // point a parabola is at three quarters of its height.
          if (gap <= 4.0f) {
            leap(1.0f, t.height / 0.75f + 0.1f, 8.0f);
            t.planned = true;
          }
          break;
        }
        case ENEMY: {
          // Coming down onto its head (two rows up) happens 72% of the way
          // through the jump; by then the mushroom has walked towards us.
          const float meet = 0.72f * 6.0f * (1.0f + ENEMY_SPEED / RUN_SPEED);
          if (t.hit < 0.0f && gap <= meet) {
            leap(1.0f, 2.5f, 6.0f);
            t.planned = true;
          }
          break;
        }
        default:
          break;
      }
      return;
    }
  }

  void box(Cube *cube, float camera, float x, int width, int y, int height, Rgb colour, int near, int far) {
    const int left = static_cast<int>(std::lround(x - camera));
    for (int column = left; column < left + width; column++) {
      if (column < 0 || column > 7) {
        continue;
      }
      for (int row = y; row < y + height; row++) {
        if (row < 0 || row > 7) {
          continue;
        }
        for (int z = far; z <= near; z++) {
          cube->set(column, row, z, toLevel(colour.r), toLevel(colour.g), toLevel(colour.b));
        }
      }
    }
  }

  void draw(Cube *cube) override {
    float time = 0.0f;

    while (isRunning()) {
      const float dt = 0.033f * pace;
      const float camera = runner - RUNNER_COLUMN;
      extendCourse(camera);

      // --- move the world ---
      for (Thing &t : things) {
        if (t.kind == ENEMY && t.hit < 0.0f && t.x < camera + 9.0f) {
          t.x -= ENEMY_SPEED * dt;
        }
      }
      things.erase(std::remove_if(things.begin(), things.end(), [&](const Thing &t) {
        return t.x + t.width < camera - 1.0f || (t.kind == ENEMY && t.hit >= 0.0f && time - t.hit > 0.4f);
      }), things.end());

      const float before = jump.active ? (runner - jump.from) / jump.distance : 0.0f;
      runner += RUN_SPEED * dt;

      if (jump.active) {
        const float s = (runner - jump.from) / jump.distance;
        const float previous = feet;
        feet = jump.base + jump.height * 4.0f * s * (1.0f - s);
        const bool falling = feet < previous;

        for (Thing &t : things) {
          const bool overlapping = std::fabs(t.x - runner) < t.width;
          // Head-butt at the top of the arc.
          if (t.kind == QUESTION && t.hit < 0.0f && before < 0.5f && s >= 0.5f && overlapping) {
            t.hit = time;
          }
          // Bricks next to it just bump.
          if (t.kind == BRICK && before < 0.5f && s >= 0.5f && std::fabs(t.x - runner) < 1.5f) {
            t.hit = time;
          }
          if (t.kind == ENEMY && t.hit < 0.0f && falling && feet <= 3.2f && std::fabs(t.x - runner) < 1.6f) {
            t.hit = time;
            leap(feet, 1.5f, 4.0f);
          }
        }

        if (jump.active && feet <= 1.0f && (runner - jump.from) / jump.distance > 0.5f) {
          feet = 1.0f;
          jump.active = false;
        }
      } else {
        // Anything that still reaches us on the ground gets squashed anyway.
        for (Thing &t : things) {
          if (t.kind == ENEMY && t.hit < 0.0f && std::fabs(t.x - runner) < 1.5f) {
            t.hit = time;
          }
        }
        plan();
      }

      // --- draw ---
      cube->clear();

      // Backdrop at half speed: a cloud every 11 columns, a hill every 13.
      const int back = static_cast<int>(std::floor(camera * 0.5f));
      for (int column = 0; column < 8; column++) {
        const int world = back + column;
        const int cloud = ((world % 11) + 11) % 11;
        if (cloud < 3) {
          box(cube, 0.0f, column, 1, 6, 1, CLOUD, BACKDROP, BACKDROP);
        }
        if (cloud == 1) {
          box(cube, 0.0f, column, 1, 7, 1, CLOUD, BACKDROP, BACKDROP);
        }
        const int hill = ((world % 13) + 13) % 13;
        const int hillHeights[] = {1, 2, 3, 2, 1};
        if (hill < 5) {
          box(cube, 0.0f, column, 1, 1, hillHeights[hill], HILL, BACKDROP, BACKDROP);
        }
      }

      // Ground across the whole depth, in two-column stripes so it visibly scrolls.
      for (int column = 0; column < 8; column++) {
        const int world = static_cast<int>(std::floor(camera + column + 0.5f));
        const Rgb ground = ((world / 2) % 2 + 2) % 2 == 0 ? GROUND_LIGHT : GROUND_DARK;
        box(cube, 0.0f, column, 1, 0, 1, ground, 7, 0);
      }

      for (const Thing &t : things) {
        const float since = time - t.hit;
        switch (t.kind) {
          case QUESTION: {
            const bool bumping = t.hit >= 0.0f && since < 0.15f;
            // The "?" flickers, like the original's shimmering block.
            const float shimmer = 0.75f + 0.25f * std::sin(time * 6.0f);
            const Rgb colour = t.hit < 0.0f ? Rgb{QUESTION_LIT.r * shimmer, QUESTION_LIT.g * shimmer, 0.0f} : USED;
            box(cube, camera, t.x, 2, bumping ? BLOCK_ROW + 1 : BLOCK_ROW, 1, colour, PLAY_NEAR, PLAY_FAR);
            if (t.hit >= 0.0f && since > 0.1f && since < 0.6f) {
              box(cube, camera, t.x, 2, 7, 1, COIN, PLAY_NEAR - 1, PLAY_FAR + 1);
            }
            break;
          }
          case BRICK: {
            const bool bumping = t.hit >= 0.0f && since < 0.15f;
            box(cube, camera, t.x, 1, bumping ? BLOCK_ROW + 1 : BLOCK_ROW, 1, BRICK_COLOUR, PLAY_NEAR, PLAY_FAR);
            break;
          }
          case PIPE:
            box(cube, camera, t.x, 2, 1, t.height - 1, PIPE_BODY, PLAY_NEAR, PLAY_FAR);
            box(cube, camera, t.x, 2, t.height, 1, PIPE_RIM, PLAY_NEAR, PLAY_FAR);
            break;
          case ENEMY:
            if (t.hit >= 0.0f) {
              // Squashed flat for a moment before it vanishes.
              box(cube, camera, t.x, 2, 1, 1, MUSHROOM_CAP, PLAY_NEAR, PLAY_FAR);
            } else {
              box(cube, camera, t.x, 2, 2, 1, MUSHROOM_CAP, PLAY_NEAR, PLAY_FAR);
              // Waddling: the feet take turns.
              const int step = static_cast<int>(time * 5.0f) % 2;
              box(cube, camera, t.x + step, 1, 1, 1, MUSHROOM_FEET, PLAY_NEAR, PLAY_FAR);
            }
            break;
        }
      }

      // The runner, three rows tall: cap, face, overalls. On the ground the
      // legs alternate as he runs; in the air both stay down.
      const int y = static_cast<int>(std::lround(feet));
      if (jump.active) {
        box(cube, camera, runner, 2, y, 1, OVERALLS, PLAY_NEAR, PLAY_FAR);
      } else {
        const int stride = static_cast<int>(runner * 1.5f) % 2;
        box(cube, camera, runner + stride, 1, y, 1, OVERALLS, PLAY_NEAR, PLAY_FAR);
        box(cube, camera, runner + 1 - stride, 1, y, 1, Rgb{OVERALLS.r * 0.5f, OVERALLS.g * 0.5f, OVERALLS.b * 0.5f}, PLAY_NEAR, PLAY_FAR);
      }
      box(cube, camera, runner, 2, y + 1, 1, SKIN, PLAY_NEAR, PLAY_FAR);
      box(cube, camera, runner, 2, y + 2, 1, CAP, PLAY_NEAR, PLAY_FAR);

      cube->update();
      time += dt;

      // Shift everything back now and then so floats stay precise. 4004 is
      // chosen so the backdrop (half of it: 2002 = 14 x 11 x 13) and the
      // ground stripes (every 4) land exactly where they were.
      if (runner > 5000.0f) {
        for (Thing &t : things) {
          t.x -= 4004.0f;
        }
        runner -= 4004.0f;
        nextThing -= 4004.0f;
        jump.from -= 4004.0f;
      }
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Platformer;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
