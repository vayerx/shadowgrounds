
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifdef _MSC_VER
#pragma warning ( disable: 4786 )
#pragma warning ( disable: 4103 )
#endif

// the new file to include,
// (in order to prevent getting too much command line defines)

// keep linkedlist nodes in pool, less dynamic memory allocations
#define LINKEDLIST_USE_NODE_POOL 1

// the new non-rts, pure action game - "crimson mode"
#define CRIMSON_MODE 1

// show rebuilding binaries loading message for buildingmaps
#ifndef LINUX
#define BUILDINGMAP_SHOW_LOADINGMESSAGE 1
#endif

// make buildingmap use game's options (force/auto recreate, etc.)
#define BUILDINGMAP_USE_OPTIONS 1

// allow script preprocessing - should be disabled for final release!!!
#define SCRIPT_PREPROCESS 1

#ifdef _DEBUG

// !!!!!!!!!!
// MEMORY MANAGER DISABLED BECAUSE NX LIBS WILL NOT COMPILE WITH IT!!!
// !!!!!!!!!!

  // enable memorymanager
  //#define FROZENBYTE_DEBUG_MEMORY 1

  // make memorymanager print extra data (string allocations' contents)
  #define FROZENBYTE_DEBUG_MEMORY_PRINT_DATA 1
	
	// old external script debug console (?)
	//#define SCRIPT_DEBUG 1

  // dump gamescene statistics to log (raytraces, pathfinds, etc.)
	#define DUMP_GAMESCENE_STATS 1

#endif

// make debug view map of pathfinding
// WARNING: greatly affects pathfind efficiency, under no 
// cicumstances leave this in the final product!!!
//#define PATHFIND_DEBUG 1

#ifdef PATHFIND_DEBUG
#pragma message("*** PATHFIND_DEBUG was defined, remove from release!!! ***")
#endif

// use unicode version of sg
//#define SHADOWGROUNDS_UNICODE 1

#define PROJECT_SHADOWGROUNDS 1

// use sg directory/file structure instead of the new one
#define LEGACY_FILES 1

#define VERBOSE_BUILD 1

// physics selection
//#define PHYSICS_NONE

//#define PHYSICS_ODE
//#define PHYSICS_ODE_BOTTOM_TRANSFORM

#if !defined(PHYSICS_NONE) && !defined(PHYSICS_ODE) && !defined(PHYSICS_PHYSX)
// PhysX on Shadowgrounds
#define PHYSICS_PHYSX
#endif

#include "../project_common/configuration_auto.h"

#endif

