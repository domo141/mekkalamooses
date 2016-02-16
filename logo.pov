/*
 * roundbox.inc embedded in this file -- logo code at the end
 */

/************************************************************\
  ________________________________________
 /                                        \
 | RoundBox Macro                         |
 |                                        |
 | Copyright © Julius Klatte 1999         |
 | Bug reports or comments:               |
 | http://surf.to/JKhome                  |
 | klatte@hotmail.com                     |
 \________________________________________/

Include this file in the scene to make use of the macro.
(#include "roundbox.inc")

Usage: RoundBox(Vector1,Vector2,RoundSize)
       optional: #declare Merge = 1;

Vector1 and Vector2 are the standard vectors(!) as in the normal
box object. The RoundSize (float) value is the radius of the
'roundness' at the sides of the box. Make sure the RoundSize is not
too big and Vector1 values are smaller than Vector2 values
(the macro assumes that Vector1 is the lower left front corner of
the rounded box and Vector2 is the upper right back corner).

If Merge is set to 1, the parts of the objects are merged so that
transparency looks better. As a default, they are combined in a union.

Examples:

1) object { RoundBox(<0,0,0>,<2,2,2>,0.2) texture {...} }

2) merge { #declare Merge=1; object { RoundBox(<-4,-1,-0.5>,<4,1,0.5>,0.3) }
           object { RoundBox(<-1,-4,-1>,<1,4,1>,0.3) }
           texture {...} }

\************************************************************/

#macro RoundBox(Vec1,Vec2,RS)
#if (Vec1.x>Vec2.x | Vec1.y>Vec2.y | Vec1.z>Vec2.z)
#debug "\n   RoundBox error: Vector2 has a value smaller than the corresponding value in Vector1.\n   Default vectors selected."
#local Vec1=<0,0,0>; #local Vec2=<1,1,1>; #end
#if (Vec2.x-Vec1.x<=2*RS | Vec2.y-Vec1.y<=2*RS | Vec2.z-Vec1.z<=2*RS)
#debug "\n   RoundBox error: RoundSize too high. Set to 0.01 .\n" #local RS=0.01; #end
#ifndef (Merge) #declare Merge=0; #end
#local Box1 = box { <Vec1.x+RS,Vec1.y,Vec1.z+RS>, <Vec2.x-RS,Vec2.y,Vec2.z-RS> }
#local Box2 = box { <Vec1.x,Vec1.y+RS,Vec1.z+RS>, <Vec2.x,Vec2.y-RS,Vec2.z-RS> }
#local Box3 = box { <Vec1.x+RS,Vec1.y+RS,Vec1.z>, <Vec2.x-RS,Vec2.y-RS,Vec2.z> }
#local P1 = <Vec1.x+RS,Vec1.y+RS,Vec1.z+RS>; #local P2 = <Vec2.x-RS,Vec1.y+RS,Vec1.z+RS>;
#local P3 = <Vec2.x-RS,Vec1.y+RS,Vec2.z-RS>; #local P4 = <Vec1.x+RS,Vec1.y+RS,Vec2.z-RS>;
#local P5 = <Vec1.x+RS,Vec2.y-RS,Vec1.z+RS>; #local P6 = <Vec2.x-RS,Vec2.y-RS,Vec1.z+RS>;
#local P7 = <Vec2.x-RS,Vec2.y-RS,Vec2.z-RS>; #local P8 = <Vec1.x+RS,Vec2.y-RS,Vec2.z-RS>;
#if (Merge=0) #local Roundness = union {
 sphere { P1 , RS } sphere { P2 , RS } sphere { P3 , RS } sphere { P4 , RS }
 sphere { P5 , RS } sphere { P6 , RS } sphere { P7 , RS } sphere { P8 , RS }
 cylinder { P1 , P2 , RS } cylinder { P2 , P3 , RS } cylinder { P3 , P4 , RS }
 cylinder { P4 , P1 , RS } cylinder { P5 , P6 , RS } cylinder { P6 , P7 , RS }
 cylinder { P7 , P8 , RS } cylinder { P8 , P5 , RS } cylinder { P1 , P5 , RS }
 cylinder { P2 , P6 , RS } cylinder { P3 , P7 , RS } cylinder { P4 , P8 , RS }
} #end
#if (Merge=1) #local Roundness = merge {
 sphere { P1 , RS } sphere { P2 , RS } sphere { P3 , RS } sphere { P4 , RS }
 sphere { P5 , RS } sphere { P6 , RS } sphere { P7 , RS } sphere { P8 , RS }
 cylinder { P1 , P2 , RS } cylinder { P2 , P3 , RS } cylinder { P3 , P4 , RS }
 cylinder { P4 , P1 , RS } cylinder { P5 , P6 , RS } cylinder { P6 , P7 , RS }
 cylinder { P7 , P8 , RS } cylinder { P8 , P5 , RS } cylinder { P1 , P5 , RS }
 cylinder { P2 , P6 , RS } cylinder { P3 , P7 , RS } cylinder { P4 , P8 , RS }
} #end
#if (Merge=0) #local RoundedBoxObject = union { object {Box1} object {Box2} object {Box3} object {Roundness} } #end
#if (Merge!=0) #local RoundedBoxObject = merge { object {Box1} object {Box2} object {Box3} object {Roundness} } #end
object { RoundedBoxObject }
#end

//global_settings { charset utf8 }

#include "colors.inc"
//background { color Black }
camera {
    location <-0.10, -0.35, -2.76>
    look_at  <+0.015, +0.05, 0>
    angle 70
    rotate 0
}

union {
//                x      y      z        x      y      z
         box { <-1.10, -0.85, -1.10>, < 1.10, -0.65, -1.09> }
         box { <-1.10,  0.75, -1.10>, < 1.10,  0.55, -1.09> }
         box { <-0.90, -0.75, -1.10>, <-1.10,  0.65, -1.09> }
         box { < 0.90, -0.75, -1.10>, < 1.10,  0.65, -1.09> }
         //pigment { color rgb <0.18, 0.37, 0.0> } // pulukan vihree
         pigment { color rgb <0.1, 0.1, 0.1> }
}

object {
       RoundBox(<-1.1, -0.88, -1>, <1.1, 0.88, 9>, 0.2)
       texture {
               //pigment { color Blue }
               pigment { color rgbt <0.0, 0.0, 1.0, 0.7> }
               //pigment { color rgb <0.36, 0.67, 1.0> }
       }
     finish {
          ambient .4
     }
}

text {
     ttf "cyrvetic.ttf" "mekkala" 1, 0
     pigment { White }
     scale <0.42, 0.60, 0.60>
     translate <-0.75, 0.00, -1.2>
     finish {
          /* ambient .3 */
          diffuse 1
     }
}

text {
     ttf "cyrvetic.ttf" "mooses" 5, 0
     pigment { White }
     scale <0.45, 0.60, 0.60>
     translate <-0.74, -0.50, -1.2>
     finish {
          /* ambient .3 */
          diffuse 1
     }
}

light_source { <0, -0, -5> color White }
