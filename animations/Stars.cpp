
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Stars : public Animation {



  void static blink(Cube *cube) {

    int i;
  	while(isRunning()){
  		int speed = rand()%40000;
  		int x = rand()%8;
  		int y = rand()%8;
  		int z = rand()%8;
  			for (i = 0; i < 16; i++) {
  				cube->set(x,y,z,i,i,i);
  				cube->update();
  				usleep(speed);
  			}
  			for (i = 15; i >= 0; i--) {
  				cube->set(x,y,z,i,i,i);
  				cube->update();
  				usleep(speed);
  			}
  	}
  }

  void draw(Cube *cube) {

    int num_blink = 5;

  	std::thread blinks[num_blink];

  	for (int i = 0; i < num_blink; i++) {
  		blinks[i] = std::thread(blink,cube);
  	}

  	for (int i = 0; i < num_blink; i++) {
  		blinks[i].join();
  	}

  }

};
extern "C" Animation * create() {
    return new Stars;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
