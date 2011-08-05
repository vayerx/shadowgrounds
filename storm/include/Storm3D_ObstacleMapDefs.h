// Copyright 2002-2004 Frozenbyte Ltd.


#ifndef STORM3D_OBSTACLEMAPDEFS_H
#define STORM3D_OBSTACLEMAPDEFS_H

// collision heightmap multiplier 
// (compared to initial loaded/rendered heightmap)
// DEPRECATED: now given to storm as a parameter
//#define COLLISION_HEIGHTMAP_SHIFT 2
//#define COLLISION_HEIGHTMAP_MULT (1 << COLLISION_HEIGHTMAP_SHIFT)

// obstacle map is 8 x heightmap accuracy (64x area size)
// DEPRECATED: now given to storm as a parameter
//#define OBSTACLE_MAP_MULT_SHIFT 3
//#define OBSTACLE_MAP_MULTIPLIER (1 << OBSTACLE_MAP_MULT_SHIFT)

// obstacle map bits (unsigned short, 16 bits)
#define OBSTACLE_MAP_MASK_HEIGHT 0xffff

#define OBSTACLE_MAP_MASK_ALL 0xffff

// min and max height that the obstaclemap can handle
#define OBSTACLE_MAP_MIN_HEIGHT 0
#define OBSTACLE_MAP_MAX_HEIGHT 0xffff

// NOTE: unhittable means that the obstacle won't be hit by a normal
// raytrace (bullets), seethrough means that the obstacle won't be
// hit by a line-of-sight raytrace.
// rounded means that the 45 deg angle corner rounding is applied.
// movable means that the obstacle is a moving obstacle

// WARNING: these values MUST equal to the ones defined in game code
// or unexpected behaviour will occur (see areamasks.h)
#define OBSTACLE_AREA_UNHITTABLE 256
#define OBSTACLE_AREA_SEETHROUGH 512
#define OBSTACLE_AREA_MOVABLE 1024
#define OBSTACLE_AREA_ROUNDED 2048
#define OBSTACLE_AREA_RESERVED1 4096
#define OBSTACLE_AREA_BUILDINGWALL ((1<<12))
#define OBSTACLE_AREA_BREAKABLE ((1<<5))


// old...
//#define COLLISION_HEIGHTMAP_MULT 2
//#define COLLISION_HEIGHTMAP_SHIFT 1

// old...
//#define OBSTACLE_MAP_MASK_HEIGHT 0x3fff
//#define OBSTACLE_MAP_MASK_UNHITTABLE 0x4000
//#define OBSTACLE_MAP_MASK_SEETHROUGH 0x8000

#endif

