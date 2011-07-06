
#include "precompiled.h"
#include "SimpleStringPool.h"
#include "../convert/str2int.h"

#include <string.h>

#include <assert.h>
#include <string>
#include <vector>

#include "Debug_MemoryManager.h"

// hold different allocation hole lists for this many different string sizes
// notice that the current correct size seek algorithm is O(n) in related to this value.
#define STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT 128
#define STRING_HOLE_ALLOCATION_SIZES_AMOUNT (STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT + 4)

// max size for the hole tracking vectors. (a kind of failsafe thingy..)
#define STRING_HOLE_VECTOR_MAX_SIZE 2048

// NOTE: these sizes should preferably be incremental 2^n sizes if large hole fragmenting
// to smaller ones is wanted...
static int string_hole_allocation_sizes[STRING_HOLE_ALLOCATION_SIZES_AMOUNT + 1] = 
{ 
	// these must be increments of one (up to ..._EXACT_AMOUNT)
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 
	70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 
	90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 
	100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 
	110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 
	120, 121, 122, 123, 124, 125, 126, 127, 

	// increments of 2^n good?
	//128,
	256,
	512,
	1024,
	2048,

	0x8fffffff
};

namespace util
{
	typedef int pool_string_pos;
	typedef int pool_string_size;

	class SimpleStringPoolImpl
	{
	public:
		SimpleStringPoolImpl()
		{
			// nop
			peakVectorSize = 0;
		}

		~SimpleStringPoolImpl()
		{
			// nop
		}

		std::vector<std::pair<pool_string_size, pool_string_pos> > allocationHoles[STRING_HOLE_ALLOCATION_SIZES_AMOUNT];
		int peakVectorSize;
	};



	SimpleStringPool::SimpleStringPool(int initialSize, ISimpleStringPoolManager *manager, bool fillHoles)
	{
		this->manager = manager;

		stringPoolStart = new char[initialSize + 1];
		memset(stringPoolStart, 0, initialSize);

		stringPoolStart[initialSize] = '\0';
		stringPoolEnd = &stringPoolStart[initialSize];
		assert(stringPoolEnd == stringPoolStart + initialSize);

		allocedStringPoolSize = initialSize;
		nextStringPoolPosition = 0;

		// add an empty string at the very beginning of the pool
		stringPoolStart[0] = '\0';
		nextStringPoolPosition = 1;

		this->fillHoles = fillHoles;

		this->impl = new SimpleStringPoolImpl();

		if (fillHoles)
		{
			// assuming string size to be on avg 20 chars, assuming 2% holes...
			int reserveSize = (initialSize / 20) / STRING_HOLE_ALLOCATION_SIZES_AMOUNT * 2 / 100;
			if (reserveSize < 4) reserveSize = 4;

			for (int i = 0; i < STRING_HOLE_ALLOCATION_SIZES_AMOUNT; i++)
			{
				impl->allocationHoles[i].reserve(reserveSize);
			}
		}

		// TODO: should check from 0 to STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT that they are actually increments of one
		assert(string_hole_allocation_sizes[STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT - 1] == STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT - 1);
		//assert(string_hole_allocation_sizes[STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT] == STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT);
	}


	SimpleStringPool::~SimpleStringPool()
	{
		this->clear();

		if (stringPoolStart != NULL)
		{
			delete [] stringPoolStart;
		}

		stringPoolStart = NULL;
		stringPoolEnd = NULL; // (should actually be excluded ptr, +1!)

		nextStringPoolPosition = 0;
		allocedStringPoolSize = 0;

		delete impl;
	}


	void SimpleStringPool::clear()
	{
#ifdef _DEBUG
		// make sure the pool is actually already clear.
		// (all "references" to pool have been removed)
		int nonZeroCount = 0;
		int nonZerosAt = -1;
		for (int i = 0; i < nextStringPoolPosition; i++)
		{
			if (stringPoolStart[i] != '\0')
			{
				nonZeroCount++;
				if (nonZerosAt == -1)
					nonZerosAt = i;
			}
		}
		assert(nonZeroCount == 0);
#endif

		memset(stringPoolStart, 0, nextStringPoolPosition);

		nextStringPoolPosition = 0;

		for (int i = 0; i < STRING_HOLE_ALLOCATION_SIZES_AMOUNT; i++)
		{
			impl->allocationHoles[i].clear();
		}

		impl->peakVectorSize = 0;

		// add an empty string at the very beginning of the pool
		stringPoolStart[0] = '\0';
		nextStringPoolPosition = 1;
	}


