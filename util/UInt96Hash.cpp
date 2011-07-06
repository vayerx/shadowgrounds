
#include "precompiled.h"

#include "UInt96Hash.h"

#include <assert.h>
#include <stdlib.h>
#include <hash_map>

#include "Debug_MemoryManager.h"

namespace util
{
	// this should be 16 bytes in size?
#pragma pack(push)
#pragma pack(4)
	struct hash_block_header
	{
		hashable_uint_96 key; 
		int reserved;

		hash_block_header(const hashable_uint_96 &key_)
			: key(key_)
		{
			this->reserved = 0;
		}
	};
#pragma pack(pop)

	typedef std::hash_map<__int64, unsigned char *> UpperHashType;
	typedef std::pair<__int64, unsigned char *> UpperHashPairType;


	uint_96_hash_data::uint_96_hash_data()
	{ 
		this->size = 0;
		this->buffer = NULL;
	}


	uint_96_hash_data::uint_96_hash_data(int size, const unsigned char *buffer)
	{ 
		this->size = size;
		assert(buffer != NULL);
		this->buffer = new unsigned char[size];
		memcpy(this->buffer, buffer, size);
	}

	uint_96_hash_data::uint_96_hash_data(const uint_96_hash_data &other)
	{
		this->size = other.size;
		assert(other.size == 0 || other.buffer != NULL);
		this->buffer = new unsigned char[size];
		memcpy(this->buffer, other.buffer, size);
	}

	uint_96_hash_data& uint_96_hash_data::operator=(const uint_96_hash_data &other)
	{
		this->size = other.size;
		assert(other.buffer != NULL);
		this->buffer = new unsigned char[size];
		memcpy(this->buffer, other.buffer, size);
		return *this;
	}


	class UInt96HashImpl
	{
	public:
		UInt96HashImpl(int upperMaxBlockSize = 65536, int lowerMaxBlockSize = 4, int maxBytesPerBlock = 32)
		{
			this->upperMaxBlockSize = upperMaxBlockSize;
			this->lowerMaxBlockSize = lowerMaxBlockSize;
			this->maxUserBytesPerBlock = maxBytesPerBlock;
			this->actualBlockSize = maxUserBytesPerBlock + sizeof(hash_block_header);
			this->lowerBufferSize = actualBlockSize * lowerMaxBlockSize;
			assert(sizeof(hash_block_header) == 16);
			this->recordAmount = 0;
			this->upperRecordAmount = 0;
			this->lastGetStateBuffer = NULL;
			this->lastGetStateBufferSize = 0;
		}

		int upperMaxBlockSize;
		int lowerMaxBlockSize;
		int maxUserBytesPerBlock;
		int actualBlockSize;
		int lowerBufferSize;

		int recordAmount;
		int upperRecordAmount;

		unsigned char *lastGetStateBuffer;
		int lastGetStateBufferSize;

		UpperHashType upperHash;

	};


	UInt96Hash::UInt96Hash(int upperMaxBlockSize, int lowerMaxBlockSize, int maxBytesPerBlock)
	{
		this->impl = new UInt96HashImpl(upperMaxBlockSize, lowerMaxBlockSize, maxBytesPerBlock);
	}

	UInt96Hash::~UInt96Hash()
	{
		clear();
		delete impl;
	}

