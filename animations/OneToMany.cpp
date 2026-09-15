
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class OneToMany : public Animation {

  int ledIndex[512];
  Cube::Color color;

  void draw(Cube *cube) {

    for (size_t i = 0; i < 512; i++) {
      ledIndex[i] = i;
    }

    int speed = 261121;//50000;
    color.set(15,15,15);

    while (isRunning()) {

      std::random_shuffle(std::begin(ledIndex),std::end(ledIndex));
      for (size_t i = 0; i < 512; i++) {
        color.random();
        int j = ledIndex[i];
        int layer = (j/64);
        int index = (j%64);

        cube->setIndex(layer, index, color.red,color.green,color.blue);
        cube->update();


        usleep(500);
        if(!isRunning()) {
          break;
        }
      }

      std::random_shuffle(std::begin(ledIndex),std::end(ledIndex));
      for (size_t i = 0; i < 512; i++) {
        int j = ledIndex[i];
        int layer = (j/64);
        int index = (j%64);

        cube->setIndex(layer, index, 0,0,0);
        cube->update();
        usleep(1000);
        if(!isRunning()) {
          break;
        }
      }
      usleep(1000000);
    }




    // std::random_shuffle(std::begin(leds),std::end(leds));

  }

  int exponentiation(int base, int exponent) {
    int result = 1;
    for (int i = 0; i < exponent; ++i)
        result = result * base;
    return result;
  }

};
extern "C" Animation * create() {
    return new OneToMany;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
