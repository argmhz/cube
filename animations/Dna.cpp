#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Dna : public Animation {

  void draw(Cube *c) {

    int x, y, xx, yy;
    static float _x, _y;
    static float t;

    while(isRunning()){

        c->shift(AXIS_Y, 1);
        _x = cos( t );
        _y = sin( t );
        x = round( dmap( _x, -1, 1, 1, 6 ) );
        y = round( dmap( _y, -1, 1, 1, 6 ) );

        _x = cos( t + M_PI );
        _y = sin( t + M_PI );
        xx = round( dmap( _x, -1, 1, 1, 6 ) );
        yy = round( dmap( _y, -1, 1, 1, 6 ) );
        
        c->line( x, 0,y, xx,  0,yy, 0,0,15);
        c->set( x,  0,y, 0,15,0 );
        c->set( xx,  0,yy, 15,0,0);

        if ( t <= 2*M_PI ) {
           t += M_PI/12;
        } else {
            t -= 2*M_PI;
        }

        c->update();
        usleep(70000);
    }

        c->clear();
  }

};

extern "C" Animation * create() {
    return new Dna;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
