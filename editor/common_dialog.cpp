#include "precompiled.h"

#include <string.h>
#include <windows.h>
#include <fstream>

// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "common_dialog.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/file_package_manager.h"

#pragma warning(disable: 4786)

namespace frozenbyte {
namespace editor {
namespace {
	char filter[1024] = "s3d files\0*.s3d\0\0";
	char filter2[1024];
	char fileName[2048] = { 0 };
	char currentDirectory[1024] = { 0 };

	class DirKeeper
	{
	public:
		DirKeeper()
		{
			GetCurrentDirectory(sizeof(currentDirectory), currentDirectory);
		}
		~DirKeeper()
		{
			SetCurrentDirectory(currentDirectory);
		}
	};

	char *createFilter(const std::string &extension)
	{		
		filter[0] = extension.c_str()[0];
		filter[1] = extension.c_str()[1];
		filter[2] = extension.c_str()[2];

		filter[12] = extension.c_str()[0];
		filter[13] = extension.c_str()[1];
		filter[14] = extension.c_str()[2];
		return filter;
	}

	char *createFileName()
	{
		fileName[0] = '\0';
		return fileName;
	}

	OPENFILENAME createStruct(const std::string &extension, const std::string &defaultDir, bool useFilter)
	{
		OPENFILENAME fileStruct = { 0 };
		fileStruct.lStructSize = sizeof(OPENFILENAME);

		if(!useFilter) 
		{
			memset(filter2, 0, sizeof(filter2));
			for(unsigned int i = 0; i < extension.size(); ++i)
				filter2[i] = extension[i];

			fileStruct.lpstrFilter = filter2;
		} 
		else
			fileStruct.lpstrFilter = createFilter(extension);
		
		fileStruct.lpstrFile = createFileName();
		fileStruct.nMaxFile = sizeof(fileName);
		fileStruct.lpstrInitialDir = defaultDir.c_str();

		
		return fileStruct;
	}

	std::string getRelativeFileName(std::string fullFileName)
	{
		std::string workingDirectory = currentDirectory;

		int workingSize = workingDirectory.size();
		int fullSize = fullFileName.size();

		{
			for(unsigned int i = 0; i < fullFileName.size(); ++i)
				fullFileName[i] = tolower(fullFileName[i]);
		}
		{
			for(unsigned int i = 0; i < workingDirectory.size(); ++i)
				workingDirectory[i] = tolower(workingDirectory[i]);
		}

		if((workingSize < fullSize) && (fullFileName.substr(0, workingSize) == workingDirectory))
			return fullFileName.substr(workingSize + 1, fullSize - workingSize - 1);

		return fullFileName;
	}
}

std::string getSaveFileName(const std::string &extension, const std::string &defaultDir, bool useFilter)
{
	DirKeeper keepDirectory;

	OPENFILENAME fileStruct = createStruct(extension, defaultDir, useFilter);
	if(GetSaveFileName(&fileStruct))
	{
		std::string fn = fileStruct.lpstrFile;

		if(useFilter)
		{
			std::string ext = std::string(".") + extension;
			if(fn.substr(fn.size() - 4, 4) != ext)
				fn += ext;
		}
		else
		{
			std::string ext = std::string(".foo");
			unsigned int i1 = extension.find_first_of(" ");
			if(i1 != extension.npos)
				ext = std::string(".") + extension.substr(0, i1);

			int i2 = fn.find_last_of(".");
			if(i2 == fn.npos)
				i2 = fn.size();

			fn = fn.substr(0, i2);
			fn += ext;
		}

		return getRelativeFileName(fn);
	}

	return "";
}

std::string getOpenFileName(const std::string &extension, const std::string &defaultDir, bool useFilter)
{
	DirKeeper keepDirectory;

	OPENFILENAME fileStruct = createStruct(extension, defaultDir, useFilter);
	if(GetOpenFileName(&fileStruct))
	{
		std::string fileName = fileStruct.lpstrFile;
		if(fileExists(fileName))
			return getRelativeFileName(fileName);
	}

	return "";
}


std::vector<std::string> getMultipleOpenFileName(const std::string &extension, const std::string &defaultDir)
{
	DirKeeper keepDirectory;

	OPENFILENAME fileStruct = createStruct(extension, defaultDir, true);
	fileStruct.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&fileStruct))
	{
		std::vector<std::string> fileNames;
		std::string directory = getRelativeFileName(fileStruct.lpstrFile);
		
		if(fileExists(directory))
		{
			fileNames.push_back(directory);
			return fileNames;
		}

		directory += '\\';

		const char *p = fileStruct.lpstrFile;
		int fileBegin = std::string(fileStruct.lpstrFile).size() + 1;
		int position = fileBegin;

		for(;;)
		{
			char c = p[position++];
			if(c != '\0')
				continue;

			std::string fileName = directory;;
			for(int i = fileBegin; i < position - 1; ++i)
				fileName.push_back(p[i]);

			if(fileExists(fileName))
				fileNames.push_back(fileName);

			fileBegin = position;
			if(p[position] == '\0')
				break;
		}

		return fileNames;
	}

	return std::vector<std::string> ();
}

} // end of namespace editor
} // end of namespace frozenbyte
