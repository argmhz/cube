#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"
#include <stdlib.h>

class Folder: public Animation {

  unsigned long start;

  void draw(Cube * c) {
    srand(time(NULL));
    int speed = 20000;

    int xx, yy, zz, pullback[16], state = 0, backorfront = 7; //backorfront 7 for back 0 for front

    int folderaddr[16], LED_Old[16], oldpullback[16], ranx = random(16), rany = random(16), ranz = random(16), ranselect;
    int bot = 0, top = 1, right = 0, left = 0, back = 0, front = 0, side = 0, side_select;

    folderaddr[0] = -7;
    folderaddr[1] = -6;
    folderaddr[2] = -5;
    folderaddr[3] = -4;
    folderaddr[4] = -3;
    folderaddr[5] = -2;
    folderaddr[6] = -1;
    folderaddr[7] = 0;

    for (xx = 0; xx < 8; xx++) {
      oldpullback[xx] = 0;
      pullback[xx] = 0;
    }

    while (isRunning()) {
      if (top == 1) {
        if (side == 0) {
          //top to left-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(7 - LED_Old[yy], yy - oldpullback[yy], xx, 0, 0, 0);
              c -> set(7 - folderaddr[yy], yy - pullback[yy], xx, ranx, rany, ranz);
            }
          }
        }
        if (side == 2) {
          //top to back-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(7 - LED_Old[yy], xx, yy - oldpullback[yy], 0, 0, 0);
              c -> set(7 - folderaddr[yy], xx, yy - pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 3) {
          //top-side to front-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(7 - LED_Old[7 - yy], xx, yy + oldpullback[yy], 0, 0, 0);
              c -> set(7 - folderaddr[7 - yy], xx, yy + pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 1) {
          //top-side to right
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(7 - LED_Old[7 - yy], yy + oldpullback[yy], xx, 0, 0, 0);
              c -> set(7 - folderaddr[7 - yy], yy + pullback[yy], xx, ranx, rany, ranz);
            }
          }
        }
      } //top

