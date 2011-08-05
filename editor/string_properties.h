// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_STRING_PROPERTIES_H
#define INCLUDED_EDITOR_STRING_PROPERTIES_H

#include <vector>
#include <map>
#include <string>

namespace frozenbyte {
namespace editor {

struct StringProperties
{
	std::vector<std::string> strings;
	std::map<std::string, std::string> defaults;

	void addProperty(const std::string &property);
	void addProperty(const std::string &property, const std::string &defaultValue);
	void add(const StringProperties &properties);

	int getPropertyAmount() const;
	std::string getProperty(int index) const;
	bool hasDefault(const std::string &property) const;
	std::string getDefault(const std::string &property) const;
};

} // editor
} // frozenbyte

#endif
