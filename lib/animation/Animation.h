#pragma once
#include <iostream>
#include <vector>
#include "../vendor/json.hpp"

using json = nlohmann::json;

class Cube;
class Animation {
public:

  Animation() {
    // std::cout << "Animation starter";
  }
  ~Animation(){}
  virtual void draw(Cube *c)=0;

  int getDuration(){
    return 10;
  }

  void stop(){
    running = false;
  }

  bool isRunning(){
    return running;
  }

  virtual void onDataUpdate(json data){}

private:
  bool running = true;
};

typedef Animation* create_t();
typedef void destroy_t(Animation*);
