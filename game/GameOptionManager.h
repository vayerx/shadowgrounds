
#ifndef GAMEOPTIONMANAGER_H
#define GAMEOPTIONMANAGER_H


// NOTE: option id defines moved under game/options/ directory.

#define DH_OPT_AMOUNT 370
#include <string>
#include <memory>

#include "GameConfigs.h"
#include "GameOption.h"

class LinkedList;

namespace game
{

	class GameOptionManager
	{
	public:

		static GameOptionManager *getInstance();

		static void cleanInstance();

		GameOptionManager(GameConfigs *gameConfigs);

		~GameOptionManager();
		
		void load();

		void save();

		GameOption *getOptionByName(const char *name);

		GameOption *getOptionById(int id);

		const LinkedList *getOptionsList();

		const char *getOptionNameForId(int id);

	private:
		static std::auto_ptr<GameOptionManager> instance;

		GameConfigs *gameConf;

		LinkedList *options;
		GameOption *optionsById[DH_OPT_AMOUNT];


		// called by GameOption class...
		// (calls just forwared to GameConfigs class)
		void setIntOptionValue(int id, int value);

		void setBoolOptionValue(int id, bool value);

		void setFloatOptionValue(int id, float value);

		void setStringOptionValue(int id, const char *value);

		int getIntOptionValue(int id);

		bool getBoolOptionValue(int id);

		float getFloatOptionValue(int id);

		char *getStringOptionValue(int id);

		bool isOptionToggleable(int id);

		void toggleOptionValue(int id);

		void resetOptionValue(int id);

		bool doesOptionNeedApply(int id);

		// Added by Pete, used in the saving progress to save a variables value
		std::string getAsStringValue(  GameOption* conf );

		IScriptVariable::VARTYPE getOptionVariableType(int id);

		// to allow GameOption to access the above methods
		friend class GameOption;

	};
}

#endif


