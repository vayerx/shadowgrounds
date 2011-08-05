#pragma warning(disable:4103)
#pragma warning(disable:4786)

#if 0

// Copyright 2002-2004 Frozenbyte Ltd.

#include "Parser.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// this is for the messagebox .. maybe not such a good decision, just a hack..

#include <windows.h> 

#include <vector>
#include "../filesystem/input_stream_wrapper.h"
#include "../util/Debug_MemoryManager.h"
#include "../system/Logger.h"

using namespace frozenbyte;

void WriteMessage(char *string, ...)
{
	va_list arg; 
	va_start(arg, string);

	FILE *fhandle = fopen("ParserDebug.txt", "at");
	if(!fhandle) 
	{ 
		//MessageBox(0,"Could not open ParserDebug.txt","Error",MB_OK); 
		exit(-1); 
	}

	char *foo = new char[666];
	vsprintf(foo, string, arg);

	fprintf(fhandle, foo);
	fprintf(fhandle,"\n");
	fclose(fhandle);

	va_end(arg);
	delete[] foo;
}

namespace Parser
{

/*
  Some helper functions
*/

// Both \r and \n causes return
std::string ReadLine(filesystem::FB_FILE *fp)
{
	std::string string;

	bool found_comment = false;

	char character = fgetc(fp);
	while(character != EOF)
	{
		if(character == '\n')
			break;
		
		// Comment?
		if((character == '/') && (string.size() > 0) && (string[string.size() - 1] == '/'))
		{
			found_comment = true;
			
			// Remove first slash
			string = string.substr(0, string.size() - 1);
		}

		if((found_comment == false) && (character != '\r'))
			string += character;
		character = fgetc(fp);
	}

	return string;
}

// Returns position of '=' sign, -1 if not found
int IsProperty(const std::string &string)
{
	if(string.size() == 0)
		return -1;

	// Cannot be the first or last sign
	for(int i = 1; i < (int)string.size() - 1; ++i)
	{
		if(string[i] == '=')
			return i;
	}

	return -1;
}

std::string GetProperty(const std::string &string, int index, bool first)
{
	std::string return_string;

	if(first == true)
	{
		// We don't want last blanks to our property name
		bool removing_blanks = true;
		for(int i = index - 1; i >= 0; --i)
		{
			char character = string[i];
			if((removing_blanks == true) && ((character == ' ') || (character == '\t')) )
				continue;

			if((character == ' ') || (character == '\t'))
				break;

			removing_blanks = false;
			return_string = character + return_string;
		}
	}
	else
	{
		// We don't want first blanks to our property value
		bool removing_blanks = true;
		for(int i = index + 1; i < (int)string.size(); ++i)
		{
			char character = string[i];
			if((removing_blanks == true) && ((character == ' ') || (character == '\t')) )
				continue;

			// would result into inability to read values with spaces.
			//if((character == ' ') || (character == '\t'))
			//	break;

			removing_blanks = false;
			return_string += character;
		}
	}

	return return_string;
}

/*
  ParserGroup
*/

ParserGroup::ParserGroup()
{
}

ParserGroup::~ParserGroup()
{
}

void ParserGroup::Parse(filesystem::FB_FILE *fp)
{
	sub_groups = string_map();
	
	while(!feof(fp))
	{
		std::string string = ReadLine(fp);

		int position = IsProperty(string);
		if(position != -1)
		{
			std::string property = GetProperty(string, position, true);
			std::string value = GetProperty(string, position, false);

			properties[ property ] = value;
		}
		else
		{
			// Branches ?
			for(int i = 0; i < (int)string.size(); ++i)
			{
				if(string[i] == '{')
				{
					ParseSubGroups(fp);
					break;
				}

				if(string[i] == '}')
				{
					// End of this group
					return;
				}
			}
		}
	}
}

const string_map &ParserGroup::GetProperties() const
{
	return properties;
}

const string_map &ParserGroup::GetSubGroups() const
{
	return sub_groups;
}

void ParserGroup::DebugPrint()
{
	WriteMessage("\tProperties");
	for(string_map::iterator it = properties.begin(); it != properties.end(); ++it)
	{
		std::string property = (*it).first;
		std::string value = (*it).second;

		WriteMessage("\t\t%s = %s", property.c_str(), value.c_str());
	}

	WriteMessage("\tSub groups");
	for(it = sub_groups.begin(); it != sub_groups.end(); ++it)
	{
		std::string sub_group = (*it).first;
		std::string value = (*it).second;

		WriteMessage("\t\t%s = %s", sub_group.c_str(), value.c_str());
	}
}


void ParserGroup::ParseSubGroups(filesystem::FB_FILE *fp)
{
	while(!feof(fp))
	{
		std::string string = ReadLine(fp);

		int position = IsProperty(string);
		if(position != -1)
		{
			std::string group = GetProperty(string, position, true);
			std::string value = GetProperty(string, position, false);

			sub_groups[ group ] = value;
		}
		else
		{
			// End of group?
			for(int i = 0; i < (int)string.size(); ++i)
			{
				if(string[i] == '}')
					return;
			}
		}
	}
}


/*
  Parser
*/
Parser::Parser(const char *file_name)
{
	Parse(file_name);
}

Parser::~Parser()
{

}

const group_map &Parser::GetGroups() const
{
	return groups;
}

const ParserGroup Parser::FindGroup(const char *name) const
{
	group_map::const_iterator i = groups.find(name);
	if(i == groups.end())
		return ParserGroup();
	else
		return (*i).second;
}

void Parser::DebugPrint()
{
	// Remove old
	fclose( fopen("ParserDebug.txt", "wb") );

	for(group_map::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		std::string group_name = (*it).first;
		ParserGroup group = (*it).second;

		WriteMessage("%s", group_name.c_str());
		group.DebugPrint();
		WriteMessage("\r\n");
	}
}

void Parser::Parse(const char *file_name)
{
	std::string group_name;
	std::string group_start;

	filesystem::FB_FILE *fp = filesystem::fb_fopen(file_name, "rb");
	if(!fp) 
	{ 
		//MessageBox(0,"Could not load parserfile","Error",MB_OK); 
		//exit(-1); 
		Logger::getInstance()->error("Could not load parserfile.");
		Logger::getInstance()->debug(file_name);
		return;
	}

	group_name = ReadLine(fp);
	group_start = ReadLine(fp);

	while(!feof(fp))
	{
		for(int i = 0; i < (int)group_start.size(); ++i)
		{
			int position = IsProperty(group_start);
			if(position != -1)
			{
				std::string property = GetProperty(group_start, position, true);
				std::string value = GetProperty(group_start, position, false);

				properties[ property ] = value;
			}
			
			if(group_start[i] == '{')
			{
				// Inheritance owns :)
				for(int j = 0; j < (int)group_name.size(); ++j)
				{
					if(group_name[j] == ':')
					{
						std::string new_name = GetProperty(group_name, j, true);
						std::string super_name = GetProperty(group_name, j, false);

						groups[ new_name ] = groups[ super_name ];
						group_name = new_name;
						break;
					}
				}

				groups[ group_name ].Parse(fp);
			}
		}

		group_name = group_start;
		group_start = ReadLine(fp);
	}

	fclose(fp);
}

const string_map &Parser::GetProperties() const
{
	return properties;
}

/*
void Parser::MakeRGBGroups()
{
	for(group_map::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		ParserGroup group = (*it).second;

		string_map properties = group.GetProperties();
		if(properties.find("rgb") != properties.end())
		{
			int rgb = GetInt(properties["rgb"]);
			rgb_groups[rgb] =  group;
		}
	}
}
*/

/*
  Helpers
*/

float GetFloat(const std::string &string)
{
	return (float)atof(string.c_str());
}

int GetInt(const std::string &string, int index)
{
	int string_size = string.size();
	if(string_size == 0)
		return 0;
	else if(string_size == 1)
		return atoi(string.c_str());

	std::vector<int> values;
	
	int dot = string_size;
	for(int i = string_size - 2; i >= 0; --i)
	{
		if(string[i] == ',')
		{
			int value = atoi(string.substr(i + 1, dot).c_str());
			dot = i;
			values.push_back(value);
		}
		else if(i == 0)
		{
			int value = atoi(string.substr(0, dot).c_str());
			values.push_back(value);
		}
	}

	if(index == -1)
	{
		int return_value = 0;
		for(int j = 0; j < (int)values.size(); ++j)
			return_value += values[j] << (j*8);
		return return_value;
	}

	if(index < (int)values.size())
		return values[values.size() - index - 1];
	else
		return 0;
}

std::string GetString(const string_map &properties, const std::string &string)
{
	string_map::const_iterator i;
	i = properties.find(string);
	
	if(i != properties.end())
		return (*i).second;
	else
		return "";
}

float GetFloat(const string_map &properties, const std::string &string)
{
	string_map::const_iterator i;
	i = properties.find(string);
	
	if(i != properties.end())
		return GetFloat((*i).second);
	else
		return 0;
}

int GetInt(const string_map &properties, const std::string &string, int index)
{
	string_map::const_iterator i;
	i = properties.find(string);
	
	if(i != properties.end())
		return GetInt((*i).second, index);
	else
		return 0;
}

bool HasProperty(const string_map &properties, const std::string &property)
{
	if(properties.find(property) != properties.end())
		return true;
	return false;
}
/*
void TestProg()
{
	Parser parser = Parser("parser.txt");
	parser.DebugPrint();

	int a = 0;
}
*/
} // namespace Parser

#endif