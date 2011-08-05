// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_INPUT_FILE_STREAM_H
#define INCLUDED_FILESYSTEM_INPUT_FILE_STREAM_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_FILESYSTEM_INPUT_STREAM_H
#include "input_stream.h"
#endif

namespace frozenbyte {
namespace filesystem {

struct InputFileStreamBufferData;

class InputFileStreamBuffer: public IInputStreamBuffer
{
	boost::scoped_ptr<InputFileStreamBufferData> data;

public:
	InputFileStreamBuffer(const std::string &fileName);
	~InputFileStreamBuffer();

	unsigned char popByte();	
	bool isEof() const;
	int getSize() const;

	void popBytes(char *buffer, int bytes);
};

InputStream createInputFileStream(const std::string &fileName);
void setInputStreamErrorReporting(bool logNonExisting);

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
