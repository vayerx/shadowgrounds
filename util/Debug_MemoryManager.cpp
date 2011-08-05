// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "Debug_MemoryManager.h"

#ifdef FROZENBYTE_DEBUG_MEMORY

#include <new>
#include <cassert>
#include <cstdio>

#ifdef WIN32
#include <windows.h>
#endif

#include <malloc.h>
#include <string.h>

//#include "assert.h"


/*

C'ish mess. Can`t really help it since using new/delete inside
allocation routines would be, well .. no fun. This excludes 
STL containers too, damn.

Some placement new tricks might have worked but ..

*/

// Don't use this here.
#ifdef new
#undef new
#endif

#ifdef _MSC_VER
//#define for if(false) {} else for
#endif

using namespace frozenbyte::debug;

namespace frozenbyte {
namespace debug {
namespace {

	// Filename where to dump leak report. Ok, not so pretty but sharing file is good
#ifdef STORM3DV2_EXPORTS
	#ifdef LEGACY_FILES
		const char logFileName[] = "StormMemoryLeaks.txt";
	#else
		const char logFileName[] = "logs/StormMemoryLeaks.txt";
	#endif
#else
	#ifdef LEGACY_FILES
		const char logFileName[] = "MemoryLeaks.txt";
	#else
		const char logFileName[] = "logs/MemoryLeaks.txt";
	#endif
#endif

	// Longs are guaranteed to be 32 bits (afaik)
	typedef unsigned long uint32;

	// Identifiers which are placed to allocated buffer (4-byte alignment)
	const uint32 memPrefix = 0xBAADF00D;
	const uint32 memPostfix = 0xBABE2BED;
	const uint32 memNotUsed = 0xDEADC0DE;

	// Identifiers for array / non array allocations / deleted allocations
	const uint32 nonArrayAllocation = 0x2BADF00D;
	const uint32 arrayAllocation = 0xBAD4ACE2;
	const uint32 invalidAllocation = 0x76543210;

	// Amounts. Be carefull, can be a memory overkill
	const int numberPrefix = 32; // 128 bytes
	const int numberPostfix = 32; // 128 bytes

	void removeMessages()
	{
#ifdef WIN32
		MSG msg = { 0 };
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_PAINT)
				return;
		}
#endif
	}

	struct AllocationUnit
	{
		// For convenience
		uint32 *prefixPointer;
		uint32 *postfixPointer;
		uint32 *dataPointer;

		// Size w/ and w/o manager extras
		size_t requestedSize;
		size_t overallSize;
	
		// Catches mixing new[]/delete and new/delete[]
    // changed from bool to int to catch problems with memory blocks
    // allocated without using memory manager.
		int arrayAllocated;

		// Allocation info which may or may not be present
		char *allocatedFrom;

		// Allocation was marked during last snapshot
		// therefore, it will not be shown at leak snapshot dump
		bool markedSnapshot;
	};

	AllocationUnit *createAllocationUnit()
	{
		AllocationUnit *unit = static_cast<AllocationUnit *> (malloc(sizeof(AllocationUnit)));
		
		unit->prefixPointer = 0;
		unit->postfixPointer = 0;
		unit->dataPointer = 0;
	
		unit->requestedSize = 0;
		unit->overallSize = 0;

		unit->arrayAllocated = nonArrayAllocation;
		unit->allocatedFrom = 0;

		unit->markedSnapshot = false;
		
		return unit;
	}

	void deleteAllocationUnit(AllocationUnit *unit)
	{
		if(unit->allocatedFrom)
			free(unit->allocatedFrom);
		if(unit->prefixPointer)
			free(unit->prefixPointer);
    unit->arrayAllocated = invalidAllocation;
		free(unit);
	}
	
	/* 
	  Here are our allocation infos. Finally implemented with hashing.
	*/

	struct AllocationLink
	{
		AllocationUnit *allocationUnit;
		AllocationLink *next;
	};

	struct AllocationRoot
	{
		AllocationLink *first;
	};
	
