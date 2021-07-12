#include <iostream>
#include <thread>
#include <mutex>
#include <string>
// #include <vector>
// #include <iomanip>

#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include "../lib/json.hpp"
#include "../lib/AniManager.cpp"
#include "../animations/Text.cpp"
#include "../lib/remotehelpers.cpp"

using json = nlohmann::json;

std::mutex msg_mutex;
Cube * cube = new Cube;
AniManager *manager;

std::string selectedAnimaiton = "./bin/animations/Text.so";

void incoming(){
  // instantiate Animation manager
  manager = new AniManager(cube);

  while (true) {

    std::lock_guard<std::mutex> lock{msg_mutex};

    for (std::string line; std::getline(std::cin, line);) {
      std::cout << line << std::endl;
      json command = json::parse(line);

      if(command["action"] == "select"){
        selectedAnimaiton = "./bin/animations/" + (std::string)command["animation"] + ".so";
        manager->getAnimation().stop();
      }

      if(command["action"] == "set") {
        manager->getAnimation().onDataUpdate(command);
      }

    }

  }
}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  srand (time(NULL));

  // Start cube
  std::thread cubeThread = cube->start();
  std::thread incomingThread(incoming);

  // // listen for incoming data....
  while(cube->isRunning()){
    std::cout << "Change to " << selectedAnimaiton.c_str() << std::endl;
    manager->loadAnimation(selectedAnimaiton.c_str());
    std::cout << manager->getAnimation().getPropertiesString() << std::endl;
    cube->clear();
    cube->update();
    manager->getAnimation().draw(cube);
    sleep(1);
  }

  cubeThread.join();
  incomingThread.join();
}
