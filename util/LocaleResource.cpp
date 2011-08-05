
#include "precompiled.h"

#include "LocaleResource.h"
#include "../system/Logger.h"
#include <fstream>

using namespace std;
typedef map<string, string> LocaleValueMap;

namespace util {
namespace {

//#define LOCALERESOURCE_MAX_STRING_LENGTH 512
//static char convertBuffer[LOCALERESOURCE_MAX_STRING_LENGTH];
string resourceConvertBuffer;

void add(const char *str, int start, int end)
{
	int length = resourceConvertBuffer.size();
	resourceConvertBuffer.resize(length + (end - start));

	for(int i = start; i < end; ++i)
		resourceConvertBuffer[i - start + length] = str[i];
}

} // unnamed

LocaleResource::LocaleResource()
{
}

LocaleResource::~LocaleResource()
{
}

const char *LocaleResource::get(const char *key) const
{
	if(!key)
		return 0;

	LocaleValueMap::const_iterator it = values.find(key);
	if(it == values.end())
		return 0;

	return it->second.c_str();
}

const char *LocaleResource::convert(const char *str) const
{
	if(!str)
		return 0;

	int length = strlen(str);
	if(length < 3)
		return str;

	resourceConvertBuffer.resize(0);
	int copyStart = 0;
	int copyEnd = 0;

	for(int i = 0; i < length - 3; ++i)
	{
		if(str[i] != '$')
			continue;
		if(str[i + 1] != '(')
			continue;

		if(i > 0)
		{
			add(str, copyStart, i);
			
			copyStart = i + 1;
			copyEnd = copyStart;
		}

		int end = - 1;
		for(int j = i + 2; j < length; ++j)
		{
			if(str[j] == ')')
			{
				end = j;
				break;
			}
		}

		if(end == -1)
			break;

		string key;
		key.resize(end - i - 2);

		if(!key.empty())
		{
			for(unsigned int k = 0; k < key.size(); ++k)
				key[k] = str[i + 2 + k];

			const char *value = get(key.c_str());
			if(value)
			{
				add(value, 0, strlen(value));

				i = end;
				copyStart = i + 1;
				copyEnd = copyStart;
			}
			else
			{
				std::fstream f;
				f.open( "missing_locales.txt", std::ios::app );
				f << key << std::endl;
				f.close();

				string error("Locale does not define key ");
				error += key;
				error += " (";
				error += str;
				error += ")";

				Logger::getInstance()->error(error.c_str());
			}
		}
	}

	if(copyEnd < length)
		add(str, copyStart, length);

	return resourceConvertBuffer.c_str();
}

void LocaleResource::addPair(const char *key, const char *value)
{
	if(!key || !value)
		return;

	//values[key] = "(L)";
	//values[key] += value;

	values[key] = value;
}

} // util
