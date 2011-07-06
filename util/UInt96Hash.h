
#ifndef UINT96HASH_H
#define UINT96HASH_H

#include "Debug_MemoryManager.h"

namespace util
{
	class UInt96Hash;

#pragma pack(push)
#pragma pack(4)
	struct hashable_uint_96
	{
		hashable_uint_96(__int64 higherValue, int lowerValue)
		{
			this->higherValue = higherValue;
			this->lowerValue = lowerValue;
		}

		unsigned __int64 higherValue;
		unsigned int lowerValue;
	};

	struct uint_96_hash_data
	{
		int size;
		unsigned char *buffer;

		uint_96_hash_data();
		uint_96_hash_data(int size, const unsigned char *buffer);

		uint_96_hash_data(const uint_96_hash_data &other);
		uint_96_hash_data& operator=(const uint_96_hash_data &other);

		~uint_96_hash_data()
		{
			if (buffer != 0)  // if (buffer != NULL)
				delete [] buffer;
		}

	};
#pragma pack(pop)

	class UInt96HashIterator
	{
	public:
		UInt96HashIterator();
		~UInt96HashIterator();
		bool next();
		hashable_uint_96 getKey();
		const uint_96_hash_data *getData();
	private:
		const unsigned char *block;
		int lowerBlockNumber;
		void *implIterator;
		UInt96Hash *hash;

		friend class UInt96Hash;
	};

	class UInt96HashImpl;

	/**
	 * A hashing implementation for 96 bit unsigned int keyed data.
	 * Particularly made with the AOV net game syncer / editor syncing in mind.
	 * In that case, lower values map to class ids, upper values to data ids.
	 *
	 * @author <jukka.kokkonen@postiloota.net>
	 */

	class UInt96Hash
	{
	public:
		// default values result in a hash buffer of (at most) about 12 megabytes in size.
		// (16 bytes for each key + maxBytesPerBlock) * upper blocks * lower blocks)
		// the 16 bytes come from upper id (8) + lower id (4) + reserved (4)
		UInt96Hash(int upperMaxBlockSize = 65536, int lowerMaxBlockSize = 4, int maxBytesPerBlock = 32);

		~UInt96Hash();

		// (the hash map takes ownership of the data, so don't delete it afterwards!)
		// note, zero key is an invalid value.
		void insertOrReplace(hashable_uint_96 key, uint_96_hash_data *data);

		const uint_96_hash_data peekData(hashable_uint_96 key);

		uint_96_hash_data popData(hashable_uint_96 key);

		void clear();

		UInt96HashIterator getIterator();

		void getState(int *bufferSize, const unsigned char **buffer);
		
		// (makes a copy of given buffer data, so does not take ownership of it)
		void setState(int bufferSize, const unsigned char *buffer);
		
		int getRecordAmount();

	private:
		UInt96HashImpl *impl;

		friend class UInt96HashIterator;
	};

}

#endif

