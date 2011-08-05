#include "precompiled.h"
#include "file_iterator.h"
#include <windows.h>

namespace frozenbyte {
namespace editor {

	bool FileIterator::validateFile(const std::string &string)
	{
		if(string == "." || string == "..")
			return false;

		if(foldersOnly && !(findData.attrib & _A_SUBDIR))
			return false;

		return true;
	}

	FileIterator::FileIterator(const std::string &searchString, bool _foldersOnly)
	{
		foldersOnly = _foldersOnly;

		ZeroMemory(&findData, sizeof(_finddata_t));
		validFile = true;

		handle = _findfirst(searchString.c_str() , &findData);
		if(!validateFile(findData.name) && handle >= 0)
			next();
	}

	FileIterator::~FileIterator()
	{
		if(handle >= 0)
			_findclose(handle);
	}

	std::string FileIterator::getFileName()
	{
		if(handle == -1 || !validFile)
			return "";

		std::string string = findData.name;
		return string;
	}

	void FileIterator::next()
	{
		do
		{
			if(_findnext(handle, &findData) == -1)
				validFile = false;
		} while(!validateFile(findData.name) && validFile);
	}

}
}