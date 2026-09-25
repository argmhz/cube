#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class Party : public Animation {

  void draw(Cube *c) {
    int px,py,pz,r,g,b;

    while(isRunning()){

      px = random(3,4);
      py = random(3,4);
      pz = random(3,4);

      r = rand() % 16;
      g = rand() % 16;
      b = rand() % 16;

      for (size_t i = 0; i < 11; i++) {
        c->sphere(px,py,pz,i,r,g,b);
        c->update();
        usleep(80000+(i*1000));
        c->clear();
      }
      usleep(100000);
    }
  }
};
extern "C" Animation * create() {
    return new Party;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
