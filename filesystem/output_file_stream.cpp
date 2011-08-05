
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef __INTEL_COMPILER
#pragma warning(disable: 373) // inaccessible constructor (remark)
#endif

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "output_file_stream.h"
#include <fstream>

#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {

struct OutputFileStreamBufferData
{
	std::ofstream stream;

	OutputFileStreamBufferData(const std::string fileName)
	:	stream(fileName.c_str(), std::ios::binary)
	{
	}

	~OutputFileStreamBufferData()
	{
	}
};

OutputFileStreamBuffer::OutputFileStreamBuffer(const std::string &fileName)
{
	boost::scoped_ptr<OutputFileStreamBufferData> tempData(new OutputFileStreamBufferData(fileName));
	data.swap(tempData);
}

OutputFileStreamBuffer::~OutputFileStreamBuffer()
{
}

void OutputFileStreamBuffer::putByte(unsigned char c)
{
	char c_ = c;
	data->stream.write(&c_, 1);
}

OutputStream createOutputFileStream(const std::string &fileName)
{
	OutputStream outputStream;
	boost::shared_ptr<OutputFileStreamBuffer> outputBuffer(new OutputFileStreamBuffer(fileName));

	outputStream.setBuffer(outputBuffer);
	return outputStream;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
