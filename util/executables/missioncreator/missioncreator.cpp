
#include "precompiled.h"

#include <string>
#include <vector>
#include <stdio.h> // printf()
#include <process.h> // system() // hack!!


#include "../../../filesystem/file_package_manager.h"
#include "../../../filesystem/standard_package.h"
#include "../../../filesystem/ifile_list.h"

#include "../../TextFileModifier.h"

using namespace std;
using namespace frozenbyte;

vector <string> getFileList(const string &p_search_dir)
{
	// get files
	vector<string> allBaseFiles;
	filesystem::FilePackageManager &filesys = filesystem::FilePackageManager::getInstance();
	boost::shared_ptr<filesystem::IFileList> baseFiles = filesys.findFiles(p_search_dir, "*");
	filesystem::getAllFiles(*baseFiles, p_search_dir, allBaseFiles, false);
	//printf("Files found: %i\n", allBaseFiles.size());

	return allBaseFiles;
}

vector <string> getFileList_byTypes(const string &p_search_dir, const vector <string> &p_fileTypes)
{
	// get files
	vector <string> allBaseFiles;
	filesystem::FilePackageManager &filesys = filesystem::FilePackageManager::getInstance();
	for(unsigned i = 0; i < p_fileTypes.size(); i++)
	{
		boost::shared_ptr<filesystem::IFileList> baseFiles = filesys.findFiles(p_search_dir, "*."+ p_fileTypes.at(i));
		filesystem::getAllFiles(*baseFiles, p_search_dir, allBaseFiles, false);
	}
	//printf("Files found: %i\n", allBaseFiles.size());

	return allBaseFiles;
}

vector <string> getFileList_notTypes(const string &p_search_dir, const vector <string> &p_fileTypes)
{
	vector <string> allBaseFiles = getFileList(p_search_dir);
	vector <string> typedBaseFiles = getFileList_byTypes(p_search_dir, p_fileTypes);

	for(vector <string>::iterator iter = allBaseFiles.begin(); iter != allBaseFiles.end();)
	{
		bool was_erased = false;
		for(unsigned i = 0; i < typedBaseFiles.size(); i++)
		{
			string comparable = *iter;
			int findTmp = comparable.compare(typedBaseFiles.at(i));
			if(findTmp == 0)
			{
				allBaseFiles.erase(iter);
				was_erased = true;
			}
		}
		if(allBaseFiles.size() < 1) { break; }
		if(!was_erased)
			iter++;
	}

	return allBaseFiles;
}

vector <string> getDirList(const string &p_search_dir, const vector <string> &p_exclude)
{
	// get dirs
	vector<string> allBaseDirs;
	filesystem::FilePackageManager &filesys = filesystem::FilePackageManager::getInstance();
	boost::shared_ptr<filesystem::IFileList> baseFiles = filesys.findFiles(p_search_dir, "*");
	filesystem::getAllDirs(*baseFiles, p_search_dir, allBaseDirs, false);

	// exclude dirs
	vector <string>::iterator iter;
	iter = allBaseDirs.begin();
	for(unsigned i = 0; i < allBaseDirs.size(); i++, iter++)
	{
		for(unsigned j = 0; j < p_exclude.size(); j++)
		{
			int findTmp = allBaseDirs.at(i).find(p_exclude.at(j));
			if(findTmp >= 0)
			{
				allBaseDirs.erase(iter);
				break;
			}
		}
	}
	//printf("Dirs found: %i\n", allBaseDirs.size());

	return allBaseDirs;
}

