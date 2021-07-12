
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class SlideColor : public Animation {

  Cube *cube;

  void draw(Cube *c) {
    cube = c;

    int speed = 50000;

    cube->all(getColor());
    cube->update();
    usleep(speed);

    while(isRunning()){
        clear(speed);
        doDraw(speed);
    }

  }



  Cube::Color getColor(){
    Cube::Color c;
    c.red = rand() % 16;
    c.green = rand() % 16;
    c.blue = rand() % 16;
    return c;
  }

  void clear(int speed) {

    int direction = (rand() % 2) + 1;
    int axis = (rand() % 3) + 1;

    if(direction == 1) {
      for (int i = 0; i < 8; i++) {
        cube->clearPlane(axis, i);
        cube->update();
        usleep(speed);
      }
    }
    else
    {
      for (int i = 7; i >= 0; i--) {
        cube->clearPlane(axis, i);
        cube->update();
        usleep(speed);
      }
    }


  }

  void doDraw(int speed ){
    int direction = (rand() % 2) + 1;
    int axis = (rand() % 3) + 1;

    Cube::Color c = getColor();

    if(direction == 1){
      for (int i = 0; i < 8; i++) {
        cube->plane(axis, i, c);
        cube->update();
        usleep(speed);
      }
    }
    else
    {
      for (int i = 7; i >= 0; i--) {
        cube->plane(axis, i, c);
        cube->update();
        usleep(speed);
      }
    }
  }

};
extern "C" Animation * create() {
    return new SlideColor;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
