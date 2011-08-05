// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "string_properties.h"
#include <algorithm>
#include <cassert>

using namespace std;
typedef vector<string> StringList;
typedef map<string, string> StringMap;

namespace frozenbyte {
namespace editor {

void StringProperties::addProperty(const std::string &property)
{
	if(find(strings.begin(), strings.end(), property) == strings.end())
		strings.push_back(property);
}

void StringProperties::addProperty(const std::string &property, const std::string &defaultValue)
{
	addProperty(property);
	defaults[property] = defaultValue;
}

void StringProperties::add(const StringProperties &properties)
{
	StringList::const_iterator is = properties.strings.begin();
	for(; is != properties.strings.end(); ++is)
		addProperty(*is);

	StringMap::const_iterator it = properties.defaults.begin();
	for(; it != properties.defaults.end(); ++it)
	{
		const std::string &a = it->first;
		const std::string &b = it->second;

		defaults[a] = b;
	}
}

int StringProperties::getPropertyAmount() const
{
	return strings.size();
}

std::string StringProperties::getProperty(int index) const
{
	assert(index >= 0 && index < getPropertyAmount());
	return strings[index];
}

bool StringProperties::hasDefault(const std::string &property) const
{
	StringMap::const_iterator it = defaults.find(property);
	if(it != defaults.end())
		return true;

	return false;
}

std::string StringProperties::getDefault(const std::string &property) const
{
	if(!hasDefault(property))
		return "";

	StringMap::const_iterator it = defaults.find(property);
	return it->second;
}

} // editor
} // frozenbyte
