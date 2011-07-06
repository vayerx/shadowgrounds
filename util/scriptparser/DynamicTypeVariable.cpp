
#include "precompiled.h"

#include "DynamicTypeVariable.h"
#include "../../convert/str2int.h"
#include <math.h>
#include "../../util/Debug_MemoryManager.h"

util::scriptparser::IDynamicTypeValueUserValueHandler *util::scriptparser::DynamicTypeVariable::userHandler = NULL;
util::scriptparser::IDynamicTypeValueErrorListener *util::scriptparser::DynamicTypeVariable::errorListener = NULL;

#define DTV_ERROR(x) \
	if (errorListener) \
  { \
		errorListener->error(x); \
	} else { \
		LOG_ERROR(x); \
	}

#define DTV_WARNING(x) \
	if (errorListener) \
  { \
		errorListener->warning(x); \
	} else { \
		LOG_WARNING(x); \
	}


namespace util
{
namespace scriptparser
{


void DynamicTypeVariable::setStringValue(const std::string &value)
{
	valueType = DYNAMICTYPEVARIABLE_TYPE_STRING;
	stringValue = value;
}

void DynamicTypeVariable::setBoolValue(bool value)
{
	valueType = DYNAMICTYPEVARIABLE_TYPE_BOOL;
	boolValue = value;
}

void DynamicTypeVariable::setIntValue(int value)
{
	valueType = DYNAMICTYPEVARIABLE_TYPE_INT;
	intValue = value;
}

void DynamicTypeVariable::setFloatValue(float value)
{
	valueType = DYNAMICTYPEVARIABLE_TYPE_INT;
	floatValue = value;
}

void DynamicTypeVariable::setUserValue(void *userValue)
{
	valueType = DYNAMICTYPEVARIABLE_TYPE_USER;
	this->userValue = userValue;
}

DynamicTypeVariable::DYNAMICTYPEVARIABLE_TYPE DynamicTypeVariable::getValueType() const
{
	return valueType;
}

static bool do_cast_warning = false;

std::string DynamicTypeVariable::getStringValue() const
{
	// TODO: optimize? (unnecessary temporary gets created here)
	do_cast_warning = true;
	const std::string &tmp = getCastStringValue();
	do_cast_warning = false;
	return tmp;
}

std::string DynamicTypeVariable::getCastStringValue() const
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_STRING)
	{
		return stringValue;
	} else {
		// dynamically cast it...
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastStringValue - Casting 'int' to 'string' (cast should be specifically requested to avoid this warning)."); }
			return std::string(int2str(intValue));
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_BOOL)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastStringValue - Casting 'bool' to 'string' (cast should be specifically requested to avoid this warning)."); }
			if (boolValue)
				return std::string("true");
			else
				return std::string("false");
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastStringValue - Casting 'float' to 'string' (cast should be specifically requested to avoid this warning)."); }
			char tmpbuf[32];
			sprintf(tmpbuf, "%f", floatValue);
			std::string ret = std::string(tmpbuf);
			return ret;
		}

		DTV_ERROR("DynamicTypeVariable::getCastStringValue - Cannot cast unsupported type to 'string'.");
		return "";
	}
}

bool DynamicTypeVariable::getBoolValue() const
{	
	do_cast_warning = true;
	bool tmp = getCastBoolValue();
	do_cast_warning = false;
	return tmp;
}

