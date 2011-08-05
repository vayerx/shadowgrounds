#include "precompiled.h"

#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)

#pragma warning(disable: 4786)
#endif

#include "file_wrapper.h"
#include "common_dialog.h" // should be reverse dep
#include "FindFileWrapper.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/ifile_list.h"
#include "../filesystem/input_stream_wrapper.h"

// for profiling...
#include "../system/Timer.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

using namespace std;
using namespace frozenbyte::filesystem;

namespace frozenbyte {
namespace editor {

	// hacking to get things work with STLport 5.1.0-RC3 and later... (intrinsic std::vector impl)
	// TODO: make sure this does not leak.
	struct Dir;
	struct DirInternal : public boost::noncopyable
	{
		Dir *dir;
		DirInternal(Dir *p_dir);
		~DirInternal();
		DirInternal(const DirInternal &d);
		const DirInternal& operator= (const DirInternal &d);
	};

	struct Dir
	{
		std::string name;

		std::vector<DirInternal> dirs;
		std::vector<std::string> files;		

		Dir()
		{
			// nop?
		}

		Dir(const Dir &d)
		{
			this->name = d.name;
			this->files = d.files;
			this->dirs = d.dirs;
		}
		const Dir& operator= (const Dir &d)
		{
			this->name = d.name;
			this->files = d.files;
			this->dirs = d.dirs;
			return *this;
		}
	};

	DirInternal::DirInternal(Dir *p_dir) 
			: dir(new Dir(*p_dir))
	{
	}
	DirInternal::~DirInternal()
	{
		delete dir;
		dir = NULL;
	}
	DirInternal::DirInternal(const DirInternal &d)
	{
		dir = new Dir(*d.dir);		
	}
	const DirInternal& DirInternal::operator= (const DirInternal &d)
	{
		if (dir != NULL)
			delete dir;
		dir = new Dir(*d.dir);
		return *this;
	}

	static std::string findFile(const Dir &dir, const std::string &fileName)
	{
		for(unsigned int i = 0; i < dir.files.size(); ++i)
		{
			if(fileName == getFileName(dir.files[i]))
				return dir.files[i];
		}

		for(unsigned int j = 0; j < dir.dirs.size(); ++j)
		{
			std::string result = findFile(*dir.dirs[j].dir, fileName);
			if(!result.empty())
				return result;
		}

		return "";
	}


// turol: do these belong here?
// probably not. FIXME

std::string getFileName(const std::string &fullFileName)
{
	for(int i = fullFileName.size() - 1; i >= 0; --i)
	{
		if(fullFileName[i] == '\\')
		{
			++i;
			return fullFileName.substr(i, fullFileName.size() - i);
		}
	}

	return fullFileName;
}

std::string getDirName(const std::string &fullFileName)
{
	for(int i = fullFileName.size() - 1; i >= 0; --i)
	{
		if(fullFileName[i] == '\\')
		{
			++i;
			return fullFileName.substr(0, i);
		}
	}

	return fullFileName;
}

bool fileExists(const std::string &fileName)
{
	if(std::ifstream(fileName.c_str()))
		return true;

	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(fileName);
	return stream.getSize() > 0;
}


struct FileWrapper::Data
{
	Dir root;
	bool caseSensitive;

	Data(const std::string &root, const std::string &extension, bool caseSensitive_)
	:	caseSensitive(caseSensitive_)
	{
		iterateDirs(root, extension);
		iterateFiles(root, extension);
	}

	void iterateDirs(const std::string &root, const std::string &extension)
	{
		boost::shared_ptr<IFileList> list = FilePackageManager::getInstance().findFiles(root, extension, caseSensitive);
		if(!list)
			return;

		std::vector<std::string> fileList;
		filesystem::getAllDirs(*list, root, fileList, true, caseSensitive);

		for(unsigned int i = 0; i < fileList.size(); ++i)
		{
			const std::string &file = fileList[i];
			
			int start = root.size() + 1;
			std::string::size_type end = file.find_last_of("/");
			if(end == file.npos)
				continue;

			//std::string dirPath = file.substr(start, end - start);
			std::string dirPath = file.substr(start, file.size() - start);
			Dir *activeDir = &this->root;

			for(;;)
			{
				std::string::size_type index = dirPath.find_first_of("/");
				std::string dir = (index != dir.npos) ? dirPath.substr(0, index) : dirPath;

				bool found = false;
				for(unsigned int j = 0; j < activeDir->dirs.size(); ++j)
				{
					if(activeDir->dirs[j].dir->name == dir)
					{
						activeDir = activeDir->dirs[j].dir;
						found = true;
						break;
					}
				}

				if(!found)
				{
					Dir newDir;
					newDir.name = dir;

					activeDir->dirs.push_back(DirInternal(&newDir));
					activeDir = activeDir->dirs[activeDir->dirs.size() - 1].dir;
				}

				if(dir.size() == dirPath.size())
					break;

				dirPath = dirPath.substr(index + 1, dirPath.size() - index - 1);
			}
		}
	}


