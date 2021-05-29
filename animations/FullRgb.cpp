#include "../Cube.cpp"
#include "../Animation.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>

class Test : public Animation {
    void draw(Cube *c) {

      int i = 0;

      for (size_t e = 0; e < 3; e++) {
        for ( i = 0; i < 16; i++) {
          drawfull(c,i,0,0);
          c->update();
          usleep(60000);
        }

        sleep(1);

        for ( i = 15; i >= 0; i--) {
          drawfull(c,i,0,0);
          c->update();
          usleep(60000);

        }

        for ( i = 0; i < 16; i++) {
          drawfull(c,0,i,0);
          c->update();
          usleep(60000);
        }

        sleep(1);

        for ( i = 15; i >= 0; i--) {
          drawfull(c,0,i,0);
          c->update();
          usleep(60000);
        }

        for ( i = 0; i < 16; i++) {
          drawfull(c,0,0,i);
          c->update();
          usleep(60000);
        }

        sleep(1);

        for ( i = 15; i >= 0; i--) {
          drawfull(c,0,0,i);
          c->update();
          usleep(60000);
        }
      }

      c->clear();
      c->update();
      c->stop();
    }

    void drawfull(Cube *c ,int r,int g,int b){
      for (size_t y = 0; y < 8; y++) {
        for (size_t x = 0; x < 8; x++) {
          for (size_t z = 0; z < 8; z++) {
            c->set(x,y,z,r,g,b);
          }
        }
      }
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
