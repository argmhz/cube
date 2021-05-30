#pragma once
#include <iostream>
#include <vector>

class Cube;
class Animation {
public:

  Animation() {}
  ~Animation(){}
  virtual void draw(Cube *c)=0;
  // virtual void onDataUpdate(std::vector<std::string> data)=0;
};

typedef Animation* create_t();
typedef void destroy_t(Animation*);
