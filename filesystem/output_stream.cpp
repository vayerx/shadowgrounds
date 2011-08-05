
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef __INTEL_COMPILER
#pragma warning(disable: 444) // Destructor for base class not virtual
#endif

#include "output_stream.h"
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
	void sendToStream(IOutputStreamBuffer &buffer, Type value)
	{
		ConvertFrom<Type> converter(value);
		for(int i = 0; i < converter.getSize(); ++i)
			buffer.putByte(converter.getByte(i));
	}
} // end of unnamed namespace

OutputStream::OutputStream()
:	textStrings(false)
{
}

OutputStream::~OutputStream()
{
}

void OutputStream::setBuffer(boost::shared_ptr<IOutputStreamBuffer> streamBuffer_)
{
	assert(streamBuffer_);
	streamBuffer = streamBuffer_;
}

void OutputStream::useTextStrings()
{
	textStrings = true;
}

OutputStream &OutputStream::write(const std::string &value)
{
	assert(streamBuffer);
	uint16_t stringSize = value.size();

	if(!textStrings)
		write(stringSize);
	
	for(int i = 0; i < stringSize; ++i)
		streamBuffer->putByte(value[i]);

	return *this;
}

OutputStream &OutputStream::write(bool value)
{
	assert(streamBuffer);

	unsigned char b = (value) ? 1 : 0;
	streamBuffer->putByte(b);

	return *this;
}

OutputStream &OutputStream::write(unsigned char value)
{
	assert(streamBuffer);

	streamBuffer->putByte(value);
	return *this;
}

OutputStream &OutputStream::write(signed char value)
{
	assert(streamBuffer);

	streamBuffer->putByte(value);
	return *this;
}

OutputStream &OutputStream::write(unsigned short value)
{
	assert(streamBuffer);

	sendToStream(*streamBuffer, value);
	return *this;
}

OutputStream &OutputStream::write(signed short value)
{
	assert(streamBuffer);

	sendToStream(*streamBuffer, value);
	return *this;
}

OutputStream &OutputStream::write(unsigned int value)
{
	assert(streamBuffer);

	sendToStream(*streamBuffer, value);
	return *this;
}

OutputStream &OutputStream::write(signed int value)
{
	assert(streamBuffer);

	sendToStream(*streamBuffer, value);
	return *this;
}

OutputStream &OutputStream::write(float value)
{
	assert(streamBuffer);

	sendToStream(*streamBuffer, value);
	return *this;
}

OutputStream &OutputStream::write(double value)
{
	assert(streamBuffer);

	sendToStream(*streamBuffer, value);
	return *this;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
