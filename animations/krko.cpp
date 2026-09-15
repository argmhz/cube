
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class krko : public Animation {

  void draw(Cube *cube) {

    while(isRunning()){

      int px = rand() % 8;
      int pz = rand() % 8;

      Cube::Color c;
      c.random();
      
      int height = random(4,7);

      for(int i = 0; i<height; i++){
  
        if(i>2){
          cube->clear(px, i-3, pz);
        }
        
        cube->set(px, i, pz, c.red, c.green, c.blue);
        
        cube->update();
        usleep(70000-i*2000);
      }

      for (size_t i = 0; i < 15; i++) {
        cube->sphere(px,height,pz,i,c.red,c.green,c.blue);
        cube->update();
        usleep(70000+(i*1000));
        cube->clear();
      }

      usleep(200000);

    }
  }

};
extern "C" Animation * create() {
    return new krko;
}

extern "C" void destroy(Animation * p) {
    delete p;
}

