
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#include "LipsyncProperties.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../util/assert.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <fstream>
#include <vector>
#include <map>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

using namespace boost;
using namespace std;
using namespace frozenbyte::editor;
using namespace frozenbyte;

namespace sfx {
namespace {

	typedef map<string, string> StringMap;
	typedef vector<LipsyncProperties::Phonem> PhonemList;
	string empty;

	const string &get(const StringMap &m, int index)
	{
		int i = 0;
		for(StringMap::const_iterator it = m.begin(); it != m.end(); ++it)
		{
			if(i++ == index)
				return it->second;
		}

		return empty;
	}

	const string &getFirst(const StringMap &m, int index)
	{
		int i = 0;
		for(StringMap::const_iterator it = m.begin(); it != m.end(); ++it)
		{
			if(i++ == index)
				return it->first;
		}

		return empty;
	}

} // unnamed

struct LipsyncProperties::Data
{
	StringMap idles;
	StringMap expressions;
	PhonemList phonems;

	int idleFadeTime;
	int expressionFadeTime;
	int sampleRate;

	Data()
	:	idleFadeTime(0),
		expressionFadeTime(0),
		sampleRate(0)
	{
	}

	void loadMap(StringMap &map, const ParserGroup &group)
	{
		for(int i = 0; i < group.getValueAmount(); ++i)
		{
			const std::string &key = group.getValueKey(i);
			const std::string &value = group.getValue(key);

			map[key] = value;
		}
	}

	void load()
	{
		EditorParser parser(false, true);
#ifdef LEGACY_FILES
		filesystem::InputStream lipsyncFile = filesystem::FilePackageManager::getInstance().getFile("Data/Animations/lipsync.txt");
#else
		filesystem::InputStream lipsyncFile = filesystem::FilePackageManager::getInstance().getFile("data/animation/lipsync.txt");
#endif
		lipsyncFile >> parser;

		ParserGroup &globals = parser.getGlobals();
		loadMap(idles, globals.getSubGroup("IdleAnimations"));
		loadMap(expressions, globals.getSubGroup("Expressions"));

		ParserGroup &phonemGroup = globals.getSubGroup("Phonemes");
		for(int i = 0; i < phonemGroup.getValueAmount(); ++i)
		{
			const std::string &key = phonemGroup.getValueKey(i);
			const std::string &value = phonemGroup.getValue(key);

			Phonem phonem;
			phonem.limit = convertFromString<int> (key, 0);
			phonem.file = value;

			phonems.push_back(phonem);
		}

		idleFadeTime = convertFromString<int> (globals.getValue("idle_fade_time"), 0);
		expressionFadeTime = convertFromString<int> (globals.getValue("expression_fade_time"), 0);
		sampleRate = convertFromString<int> (globals.getValue("sample_rate"), 0);
	}
};

LipsyncProperties::LipsyncProperties()
{
	scoped_ptr<Data> tempData(new Data());
	tempData->load();

	data.swap(tempData);
}

LipsyncProperties::~LipsyncProperties()
{
}

int LipsyncProperties::getPropertyValue(Property property) const
{
	if(property == IdleFadeTime)
		return data->idleFadeTime;
	else if(property == ExpressionFadeTime)
		return data->expressionFadeTime;
	else if(property == SampleRate)
		return data->sampleRate;

	return 0;
}

int LipsyncProperties::getIdleAnimationAmount() const
{
	return data->idles.size();
}

const std::string &LipsyncProperties::getIdleAnimation(int index) const
{
	return get(data->idles, index);
}

const std::string &LipsyncProperties::getIdleAnimationName(int index) const
{
	return getFirst(data->idles, index);
}

int LipsyncProperties::getExpressionAnimationAmount() const
{
	return data->expressions.size();
}

const std::string &LipsyncProperties::getExpressionAnimation(int index) const
{
	return get(data->expressions, index);
}

const std::string &LipsyncProperties::getExpressionAnimationName(int index) const
{
	return getFirst(data->expressions, index);
}

int LipsyncProperties::getPhonemAmount() const
{
	return data->phonems.size();
}

const LipsyncProperties::Phonem &LipsyncProperties::getPhonem(int index) const
{
	FB_ASSERT(index >= 0 && index < int(data->phonems.size()));
	return data->phonems[index];
}

} // sfx
