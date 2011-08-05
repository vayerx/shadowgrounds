
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable: 373) // inaccessible constructor (remark)
#endif

#include "input_file_stream.h"
#include <fstream>

#include "../system/Logger.h"
#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {

struct InputFileStreamBufferData
{
	std::ifstream stream;
	std::streamsize size;

	InputFileStreamBufferData(const std::string fileName)
	:	stream(fileName.c_str(), std::ios::binary),
		size(0)
	{
		if(stream)
		{
			std::filebuf *buffer = stream.rdbuf();
			if(buffer)
				size = buffer->in_avail();
		}
	}
};

InputFileStreamBuffer::InputFileStreamBuffer(const std::string &fileName)
{
	boost::scoped_ptr<InputFileStreamBufferData> tempData(new InputFileStreamBufferData(fileName));
	data.swap(tempData);
}

InputFileStreamBuffer::~InputFileStreamBuffer()
{
}

unsigned char InputFileStreamBuffer::popByte()
{
	char byte = 0;
	if(!isEof())
		data->stream.read(&byte, 1);

	return byte;
}

bool InputFileStreamBuffer::isEof() const
{
	if(!data->stream)
		return true;

	return data->stream.eof();
}

int InputFileStreamBuffer::getSize() const
{
	if(!data->stream)
		return 0;

	// FIXME: causing a possible 2Gb limitation to file size here when casting to int!
	// --jpk
	std::ifstream::pos_type currPos = data->stream.tellg();
	data->stream.seekg(0, std::ios::end);
	std::ifstream::pos_type res = data->stream.tellg();
	data->stream.seekg(currPos, std::ios::beg);
	return int(res);
}

void InputFileStreamBuffer::popBytes(char *buffer, int bytes)
{
	if(isEof())
	{
		for(int i = 0; i < bytes; ++i)
			buffer[i] = 0;
	}

	data->stream.read(buffer, bytes);
}

// HACK: ffs. this is needed to actually get some sense into the input stream error reportings...
//bool input_file_stream_no_nonexisting_error_message = false;

//void setInputStreamErrorReporting(bool logNonExisting)
//{
//	input_file_stream_no_nonexisting_error_message = !logNonExisting;
//}

InputStream createInputFileStream(const std::string &fileName)
{
	InputStream inputStream;
	boost::shared_ptr<InputFileStreamBuffer> inputBuffer(new InputFileStreamBuffer(fileName));

	// TODO: would need a seperate error check, eof is not the same as file does not exist!
	// (for now, just assuming that there will be no 0 byte length files, and if there are, those are errors)
	// NOTE: this will spam the error message even in some cases when we are not really trying to read a file,
	// but just to check for its existance... (shit happens)
	//if (inputBuffer->isEof())
	//{
	//	if (!input_file_stream_no_nonexisting_error_message)
	//	{
	//		Logger::getInstance()->error("createInputFileStream - File does not exist or is zero length.");
	//		Logger::getInstance()->debug(fileName.c_str());
	//	}
	//}

	inputStream.setBuffer(inputBuffer);
	return inputStream;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
