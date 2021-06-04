#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>

class Test : public Animation {

    int time = 50000;

    void draw(Cube *c) {

      c->plane(AXIS_X,7,0,15,0);
      c->update();
      usleep(time);

      while (isRunning()) {

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