	// (the hash map takes ownership of the data, so don't delete it afterwards!)
	// note, zero key is an invalid value.
	void UInt96Hash::insertOrReplace(hashable_uint_96 key, uint_96_hash_data *data)
	{
		assert(key.higherValue != 0 || key.lowerValue != 0);

		assert(data->size < impl->maxUserBytesPerBlock);

		// if existing upper level record exists, use that...
		UpperHashType::iterator iter = impl->upperHash.find(key.higherValue);
		if (iter != impl->upperHash.end())
		{
			// upper level record existed. see if lower level exists as well...
			unsigned char *buf = iter->second;
			int emptyLowerSlot = -1;
			for (int i = 0; i < impl->lowerMaxBlockSize; i++)
			{
				unsigned char *block = &buf[i * impl->actualBlockSize];
				unsigned int blockLowerValue = ((hash_block_header *)block)->key.lowerValue;
				assert(((hash_block_header *)block)->reserved == 0);
				if (blockLowerValue == key.lowerValue)
				{
					// found it, the record already exists!
					// overwrite it.
					// header should remain unchanged?
					//hash_block_header header(key);
					//memcpy(&buf[(emptyLowerSlot * impl->actualBlockSize)], &header, sizeof(hash_block_header));
					// copy user data of the first lower record
					memcpy(&buf[(i * impl->actualBlockSize) + sizeof(hash_block_header)], &data->size, sizeof(int));
					memcpy(&buf[(i * impl->actualBlockSize) + sizeof(hash_block_header) + sizeof(int)], data->buffer, data->size);

					return;
				} else {
					if (blockLowerValue == 0)
					{
						unsigned __int64 blockHigherValue = ((hash_block_header *)block)->key.higherValue;
						if (blockHigherValue == 0)
						{
						  emptyLowerSlot = i;
						}
					}
				}
			}
			// didn't find the lower record, thus the record does not exist and need to be created...
			if (emptyLowerSlot != -1)
			{
				assert(emptyLowerSlot >= 0 && emptyLowerSlot < impl->lowerMaxBlockSize);
				// copy header of the first lower record
				hash_block_header header(key);
				memcpy(&buf[(emptyLowerSlot * impl->actualBlockSize)], &header, sizeof(hash_block_header));
				// copy user data of the first lower record
				memcpy(&buf[(emptyLowerSlot * impl->actualBlockSize) + sizeof(hash_block_header)], &data->size, sizeof(int));
				memcpy(&buf[(emptyLowerSlot * impl->actualBlockSize) + sizeof(hash_block_header) + sizeof(int)], data->buffer, data->size);

				++impl->recordAmount;
			} else {
				// crap, lower block maximum reached!!!
				assert(!"UInt96Hash::insertOrReplace - lower block maximum reached!");
				// what would be the correct way to handle this situation? just do nothing?
			}
		} else {
			// otwerwise, create a totally new upper record...
			unsigned char *buf = new unsigned char[impl->lowerBufferSize];
			// clear whole buffer to zeroes.
			memset(buf, 0, impl->lowerBufferSize);
			// copy header of the first lower record
			int lowerRecordNum = 0;
			hash_block_header header(key);
			memcpy(&buf[(lowerRecordNum * impl->actualBlockSize)], &header, sizeof(hash_block_header));
			// copy user data of the first lower record
			memcpy(&buf[(lowerRecordNum * impl->actualBlockSize) + sizeof(hash_block_header)], &data->size, sizeof(int));
			memcpy(&buf[(lowerRecordNum * impl->actualBlockSize) + sizeof(hash_block_header) + sizeof(int)], data->buffer, data->size);
			// put this buffer to upper hash
			impl->upperHash.insert(UpperHashPairType(key.higherValue, buf));

			++impl->recordAmount;
			++impl->upperRecordAmount;
		}
	}

	const uint_96_hash_data UInt96Hash::peekData(hashable_uint_96 key)
	{
		assert(key.higherValue != 0 || key.lowerValue != 0);

		// seek upper level record...
		UpperHashType::iterator iter = impl->upperHash.find(key.higherValue);
		if (iter != impl->upperHash.end())
		{
			// upper level record existed. see if lower level exists as well...
			unsigned char *buf = iter->second;
			int foundLowerSlot = -1;
			for (int i = 0; i < impl->lowerMaxBlockSize; i++)
			{
				unsigned char *block = &buf[i * impl->actualBlockSize];
				unsigned int blockLowerValue = ((hash_block_header *)block)->key.lowerValue;
				assert(((hash_block_header *)block)->reserved == 0);
				if (blockLowerValue == key.lowerValue)
				{
					// found it, the record exists!
					const int *bufsize = (const int *)&buf[(i * impl->actualBlockSize) + sizeof(hash_block_header)];
					const unsigned char *bufdata = (const unsigned char *)&buf[(i * impl->actualBlockSize) + sizeof(hash_block_header) + sizeof(int)];

					// oh, can't do this. need to memcpy the buffer...
					//uint_96_hash_data ret;
					//ret.size = *bufsize;
					//ret.buffer = (unsigned char *)bufdata;
					uint_96_hash_data ret(*bufsize, (unsigned char *)bufdata);
					return ret;
				}
			}
			// no lower lever record found
			uint_96_hash_data empty;
			return empty;
		} else {
			// no upper lever record found
			uint_96_hash_data empty;
			return empty;
		}
	}


