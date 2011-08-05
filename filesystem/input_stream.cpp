
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable: 444) // Destructor for base class not virtual
#endif

#include "input_stream.h"
#include "convert_type.h"
#include <boost/static_assert.hpp>

#include <limits.h>

#ifdef _MSC_VER
// Visual Studio doesn't recognise this
typedef unsigned short uint16_t;

#else
// and gcc >= 4.4 requires stdint
#include <stdint.h>

#endif

BOOST_STATIC_ASSERT(sizeof(uint16_t) * CHAR_BIT == 16);
BOOST_STATIC_ASSERT(CHAR_BIT == 8);

#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {
namespace {
	template<class Type>
	void readFromStream(IInputStreamBuffer &buffer, Type &value)
	{
		ConvertTo<Type> converter;
		for(int i = 0; i < converter.getSize(); ++i)
			converter.setByte(i, buffer.popByte());

		value = converter.getValue();
	}
} // end of unnamed namespace

InputStream::InputStream()
{
}

InputStream::~InputStream()
{
}

void InputStream::setBuffer(boost::shared_ptr<IInputStreamBuffer> streamBuffer_)
{
	assert(streamBuffer_);
	streamBuffer = streamBuffer_;
}

bool InputStream::isEof() const
{
	assert(streamBuffer);
	return streamBuffer->isEof();
}

int InputStream::getSize() const
{
	assert(streamBuffer);
	return streamBuffer->getSize();
}

InputStream &InputStream::read(std::string &value)
{
	assert(streamBuffer);

	uint16_t stringSize = 0;
	this->read(stringSize);

	value.resize(stringSize);
	for(int i = 0; i < stringSize; ++i)
		value[i] = streamBuffer->popByte();

	return *this;
}

InputStream &InputStream::read(bool &value)
{
	assert(streamBuffer);

	value = streamBuffer->popByte() != 0;
	return *this;
}

InputStream &InputStream::read(unsigned char &value)
{
	assert(streamBuffer);

	value = streamBuffer->popByte();
	return *this;
}

InputStream &InputStream::read(char &value)
{
	assert(streamBuffer);

	value = streamBuffer->popByte();
	return *this;
}

InputStream &InputStream::read(signed char &value)
{
	assert(streamBuffer);

	value = streamBuffer->popByte();
	return *this;
}

InputStream &InputStream::read(unsigned short &value)
{
	assert(streamBuffer);

	readFromStream(*streamBuffer, value);
	return *this;
}

InputStream &InputStream::read(signed short &value)
{
	assert(streamBuffer);

	readFromStream(*streamBuffer, value);
	return *this;
}

InputStream &InputStream::read(unsigned int &value)
{
	assert(streamBuffer);

	readFromStream(*streamBuffer, value);
	return *this;
}

InputStream &InputStream::read(signed int &value)
{
	assert(streamBuffer);

	readFromStream(*streamBuffer, value);
	return *this;
}

InputStream &InputStream::read(float &value)
{
	assert(streamBuffer);

	readFromStream(*streamBuffer, value);
	return *this;
}

InputStream &InputStream::read(double &value)
{
	assert(streamBuffer);

	readFromStream(*streamBuffer, value);
	return *this;
}

InputStream &InputStream::read(unsigned char *buffer, int elements)
{
	assert(streamBuffer);

	streamBuffer->popBytes(reinterpret_cast<char *> (buffer), elements);
	return *this;
}

InputStream &InputStream::read(char *buffer, int elements)
{
	assert(streamBuffer);

	streamBuffer->popBytes(buffer, elements);
	return *this;
}

InputStream &InputStream::read(unsigned short *buffer, int elements)
{
	assert(streamBuffer);

	streamBuffer->popBytes(reinterpret_cast<char *> (buffer), elements * sizeof(short));
	return *this;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
