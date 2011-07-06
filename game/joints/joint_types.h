
#ifndef JOINT_TYPES_H
#define JOINT_TYPES_H

// joint types (that are handled seperately of each other)

// these are actually bit numbers, thus 30 is current maximum (0 is reserved for invalid, 31 is reserved for signed bit)
// there is no reason for these being bit numbers at the moment, but may be later extended so that
// one are may be of multiple types maybe? 

// a bitmask with specific joint type bits set
typedef int joint_type_mask;

// a value indicating the specific bit order number in mask (not mask)
typedef char joint_type_value;

#define JOINT_TYPE_INVALID 0

#define JOINT_TYPE_ROPE 1
#define JOINT_TYPE_PHYSICS 2
// ...
#define JOINT_TYPE_RESERVED30 30

#define JOINT_TYPE_MAX_AMOUNT 31

#endif

