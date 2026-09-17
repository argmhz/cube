#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class Sine : public Animation {

  Cube::Color color;

  void draw(Cube *c) {

    int x, y;
    double Z;

    c->clear();

    double phase =0.50;
    int size = 8;
    int i = 0;

    while(isRunning()){
      c->clear();
      color = makeColorGradient(i);
      //basic sine function
      for(x = 0; x < size; x++){
        for(y = 0; y < size; y++){
          Z = sin(phase + sqrt(pow(dmap(x,0,size-1,-M_PI,M_PI),2) + pow(dmap(y,0,size-1,-M_PI,M_PI),2)));
          Z = round(dmap(Z,-1,1,0,size - 1));
          c->set(x,(int)Z,y,color.red,color.green,color.blue);
        }
      }

      c->update();
      usleep(7000);

      //handle the speen and increment phase
      phase += dmap(1, 1, 8, M_PI/64, M_PI/18);
      i++;

      if(i == 256){
        i = 0;
      }
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
