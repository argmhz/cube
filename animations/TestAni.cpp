#include "../Cube.cpp"
#include "../Animation.hpp"
#include <iostream>
#include <vector>

class Test : public Animation {
    void draw(Cube *c) {
      for (size_t y = 0; y < 8; y++) {
        for (size_t x = 0; x < 8; x++) {
          for (size_t z = 0; z < 8; z++) {
              c->set(z,x,y,0,15,0);
               // Cube::Color a = c->get(z,x,y);
               //  std::cout << a.red << " " << a.green << " " << a.blue << std::endl;
          }
        }
      }
      c->update();
    }

    void onDataUpdate(std::vector<std::string> data){

    }
};

extern "C" Animation * create() {
    return new Test;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