	void iterateFiles(const std::string &root, const std::string &extension)
	{
		boost::shared_ptr<IFileList> list = FilePackageManager::getInstance().findFiles(root, extension, caseSensitive);
		if(!list)
			return;

		std::vector<std::string> fileList;
		filesystem::getAllFiles(*list, root, fileList, true, caseSensitive);

// TEST HACK: fbem modifier files..
if (extension == "*.s3d")
{
	std::vector<std::string> fbemList;
	for(unsigned int t = 0; t < fileList.size(); ++t)
	{
		const std::string &file = fileList[t];
		std::string tmpname = file + ".fbem";
		FILE *tmpf = fopen(tmpname.c_str(), "rb");
		if (tmpf != NULL)
		{
			fseek(tmpf, 0, SEEK_END);
			int fsize = ftell(tmpf);
			fseek(tmpf, 0, SEEK_SET);

			if (fsize > 0)
			{
				char *buf = new char[fsize + 1];
				int got = fread(buf, fsize, 1, tmpf);
				buf[fsize] = '\0';
				if (got == 1)
				{
					int lastPos = 0;
					for (int i = 0; i < fsize+1; i++)
					{
						if (buf[i] == '\r' || buf[i] == '\n' || buf[i] == '\0')
						{
							if (i > lastPos)
							{
								buf[i] = '\0';
								tmpname = file + &buf[lastPos];
								fbemList.push_back(tmpname);							
							}
							lastPos = i + 1;
						}
					}
				}
				delete [] buf;
			}
			fclose(tmpf);
		}
	}
	for(unsigned int t = 0; t < fbemList.size(); ++t)
	{
		fileList.push_back(fbemList[t]);
	}
}

		for(unsigned int i = 0; i < fileList.size(); ++i)
		{
			const std::string &file = fileList[i];

			int start = root.size() + 1;
			std::string::size_type end = file.find_last_of("/");
			if(end == file.npos)
				continue;

			std::string dirPath = file.substr(start, end - start);
			Dir *activeDir = &this->root;

			for(;;)
			{
				/*
				int index = dirPath.find_first_of("/");
				if(index == dirPath.npos)
				{
					std::string foo = file;
					for(unsigned int k = 0; k < foo.size(); ++k)
						if(foo[k] == '/')
							foo[k] = '\\';

					activeDir->files.push_back(foo);
					break;			
				}
				*/

				std::string::size_type index = dirPath.find_first_of("/");
				std::string dir = (index != dir.npos) ? dirPath.substr(0, index) : dirPath;

				bool found = false;
				for(unsigned int j = 0; j < activeDir->dirs.size(); ++j)
				{
					if(activeDir->dirs[j].dir->name == dir)
					{
						activeDir = activeDir->dirs[j].dir;
						found = true;
						break;
					}
				}

				if(!found)
				{
					break;
				}

				if(dir.size() == dirPath.size())
				{
					std::string foo = file;
					for(unsigned int k = 0; k < foo.size(); ++k)
						if(foo[k] == '/')
							foo[k] = '\\';

					activeDir->files.push_back(foo);

					break;
				}

				dirPath = dirPath.substr(index + 1, dirPath.size() - index - 1);
			}
		}
	}


	void getFiles(const Dir &dir, vector<string> &result)
	{
		for(unsigned int i = 0; i < dir.files.size(); ++i)
		{
			const string &file = dir.files[i];
			result.push_back(file);
		}

		for(unsigned int j = 0; j < dir.dirs.size(); ++j)
			getFiles(*dir.dirs[j].dir, result);
	}


