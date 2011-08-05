// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_IFILE_PACKAGE
#define INCLUDED_FILESYSTEM_IFILE_PACKAGE

#ifndef INCLUDED_FILESYSTEM_INPUT_STREAM_H
#include "input_stream.h"
#endif

namespace frozenbyte {
namespace filesystem {

class IFileList;

class IFilePackage
{
public:
	virtual ~IFilePackage() {}

	virtual void findFiles(const std::string &dir, const std::string &extension, IFileList &result) = 0;
	virtual InputStream getFile(const std::string &fileName) = 0;
	virtual unsigned int getCrc(const std::string &fileName) { return 0; }
};

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
