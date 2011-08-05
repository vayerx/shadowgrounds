#ifndef DHLOCALEMANAGER_H
#define DHLOCALEMANAGER_H

#include "../util/LocaleManager.h"

namespace game {

class DHLocaleManager : public util::LocaleManager
{
	int getBank(const char *str);

public:
	DHLocaleManager();
	~DHLocaleManager();

	enum Bank
	{
		BANK_GUI = 0,
		BANK_SPEECH = 1,
		BANK_SUBTITLES = 2
	};

	static DHLocaleManager *getInstance();
	static void cleanInstance();
};

const char *getLocaleGuiString(const char *str);
const char *getLocaleSpeechString(const char *str);
const char *getLocaleSubtitleString(const char *str);
const char *convertLocaleGuiString(const char *str);
const char *convertLocaleSpeechString(const char *str);
const char *convertLocaleSubtitleString(const char *str);

int getLocaleGuiInt(const char *str, int defaultValue);

} // game

#endif

