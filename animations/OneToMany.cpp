#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <random>

using json = nlohmann::json;

// Fills the cube one voxel at a time in random order, each in its own
// colour, then empties it again the same way -- so it reads as the cube
// being painted in and wiped out rather than switched on and off.
class OneToMany : public Animation {
  struct Rgb {
    int red;
    int green;
    int blue;
  };

  int ledIndex[512] = {};
  std::mt19937 rng{std::random_device{}()};

  int speed = 500;
  int pause = 1000000;

  // A random hue at full brightness, rather than three independent random
  // channels: those can land on near-black mixes like (1, 0, 2), which
  // just look like dead voxels on the real cube.
  Rgb randomColor() {
    float hue = static_cast<float>(rand() % 360) / 60.0f;
    const int sector = static_cast<int>(hue);
    const int rising = static_cast<int>(std::lround((hue - sector) * MAX_COLOR));
    const int falling = MAX_COLOR - rising;

    switch (sector % 6) {
      case 0: return {MAX_COLOR, rising, 0};
      case 1: return {falling, MAX_COLOR, 0};
      case 2: return {0, MAX_COLOR, rising};
      case 3: return {0, falling, MAX_COLOR};
      case 4: return {rising, 0, MAX_COLOR};
      default: return {MAX_COLOR, 0, falling};
    }
  }

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["pause"].is_number()) {
      int value = data["pause"].get<int>();
      if (value >= 0) {
        pause = value;
      }
    }
  }

  void draw(Cube *cube) override {
    for (int i = 0; i < 512; i++) {
      ledIndex[i] = i;
    }

    while (isRunning()) {
      std::shuffle(std::begin(ledIndex), std::end(ledIndex), rng);
      for (int i = 0; i < 512 && isRunning(); i++) {
        const Rgb color = randomColor();
        cube->setIndex(ledIndex[i] / 64, ledIndex[i] % 64, color.red, color.green, color.blue);
        cube->update();
        usleep(speed);
      }

      std::shuffle(std::begin(ledIndex), std::end(ledIndex), rng);
      for (int i = 0; i < 512 && isRunning(); i++) {
        cube->setIndex(ledIndex[i] / 64, ledIndex[i] % 64, 0, 0, 0);
        cube->update();
        usleep(speed * 2);
      }

      usleep(pause);
    }
  }
};
extern "C" Animation *create() {
  return new OneToMany;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
