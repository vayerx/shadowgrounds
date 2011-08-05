// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "string_replace.h"
#include <cassert>

#pragma warning(disable: 4786)
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

	bool replaceToken(string &result, const StringMap &strings, const StringMap &defaults)
	{
		if(result.size() <= 2)
			return false;

		int start = result.find_first_of("<");
		int end = result.find_first_of(">");

		if(start == result.npos || end == result.npos)
			return false;

		start += 1;
		if(end <= start)
			return false;

		string key = result.substr(start, end - start);
		string data;

		StringMap::const_iterator it = strings.find(key);
		if(it != strings.end() && !it->second.empty())
			data = it->second;

		if(data.empty())
		{
			StringMap::const_iterator it = defaults.find(key);
			if(it != defaults.end())
				data = it->second;
			else
				data = "STRING_NOT_FOUND";
		}

		// special case: IGNORE_LINE tag causes the whole line to be emptied
		if (data == "IGNORE_LINE")
		{
			result = std::string("");
		} else {
			start -= 1;
			end += 1;
			result.replace(result.begin() + start, result.begin() + end, data.begin(), data.end());
		}

		return true;
	}

} // unnamed

string replaceString(const std::string &source, const StringMap &strings, const StringMap &defaults)
{
	string result = source;

	for(;;)
	{
		if(!replaceToken(result, strings, defaults))
			break;
	}

	return result;
}

} // editor
} // frozenbyte
