// Copyright 2002-2004 Frozenbyte Ltd.

// from 1350

#define GS_CMD_BASE 1350

GS_CMD(0, GS_CMD_VALUEEQUALS, "valueEquals", INT)
GS_CMD(1, GS_CMD_VALUEGREATERTHAN, "valueGreaterThan", INT)
GS_CMD(2, GS_CMD_VALUELESSTHAN, "valueLessThan", INT)
GS_CMD(3, GS_CMD_SETVALUE, "setValue", INT)
GS_CMD(4, GS_CMD_ADDVALUE, "addValue", INT)
GS_CMD(5, GS_CMD_MULTIPLYVALUE, "multiplyValue", INT)
GS_CMD(6, GS_CMD_DIVIDEVALUE, "divideValue", INT)
GS_CMD(7, GS_CMD_ORNOTSECONDARY, "orNotSecondary", NONE)
GS_CMD(8, GS_CMD_ANDNOTSECONDARY, "andNotSecondary", NONE)
GS_CMD(9, GS_CMD_NOTVALUE, "notValue", NONE)
GS_CMD(10, GS_CMD_NOTSECONDARY, "notSecondary", NONE)
GS_CMD(11, GS_CMD_ORSECONDARY, "orSecondary", NONE)
GS_CMD(12, GS_CMD_ANDSECONDARY, "andSecondary", NONE)
GS_CMD(13, GS_CMD_MODULOVALUE, "moduloValue", INT)
		
GS_CMD(14, GS_CMD_CLAMPABOVE, "clampAbove", INT)
GS_CMD(15, GS_CMD_CLAMPBELOW, "clampBelow", INT)

GS_CMD(16, GS_CMD_NEGATEVALUE, "negateValue", NONE)

GS_CMD_SIMPLE(17, getFloatVariable, NONE)
GS_CMD_SIMPLE(18, setFloatVariable, NONE)
GS_CMD_SIMPLE(19, valueToFloatValue, NONE)
GS_CMD_SIMPLE(20, floatValueToValue, NONE)
GS_CMD_SIMPLE(21, multiplyFloatValueWithValue, NONE)
GS_CMD_SIMPLE(22, divideFloatValueWithValue, NONE)
GS_CMD_SIMPLE(23, addFloatValue, FLOAT)
GS_CMD_SIMPLE(24, divideFloatValue, FLOAT)
GS_CMD_SIMPLE(25, multiplyFloatValue, FLOAT)
GS_CMD_SIMPLE(26, floatValueEquals, FLOAT)
GS_CMD_SIMPLE(27, floatValueGreaterThan, FLOAT)
GS_CMD_SIMPLE(28, floatValueLessThan, FLOAT)
GS_CMD_SIMPLE(29, isFloatValueNAN, NONE)
GS_CMD_SIMPLE(30, setFloatValue, FLOAT)
GS_CMD_SIMPLE(31, negateFloatValue, NONE)
GS_CMD_SIMPLE(32, addFloatValueToValue, NONE)
GS_CMD_SIMPLE(33, addValueToFloatValue, NONE)

GS_CMD_SIMPLE(34, valueGreaterThanOrEqual, INT)
GS_CMD_SIMPLE(35, valueLessThanOrEqual, INT)

#undef GS_CMD_BASE

// up to 1399
