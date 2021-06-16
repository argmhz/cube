#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/AniManager.cpp"
#include "../lib/Animation.cpp"

Cube * cube = new Cube;
AniManager * Manager;

void animationChanger(Cube *c, char * name){
  sleep(1);
  Manager = new AniManager(cube);
  Manager->loadAnimation(name);
  Manager->getAnimation().draw(cube);
}

int main(int argc, char *argv[]){

  // srand (time(NULL));

  std::thread t(animationChanger, cube,argv[1]);
  std::thread c = cube->start();

  c.join();
  t.join();
}
