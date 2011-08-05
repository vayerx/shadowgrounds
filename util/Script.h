
#ifndef SCRIPT_H
#define SCRIPT_H

#include <map>

#include <string>
#include <vector>

#include "ScriptProcess.h"
#include "IScriptProcessor.h"

#define SCRIPT_DATATYPE_NONE 0
#define SCRIPT_DATATYPE_INT 1
#define SCRIPT_DATATYPE_STRING 2
#define SCRIPT_DATATYPE_FLOAT 3

// NOTE: these ones are for variables only! (not supported by commands!)
#define SCRIPT_DATATYPE_ARRAY 4
#define SCRIPT_DATATYPE_POSITION 5

// NOTE: these ones are for commands only! (not supported by variables!)
// TODO:
#define SCRIPT_DATATYPE_EXPRESSION 6


#define SCRIPT_HASHCODE_CALC(name, result) \
	{ \
		int hashCode = 0; \
		int hashi = 0; \
		int hashmult = 0; \
		while(name[hashi] != '\0') \
		{ \
			hashCode += (name[hashi] << hashmult); \
			hashCode += (name[hashi] * (13 + hashi)); \
			hashmult+=4; \
			if (hashmult > 23) hashmult -= 23; \
			hashi++; \
		} \
		*result = hashCode; \
	}


#define SCRIPT_CMD_NOP 0
#define SCRIPT_CMD_GOTO 1
#define SCRIPT_CMD_LABEL 2
#define SCRIPT_CMD_CALL 3
#define SCRIPT_CMD_SUB 4
#define SCRIPT_CMD_ENDSUB 5
#define SCRIPT_CMD_IF 6
#define SCRIPT_CMD_THEN 7
#define SCRIPT_CMD_ELSE 8
#define SCRIPT_CMD_ENDIF 9
#define SCRIPT_CMD_SELECT 10
#define SCRIPT_CMD_ENDSELECT 11
#define SCRIPT_CMD_CASE 12
#define SCRIPT_CMD_DEFAULT 13
#define SCRIPT_CMD_NULL 14
#define SCRIPT_CMD_EXTERNCALL 15
#define SCRIPT_CMD_EXTERNCALLPUSH 16
#define SCRIPT_CMD_EXTERNCALLPOP 17
#define SCRIPT_CMD_RETURN 18
#define SCRIPT_CMD_EXTERNCALLRETURN 19
#define SCRIPT_CMD_PUSHVALUE 20
#define SCRIPT_CMD_POPVALUE 21
#define SCRIPT_CMD_GLOBAL 22
#define SCRIPT_CMD_LOCAL 23
#define SCRIPT_CMD_SETVARIABLE 24
#define SCRIPT_CMD_GETVARIABLE 25
#define SCRIPT_CMD_SHAREDLOCAL 26
#define SCRIPT_CMD_SETSECONDARY 27
#define SCRIPT_CMD_GETSECONDARY 28
#define SCRIPT_CMD_ADDSECONDARY 29
#define SCRIPT_CMD_DECREASESECONDARY 30
#define SCRIPT_CMD_MULTIPLYSECONDARY 31
#define SCRIPT_CMD_DIVIDESECONDARY 32
#define SCRIPT_CMD_LOOP 33
#define SCRIPT_CMD_ENDLOOP 34
#define SCRIPT_CMD_BREAKLOOP 35
#define SCRIPT_CMD_RESTARTLOOP 36
#define SCRIPT_CMD_SUBEXISTS 37
#define SCRIPT_CMD_CALLIFEXISTS 38
#define SCRIPT_CMD_ADDVARIABLETOVALUE 39
#define SCRIPT_CMD_DECREASEVARIABLEFROMVALUE 40
#define SCRIPT_CMD_ADDVALUETOVARIABLE 41
#define SCRIPT_CMD_DECREASEVALUEFROMVARIABLE 42
#define SCRIPT_CMD_EXTERNCALLRETURNMULTIPLE 43
#define SCRIPT_CMD_PERMANENTGLOBAL 44
#define SCRIPT_CMD_SETARRAYVARIABLEVALUES 45
#define SCRIPT_CMD_RETURNMULTIPLE 46
#define SCRIPT_CMD_CASES 47
#define SCRIPT_CMD_UPTOCASE 48
#define SCRIPT_CMD_PUSHCALLSTACK_INT 49
#define SCRIPT_CMD_POPCALLSTACK_INT 50
#define SCRIPT_CMD_PUSHCALLSTACK_MARKER 51
#define SCRIPT_CMD_POPCALLSTACK_MARKER 52
#define SCRIPT_CMD_PARAMGETVARIABLE 53
#define SCRIPT_CMD_VARIABLEEQUALSVALUE 54

#define SCRIPT_KEYWORDS_AMOUNT 55


namespace util
{
	class VarOptimizeData;

	class VariableDataType
	{
	public:
		VariableDataType()
		{
			name = NULL;
			dataType = SCRIPT_DATATYPE_NONE;
			stringData = NULL;
			intValue = 0;
			floatValue = 0;
			xValue = 0;
			yValue = 0;
			zValue = 0;
			arrayValue = NULL;
			arraySize = 0;
			permanent = false;
			global = true;
		}

		~VariableDataType()
		{
			if (name != NULL)
			{
				delete [] name;
				name = NULL;
			}
			if (stringData != NULL)
			{
				delete [] stringData;
				stringData = NULL;
			}
			if (arrayValue != NULL)
			{
				delete [] arrayValue;
				arrayValue = NULL;
			}
		}

