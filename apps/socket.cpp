#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include "../lib/json.hpp"
#include "../lib/AniManager.cpp"
#include "../animations/Text.cpp"
#include "../lib/remotehelpers.cpp"
#include "../lib/Socket.cpp"


using json = nlohmann::json;

Cube * cube = new Cube;
AniManager *manager;

std::string selectedAnimaiton = "./bin/animations/Text.so";

void response(std::string type, std::string responseMessage){
  responseMessage.erase(std::remove(responseMessage.begin(), responseMessage.end(), '\n'), responseMessage.end());
  responseMessage.erase(std::remove(responseMessage.begin(), responseMessage.end(), '\r'), responseMessage.end());

  std::cout << "{\"" << type << "\":" << responseMessage << "}" <<  std::endl;
}


void incoming(){
  // instantiate Animation manager
  manager = new AniManager(cube);

  string ip = "localhost";
  string port = "1234";

  Socket *masterSocket = new Socket(AF_INET,SOCK_STREAM,0); //AF_INET (Internet mode) SOCK_STREAM (TCP mode) 0 (Protocol any)
  int optVal = 1;

  masterSocket->socket_set_opt(SOL_SOCKET, SO_REUSEADDR, &optVal); //You can reuse the address and the port
  masterSocket->bind(ip, port); //Bind socket on localhost:1234
  masterSocket->listen(10); //Start listening for incoming connections (10 => maximum of 10 Connections in Queue)

  while (true) {
    vector<Socket> reads(1);
    reads[0] = *masterSocket;
    int seconds = 10; //Wait 10 seconds for incoming Connections
    if(Socket::select(&reads, NULL, NULL, seconds) < 1){ continue; } else { break; }
  }

  Socket *newSocket = masterSocket->accept();

  while (true) {
    vector<Socket> reads(1);
    reads[0] = *newSocket;
    int seconds = 10; //Wait 10 seconds for input
    if(Socket::select(&reads, NULL, NULL, seconds) < 1){  continue; } else {
      string buffer;
      newSocket->socket_read(buffer, 1024); //Read 1024 bytes of the stream

      json command = json::parse(buffer);

      if(command["action"] == "select"){
        selectedAnimaiton = (std::string)command["animation"];
        manager->getAnimation().stop();
      }

      if(command["action"] == "set") {
        manager->getAnimation().onDataUpdate(command);
      }

      if(command["action"] == "options"){
        std::vector<string> files = manager->getAnimationsFiles("./bin/animations");
        json result;
        result["action"] = "options";
        result["animations"] = files;
        newSocket->socket_write((string)result.dump());
      }

    }
  }

  newSocket->socket_shutdown(2);
  newSocket->close();

  masterSocket->socket_shutdown(2);
  masterSocket->close();

}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  srand (time(NULL));

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
