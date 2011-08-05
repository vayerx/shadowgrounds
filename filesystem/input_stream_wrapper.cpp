
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include "input_stream_wrapper.h"
#include "file_package_manager.h"
#include "../system/Logger.h"
#include <vector>


namespace frozenbyte {
namespace filesystem {
namespace {

	int openFileAmount = 0;

	struct Tracker
	{
		Tracker()
		{
		}

		~Tracker()
		{
			assert(openFileAmount == 0);
		}
	};

	Tracker tracker;

} // unnamed

struct FB_FILE
{
	InputStream stream;

	FB_FILE(InputStream &stream_)
	:	stream(stream_)
	{
		++openFileAmount;	
	}

	~FB_FILE()
	{
		--openFileAmount;
	}
};


FB_FILE *fb_fopen(const char *filename, const char *)
{
	if(!filename)
		return 0;

	FilePackageManager &manager = FilePackageManager::getInstance();

	manager.setInputStreamErrorReporting(false);

	InputStream stream = manager.getFile(filename);

	manager.setInputStreamErrorReporting(true);

	if(stream.isEof())
		return 0;

	return new FB_FILE(stream);
}

size_t fb_fread(void *buffer, size_t size, size_t count, FB_FILE *stream)
{
	// always zero out the buffer
	memset(buffer, 0, size * count);

	if(!stream)
	{
		Logger::getInstance()->warning("fb_fread - Attempt to read when no stream available.");
		return 0;
	}

	if(stream->stream.isEof())
	{
		Logger::getInstance()->warning("fb_fread - Attempt to read past end of file.");
	}

	unsigned char *charBuffer = reinterpret_cast<unsigned char *> (buffer);
	stream->stream.read(charBuffer, size * count);

	return count;
}

size_t fb_fsize(FB_FILE *stream)
{
	if(!stream)
		return 0;

	return stream->stream.getSize();
}

int fb_feof(FB_FILE *stream)
{
	if(!stream || stream->stream.isEof())
		return 1;

	return 0;
}

int fb_fclose(FB_FILE *stream)
{
	delete stream;
	return 0;
}

} // filesystem
} // frozenbyte
