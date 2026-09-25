#include <iostream>
#include <thread>
#include <vector>
#include "../lib/core/Cube.h"
#include "../lib/helpers.h"


Cube * cube = new Cube;


void animation(Cube *cube){

  // Z
  sleep(1);
  cube->line(0,0,0,7,7,7,15,15,15);

  for (size_t i = 0; i < 360; i++) {
    cube->rotate(AXIS_X,i);
    cube->update();
    sleep(1);
  }

  // cube->rotate(AXIS_X,45);
  // cube->update();
  // sleep(1);


}



int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread t(animation, cube);
  std::thread c = cube->start();

  t.join();
  c.join();

}
