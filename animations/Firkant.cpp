
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Firkant : public Animation {

  int speed = 200000;	
	
  void draw(Cube *cube) {
	
	
	//cube->set(0,0,0,15,15,15);
	//cube->update();
	
	//usleep(1000000000);
	
	
	Cube::Color c;
	while(isRunning()){
	c.random();
		for(int i = 0; i<3;i++){
		cube->clear();
		
		cube->boxOutline(3-i,3-i,3-i,4+i,4+i,4+i,c.red,c.green,c.blue);
		cube->update();
		usleep(speed);
		}
		
		for(int i = 4; i >1; i--){
		  cube->clear();
		  cube->boxOutline(4-i,4-i,4-i,3+i,3+i,3+i,c.red,c.green,c.blue);
		  cube->update();
		  usleep(speed);		
		}
	
	}
	

  }

};
extern "C" Animation * create() {
    return new Firkant;
}

extern "C" void destroy(Animation * p) {
    delete p;
}