	void getDirs(const Dir &dir, vector<string> &result)
	{
		for(unsigned int i = 0; i < dir.dirs.size(); ++i)
		{
			const string &tmp = dir.dirs[i].dir->name;
			result.push_back(tmp);
		}

		// NOTE: does not recurse subdirectories!
		//for(unsigned int j = 0; j < dir.dirs.size(); ++j)
		//	getDirs(dir.dirs[j], result);
	}
};

FileWrapper::FileWrapper(const std::string &dir, const std::string &extension, bool caseSensitive)
{
	/*
	Timer::update();
	int startTime = Timer::getUnfactoredTime();
	*/

	boost::scoped_ptr<Data> tempData(new Data(dir, extension, caseSensitive));
	data.swap(tempData);

	/*
	int endTime = Timer::getUnfactoredTime();
	Logger::getInstance()->debug("FileWrapper - Time used constructing data structures follows (in msec):");
	Logger::getInstance()->debug(int2str(endTime - startTime));
	*/
}

FileWrapper::~FileWrapper()
{
}

int FileWrapper::getRootDirAmount() const
{
	return data->root.dirs.size();
}

const std::string &FileWrapper::getRootDir(int index) const
{
#ifndef NDEBUG
	Dir &root = data->root;
	assert(index < int(root.dirs.size()));
#endif

	return data->root.dirs[index].dir->name;
}

int FileWrapper::getDirAmount(int rootIndex) const
{
	Dir &root = data->root;
	assert(rootIndex < int(root.dirs.size()));

	return root.dirs[rootIndex].dir->dirs.size();
}

const std::string &FileWrapper::getDir(int rootIndex, int index) const
{
	Dir &root = data->root;
	assert(rootIndex < int(root.dirs.size()));
	assert(index < int(root.dirs[rootIndex].dir->dirs.size()));

	return root.dirs[rootIndex].dir->dirs[index].dir->name;
}

int FileWrapper::getFileAmount(int rootIndex) const
{
	Dir &root = data->root;
	assert(rootIndex < int(root.dirs.size()));

	return root.dirs[rootIndex].dir->files.size();
}

const std::string &FileWrapper::getFile(int rootIndex, int fileIndex) const
{
	Dir &root = data->root;
	assert(rootIndex < int(root.dirs.size()));
	assert(fileIndex < int(root.dirs[rootIndex].dir->files.size()));

	return root.dirs[rootIndex].dir->files[fileIndex];
}

int FileWrapper::getFileAmount(int rootIndex, int dirIndex) const
{
	Dir &root = data->root;
	assert(rootIndex < int(root.dirs.size()));
	Dir &rootDir = *root.dirs[rootIndex].dir;
	assert(dirIndex < int(rootDir.dirs.size()));
	Dir &dir = *rootDir.dirs[dirIndex].dir;

	return dir.files.size();
}

const std::string &FileWrapper::getFile(int rootIndex, int dirIndex, int fileIndex) const
{
	Dir &root = data->root;
	assert(rootIndex < int(root.dirs.size()));
	Dir &rootDir = *root.dirs[rootIndex].dir;
	assert(dirIndex < int(rootDir.dirs.size()));
	Dir &dir = *rootDir.dirs[dirIndex].dir;
	assert(fileIndex < int(dir.files.size()));

	return dir.files[fileIndex];
}

vector<string> FileWrapper::getAllFiles() const
{
	vector<string> files;
	data->getFiles(data->root, files);
  // TODO: Hax to fix deleting of player profiles
	for(unsigned int i=0;i<files.size();++i)
		for(unsigned int j=0;j<files[i].length();++j)
			if (files[i][j] == '\\') files[i][j] = '/';

	return files;
}

vector<string> FileWrapper::getAllDirs() const
{
	vector<string> dirs;
	data->getDirs(data->root, dirs);

	return dirs;
}

std::string FileWrapper::resolveModelName(const std::string &rootDir, const std::string &fileName)
{
	if(fileExists(fileName))
		return fileName;

	FileWrapper files(rootDir, "*.s3d");
	std::string result = findFile(files.data->root, getFileName(fileName));
	if(!result.empty())
		return result;

	std::string file = fileName.substr(fileName.find_last_of('\\') + 1);
	boost::shared_ptr<IFileList> fileList = filesystem::FilePackageManager::getInstance().findFiles(rootDir, "*" + file);
	int dirs = fileList->getDirAmount(rootDir);
	for(int i = 0; i < dirs; i++)
	{
		std::string dir = fileList->getDirName(rootDir, i);
		if(fileList->getFileAmount(dir) > 0)
		{
			return fileList->getFileName(dir, 0);
		}
	}

	return fileName;
}

bool fileExists(const char *name)
{
	/*
	if(!name)
		return false;

	filesystem::FB_FILE *fp = filesystem::fb_fopen(name, "rb");
	if(fp == 0)
		return false;

	filesystem::fb_fclose(fp);
	return true;
	*/
	if(!name)
		return false;

	FILE *fp = fopen(name, "rb");
	if(fp != 0)
	{
		return true;
		fclose(fp);
	}

	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(name);
	return stream.getSize() > 0;
}

} // editor
} // frozenbyte
