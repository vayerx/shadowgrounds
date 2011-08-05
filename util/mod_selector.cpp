#include "precompiled.h"

#include "mod_selector.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/output_file_stream.h"
#include "../filesystem/ifile_package.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/zip_package.h"
#include "../filesystem/file_list.h"
#include "../filesystem/standard_package.h"
#include "../editor/parser.h"
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

#ifdef WIN32
#include <windows.h>

#else
#include <unistd.h>

static inline int GetCurrentDirectory(int size, char *buf) {
#ifdef __APPLE__
	return strlen(getcwd(buf, size));
#else
	return strnlen(getcwd(buf, size), size);
#endif
}

#define SetCurrentDirectory(dir) chdir((dir))

#endif

using namespace frozenbyte;
namespace util {

struct ModSelector::Data
{
	char workingDir[1024];

	std::string active;
	int activeIndex;

	std::map<std::string, std::string> modList;
	std::vector<std::string> modStringList;

	Data()
	:	activeIndex(-1)
	{
		GetCurrentDirectory(sizeof(workingDir), workingDir);
		findMods();
		findActive();
	}

	void findMods()
	{
		const std::string root = "Mods";

		filesystem::StandardPackage files;
		filesystem::FileList fileList;
		files.findFiles(root, "*.*", fileList);
		//files.findFiles(root, "*.zip", fileList);

		modList.clear();
		modStringList.clear();

		for(int i = 0; i < fileList.getDirAmount(root); ++i)
		{
			const std::string &fullName = fileList.getDirName(root, i);
			std::string dirName;
			std::string::size_type index = fullName.find_first_of("/\\");
			if(index != fullName.npos)
				dirName = fullName.substr(index + 1, fullName.size() - index - 1);

			filesystem::InputStream stream = filesystem::createInputFileStream(fullName + std::string("/description.txt"));
			if(stream.isEof())
				continue;

			editor::EditorParser parser;
			stream >> parser;

			const editor::ParserGroup &group = parser.getGlobals();
			if(group.getLineCount() < 1)
				continue;

			std::string name = group.getLine(0);
			name = name.c_str();
			if(name.empty())
				continue;

			modList[name] = dirName;
			modStringList.push_back(name);
		}

		std::sort(modStringList.begin(), modStringList.end());
	}

	void findActive()
	{
#ifdef LEGACY_FILES
		filesystem::InputStream stream = filesystem::createInputFileStream("Mods/active.txt");
#else
		filesystem::InputStream stream = filesystem::createInputFileStream("mods/active.txt");
#endif

		editor::EditorParser parser;
		stream >> parser;

		const editor::ParserGroup &group = parser.getGlobals();
		if(group.getLineCount() < 1)
			return;

		std::string name = group.getLine(0);
		if(name.empty())
			return;

		active = name.c_str();

		std::map<std::string, std::string>::iterator it = modList.begin();
		for(; it != modList.end(); ++it)
		{
			if(strcmp(it->second.c_str(), active.c_str()) == 0)
			{
				for(unsigned int i = 0; i < modStringList.size(); ++i)
				{
					if(strcmp(modStringList[i].c_str(), it->first.c_str()) == 0)
					{
						activeIndex = i;
						return;
					}
				}
			}
		}
	}

	void saveActive(const std::string &file)
	{
		std::string txtName = workingDir;
#ifdef LEGACY_FILES
		txtName += "\\Mods\\Active.txt";
#else
		txtName += "\\mods\\active.txt";
#endif

		filesystem::OutputStream stream = filesystem::createOutputFileStream(txtName);
		//stream << file;

		for(unsigned int i = 0; i < file.size(); ++i)
			stream << (unsigned char)(file[i]);
	}

	void restoreDir()
	{
		SetCurrentDirectory(workingDir);
	}

	void changeDir()
	{
		if(active.empty())
			return;

		std::string dir = workingDir;
		dir += "\\Mods\\";
		dir += active;

		{
			const char *ptr = dir.c_str();
			if(!SetCurrentDirectory(ptr))
			{
				active.clear();
				activeIndex = -1;
				return;
			}
		}

		dir += "\\";

		/*
		filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
		boost::shared_ptr<filesystem::IFilePackage> zipPackage1(new filesystem::ZipPackage(dir + std::string("data1.fbz")));
		boost::shared_ptr<filesystem::IFilePackage> zipPackage2(new filesystem::ZipPackage(dir + std::string("data2.fbz")));
		boost::shared_ptr<filesystem::IFilePackage> zipPackage3(new filesystem::ZipPackage(dir + std::string("data3.fbz")));
		boost::shared_ptr<filesystem::IFilePackage> zipPackage4(new filesystem::ZipPackage(dir + std::string("data4.fbz")));
		manager.addPackage(zipPackage1, 11);
		manager.addPackage(zipPackage2, 12);
		manager.addPackage(zipPackage3, 13);
		manager.addPackage(zipPackage4, 14);
		*/
	}
};

ModSelector::ModSelector()
:	data(new Data())
{
}

ModSelector::~ModSelector()
{
}
/*
const std::string &ModSelector::getActiceModFile() const
{
	return data->active;
}
*/
int ModSelector::getActiveIndex() const
{
	return data->activeIndex;
}

int ModSelector::getModAmount() const
{
	return data->modStringList.size();
}

const std::string &ModSelector::getDescription(int index) const
{
	return data->modStringList[index];
}

const std::string &ModSelector::getModDir(int index) const
{
	return data->modList[data->modStringList[index]];
}

void ModSelector::fixFileName(std::string &file) const
{
	if(data->activeIndex < 0)
		return;

	std::string base = "mods\\" + getModDir(data->activeIndex) + "\\";
	if(base.length() < file.length())
	{
		for(unsigned int i = 0; i < base.size(); ++i)
		{
			if(base[i] != file[i])
				return;
		}

		file = file.substr(base.size(), file.size() - base.size());
	}
}

void ModSelector::saveActiveModFile(int index)
{
	std::string mod;
	if(index >= 0)
		mod = data->modList[data->modStringList[index]];

	data->saveActive(mod);
}

void ModSelector::restoreDir()
{
	data->restoreDir();
}

void ModSelector::changeDir()
{
	data->changeDir();
}

} // util
