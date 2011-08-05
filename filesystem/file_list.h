// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_FILE_LIST_H
#define INCLUDED_FILESYSTEM_FILE_LIST_H

#include "ifile_list.h"

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>

namespace frozenbyte {
namespace filesystem {

class FileList: public IFileList
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	FileList();
	~FileList();

	void setCaseSensitivity(bool enable);

	void addDir(const std::string &dir);
	void addFile(const std::string &file);

	int getDirAmount(const std::string &root) const;
	std::string getDirName(const std::string &root, int index) const;
	int getFileAmount(const std::string &root) const;
	const std::string &getFileName(const std::string &root, int index) const;
};

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