bool convertFiles(const vector <string> &p_fileList, const string &p_copy_dir,\
				  const vector <string> &p_tagList, const vector <string> &p_convList, const string &p_filePrefix)
{
	if(p_tagList.size() != p_convList.size())
	{
		printf("Error, tag & conv counts don't match.\n");
		return false;
	}

	util::TextFileModifier textfilemod;
	for(int i = 0; i < (int)p_fileList.size(); i++)
	{
		string current_file = p_fileList.at(i);
		if(!textfilemod.loadFile(current_file.c_str()))
		{
			printf("\nError, file (%s) couldn't be opened.\n", current_file.c_str());
			return false;
		}

		// convert content
		for(unsigned i = 0; i < p_tagList.size(); i++)
		{
			textfilemod.replaceString(p_tagList.at(i).c_str(), p_convList.at(i).c_str(), false);
		}

		// get the filename
		int fileNamePos = current_file.rfind("\\"); // windows style
		if(fileNamePos < 0)
		{
			fileNamePos = current_file.rfind("/"); // unix (internal?) style
			if(fileNamePos < 0)
			{
				printf("\nError, file address (%s) couldn't be used.\n", current_file.c_str());
				return false;
			}
		}

		//string openedFilename = p_filePrefix + current_file.substr(fileNamePos+1);
		// new, don't just add the mission name in front of the filenames, first remove template name
		// --jpk
		string nameWithTemplate = current_file.substr(fileNamePos+1);
		string nameWithoutTemplate = nameWithTemplate;
		string openedFilename;
		if (nameWithTemplate.substr(0, 8) == "template")
		{
			nameWithoutTemplate = nameWithTemplate.substr(8);
			openedFilename = p_filePrefix + nameWithoutTemplate;
		} else {
			openedFilename = nameWithoutTemplate;
		}

		// save modified file to the new dir
		string saveAsPath = p_copy_dir +"/"+ openedFilename;

		if(!textfilemod.saveFileAs(saveAsPath.c_str()))
		{
			printf("\nError, file (%s) couldn't be saved to (%s).\n", openedFilename.c_str(), saveAsPath.c_str());
			return false;
		}
		textfilemod.closeFile();
	}
	return true;
}

// converts string to lower case
string str2lower(const string &p_text)
{
	string tmpText = "";
	for(unsigned i = 0; i < p_text.length(); i++)
	{
		tmpText += tolower(p_text.at(i));
	}
	return tmpText;
}

// converts string to upper case
string str2upper(const string &p_text)
{
	string tmpText = "";
	for(unsigned i = 0; i < p_text.length(); i++)
	{
		tmpText += toupper(p_text.at(i));
	}
	return tmpText;
}

// returns a string with all occurrences of search in subject replaced with the given replace value
string str_replace(const string &p_subject, const char p_search, const char p_replace)
{
	string tmpText = "";
	for(unsigned i = 0; i < p_subject.length(); i++)
	{
		if(p_subject.at(i) == p_search)
		{
			tmpText += p_replace;
		}
		else
		{
			tmpText += p_subject.at(i);
		}
	}
	return tmpText;
}

// returns a string with all occurrences of search in subject replaced with the given replace value
// (ignoring case)
string str_ireplace(const string &p_subject, const char p_search, const char p_replace)
{
	string tmpText = "";
	for(unsigned i = 0; i < p_subject.length(); i++)
	{
		if(tolower(p_subject.at(i)) == tolower(p_search))
		{
			tmpText += p_replace;
		}
		else
		{
			tmpText += p_subject.at(i);
		}
	}
	return tmpText;
}

