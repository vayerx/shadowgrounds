
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifndef __GNUC__
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
//#define BUILDINGMAP_SHOW_LOADINGMESSAGE 1

// make buildingmap use game's options (force/auto recreate, etc.)
#define BUILDINGMAP_USE_OPTIONS 1

// allow script preprocessing - should be disabled for final release!!!
#define SCRIPT_PREPROCESS 1

#ifdef _DEBUG

#ifndef __GNUC__
  // enable memorymanager
  #define FROZENBYTE_DEBUG_MEMORY 1

  // make memorymanager print extra data (string allocations' contents)
  #define FROZENBYTE_DEBUG_MEMORY_PRINT_DATA 1
#endif
	
	// old external script debug console (?)
	//#define SCRIPT_DEBUG 1

  // dump gamescene statistics to log (raytraces, pathfinds, etc.)
	#define DUMP_GAMESCENE_STATS 1

#endif

// make debug view map of pathfinding
// WARNING: greatly affects pathfind efficiency, under no 
// cicumstances leave this in the final product!!!
//#define PATHFIND_DEBUG 1

// use unicode version of sg
//#define SHADOWGROUNDS_UNICODE 1

#define PROJECT_SURVIVOR 1

// physics selection
//#define PHYSICS_NONE
//#define PHYSICS_ODE
//#define PHYSICS_ODE_BOTTOM_TRANSFORM
#if !defined(PHYSICS_NONE) && !defined(PHYSICS_ODE) && !defined(PHYSICS_PHYSX)
#define PHYSICS_PHYSX
#endif

// use sg directory/files structure instead of the new one
#define LEGACY_FILES 1

#define VERBOSE_BUILD 1

#include "../project_common/configuration_auto.h"

// light amount
#ifdef PROJECT_SURVIVOR

#define LIGHT_MAX_AMOUNT 5

#endif

#endif

