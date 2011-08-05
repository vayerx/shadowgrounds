
#include "precompiled.h"

#include "CheckedIntValue.h"
#include "../system/Logger.h"

util::ScriptProcess *checked_int_value_sp = NULL;

#define CHECKED_INT_WARNING(x) \
{ \
	if (checked_int_value_sp != NULL) \
	{ \
		checked_int_value_sp->warning(x); \
	} else { \
		Logger::getInstance()->warning(x); \
		assert(0 && x); \
	} \
} \


bool operator== (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int == CheckedIntValue).");
	}
	return (v1 == v2.value);
}

bool operator== (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue == int).");
	}
	return (v1.value == v2);
}

bool operator== (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue == CheckedIntValue).");
	}
	return (v1.value == v2.value);
}

bool operator!= (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int != CheckedIntValue).");
	}
	return (v1 != v2.value);
}

bool operator!= (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue != int).");
	}
	return (v1.value != v2);
}

bool operator!= (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue != CheckedIntValue).");
	}
	return (v1.value != v2.value);
}

bool operator>= (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int >= CheckedIntValue).");
	}
	return (v1 >= v2.value);
}

bool operator>= (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue >= int).");
	}
	return (v1.value >= v2);
}

bool operator>= (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue >= CheckedIntValue).");
	}
	return (v1.value >= v2.value);
}

bool operator<= (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int <= CheckedIntValue).");
	}
	return (v1 <= v2.value);
}

bool operator<= (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue <= int).");
	}
	return (v1.value <= v2);
}

bool operator<= (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue <= CheckedIntValue).");
	}
	return (v1.value <= v2.value);
}

int operator+ (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int + CheckedIntValue).");
	}
	return (v1 + v2.value);
}

int operator+ (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue + int).");
	}
	return (v1.value + v2);
}

int operator+ (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue + CheckedIntValue).");
	}
	return (v1.value + v2.value);
}

int operator- (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int - CheckedIntValue).");
	}
	return (v1 - v2.value);
}

int operator- (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue - int).");
	}
	return (v1.value - v2);
}

int operator- (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue - CheckedIntValue).");
	}
	return (v1.value - v2.value);
}

int operator* (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int * CheckedIntValue).");
	}
	return (v1 * v2.value);
}

int operator* (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue * int).");
	}
	return (v1.value * v2);
}

int operator* (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue * CheckedIntValue).");
	}
	return (v1.value * v2.value);
}

int operator/ (const int &v1, const CheckedIntValue &v2)
{
	if (!v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (int / CheckedIntValue).");
	}
	if (v2.value != 0)
	{
		return (v1 / v2.value);
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (int / CheckedIntValue).");
		return 0;
	}
}

int operator/ (const CheckedIntValue &v1, const int &v2)
{
	if (!v1.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue / int).");
	}
	if (v2 != 0)
	{
		return (v1.value / v2);
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (CheckedIntValue / int).");
		return 0;
	}
}

int operator/ (const CheckedIntValue &v1, const CheckedIntValue &v2)
{
	if (!v1.initialized
		|| !v2.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (CheckedIntValue / CheckedIntValue).");
	}
	if (v2.value != 0)
	{
		return (v1.value / v2.value);
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (CheckedIntValue / CheckedIntValue).");
		return 0;
	}
}




CheckedIntValue::CheckedIntValue() 
	: value(0),
	initialized(false)
{
}

CheckedIntValue::~CheckedIntValue()
{
	//if (!initialized)
	//  CHECKED_INT_WARNING("Unused CheckedIntValue.");
}

CheckedIntValue::CheckedIntValue(const CheckedIntValue &v)
{
	if (!v.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (copy).");
	}
	initialized = true;
	this->value = v.value;
}

CheckedIntValue& CheckedIntValue::operator= (const CheckedIntValue &v)
{
	if (!v.initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (= CheckedIntValue).");
	}
	initialized = true;
	this->value = v.value;
	return *this;
}

CheckedIntValue& CheckedIntValue::operator= (int value)
{
	initialized = true;
	this->value = value;
	return *this;
}

/*
CheckedIntValue& CheckedIntValue::operator= (float value)
{
	initialized = true;
	this->value = (int)value;
	return *this;
}
*/

