
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef __GNUC__
#pragma warning(disable:4103)
#endif

#include "file_list.h"
#include <string>
#include <map>
#include <vector>
#include <algorithm>

//#include "../system/logger.h"
#ifndef __GNUC__
#pragma warning(disable: 4786)
#endif

using namespace std;
using namespace boost;

namespace frozenbyte {
namespace filesystem {

void convertLower(std::string &str)
{
	for(unsigned int i = 0; i < str.size(); ++i)
	{
		str[i] = tolower(str[i]);

		if(str[i] == '\\')
			str[i] = '/';
	}
}


	/*
	void addAllFiles(vector<string> &result, const FileList::Dir &dir)
	{
		for(unsigned int i = 0; i < dir.files.size(); ++i)
			result.push_back(dir.files[i]);

		for(unsigned int j = 0; j < dir.dirs.size(); ++j)
			addAllFiles(result, dir.dirs[j]);
	}
	*/

/*
int FileList::Dir::findFileIndex(const string &name) const
{
	for(unsigned int i = 0; i < files.size(); ++i)
	{
		if(files[i] == name)
			return i;
	}

	return -1;
}

int FileList::Dir::findDirIndex(const string &name) const
{
	for(unsigned int i = 0; i < dirs.size(); ++i)
	{
		if(dirs[i].name == name)
			return i;
	}

	return -1;
}

void FileList::getAllFiles(vector<string> &result) const
{
	addAllFiles(result, root);
	sort(result.begin(), result.end());
}
*/

struct Dir;
typedef vector<string> StringList;
typedef map<string, Dir> DirList;
typedef vector<DirList::iterator> IteratorList;

struct Dir
{
	StringList files;
	IteratorList subDirs;

	Dir () :
		files(),
		subDirs()
	{}

	void addFile(const string &file)
	{
		for(unsigned int i = 0; i < files.size(); ++i)
		{
			if(file == files[i])
				return;
		}

		files.push_back(file);
	}
};

static void haxFile(std::string &str)
{
	for(unsigned int i = 0; i < str.size(); ++i)
	{
		if(str[i] == '\\')
			str[i] = '/';
	}
}

static void getDirPart(const string &file, string &dir)
{
	string::size_type index = file.find_last_of("\\/");
	if(index == file.npos)
	{
		dir = string();
		return;
	}

	dir = file.substr(0, index);
}

static int getDirAmountImp(const IteratorList &list)
{
	int result = list.size();
	/*
	for(IteratorList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		const DirList::const_iterator &i = *it;
		result += getDirAmountImp(i->second.subDirs);
	}
	*/

	return result;
}

static int getDirNameImp(const IteratorList &list, string &result, int currentIndex, int findIndex)
{
	int original = currentIndex;
	for(IteratorList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		const DirList::const_iterator &i = *it;
		if(currentIndex++ == findIndex)
		{
			result = i->first;
			break;
		}

		//if(currentIndex > findIndex)
		//	break;

		//currentIndex += getDirNameImp(i->second.subDirs, result, currentIndex, findIndex);
	}

	return currentIndex - original;
}

static string empty;


struct FileList::Data
{
	bool caseSensitive;
	DirList dirs;

	Data() :
	caseSensitive(false),
	dirs()
	{}

	/*
	void addDirHierarchy(const string &dir)
	{
		DirList::iterator it;
		if(!findDir(it, dir))
		{
			dirs[dir] = Dir();
			findDir(it, dir);

			// Add dir to parents list
			// FIXME: Does not work for roots ,,

			string parent;
			getDirPart(dir, parent);
			if(parent.empty())
				return;

			DirList::iterator parentIt;
			if(!findDir(parentIt, parent))
			{
				addDirHierarchy(parent);
				findDir(parentIt, parent);
				parentIt->second.subDirs.push_back(it);
			}
		}
	}

	void addDir(const string &dir)
	{
		DirList::iterator it;
		if(!findDir(it, dir))
		{
			dirs[dir] = Dir();
			findDir(it, dir);

			// Add dir to parents list
			// FIXME: Does not work for roots ,,

			string parent;
			getDirPart(dir, parent);
			if(parent.empty())
				return;

			DirList::iterator parentIt;
			if(!findDir(parentIt, parent))
				addDir(parent);

			findDir(parentIt, parent);
			bool found = false;
			for(IteratorList::iterator i = parentIt->second.subDirs.begin(); i != parentIt->second.subDirs.end(); ++i)
			{
				if(*i == it)
				{
					found = true;
					break;
				}
			}

			if(!found)
				parentIt->second.subDirs.push_back(it);
		}

	}
	*/

