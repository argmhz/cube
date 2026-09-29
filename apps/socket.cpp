#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include "../lib/core/Cube.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"
#include "../lib/animation/AniManager.h"
#include "../lib/net/CommandHandler.h"
#include "../lib/net/AnimationCommandHandler.h"
#include "../lib/net/ConnectionLoop.h"
#include "../lib/net/FrameStream.h"
#include "../lib/Log.h"
#include "../lib/net/Config.h"
#include "../animations/Text.cpp"
#include "../lib/remotehelpers.h"
#include "../lib/net/Socket.h"


using json = nlohmann::json;

Cube * cube = new Cube;
AniManager *manager;

std::string selectedAnimaiton;

void incoming(const Config &config){
  AnimationCommandHandler handler(*manager, selectedAnimaiton, config.animationsDir);

  string ip = config.host;
  string port = config.port;

  Socket *masterSocket = new Socket(AF_INET,SOCK_STREAM,0); //AF_INET (Internet mode) SOCK_STREAM (TCP mode) 0 (Protocol any)
  int optVal = 1;

  masterSocket->socket_set_opt(SOL_SOCKET, SO_REUSEADDR, &optVal); //You can reuse the address and the port
  masterSocket->bind(ip, port); //Bind socket on localhost:1234
  masterSocket->listen(10); //Start listening for incoming connections (10 => maximum of 10 Connections in Queue)
  Log::info("listening on " + ip + ":" + port);

  while (true) {
    // Wait for a client to connect.
    Socket *newSocket = nullptr;
    while (!newSocket) {
      vector<Socket> reads(1);
      reads[0] = *masterSocket;
      int seconds = 10; //Wait 10 seconds for incoming connections
      if(Socket::select(&reads, NULL, NULL, seconds) < 1){ continue; }

      Socket *candidate = masterSocket->accept();
      if (candidate->sock >= 0) {
        newSocket = candidate;
      }
    }

    Log::info("client connected from " + newSocket->address);
    serveConnection(*newSocket, handler);
    Log::info("client disconnected");

    newSocket->socket_shutdown(2);
    newSocket->close();
    // Loop back around and wait for the next connection instead of exiting.
  }
}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  srand (time(NULL));

  Config config = parseArgs(std::vector<std::string>(argv + 1, argv + argc));
  selectedAnimaiton = config.animationsDir + "/Text.so";

  // instantiate Animation manager before any thread can touch it
  manager = new AniManager(cube);

  // Start cube
  std::thread cubeThread = cube->start();
  std::thread incomingThread([&config]{ incoming(config); });

  // Off unless --stream-port is given: only `make sim-socket` runs with it,
  // to let sim/viewer watch the cube live on a machine that has no LEDs.
  if (!config.streamPort.empty()) {
    std::thread([&config]{
      runFrameStream(*cube, config.host, config.streamPort, config.streamFps);
    }).detach();
  }


  Text *t = new Text;
  t->setText(getIpAddress());
  manager->loadAnimation(t);
  manager->getAnimation().draw(cube);

  while(cube->isRunning()){
    manager->loadAnimation(selectedAnimaiton.c_str());

    cube->clear();
    cube->update();

    manager->getAnimation().draw(cube);

  }

  cubeThread.join();
  incomingThread.join();
}
