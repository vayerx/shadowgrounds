
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

#include "standard_package.h"
#include "input_file_stream.h"
#include "ifile_list.h"
#include "../editor/findfilewrapper.h"
#include <string>

#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {

using namespace editor;
using namespace std;

namespace filesystem {
namespace {

/*
void getFiles(const std::string &dir, const std::string &extension, FileList::Dir &result)
{
	string searchString = dir;
	searchString += "/";
	searchString += extension;

	editor::FindFileWrapper wrapper(searchString.c_str(), editor::FindFileWrapper::File);
	for(; !wrapper.end(); wrapper.next())
	{
		std::string fileName = dir + std::string("/") + wrapper.getName();
		if(result.findFileIndex(fileName) < 0)
			result.files.push_back(fileName);
	}

	//std::sort(result.files.begin(), result.files.end());
}

void iterateDir(const std::string &dir, const std::string &extension, FileList::Dir &result)
{
	getFiles(dir, extension, result);

	string searchString = dir;
	searchString += "/*.*";

	editor::FindFileWrapper wrapper(searchString.c_str(), editor::FindFileWrapper::Dir);
	for(; !wrapper.end(); wrapper.next())
	{
		std::string currentDir = dir;
		currentDir += "\\";
		currentDir += wrapper.getName();

		int index = result.findDirIndex(wrapper.getName());
		if(index < 0)
		{
			FileList::Dir newDir;
			newDir.name = wrapper.getName();
			result.dirs.push_back(newDir);
			index = result.dirs.size() - 1;
		}
		
		iterateDir(currentDir, extension, result.dirs[index]);
	}

	//std::sort(result.dirs.begin(), result.dirs.end(), DirSorter());
}
*/

void getFiles(const std::string &dir, const std::string &extension, IFileList &result)
{
	string searchString = dir;
	searchString += "/";
	searchString += extension;

	editor::FindFileWrapper wrapper(searchString.c_str(), editor::FindFileWrapper::File);
	for(; !wrapper.end(); wrapper.next())
	{
		std::string fileName = dir + std::string("/") + wrapper.getName();
		result.addFile(fileName);
	}
}

void iterateDir(const std::string &dir, const std::string &extension, IFileList &result)
{
	getFiles(dir, extension, result);

	string searchString = dir;
	searchString += "/*.*";

	editor::FindFileWrapper wrapper(searchString.c_str(), editor::FindFileWrapper::Dir);
	for(; !wrapper.end(); wrapper.next())
	{
		std::string currentDir = dir;
		currentDir += "\\";
		currentDir += wrapper.getName();

		result.addDir(currentDir);
		iterateDir(currentDir, extension, result);
	}
}

} // unnamed

StandardPackage::StandardPackage()
{
}

StandardPackage::~StandardPackage()
{
}

void StandardPackage::findFiles(const std::string &dir, const std::string &extension, IFileList &result)
{
	iterateDir(dir, extension, result);
}

InputStream StandardPackage::getFile(const std::string &fileName)
{
	return createInputFileStream(fileName);
}

} // end of namespace filesystem
} // end of namespace frozenbyte