	const char *SimpleStringPool::addStringInPool(const char *str)
	{
		int slen = strlen(str);

		// empty string special case...
		if (slen == 0)
		{
			return stringPoolStart;
		}

		if (slen >= allocedStringPoolSize)
		{
			assert(!"SimpleStringPool::addStringInPool - Pool can only hold strings that are smaller than pool's current allocation size.");
			return NULL;
		}

		// TODO: the below check only puts small strings into small pool holes, when it could actually put
		// them in larger holes as well (by fragmenting a larger (n+1)^2 hole into two n^2 holes).
		// that however might encourage fragmenting the larger memory holes into tiny holes, which may
		// not be optimal in the end (it may be better to re-use the larger holes for future large strings adds)

		if (fillHoles)
		{

			if (slen < STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT)
			{
				// put it to a previous exact hole if one available...
				if (!impl->allocationHoles[slen].empty())
				{
					std::pair<pool_string_size,pool_string_pos> &tmpPair = impl->allocationHoles[slen][impl->allocationHoles[slen].size() - 1];
					assert(tmpPair.first == slen);
					int tmpPos = tmpPair.second;
					impl->allocationHoles[slen].pop_back();

					assert(tmpPos > 0 && tmpPos + slen < nextStringPoolPosition);
					char *mypos = &stringPoolStart[tmpPos];

#ifdef _DEBUG
					for (int i = 0; i < slen+1; i++)
					{
						assert(mypos[i] == '\0');
					}
#endif

					strcpy(mypos, str);
					return mypos;
				}
			}	else {		
				// these are not exactly matching sized holes, but large enough to fit into...
				for (int i = STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT; i < STRING_HOLE_ALLOCATION_SIZES_AMOUNT; i++)
				{
					if (slen < string_hole_allocation_sizes[i + 1])
					{
						if (!impl->allocationHoles[i].empty())
						{
							// put it to a previous hole...

							int vecSize = impl->allocationHoles[i].size();
							for (int j = 0; j < vecSize; j++)
							{
								std::pair<pool_string_size,pool_string_pos> &tmpPair = impl->allocationHoles[i][j];
								int tmpSize = tmpPair.first;
								int tmpPos = tmpPair.second;

								// allow shrinkage to 90%, leaving 10% unused memory fragment hole
								//if (tmpSize <= slen && tmpSize >= slen * 90 / 100)
								if (tmpSize == slen)
								{
									impl->allocationHoles[i].erase(impl->allocationHoles[i].begin() + j);

									assert(tmpPos > 0 && tmpPos + slen < nextStringPoolPosition);
									char *mypos = &stringPoolStart[tmpPos];

#ifdef _DEBUG
									for (int i = 0; i < slen+1; i++)
									{
										assert(mypos[i] == '\0');
									}
#endif
									strcpy(mypos, str);
									return mypos;
								}
							}

							/*
							int tmp = impl->allocationHoles[i][impl->allocationHoles[i].size() - 1];
							impl->allocationHoles[i].pop_back();

							assert(tmp > 0 && tmp + slen < nextStringPoolPosition);
							char *mypos = &stringPoolStart[tmp];
//#ifdef _DEBUG
							for (int j = 0; j < slen+1; j++)
							{
								assert(mypos[j] == '\0');
							}
//#endif
							strcpy(mypos, str);
							return mypos;
							*/
						} else {
							// no previous hole available for this string size.
							break;
						}
					}
				}
			}
		}

		// reallocate?
		if (nextStringPoolPosition + slen + 1 >= allocedStringPoolSize)
		{
			if (manager != NULL)
			{
				int previousSize = allocedStringPoolSize;
				int newSize = allocedStringPoolSize * 2;

				assert(newSize > nextStringPoolPosition + slen + 1);

				char *oldPoolStart = stringPoolStart;
				char *newPoolStart = new char[newSize + 1];

				memcpy(newPoolStart, oldPoolStart, previousSize);
				memset(&newPoolStart[previousSize], 0, newSize - previousSize);

				manager->poolBaseWillChange(oldPoolStart, newPoolStart);

				delete [] oldPoolStart;
				
				stringPoolStart = newPoolStart;
				stringPoolEnd = &newPoolStart[newSize];
				allocedStringPoolSize = newSize;

				manager->poolBaseHasChanged();

			} else {
				//assert(!"SimpleStringPool::addStringInPool - pool buffer full but no manager, so cannot resize.");
				return NULL;
			}
		}

		char *mypos = &stringPoolStart[nextStringPoolPosition];

		strcpy(mypos, str);
		nextStringPoolPosition += slen + 1;

		return mypos;
	}



