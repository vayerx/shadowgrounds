
#ifndef AREA_TYPES_H
#define AREA_TYPES_H

// area types (that are handled seperately of each other)

// these are actually bit numbers, thus 30 is current maximum (0 is reserved for invalid, 31 is reserved for signed bit)
// there is no reason for these being bit numbers at the moment, but may be later extended so that
// one are may be of multiple types maybe? 

// a bitmask with specific area type bits set
typedef int area_type_mask;

// a value indicating the specific bit order number in mask (not mask)
typedef char area_type_value;

#define AREA_TYPE_INVALID 0

#define AREA_TYPE_CAMERA 1
#define AREA_TYPE_TRIGGER 2
#define AREA_TYPE_FOG 3
#define AREA_TYPE_AMBIENT 4
#define AREA_TYPE_RESERVED5 5
#define AREA_TYPE_RESERVED6 6
// ...
#define AREA_TYPE_RESERVED30 30

#define AREA_TYPE_MAX_AMOUNT 31

#endif

