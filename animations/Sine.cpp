#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Sine : public Animation {

  void draw(Cube *c) {

    int x, y;
    double Z;
    c->clear();

    double phase = 0;
    int size = 8;

    while(isRunning()){
        c->clear();
        //basic sine function
        for(x = 0; x < size; x++){
            for(y = 0; y < size; y++){
                Z = sin(phase + sqrt(pow(dmap(x,0,size-1,-M_PI,M_PI),2) + pow(dmap(y,0,size-1,-M_PI,M_PI),2)));
                Z = dmap(Z,-1,0.95,0,size - 1);
                c->set(x,(int)Z,y,0,0,15);
            }
        }

        c->update();
        usleep(7000);

        //handle the speen and increment phase
        phase += dmap(1, 1, 10, M_PI/64, M_PI/18);
      }
  }

  // void onDataUpdate(std::vector<std::string> data){
  //
  // }
};
extern "C" Animation * create() {
    return new Sine;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
