#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class ColorWheel : public Animation {

  void draw(Cube *c){

    int t = 20000;
    int xx, yy, zz, ww, rr=1, gg=1, bb=1, ranx, rany, swiper;

    while(isRunning()){
      swiper=random(3);
       ranx=random(16);
       rany=random(16);

      for(xx=0;xx<8;xx++){
        for(yy=0;yy<8;yy++){
          for(zz=0;zz<8;zz++){
            c->set(xx, yy, zz,  ranx, 0, rany);
          }
        }

        c->update();
        usleep(t);
      }

      ranx=random(16);
      rany=random(16);

      for(xx=7;xx>=0;xx--){
        for(yy=0;yy<8;yy++){
          for(zz=0;zz<8;zz++){
            c->set(xx,yy, zz, ranx, rany, 0);
          }
        }
        c->update();
        usleep(t);
      }
      ranx=random(16);
      rany=random(16);

      for(xx=0;xx<8;xx++){
        for(yy=0;yy<8;yy++){
          for(zz=0;zz<8;zz++){
            c->set(xx,yy, zz, 0, ranx, rany);
          }
        }
        c->update();
        usleep(t);
      }

      ranx=random(16);
      rany=random(16);
      for(xx=7;xx>=0;xx--){
        for(yy=0;yy<8;yy++){
          for(zz=0;zz<8;zz++){
            c->set(xx,yy, zz, rany, ranx, 0);
          }
        }
        c->update();
        usleep(t);
      }

    }//while
    c->clear();
  }

};

extern "C" Animation * create() {
    return new ColorWheel;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