	void addDir(const string &dir)
	{
		DirList::iterator it;
		if(!findDir(it, dir))
		{
			dirs[dir] = Dir();
			findDir(it, dir);

			// Add to parent list

			string current = dir;
			DirList::iterator currentIt = it;

			for(int i = 0; ; ++i)
			{
				string parent;
				getDirPart(current, parent);
				if(parent.empty())
					break;

				bool needAdd = false;

				DirList::iterator parentIt;
				if(!findDir(parentIt, parent))
				{
					dirs[parent] = Dir();
					findDir(parentIt, parent);
					needAdd = true;
				}

				parentIt->second.subDirs.push_back(currentIt);
				if(!needAdd)
					break;

				current = parent;
				currentIt = parentIt;
			}
		}
	}

	bool findDir(DirList::iterator &it, const string &dir)
	{
		it = dirs.find(dir);
		if(it == dirs.end())
			return false;

		return true;
	}
};

FileList::FileList() :
	data(NULL)
{
	scoped_ptr<Data> tempData(new Data());
	data.swap(tempData);
}

FileList::~FileList()
{
}

void FileList::setCaseSensitivity(bool enable)
{
	data->caseSensitive = enable;
}

void FileList::addDir(const std::string &dir_)
{
	string dir = dir_;
	if(!data->caseSensitive)
		convertLower(dir);

	haxFile(dir);
	data->addDir(dir);
}

void FileList::addFile(const std::string &file_)
{
	string file = file_;
	if(!data->caseSensitive)
		convertLower(file);
	haxFile(file);

	string dir;
	getDirPart(file, dir);
	data->addDir(dir);

	DirList::iterator it;
	data->findDir(it, dir);
	it->second.addFile(file);
}

int FileList::getDirAmount(const std::string &root_) const
{
	string root = root_;
	if(!data->caseSensitive)
		convertLower(root);

	haxFile(root);
	DirList::iterator it;
	if(!data->findDir(it, root))
		return 0;

	return getDirAmountImp(it->second.subDirs);
}

std::string FileList::getDirName(const std::string &root_, int index) const
{
	string root = root_;
	if(!data->caseSensitive)
		convertLower(root);

	haxFile(root);
	DirList::iterator it;
	if(!data->findDir(it, root))
		return empty;

	string result;
	getDirNameImp(it->second.subDirs, result, 0, index);

	return result;
}

int FileList::getFileAmount(const std::string &root_) const
{
	string root = root_;
	if(!data->caseSensitive)
		convertLower(root);
	haxFile(root);

	DirList::iterator it;
	if(!data->findDir(it, root))
		return 0;

	return it->second.files.size();
}

const std::string &FileList::getFileName(const std::string &root_, int index) const
{
	string root = root_;
	if(!data->caseSensitive)
		convertLower(root);
	haxFile(root);

	DirList::iterator it;
	if(!data->findDir(it, root))
		return empty;

	return it->second.files[index];
}

// ---------

namespace {

	void getAllFilesImp(IFileList &list, const std::string &root, std::vector<std::string> &result, bool recurse)
	{
		//if(root.find("data/textures/human_weapons") != root.npos)
		//	int a = 0;

		int fileAmount = list.getFileAmount(root);
		for(int i = 0; i < fileAmount; ++i)
		{
			const std::string &file = list.getFileName(root,i);
			result.push_back(file);
		}

		if(recurse)
		{
			int dirAmount = list.getDirAmount(root);
			for(int i = 0; i < dirAmount; ++i)
			{
				const std::string &dir = list.getDirName(root, i);
				if (dir.length() > 4 && dir.substr(dir.length() - 4, 4) == ".svn")
				{
					// nop
				} else {
					getAllFilesImp(list, dir, result, recurse);
				}
			}
		}
	}

	void getAllDirsImp(IFileList &list, const std::string &root, std::vector<std::string> &result, bool recurse)
	{
		int dirAmount = list.getDirAmount(root);
		for(int i = 0; i < dirAmount; ++i)
		{
			const std::string &dir = list.getDirName(root, i);
			if (dir.length() > 4 && dir.substr(dir.length() - 4, 4) == ".svn")
			{
				// nop
			} else {
				result.push_back(dir);

				if(recurse)
					getAllDirsImp(list, dir, result, recurse);
			}
		}
	}

} // unnamed

void getAllFiles(IFileList &list, const std::string &root_, std::vector<std::string> &result, bool recurse, bool caseSensitive)
{
	string root = root_;
	if(!caseSensitive)
		convertLower(root);
	haxFile(root);

	getAllFilesImp(list, root, result, recurse);
	std::sort(result.begin(), result.end());
}

void getAllDirs(IFileList &list, const std::string &root_, std::vector<std::string> &result, bool recurse, bool caseSensitive)
{
	string root = root_;
	if(!caseSensitive)
		convertLower(root);
	haxFile(root);

	getAllDirsImp(list, root, result, recurse);
	std::sort(result.begin(), result.end());
}

} // end of namespace filesystem
} // end of namespace frozenbyte

