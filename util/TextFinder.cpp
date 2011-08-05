
#include "precompiled.h"

#include "TextFinder.h"
#include <assert.h>
#include <string>

namespace util
{
	class TextFinderImpl
	{
		private:
			TextFinderImpl(const char *textBuffer, bool skipComments)
			{
				bufstr = textBuffer;
				bufSize = bufstr.size();
				pos = 0;
				this->skipComments = skipComments;
			}

			~TextFinderImpl()
			{
				// nop
			}

			std::string bufstr;
			int pos;
			int bufSize;
			bool skipComments;

		friend class TextFinder;
	};


	TextFinder::TextFinder(const char *textBuffer, bool skipComments)
	{
		impl = new TextFinderImpl(textBuffer, skipComments); 
	}

	TextFinder::~TextFinder()
	{
		delete impl;
	}

	void TextFinder::setSkipComments(bool skipComments)
	{
		impl->skipComments = skipComments;
	}

	void TextFinder::moveToStart()
	{
		impl->pos = 0;
	}

	void TextFinder::moveToEnd()
	{
		impl->pos = impl->bufSize;
	}

	void TextFinder::moveToPosition(int position)
	{
		impl->pos = position;
		if (impl->pos < 0) impl->pos = 0;
		if (impl->pos > impl->bufSize) impl->pos = impl->bufSize;
	}

	int TextFinder::getCurrentPosition()
	{
		return impl->pos;
	}

	int TextFinder::findNext(const char *stringToFind)
	{
		if (impl->pos == impl->bufSize)
		{
			return -1;
		}

		while (true)
		{
			bool foundNonCommented = true;
			std::string::size_type fpos = impl->bufstr.find(stringToFind, impl->pos + 1);
			if (impl->skipComments)
			{
				int commseekpos = fpos - 2;
				if (commseekpos >= 0)
				{
					std::string::size_type commentpos = impl->bufstr.rfind("//", commseekpos);
					if (commentpos != std::string::npos)
					{
						assert(fpos - commentpos > 0);
						std::string::size_type endlinepos = impl->bufstr.find_first_of("\n", commentpos, fpos - commentpos);
						if (endlinepos == std::string::npos)
						{
							foundNonCommented = false;
						}
					}
				}
			}
			if (fpos != std::string::npos)
			{
				impl->pos = fpos;
				if (foundNonCommented)
				{
					return impl->pos;
				}
			} else {
				impl->pos = impl->bufSize;
				return -1;
			}
		}
		//return -1;
	}

	int TextFinder::findOneOfMany(const char *stringToFind, int findNumber)
	{
		assert(findNumber > 0);
		int ret = -1;

		for (int i = 0; i < findNumber; i++)
		{
			ret = findNext(stringToFind);
			if (ret == -1)
			{
				break;
			}
		}

		return ret;
	}

	int TextFinder::countOccurances(const char *stringToFind)
	{
		int oldPos = impl->pos;

		int amount = 0;
		while (findNext(stringToFind) != -1)
		{
			amount++;
		}

		impl->pos = oldPos;

		return amount;
	}

}


