#ifndef PARSER_H
#define PARSER_H

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <map>
#include <string>
//#include <hash_map>

namespace frozenbyte {
namespace filesystem {
	struct FB_FILE;
}
}


namespace Parser
{
/*
inline int MAKERGB(unsigned char r, unsigned char g, unsigned char b)
{
	return (r << 16) + (g << 8) + b;
}
*/

typedef std::map< std::string, std::string > string_map;
typedef std::map< std::string, std::string > string_vector;
//typedef std::hash_map< std::string, std::string > string_map;
//typedef std::hash_map< std::string, std::string > string_vector;

class ParserGroup
{
public:
	ParserGroup();
	~ParserGroup();

	void Parse(frozenbyte::filesystem::FB_FILE *fp);

	const string_map &GetProperties() const;
	const string_map &GetSubGroups() const;

	void DebugPrint();

private:
	void ParseSubGroups(frozenbyte::filesystem::FB_FILE *fp);

	string_map properties;
	string_map sub_groups;
};

typedef std::map< std::string, ParserGroup > group_map;

class Parser
{
public:
	Parser(const char *file_name);
	~Parser();

	const group_map &GetGroups() const;
	const ParserGroup FindGroup(const char *name) const;
	const string_map &GetProperties() const;

	void DebugPrint();

private:
	void Parse(const char *file_name);

	group_map groups;
	string_map properties;
};

/* 
  Helper functions
*/

float GetFloat(const std::string &string);
// index -1 -> rgb
int GetInt(const std::string &string, int index = 0); // 0,0, ... parsed as one int

float GetFloat(const string_map &properties, const std::string &string);
// index -1 -> rgb
int GetInt(const string_map &properties, const std::string &string, int index = 0);
std::string GetString(const string_map &properties, const std::string &string);

bool HasProperty(const string_map &properties, const std::string &property);

// Main test prog
void TestProg();

} // namespace Parser

#endif
