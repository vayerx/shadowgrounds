
#ifndef GAMEOPTION_H
#define GAMEOPTION_H

#include "IScriptVariable.h"

namespace game
{
	class GameOptionManager;

	/**
	 * A class that holds one game configuration option.
	 * 
   * @version 1.0, 13.3.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see GameOptionManager
	 */
	class GameOption : public IScriptVariable
	{
		public:
			virtual IScriptVariable::VARTYPE getVariableType();

			virtual void setIntValue(int value);

			virtual void setBooleanValue(bool value);

			virtual void setFloatValue(float value);

			virtual void setStringValue(const char *value);

			virtual int getIntValue();

			virtual bool getBooleanValue();

			virtual float getFloatValue();

			virtual char *getStringValue();

			virtual bool isReadOnly();

			virtual bool isToggleable();

			virtual void toggleValue();

			virtual void resetValue();

			virtual bool doesNeedApply();

			virtual void makeReadOnly();

			int getId();

			GameOption(GameOptionManager *manager, int id);

			virtual ~GameOption();

		private:
			GameOptionManager *manager;
			int id;
			bool readOnly;
	};
}

#endif