CheckedIntValue& CheckedIntValue::operator++()
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (++).");
	}
	this->value++;
	return *this;
}

CheckedIntValue& CheckedIntValue::operator--()
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (--).");
	}
	this->value--;
	return *this;
}

CheckedIntValue CheckedIntValue::operator++(int)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (++).");
	}
	CheckedIntValue copy(*this);
	this->value++;
	return copy;
}

CheckedIntValue CheckedIntValue::operator--(int)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (--).");
	}
	CheckedIntValue copy(*this);
	this->value--;
	return copy;
}


CheckedIntValue& CheckedIntValue::operator+= (int value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (+ int).");
	}
	this->value += value;
	return *this;
}

/*
CheckedIntValue& CheckedIntValue::operator+= (float value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (+ float).");
	}
	this->value = (int)((float)this->value + value);
	return *this;
}
*/

CheckedIntValue& CheckedIntValue::operator+= (const CheckedIntValue& value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (+ CheckedIntValue).");
	}
	this->value += value.value;
	return *this;
}

CheckedIntValue& CheckedIntValue::operator-= (int value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (- int).");
	}
	this->value -= value;
	return *this;
}

/*
CheckedIntValue& CheckedIntValue::operator-= (float value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (- float).");
	}
	this->value = (int)((float)this->value - value);
	return *this;
}
*/

CheckedIntValue& CheckedIntValue::operator-= (const CheckedIntValue& value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (- CheckedIntValue).");
	}
	this->value -= value.value;
	return *this;
}

CheckedIntValue& CheckedIntValue::operator*= (int value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (* int).");
	}
	this->value *= value;
	return *this;
}

/*
CheckedIntValue& CheckedIntValue::operator*= (float value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (* float).");
	}
	this->value = (int)((float)this->value * value);
	return *this;
}
*/

CheckedIntValue& CheckedIntValue::operator*= (const CheckedIntValue& value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (* CheckedIntValue).");
	}
	this->value *= value.value;
	return *this;
}

CheckedIntValue& CheckedIntValue::operator/= (int value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (/ int).");
	}
	if (value != 0)
	{
		this->value /= value;
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (/ int).");
	}
	return *this;
}

/*
CheckedIntValue& CheckedIntValue::operator/= (float value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (/ float).");
	}
	if (value != 0.0f)
	{
		this->value = (int)((float)this->value / value);
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (/ float).");
	}
	return *this;
}
*/


CheckedIntValue& CheckedIntValue::operator/= (const CheckedIntValue& value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (/ CheckedIntValue).");
	}
	if (value.value != 0)
	{
		this->value /= value.value;
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (/ CheckedIntValue).");
	}
	return *this;
}

CheckedIntValue& CheckedIntValue::operator%= (int value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (% int).");
	}
	if (value != 0)
	{
		this->value %= value;
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (% int).");
	}
	return *this;
}

/*
CheckedIntValue& CheckedIntValue::operator%= (float value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (% float).");
	}
	if (value != 0.0f)
	{
		this->value = (int)((float)this->value % value);
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (% float).");
	}
	return *this;
}
*/


CheckedIntValue& CheckedIntValue::operator%= (const CheckedIntValue& value)
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (% CheckedIntValue).");
	}
	if (value.value != 0)
	{
		this->value %= value.value;
	} else {
		CHECKED_INT_WARNING("CheckedIntValue - division by zero (% CheckedIntValue).");
	}
	return *this;
}

CheckedIntValue::CheckedIntValue(int value)
{
	initialized = true;
	this->value = value;
}

CheckedIntValue::CheckedIntValue(float value)
{
	initialized = true;
	this->value = (int)value;
}


CheckedIntValue::operator int() const
{
	if (!initialized)
	{
		//CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (-> int).");
	}
	return this->value;
}

/*
CheckedIntValue::operator float() const
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (-> float).");
	}
	return (float)this->value;
}
*/

CheckedIntValue::operator void*() const
{
	if (!initialized)
	{
		CHECKED_INT_WARNING("CheckedIntValue - use of unitialized value (-> void*).");
	}
  return (void*)(this->value);
}

// for error checking...
bool CheckedIntValue::wasInitialized()
{
	return this->initialized;
}

void CheckedIntValue::clearInitializedFlag()
{
	this->initialized = false;
}

