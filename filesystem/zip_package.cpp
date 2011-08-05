
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "zip_package.h"
#include "empty_buffer.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"
#include "ifile_list.h"

#include "detail/unzip.h"
#ifdef _MSC_VER
#pragma comment(lib, "zlib.lib")
#endif

#include <map>
#include <stack>
#include <vector>
#include "../util/Debug_MemoryManager.h"

namespace frozenbyte {
namespace filesystem {


	static const int BUFFER_SIZE = 100 * 1024;

	// ---

	static std::string::size_type findTokenIndex(const std::string &str, const std::string &token, std::string::size_type start)
	{
		if(token.size() >= str.size() - start)
			return str.npos;

		for(std::string::size_type i = start; i <= str.size() - token.size(); ++i)
		{
			bool found = true;
			for(std::string::size_type j = 0; j < token.size(); ++j)
			{
				if(token[j] != str[i + j])
				{
					found =  false;
					break;
				}
			}

			if(found)
				return i;
		}

		return str.npos;
	}

	static bool containsToken(const std::string &string, const std::string &token, std::string::size_type start)
	{
		if(start + token.size() >= string.size())
			return false;

		for(unsigned int i = 0; i < token.size(); ++i)
		{
			if(token[i] != string[i + start])
				return false;
		}

		return true;
	}

	static void findTokens(const std::string &string, std::vector<std::string> &result, const std::string &separator)
	{
		std::string::size_type start = string.find_first_of(separator);
		if(start == string.npos)
		{
			result.push_back(string);
			return;
		}

		for(;;)
		{
			std::string::size_type end = string.find_first_of(separator, start + 1);
			if(end == string.npos)
			{
				std::string token = string.substr(start + 1, string.size() - start - 1);
				result.push_back(token);
				break;
			}

			std::string token = string.substr(start, end - start);
			result.push_back(token);

			start = end;
		}
	}

	/*
	void addFile(const std::string &file, IFileList &result)
	{
		std::vector<std::string> dirs;
		findTokens(file, dirs, "/");

		std::stack<FileList::Dir *> dirStack;
		dirStack.push(&result.root);

		for(unsigned int i = 0; i < dirs.size() - 1; ++i)
		{
			const std::string &dirName = dirs[i];

			FileList::Dir *current = dirStack.top();
			int index = current->findDirIndex(dirName);
			if(index == -1)
			{
				FileList::Dir newDir;
				newDir.name = dirName;
				current->dirs.push_back(newDir);
				index = current->dirs.size() - 1;
			}

			dirStack.push(&current->dirs[index]);
		}

		FileList::Dir *final = dirStack.top();
		final->files.push_back(dirs[dirs.size() - 1]);
	}
	*/

	struct ZipFileData
	{
		int size;
		unz_file_pos filePosition;

		std::string filename;
		int crc;

		ZipFileData()
		:	size(0)
		{
		}
	};

	typedef std::map<std::string, ZipFileData> ZipFileList;

	struct ZipData
	{
		unzFile fileId;
		ZipFileList fileList;

		ZipData(const std::string &archive)
		:	fileId(0)
		{
			fileId = unzOpen(archive.c_str());
			if(fileId)
				findFiles();
		};

		~ZipData()
		{
			if(fileId)
				unzClose(fileId);
		}

		void findFiles()
		{
			unz_global_info globalInfo = { 0 };
			if(unzGetGlobalInfo(fileId, &globalInfo) != UNZ_OK)
				return;

			char file[1024] = { 0 };
			for(unsigned int i = 0; i < globalInfo.number_entry; ++i)
			{
				if(i != 0 && unzGoToNextFile(fileId) != UNZ_OK)
					break;

				unz_file_info fileInfo = { 0 };
				if(unzGetCurrentFileInfo(fileId, &fileInfo, file, sizeof(file) - 1, 0, 0, 0, 0) != UNZ_OK)
					return;
				if(fileInfo.uncompressed_size <= 0)
					continue;

				ZipFileData zipFile;
				zipFile.filename = file;
				zipFile.size = fileInfo.uncompressed_size;
				zipFile.crc = fileInfo.crc;
				unzGetFilePos(fileId, &zipFile.filePosition);

				std::string filename = file;
				convertLower(filename);
				fileList[filename] = zipFile;
			}
		}

