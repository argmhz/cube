
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class DoubleHelix : public Animation {


  int speed = 50000;
  Cube::Color color1;
  Cube::Color color2;

  void onDataUpdate(json data){
    if(data["speed"].is_number()){
      speed = data["speed"].get<int>();
    }
    if(data["color1"].is_string()) {
      std::string c = data["color1"].get<std::string>();
      color1 = colorConverter(c.erase(0, 1));
    }
    if(data["color2"].is_string()) {
      std::string c = data["color2"].get<std::string>();
      color2 = colorConverter(c.erase(0, 1));
    }
  }

  void draw(Cube *cube) {

    static int x, y;
    static float _x, _y, t;

    color1.set(0,15,0);
    color2.set(0,0,15);

    while(isRunning()){
        // Shift previous frame up one first.
        cube->shift(AXIS_Z, 1);

        _x = cos( t );
        _y = sin( t );
        x = (int)round( dmap( _x, -1, 1, 0, 7 ) );
        y = (int)round( dmap( _y, -1, 1, 0, 7 ) );
        cube->set( x, y, 0, color1.red,color1.green,color1.blue);

        // all we need to do here is add a 180 deg. phase offset to create the double helix
        _x = cos( t + M_PI );
        _y = sin( t + M_PI );
        x = (int)round( dmap( _x, -1, 1, 0, 7 ) );
        y = (int)round( dmap( _y, -1, 1, 0, 7 ) );

        cube->set( x, y, 0, color2.red,color2.green,color2.blue);

        if (t <= 2*M_PI) {
           t += M_PI/12;
        } else {
            t -= 2*M_PI;
        }

        cube->update();
        usleep(speed);
    }

  }

};
extern "C" Animation * create() {
    return new DoubleHelix;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
