#pragma once
#include "CubeBuffer.h"
#include "helpers.h"
#include "Vec3.h"
#include <math.h>
#include <iostream>

void CubeBuffer::plane(int axis,int index,int r,int g,int b){
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
void CubeBuffer::plane(int axis,int index,Color color) {
  plane(axis, index, color.red, color.green, color.blue);
}

void CubeBuffer::clearPlane(int axis,int index){
  switch (axis) {
    case AXIS_X:
      for (size_t z = 0; z < 8; z++) {
        for (size_t y = 0; y < 8; y++) {
          clear(index,y,z);
        }
      }
    break;
    case AXIS_Y:
      for (size_t z = 0; z < 8; z++) {
        for (size_t x = 0; x < 8; x++) {
          clear(x,index,z);
        }
      }
    break;
    case AXIS_Z:
      for (size_t x = 0; x < 8; x++) {
        for (size_t y = 0; y < 8; y++) {
          clear(x,y,index);
        }
      }
    break;
  }
}

void CubeBuffer::sphere(int x,int y,int z,int radius,int r, int g,int b) {

	    // Iterate through phi, theta then convert r,theta,phi to  XYZ
    for (double phi = 0.; phi < 2*M_PI; phi += M_PI/10.) // Azimuth [0, M_2PI]
    {
        for (double theta = 0.; theta < M_PI; theta += M_PI/10.) // Elevation [0, M_PI]
        {
            set(round(radius * cos(phi) * sin(theta) + x),round(radius * sin(phi) * sin(theta) + y),round(radius * cos(theta) + z),r,g,b);
        }
    }
}

void CubeBuffer::sphere(int x,int y,int z,int radius, Color color) {
  sphere(x,y,z,radius, color.red, color.green, color.blue);
}

void CubeBuffer::shiftPlane(int axis,int index,int direction){

	int _z,_y,__z,__y;

		for(int z=0;z<8;z++){
			for(int y=0;y<8;y++){

				if(direction == -1){
					_z = z;
					__z = z+1;
				} else {
					_z = abs(z-7);
					__z = _z-1;
				}

				switch(axis){
				case AXIS_X:
					if(inBounce(index,y,__z)){
						set(index,y,_z,get(index,y,__z));
					}
					else
					{
						clear(index,y,_z);
					}
				break;

				case AXIS_Y:
					if(inBounce(__z,index,y)){
						set(_z,index,y,get(__z,index,y));
					}
					else
					{
						clear(_z,index,y);
					}
				break;
				case AXIS_Z:
					if(inBounce(__z,y,index)){
						set(_z,y,index,get(__z,y,index));
					}
					else
					{
						clear(_z,y,index);
					}
				break;
				}

			}
		}

}


void CubeBuffer::line(int x1,int y1,int z1,int x2,int y2,int z2,int r,int g,int b){
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

void CubeBuffer::all(int r, int g,int b){
  for (size_t z = 0; z < 8; z++) {
    for (size_t y = 0; y < 8; y++) {
      for (size_t x = 0; x < 8; x++) {
        set(x,y,z,r,g,b);
      }
    }
  }
}

void CubeBuffer::all(Color color) {
  all(color.red,color.green,color.blue);
}

void CubeBuffer::shift(int axis,int direction){

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

void CubeBuffer::box(int startx, int starty, int startz, int endx, int endy, int endz, int r, int g, int b) {
  if (startx > endx) swapint(startx,endx);
  if (starty > endy) swapint(starty,endy);
  if (startz > endz) swapint(startz,endz);

  for (int i = startx; i <= endx; i++) {
    for (int j = starty; j <= endy; j++) {
      for (int k = startz; k <= endz; k++) {
        set(i,j,k,r,g,b);
      }
    }
  }
}

void CubeBuffer::hollowBox(int startx, int starty, int startz, int endx, int endy, int endz, int r, int g,int b) {
  if (startx > endx) swapint(startx,endx);
  if (starty > endy) swapint(starty,endy);
  if (startz > endz) swapint(startz,endz);

  for (int i = startx; i <= endx; i ++) {
    for (int j = starty; j <= endy; j ++) {
      for (int k = startz; k <= endz; k ++) {
        if (i == startx || i == endx || j == starty || j == endy || k == startz || k == endz) {
          set(i,j,k,r,g,b);
        }
      }
    }
  }
}

void CubeBuffer::boxOutline(int startx, int starty, int startz, int endx, int endy, int endz, int r, int g,int b) {
  if (startx > endx) swapint(startx,endx);
  if (starty > endy) swapint(starty,endy);
  if (startz > endz) swapint(startz,endz);


  for (int i = startx; i <= endx; i++) {
    for (int j = starty; j <= endy; j++) {
      for (int k = startz; k <= endz; k++) {
        int sum =  (i == startx) + (i == endx) + (j == starty) + (j == endy) + (k == startz) + (k == endz);
        if (sum >= 2){
          set(i,j,k,r,g,b);
        }
      }
    }
  }
}

void CubeBuffer::rotate(int axis, int degree) {

  // Rotates around the cube's own center (3.5,3.5,3.5 -- the middle of the
  // 0..7 lattice), not the corner (0,0,0), using the standard rotation
  // matrices in Vec3.h instead of a hand-derived formula per axis.
  const double CENTER = 3.5;
  double radians = degree * PI / 180.0;

  Color n[8][8][8];
  for (size_t z = 0; z < 8; z++) {
    for (size_t x = 0; x < 8; x++) {
      for (size_t y = 0; y < 8; y++) {
        Vec3 centered{ x - CENTER, y - CENTER, z - CENTER };
        Vec3 rotated = rotateAroundAxis(centered, axis, radians);

        int _x = (int)round(rotated.x + CENTER);
        int _y = (int)round(rotated.y + CENTER);
        int _z = (int)round(rotated.z + CENTER);

        Color c = get(x,y,z);
        if (_x >= 0 && _x < 8 && _y >= 0 && _y < 8 && _z >= 0 && _z < 8) {
          n[_x][_y][_z] = c;
        }
      }
    }
  }
  clear();
  for (int z = 0; z < 8; z++) {
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        Color c = n[x][y][z];
        set(x,y,z,c);
      }
    }
  }
}

void CubeBuffer::rotateZ(int degree){
  // Kept only so any existing caller of rotateZ keeps working; the actual
  // rotation logic lives in rotate() now, so there is only one to maintain.
  rotate(AXIS_Z, degree);
}
// i and j are angles like latitude and longitude
// x=radius*sin(j)*cos(i); y=radius*sin(j)*sin(i); z=radius*cos(j);