	uint_96_hash_data UInt96Hash::popData(hashable_uint_96 key)
	{
		assert(key.higherValue != 0 || key.lowerValue != 0);

		// seek upper level record...
		UpperHashType::iterator iter = impl->upperHash.find(key.higherValue);
		if (iter != impl->upperHash.end())
		{
			// upper level record existed. see if lower level exists as well...
			unsigned char *buf = iter->second;
			int foundLowerSlot = -1;
			for (int i = 0; i < impl->lowerMaxBlockSize; i++)
			{
				unsigned char *block = &buf[i * impl->actualBlockSize];
				unsigned int blockLowerValue = ((hash_block_header *)block)->key.lowerValue;
				assert(((hash_block_header *)block)->reserved == 0);
				if (blockLowerValue == key.lowerValue)
				{
					// found it, the record exists!
					const int *bufsize = (const int *)&buf[(i * impl->actualBlockSize) + sizeof(hash_block_header)];
					const unsigned char *bufdata = (const unsigned char *)&buf[(i * impl->actualBlockSize) + sizeof(hash_block_header) + sizeof(int)];

					// oh, can't do this. need to memcpy the buffer...
					//uint_96_hash_data ret;
					//ret.size = *bufsize;
					//ret.buffer = (unsigned char *)bufdata;
					uint_96_hash_data ret(*bufsize, (unsigned char *)bufdata);

					// clear this block
					memset(block, 0, impl->actualBlockSize);
					--impl->recordAmount;

					// TODO: if whole lower record empty, delete the upper record as well!

					return ret;
				}
			}

			// no lower lever record found
			uint_96_hash_data empty;
			return empty;
		} else {
			// no upper lever record found
			uint_96_hash_data empty;
			return empty;
		}
	}


	void UInt96Hash::clear()
	{
		UpperHashType::iterator iter = impl->upperHash.begin();
		while (iter != impl->upperHash.end())
		{
			unsigned char *buffer = iter->second;
			assert(buffer != NULL);

#ifdef _DEBUG
			// check that how many lower records this buffer actually contained...
			for (int i = 0; i < impl->lowerMaxBlockSize; i++)
			{
				unsigned char *block = &buffer[i * impl->actualBlockSize];
				unsigned int blockLowerValue = ((hash_block_header *)block)->key.lowerValue;
				unsigned __int64 blockUpperValue = ((hash_block_header *)block)->key.higherValue;
				assert(((hash_block_header *)block)->reserved == 0);

				if (blockLowerValue != 0 && blockUpperValue != 0)
				{
					--impl->recordAmount;
				}
			}
#endif

			delete [] buffer;

			--impl->upperRecordAmount;
			++iter;
		}
		impl->upperHash.clear();

#ifdef _DEBUG
		assert(impl->recordAmount == 0);
		assert(impl->upperRecordAmount == 0);
#endif
		impl->recordAmount = 0;
		impl->upperRecordAmount = 0;

		if (impl->lastGetStateBuffer != NULL)
		{
			delete [] impl->lastGetStateBuffer;
			impl->lastGetStateBuffer = NULL;
		}
		impl->lastGetStateBufferSize = 0;
	}


	void UInt96Hash::getState(int *bufferSizeOut, const unsigned char **bufferOut)
	{
		int hdrsize = 4 + sizeof(int)*2;
		assert(hdrsize == 12);
		int ver = 1;

		impl->lastGetStateBufferSize = hdrsize + (impl->recordAmount * impl->actualBlockSize);
		impl->lastGetStateBuffer = new unsigned char[impl->lastGetStateBufferSize];

		assert(impl->lastGetStateBuffer != NULL);

		memcpy(impl->lastGetStateBuffer, "UI96", 4);
		memcpy(&impl->lastGetStateBuffer[4], &ver, sizeof(int));
		memcpy(&impl->lastGetStateBuffer[8], &impl->recordAmount, sizeof(int));

		int bufpos = hdrsize;

		UpperHashType::iterator iter = impl->upperHash.begin();
		while (iter != impl->upperHash.end())
		{
			unsigned char *buffer = iter->second;
			assert(buffer != NULL);

			for (int i = 0; i < impl->lowerMaxBlockSize; i++)
			{
				unsigned char *block = &buffer[i * impl->actualBlockSize];
				unsigned int blockLowerValue = ((hash_block_header *)block)->key.lowerValue;
				unsigned __int64 blockUpperValue = ((hash_block_header *)block)->key.higherValue;
				assert(((hash_block_header *)block)->reserved == 0);

				if (blockLowerValue != 0 && blockUpperValue != 0)
				{
					// FIXME: don't copy all the empty zeroes!!!
					// only the header + data size amount of data!!!
					assert(bufpos <= impl->lastGetStateBufferSize - impl->actualBlockSize);
					memcpy(&impl->lastGetStateBuffer[bufpos], block, impl->actualBlockSize);
					bufpos += impl->actualBlockSize;
				}
			}
			++iter;
		}

		*bufferOut = impl->lastGetStateBuffer;
		*bufferSizeOut = impl->lastGetStateBufferSize;
	}
	

