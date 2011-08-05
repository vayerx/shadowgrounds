// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILE_WRAPPER_H
#define INCLUDED_FILE_WRAPPER_H

#include <boost/scoped_ptr.hpp>
#include <string>
#include <vector>

namespace frozenbyte {
namespace editor {

class FileWrapper
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	FileWrapper(const std::string &dir, const std::string &extension, bool caseSensitive = false);
	~FileWrapper();

	// Dirs
	int getRootDirAmount() const;
	const std::string &getRootDir(int index) const;
	int getDirAmount(int rootIndex) const;
	const std::string &getDir(int rootIndex, int index) const;

	// Files
	int getFileAmount(int rootIndex) const;
	const std::string &getFile(int rootIndex, int fileIndex) const;
	int getFileAmount(int rootIndex, int dirIndex) const;
	const std::string &getFile(int rootIndex, int dirIndex, int fileIndex) const;

	std::vector<std::string> getAllFiles() const;
	std::vector<std::string> getAllDirs() const;

	static std::string resolveModelName(const std::string &rootDir, const std::string &fileName);
};

bool fileExists(const std::string &fileName);

} // editor
} // frozenbyte

#endif
