
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class FadeColor : public Animation {

  Cube::Color color;
  Cube::Color newColor;
  std::vector<Cube::Color> colors;
  void draw(Cube *cube) {


    while(isRunning()){
      for (size_t i = 0; i < 256; i++) {

        if(!isRunning()){
          break;
        }

        newColor = makeColorGradient(i);
        colors = fadeColor(color.red,color.green,color.blue,newColor.red,newColor.green,newColor.blue);
        for (size_t j = 0; j < 16; j++) {
          cube->all(colors[j]);
          cube->update();
          usleep(100000);
        }
        color = newColor;
      }
    }
  }

};
extern "C" Animation * create() {
    return new FadeColor;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
