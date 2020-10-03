#include <iostream>
#include "Cube.hpp"
#include "Animation.hpp"


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
