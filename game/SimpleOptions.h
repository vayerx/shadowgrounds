
#ifndef SIMPLEOPTIONS_H
#define SIMPLEOPTIONS_H

#include "GameOptionManager.h"

namespace game
{
	class SimpleOptions
	{
		public:
			inline static bool getBool(int id)
			{
				// EVIL HAX
				// this whole thing should go away and be replaced with something sane..
				GameOption *o = game::GameOptionManager::getInstance()->getOptionById(id);
				if (o)
					return o->getBooleanValue();
				else
					return false;
			}

			inline static int getInt(int id)
			{
				return game::GameOptionManager::getInstance()->getOptionById(id)->getIntValue();
			}

			inline static float getFloat(int id)
			{
				return game::GameOptionManager::getInstance()->getOptionById(id)->getFloatValue();
			}

			inline static char *getString(int id)
			{
				return game::GameOptionManager::getInstance()->getOptionById(id)->getStringValue();
			}

			inline static void setBool(int id, bool value)
			{
				game::GameOptionManager::getInstance()->getOptionById(id)->setBooleanValue(value);
			}

			inline static void setInt(int id, int value)
			{
				game::GameOptionManager::getInstance()->getOptionById(id)->setIntValue(value);
			}

			inline static void setFloat(int id, float value)
			{
				game::GameOptionManager::getInstance()->getOptionById(id)->setFloatValue(value);
			}

			inline static void setString(int id, const char *value)
			{
				game::GameOptionManager::getInstance()->getOptionById(id)->setStringValue(value);
			}

			inline static void resetValue(int id)
			{
				game::GameOptionManager::getInstance()->getOptionById(id)->resetValue();
			}
	};
}

#endif


