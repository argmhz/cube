#include <iostream>
#include <thread>
#include <vector>
#include <iomanip>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

Cube * cube = new Cube;

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  char text[] = R"(
      {
          "Book": {
              "Width":  450,
              "Height": 30,
              "Title":  "Hello World",
              "isBiography": false,
              "NumOfCopies": 4,
              "LibraryIDs": [2319, 1406, 3854, 987]
          }
      }
      )";

      // Let's parse and serialize JSON
    json j_complete = json::parse(text);
    std::cout << std::setw(4) << j_complete << std::endl;



  // std::thread c = cube->start();
  // c.join();

}
