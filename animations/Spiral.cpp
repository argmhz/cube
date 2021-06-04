#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Spiral : public Animation {

  void draw(Cube *c) {

    int z, i;
    double X = 0;
    double Y = 0;
    double phase = 0.5;
    int size = 8;
    c->clear();
    Cube::Color color;
    int a = 0;

    while(isRunning()) {
      c->clear();

      color = makeColorGradient(a);

      for(z = 0; z < size; z++){
          //the 3 itterations make the spiral thicker which looks better
          for(i = 0; i < 3; i++){
              Y = cos(phase + i*M_PI/8 +dmap(z,0,size-1,0,2*M_PI));
              X = sin(phase + i*M_PI/8 +dmap(z,0,size-1,0,2*M_PI));
              Y =dmap(Y,-1.1,0.9,0,size-1);
              X =dmap(X,-1.1,0.9,0,size-1);
              c->set((int)X,z,(int)Y,color.red,color.green,color.blue);
              c->update();
          }
      }

      phase += dmap(10, 1, 10, M_PI/22, M_PI/8);
      usleep(40000);
      a++;
    }



  }

};

extern "C" Animation * create() {
    return new Spiral;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
