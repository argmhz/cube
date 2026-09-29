
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class Lamp : public Animation {

  Cube::Color color;

  void onDataUpdate(json data){

    if(data["color"].is_string()) {
      std::string c = data["color"].get<std::string>();

      color = colorConverter(c.erase(0, 1));
    }

  }

  void draw(Cube *cube) {

    color.red = 15;

    while (isRunning()) {
      cube->all(color.red,color.green,color.blue);
      cube->update();
      usleep(20000);
    }

  }

};
extern "C" Animation * create() {
    return new Lamp;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
