#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/AniManager.cpp"

Cube * cube;
AniManager * manager;
int sleepTime = 20;

void animationChanger(Cube *c){
  manager = new AniManager(c);
  std::vector<std::string> files = manager->getAnimationsFiles("bin/animations");

  for (std::string file : files) {
    try {
      std::cout << file << std::endl;
      manager->loadAnimation(file.c_str());
        std::cout << "her";
      manager->getAnimation().draw(c);
    } catch (int e){
        std::cout << "nej nej nej";
    }

  }

  c->stop();
}

void next(Cube *c){
  // int s = manager->getAnimation().getDuration() || 10;

  while(c->isRunning()){
    // std::cout << manager->isReady() << std::endl;
    sleep(1);
    if(manager->isReady()) {
      std::cout << manager->getAnimation().getDuration() << std::endl;
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
    // manager->loadAnimation("bin/animations/SinewaveTwo.so");
    // manager->getAnimation().draw(cube);


  start.join();
  th.join();
  nextThread.join();

}
