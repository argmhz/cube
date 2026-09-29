#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

#include <algorithm>
#include <cmath>
#include <unistd.h>

// A four-movement demo of the cube's geometry toolbox. Every movement uses
// intermediate 4-bit color values, so BAM produces real fades rather than
// merely switching LEDs on and off.
class BamSymphony : public Animation {
  int speed = 40000;

  static int wave(int frame, float rate, float phase = 0.0f) {
    return std::clamp(
        static_cast<int>(std::round(7.5f + 7.5f * std::sin(frame * rate + phase))),
        0, MAX_COLOR);
  }

  static Cube::Color scaled(Cube::Color color, int light) {
    color.red = color.red * light / MAX_COLOR;
    color.green = color.green * light / MAX_COLOR;
    color.blue = color.blue * light / MAX_COLOR;
    return color;
  }

  void breathingBoxes(Cube *cube, int frame) {
    cube->clear();

    for (int shell = 0; shell < 4; ++shell) {
      const int light = wave(frame, 0.10f, shell * 0.85f);
      Cube::Color color = scaled(makeColorGradient(frame + shell * 17), light);
      cube->boxOutline(shell, shell, shell, 7 - shell, 7 - shell, 7 - shell,
                       color.red, color.green, color.blue);
    }

    const int core = wave(frame, 0.14f, PI / 2.0f);
    cube->sphere(4, 4, 4, 1, core, core, core);
    cube->rotate(AXIS_Z, frame * 3);
  }

  void spectralScanner(Cube *cube, int frame) {
    cube->clear();

    // Eight adjacent BAM levels make the brightness resolution plainly
    // visible while the scanning planes travel through the volume.
    for (int x = 0; x < 8; ++x) {
      const int light = 1 + x * 2;
      cube->line(x, 0, 0, x, 7, 7, 0, light, light);
    }

    const int scanner = (frame / 3) % 8;
    const int echo = (scanner + 6) % 8;
    cube->plane(AXIS_Z, scanner, 15, 2, 0);
    cube->plane(AXIS_Z, echo, 3, 0, 5);

    const int pulse = wave(frame, 0.16f);
    cube->hollowBox(1, 1, 1, 6, 6, 6, pulse / 3, pulse, pulse / 2);
  }

  void colorComet(Cube *cube, int frame) {
    if (frame == 0) {
      cube->clear();
    }

    cube->shift(AXIS_Z, -1);
    const int head = frame % 8;
    const int opposite = 7 - head;
    Cube::Color color = makeColorGradient(frame * 3);

    cube->line(0, head, 7, 7, opposite, 7, color.red, color.green, color.blue);
    cube->set(head, opposite, 7, 15, 15, 15);

    // Bend one slice independently, exercising shiftPlane without breaking
    // the long trails travelling down the Z axis.
    if (frame % 6 == 0) {
      cube->shiftPlane(AXIS_X, head, frame % 12 == 0 ? 1 : -1);
    }
  }

  void radialFinale(Cube *cube, int frame) {
    cube->clear();

    const int outer = wave(frame, 0.09f);
    const int inner = wave(frame, 0.13f, PI);
    cube->sphere(4, 4, 4, 3, outer, outer / 4, 15 - outer);
    cube->sphere(4, 4, 4, 1, inner, inner, 15);

    Cube::Color color = makeColorGradient(frame * 2);
    cube->line(0, 0, 0, 7, 7, 7, color.red, color.green, color.blue);
    cube->line(7, 0, 0, 0, 7, 7, color.blue, color.red, color.green);
    cube->line(0, 7, 0, 7, 0, 7, color.green, color.blue, color.red);
    cube->line(7, 7, 0, 0, 0, 7, 15, 15, 15);
    cube->rotate(AXIS_Y, frame * 2);
  }

public:
  void onDataUpdate(json data) override {
    if (data["speed"].is_number_integer()) {
      speed = std::clamp(data["speed"].get<int>(), 10000, 250000);
    }
  }

  void draw(Cube *cube) override {
    int frame = 0;

    while (isRunning()) {
      const int movement = (frame / 96) % 4;
      const int localFrame = frame % 96;

      if (movement == 0) {
        breathingBoxes(cube, localFrame);
      } else if (movement == 1) {
        spectralScanner(cube, localFrame);
      } else if (movement == 2) {
        colorComet(cube, localFrame);
      } else {
        radialFinale(cube, localFrame);
      }

      cube->update();
      usleep(speed);
      frame = (frame + 1) % (96 * 4);
    }
  }
};

extern "C" Animation *create() {
  return new BamSymphony;
}

extern "C" void destroy(Animation *animation) {
  delete animation;
}
