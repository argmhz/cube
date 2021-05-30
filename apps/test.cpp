#include <iostream>
#include <bitset>
#include <thread>
#include <unistd.h>
#include "../Cube.cpp"
// #include "Animation.hpp"
// #include "Manager.cpp"


int main(int argc, char *argv[])
{
  // int s = atoi(argv[1]);

  Cube * c = new Cube;
  std::thread th(&Cube::run, c);
  c->loadAnimation("bin/animations/TestAni.so");
  // c->loadAnimation("bin/animations/FullRgb.so");
  c->start();
  sleep(2);



  th.join();

}
