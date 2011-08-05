
#include "precompiled.h"

#include "EngineMetaValues.h"
#include "../convert/str2int.h"

#include <string>
#include <map>


using std::string;
using std::pair;
using std::map;

namespace game
{

enum META_VALUE_DATA_TYPE
{
	META_VALUE_DATA_TYPE_INT
};

typedef map<string, pair<string, META_VALUE_DATA_TYPE> > MetaValueHashType;

static MetaValueHashType valueStorage;
static int valuesInHash = 0;


bool EngineMetaValues::doesMetaValueExist(const char *key)
{
	std::string keystr = key;
	MetaValueHashType::iterator iter = valueStorage.find(keystr);
	if (iter != valueStorage.end())
	{
		return true;
	} else {
		return false;
	}
}


bool EngineMetaValues::isMetaValueTypeInt(const char *key)
{
	std::string keystr = key;
	MetaValueHashType::iterator iter = valueStorage.find(keystr);
	if (iter != valueStorage.end())
	{
		if ((*iter).second.second == META_VALUE_DATA_TYPE_INT)
			return true;
		else
			return false;
	} else {
		// oopsie. it does not exist!
		// maybe could do LOG_WARNING("engine meta value did not exist")
		assert(!"engine meta value did not exist");
		return false;
	}
}


int EngineMetaValues::getMetaValueInt(const char *key)
{
	string keystr = key;
	MetaValueHashType::iterator iter = valueStorage.find(keystr);
	if (iter != valueStorage.end())
	{
		string &valstr = (*iter).second.first;
		int val = str2int(valstr.c_str());
		return val;
	} else {
		// oopsie. it does not exist!
		// maybe could do LOG_WARNING("engine meta value did not exist")
		assert(!"engine meta value did not exist");
		return 0;
	}
}


void EngineMetaValues::setMetaValueInt(const char *key, int value)
{
	string keystr = key;
	string valstr = int2str(value);

	MetaValueHashType::iterator iter = valueStorage.find(keystr);
	if (iter != valueStorage.end())
	{
		(*iter).second.first = valstr;
	} else {
		if (valuesInHash > 10000)
		{
			LOG_ERROR("EngineMetaValues::setMetaValueInt - Failsafe meta value amount limit reached.");
			assert(!"EngineMetaValues::setMetaValueInt - Failsafe meta value amount limit reached.");
			return;
		}

		pair<string, pair<string, META_VALUE_DATA_TYPE> > insertThis;
		insertThis.first = key;
		insertThis.second.first = valstr;
		insertThis.second.second = META_VALUE_DATA_TYPE_INT;
		valueStorage.insert(insertThis);

		valuesInHash++;
	}
}

}

