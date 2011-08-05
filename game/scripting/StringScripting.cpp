
#include "precompiled.h"

#include "StringScripting.h"

#include "scripting_macros_start.h"
#include "string_script_commands.h"
#include "scripting_macros_end.h"

#include "GameScriptData.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../system/Logger.h"

#include "../../util/Debug_MemoryManager.h"


using namespace ui;

namespace game
{
	void StringScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_SETSTRING:
			if (stringData == NULL)
			{
				gsd->setStringValue("");
			} else {
				gsd->setStringValue(stringData);
			}
			break;

		case GS_CMD_APPENDSTRING:
		case GS_CMD_APPENDNEXTLINESTRING:
			if (stringData != NULL)
			{
				if (gsd->stringValue != NULL)
				{
					char *tmp = new char[strlen(gsd->stringValue) + strlen(stringData) + 2];
					strcpy(tmp, gsd->stringValue);
					if (command == GS_CMD_APPENDNEXTLINESTRING)
						strcat(tmp, "\n");
					strcat(tmp, stringData);
					gsd->setStringValue(tmp);
					delete [] tmp;
				} else {
					sp->warning("StringScripting::process - Attempt to append/appendNextLineString to null string.");
				}
			} else {
				sp->warning("StringScripting::process - append/appendNextLineString parameter missing.");
			}
			break;

		case GS_CMD_APPENDSTRINGLINEFEED:
			if (gsd->stringValue != NULL)
			{
				char *tmp = new char[strlen(gsd->stringValue) + 2];
				strcpy(tmp, gsd->stringValue);
				strcat(tmp, "\n");
				gsd->setStringValue(tmp);
				delete [] tmp;
			} else {
				sp->warning("StringScripting::process - Attempt to appendLineFeed to null string.");
			}
			break;

		case GS_CMD_PREPENDSTRING:
			if (stringData != NULL)
			{
				if (gsd->stringValue != NULL)
				{
					char *tmp = new char[strlen(gsd->stringValue) + strlen(stringData) + 1];
					strcpy(tmp, stringData);
					strcat(tmp, gsd->stringValue);
					gsd->setStringValue(tmp);
					delete [] tmp;
				} else {
					sp->warning("StringScripting::process - Attempt to prependString to null string.");
				}
			} else {
				sp->warning("StringScripting::process - prependString parameter missing.");
			}
			break;

		case GS_CMD_SUBSTRINGFROMSTART:
			if (gsd->stringValue != NULL)
			{
				if (intData < (int)strlen(gsd->stringValue)
					&& intData >= 0)
				{
					gsd->stringValue[intData] = '\0';
				} else {
					sp->debug("StringScripting::process - substringFromStart parameter value greater or equal to string length.");
				}
			} else {
				sp->warning("StringScripting::process - Attempt to substringFromStart from null string.");
			}
			break;

		case GS_CMD_SUBSTRINGFROMEND:
			if (gsd->stringValue != NULL)
			{
				int slen = strlen(gsd->stringValue);
				if (intData < slen
					&& intData >= 0)
				{
					for (int i = 0; i < intData + 1; i++)
					{
						gsd->stringValue[i] = gsd->stringValue[(slen - intData) + i];
					}
				} else {
					sp->debug("StringScripting::process - substringFromEnd parameter value greater or equal to string length.");
				}
			} else {
				sp->warning("StringScripting::process - Attempt to substringFromEnd from null string.");
			}
			break;

		case GS_CMD_APPENDVALUETOSTRING:
			if (gsd->stringValue != NULL)
			{
				char *valstr = int2str(*lastValue);
				char *tmp = new char[strlen(gsd->stringValue) + strlen(valstr) + 1];
				strcpy(tmp, gsd->stringValue);
				strcat(tmp, valstr);
				gsd->setStringValue(tmp);
				delete [] tmp;
			} else {
				sp->warning("StringScripting::process - Attempt to appendValueToString to null string.");
			}
			break;

