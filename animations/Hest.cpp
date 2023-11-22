
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Hest : public Animation {

  int x = 4;
  int y = 4;
  int z = 1;

  void draw(Cube *cube) {

      //cube->line(2,2,2,5,5,5,0,0,15);
      cube->set(x,y,z,0,0,15);
      rotate2D(90);
      cube->set(x,y,z,15,0,0);
      rotate2D(90);
      cube->set(x,y,z,0,15,0);
      rotate2D(90);
      cube->set(x,y,z,15,15,15);
      rotate2D(90);
      cube->set(x,y,z,15,0,15);
      rotate2D(90);
      cube->set(x,y,z,15,0,15);
      cube->update();




    // while(isRunning()) {
    //   cube->rotateZ(45);
    //   cube->update();
    //  s usleep(1000000);
    // }
  }

  void rotate2D(int degrees)
  {
    int newX = (x * cos(degrees)) - (y * sin(degrees));
    int newY = (x * sin(degrees)) + (y * cos(degrees));
    std::cout << "X: " << newX << " Y: " << newY << std::endl;

    x = newX;
    z = newY;

    // int newX =
  }


};
extern "C" Animation * create() {
    return new Hest;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
