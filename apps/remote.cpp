#include <iostream>
#include <thread>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"


Cube * cube = new Cube;

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread c = cube->start();

  c.join();

}