      if (right == 1) {
        if (side == 4) {
          //right-side to top
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy + oldpullback[7 - yy], 7 - LED_Old[7 - yy], xx, 0, 0, 0);
              c -> set(yy + pullback[7 - yy], 7 - folderaddr[7 - yy], xx, ranx, rany, ranz);
            }
          }
        }
        if (side == 3) {
          //right-side to front-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, 7 - LED_Old[7 - yy], yy + oldpullback[yy], 0, 0, 0);
              c -> set(xx, 7 - folderaddr[7 - yy], yy + pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 2) {
          //right-side to back-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, 7 - LED_Old[yy], yy - oldpullback[yy], 0, 0, 0);
              c -> set(xx, 7 - folderaddr[yy], yy - pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 5) {
          //right-side to bottom
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy - oldpullback[yy], 7 - LED_Old[yy], xx, 0, 0, 0);
              c -> set(yy - pullback[yy], 7 - folderaddr[yy], xx, ranx, rany, ranz);
            }
          }
        }
      } //right

      if (left == 1) {
        if (side == 4) {
          //left-side to top
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy + oldpullback[yy], LED_Old[7 - yy], xx, 0, 0, 0);
              c -> set(yy + pullback[yy], folderaddr[7 - yy], xx, ranx, rany, ranz);
            }
          }
        }
        if (side == 3) {
          //left-side to front-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, LED_Old[7 - yy], yy + oldpullback[yy], 0, 0, 0);
              c -> set(xx, folderaddr[7 - yy], yy + pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 2) {
          //left-side to back-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, LED_Old[yy], yy - oldpullback[yy], 0, 0, 0);
              c -> set(xx, folderaddr[yy], yy - pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 5) {
          //left-side to bottom
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy - oldpullback[yy], LED_Old[yy], xx, 0, 0, 0);
              c -> set(yy - pullback[yy], folderaddr[yy], xx, ranx, rany, ranz);
            }
          }
        }
      } //left

      if (back == 1) {
        if (side == 1) {
          //back-side to right-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, yy + oldpullback[yy], LED_Old[7 - yy], 0, 0, 0);
              c -> set(xx, yy + pullback[yy], folderaddr[7 - yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 4) {
          // back-side to top-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy + oldpullback[yy], xx, LED_Old[7 - yy], 0, 0, 0);
              c -> set(yy + pullback[yy], xx, folderaddr[7 - yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 5) {
          // back-side to bottom
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy - oldpullback[yy], xx, LED_Old[yy], 0, 0, 0);
              c -> set(yy - pullback[yy], xx, folderaddr[yy], ranx, rany, ranz);
            }
          }
        } //state1
        if (side == 0) {
          //back-side to left-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, yy - oldpullback[yy], LED_Old[yy], 0, 0, 0);
              c -> set(xx, yy - pullback[yy], folderaddr[yy], ranx, rany, ranz);
            }
          }
        }
      } //back
      if (bot == 1) {
        if (side == 1) {
          // bottom-side to right-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(LED_Old[7 - yy], yy + oldpullback[yy], xx, 0, 0, 0);
              c -> set(folderaddr[7 - yy], yy + pullback[yy], xx, ranx, rany, ranz);
            }
          }
        }
        if (side == 3) {
          //bottom to front-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(LED_Old[7 - yy], xx, yy + oldpullback[yy], 0, 0, 0);
              c -> set(folderaddr[7 - yy], xx, yy + pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 2) {
          //bottom to back-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(LED_Old[yy], xx, yy - oldpullback[yy], 0, 0, 0);
              c -> set(folderaddr[yy], xx, yy - pullback[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 0) {
          //bottom to left-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(LED_Old[yy], yy - oldpullback[yy], xx, 0, 0, 0);
              c -> set(folderaddr[yy], yy - pullback[yy], xx, ranx, rany, ranz);
            }
          }
        }
      } //bot

      if (front == 1) {
        if (side == 0) {
          //front-side to left-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, yy - oldpullback[yy], 7 - LED_Old[yy], 0, 0, 0);
              c -> set(xx, yy - pullback[yy], 7 - folderaddr[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 5) {
          // front-side to bottom
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy - oldpullback[yy], xx, 7 - LED_Old[yy], 0, 0, 0);
              c -> set(yy - pullback[yy], xx, 7 - folderaddr[yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 4) {
          // front-side to top-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(yy + oldpullback[yy], xx, 7 - LED_Old[7 - yy], 0, 0, 0);
              c -> set(yy + pullback[yy], xx, 7 - folderaddr[7 - yy], ranx, rany, ranz);
            }
          }
        }
        if (side == 1) {
          //front-side to right-side
          for (yy = 0; yy < 8; yy++) {
            for (xx = 0; xx < 8; xx++) {
              c -> set(xx, yy + oldpullback[yy], 7 - LED_Old[7 - yy], 0, 0, 0);
              c -> set(xx, yy + pullback[yy], 7 - folderaddr[7 - yy], ranx, rany, ranz);
            }
          }
        }
      } //front

      c -> update();
      usleep(speed);
      for (xx = 0; xx < 8; xx++) {
        LED_Old[xx] = folderaddr[xx];
        oldpullback[xx] = pullback[xx];
      }

      if (folderaddr[7] == 7) {
        // pullback=8;
        for (zz = 0; zz < 8; zz++)
          pullback[zz] = pullback[zz] + 1;

        if (pullback[7] == 8) { //finished with fold
          c -> update();
          usleep(speed);
          //state++;
          //if(state==4)
          //state=0;

          ranselect = random(3);
          if (ranselect == 0) {
            ranx = 0;
            rany = random(0, 16);
            ranz = random(0, 16);
          }
          if (ranselect == 1) {
            ranx = random(0, 16);
            rany = 0;
            ranz = random(0, 16);
          }
          if (ranselect == 2) {
            ranx = random(0, 16);
            rany = random(0, 16);
            ranz = 0;
          }

          side_select = random(3);

          if (top == 1) { //                 TOP
            top = 0;
            if (side == 0) { //top to left
              left = 1;
              if (side_select == 0) side = 2;
              if (side_select == 1) side = 3;
              //if(side_select==2) side=4;
              if (side_select == 2) side = 5;
            } else
            if (side == 1) { //top to right
              right = 1;
              if (side_select == 0) side = 5;
              if (side_select == 1) side = 2;
              if (side_select == 2) side = 3;
              //if(side_select==3) side=4;
            } else
            if (side == 2) { //top to back
              back = 1;
              if (side_select == 0) side = 0;
              if (side_select == 1) side = 1;
              if (side_select == 2) side = 5;
              //if(side_select==3) side=4;
            } else
            if (side == 3) { //top to front
              front = 1;
              if (side_select == 0) side = 0;
              if (side_select == 1) side = 1;
              if (side_select == 2) side = 5;
              //if(side_select==3) side=4;
            }
          } else //top
            if (bot == 1) { //                 BOTTOM
              bot = 0;
              if (side == 0) { //bot to left
                left = 1;
                if (side_select == 0) side = 2;
                if (side_select == 1) side = 3;
                if (side_select == 2) side = 4;
                //if(side_select==3) side=5;
              } else
              if (side == 1) { //bot to right
                right = 1;
                //if(side_select==0) side=5;
                if (side_select == 0) side = 2;
                if (side_select == 1) side = 3;
                if (side_select == 2) side = 4;
              } else
              if (side == 2) { //bot to back
                back = 1;
                if (side_select == 0) side = 0;
                if (side_select == 1) side = 1;
                //if(side_select==2) side=5;
                if (side_select == 2) side = 4;
              } else
              if (side == 3) { //bot to front
                front = 1;
                if (side_select == 0) side = 0;
                if (side_select == 1) side = 1;
                //if(side_select==2) side=5;
                if (side_select == 2) side = 4;
              }
            } else //bot
              if (right == 1) { //                 RIGHT
                right = 0;
                if (side == 4) { //right to top
                  top = 1;
                  if (side_select == 0) side = 2;
                  if (side_select == 1) side = 3;
                  if (side_select == 2) side = 0;
                  //if(side_select==3) side=1;
                } else
                if (side == 5) { //right to bot
                  bot = 1;
                  if (side_select == 0) side = 0;
                  if (side_select == 1) side = 2;
                  if (side_select == 2) side = 3;
                  //if(side_select==3) side=1;
                } else
                if (side == 2) { //right to back
                  back = 1;
                  if (side_select == 0) side = 0;
                  //if(side_select==1) side=1;
                  if (side_select == 1) side = 5;
                  if (side_select == 2) side = 4;
                } else
                if (side == 3) { //right to front
                  front = 1;
                  if (side_select == 0) side = 0;
                  //if(side_select==1) side=1;
                  if (side_select == 1) side = 5;
                  if (side_select == 2) side = 4;
                }
              } else //bot
                if (left == 1) { //                 LEFT
                  left = 0;
                  if (side == 4) { //left to top
                    top = 1;
                    //if(side_select==0) side=2;
                    if (side_select == 0) side = 3;
                    if (side_select == 1) side = 2;
                    if (side_select == 2) side = 1;
                  } else
                  if (side == 5) { //left to bot
                    bot = 1;
                    //if(side_select==0) side=0;
                    if (side_select == 0) side = 2;
                    if (side_select == 1) side = 3;
                    if (side_select == 2) side = 1;
                  } else
                  if (side == 2) { //left to back
                    back = 1;
                    //if(side_select==0) side=0;
                    if (side_select == 0) side = 1;
                    if (side_select == 1) side = 5;
                    if (side_select == 2) side = 4;
                  } else
                  if (side == 3) { //left to front
                    front = 1;
                    //if(side_select==0) side=0;
                    if (side_select == 0) side = 1;
                    if (side_select == 1) side = 5;
                    if (side_select == 2) side = 4;
                  }
                } else //bot
                  if (front == 1) { //                 front
                    front = 0;
                    if (side == 4) { //front to top
                      top = 1;
                      if (side_select == 0) side = 2;
                      //if(side_select==1) side=3;
                      if (side_select == 1) side = 0;
                      if (side_select == 2) side = 1;
                    } else
                    if (side == 5) { //front to bot
                      bot = 1;
                      if (side_select == 0) side = 0;
                      if (side_select == 1) side = 2;
                      //if(side_select==2) side=3;
                      if (side_select == 2) side = 1;
                    } else
                    if (side == 0) { //front to left
                      left = 1;
                      if (side_select == 0) side = 2;
                      // if(side_select==1) side=3;
                      if (side_select == 1) side = 5;
                      if (side_select == 2) side = 4;
                    } else
                    if (side == 1) { //front to right
                      right = 1;
                      if (side_select == 0) side = 2;
                      // if(side_select==1) side=3;
                      if (side_select == 1) side = 5;
                      if (side_select == 2) side = 4;
                    }
                  } else //bot
                    if (back == 1) { //                 back
                      back = 0;
                      if (side == 4) { //back to top
                        top = 1;
                        //if(side_select==0) side=2;
                        if (side_select == 0) side = 3;
                        if (side_select == 1) side = 0;
                        if (side_select == 2) side = 1;
                      } else
                      if (side == 5) { //back to bot
                        bot = 1;
                        if (side_select == 0) side = 0;
                        //if(side_select==1) side=2;
                        if (side_select == 1) side = 3;
                        if (side_select == 2) side = 1;
                      } else
                      if (side == 0) { //back to left
                        left = 1;
                        //if(side_select==0) side=2;
                        if (side_select == 0) side = 3;
                        if (side_select == 1) side = 5;
                        if (side_select == 2) side = 4;
                      } else
                      if (side == 1) { //back to right
                        right = 1;
                        //if(side_select==0) side=2;
                        if (side_select == 0) side = 3;
                        if (side_select == 1) side = 5;
                        if (side_select == 2) side = 4;
                      }
                    } //bot

          // for(yy=0; yy<8; yy++)
          //for(xx=0; xx<8; xx++)
          //c->set(LED_Old[yy], xx, yy-oldpullback[yy], 0, 0, 0);
          for (xx = 0; xx < 8; xx++) {
            oldpullback[xx] = 0;
            pullback[xx] = 0;
          }

          folderaddr[0] = -8;
          folderaddr[1] = -7;
          folderaddr[2] = -6;
          folderaddr[3] = -5;
          folderaddr[4] = -4;
          folderaddr[5] = -3;
          folderaddr[6] = -2;
          folderaddr[7] = -1;

        } //pullback==7
      } //folderaddr==7

      if (folderaddr[7] != 7)
        for (zz = 0; zz < 8; zz++)
          folderaddr[zz] = folderaddr[zz] + 1;

    } //while

    c -> clear();
  }

  // void onDataUpdate(std::vector<std::string> data){
  //
  // }
};

extern "C"
Animation * create() {
  return new Folder;
}

extern "C"
void destroy(Animation * p) {
  delete p;
}
