#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class WormSqueeze : public Animation {

  void draw(Cube *cube) {

    srand (time(NULL));

    int size=2;
    int axis=AXIS_Y;
    int direction=1;
    int _delay=50000;

    int x, y, i,j,k, dx, dy;
    int cube_size;
    int origin = 0;

    if (direction == -1)
      origin = 7;

    cube_size = 8-(size-1);

    x = rand()%cube_size;
    y = rand()%cube_size;

    int maxColorCount = 50;
    int colorCount = 0;
    Cube::Color color;

    while (isRunning())
    {
      color = makeColorGradient(colorCount);

      if(colorCount < maxColorCount){
        colorCount++;
      }
      else
      {
        colorCount=0;
      }

      dx = ((rand()%3)-1);
      dy = ((rand()%3)-1);

      if ((x+dx) > 0 && (x+dx) < cube_size)
        x += dx;

      if ((y+dy) > 0 && (y+dy) < cube_size)
        y += dy;

      cube->shift(axis, direction);

      for (j=0; j<size;j++)
      {
        for (k=0; k<size;k++)
        {

          if (axis == 3)
            cube->set(x+j,y+k,origin,color.red,color.green,color.blue);

          if (axis == 2)
            cube->set(x+j,origin,y+k,color.red,color.green,color.blue);

          if (axis == 1)
            cube->set(origin,y+j,x+k,color.red,color.green,color.blue);

          cube->update();
        }
      }

      usleep(_delay);
    }

  }

};
extern "C" Animation * create() {
    return new WormSqueeze;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
