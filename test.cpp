#include <iostream>
#include <bitset>
#include <thread>
#include <unistd.h>
#include "Cube.cpp"
// #include "Animation.hpp"
// #include "Manager.cpp"


int main(int argc, char *argv[])
{
  // int s = atoi(argv[1]);

  Cube * c = new Cube;
  std::thread th(&Cube::run, c);
  c->loadAnimation("bin/animations/TestAni.so");
  c->start();
  
  th.join();

}