	void UInt96Hash::setState(int bufferSize, const unsigned char *buffer)
	{
		clear();

		int hdrsize = 4 + sizeof(int)*2;
		assert(hdrsize == 12);
		int expectedVer = 1;

		if (bufferSize < hdrsize)
		{
			assert(!"UInt96Hash::setState() - Given buffer is too small to be valid.");
			return;
		}

		if (strncmp((char *)buffer, "UI96", 4) != 0)
		{
			assert(!"UInt96Hash::setState() - Given buffer does not contain expected id tag.");
			return;
		}
		if (*(int *)(&buffer[4]) != expectedVer)
		{
			assert(!"UInt96Hash::setState() - Unsupported version number.");
			return;
		}
		int bufferBlocks = *(int *)(&buffer[8]);
		if (bufferBlocks * impl->actualBlockSize + hdrsize != bufferSize)
		{
			assert(!"UInt96Hash::setState() - Block amount*size and buffer size mismatch.");
			return;
		}

		// NOTE: this is rather ineffective!
		// (among other things, the uint_96_hash_data creates temporary buffer copies)
		int bufpos = hdrsize;
		for (int i = 0; i < bufferBlocks; i++)
		{
			const unsigned char *block = &buffer[bufpos];
			unsigned int blockLowerValue = ((hash_block_header *)block)->key.lowerValue;
			unsigned __int64 blockUpperValue = ((hash_block_header *)block)->key.higherValue;
			assert(((hash_block_header *)block)->reserved == 0);

			// FIXME: data should not contain buffer padding zeroes!!!
			// only the header + data size amount of data!!!
			// (fix getState and then this to match that)
			uint_96_hash_data data(*((int *)(&block[sizeof(hash_block_header)])), &block[sizeof(hash_block_header) + sizeof(int)]);
			hashable_uint_96 key(blockUpperValue, blockLowerValue);

			insertOrReplace(key, &data);	

			bufpos += impl->actualBlockSize;
		}

		assert(bufpos == bufferSize);
	}
	

	int UInt96Hash::getRecordAmount()
	{
		return impl->recordAmount;
	}

	// ----------------------------------------------------------

	UInt96HashIterator::UInt96HashIterator()
	{
		this->implIterator = NULL;
		this->lowerBlockNumber = 0;
		this->block = NULL;
		this->hash = NULL;
	}

	UInt96HashIterator::~UInt96HashIterator()
	{
		delete this->implIterator;
	}

	bool UInt96HashIterator::next()
	{
		assert(this->implIterator != NULL);
		UpperHashType::iterator *iter = (UpperHashType::iterator *)this->implIterator;

		if (*iter == hash->impl->upperHash.end())
			return false;

		if (lowerBlockNumber >= hash->impl->lowerMaxBlockSize)
		{
			++(*iter);
			lowerBlockNumber = 0;

			if (*iter == hash->impl->upperHash.end())
				return false;
		}

		const unsigned char *buffer = (*iter)->second;
		block = &buffer[lowerBlockNumber * hash->impl->actualBlockSize];

		// NOTE: lowerBlockNumber indicates the next lower block in queue, not the returned one!
		++lowerBlockNumber;
		
		// umm... skip empty records here...? 

		// this hacky static variable makes it non-recursive...
		// WARNING: certainly not thread safe here!
		static bool check_for_empty = true;
		if (!check_for_empty)
		{
			return true;
		}

		check_for_empty = false;
		while (true)
		{
			hashable_uint_96 key = getKey();
			if (key.higherValue == 0 && key.lowerValue == 0)
			{
				// oh, skip this.
				if (!next())
				{
					block = NULL;
					check_for_empty = true;
					return false;
				}
			} else {
				// this was not empty, no need to skip further...
				check_for_empty = true;
				return true;
			}
		}
	}

	hashable_uint_96 UInt96HashIterator::getKey()
	{
		return ((hash_block_header *)block)->key;
	}

	const uint_96_hash_data *UInt96HashIterator::getData()
	{
		assert(((uint_96_hash_data *)&block[sizeof(hash_block_header)])->size > 0 
			&& ((uint_96_hash_data *)&block[sizeof(hash_block_header)])->size < hash->impl->maxUserBytesPerBlock);
		return (uint_96_hash_data *)&block[sizeof(hash_block_header)];
	}



}
