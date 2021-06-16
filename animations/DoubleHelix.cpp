
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class DoubleHelix : public Animation {

  void draw(Cube *cube) { 

    static int x, y;
    static float _x, _y, t;

    while(isRunning()){
        // Shift previous frame up one first.
        cube->shift(AXIS_Z, 1);

        _x = cos( t );
        _y = sin( t );
        x = (int)round( dmap( _x, -1, 1, 0, 7 ) );
        y = (int)round( dmap( _y, -1, 1, 0, 7 ) );
        cube->set( x, y, 0, 0,15,0);

        // all we need to do here is add a 180 deg. phase offset to create the double helix
        _x = cos( t + M_PI );
        _y = sin( t + M_PI );
        x = (int)round( dmap( _x, -1, 1, 0, 7 ) );
        y = (int)round( dmap( _y, -1, 1, 0, 7 ) );

        cube->set( x, y, 0, 0,0,15);

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
    return new DoubleHelix;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
