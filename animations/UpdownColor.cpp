#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <iterator>
#include <random>

using json = nlohmann::json;

// Columns light up one at a time in random order: a single voxel travels
// the height of the cube and stays at the far end, so the ceiling fills up
// column by column, then the same in reverse to fill the floor. The colour
// changes each time a face has been filled.
class UpdownColor : public Animation {
  static constexpr int COLOURS = 3;
  static constexpr int columns = 64;

  const int palette[COLOURS][3] = {
      {MAX_COLOR, 0, 0},
      {0, MAX_COLOR, 0},
      {0, 0, MAX_COLOR},
  };

  int leds[columns] = {};
  std::mt19937 rng{std::random_device{}()};

  int speed = 30000;
  int settle = 100000;

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["settle"].is_number()) {
      int value = data["settle"].get<int>();
      if (value >= 0) {
        settle = value;
      }
    }
  }

  void draw(Cube *cube) override {
    for (int i = 0; i < columns; i++) {
      leds[i] = i;
    }

    int colour = 0;

    cube->plane(AXIS_Y, 0, palette[COLOURS - 1][0], palette[COLOURS - 1][1], palette[COLOURS - 1][2]);
    cube->update();
    sleep(1);

    while (isRunning()) {
      // Upwards: each voxel climbs and stays at the top.
      std::shuffle(std::begin(leds), std::end(leds), rng);
      for (int i = 0; i < columns && isRunning(); i++) {
        const int x = leds[i] / 8;
        const int z = leds[i] % 8;

        for (int y = 0; y < 8; y++) {
          if (y > 0) {
            cube->clear(x, y - 1, z);
          }
          cube->set(x, y, z, palette[colour][0], palette[colour][1], palette[colour][2]);
          cube->update();
          usleep(speed);
        }
        usleep(settle);
      }
      colour = (colour + 1) % COLOURS;

      // Downwards, and rather faster -- the cube is already full, so this
      // reads as the light draining out of it.
      std::shuffle(std::begin(leds), std::end(leds), rng);
      for (int i = 0; i < columns && isRunning(); i++) {
        const int x = leds[i] / 8;
        const int z = leds[i] % 8;

        for (int y = 7; y >= 0; y--) {
          if (y < 7) {
            cube->set(x, y + 1, z, 0, 0, 0);
          }
          cube->set(x, y, z, palette[colour][0], palette[colour][1], palette[colour][2]);
          cube->update();
          usleep(speed / 10);
        }
        usleep(settle / 100);
      }
      colour = (colour + 1) % COLOURS;
    }
  }
};

extern "C" Animation *create() {
  return new UpdownColor;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
