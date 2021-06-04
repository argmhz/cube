#pragma once
#include <iostream>
#include <vector>

class Cube;
class Animation {
public:

  Animation() {
    std::cout << "Animation starter";
  }
  ~Animation(){}
  virtual void draw(Cube *c)=0;

  int getDuration(){
    return 2;
  }

  void stop(){
    running = false;
  }

  bool isRunning(){
    return running;
  }
  // virtual void onDataUpdate(std::vector<std::string> data);

private:
  bool running = true;
};

typedef Animation* create_t();
typedef void destroy_t(Animation*);
