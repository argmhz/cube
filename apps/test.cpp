#include <iostream>
#include <thread>

#include "../lib/Cube.cpp"
#include "../lib/helpers.h"

Cube * cube = new Cube;

void animation(Cube *cube){

  while(cube->isRunning()){

  }

}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread t(animation, cube);
  std::thread c = cube->start();

  t.join();
  c.join();

}
