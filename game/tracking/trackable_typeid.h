
#ifndef TRACKABLE_TYPEID_H
#define TRACKABLE_TYPEID_H

// note, trackable_typeid is a bit mask value 
// one TRACKABLE_TYPEID_DATATYPE may refer to multiple datatypes
// function parameters should generally be in plural if multiple types are expected.
// (i.e. getObjects(TRACKABLE_TYPEID_DATATYPE myType) versus getObjects(TRACKABLE_TYPEID_DATATYPE myTypes))

#define TRACKABLE_TYPEID_MAX_TYPES 30
#define TRACKABLE_TYPEID_ALL_TYPES_MASK 0x8fffffff
#define TRACKABLE_TYPEID_DATATYPE unsigned long

#endif
