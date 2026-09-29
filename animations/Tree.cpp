#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

// A tree grows through a year -- about eight seconds of it at the default
// pace: the trunk rises and splits into branches, leaves unfold round the
// twig tips with a scattering of blossom, turn yellow and red, and drop to
// the floor one by one. Then it all fades and a new tree -- shaped
// differently every time -- sprouts in its place.
class Tree : public Animation {
  struct Vec {
    float x;
    float y;
    float z;
  };

  struct Rgb {
    float r;
    float g;
    float b;
  };

  struct Branch {
    Vec from;
    Vec dir;
    float length;
    float start;
    int depth;
  };

  struct Leaf {
    int x;
    int y;
    int z;
    float appear;
    float turn;
    float drop;
    Rgb green;
    Rgb autumn;
    bool blossom;
  };

  // The trunk is depth 0; twigs at this depth carry the leaves.
  static constexpr int TWIG_DEPTH = 2;
  // Voxels a branch grows per second.
  static constexpr float GROWTH_RATE = 1.4f;
  static constexpr float LEAF_RADIUS = 1.5f;
  static constexpr float GRAVITY = 9.0f;

  int speed = 30000;
  float pace = 3.0f;
  float brightness = 1.0f;

  std::mt19937 random{std::random_device{}()};
  std::vector<Branch> branches;
  std::vector<Leaf> leaves;

  // When each stage of the year begins, in seconds of tree time.
  float summer = 0.0f;
  float autumn = 0.0f;
  float bare = 0.0f;
  float end = 0.0f;

