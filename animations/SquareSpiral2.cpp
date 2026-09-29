
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class SquareSpiral2 : public Animation {

  void draw(Cube *cube) {

    int loc = 0;
    int iter = 0;
    while (isRunning())
    {
    for (loc =0;loc < 7; loc ++)
    	{
    		cube->shift (AXIS_Z,-1);
    		cube->set(0,loc,7,0,0,15);
    		cube->set(loc,7,7,0,0,15);
    		cube->set(7,7-loc,7,0,0,15);
    		cube->set(7-loc,0,7,0,0,15);
    		cube->set(0,7-loc,7,0,0,15);
    		cube->set(7-loc,7,7,0,0,15);
    		cube->set(7,loc,7,0,0,15);
    		cube->set(loc,0,7,0,0,15);
        cube->update();
    		usleep (100000);
    		iter++;
    	}
    loc = 0;
    }

  }

};
extern "C" Animation * create() {
    return new SquareSpiral2;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
