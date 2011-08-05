#include "precompiled.h"
#include <vector>
#include "detail/zlib.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef __INTEL_COMPILER
#pragma warning(disable: 373) // inaccessible constructor (remark)
#endif

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "output_compressed_file_stream.h"
#include <fstream>

#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {

struct OutputCompressedFileStreamBufferData
{
	std::ofstream stream;
	std::vector<unsigned char> buffer;

	OutputCompressedFileStreamBufferData(const std::string fileName)
	:	stream(fileName.c_str(), std::ios::binary)
	{
	}

	~OutputCompressedFileStreamBufferData()
	{
	}

	void compress()
	{
		int bytes = buffer.size();
		stream.write((char *) &bytes, sizeof(int));
		if(!bytes)
			return;

		std::vector<char> tempBuffer(bytes + 12);
		Bytef *source = &buffer[0];
		uLong sourceLength = buffer.size();
		Bytef *destination = (Bytef *) &tempBuffer[0];
		uLong destinationLength = tempBuffer.size();

		int code = ::compress(destination, &destinationLength, source, sourceLength);
		if(code != Z_OK)
			int a = 0;

		stream.write(&tempBuffer[0], destinationLength);
	}
};

OutputCompressedFileStreamBuffer::OutputCompressedFileStreamBuffer(const std::string &fileName)
{
	boost::scoped_ptr<OutputCompressedFileStreamBufferData> tempData(new OutputCompressedFileStreamBufferData(fileName));
	data.swap(tempData);
}

OutputCompressedFileStreamBuffer::~OutputCompressedFileStreamBuffer()
{
	data->compress();
}

void OutputCompressedFileStreamBuffer::putByte(unsigned char c)
{
	data->buffer.push_back(c);
	//char c_ = c;
	//data->stream.write(&c_, 1);
}

OutputStream createOutputCompressedFileStream(const std::string &fileName)
{
	OutputStream outputStream;
	boost::shared_ptr<OutputCompressedFileStreamBuffer> outputBuffer(new OutputCompressedFileStreamBuffer(fileName));

	outputStream.setBuffer(outputBuffer);
	return outputStream;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
