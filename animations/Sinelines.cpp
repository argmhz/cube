
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Sinelines : public Animation {

  void draw(Cube *cube) {
    int i=0,x;

		float left, right, sine_base, x_dividor,ripple_height;

		while (isRunning())
		{
			for (x=0; x<8 ;++x)
			{
				x_dividor = 2 + sin((float)i/100)+1;
				ripple_height = 3 + (sin((float)i/200)+1)*6;

				sine_base = (float) i/40 + (float) x/x_dividor;

				left = 4 + sin(sine_base)*ripple_height;
				right = 4 + cos(sine_base)*ripple_height;
				right = 7-left;

				cube->line(0-3, x, (int) left, 7+3, x, (int) right,10,2,5);
			}
			i++;
			cube->update();
		usleep(100);
		cube->clear();
		}

  }

};
extern "C" Animation * create() {
    return new Sinelines;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
