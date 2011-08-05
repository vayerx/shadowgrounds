
#include "precompiled.h"

#include "DHLocaleManager.h"
#include "SimpleOptions.h"
#include "options/options_locale.h"
#include "../util/LocaleResource.h"
#include "../system/Logger.h"

#include <sstream>

using namespace util;

namespace game {

DHLocaleManager *instance = 0;

namespace {

	struct Tracker
	{
		Tracker()
		{
		}

		~Tracker()
		{
			delete instance;
			instance = 0;
		}
	};

	Tracker tracker;
}

int DHLocaleManager::getBank(const char *str)
{
	if(strcmp(str, "Gui") == 0)
		return BANK_GUI;
	if(strcmp(str, "Speech") == 0)
		return BANK_SPEECH;
	if(strcmp(str, "Subtitles") == 0)
		return BANK_SUBTITLES;

	return -1;
}

DHLocaleManager::DHLocaleManager()
{
#ifdef LEGACY_FILES
	init("Data/Locales/locales.txt");
#else
	init("data/locale/locales.txt");
#endif

	setCurrentLocale(BANK_GUI, SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE), "Gui");
	setCurrentLocale(BANK_SUBTITLES, SimpleOptions::getInt(DH_OPT_I_SUBTITLE_LANGUAGE), "Subtitles");
	setCurrentLocale(BANK_SPEECH, SimpleOptions::getInt(DH_OPT_I_SPEECH_LANGUAGE), "Speech");

	parse();
}

DHLocaleManager::~DHLocaleManager()
{
}

DHLocaleManager *DHLocaleManager::getInstance()
{
	if(!instance)
	{
		instance = new DHLocaleManager();

		const LocaleResource *gui = instance->getResource(BANK_GUI);
		const LocaleResource *speech = instance->getResource(BANK_SPEECH);
		const LocaleResource *subtitles = instance->getResource(BANK_SUBTITLES);

		if(!gui)
			Logger::getInstance()->error("Failed to load locale resource BANK_GUI");
		if(!speech)
			Logger::getInstance()->error("Failed to load locale resource BANK_SPEECH");
		if(!subtitles)
			Logger::getInstance()->error("Failed to load locale resource BANK_SUBTITLES");
	}

	return instance;
}

void DHLocaleManager::cleanInstance()
{
	delete instance;
	instance = 0;
}

const char *getLocaleGuiString(const char *str)
{
	return DHLocaleManager::getInstance()->getString(DHLocaleManager::BANK_GUI, str);
}

const char *getLocaleSpeechString(const char *str)
{
	return DHLocaleManager::getInstance()->getString(DHLocaleManager::BANK_SPEECH, str);
}

const char *getLocaleSubtitleString(const char *str)
{
	return DHLocaleManager::getInstance()->getString(DHLocaleManager::BANK_SUBTITLES, str);
}

const char *convertLocaleGuiString(const char *str)
{
	return DHLocaleManager::getInstance()->convert(DHLocaleManager::BANK_GUI, str);
}

const char *convertLocaleSpeechString(const char *str)
{
	return DHLocaleManager::getInstance()->convertPath(DHLocaleManager::BANK_SPEECH, str);
}

const char *convertLocaleSubtitleString(const char *str)
{
	return DHLocaleManager::getInstance()->convert(DHLocaleManager::BANK_SUBTITLES, str);
}

int getLocaleGuiInt(const char *str, int defaultValue)
{
	// Fixed by Pete
	if( DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, str ) )
	{
		const char *s = getLocaleGuiString(str);
		int result = defaultValue;
		
		std::stringstream ss( s );
		ss >> result;
		
		return result;
	}
	else
	{
		return defaultValue;
	}

	// return atoi(s);
}

} // game