	// Hash data
	static const int hashSize = 3677; // Prime number. Big enough?
	static AllocationRoot hashMap[hashSize] = { { 0 } };
	
	// Amount of allocation
	static int allocationCount = 0;
	// Memory allocated
	static int allocationMemory = 0;

	static int peakMemoryUsage = 0;
	static int peakPointers = 0;
	
	int calculateHashIndex(const void *buffer)
	{
		int value = reinterpret_cast<int> (buffer);
		
		// Shift lower bits (alignment would kill coverage)
		value >>= 4;

		// Create index
		value %= hashSize;
		return value;
	}
	
	void addAllocation(AllocationUnit *allocation)
	{
		assert(allocation);

		++allocationCount;
		allocationMemory += allocation->requestedSize;
		
		AllocationLink *link = static_cast<AllocationLink *> (malloc(sizeof(AllocationLink)));
		link->allocationUnit = allocation;
		link->next = 0;

		int hashIndex = calculateHashIndex(allocation->dataPointer);
		if(hashMap[hashIndex].first == 0)
			hashMap[hashIndex].first = link;
		else
		{
			// Push front
			link->next = hashMap[hashIndex].first;
			hashMap[hashIndex].first = link;
		}

		if(allocationMemory > peakMemoryUsage)
			peakMemoryUsage = allocationMemory;
		if(allocationCount > peakPointers)
			peakPointers = allocationCount;
	}
	
	AllocationUnit *findAllocation(void *pointer)
	{
		int hashIndex = calculateHashIndex(pointer);		
		AllocationLink *current = hashMap[hashIndex].first;

		while(current)
		{
			if(current->allocationUnit->dataPointer == pointer)
				return current->allocationUnit;

			current = current->next;
		}

		removeMessages();
		assert(!"Allocation not found. Unintialized pointer?");
		return 0;
	}

	void removeAllocation(AllocationUnit *allocation)
	{
		if(allocationCount <= 0)
		{
			removeMessages();
			assert(allocationCount > 0);
		}

		int hashIndex = calculateHashIndex(allocation->dataPointer);

		AllocationLink *current = hashMap[hashIndex].first;
		AllocationLink *previous = 0;

		while(current)
		{
			if(current->allocationUnit == allocation)
			{
				// Remove
				if(previous)
					previous->next = current->next;
				else
					hashMap[hashIndex].first = current->next;

				--allocationCount;
				allocationMemory -= current->allocationUnit->requestedSize;

				// Free memory
				deleteAllocationUnit(current->allocationUnit);
				free(current);
		
				return;
			}

			previous = current;
			current = current->next;
		}

		removeMessages();
		assert(!"Allocation not found. Unintialized pointer?");
	}

	void dumpLeakReport()
	{
		if(allocationCount > 0)
		{
			dumpLeakSnapshot(true);
		}
		else
		{
			// Remove file
			fclose(fopen(logFileName, "wt"));
		}
	}

	void testIdentifiers(AllocationUnit *allocation)
	{
		for(int i = 0; i < numberPrefix; ++i)
		{
			if(allocation->prefixPointer[i] != memPrefix)
			{
				removeMessages();
				assert(!"Buffer prefix messed up!");
			}
		}
		for(int i = 0; i < numberPostfix; ++i)
		{
			if(allocation->postfixPointer[i] != memPostfix)
			{
				removeMessages();
				assert(!"Buffer postfix messed up!");
			}
		}
	}

	// After deinitialization, dump leak report on every deallocation
	struct InitializationTracker
	{
		static bool programExiting;

		InitializationTracker()
		{
			programExiting = false;
		}
		~InitializationTracker()
		{	
			programExiting = true; 
			dumpLeakReport();
		}
	};
	