bool convertDir(const string &p_mission_dir, const string &p_template_name,\
				const string &p_mission_name, const string &p_mission_dir_prefix, const string &p_mission_file_prefix,\
				const vector <string> &dirExcludes, const vector <string> &fileTypes,\
				const vector <string> &tagList, const vector <string> &convList, const string &p_current_dir)
{
	string current_dir = p_current_dir;
	if(!current_dir.empty()) { current_dir = "/"+ current_dir; } // no extra slashes for the first dirs

	// current template dir
	string current_template_dir = p_mission_dir +"/"+ p_template_name + current_dir;

	// dir to copy new files (+ added prefix)
	string current_copy_dir = p_mission_dir +"/"+ p_mission_dir_prefix + p_mission_name + current_dir;

	// create directory for the mission
	string mkdir_str = "mkdir \""+ current_copy_dir +"\"";
	if(system(mkdir_str.c_str()) > 0) // hack!!
	{
		printf("\nError, mission dir (%s) already exists.\n", current_copy_dir.c_str());
		return false;
	}

	// search & convert files
	vector <string> fileList = getFileList_byTypes(current_template_dir, fileTypes);
	if(!convertFiles(fileList, current_copy_dir, tagList, convList, p_mission_file_prefix))
	{
		printf("\nError, file conversion failed.\n");
		return false;
	}

	// file copying here
	vector <string> copyFileList = getFileList_notTypes(current_template_dir, fileTypes);
	for(unsigned i = 0; i < copyFileList.size(); i++)
	{
		string copy_str = "xcopy \""+ str_replace(copyFileList.at(i), '/', '\\') +"\" \""+ str_replace(current_copy_dir, '/', '\\') +"\" /I /Y /Q";
		if(system(copy_str.c_str()) > 0) // hack!!
		{
			printf("\nError, coundn't copy (%s).\n", copy_str.c_str());
			return false;
		}
	}

	// search dirs
	vector <string> dirList = getDirList(current_template_dir, dirExcludes);
	for(unsigned i = 0; i < dirList.size(); i++)
	{
		string subdir = "";
		if(!p_current_dir.empty()) { subdir += p_current_dir +"/"; } // no extra slashes for the first dirs
		subdir += dirList.at(i).substr((current_template_dir).length()+1);

		// convert sub dir
		convertDir(p_mission_dir, p_template_name, p_mission_name, p_mission_dir_prefix,\
			p_mission_file_prefix, dirExcludes, fileTypes, tagList, convList, subdir);
	}
	return true;
}

int main(int argc, char *argv[])
{
	{
		using namespace frozenbyte::filesystem;
		boost::shared_ptr<IFilePackage> standardPackage(new StandardPackage());
		FilePackageManager &manager = FilePackageManager::getInstance();
		manager.addPackage(standardPackage, 0);
	}

	//printf("Working dir: "); system("cd");

	if(argc != 3+1) // if there are two arguments
	{
		printf("Usage:\nMISSIONCREATOR mission_dir, template_name, mission_name\n\n");
	}
	else
	{
		string p_mission_dir = argv[1];		// "C:\arena_src\util\executables\missioncreator\runtime" // "..\..\..\..\..\Missions"
		string p_template_name = argv[2];	// "MissionTemplate"
		string p_mission_name = argv[3];	// "testi"

		// MISSION DIR PREFIX
		//string mission_dir_prefix = "Mission";
		string mission_dir_prefix = "";

		// MISSION FILE PREFIX (ONLY FOR CONVERTED)
		//string mission_file_prefix = str2lower(p_mission_name) +"_";
		// now, template is ripped off, so file prefix does not need to add "_"
		// --jpk
		string mission_file_prefix = str2lower(p_mission_name);

		// DIR EXCLUDES
		vector <string> dirExcludes;
		dirExcludes.push_back(".svn");
		dirExcludes.push_back("CVS");

		// FILETYPES
		vector <string> fileTypes;
		fileTypes.push_back("dhm");
		fileTypes.push_back("dhs");
		fileTypes.push_back("txt");

		// TAGS
		vector <string> tagList;
		tagList.push_back("<mission_name>");			// mission name
		tagList.push_back("<mission_name_lower>");		// mission name lower

		// CONVERTS
		vector <string> convList;
		convList.push_back(p_mission_name);				// mission name
		convList.push_back(str2lower(p_mission_name));	// mission name lower

		if(!convertDir(p_mission_dir, p_template_name, p_mission_name, mission_dir_prefix,\
			mission_file_prefix, dirExcludes, fileTypes, tagList, convList, ""))
		{ printf("\nFailed!\n\n"); }
		else
		{ printf("\nDone.\n\n"); }
	}
	return 0;
}