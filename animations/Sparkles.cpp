
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Sparkles : public Animation {

  void draw(Cube *cube) {
    int i;
		int v;
		int voxels = 100;
		while (isRunning())
		{
			for (v=0;v<=voxels;++v)
				cube->set(rand()%8,rand()%8,rand()%8,15,15,15);

			cube->update();
			usleep(100000);
			cube->clear();
		}

  }

};
extern "C" Animation * create() {
    return new Sparkles;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