	bool InitializationTracker::programExiting = false;
	static InitializationTracker tracker;

} // end of nameless namespace


	void markLeakSnapshot()
	{
		if(allocationCount > 0)
		{
			for(int i = 0; i < hashSize; ++i)
			{
				AllocationLink *currentLink = hashMap[i].first;
				while(currentLink != 0)
				{
					currentLink->allocationUnit->markedSnapshot = true;
					currentLink = currentLink->next;
				}
			}
		}
	}

	void dumpLeakSnapshot(bool fromStart)
	{
		if(allocationCount > 0)
		{
			FILE *fp = fopen(logFileName, "wt");
			if (fp == NULL)
			{
				return;
			}

			if (!fromStart)
				fprintf(fp, "(SNAPSHOT)\n\n");

			fprintf(fp, "Peak memory usage: %d bytes\n", peakMemoryUsage);
			fprintf(fp, "Overall memory leaked: %d bytes\n", allocationMemory);
			fprintf(fp, "Pointers left: %d\n\n", allocationCount);
			fprintf(fp, "Note - Assumes ??? allocations to be from STLport/etc. Those are not shown anymore.\n\n");

			int currentIndex = 0;
			for(int i = 0; i < hashSize; ++i)
			{
				AllocationLink *currentLink = hashMap[i].first;
				while(currentLink != 0)
				{
					if (!currentLink->allocationUnit->markedSnapshot
						|| fromStart)
					{
						//if(strcmp(currentLink->allocationUnit->allocatedFrom, "(???: line 0)") != 0)
						if(!strstr(currentLink->allocationUnit->allocatedFrom, "???"))
						{
// TEMP: show only over 2 MB
//if (currentLink->allocationUnit->requestedSize > 1*1024*1024)
//{
							fprintf(fp, "Allocation %d:\n", ++currentIndex);
							fprintf(fp, "\tAllocated from: %s\n", currentLink->allocationUnit->allocatedFrom);
							fprintf(fp, "\tAllocation size: %d bytes\n", currentLink->allocationUnit->requestedSize);
							if(currentLink->allocationUnit->arrayAllocated == nonArrayAllocation)
								fprintf(fp, "\tAllocated with new()\n");
							else
								fprintf(fp, "\tAllocated with new[]\n");
						
							// to get the contents of some char array strings - jpk
	#ifdef FROZENBYTE_DEBUG_MEMORY_PRINT_DATA
							#define DEBUG_MEMORYMANAGER_MAX_PRINT_SIZE 80

							int arraySize = currentLink->allocationUnit->requestedSize;
							if (currentLink->allocationUnit->arrayAllocated == arrayAllocation
								&& arraySize < DEBUG_MEMORYMANAGER_MAX_PRINT_SIZE)
							{
								char *data = (char *)currentLink->allocationUnit->dataPointer;
								char databuf[DEBUG_MEMORYMANAGER_MAX_PRINT_SIZE + 2];
								bool noControlChars = true;
								int j;
								for (j = 0; j < arraySize; j++)
								{
									if (data[j] == '\n' || data[j] == '\r')
										databuf[j] = ' ';
									else
										databuf[j] = data[j];
									if (data[j] < 32 && data[j] != '\n' && data[j] != '\r')
									{
										if (data[j] != '\0') noControlChars = false;
										break;
									}
								}
								databuf[j] = '\0';
								if (noControlChars)
								{
									fprintf(fp, "\tData: \"%s\"\n", data);
								}
							}
	#endif
							fprintf(fp, "\n");
//}
						}
					}

					currentLink = currentLink->next;
				}
			}

			fclose(fp);
		}		
	}


char debug_allocInfo[256 + 1] = { 0 };
int debug_allocedSinceInfo = -1;
// just a hack to add extra info to allocations
void debugSetAllocationInfo(const char *allocationInfo)
{
  if (allocationInfo == NULL)
		debug_allocInfo[0] = '\0';
	else
	  strncpy(debug_allocInfo, allocationInfo, 256);
	debug_allocedSinceInfo = 0;
}


} // end of namespace debug
} // end of namespace frozenbyte


