
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "memory_stream.h"
#include <queue>

#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {

struct MemoryStreamBufferData
{
	std::queue<unsigned char> buffer;
};

MemoryStreamBuffer::MemoryStreamBuffer()
{
	boost::scoped_ptr<MemoryStreamBufferData> tempData(new MemoryStreamBufferData());
	data.swap(tempData);
}

MemoryStreamBuffer::~MemoryStreamBuffer()
{
}

unsigned char MemoryStreamBuffer::popByte()
{
	if(!data->buffer.empty())
	{
		unsigned char value = data->buffer.front();
		data->buffer.pop();

		return value;
	}

	return 0;
}

bool MemoryStreamBuffer::isEof() const
{
	return data->buffer.empty();
}

int MemoryStreamBuffer::getSize() const
{
	return data->buffer.size();
}


void MemoryStreamBuffer::putByte(unsigned char byte)
{
	data->buffer.push(byte);
}

void MemoryStreamBuffer::popBytes(char *buffer, int bytes)
{
	for(int i = 0; i < bytes; ++i)
		buffer[i] = popByte();
}

} // end of namespace filesystem
} // end of namespace frozenbyte
