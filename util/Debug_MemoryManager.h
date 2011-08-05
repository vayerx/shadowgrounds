#ifndef INCLUDED_DEBUG_MEMORYMANAGER_H
#define INCLUDED_DEBUG_MEMORYMANAGER_H

#pragma once

#if !defined(NDEBUG) && !defined(FROZENBYTE_DEBUG_MEMORY)
//#define FROZENBYTE_DEBUG_MEMORY
#endif

#ifdef FROZENBYTE_DEBUG_MEMORY // Define this to enable tracking

/*
Debug versions of C++ heap memory handling operators. Using is 
transparent to client code. Note that this system might increase 
memory footprint considerably if you are allocating lots of small chunks.

This header doesn`t have to be included from client code. Operators
will be overrided anyway (on linking), this 'just' gives ability to
store allocations file / line number.
Should also be used on headers which do dynamic memory management. 
Just remember to #undef new to avoid breaking things later on.

Note that MSVC doesn't separate arrayed versions unless this header
is included from code.

This does the following:
	- Keeps list of all occured allocations
		-> Reports if forgot to delete some block (on exit)
		-> Reports if tried to delete unallocated block
	- Keeps some space around allocated buffer
		-> Detects most out-of-bounds writes
			-> Uses 4-byte boundary so with chars minor errors go undetected
		-> Checking done only on deallocation, can`t tell where it happened.
			-> Use validatePointer() on tricky situations
	- Notices if one tries to delete new[]`ed memory or vice versa
		-> Mixing array new/delete to normal versions, that is 
		-> At least on MSVC, requires including this header thought
			-> GCC works fine without it. Surprise, surprise
	- Reports has the information of where buffer was allocated
		-> Works only if included this header, though
		-> Filename & line number where it was allocated
   - Obeys all memory handling semantics with one exception
		-> Does not define a throw() version which would return 0-pointer on out-of-memory situations.
		-> That`s legacy anyway
	- Ability to query some useful statistics
		-> ToDo but see below
		
Started coding this on my own but later on found Paul Nettle's manager. 
Some ideas from there.
	-- psd
*/

#ifndef INCLUDED_NEW
#define INCLUDED_NEW
#include <new>
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4290) // Exception specification ignored. MSC uses legacy system anyway
#pragma warning(disable: 4291) // Whining too much about this, so let's just disable it ;)
#endif

namespace frozenbyte {
namespace debug {

// Utility functions. These are ToDo.
// Should define dummies for release builds?
class Debug_MemoryManager
{
	// Not implemented
	Debug_MemoryManager();
	~Debug_MemoryManager();

public:
	// Affect behavior
	static void setFailingPercentage(int percentage);

	// Pointers
	static void validatePointer(void *pointer);
	static void validateAllPointers();
	static int amountMemoryAllocated(void *pointer, bool includeManagerExtra = false);
	static int amountMemoryInUse(void *pointer);

	// Logging (leaks reporter automaticly)
	static void logStatistics(const char *fileName);
	static void logUnusedPointers(const char *fileName, float freePercentage);

	// Memory statistics
	static int amountMemoryInUse(bool includeManagerExtra = false);
	static int amountPeakMemoryInUse(bool includeManagerExtra = false);
	static int amountMemoryAllocations();
	static int amountPeakMemoryAllocations();
};

// a quick hack to get extra info about allocations -jpk
void debugSetAllocationInfo(const char *allocationInfo);
void dumpLeakSnapshot(bool fromStart = false);
void markLeakSnapshot();

} // end of namespace debug
} // end of namespace frozenbyte

// Global operators
void *operator new(size_t size, const char *fileName, int lineNumber) throw(std::bad_alloc);
void *operator new(size_t size) throw(std::bad_alloc);
void *operator new[](size_t size, const char *fileName, int lineNumber)  throw(std::bad_alloc);
void *operator new[](size_t size) throw(std::bad_alloc);
void operator delete(void *buffer) throw();
void operator delete[](void *buffer) throw();

// For some compilers. Are there any which doesn't define thsee?
#ifndef __FILE__
#define __FILE__ "???"
#endif
#ifndef __LINE__
#define __LINE__ 0
#endif

#endif // #ifdef FROZENBYTE_DEBUG_MEMORY
#endif

// Overwrite these multiple times if wanted

#ifdef FROZENBYTE_DEBUG_MEMORY

// The Macro(tm). There's no way to add info on delete. Wouldn't do much good anyway
//#define FROZENBYTE_DEBUG_NEW new(__FILE__, __LINE__)
//#define new FROZENBYTE_DEBUG_NEW

#endif // #ifdef FROZENBYTE_DEBUG_MEMORY
