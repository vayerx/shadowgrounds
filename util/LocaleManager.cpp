#include "precompiled.h"

#include "LocaleManager.h"
#include "LocaleResource.h"
#include "../editor/parser.h"
#include "../system/Logger.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <fstream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <boost/lexical_cast.hpp>

#ifdef FB_RU_HAX
#include "../filesystem/input_file_stream.h"
#include <string>
#endif

using namespace frozenbyte;
using namespace frozenbyte::editor;
using namespace boost;
using namespace std;

namespace util {
namespace {

#ifdef LEGACY_FILES
	const char *configName = "Data/Locales/configuration.txt";
#else
	const char *configName = "data/locale/configuration.txt";
#endif
	string convertBuffer;

	void convertString(const char localeId[2], string &str)
	{
		if(str.empty())
			return;

		for(unsigned int j = 0; j < str.size() - 1; ++j)
		{
			if(str[j] != '$')
				continue;
			if(str[j + 1] != '$')
				continue;

			str[j] = localeId[0];
			str[j + 1] = localeId[1];
		}
	}

#ifdef FB_RU_HAX
	struct HaxParser
	{
		std::map<std::string, std::wstring> values;

		HaxParser()
		{
		}

		void parse()
		{
			//frozenbyte::filesystem::InputStream stream = frozenbyte::filesystem::createInputFileStream("Data/Locales/ru/launcher.txt");
#ifdef LEGACY_FILES
			frozenbyte::filesystem::InputStream stream = frozenbyte::filesystem::FilePackageManager::getInstance().getFile("Data/Locales/ru/launcher.txt");
#else
			frozenbyte::filesystem::InputStream stream = frozenbyte::filesystem::FilePackageManager::getInstance().getFile("data/locale/ru/launcher.txt");
#endif

			int size = stream.getSize() / 2;
			if(size < 4)
				return;

			std::wstring buffer;
			buffer.resize(size);
			stream.read((unsigned char *) &buffer[0], size * 2);

			int start = 1;
			int end = size;

			std::wstring key;
			std::wstring value;
			while(getNextPair(buffer, start, end, key, value))
			{
				if(key.empty())
					continue;

				std::string key8;
				for(unsigned int i = 0; i < key.size(); ++i)
				{
					unsigned short c = key[i];
					key8 += (unsigned char) c;
				}

				const char *tmp8 = key8.c_str();
				const wchar_t *tmp16 = value.c_str();
				values[key8] = value;
			}
		}

		bool isWhiteSpace(unsigned int character)
		{
			if(character == ' ')
				return true;
			if(character == '\t')
				return true;
			if(character == '\r')
				return true;
			if(character == '\n')
				return true;

			return false;
		}

		void getTrimmed(const std::wstring &buffer, int start, int end, std::wstring &result)
		{
			for(; start < end && isWhiteSpace(buffer[start]); ++start)
				;

			for(; end > start && isWhiteSpace(buffer[end]); --end)
				;

			if(start < end)
				result = buffer.substr(start, end - start + 1);
		}

		bool getNextPair(const std::wstring &buffer, int &start, int fileEnd, std::wstring &key, std::wstring &value)
		{
			key.clear();

			// Find end of line
			int end = start;
			for(; end < fileEnd - 1; ++end)
			{
				if(buffer[end] == '\r' && buffer[end + 1] == '\n')
				{
					++end;
					break;
				}
			}

			// Find '='
			int split = -1;
			for(int i = start; i < end; ++i)
			{
				if(buffer[i] == '=')
				{
					split = i;
					break;
				}
			}

			if(split == -1)
			{
				if(start == end)
					return false;

				start = end;
				return getNextPair(buffer, start, fileEnd, key, value);

			}

			getTrimmed(buffer, start, split - 1, key);
			getTrimmed(buffer, split + 1, end, value);

			start = end;
			return true;
		}

		bool hasKey(const std::string &key) const
		{
			if(values.find(key) != values.end())
				return true;

			return false;
		}

		std::wstring getValueW(const std::string &key) const
		{
			std::map<std::string, std::wstring>::const_iterator it = values.find(key);
			if(it != values.end())
				return it->second;

			return std::wstring();
		}

	};

