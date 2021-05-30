#include "Cube.h"

void Cube::plane(int axis,int index,int r,int g,int b){
  switch(axis){
    case AXIS_X:
    for(int z=0;z<8;z++){
      for(int y=0;y<8;y++){
        set(index,y,z,r,g,b);
      }
    }
    break;
    case AXIS_Y:
    for(int z=0;z<8;z++){
      for(int x=0;x<8;x++){
        set(x,index,z,r,g,b);
      }
    }
    break;
    case AXIS_Z:
    for(int y=0;y<8;y++){
      for(int x=0;x<8;x++){
        set(x,y,index,r,g,b);
      }
    }
    break;
   }
}

void Cube::line(int x1,int y1,int z1,int x2,int y2,int z2,int r,int g,int b){
  bool reverseX = false;
  bool reverseY = false;
  bool reverseZ = false;

  if (x1 > x2) {swapint(x1,x2);reverseX=true;}
  if (y1 > y2) {swapint(y1,y2);reverseY=true;}
  if (z1 > z2) {swapint(z1,z2);reverseZ=true;}

  int delx = x2 - x1;
  int dely = y2 - y1;
  int delz = z2 - z1;

  int longest = (delx>dely?delx>delz?delx:delz>dely?delz:dely:dely>delz?dely:delz>delx?delz:delx);
  for (int i = 0; i < longest; i++) {
  int xpos;
  if (reverseX) xpos = roundClostest(((longest-i)*delx),longest) + x1;
  else xpos = roundClostest((i*delx),longest) + x1;

  int ypos;
  if (reverseY) ypos = roundClostest(((longest-i)*dely),longest) + y1;
  else ypos = roundClostest((i*dely),longest) + y1;

  int zpos;
  if (reverseZ) zpos = roundClostest(((longest-i)*delz),longest) + z1;
  else zpos = roundClostest((i*delz),longest) + z1;

    set(xpos,ypos,zpos,r,g,b);
  }

  if (reverseX) swapint(x1,x2);
  if (reverseY) swapint(y1,y2);
  if (reverseZ) swapint(z1,z2);
  set(x2,y2,z2,r,g,b);
}

void Cube::all(int r, int g,int b){
  for (size_t z = 0; z < 8; z++) {
    for (size_t y = 0; y < 8; y++) {
      for (size_t x = 0; x < 8; x++) {
        set(x,y,z,r,g,b);
      }
    }
  }
}

void Cube::shift(int axis,int direction){

  int i, x ,y;
  int ii, iii;
  int state;

  for (i = 0; i < 8; i++) {

    if (direction == -1) {
      ii = i;
    } else {
      ii = (7-i);
    }

    for (x = 0; x < 8; x++) {

      for (y = 0; y < 8; y++) {

        if (direction == -1) {
          iii = ii+1;
        } else {
          iii = ii-1;
        }

    		Color color;
   			if (axis == AXIS_Z) {
					color = get(x,y,iii);
					set(x,y,ii,color.red,color.green,color.blue);
				}

				if (axis == AXIS_Y) {
					color = get(x,iii,y);
					set(x,ii,y,color.red,color.green,color.blue);
				}

  			if (axis == AXIS_X) {
					color = get(iii,y,x);
					set(ii,y,x,color.red,color.green,color.blue);
				}
      }
    }
  }

    if (direction == -1){
      i = 7;
    } else {
      i = 0;
    }

    for (x = 0; x < 8; x++){
        for (y = 0; y < 8; y++){
            if (axis == AXIS_Z)
                clear(x,y,i);

            if (axis == AXIS_Y)
                clear(x,i,y);

            if (axis == AXIS_X)
                clear(i,y,x);
	    }
    }

}