// operator new implementation as suggested by Meyers on Effective C++ (item 8)
void *operator new(size_t originalSize, const char *fileName, int lineNumber, bool arrayAllocated)
{
	// Handle 0-byte request. The Holy Standard says we must 
	// return unique pointer (or unique value actually)
	if(originalSize == 0)
		originalSize = 1;

	// To 4-byte boundary (since our identifiers are uint32`s)
	if(int foo = originalSize % 4)
		originalSize += 4 - foo;

	// Make room for prefix & postfix
	size_t size = originalSize;
	size += frozenbyte::debug::numberPrefix * 4;
	size += frozenbyte::debug::numberPostfix * 4;

	// Yes. Infinite loop really is the way to go ;-)
	while(true)
	{
		frozenbyte::debug::AllocationUnit *allocation = frozenbyte::debug::createAllocationUnit();
		void *buffer = malloc(size);

		// Both have to succeed. We want to handle out-of-memory at least
		// on our mighty memory manager >;)
		if((buffer) && (allocation))
		{
			char *info;
			if (frozenbyte::debug::debug_allocInfo[0] != '\0' && frozenbyte::debug::debug_allocedSinceInfo >= 0)
			{
				info = static_cast<char *> (malloc(strlen(fileName) + strlen(frozenbyte::debug::debug_allocInfo) + 60));
				if(info)
				{
					if (frozenbyte::debug::debug_allocedSinceInfo == 0)
						sprintf(info, "(%s: line %d)\n\tInfo: \"%s\"", fileName, lineNumber, frozenbyte::debug::debug_allocInfo);
					else
						sprintf(info, "(%s: line %d)\n\tInfo: (\"%s\", %d allocs ago)", fileName, lineNumber, frozenbyte::debug::debug_allocInfo, frozenbyte::debug::debug_allocedSinceInfo);
					frozenbyte::debug::debug_allocedSinceInfo++;
				}
			} else {
				info = static_cast<char *> (malloc(strlen(fileName) + 20));
				if(info)
				{
					sprintf(info, "(%s: line %d)", fileName, lineNumber);
				}
			}

			// Fill in allocation info
			allocation->prefixPointer = static_cast<frozenbyte::debug::uint32 *> (buffer);
			allocation->dataPointer = allocation->prefixPointer + frozenbyte::debug::numberPrefix;
			allocation->postfixPointer = allocation->dataPointer + (originalSize / 4);
			
			allocation->allocatedFrom = info;
			if (arrayAllocated)
  				allocation->arrayAllocated = frozenbyte::debug::arrayAllocation;
			else
  				allocation->arrayAllocated = frozenbyte::debug::nonArrayAllocation;
			allocation->overallSize = size;
			allocation->requestedSize = originalSize;

			// Fill in our identifiers
			for(int i = 0; i < frozenbyte::debug::numberPrefix; ++i)
				allocation->prefixPointer[i] = frozenbyte::debug::memPrefix;
			for(int i = 0; i < int(originalSize / 4); ++i)
				allocation->dataPointer[i] = frozenbyte::debug::memNotUsed;
			for(int i = 0; i < frozenbyte::debug::numberPostfix; ++i)
				allocation->postfixPointer[i] = frozenbyte::debug::memPostfix;

			frozenbyte::debug::addAllocation(allocation);
			return allocation->dataPointer;
		}

		// If only one of them succeeded, free it first
		if(buffer)
			free(buffer);
		if(allocation)
			frozenbyte::debug::deleteAllocationUnit(allocation);

		// Test error-handling function
		std::new_handler global_handler = std::set_new_handler(0);
		std::set_new_handler(global_handler);

		// If has one, try it. Otherwise throw bad_alloc 
		// (and hope for someone to catch <g>)
		if(global_handler)
			(*global_handler) ();
		else
			throw std::bad_alloc();
	}
}

