#include <iostream>
#include <thread>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"


Cube * cube = new Cube;


void animation(Cube *cube){

	cube->plane(AXIS_Z,4,15,0,0);
	// cube->rotateZ(90);
	cube->update();


	// cube->box(4,4,4,7,7,7,15,0,0);

	// int x,y,i;
	//
	// for (i=0;i<1000;i++)
	// {
	// 	x = sin(i/8)*2+3.5;
	// 	y = cos(i/8)*2+3.5;
	//
	// 	cube->set(x,y,1, 15, 0, 0);
	// 	cube->set(x,y,1, 15, 0, 0);
	// 	cube->update();
	// 	usleep(30000);
	// 	cube->clear();
	// }
}



int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread t(animation, cube);
  std::thread c = cube->start();

  t.join();
  c.join();

}
