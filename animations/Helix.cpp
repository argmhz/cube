
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Helix : public Animation {

  void draw(Cube *cube) {

    static int x, y, z;
    static float _x, _y, _z, t;

    while(isRunning()){
        cube->shift( AXIS_Z, -1 );
       _x = cos( t );
       _y = sin( t );
        x = round( dmap( _x, -1, 1, 0, 7 ) );
        y = round( dmap( _y, -1, 1, 0, 7 ) );

        cube->set( x,y,7,0,0,15);

        if (t <= 2*M_PI) {
           t += M_PI/12;
        } else {
            t -= 2*M_PI;
        }

        cube->update();
        usleep(50000);
    }
  }

};
extern "C" Animation * create() {
    return new Helix;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
