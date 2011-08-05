#ifndef INCLUDED_FILE_EMPTY_BUFFER_H
#define INCLUDED_FILE_EMPTY_BUFFER_H

#include "input_stream.h"

namespace frozenbyte {
namespace filesystem {

	struct EmptyBuffer: public IInputStreamBuffer
	{
		unsigned char popByte()
		{
			return 0;
		}

		bool isEof() const
		{
			return true;
		}

		int getSize() const
		{
			return 0;
		}

		void popBytes(char *, int)
		{
		}
	};

} // filesystem
} // frozenbyte

#endif
