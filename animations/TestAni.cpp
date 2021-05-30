#include "../Cube.cpp"
#include "../Animation.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>

class Test : public Animation {

    int time = 50000;

    void draw(Cube *c) {
      c->clear();
      c->update();
      sleep(1);

      c->plane(AXIS_X,7,0,15,0);
      c->update();
      // usleep(time);
      usleep(time);
      for (size_t t = 0; t < 5; t++) {

        for (size_t i = 0; i < 7; i++) {
          c->shift(AXIS_X,-1);
          c->update();
          usleep(time);
        }
        for (size_t i = 0; i < 7; i++) {
          c->shift(AXIS_X,1);
          c->update();
          usleep(time);
        }

      }

      c->clear();
      c->update();
      c->stop();
    }


    void onDataUpdate(std::vector<std::string> data){

    }
};

extern "C" Animation * create() {
    return new Test;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
