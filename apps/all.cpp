#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <vector>
#include "../lib/core/Cube.h"
#include "../lib/animation/AniManager.h"

Cube * cube;
AniManager * manager;
int sleepTime = 20;

void animationChanger(Cube *c){
  manager = new AniManager(c);
  std::vector<std::string> files = manager->getAnimationsFiles("bin/animations");

  for (std::string file : files) {
    try {
      manager->loadAnimation(file.c_str());
      manager->getAnimation().draw(c);
    } catch (int e){
        std::cout << "Der skete en fejl med " << file.c_str();
    }

  }

  c->clear();
  c->update();
  c->stop();
}

void next(Cube *c){
  while(c->isRunning()){
    sleep(1);
    if(manager->isReady()) {
      sleep((unsigned)manager->getAnimation().getDuration());
      manager->stopAnimation();
    }
  }
}

int main(int argc, char *argv[])
{
  srand (time(NULL));
  // int s = atoi(argv[1]);
  cube = new Cube;

  sleep(1);
  std::thread th(animationChanger, cube);
  std::thread start = cube->start();
  cube->clear();
  cube->update();
  std::thread nextThread(next,cube);

  start.join();
  th.join();
  nextThread.join();

}
