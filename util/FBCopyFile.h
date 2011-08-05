
#ifndef COPYFILE_H
#define COPYFILE_H

#include <string>

namespace util
{
	class FBCopyFile
	{
	public:
		// TODO: this will currently only handle (smallish) text file properly
		// DON'T USE FOR BINARY FILES!!!
		static void copyFile(const std::string &from, const std::string &to);
	};
}

#endif