	static HaxParser parser;
#endif

} // unnamed

// Private

const char *LocaleManager::convertPath(const char localeId[2], const char *str) const
{
	if(!str)
		return 0;

	convertBuffer = str;
	convertString(localeId, convertBuffer);

	return convertBuffer.c_str();
}

// Protected

void LocaleManager::loadResource(int resourceId, const char *filename, const char localeId[2])
{
	if(!filename)
		return;

	int locale = resourceLocale[resourceId];
	Locale &l = locales[locale];
	assert(resourceId >= 0);

	LocaleResource &resource = *l.resources[resourceId].get();
	EditorParser parser;

	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(filename);
	if(!stream.isEof())
		stream >> parser;
	else
	{
		string message = "Failed to load locale file ";
		if(filename)
			message += filename;
		else
			message += "(null)";

		Logger::getInstance()->error(message.c_str());
	}

	const ParserGroup &group = parser.getGlobals();
	for(int i = 0; i < group.getValueAmount(); ++i)
	{
		const std::string &key = group.getValueKey(i);
		std::string value = group.getValue(key);

// we don't want to automatically convert $$
#ifndef PROJECT_SURVIVOR
		convertString(localeId, value);
#endif
		resource.addPair(key.c_str(), value.c_str());
	}

	for(int i = 0; i < group.getSubGroupAmount(); ++i)
	{
		const std::string &key = group.getSubGroupName(i);
		const ParserGroup &subGroup = group.getSubGroup(key);

		std::string result;
		for(int j = 0; j < subGroup.getLineCount(); ++j)
		{
			if(j > 0)
				result += "\n";

			std::string line = subGroup.getLine(j);

// we don't want to automatically convert $$
#ifndef PROJECT_SURVIVOR
			convertString(localeId, line);
#endif
			result += line;
		}

		bool inside_quete = false;
		for(unsigned int k = 0; k < result.size(); ++k)
		{
			if(result[k] == '"') 
				inside_quete = !inside_quete;

			if(result[k] == '_' && !inside_quete)
				result[k] = ' ';
		}

		resource.addPair(key.c_str(), result.c_str());
	}
}

void LocaleManager::parse(const char localeId[2], const std::string &group)
{
	EditorParser parser;

	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(configName);
	if(!stream.isEof())
		stream >> parser;
	else
	{
		string message = "Failed to load locale main file ";
		if(configName)
			message += configName;
		else
			message += "(null)";

		Logger::getInstance()->error(message.c_str());
	}

	const ParserGroup &globals = parser.getGlobals();
	int groups = globals.getSubGroupAmount();

	for(int i = 0; i < groups; ++i)
	{
		const std::string &name = globals.getSubGroupName(i);
		if(name != group)
			continue;

		int id = getBank(name.c_str());
		{
			int locale = resourceLocale[id];
			Locale &l = locales[locale];

			if(id >= int(l.resources.size()))
				l.resources.resize(id + 1);

			if(!l.resources[id])
				l.resources[id].reset(new LocaleResource());

		}

		const ParserGroup &group = globals.getSubGroup(name);
		int files = group.getLineCount();

		for(int j = 0; j < files; ++j)
		{
			const string &line = group.getLine(j);
			loadResource(id, convertPath(localeId, line.c_str()), localeId);
		}
	}
}

void LocaleManager::parse()
{
	//vector<int> parsed;

	for(unsigned int i = 0; i < resourceLocale.size(); ++i)
	{
		int locale = resourceLocale[i];
		const std::string &group = resourceGroup[i];

		parse(locales[locale].id, group);
	}
}

// Public

LocaleManager::LocaleManager()
{

#ifdef FB_RU_HAX
	parser.parse();
#endif

}

LocaleManager::~LocaleManager()
{
}

void LocaleManager::init(const char *configurationFile)
{
	EditorParser parser;
	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(configurationFile);
	if(!stream.isEof())
		stream >> parser;
	else
	{
		string message = "Failed to load locale init file ";
		if(configurationFile)
			message += configurationFile;
		else
			message += "(null)";

		Logger::getInstance()->error(message.c_str());
	}

	const ParserGroup &globals = parser.getGlobals();
	const ParserGroup &id = globals.getSubGroup("Ids");
	const ParserGroup &english = globals.getSubGroup("EnglishNames");
	const ParserGroup &native = globals.getSubGroup("NativeNames");

	int values = id.getValueAmount();
	for(int i = 0; i < values; ++i)
	{
		const std::string &key = id.getValueKey(i);
		const std::string &value = id.getValue(key);

		try
		{
			int index = lexical_cast<int> (key);
			if(index < 0 || index >= MAX_LOCALES)
			{
				Logger::getInstance()->error("Invalid locale index");
				Logger::getInstance()->error(configurationFile);
				break;
			}

			if(value.size() < 2 || value.size() > 2)
			{
				Logger::getInstance()->error("Invalid locale id");
				Logger::getInstance()->error(configurationFile);
				break;
			}

			Locale &l = locales[index];
			l.id[0] = value[0];
			l.id[1] = value[1];

			l.englishName = english.getValue(value);
			l.nativeName = native.getValue(value);
		}
		catch(...)
		{
			break;
		}
	}
}

void LocaleManager::setCurrentLocale(int resourceId, int number, const std::string &group)
{
	if(resourceId < 0)
	{
		assert(!"Invalid resource id");
		return;
	}

	if(resourceId >= int(resourceLocale.size()))
	{
		resourceLocale.resize(resourceId + 1);
		resourceGroup.resize(resourceId + 1);
	}

	assert(number >= 0 && number < MAX_LOCALES);
	resourceLocale[resourceId] = number;
	resourceGroup[resourceId] = group;
}

const char *LocaleManager::getLocaleIdString(int number) const
{
	assert(number >= 0 && number < MAX_LOCALES);
	// null terminate!
	static char id_string[3];
	id_string[0] = locales[number].id[0];
	id_string[1] = locales[number].id[1];
	id_string[2] = 0;
	return id_string;
}

const char *LocaleManager::getLocaleEnglishName(int number) const
{
	assert(number >= 0 && number < MAX_LOCALES);
	return locales[number].englishName.c_str();
}

const char *LocaleManager::getLocaleNativeName(int number) const
{
	assert(number >= 0 && number < MAX_LOCALES);
	return locales[number].nativeName.c_str();
}

const LocaleResource *LocaleManager::getResource(int resourceId) const
{
	if(resourceId < 0 || resourceId >= int(resourceLocale.size()))
	{
		//assert(!"Invalid resource id");
		return 0;
	}

	int locale = resourceLocale[resourceId];
	if(locale >= 0)
	{
		const Locale &l = locales[locale];
		if(resourceId < 0 || resourceId >= int(l.resources.size()))
			return 0;

		if(l.resources[resourceId])
			return l.resources[resourceId].get();
	}
	
	return 0;
}

const char *LocaleManager::convertPath(int resourceId, const char *str) const
{
	if(resourceId < 0 || resourceId >= int(resourceLocale.size()))
	{
		assert(!"Invalid resource id");
		return 0;
	}

	int locale = resourceLocale[resourceId];
	if(locale >= 0)
	{
		const Locale &l = locales[locale];
		return convertPath(l.id, str);
	}

	return 0;
}

const char *LocaleManager::convert(int resourceId, const char *str) const
{
	const LocaleResource *r = getResource(resourceId);
	if(r)
		return r->convert(str);

	return 0;
}

const char *LocaleManager::getString(int resourceId, const char *key) const
{
#ifdef FB_RU_HAX
	if(parser.hasKey(key))
		return key;
#endif

	const LocaleResource *r = getResource(resourceId);
	const char *result = (r) ? r->get(key) : 0;
	if(result)
		return result;

	string message = "Couldn't find locale key: ";
	if(key)
		message += key;
	else
		message += "(null)";


// #ifndef NDEBUG
/*
	std::fstream file( "missing_locales.txt", std::ios::out | std::ios::app );

	file << key << " = " << std::endl;

	file.close();
*/
// #endif
	
	Logger::getInstance()->error(message.c_str());
	return "(LOCALIZATION MISSING)";
}

#ifdef FB_RU_HAX
	std::wstring LocaleManager::getWideString(int resourceId, const char *key) const
	{
		return parser.getValueW(key);
	}
#endif

bool LocaleManager::hasString( int resourceId, const char* key ) const
{
#ifdef FB_RU_HAX
	if(parser.hasKey(key))
		return true;
#endif

	const LocaleResource *r = getResource(resourceId);
	return (bool)( (r) ?( r->get(key) != 0 ) : 0 );
}

bool LocaleManager::getString( int resourceId, const char* key, const char **return_value ) const
{
#ifdef FB_RU_HAX
	#pragma error("------- LocaleManager::getString with FB_RU_HAX: FIX ME! -------")
#endif

	const LocaleResource *r = getResource(resourceId);
	const char *ret = 0;
	if(r) ret = r->get(key);
	if(ret)
	{
		(*return_value) = ret;
		return true;
	}
	return false;
}

int LocaleManager::getInt(int resourceId, const char *key, int defaultValue) const
{
	const LocaleResource *r = getResource(resourceId);
	const char *result = (r) ? r->get(key) : 0;
	if(result)
		return atoi(result);

	string message = "Couldn't find locale key: ";
	if(key)
		message += key;
	else
		message += "(null)";

	Logger::getInstance()->error(message.c_str());
	return defaultValue;
}

} // util
