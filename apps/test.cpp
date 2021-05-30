#include <iostream>
#include <bitset>
// #include <thread>
#include <unistd.h>
#include "../Cube.cpp"
// #include "Animation.hpp"
// #include "Manager.cpp"


int main(int argc, char *argv[])
{
  srand (time(NULL));
  // int s = atoi(argv[1]);

  Cube * c = new Cube;
  // std::thread th(&Cube::run, c);
  // c->loadAnimation("bin/animations/TestAni.so");
  // c->loadAnimation("bin/animations/FullRgb.so");
  // c->loadAnimation("bin/animations/Wipeout.so");
  // c->loadAnimation("bin/animations/Folder.so");
  // c->loadAnimation("bin/animations/Allan.so");
  // c->loadAnimation("bin/animations/BouncyvTwo.so");
  c->loadAnimation("bin/animations/SinewaveTwo.so");
  c->start();

}
