#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>
#include <mutex>
#include <vector>

using json = nlohmann::json;

class Stars : public Animation {
  Cube *cube;
  std::mutex cubeMutex;

  // Upper bound (microseconds) for the random per-step delay each star
  // picks for itself -- keeps every star's twinkle its own cadence while
  // still giving an overall pace knob.
  int speed = 40000;
  // Thread count is only sized once, at the top of draw() -- a change here
  // takes effect the next time this animation is selected, not live.
  int numStars = 5;

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      speed = std::clamp(data["speed"].get<int>(), 5000, 100000);
    }
    if (data["stars"].is_number()) {
      numStars = std::clamp(data["stars"].get<int>(), 1, 12);
    }
  }

  void blink() {
    while (isRunning()) {
      int stepDelay = rand() % speed;
      int x = rand() % 8;
      int y = rand() % 8;
      int z = rand() % 8;

      for (int i = 0; i <= 15; i++) {
        {
          std::lock_guard<std::mutex> lock(cubeMutex);
          cube->set(x, y, z, i, i, i);
          cube->update();
        }
        usleep(stepDelay);
      }
      for (int i = 15; i >= 0; i--) {
        {
          std::lock_guard<std::mutex> lock(cubeMutex);
          cube->set(x, y, z, i, i, i);
          cube->update();
        }
        usleep(stepDelay);
      }
    }
  }

  void draw(Cube *c) override {
    cube = c;

    std::vector<std::thread> blinks(numStars);
    for (auto &t : blinks) {
      t = std::thread([this] { this->blink(); });
    }
    for (auto &t : blinks) {
      t.join();
    }
  }
};
extern "C" Animation *create() {
  return new Stars;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
