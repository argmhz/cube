#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Earth : public Animation {

  void draw(Cube *c) {

    while(isRunning()){
      c->sphere(4,3,4,2,0,15,0);
      c->sphere(4,3,4,3,15,0,0);
      c->sphere(4,3,4,4,0,0,15);
      c->update();

      sleep(2);
    }

    // c->clear();
  }

  // void onDataUpdate(std::vector<std::string> data){
  //
  // }
};
extern "C" Animation * create() {
    return new Earth;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