	void SimpleStringPool::removeStringFromPool(const char *pooledStr)
	{
		//assert(!"SimpleStringPool::removeStringFromPool - currently, this is a add only pool implementation.");

		// NOTE: this does not actually free the slot in the pool!
		// it only sets it to zero filled string so that the emptiness of the pool can be checked later on at 
		// destruction.
		
		// empty string special case...
		if (pooledStr == stringPoolStart)
		{
			return;
		}

		assert(pooledStr > stringPoolStart);
		assert(pooledStr[-1] == '\0');

		assert(isStringInPool(pooledStr));
		int slen = strlen(pooledStr);

		if (fillHoles && slen > 0)
		{
			if (slen < STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT)
			{
				if (impl->allocationHoles[slen].size() < STRING_HOLE_VECTOR_MAX_SIZE)
				{
					impl->allocationHoles[slen].push_back(std::pair<pool_string_size, pool_string_pos>(slen, (int)(pooledStr - stringPoolStart)) );
				}
			} else {
				// find a non-exact hole list to fit into to.
				for (int i = STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT; i < STRING_HOLE_ALLOCATION_SIZES_AMOUNT; i++)
				{
					if (slen < string_hole_allocation_sizes[i + 1])
					{
						/*
						// NOTE: since the string may not have a full 2^n buffer, making the slot available as a smaller slot...
						// TODO: could check for special case where the string is exactly the 2^n (-1?) size and there is no 
						// need to make the slot available as a smaller one.
						//if (i >= 1)
						//{
							if (impl->allocationHoles[i - 1].size() < STRING_HOLE_VECTOR_MAX_SIZE)
							{
								impl->allocationHoles[i - 1].push_back((int)(pooledStr - stringPoolStart));
#ifdef _DEBUG
								if (impl->allocationHoles[i - 1].size() > impl->peakVectorSize)
								{
									impl->peakVectorSize = impl->allocationHoles[i - 1].size();
								}
#endif
							}
						//}
						*/
						if (impl->allocationHoles[i].size() < STRING_HOLE_VECTOR_MAX_SIZE)
						{
							impl->allocationHoles[i].push_back(std::pair<pool_string_size, pool_string_pos>(slen, (int)(pooledStr - stringPoolStart)));
#ifdef _DEBUG
							if ((int)impl->allocationHoles[i].size() > impl->peakVectorSize)
							{
								impl->peakVectorSize = impl->allocationHoles[i].size();
							}
#endif
						}
						break;
					}
				}
			}
		}


		assert(pooledStr + slen <= stringPoolEnd);

		// oopsie, the parameter seems to be const (which in a way could be considered true, but not really ;)
		memset((char *)pooledStr, '\0', slen);
	}


	char *SimpleStringPool::getStatusInfo()
	{
		int nonZeroCount = 0;
		int consecutiveZeroCount = 0;
		for (int i = 0; i < nextStringPoolPosition; i++)
		{
			if (stringPoolStart[i] != '\0')
			{
				nonZeroCount++;
			} else {
				if (i > 0 && stringPoolStart[i - 1] == '\0')
				{
					consecutiveZeroCount++;
				}
			}
		}
		// (note, this is also pretty much the string count - not counting the empty string)
		int singleZeroCount = nextStringPoolPosition - nonZeroCount - consecutiveZeroCount;
		assert(singleZeroCount >= 0);

		int stringsInPool = singleZeroCount;

		assert(consecutiveZeroCount <= nextStringPoolPosition);

		std::string tmp = "SimpleStringPool:\r\n";
		tmp += std::string("Pool fill percentage: ") + int2str(100 * nextStringPoolPosition / allocedStringPoolSize) +  "%\r\n";
		// NOTE: this non-empty may be off by one (still counting all empty strings as a single instance)
		tmp += std::string("Non-empty strings in pool: ") + int2str(stringsInPool) +  "\r\n";
		tmp += std::string("Allocated memory: ") + int2str(allocedStringPoolSize) +  "\r\n";
		
		// (result may be larger than actual?)
		tmp += std::string("Avg string length: ") + int2str(nonZeroCount / stringsInPool) + std::string(".");

		tmp += std::string(int2str(((nonZeroCount * 10) / stringsInPool) % 10)) + "\r\n";
		tmp += std::string("Memory wasted in pool holes: ") + int2str(consecutiveZeroCount) + std::string(" (");
		tmp += std::string(int2str(100 * consecutiveZeroCount / nextStringPoolPosition)) + "%)\r\n";

		int exactStringHoleCaches = 0;
		int largeStringHoleCaches = 0;
		tmp += std::string("Small hole caches: ");
		for (int i = 0; i < STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT; i++)
		{
			exactStringHoleCaches += impl->allocationHoles[i].size();
			tmp += std::string(int2str(impl->allocationHoles[i].size())) + ",";
		}
		tmp += std::string("\r\n");
		tmp += std::string("Large hole caches: ");
		for (int i = STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT; i < STRING_HOLE_ALLOCATION_SIZES_AMOUNT; i++)
		{
			largeStringHoleCaches += impl->allocationHoles[i].size();
			tmp += std::string(int2str(impl->allocationHoles[i].size())) + ",";
		}
		tmp += std::string("\r\n");
		tmp += std::string("Avg exact small string holes in cache per size: ") + int2str(exactStringHoleCaches / STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT) + std::string("\r\n");
		tmp += std::string("Avg large string holes in cache per size: ") + int2str(largeStringHoleCaches / (STRING_HOLE_ALLOCATION_SIZES_AMOUNT - STRING_HOLE_ALLOCATION_SIZES_EXACT_AMOUNT)) + std::string("\r\n");
#ifdef _DEBUG
		tmp += std::string("Peak hole cache size (for a single size): ") + int2str(impl->peakVectorSize) + std::string("\r\n");
#endif

		char *ret = new char[tmp.size() + 1];
		strcpy(ret, tmp.c_str());
		return ret;
	}

}
