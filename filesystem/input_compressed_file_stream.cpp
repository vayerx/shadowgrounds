#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

#ifdef __INTEL_COMPILER
#pragma warning(disable: 373) // inaccessible constructor (remark)
#endif

#include "input_compressed_file_stream.h"
#include <fstream>
#include <vector>
#include "detail/zlib.h"

#include "../system/Logger.h"
#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {

struct InputCompressedFileStreamBufferData
{
	std::vector<unsigned char> buffer;
	int readPosition;

	InputCompressedFileStreamBufferData(const std::string fileName)
	:	readPosition(0)
	{
		std::ifstream stream(fileName.c_str(), std::ios::binary);
		if(!stream)
			return;

		std::filebuf *streamBuffer = stream.rdbuf();
		if(!streamBuffer)
			return;
		std::streamsize compressedSize = streamBuffer->in_avail() - sizeof(int);

		int bytes = 0;
		stream.read((char *) &bytes, sizeof(int));
		if(!bytes)
			return;

		buffer.resize(bytes);
		std::vector<char> compressedBuffer((int) compressedSize);
		stream.read(&compressedBuffer[0],  compressedSize);

		Bytef *source = (Bytef *) &compressedBuffer[0];
		uLong sourceLength = compressedBuffer.size();
		Bytef *destination = &buffer[0];
		uLong destinationLength = buffer.size();

		int code = ::uncompress(destination, &destinationLength, source, sourceLength);
		if(code != Z_OK)
			int a = 0;
	}
};

InputCompressedFileStreamBuffer::InputCompressedFileStreamBuffer(const std::string &fileName)
{
	boost::scoped_ptr<InputCompressedFileStreamBufferData> tempData(new InputCompressedFileStreamBufferData(fileName));
	data.swap(tempData);
}

InputCompressedFileStreamBuffer::~InputCompressedFileStreamBuffer()
{
}

unsigned char InputCompressedFileStreamBuffer::popByte()
{
	char byte = 0;
	if(!isEof())
		byte = data->buffer[data->readPosition++];

	return byte;
}

bool InputCompressedFileStreamBuffer::isEof() const
{
	if(data->readPosition >= getSize())
		return true;

	return false;
}

int InputCompressedFileStreamBuffer::getSize() const
{
	return int(data->buffer.size());
}

void InputCompressedFileStreamBuffer::popBytes(char *buffer, int bytes)
{
	int readSize = bytes;
	if(data->readPosition + readSize > getSize())
	{
		readSize = getSize() - data->readPosition;
		for(int i = readSize; i < bytes; ++i)
			buffer[i] = 0;
	}

	for(int i = 0; i < readSize; ++i)
		buffer[i] = data->buffer[data->readPosition++];
}

InputStream createInputCompressedFileStream(const std::string &fileName)
{
	InputStream inputStream;
	boost::shared_ptr<InputCompressedFileStreamBuffer> inputBuffer(new InputCompressedFileStreamBuffer(fileName));

	inputStream.setBuffer(inputBuffer);
	return inputStream;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