		void findFiles(std::string dir, std::string extension, IFileList &result)
		{
			convertLower(dir);
			convertLower(extension);

			if(extension.empty())
				return;

			std::vector<std::string> tokens;
			findTokens(extension, tokens, "*");
			if(tokens.empty() || tokens.size() > 1)
				return;

			ZipFileList::iterator it = fileList.begin();
			for(; it != fileList.end(); ++it)
			{
				const std::string &file = it->first;				
				if(!containsToken(file, dir, 0))
					continue;

				//if(containsToken(file, "data/textures/human_weapons", 0))
					//	int a = 0;

				std::string::size_type fileStart = file.find_last_of("/");
				std::string::size_type index = findTokenIndex(file, tokens[0], fileStart);

				// FIXME!! 
				// This only detects search tokens which begin with "*"

				if(index == file.size() - tokens[0].size())
				{
					result.addFile(it->second.filename);
				}

				/*
				// Crap. Just find each token index (if has) and move those forward hierarchically if needed

				if(!containsToken(file, dir, 0))
					continue;
				//if(startAny && extension.size() == 1)
				//	addFile(file, result);

				int startIndexFrom = 0;
				int startIndexTo = 1;
				if(startAny)
					startIndexTo = file.size();

				for(int i = startIndexFrom; i != startIndexTo; ++i)
				{
					int start = i;
					if(!containsToken(file, tokens[0], start))
						continue;

					start += tokens[0].size();
					if(!endAny && start < int(file.size()))
						continue;

					//addFile(file, result);
				}
				*/
			}
		}

		bool findFile(std::string file, ZipFileList::iterator &it)
		{
			convertLower(file);

			it = fileList.find(file);
			
			/*
			ZipFileList::iterator iter;
			int t = file.find("pictures/startup.dds", 0);
			if( t != std::string::npos )
			{
				for( iter = fileList.begin(); iter != fileList.end(); iter++ )
				{
					igiosWarning("%s \n", iter->first);
				}

				igiosWarning("========================================================\n");
			}
			*/

			if(it == fileList.end())
				return false;

			return true;
		}
	};


#if READ_CHUNKS
	class ZipInputBuffer: public IInputStreamBuffer
	{
		boost::shared_ptr<ZipData> zipData;
		unsigned char buffer[BUFFER_SIZE];
		int currentPosition;
		int readBytes;
		unsigned long offset;

		ZipFileData fileData;

		/*
		void release() const
		{
			if(hasCurrentFile)
				unzCloseCurrentFile(zipData->fileId);
			//if(fileId)
			//	unzClose(fileId);

			hasCurrentFile = false;
		}

		bool fillBuffer() const
		{
			beginTime();

			if(currentPosition == -1 || currentPosition == BUFFER_SIZE)
			{
				int readSize = BUFFER_SIZE;
				if(currentPosition + BUFFER_SIZE > fileData.size)
					readSize = fileData.size - currentPosition;

				readBytes = unzReadCurrentFile(zipData->fileId, buffer, readSize);
				if(readBytes <= 0)
				{
					release();
					return false;
				}
	
				currentPosition = 0;
			}

			endTime("fillBuffer");
			return true;
		}
		*/

		void fillBuffer()
		{
			beginTime();

			int readSize = BUFFER_SIZE;
			if(readBytes + readSize >= fileData.size)
				readSize = fileData.size - readBytes;

			currentPosition = 0;

			int id = unzGoToFilePos(zipData->fileId, &fileData.filePosition);
			if(id != UNZ_OK)
				return;
			if(unzOpenCurrentFile(zipData->fileId) != UNZ_OK)
				return;
			
			if(offset > 0 && if(unzSetOffset(zipData->fileId, readBytes) != UNZ_OK)
				return;

			unzReadCurrentFile(zipData->fileId, buffer, readSize);
			//offset = unzGetOffset(zipData->fileId);
			unzCloseCurrentFile(zipData->fileId);

			endTime("fillBuffer");
		}

	public:
		ZipInputBuffer(boost::shared_ptr<ZipData> &zipData_, const ZipFileData &fileData_)
		:	zipData(zipData_),
			fileData(fileData_)
		{
			currentPosition = -1;
			readBytes = 0;
			offset = 0;

			/*
			if(zipData->fileId)
			{
				beginTime();
				int id = unzGoToFilePos(zipData->fileId, &fileData.filePosition);
				if(id != UNZ_OK)
				{
					release();
					return;
				}

				if(unzOpenCurrentFile(zipData->fileId) != UNZ_OK)
				{
					release();
					return;
				}

				hasCurrentFile = true;
				endTime("Locate file in zip");
			}
			*/
		}

		~ZipInputBuffer()
		{
		}

		unsigned char popByte()
		{
			if(!zipData->fileId || readBytes >= fileData.size)
				return 0;

			if(currentPosition == -1 || currentPosition >= BUFFER_SIZE)
				fillBuffer();

			++readBytes;
			return buffer[currentPosition++];
		}

		bool isEof() const
		{
			if(!zipData->fileId || readBytes >= fileData.size)
				return true;

			return false;
		}

		int getSize() const
		{
			return fileData.size;
		}

		void popBytes(char *buffer, int bytes)
		{
			for(int i = 0; i < bytes; ++i)
				buffer[i] = popByte();
		}
	};

#else

