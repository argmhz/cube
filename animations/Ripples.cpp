
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"


class Ripples : public Animation {



  void draw(Cube *cube) {
    // 16 values for square root of a^2+b^2.  index a*4+b = 10*sqrt
    // This gives the distance to 3.5,3.5 from the point
    unsigned char sqrt_LUT[]={49,43,38,35,43,35,29,26,38,29,21,16,35,25,16,7};
    //LUT_START // Macro from new tottymath.  Commented and replaced with full code
    unsigned char LUT[65];
    init_LUT(LUT);
    int i;
    unsigned char x,y,height,distance;
    while (isRunning())
    {
      i+=4;
      cube->clear();

      for (x=0;x<4;x++)
        for(y=0;y<4;y++)
        {
          // x+y*4 gives no. from 0-15 for sqrt_LUT
          distance=sqrt_LUT[x+y*4];// distance is 0-50 roughly
          // height is sin of distance + iteration*4
          //height=4+totty_sin(LUT,distance+i)/52;
          height=(196+totty_sin(LUT,distance+i))/49;
          // Use 4-way mirroring to save on calculations
          cube->set(x,y,height, 0,0,15);
          cube->set(7-x,y,height, 0,0,15);
          cube->set(x,7-y,height, 0,0,15);
          cube->set(7-x,7-y,height, 0,0,15);

        }
        cube->update();
      usleep(10000);
    }

  }

};
extern "C" Animation * create() {
    return new Ripples;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
