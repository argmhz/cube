#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include "../lib/json.hpp"
#include "../lib/AniManager.cpp"
#include "../lib/CommandHandler.h"
#include "../lib/AnimationCommandHandler.h"
#include "../lib/ConnectionLoop.h"
#include "../lib/Log.h"
#include "../animations/Text.cpp"
#include "../lib/remotehelpers.cpp"
#include "../lib/Socket.cpp"


using json = nlohmann::json;

Cube * cube = new Cube;
AniManager *manager;

std::string selectedAnimaiton = "./bin/animations/Text.so";

void incoming(){
  AnimationCommandHandler handler(*manager, selectedAnimaiton, "./bin/animations");

  string ip = "localhost";
  string port = "1234";

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

  // instantiate Animation manager before any thread can touch it
  manager = new AniManager(cube);

  // Start cube
  std::thread cubeThread = cube->start();
  std::thread incomingThread(incoming);


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