		VariableDataType(const VariableDataType &obj)
		{
			name = NULL;
			stringData = NULL;
			*this = obj;
		}

		VariableDataType &operator= (const VariableDataType &obj)
		{
			if (this == &obj)
				return *this;

			if (name != NULL)
			{
				delete [] name;
				name = NULL;
			}
			if (obj.name != NULL)
			{
				name = new char[strlen(obj.name) + 1];
				strcpy(name, obj.name);
			} else {
				name = NULL;
			}
			if (stringData != NULL)
			{
				delete [] stringData;
				stringData = NULL;
			}
			if (obj.stringData != NULL)
			{
				stringData = new char[strlen(obj.stringData) + 1];
				strcpy(stringData, obj.stringData);
			} else {
				stringData = NULL;
			}
			dataType = obj.dataType;
			intValue = obj.intValue;
			floatValue = obj.floatValue;
			xValue = obj.xValue;
			yValue = obj.yValue;
			zValue = obj.zValue;
			if (obj.arrayValue != NULL
				&& obj.arraySize > 0)
			{
				arraySize = obj.arraySize;
				arrayValue = new int[arraySize];
				for (int i = 0; i < arraySize; i++)
				{
					arrayValue[i] = obj.arrayValue[i];
				}
			} else {
				arraySize = 0;
				arrayValue = NULL;
			}
			permanent = obj.permanent;
			global = obj.global;

			return *this;
		}

		char *name;
		int dataType;
		char *stringData;
		int intValue;
		float floatValue;
		float xValue;
		float yValue;
		float zValue;
		int *arrayValue;
		int arraySize;
		bool permanent;
		bool global;
	};

	typedef std::map<int, int> SubIPHashType;
	typedef std::map<int, int> VariableIPHashType;
	typedef std::map<int, VariableDataType> VariableHashType;

	class ScriptManager;

	class Script
	{
	public:

		Script();
		~Script();

		ScriptProcess *createNewProcess(const char *startSub);

		void run(ScriptProcess *sp, bool pausable = true);

		bool hasSub(const char *subName);

		char *matchSuitableCommands(int *amount, const char *command,
			int *smallestMatchLength);

		char *getName();

		// returns debug dump, you should delete the returned string
		// once done with it. (or you'll leak memory)
		char *getDebugDump(ScriptProcess *sp, bool fullDump = false);

		static void newGlobalIntVariable( const std::string& global_int, bool permanent = false );
		static void newGlobalStringVariable( const std::string& global_string, bool permanent = false );
		static void newGlobalFloatVariable( const std::string& global_float, bool permanent = false );
		static void newGlobalPositionVariable( const std::string& global_position, bool permanent = false );
		static void newGlobalArrayVariable( const std::string& global_array, int size, bool permanent = false );

		static int getGlobalVariableType(const char *variablename);

		static bool setGlobalIntVariableValue(const char *variablename, int value);
		static bool getGlobalIntVariableValue(const char *variablename, int *value);

		static bool setGlobalStringVariableValue(const char *variablename, const char *value);
		static bool getGlobalStringVariableValue(const char *variablename, const char **value);

		static bool setGlobalFloatVariableValue(const char *variablename, float value);
		static bool getGlobalFloatVariableValue(const char *variablename, float *value);

		static bool setGlobalPositionVariableValue(const char *variablename, float xValue, float yValue, float zValue);
		static bool getGlobalPositionVariableValue(const char *variablename, float *xValue, float *yValue, float *zValue);

		static bool setGlobalArrayVariableValue(const char *variablename, int entry, int value);
		static bool getGlobalArrayVariableValue(const char *variablename, int entry, int *value);
		static bool getGlobalArrayVariableSize(const char *variablename, int *size);

		static void resetNonPermanentGlobalVariables();

		static LinkedList *getGlobalVariableList(bool permanentOnly);
		static VariableHashType *getGlobalVariableHash();

		// prevents "forced pause" when a script seems to be misbehaving
		// and does not pause within given tick limit
		// (setting this to no will currently 50x the limit, totally
		// preventing forced pause might cause erronous scripts to lock up)
		void setNoForcedPause(bool noForcedPause);

	private:

		void optimizeArrayVariableData(ScriptProcess *sp, int ip, const char *s, int arrIdxStart, int arrIdxEnd, bool arrIsValueIndex, bool arrIsRegIndex, int *varHashCode);

		void logMessage(ScriptProcess *sp, const char *message, int loglevel);

		int getSubIP(const char *subName);

		int getLabelIP(char *labelName);

		bool addCommand(const char *cmd, const char *data);

		void optimizeJumps();

		char *name;

		//static char **keywords;
		//static int keywordsAmount;

		char **processorKeywords;
		int *processorDatatypes;
		int processorKeywordsAmount;

		IScriptProcessor *processor;

		SubIPHashType *subIPHash;
		VariableIPHashType *variableIPHash;

		//static VariableHashType *globalVariableHash;
		
		int allocedSize;

		int commandAmount;
		int *commandArray;
		floatint *intDataArray;
		char **stringDataArray;
		VarOptimizeData **varOptimizeDataArray;

		LinkedList *loopNumBuf;

		bool noForcedPause;

		friend class ScriptManager;

		// access to logMessage for ScriptProcess...
		friend void ScriptProcess::error(const char *message);
		friend void ScriptProcess::warning(const char *message);
		friend void ScriptProcess::debug(const char *message);
	};

	extern bool parse_script_params(const std::string &paramLine, std::vector<std::string> &result);

}

#endif