void operator delete(void *buffer, bool arrayDeleted) throw()
{
	// Deleting null-pointer is legal
	if(buffer == 0)
		return;

	frozenbyte::debug::AllocationUnit *allocation = frozenbyte::debug::findAllocation(buffer);
	if(!allocation)
	{
		removeMessages();
		assert(allocation);
	}

	// Test out-of-bounds
	frozenbyte::debug::testIdentifiers(allocation);

  // Test that the block was allocated by memory manager,
	// Test array operator mixing
  if (allocation->arrayAllocated != frozenbyte::debug::arrayAllocation
    && allocation->arrayAllocated != frozenbyte::debug::nonArrayAllocation)
  {
	removeMessages();
    assert(!"Deleting block with invalid allocation type");
  } else {
    if ((arrayDeleted && allocation->arrayAllocated == frozenbyte::debug::nonArrayAllocation)
      || (!arrayDeleted && allocation->arrayAllocated == frozenbyte::debug::arrayAllocation))
    {
		removeMessages();
		assert(!"Mixed array and normal versions");
    }
  }

	frozenbyte::debug::removeAllocation(allocation);

	// If quitting, dump report on each deallocation
	if(frozenbyte::debug::InitializationTracker::programExiting == true)
		frozenbyte::debug::dumpLeakReport();
}


/*
  These get called from client code
*/
void *operator new(size_t size, const char *fileName, int lineNumber) throw(std::bad_alloc)
{
	return operator new(size, fileName, lineNumber, false);
}
void *operator new(size_t size) throw(std::bad_alloc)
{
	return operator new(size, "???", 0, false);
}
void *operator new[](size_t size, const char *fileName, int lineNumber) throw(std::bad_alloc)
{
	return operator new(size, fileName, lineNumber, true);
}
void *operator new[](size_t size) throw(std::bad_alloc)
{
	return operator new(size, "???", 0, true);
}

void operator delete(void *buffer) throw()
{
	operator delete(buffer, false);
}
void operator delete[](void *buffer) throw()
{
	operator delete(buffer, true);
}

/*
  Utility functions
*/

namespace frozenbyte {
namespace debug {

// Affect behavior
void Debug_MemoryManager::setFailingPercentage(int percentage)
{
}

// Pointers
void Debug_MemoryManager::validatePointer(void *pointer)
{
	// Try to find
	frozenbyte::debug::AllocationUnit *allocation = frozenbyte::debug::findAllocation(pointer);
	if(!allocation)
	{
		removeMessages();
		assert(allocation);

		return;
	}

	// Test out-of-bounds
	frozenbyte::debug::testIdentifiers(allocation);
}

void Debug_MemoryManager::validateAllPointers()
{
	for(int i = 0; i < hashSize; ++i)
	{
		AllocationLink *currentLink = hashMap[i].first;
		while(currentLink != 0)
		{
			if(currentLink)
				testIdentifiers(currentLink->allocationUnit);

			currentLink = currentLink->next;
		}
	}
}

int Debug_MemoryManager::amountMemoryAllocated(void *pointer, bool includeManagerExtra)
{
	return 0;
}

int Debug_MemoryManager::amountMemoryInUse(void *pointer)
{
	int result = 0;

	for(int i = 0; i < hashSize; ++i)
	{
		AllocationLink *currentLink = hashMap[i].first;
		while(currentLink != 0)
		{
			if(currentLink)
				result += currentLink->allocationUnit->requestedSize;

			currentLink = currentLink->next;
		}
	}

	return result;
}

// Logging
void Debug_MemoryManager::logStatistics(const char *fileName)
{
}

void Debug_MemoryManager::logUnusedPointers(const char *fileName, float freePercentage)
{
}

// Memory statistics
int Debug_MemoryManager::amountMemoryInUse(bool includeManagerExtra)
{
	return frozenbyte::debug::allocationMemory;
}

int Debug_MemoryManager::amountPeakMemoryInUse(bool includeManagerExtra)
{
	return 0;
}

int Debug_MemoryManager::amountMemoryAllocations()
{
	return frozenbyte::debug::allocationCount;
}

int Debug_MemoryManager::amountPeakMemoryAllocations()
{
	return 0;
}

} // end of namespace debug
} // end of namespace frozenbyte

#endif // FROZENBYTE_DEBUG_MEMORY
