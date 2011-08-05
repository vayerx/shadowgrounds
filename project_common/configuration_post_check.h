#ifndef CONFIGURATION_POST_CHECK_H
#define CONFIGURATION_POST_CHECK_H

#ifdef _MSC_VER

#ifdef PROJECT_SHADOWGROUNDS
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: PROJECT_SHADOWGROUNDS - Shadowgrounds")
	#endif
#elif PROJECT_ACTIONRPG
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: PROJECT_ACTIONRPG - Action RPG")
	#endif
#elif PROJECT_ARENASG
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: PROJECT_ARENASG - Arena SG")
	#endif
#elif PROJECT_AOV
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: PROJECT_AOV - Project name AOV")
	#endif
#elif PROJECT_SURVIVOR
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: PROJECT_SURVIVOR - SG Survivor")
	#endif
#elif PROJECT_CLAW_PROTO
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: PROJECT_CLAW_PROTO - Claw proto")
	#endif
#else
#pragma message("WARNING: Unknown project")
#endif



#ifdef LEGACY_FILES
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: LEGACY_FILES - Old directory/file structure")
	#endif
#elif NEW_FILES
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: NEW_FILES - New directory/file structure")
	#endif
#else
	#error "ERROR: No NEW_FILES or LEGACY_FILES defined."
#endif


#ifdef FB_TESTBUILD
	#ifdef _DEBUG
		#ifdef VERBOSE_BUILD
			#error "ERROR: Test build with _DEBUG defined."
		#endif
  #else
		#ifdef VERBOSE_BUILD
			#pragma message("VERBOSE_BUILD: FB_TESTBUILD - Test build")
		#endif
  #endif
#else
	#ifdef NDEBUG
		#ifdef VERBOSE_BUILD
			#pragma message("VERBOSE_BUILD: NDEBUG - Release build")
		#endif
	#else
		#ifdef VERBOSE_BUILD
			#pragma message("VERBOSE_BUILD: _DEBUG - Debug build")
		#endif
	#endif
#endif


#ifdef FROZENBYTE_DEBUG_MEMORY
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: FROZENBYTE_DEBUG_MEMORY - Memory debugging enabled")
	#endif
#endif


#ifdef NO_CRYPT
	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: NO_CRYPT - No data encryption")
	#endif
#else
  // TODO: check for some of the CRYPT_... flags

	#ifdef VERBOSE_BUILD
		#pragma message("VERBOSE_BUILD: CRYPT_... - Data encrypted")
	#endif
#endif

#endif


#endif