bool DynamicTypeVariable::getCastBoolValue() const
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_BOOL)
	{
		return boolValue;
	} else {
		// dynamically cast it...
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastBoolValue - Casting 'int' to 'bool' (cast should be specifically requested to avoid this warning)."); }
			if (intValue != 0)
				return true;
			else
				return false;
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_STRING)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastBoolValue - Casting 'string' to 'bool' (cast should be specifically requested to avoid this warning)."); }
			if (!stringValue.empty())
			{
				if (stringValue == "0")
				{
					return false;
				} else if (stringValue == "false") {
					return false;
				} else {
					if (stringValue != "true"
						&& stringValue != "1")
					{
						DTV_WARNING("DynamicTypeVariable::getCastBoolValue - String value was not a proper bool value.");
						LOG_DEBUG(stringValue.c_str());
					}
					return true;
				}
			} else {
				DTV_WARNING("DynamicTypeVariable::getCastBoolValue - Casting empty string to bool.");
				return false;
			}
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastStringValue - Casting 'float' to 'bool' (cast should be specifically requested to avoid this warning)."); }
			if (floatValue != 0.0f)
				return true;
			else
				return false;
		}

		DTV_ERROR("DynamicTypeVariable::getCastBoolValue - Cannot cast unsupported type to 'bool'.");
		return false;
	}
}

int DynamicTypeVariable::getIntValue() const
{	
	do_cast_warning = true;
	int tmp = getCastIntValue();
	do_cast_warning = false;
	return tmp;
}

int DynamicTypeVariable::getCastIntValue() const
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
	{
		return intValue;
	} else {
		// dynamically cast it...
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_BOOL)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastIntValue - Casting 'bool' to 'int' (cast should be specifically requested to avoid this warning)."); }
			if (boolValue)
				return 1;
			else
				return 0;
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_STRING)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastIntValue - Casting 'string' to 'int' (cast should be specifically requested to avoid this warning)."); }
			if (!stringValue.empty())
			{
				int tmp = str2int(stringValue.c_str());
				if (str2int_errno() != 0)
				{
					DTV_WARNING("DynamicTypeVariable::getCastIntValue - String was not a proper int value.");
					LOG_DEBUG(stringValue.c_str());
				}
				return tmp;
			} else {
				DTV_WARNING("DynamicTypeVariable::getCastIntValue - Casting empty string to int.");
				return 0;
			}
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastIntValue - Casting 'float' to 'int' (cast should be specifically requested to avoid this warning)."); }
			return (int)(floatValue);
		}


		DTV_ERROR("DynamicTypeVariable::getCastIntValue - Cannot cast unsupported type to 'int'.");
		return 0;
	}
}

float DynamicTypeVariable::getFloatValue() const
{	
	do_cast_warning = true;
	float tmp = getCastFloatValue();
	do_cast_warning = false;
	return tmp;
}

float DynamicTypeVariable::getCastFloatValue() const
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
	{
		return floatValue;
	} else {
		// dynamically cast it...
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastFloatValue - Casting 'int' to 'float' (cast should be specifically requested to avoid this warning)."); }
			return (float)intValue;
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_BOOL)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastFloatValue - Casting 'bool' to 'float' (cast should be specifically requested to avoid this warning)."); }
			if (boolValue)
				return 1.0f;
			else
				return 0.0f;
		}
		if (valueType == DYNAMICTYPEVARIABLE_TYPE_STRING)
		{
			if (do_cast_warning) { DTV_WARNING("DynamicTypeVariable::getCastFloatValue - Casting 'string' to 'float' (cast should be specifically requested to avoid this warning)."); }
			if (!stringValue.empty())
			{
				float tmp = (float)atof(stringValue.c_str());
				//if (errno() != 0)
				//{
				//	DTV_WARNING("DynamicTypeVariable::getCastFloatValue - String was not a proper float value.");
				//	LOG_DEBUG(stringValue.c_str());
				//}
				return tmp;
			} else {
				DTV_WARNING("DynamicTypeVariable::getCastFloatValue - Casting empty string to float.");
				return 0.0f;
			}
		}

		DTV_ERROR("DynamicTypeVariable::getCastFloatValue - Cannot cast unsupported type to 'float'.");
		return 0.0f;
	}
}

void *DynamicTypeVariable::getUserValue() const
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_USER)
	{
		return userValue;
	} else {
		// TODO: dynamically cast it...???
		// (the user type handler should tell us if the cast is possible, and do that)
		DTV_ERROR("DynamicTypeVariable::getCastFloatValue - Cannot cast any other type to 'user type'.");

		return NULL;
	}
}

