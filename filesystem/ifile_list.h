// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_IFILE_LIST_H
#define INCLUDED_FILESYSTEM_IFILE_LIST_H

#include <vector>
#include <string>

namespace frozenbyte {
namespace filesystem {

class IFileList
{
public:
	virtual ~IFileList() {}

	virtual void setCaseSensitivity(bool enable) = 0;

	virtual void addDir(const std::string &dir) = 0;
	virtual void addFile(const std::string &file) = 0;

	virtual int getDirAmount(const std::string &root) const = 0;
	virtual std::string getDirName(const std::string &root, int index) const = 0;
	virtual int getFileAmount(const std::string &root) const = 0;
	virtual const std::string &getFileName(const std::string &root, int index) const = 0;
};

// These to file_list_util.h or something
void getAllFiles(IFileList &list, const std::string &root, std::vector<std::string> &result, bool recurse, bool caseSensitive = false);
void getAllDirs(IFileList &list, const std::string &root, std::vector<std::string> &result, bool recurse, bool caseSensitive = false);
void convertLower(std::string &str);

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
