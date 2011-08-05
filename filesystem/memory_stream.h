// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_MEMORY_STREAM_H
#define INCLUDED_FILESYSTEM_MEMORY_STREAM_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_FILESYSTEM_INPUT_STREAM_H
#include "input_stream.h"
#endif
#ifndef INCLUDED_FILESYSTEM_OUTPUT_STREAM_H
#include "output_stream.h"
#endif

namespace frozenbyte {
namespace filesystem {

struct MemoryStreamBufferData;

class MemoryStreamBuffer: public IInputStreamBuffer, public IOutputStreamBuffer
{
	boost::scoped_ptr<MemoryStreamBufferData> data;

public:
	MemoryStreamBuffer();
	~MemoryStreamBuffer();

	unsigned char popByte();
	bool isEof() const;
	int getSize() const;

	void putByte(unsigned char byte);
	void popBytes(char *buffer, int bytes);
};

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