DynamicTypeVariable& DynamicTypeVariable::operator= (const DynamicTypeVariable &v)
{
	valueType = v.valueType;
	boolValue = v.boolValue;
	intValue = v.intValue;
	floatValue = v.floatValue;
	stringValue = v.stringValue;
	userValue = v.userValue;
	return *this;
}

DynamicTypeVariable& DynamicTypeVariable::operator= (bool value)
{
	setBoolValue(value);
	return *this;
}

DynamicTypeVariable& DynamicTypeVariable::operator= (int value)
{
	setIntValue(value);
	return *this;
}

DynamicTypeVariable& DynamicTypeVariable::operator= (float value)
{
	setFloatValue(value);
	return *this;
}

DynamicTypeVariable& DynamicTypeVariable::operator= (const std::string &value)
{
	setStringValue(value);
	return *this;
}



DynamicTypeVariable& DynamicTypeVariable::operator+= (const DynamicTypeVariable &v)
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			intValue += v.intValue;
			return *this;
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			intValue += (int)v.floatValue;
			return *this;
		}
	}
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			floatValue += v.floatValue;
			return *this;
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			floatValue += (float)v.intValue;
			return *this;
		}
	}

	return *this;
}



DynamicTypeVariable& DynamicTypeVariable::operator-= (const DynamicTypeVariable &v)
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			intValue -= v.intValue;
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			intValue -= (int)v.floatValue;
		}
	}
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			floatValue -= v.floatValue;
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			floatValue -= (float)v.intValue;
		}
	}
	return *this;
}



DynamicTypeVariable& DynamicTypeVariable::operator*= (const DynamicTypeVariable &v)
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			intValue *= v.intValue;
		}
	}
	return *this;
}



DynamicTypeVariable& DynamicTypeVariable::operator/= (const DynamicTypeVariable &v)
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (v.intValue != 0)
			{
				intValue /= v.intValue;
			} else {
				DTV_ERROR("DynamicTypeVariable - division by zero (integer/integer).");
			}
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (v.floatValue != 0)
			{
				valueType = DYNAMICTYPEVARIABLE_TYPE_FLOAT;
				floatValue = (float)(intValue) / v.floatValue;
			} else {
				DTV_ERROR("DynamicTypeVariable - division by zero (integer/float).");
			}
		}
	}
	else if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (v.floatValue != 0.0f)
			{
				floatValue /= v.floatValue;
			} else {
				DTV_ERROR("DynamicTypeVariable - division by zero (float/float).");
			}
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (v.intValue != 0)
			{
				floatValue /= (float)(v.intValue);
			} else {
				DTV_ERROR("DynamicTypeVariable - division by zero (float/int).");
			}
		}
	}
	return *this;
}



DynamicTypeVariable& DynamicTypeVariable::operator%= (const DynamicTypeVariable &v)
{
	if (valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (v.intValue != 0)
			{
				intValue %= v.intValue;
			} else {
				DTV_ERROR("DynamicTypeVariable - modulo division by zero (integer/integer).");
			}
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (v.floatValue != 0)
			{
				valueType = DYNAMICTYPEVARIABLE_TYPE_FLOAT;
				floatValue = fmod((float)(intValue), v.floatValue);
			} else {
				DTV_ERROR("DynamicTypeVariable - modulo division by zero (integer/float).");
			}
		}
	}
	else if (valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
	{
		if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_FLOAT)
		{
			if (v.floatValue != 0.0f)
			{
				floatValue = fmod(floatValue, v.floatValue);
			} else {
				DTV_ERROR("DynamicTypeVariable - modulo division by zero (float/float).");
			}
		}
		else if (v.valueType == DYNAMICTYPEVARIABLE_TYPE_INT)
		{
			if (v.intValue != 0)
			{
				floatValue = fmod(floatValue, (float)(v.intValue));
			} else {
				DTV_ERROR("DynamicTypeVariable - modulo division by zero (float/int).");
			}
		}
	}
	return *this;
}


}
}


