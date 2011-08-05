
#include "precompiled.h"

#include "MathScripting.h"

#include "scripting_macros_start.h"
#include "math_script_commands.h"
#include "scripting_macros_end.h"

#include "GameScriptData.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"

#include "../../util/Debug_MemoryManager.h"


using namespace ui;

namespace game
{
	void MathScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_VALUEEQUALS:
			if (*lastValue == intData)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_VALUEGREATERTHAN:
			if (*lastValue > intData)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_VALUELESSTHAN:
			if (*lastValue < intData)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_SETVALUE:
			*lastValue = intData;
			break;

		case GS_CMD_ADDVALUE:
			*lastValue += intData;
			break;

		case GS_CMD_MULTIPLYVALUE:
			*lastValue *= intData;
			break;

		case GS_CMD_DIVIDEVALUE:
			if (intData == 0)
			{ 			
				*lastValue = 0;
				sp->error("MathScripting::process - divideValue, division by zero.");
			} else {
				*lastValue /= intData;
			}
			break;

		case GS_CMD_MODULOVALUE:
			if (intData == 0)
			{ 			
				*lastValue = 0;
				sp->error("MathScripting::process - moduloValue, division by zero.");
			} else {
				*lastValue %= intData;
			}
			break;

		case GS_CMD_ORNOTSECONDARY:
			if (sp->getLastValue() != 0 || sp->getSecondaryValue() == 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ANDNOTSECONDARY:
			if (sp->getLastValue() != 0 && sp->getSecondaryValue() == 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_NOTVALUE:
			if (sp->getLastValue() == 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_NEGATEVALUE:
			*lastValue = 0 - *lastValue;
			break;

		case GS_CMD_NOTSECONDARY:
			if (sp->getSecondaryValue() == 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ORSECONDARY:
			if (sp->getLastValue() != 0 || sp->getSecondaryValue() != 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_ANDSECONDARY:
			if (sp->getLastValue() != 0 && sp->getSecondaryValue() != 0)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_CLAMPBELOW:
			if (*lastValue > intData)
				*lastValue = intData;
			break;

		case GS_CMD_CLAMPABOVE:
			if (*lastValue < intData)
				*lastValue = intData;
			break;

		case GS_CMD_getFloatVariable:
			assert(!"TODO - getFloatVariable");
			break;

		case GS_CMD_setFloatVariable:
			assert(!"TODO - setFloatVariable");
			break;

		case GS_CMD_valueToFloatValue:
			gsd->floatValue = (float)(*lastValue);
			break;

		case GS_CMD_floatValueToValue:
			*lastValue = (int)gsd->floatValue;
			break;

		case GS_CMD_multiplyFloatValueWithValue:
			gsd->floatValue *= (float)(*lastValue);
			break;

		case GS_CMD_divideFloatValueWithValue:
			if (*lastValue != 0)
			{
				gsd->floatValue /= (float)(*lastValue);
			} else {
				gsd->floatValue = 0.0f;
				sp->error("MathScripting::process - divideFloatValueWithValue, division by zero.");
			}
			break;

		case GS_CMD_addFloatValue:
			{
				float floatData = intFloat.f;
				gsd->floatValue += floatData;
			}
			break;

		case GS_CMD_divideFloatValue:
			{
				float floatData = intFloat.f;
				if (floatData != 0.0f)
				{
					gsd->floatValue /= floatData;
				} else {
					gsd->floatValue = 0.0f;
					sp->error("MathScripting::process - divideFloatValue, division by zero.");
				}
			}
			break;

		case GS_CMD_multiplyFloatValue:
			{
				float floatData = intFloat.f;
				gsd->floatValue *= floatData;
			}
			break;

		case GS_CMD_floatValueEquals:
			{
				float floatData = intFloat.f;
				if (gsd->floatValue == floatData)
					*lastValue = 1;
				else
					*lastValue = 0;
			}
			break;

		case GS_CMD_floatValueGreaterThan:
			{
				float floatData = intFloat.f;
				if (gsd->floatValue > floatData)
					*lastValue = 1;
				else
					*lastValue = 0;
			}
			break;

		case GS_CMD_floatValueLessThan:
			{
				float floatData = intFloat.f;
				if (gsd->floatValue < floatData)
					*lastValue = 1;
				else
					*lastValue = 0;
			}
			break;

		case GS_CMD_isFloatValueNAN:
			assert(!"TODO - isFloatValueNAN");
			break;

		case GS_CMD_setFloatValue:
			{
				float floatData = intFloat.f;
				gsd->floatValue = floatData;
			}
			break;

		case GS_CMD_negateFloatValue:
			gsd->floatValue = -gsd->floatValue;
			break;

		case GS_CMD_addFloatValueToValue:
			*lastValue += (int)gsd->floatValue;
			break;

		case GS_CMD_addValueToFloatValue:
			gsd->floatValue += (float)*lastValue;
			break;

		case GS_CMD_valueGreaterThanOrEqual:
			if (*lastValue >= intData)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_valueLessThanOrEqual:
			if (*lastValue <= intData)
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		default:
			sp->error("MathScripting::process - Unknown command.");
			assert(0);
		}
	}
}


