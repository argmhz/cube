#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Allan : public Animation {

  void draw(Cube *c) {

    while(isRunning()){
      for (size_t x = 0; x < 8; x++) {
        for (size_t y = 0; y < 8; y++) {
          for (size_t z = 0; z < 8; z++) {
            if(random(7) == 1){
              c->set(x,y,z,random(0,5),random(0,5),random(0,5));
            }
          }
        }
      }
        c->update();
        usleep(random(1000,2000000));
        c->clear();
    }

    c->clear();
  }  

};
extern "C" Animation * create() {
    return new Allan;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
