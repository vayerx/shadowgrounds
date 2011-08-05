// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_INPUT_COMPRESSED_FILE_STREAM_H
#define INCLUDED_FILESYSTEM_INPUT_COMPRESSED_FILE_STREAM_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_FILESYSTEM_INPUT_STREAM_H
#include "input_stream.h"
#endif

namespace frozenbyte {
namespace filesystem {

struct InputCompressedFileStreamBufferData;

class InputCompressedFileStreamBuffer: public IInputStreamBuffer
{
	boost::scoped_ptr<InputCompressedFileStreamBufferData> data;

public:
	InputCompressedFileStreamBuffer(const std::string &fileName);
	~InputCompressedFileStreamBuffer();

	unsigned char popByte();	
	bool isEof() const;
	int getSize() const;

	void popBytes(char *buffer, int bytes);
};

InputStream createInputCompressedFileStream(const std::string &fileName);

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
