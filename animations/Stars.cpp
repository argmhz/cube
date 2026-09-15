
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include <mutex>

class Stars : public Animation {

  Cube *cube;
  std::mutex cubeMutex;

  void blink() {

    int i;
  	while(isRunning()){
  		int speed = rand()%40000;
  		int x = rand()%8;
  		int y = rand()%8;
  		int z = rand()%8;
  			for (i = 0; i < 16; i++) {
  				{
  					std::lock_guard<std::mutex> lock(cubeMutex);
  					cube->set(x,y,z,i,i,i);
  					cube->update();
  				}
  				usleep(speed);
  			}
  			for (i = 15; i >= 0; i--) {
  				{
  					std::lock_guard<std::mutex> lock(cubeMutex);
  					cube->set(x,y,z,i,i,i);
  					cube->update();
  				}
  				usleep(speed);
  			}
  	}
  }

  void draw(Cube *c) {

    cube = c;

    int num_blink = 5;

  	std::thread blinks[num_blink];

  	for (int i = 0; i < num_blink; i++) {
  		blinks[i] = std::thread([this] {this->blink(); });
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
