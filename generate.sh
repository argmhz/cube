
if [ $# -eq 0 ] ; then
    echo 'No name given'
    exit 0
fi

FILE=./animations/$1.cpp
if [ -f "$FILE" ]; then
    echo "$FILE already exists."
    exit 0
fi

echo "generating $1"

echo '
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class '$1' : public Animation {

  void draw(Cube *cube) {


  }

};
extern "C" Animation * create() {
    return new '$1';
}

extern "C" void destroy(Animation * p) {
    delete p;
}
' >> ./animations/$1.cpp
