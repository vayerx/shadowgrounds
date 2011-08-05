
#ifndef SCRIPTDEV_CONFIGURATION_H
#define SCRIPTDEV_CONFIGURATION_H

#pragma warning ( disable: 4786 )
#pragma warning ( disable: 4103 )

// allow script preprocessing 
#define SCRIPT_PREPROCESS 1

#ifdef _DEBUG

  // enable memorymanager
  #define FROZENBYTE_DEBUG_MEMORY 1

  // make memorymanager print extra data (string allocations' contents)
  #define FROZENBYTE_DEBUG_MEMORY_PRINT_DATA 1

#endif

#endif
