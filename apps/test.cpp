#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
// #include "../lib/AniManager.cpp"

Cube * cube = new Cube;
// AniManager * Manager;

void animationChanger(Cube *c){
  sleep(1);
  int px,py,pz,r,g,b;

  while(c->isRunning()){

    px = random(3,4);
    py = random(3,4);
    pz = random(3,4);

    r = rand() % 16;
    g = rand() % 16;
    b = rand() % 16;

    for (size_t i = 0; i < 11; i++) {
      c->sphere(px,py,pz,i,r,g,b);
      c->update();
      usleep(80000+(i*1000));
      c->clear();
    }
    usleep(100000);
  }


}

int main(int argc, char *argv[]){

  srand (time(NULL));

  std::thread t(animationChanger, cube);
  std::thread c = cube->start();
  std::cout << "hej med dig";
  c.join();
  t.join();
}
