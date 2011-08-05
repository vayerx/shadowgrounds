
#ifndef SCALEDEFS_H
#define SCALEDEFS_H

//
// Different constant scales and stuff used in the game.
//

// (Unit "equals" VisualObject)

// Unit coordinates (unit coordinates / 1024 = storm terrain coordinates)
//#define UNIT_COORD_SCALE 1024

// Unit angle (unit angle / 10 = degree angle)
//#define UNIT_ANGLE_SCALE 10

// Unit angle to storm angle (to radians)
//#define UNIT_ANGLE_TO_RAD(x) ((float)x * 3.1415927f/(180 * UNIT_ANGLE_SCALE))
#define UNIT_ANGLE_TO_RAD(x) ((x) * (3.1415927f/180))

// And vice versa
//#define RAD_TO_UNIT_ANGLE(x) ((int)(x * (180 * UNIT_ANGLE_SCALE)/3.1415927f))
#define RAD_TO_UNIT_ANGLE(x) ((x) * (180/3.1415927f))

// Heightmap <-> storm terrain scale is depends on GameMap

#endif
