
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include "file_package_manager.h"
#include "empty_buffer.h"
#include "file_list.h"
#include <map>

#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"


// HACK: bad dependency (for error reporting...)
#include "input_file_stream.h"


namespace frozenbyte {
namespace filesystem {
namespace {

/*
	struct EmptyBuffer: public IInputStreamBuffer
	{
		unsigned char popByte()
		{
			return 0;
		}

		bool isEof() const
		{
			return true;
		}

		int getSize() const
		{
			return 0;
		}

		void popBytes(char *, int)
		{
		}
	};
*/

	typedef std::multimap<int, boost::shared_ptr<IFilePackage> > PackageMap;

	FilePackageManager instance;
	FilePackageManager *instancePtr = 0;
}

struct FilePackageManagerData
{
	PackageMap packages;
	bool logNonExisting;

	FilePackageManagerData()
		: logNonExisting(true)
	{
	}

	InputStream getFile(std::string fileName)
	{
		for(unsigned int i = 0; i < fileName.size(); ++i)
		{
			if(fileName[i] == '\\')
				fileName[i] = '/';
		}

		for(PackageMap::reverse_iterator it = packages.rbegin(); it != packages.rend(); ++it)
		{
			InputStream result = it->second->getFile(fileName);
			if(!result.isEof())
				return result;
		}

		// Not found, try again all lowercase
		for(unsigned int i = 0; i < fileName.size(); ++i)
		{
			if(isupper(fileName[i]))
				fileName[i] = tolower(fileName[i]);
		}

		for(PackageMap::reverse_iterator it = packages.rbegin(); it != packages.rend(); ++it)
		{
			InputStream result = it->second->getFile(fileName);
			if(!result.isEof())
				return result;
		}

		if (logNonExisting)
		{
			// NOTE: this may be a major problem.. cannot use logger directly inside storm
			// (as the instance would be different from the game's instance - unless used through a pointer given by the game)
			//::Logger::getInstance()->error("FilePackageManager::getFile - File does not exist or is zero length.");
			//::Logger::getInstance()->debug(fileName.c_str());
      // igiosWarning("FilePackageManager::getFile - File does not exist or is zero length. (%s)\n",fileName.c_str());
		}

		InputStream inputStream;
		boost::shared_ptr<EmptyBuffer> inputBuffer(new EmptyBuffer());

		inputStream.setBuffer(inputBuffer);
		return inputStream;		
	}

	unsigned int getCrc(std::string fileName)
	{
		for(unsigned int i = 0; i < fileName.size(); ++i)
		{
			if(fileName[i] == '\\')
				fileName[i] = '/';
		}

		for(PackageMap::reverse_iterator it = packages.rbegin(); it != packages.rend(); ++it)
		{
			unsigned int result = it->second->getCrc(fileName);
			if(result != 0)
				return result;
		}

		return 0;
	}
};

FilePackageManager::FilePackageManager()
{
	boost::scoped_ptr<FilePackageManagerData> tempData(new FilePackageManagerData());
	data.swap(tempData);
}

FilePackageManager::~FilePackageManager()
{
}

void FilePackageManager::addPackage(boost::shared_ptr<IFilePackage> filePackage, int priority)
{
	std::pair<int, boost::shared_ptr<IFilePackage> > value(priority, filePackage);
	data->packages.insert(value);
}

boost::shared_ptr<IFileList> FilePackageManager::findFiles(const std::string &dir, const std::string &extension, bool caseSensitive)
{
	boost::shared_ptr<IFileList> result(new FileList());
	result->setCaseSensitivity(caseSensitive);

	for(PackageMap::iterator it = data->packages.begin(); it != data->packages.end(); ++it)
		it->second->findFiles(dir, extension, *result);

	return result;
}

InputStream FilePackageManager::getFile(const std::string &fileName)
{
	return data->getFile(fileName);
}

unsigned int FilePackageManager::getCrc(const std::string &fileName)
{
	return data->getCrc(fileName);
}

void FilePackageManager::setInputStreamErrorReporting(bool logNonExisting)
{
	// HACK: this goes directly to input file stream..
	//frozenbyte::filesystem::setInputStreamErrorReporting(logNonExisting);
	//data->logNonExisting = logNonExisting;
}


FilePackageManager &FilePackageManager::getInstance()
{
	if(!instancePtr)
		instancePtr = &instance;

	return *instancePtr;
}

void FilePackageManager::setInstancePtr(FilePackageManager *newInstance)
{
	assert(newInstance);
	instancePtr = newInstance;
	//FilePackageManager &oldInstance = getInstance();
	//oldInstance.data->packages = newInstance->data->packages;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
