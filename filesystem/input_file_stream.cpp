
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

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
	FILE *fp;
	size_t size;

	InputFileStreamBufferData(const std::string fileName)
	:	fp(0),
		size(0)
	{
		fp = fopen(fileName.c_str(), "rb");
		if(fp)
		{
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		}

		if(!size)
			close();
	}

	~InputFileStreamBufferData()
	{
		close();
	}

	void close()
	{
		if(fp)
		{
			fclose(fp);
			fp = 0;
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
	if(data->fp)
	{
		int input = fgetc(data->fp);
		if(input == EOF)
			data->close();
		else
			byte = (char) input;
	}

	return byte;
}


bool InputFileStreamBuffer::isEof() const
{
	if(!data->fp)
		return true;

	return false;
}

int InputFileStreamBuffer::getSize() const
{
	if(!data->fp)
		return 0;

	// FIXME: causing a possible 2Gb limitation to file size here when casting to int!
	// --jpk
	return int(data->size);
}

void InputFileStreamBuffer::popBytes(char *buffer, int bytes)
{
	if(!data->fp)
	{
		for(int i = 0; i < bytes; ++i)
			buffer[i] = 0;
	}
	else
	{
		//data->stream.read(buffer, bytes);
		if(fread(buffer, 1, bytes, data->fp) != (unsigned)bytes)
			data->close();
	}
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
