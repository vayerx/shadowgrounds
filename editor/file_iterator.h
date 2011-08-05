#ifndef INCLUDED_FILE_ITERATOR_H
#define INCLUDED_FILE_ITERATOR_H
#include <string>
#include <io.h>

namespace frozenbyte {
namespace editor {

	class FileIterator
	{
		long handle;
		_finddata_t findData;
		bool validFile;
		bool foldersOnly;

		bool validateFile(const std::string &string);

	public:
		FileIterator(const std::string &searchString, bool foldersOnly);
		~FileIterator();
		std::string getFileName();
		void next();
	};
}
}

#endif