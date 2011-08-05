
#include "precompiled.h"

#include "TextFileModifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <string>

#include "Debug_MemoryManager.h"

#define TEXTFILEMODIFIER_EXTRA_BUF_ALLOC 256


namespace util
{
	class TextFileModifierImpl
	{
		public:
			TextFileModifierImpl()
			{
				filename = NULL;
				buf = NULL;
				bufAlloced = 0;
				selectionStart = 0;
				selectionEnd = 0;
				fileSize = 0;
			}

			~TextFileModifierImpl()
			{
				if (filename != NULL)
				{
					delete [] filename;
				}
				if (buf != NULL)
				{
					delete [] buf;
				}
			}

			char *filename;
			char *buf;
			int bufAlloced;
			int selectionStart; // inclusive
			int selectionEnd;   // exclusive
			int fileSize;

	};


	TextFileModifier::TextFileModifier()
	{
		this->impl = new TextFileModifierImpl();
	}

	TextFileModifier::~TextFileModifier()
	{
		delete impl;
	}

	int TextFileModifier::replaceString(const char *stringToBeReplaced, const char *newString, bool insideSelectionOnly)
	{
		assert(stringToBeReplaced != NULL);
		assert(newString != NULL);

		assert(impl->buf != NULL);

		int replaced = 0;

		std::string bufstr = impl->buf;
		int findStart = 0;
		int findEnd = bufstr.size();
		if (insideSelectionOnly)
		{
			findStart = impl->selectionStart;
			findEnd = impl->selectionEnd;
		}

		int stringToBeReplacedLen = strlen(stringToBeReplaced);

		while (true)
		{
			std::string::size_type fpos = bufstr.find(stringToBeReplaced, findStart);
			if (fpos != std::string::npos && (int)fpos <= (findEnd - stringToBeReplacedLen))
			{
				bufstr.replace(fpos, stringToBeReplacedLen, newString);
			} else {
				break;
			}
		}

		delete [] impl->buf;
		impl->buf = new char[bufstr.size() + 1];
		strcpy(impl->buf, bufstr.c_str());
		impl->fileSize = bufstr.length();

		// TODO: proper handling of selection start and end
		// (for now, just setting those both to file start)
		setBothSelectionsToStart();

		return replaced;
	}

	void TextFileModifier::setStartSelectionToStart()
	{
		impl->selectionStart = 0;
	}

	void TextFileModifier::setEndSelectionToEnd()
	{
		impl->selectionEnd = impl->fileSize;
	}

	void TextFileModifier::setBothSelectionsToStart()
	{
		impl->selectionStart = 0;
		impl->selectionEnd = 0;
	}

	void TextFileModifier::setBothSelectionsToEnd()
	{
		impl->selectionStart = impl->fileSize;
		impl->selectionEnd = impl->fileSize;
	}

	bool TextFileModifier::setStartSelectionNearMarker(const char *marker)
	{
		assert(marker != NULL);
		int markLen = strlen(marker);
		int lastLinefeed = -1;

		char *buf = impl->buf;

		for (int i = 0; i < impl->fileSize; i++)
		{
			if (buf[i] == marker[0])
			{
				if (strncmp(&buf[i], marker, markLen) == 0)
				{
					impl->selectionStart = lastLinefeed + 1;
					return true;
				}
			} else {
				if (buf[i] == '\n')
				{
					lastLinefeed = i;
				}
			}
		}
		return false;
	}

	bool TextFileModifier::setEndSelectionNearMarker(const char *marker)
	{
		assert(marker != NULL);
		int markLen = strlen(marker);

		char *buf = impl->buf;

		for (int i = 0; i < impl->fileSize; i++)
		{
			if (buf[i] == marker[0])
			{
				if (strncmp(&buf[i], marker, markLen) == 0)
				{
					for (int j = i; j < impl->fileSize; j++)
					{
						if (buf[j] == '\n')
						{
							impl->selectionEnd = j + 1;
							return true;
						}
					}
					impl->selectionEnd = impl->fileSize;
					return true;
				}
			}
		}
		return false;
	}

	void TextFileModifier::cropToSelection()
	{
		int selSize = impl->selectionEnd - impl->selectionStart;
		char *newbuf = new char[selSize + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC + 1];
		strncpy(newbuf, &impl->buf[impl->selectionStart], selSize);
		newbuf[selSize] = '\0';
		delete [] impl->buf;

		impl->buf = newbuf;
		impl->bufAlloced = selSize + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC;
		impl->fileSize = selSize;
		impl->selectionStart = 0;
		impl->selectionEnd = selSize;
	}

	void TextFileModifier::deleteSelection()
	{
		assert(impl->selectionEnd >= impl->selectionStart);

		int selSize = impl->selectionEnd - impl->selectionStart;
		int moveBlockSize = impl->fileSize - impl->selectionEnd;
		for (int i = 0; i < moveBlockSize; i++)
		{
			impl->buf[impl->selectionStart + i] = impl->buf[impl->selectionEnd + i];
		}
		impl->buf[impl->selectionStart + moveBlockSize] = '\0';
		impl->fileSize -= selSize;
		impl->selectionEnd -= selSize;
	}

	char *TextFileModifier::getSelectionAsNewBuffer()
	{
		assert(impl->selectionEnd >= impl->selectionStart);

		int selSize = impl->selectionEnd - impl->selectionStart;
		char *ret = new char[selSize + 2];
		strncpy(ret, &impl->buf[impl->selectionStart], selSize);
		ret[selSize] = '\0';
		return ret;
	}

