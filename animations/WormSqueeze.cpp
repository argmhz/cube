#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

#include <algorithm>

using json = nlohmann::json;

// A colour-cycling worm's head wanders the Y=0 plane while cube->shift()
// pushes everything already drawn one step up the Y axis -- so the trail
// left behind squeezes up through the cube like a column of toothpaste.
class WormSqueeze : public Animation {
  int speed = 50000;
  int size = 2;

  void onDataUpdate(json data) override {
    if (data["speed"].is_number()) {
      int value = data["speed"].get<int>();
      if (value > 0) {
        speed = value;
      }
    }
    if (data["size"].is_number()) {
      size = std::clamp(data["size"].get<int>(), 1, 4);
    }
  }

  void draw(Cube *cube) override {
    const int axis = AXIS_Y;
    const int direction = 1;

    int cubeSize = 8 - (size - 1);
    int x = rand() % cubeSize;
    int y = rand() % cubeSize;

    int maxColorCount = 50;
    int colorCount = 0;

    while (isRunning()) {
      Cube::Color color = makeColorGradient(colorCount);
      colorCount = (colorCount < maxColorCount) ? colorCount + 1 : 0;

      int dx = (rand() % 3) - 1;
      int dy = (rand() % 3) - 1;
      if ((x + dx) > 0 && (x + dx) < cubeSize) {
        x += dx;
      }
      if ((y + dy) > 0 && (y + dy) < cubeSize) {
        y += dy;
      }

      cube->shift(axis, direction);

      for (int j = 0; j < size; j++) {
        for (int k = 0; k < size; k++) {
          cube->set(x + j, y + k, 0, color);
          cube->update();
        }
      }

      usleep(speed);
    }
  }
};
extern "C" Animation *create() {
  return new WormSqueeze;
}

extern "C" void destroy(Animation *p) {
  delete p;
}