	class ZipInputBuffer: public IInputStreamBuffer
	{
		boost::shared_ptr<ZipData> zipData;
		std::vector<unsigned char> buffer;
		int position;

		ZipFileData fileData;

		void fillBuffer()
		{
			int readSize = fileData.size;
			if (buffer.size() < readSize)
				buffer.resize(readSize);

			if(unzGoToFilePos(zipData->fileId, &fileData.filePosition) != UNZ_OK)
				return;
			if(unzOpenCurrentFile(zipData->fileId) != UNZ_OK)
				return;

#ifndef NDEBUG
			int bytes =
#endif
			unzReadCurrentFile(zipData->fileId, &buffer[0], readSize);
#ifndef NDEBUG
			// FIXME: this should not be assert but checked always
			// assert is only for programmer bugs, not runtime errors!
			assert(bytes == readSize);
#endif

			unzCloseCurrentFile(zipData->fileId);
		}

	public:
		ZipInputBuffer(boost::shared_ptr<ZipData> &zipData_, const ZipFileData &fileData_)
		:	zipData(zipData_),
			fileData(fileData_)
		{
			position = 0;
		}

		~ZipInputBuffer()
		{
		}

		unsigned char popByte()
		{
			if(position >= fileData.size)
				return 0;

			if(position == 0)
			{
				buffer.reserve(fileData.size);
				fillBuffer();
			}

			return buffer[position++];
		}

		bool isEof() const
		{
			if(position >= fileData.size)
				return true;

			return false;
		}

		int getCRC() const
		{
			return fileData.crc;
		}

		int getSize() const
		{
			return fileData.size;
		}

		void popBytes(char *buffer, int bytes)
		{
			for(int i = 0; i < bytes; ++i)
				buffer[i] = popByte();
		}
	};

#endif


struct ZipPackageData
{
	std::string archiveName;
	boost::shared_ptr<ZipData> zipData;

	ZipPackageData()
	{
	}

	void load()
	{
		zipData.reset(new ZipData(archiveName));
	}
};

ZipPackage::ZipPackage(const std::string &archiveName)
{
	boost::scoped_ptr<ZipPackageData> tempData(new ZipPackageData());
	tempData->archiveName = archiveName;
	tempData->load();
	data.swap(tempData);
}

ZipPackage::~ZipPackage()
{
}

void ZipPackage::findFiles(const std::string &dir, const std::string &extension, IFileList &result)
{
	data->zipData->findFiles(dir, extension, result);
}

InputStream ZipPackage::getFile(const std::string &fileName)
{
	InputStream inputStream;
	
	ZipFileList::iterator it;
	if(data->zipData->findFile(fileName, it))
	{
		boost::shared_ptr<ZipInputBuffer> inputBuffer(new ZipInputBuffer(data->zipData, it->second));
		inputStream.setBuffer(inputBuffer);
	}
	else
	{
		boost::shared_ptr<EmptyBuffer> inputBuffer(new EmptyBuffer());
		inputStream.setBuffer(inputBuffer);
	}

	return inputStream;
}


unsigned int ZipPackage::getCrc(const std::string &fileName)
{
	ZipFileList::iterator it;
	if(data->zipData->findFile(fileName, it))
	{
		return it->second.crc;
	}
	return 0;
}

} // end of namespace filesystem
} // end of namespace frozenbyte
