#include <iostream>
#include <bitset>

#include "Cube.cpp"
// #include "Animation.hpp"
// #include "Manager.cpp"


int main()
{

  Cube *c = new Cube;

  c->loadAnimation("bin/animations/TestAni.so");
  c->run();
  //
//   // c->loadAnimation("animations/TestAni.so");
//   //
//   // c->loadAnimation("animations/TestAni.so");
//

  // std::cout << "Printable ASCII:\n";
  // for (char i = 32; i < 127; ++i) {
  //   std::cout << i << " ";
  //   if (i % 16 == 15)
  //   std::cout << '\n';
  // }
}