	void TextFileModifier::addAfterSelection(const char *text)
	{
		assert(text != NULL);
		int txtlen = strlen(text);
		if (impl->fileSize + txtlen >= impl->bufAlloced)
		{
			char *newbuf = new char[impl->fileSize + txtlen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC + 1];
			strcpy(newbuf, impl->buf);
			newbuf[impl->fileSize + txtlen] = '\0';
			delete [] impl->buf;
			impl->buf = newbuf;
			impl->bufAlloced = impl->fileSize + txtlen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC;
		}

		int moveBlockSize = impl->fileSize - impl->selectionEnd;
		for (int i = moveBlockSize - 1; i >= 0; i--)
		{
			impl->buf[impl->selectionEnd + txtlen + i] = impl->buf[impl->selectionEnd + i];
		}
		impl->buf[impl->fileSize + txtlen] = '\0';
		for (int j = 0; j < txtlen; j++)
		{
			impl->buf[impl->selectionEnd + j] = text[j];
		}

		impl->fileSize += txtlen;
	}

	void TextFileModifier::addBeforeSelection(const char *text)
	{
		assert(text != NULL);
		int txtlen = strlen(text);
		if (impl->fileSize + txtlen >= impl->bufAlloced)
		{
			char *newbuf = new char[impl->fileSize + txtlen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC + 1];
			strcpy(newbuf, impl->buf);
			newbuf[impl->fileSize + txtlen] = '\0';
			delete [] impl->buf;
			impl->buf = newbuf;
			impl->bufAlloced = impl->fileSize + txtlen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC;
		}

		int moveBlockSize = impl->fileSize - impl->selectionStart;
		for (int i = moveBlockSize - 1; i >= 0; i--)
		{
			impl->buf[impl->selectionStart + txtlen + i] = impl->buf[impl->selectionStart + i];
		}
		impl->buf[impl->fileSize + txtlen] = '\0';
		for (int j = 0; j < txtlen; j++)
		{
			impl->buf[impl->selectionStart + j] = text[j];
		}

		impl->selectionStart += txtlen;
		impl->selectionEnd += txtlen;
		impl->fileSize += txtlen;
	}

	bool TextFileModifier::loadFile(const char *filename)
	{
		closeFile();

		FILE *f = fopen(filename, "rb");
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			int flen = ftell(f);
			fseek(f, 0, SEEK_SET);

			if (flen >= 0)
			{
				// TODO: should check that fread, and others, succeed.
				impl->buf = new char[flen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC + 1];
				impl->bufAlloced = flen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC;
				impl->fileSize = flen;
				fread(impl->buf, flen, 1, f);
				impl->buf[flen] = '\0';
				fclose(f);

				impl->filename = new char[strlen(filename) + 1];
				strcpy(impl->filename, filename);

				return true;
			} else {
				fclose(f);
				return false;
			}
		} else {
			return false;
		}

	}

	bool TextFileModifier::saveFile()
	{
		if (impl->filename == NULL)
		{
			assert(0);
			return false;
		}

		if (impl->buf == NULL)
		{
			assert(0);
			return false;
		}

		FILE *f = fopen(impl->filename, "wb");
		if (f != NULL)
		{
			if (impl->fileSize > 0)
			{
				int succ = fwrite(impl->buf, impl->fileSize, 1, f);
			  fclose(f);
				if (succ == 1)
					return true;
				else
					return false;
			} else {
				return true;
			}
		}

		return false;
	}
	
	bool TextFileModifier::saveFileAs(const char *filename)
	{
		assert(filename != NULL);

		if (impl->filename != NULL)
		{
			delete [] impl->filename;
			impl->filename = NULL;
		}

		impl->filename = new char[strlen(filename) + 1];
		strcpy(impl->filename, filename);

		return saveFile();
	}
	
	void TextFileModifier::closeFile()
	{
		if (impl->filename != NULL)
		{
			delete [] impl->filename;
		}
		impl->filename = NULL;
		if (impl->buf != NULL)
		{
			delete [] impl->buf;
		}
		impl->buf = NULL;

		impl->bufAlloced = 0;
		impl->selectionEnd = 0;
		impl->selectionStart = 0;
		impl->fileSize = 0;
	}

	void TextFileModifier::loadMemoryFile(const char *buf, int buflen)
	{
		closeFile();

		assert(buf != NULL);
		assert(buflen >= 0);

		assert((int)strlen(buf) == buflen);

		impl->buf = new char[buflen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC + 1];
		strcpy(impl->buf, buf);
		impl->bufAlloced = buflen + TEXTFILEMODIFIER_EXTRA_BUF_ALLOC;
	}

	void TextFileModifier::newFile()
	{
		// FIXME: this bugs!
		//loadMemoryFile("", 0);
		// howabout this?
		loadMemoryFile("\n", 1);
	}

	// just combines start to start and end to end.
	void TextFileModifier::selectAll()
	{
		setStartSelectionToStart();
		setEndSelectionToEnd();
	}

	// begin selection from given position (inclusive)
	void TextFileModifier::setStartSelectionToPosition(int position)
	{
		impl->selectionStart = position;
		if (impl->selectionStart < 0)
			impl->selectionStart = 0;
		if (impl->selectionStart > impl->fileSize)
			impl->selectionStart = impl->fileSize;

		if (impl->selectionEnd < impl->selectionStart)
			impl->selectionEnd = impl->selectionStart;
	}

	// end selection to given position (exclusive)
	void TextFileModifier::setEndSelectionToPosition(int position)
	{
		impl->selectionEnd = position;
		if (impl->selectionEnd < 0)
			impl->selectionEnd = 0;
		if (impl->selectionEnd > impl->fileSize)
			impl->selectionEnd = impl->fileSize;

		if (impl->selectionStart > impl->selectionEnd)
			impl->selectionStart = impl->selectionEnd;
	}

}