  float uniform(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random);
  }

  static Vec normalised(Vec v) {
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / length, v.y / length, v.z / length};
  }

  static bool inside(Vec p) {
    return p.x >= -0.4f && p.x <= 7.4f && p.y >= -0.4f && p.y <= 7.4f && p.z >= -0.4f && p.z <= 7.4f;
  }

  static Rgb mix(Rgb a, Rgb b, float amount) {
    amount = std::clamp(amount, 0.0f, 1.0f);
    return {a.r + (b.r - a.r) * amount, a.g + (b.g - a.g) * amount, a.b + (b.b - a.b) * amount};
  }

  int toLevel(float value) const {
    return std::clamp(static_cast<int>(std::lround(value * brightness * MAX_COLOR)), 0, MAX_COLOR);
  }

  void plot(Cube *cube, float x, float y, float z, Rgb colour, float level) {
    const int vx = std::clamp(static_cast<int>(std::lround(x)), 0, 7);
    const int vy = std::clamp(static_cast<int>(std::lround(y)), 0, 7);
    const int vz = std::clamp(static_cast<int>(std::lround(z)), 0, 7);
    cube->set(vx, vy, vz, toLevel(colour.r * level), toLevel(colour.g * level), toLevel(colour.b * level));
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["pace"].is_number()) {
      pace = std::clamp(data["pace"].get<float>(), 0.25f, 4.0f);
    }
    if (data["brightness"].is_number()) {
      brightness = std::clamp(data["brightness"].get<float>(), 0.15f, 1.0f);
    }
  }

  // Splits a branch into children that lean away from it at random, each
  // shorter than its parent and starting the moment the parent is done.
  void grow(const Branch &parent) {
    if (parent.depth >= TWIG_DEPTH) {
      return;
    }

    // Two directions at right angles to the parent to lean the children in.
    const Vec d = parent.dir;
    const Vec helper = std::fabs(d.y) < 0.9f ? Vec{0.0f, 1.0f, 0.0f} : Vec{1.0f, 0.0f, 0.0f};
    const Vec side = normalised({d.y * helper.z - d.z * helper.y, d.z * helper.x - d.x * helper.z, d.x * helper.y - d.y * helper.x});
    const Vec other = {d.y * side.z - d.z * side.y, d.z * side.x - d.x * side.z, d.x * side.y - d.y * side.x};

    const int children = parent.depth == 0 ? 3 : 2;
    const float offset = uniform(0.0f, 2.0f * static_cast<float>(M_PI));
    const Vec from = {parent.from.x + d.x * parent.length, parent.from.y + d.y * parent.length, parent.from.z + d.z * parent.length};

    for (int i = 0; i < children; i++) {
      const float around = offset + i * (2.0f * static_cast<float>(M_PI) / children) + uniform(-0.4f, 0.4f);
      const float lean = uniform(0.55f, 0.95f);
      const float out = std::sin(lean);
      // Trees reach for the light, so every child is nudged upwards.
      Vec dir = normalised({
        d.x * std::cos(lean) + (side.x * std::cos(around) + other.x * std::sin(around)) * out,
        d.y * std::cos(lean) + (side.y * std::cos(around) + other.y * std::sin(around)) * out + 0.35f,
        d.z * std::cos(lean) + (side.z * std::cos(around) + other.z * std::sin(around)) * out,
      });

      // Cut short rather than grow out through the side of the cube.
      float length = parent.length * uniform(0.6f, 0.8f);
      while (length > 0.8f && !inside({from.x + dir.x * length, from.y + dir.y * length, from.z + dir.z * length})) {
        length -= 0.25f;
      }

      const Branch child = {from, dir, length, parent.start + parent.length / GROWTH_RATE, parent.depth + 1};
      branches.push_back(child);
      grow(child);
    }
  }

  void plant() {
    branches.clear();
    leaves.clear();

    const Branch trunk = {{uniform(3.0f, 4.0f), 0.0f, uniform(3.0f, 4.0f)}, normalised({uniform(-0.1f, 0.1f), 1.0f, uniform(-0.1f, 0.1f)}), uniform(2.6f, 3.4f), 0.0f, 0};
    branches.push_back(trunk);
    grow(trunk);

    // Leaves fill the voxels round every twig tip, unfolding outwards from
    // it once the twig has finished growing.
    bool taken[8][8][8] = {};
    float grown = 0.0f;
    for (const Branch &b : branches) {
      if (b.depth != TWIG_DEPTH) {
        continue;
      }
      const float done = b.start + b.length / GROWTH_RATE;
      const Vec tip = {b.from.x + b.dir.x * b.length, b.from.y + b.dir.y * b.length, b.from.z + b.dir.z * b.length};

      for (int x = 0; x < 8; x++) {
        for (int y = 1; y < 8; y++) {
          for (int z = 0; z < 8; z++) {
            const float dx = x - tip.x, dy = y - tip.y, dz = z - tip.z;
            const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (distance > LEAF_RADIUS || taken[x][y][z]) {
              continue;
            }
            taken[x][y][z] = true;

            const float appear = done + distance * 0.8f + uniform(0.0f, 0.6f);
            grown = std::max(grown, appear);
            const Rgb green = {uniform(0.0f, 0.15f), uniform(0.55f, 0.95f), uniform(0.0f, 0.12f)};
            const Rgb autumnColours[] = {{1.0f, 0.75f, 0.0f}, {1.0f, 0.4f, 0.0f}, {0.9f, 0.1f, 0.0f}};
            const Rgb turned = autumnColours[std::uniform_int_distribution<int>(0, 2)(random)];
            leaves.push_back({x, y, z, appear, 0.0f, 0.0f, green, turned, uniform(0.0f, 1.0f) < 0.18f});
          }
        }
      }
    }

    summer = grown + 1.0f;
    autumn = summer + 4.0f;
    // Each leaf turns at its own moment, and falls a while after turning.
    float lastDrop = autumn;
    for (Leaf &leaf : leaves) {
      leaf.turn = autumn + uniform(0.0f, 4.0f);
      leaf.drop = leaf.turn + uniform(2.0f, 6.0f);
      lastDrop = std::max(lastDrop, leaf.drop);
    }
    bare = lastDrop + 2.0f;
    end = bare + 2.5f;
  }

  void draw(Cube *cube) override {
    plant();
    float time = 0.0f;

    while (isRunning()) {
      cube->clear();

      // The last stage fades everything out, fallen leaves included.
      const float fade = 1.0f - std::clamp((time - bare) / (end - bare - 0.5f), 0.0f, 1.0f);

      for (const Leaf &leaf : leaves) {
        if (time < leaf.appear) {
          continue;
        }
        const float unfold = std::clamp((time - leaf.appear) / 0.5f, 0.0f, 1.0f);

        Rgb colour = leaf.green;
        if (leaf.blossom && time > summer && time < autumn) {
          const Rgb pink = {1.0f, 0.3f, 0.5f};
          colour = mix(leaf.green, pink, std::min(time - summer, autumn - time));
        }
        // Green through yellow to the leaf's own autumn colour.
        const Rgb yellow = {0.9f, 0.8f, 0.0f};
        colour = mix(colour, yellow, (time - leaf.turn) / 1.0f);
        colour = mix(colour, leaf.autumn, (time - leaf.turn - 1.0f) / 1.5f);

        float y = leaf.y;
        if (time > leaf.drop) {
          const float falling = time - leaf.drop;
          y = std::max(0.0f, leaf.y - 0.5f * GRAVITY * 0.15f * falling * falling);
        }
        plot(cube, leaf.x, y, leaf.z, colour, unfold * fade);
      }

      // Wood on top, so it shows through the leaves as they thin out.
      for (const Branch &b : branches) {
        const float length = std::clamp((time - b.start) * GROWTH_RATE, 0.0f, b.length);
        if (length <= 0.0f) {
          continue;
        }
        const Rgb bark = b.depth == 0 ? Rgb{0.55f, 0.2f, 0.0f} : Rgb{0.45f, 0.2f, 0.02f};
        for (float s = 0.0f; s <= length; s += 0.2f) {
          plot(cube, b.from.x + b.dir.x * s, b.from.y + b.dir.y * s, b.from.z + b.dir.z * s, bark, fade);
        }
      }

      cube->update();
      time += 0.033f * pace;
      if (time > end) {
        plant();
        time = 0.0f;
      }
      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new Tree;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