		case GS_CMD_VALUETOSTRING:
			gsd->setStringValue(int2str(*lastValue));
			break;

		case GS_CMD_VALUETOCLOCKSTRING:
			{
				const char *sep = ":";
				if (stringData != NULL)
				{
					sep = stringData;
				}
				char *tmp = new char[strlen(sep) + 4 + 1];
				tmp[0] = '0' + (((*lastValue) / 600) % 10);
				tmp[1] = '0' + (((*lastValue) / 60) % 10);
				tmp[2] = '\0';
				strcat(tmp, sep);
				int pos = strlen(tmp);
				tmp[pos] = '0' + (((*lastValue) / 10) % 6);
				tmp[pos + 1] = '0' + ((*lastValue) % 10);
				tmp[pos + 2] = '\0';
				gsd->setStringValue(tmp);
				delete [] tmp;
			}
			break;

		case GS_CMD_stringToValue:
			if (gsd->stringValue != NULL)
			{
				*lastValue = str2int(gsd->stringValue);
				if (str2int_errno() != 0)
				{
					sp->warning("StringScripting::process - stringToValue, string was not a valid integer number.");
					LOG_DEBUG(gsd->stringValue);
				}
			} else {
				*lastValue = 0;
				sp->warning("StringScripting::process - Attempt to stringToValue for null string.");
			}
			break;

		case GS_CMD_STRINGLENGTH:
			if (gsd->stringValue != NULL)
			{
				*lastValue = (int)strlen(gsd->stringValue);				
			} else {
				*lastValue = 0;
				sp->warning("StringScripting::process - Attempt to stringLength for null string.");
			}
			break;

		case GS_CMD_ISNULLSTRING:
			if (gsd->stringValue == NULL)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_SETNULLSTRING:
			gsd->setStringValue(NULL);
			break;

		case GS_CMD_PRINTSTRINGVALUE:
			if (gsd->stringValue != NULL)
			{
				Logger::getInstance()->info(gsd->stringValue);
			} else {
				Logger::getInstance()->info("(null)");
			}
			break;

		case GS_CMD_SUBSTRINGFROMSTARTBYVALUE:
			if (gsd->stringValue != NULL)
			{
				if (*lastValue < (int)strlen(gsd->stringValue)
					&& *lastValue >= 0)
					gsd->stringValue[*lastValue] = '\0';
			} else {
				sp->warning("StringScripting::process - Attempt to substringFromStartByValue from null string.");
			}
			break;

		case GS_CMD_SUBSTRINGFROMENDBYVALUE:
			if (gsd->stringValue != NULL)
			{
				int slen = strlen(gsd->stringValue);
				if (*lastValue < slen
					&& *lastValue >= 0)
				{
					for (int i = 0; i < *lastValue + 1; i++)
					{
						gsd->stringValue[i] = gsd->stringValue[(slen - *lastValue) + i];
					}
				}
			} else {
				sp->warning("StringScripting::process - Attempt to substringFromEndByValue from null string.");
			}
			break;

		case GS_CMD_STRINGEQUALS:
			if (stringData != NULL)
			{
				if (gsd->stringValue != NULL)
				{
					if (strcmp(gsd->stringValue, stringData) == 0)
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}
				} else {
					sp->warning("StringScripting::process - Attempt to stringEquals for null string.");
					*lastValue = 0;
				}
			} else {
				sp->warning("StringScripting::process - appendString parameter missing.");
			}
			break;

		case GS_CMD_stringCutAtFirstLine:
			if (gsd->stringValue != NULL)
			{
				int slen = strlen(gsd->stringValue);
				for (int i = 0; i < slen; i++)
				{
					if(gsd->stringValue[i] == '\r' || gsd->stringValue[i] == '\n')
					{
						gsd->stringValue[i] = '\0';
						break;
					}
				}
			} else {
				sp->warning("StringScripting::process - Attempt to stringCutAtFirstLine from null string.");
			}
			break;

		default:
			sp->error("StringScripting::process - Unknown command.");
			assert(0);
		}
	}
}


