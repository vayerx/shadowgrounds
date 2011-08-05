
#ifndef VARIABLESCRIPTINGUTILS_H
#define VARIABLESCRIPTINGUTILS_H

#include "../../system/Logger.h"
#include "../../convert/str2int.h"

namespace game
{
	class Game;

	class VariableScriptingUtils
	{
	public:
		//VariableScriptingUtils(Game *game);

		void getVariableValueToInt(IScriptVariable *variable, int *resultValue)
		{
			if (variable != NULL)
			{
				if (variable->getVariableType() == game::IScriptVariable::VARTYPE_BOOLEAN)
				{
					if (variable->getBooleanValue())							
						*resultValue = 1;
					else
						*resultValue = 0;
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_INT)
				{
					*resultValue = variable->getIntValue();
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_FLOAT)
				{
					Logger::getInstance()->warning("VariableScriptingUtils::getVariableValueToInt - Converting float to int.");
					*resultValue = (int)variable->getFloatValue();
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_STRING)
				{
					Logger::getInstance()->warning("VariableScriptingUtils::getVariableValueToInt - Converting string to int.");
					*resultValue = str2int(variable->getStringValue());
				} else {
					Logger::getInstance()->error("VariableScriptingUtils::getVariableValueToInt - Unknown variable type.");
					*resultValue = 0;
				}
			} else {
				Logger::getInstance()->error("VariableScriptingUtils::getVariableValueToInt - Null parameter variable given.");
			}
		}


		void setVariableValueToInt(IScriptVariable *variable, int toValue)
		{
			if (variable != NULL)
			{
				if (variable->getVariableType() == game::IScriptVariable::VARTYPE_BOOLEAN)
				{
					if (toValue != 0)
						variable->setBooleanValue(true);
					else
						variable->setBooleanValue(false);
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_INT)
				{
					variable->setIntValue(toValue);
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_FLOAT)
				{
					Logger::getInstance()->warning("VariableScriptingUtils::setVariableValueToInt - Converting int to float.");
					variable->setFloatValue((float)toValue);
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_STRING)
				{
					Logger::getInstance()->warning("VariableScriptingUtils::setVariableValueToInt - Converting int to string.");
					variable->setStringValue(int2str(toValue));
				} else {
					Logger::getInstance()->error("VariableScriptingUtils::setVariableValueToInt - Unknown variable type.");
				}
			} else {
				Logger::getInstance()->error("VariableScriptingUtils::setVariableValueToInt - Null parameter variable given.");
			}
		}


		void logVariableValue(IScriptVariable *variable)
		{
			if (variable != NULL)
			{
				if (variable->getVariableType() == game::IScriptVariable::VARTYPE_BOOLEAN)
				{
					if (variable->getBooleanValue())
						Logger::getInstance()->info("true");
					else
						Logger::getInstance()->info("false");
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_INT)
				{
					Logger::getInstance()->info(int2str(variable->getIntValue()));
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_FLOAT)
				{
					// WARNING: sprintf to static buffer
					char buf[256];
					sprintf(buf, "%f", variable->getFloatValue());
					Logger::getInstance()->info(buf);
				}
				else if (variable->getVariableType() == game::IScriptVariable::VARTYPE_STRING)
				{
					Logger::getInstance()->info(variable->getStringValue());
				} else {
					Logger::getInstance()->error("VariableScriptingUtils::logVariableValue - Unknown variable type.");
				}
			} else {
				Logger::getInstance()->error("VariableScriptingUtils::logVariableValue - Null parameter variable given.");
			}
		}


		void splitFromComma(const char *stringData, char *first, int firstMaxLen, char *second, int secondMaxLen)
		{
			if (stringData != NULL)
			{
				int firstPos = 0;
				int secondPos = 0;
				bool atSecond = false;
				for (int i = 0; i < (int)strlen(stringData); i++)
				{
					if (stringData[i] == ',' && !atSecond)
					{
						atSecond = true;
					} else {
						if (stringData[i] == ',')
						{
							Logger::getInstance()->debug("VariableScriptingUtils::splitFromComma - Multiple comma seperators ignored.");
						}
						if (atSecond)
						{
							if (secondPos < secondMaxLen - 1)
							{
								second[secondPos] = stringData[i];
								secondPos++;
							}
						} else {
							if (firstPos < firstMaxLen - 1)
							{
								first[firstPos] = stringData[i];
								firstPos++;
							}
						}
					}
				}
				first[firstPos] = '\0';
				second[secondPos] = '\0';
			} else {
				Logger::getInstance()->error("VariableScriptingUtils::splitFromComma - Null parameter variable given.");
				first[0] = '\0';
				second[0] = '\0';
			}
		}


	};
}

#endif


