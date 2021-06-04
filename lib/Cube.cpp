#include "Cube.h"
#include <math.h>

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

void Cube::sphere(int x,int y,int z,int radius,int r, int g,int b) {

	    // Iterate through phi, theta then convert r,theta,phi to  XYZ
    for (double phi = 0.; phi < 2*M_PI; phi += M_PI/10.) // Azimuth [0, M_2PI]
    {
        for (double theta = 0.; theta < M_PI; theta += M_PI/10.) // Elevation [0, M_PI]
        {
            set(round(radius * cos(phi) * sin(theta) + x),round(radius * sin(phi) * sin(theta) + y),round(radius * cos(theta) + z),r,g,b);
        }
    }

}
// void Cube::shiftPlane(int axis,int index,int direction){
//
// 	int _z,_y,__z,__y;
//
// 		for(int z=0;z<8;z++){
// 			for(int y=0;y<8;y++){
//
// 				if(direction == -1){
// 					_z = z;
// 					__z = z+1;
// 				} else {
// 					_z = abs(z-7);
// 					__z = _z-1;
// 				}
//
// 				switch(axis){
// 				case AXIS_X:
// 					if(inRange(index,y,__z)){
// 						set(index,y,_z,get(index,y,__z));
// 					}
// 					else
// 					{
// 						clear(index,y,_z);
// 					}
// 				break;
//
// 				case AXIS_Y:
// 					if(inRange(__z,index,y)){
// 						set(_z,index,y,get(__z,index,y));
// 					}
// 					else
// 					{
// 						clear(_z,index,y);
// 					}
// 				break;
// 				case AXIS_Z:
// 					if(inRange(__z,y,index)){
// 						set(_z,y,index,get(__z,y,index));
// 					}
// 					else
// 					{
// 						clear(_z,y,index);
// 					}
// 				break;
// 				}
//
// 			}
// 		}
//
// }


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

// // rotate(pitch, roll, yaw) {
//     var cosa = Math.cos(yaw);
//     var sina = Math.sin(yaw);
//
//     var cosb = Math.cos(pitch);
//     var sinb = Math.sin(pitch);
//
//     var cosc = Math.cos(roll);
//     var sinc = Math.sin(roll);
//
//     var Axx = cosa*cosb;
//     var Axy = cosa*sinb*sinc - sina*cosc;
//     var Axz = cosa*sinb*cosc + sina*sinc;
//
//     var Ayx = sina*cosb;
//     var Ayy = sina*sinb*sinc + cosa*cosc;
//     var Ayz = sina*sinb*cosc - cosa*sinc;
//
//     var Azx = -sinb;
//     var Azy = cosb*sinc;
//     var Azz = cosb*cosc;
//
//     for (var i = 0; i < points.length; i++) {
//         var px = points[i].x;
//         var py = points[i].y;
//         var pz = points[i].z;
//
//         points[i].x = Axx*px + Axy*py + Axz*pz;
//         points[i].y = Ayx*px + Ayy*py + Ayz*pz;
//         points[i].z = Azx*px + Azy*py + Azz*pz;
//     }
// }
