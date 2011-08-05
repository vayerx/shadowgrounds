// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#include "Script.h"

// BAD DEPENDENCY DUE TO externCall COMMAND...
// ALSO DUE TO NEW MACRO MATCHING FOR AUTOCOMPLETE...
#include "ScriptManager.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <assert.h>
#include <string>
#include <vector>
#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "StringUtil.h"

// temporary for debugging
#ifdef SCRIPT_TIMING_DEBUG
#include "../system/Timer.h"
#endif

#include "../util/Debug_MemoryManager.h"


#ifdef LEGACY_FILES
// legacy scripts are assumed to be much larger in size on average (due to massively using copy pasting and 
// externIncludes instead of calls)
#define SCRIPT_ALLOC_ARRAY_SIZE 1024
#else
#define SCRIPT_ALLOC_ARRAY_SIZE 512
#endif

#define SCRIPT_FORCED_PAUSE_LIMIT 4000

#define USER_STACK_SIZE_LIMIT 1000

// this marks a local variable, instead of sharedLocal when variable command has non-zero intData 
#define SCRIPT_LOCAL_VAR_BITMASK 0x40000000

// (some hacky value to indicate unitialized value in stack.)
#define UNITIALIZED_MAGIC_VALUE -1999991234

// HACK: ...
extern util::ScriptProcess *checked_int_value_sp;


namespace util
{

	bool script_doublequotes_warning = false;

	extern int scrman_currentline;

	/*
	char **Script::keywords = NULL;
	int Script::keywordsAmount = SCRIPT_KEYWORDS_AMOUNT;

	VariableHashType *Script::globalVariableHash = NULL;
	*/

	const char **keywords = NULL;
	int keywordsAmount = SCRIPT_KEYWORDS_AMOUNT;
	VariableHashType *globalVariableHash = NULL;

	bool dont_convert_paramgetvariable = false;
	std::vector<std::string> script_paramVars;

	bool dont_add_externcallpushpop = false;

	std::string script_currently_parsing_sub = "";

	bool parse_script_params(const std::string &paramLine, std::vector<std::string> &result)
	{
		assert(result.empty());

		bool ok = true;

		std::string paramLineTrimmed = StringRemoveWhitespace(paramLine);

		std::vector<std::string> tmp = StringSplit(",", paramLineTrimmed);
		for (int i = 0; i < (int)tmp.size(); i++)
		{
			std::string trimmed = StringRemoveWhitespace(tmp[i]);
			if (trimmed.empty())
			{
				ok = false;
			} else {
				result.push_back(trimmed);
			}
		}

		return ok;
	}


	#define ARRAY_INDEX_VALUE_NONE -7777777

	class VarOptimizeData
	{
	public:
		VarOptimizeData(int hashCode, int arrayIndexValue, int arrayIndexVariableHC, int arrayIndexVariableLocalIP, int arrayIndexRegister)
		{
			this->hashCode = hashCode;
			this->arrayIndexValue = arrayIndexValue;
			this->arrayIndexVariableHC = arrayIndexVariableHC;
			this->arrayIndexVariableLocalIP = arrayIndexVariableLocalIP;
			this->arrayIndexRegister = arrayIndexRegister;
		}

		int hashCode;
		int arrayIndexValue;
		int arrayIndexVariableHC;
		int arrayIndexVariableLocalIP;
		int arrayIndexRegister;
	};

	/*
	namespace {
		struct ScriptResourceReleaser
		{
			ScriptResourceReleaser()
			{
			}

			~ScriptResourceReleaser()
			{
				{
					for(VariableHashType::iterator it = globalVariableHash->begin(); it != globalVariableHash->end(); ++it)
					{
						delete[] it->second.name;
					}

					delete globalVariableHash;
					globalVariableHash = 0;
				}

				{
					delete[] keywords;
					keywords = 0;
				}
			}
		};

		ScriptResourceReleaser releaser;
	}
	*/

	Script::Script() :
		noForcedPause(false)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (keywords == NULL)
		{
			keywords = new const char *[keywordsAmount];
			for (int i = 0; i < keywordsAmount; i++)
			{
				keywords[i] = NULL;
			}
			keywords[SCRIPT_CMD_NOP] = "noOperation";
			keywords[SCRIPT_CMD_GOTO] = "goto";
			keywords[SCRIPT_CMD_LABEL] = "label";
			keywords[SCRIPT_CMD_CALL] = "call";
			keywords[SCRIPT_CMD_SUB] = "sub";
			keywords[SCRIPT_CMD_ENDSUB] = "endSub";
			keywords[SCRIPT_CMD_IF] = "if";
			keywords[SCRIPT_CMD_THEN] = "then";//
			keywords[SCRIPT_CMD_ELSE] = "else";
			keywords[SCRIPT_CMD_ENDIF] = "endif";
			keywords[SCRIPT_CMD_SELECT] = "select";
			keywords[SCRIPT_CMD_ENDSELECT] = "endSelect";
			keywords[SCRIPT_CMD_CASE] = "case";
			keywords[SCRIPT_CMD_DEFAULT] = "default";
			keywords[SCRIPT_CMD_NULL] = "null";
			keywords[SCRIPT_CMD_EXTERNCALL] = "externInclude";	// "externCall"
			keywords[SCRIPT_CMD_EXTERNCALLPUSH] = "_externCallPush";
			keywords[SCRIPT_CMD_EXTERNCALLPOP] = "_externCallPop";
			keywords[SCRIPT_CMD_RETURN] = "return";
			keywords[SCRIPT_CMD_EXTERNCALLRETURN] = "_externCallReturn";
			keywords[SCRIPT_CMD_PUSHVALUE] = "pushValue";
			keywords[SCRIPT_CMD_POPVALUE] = "popValue";
			keywords[SCRIPT_CMD_GLOBAL] = "global";
			keywords[SCRIPT_CMD_LOCAL] = "local";
			keywords[SCRIPT_CMD_SETVARIABLE] = "setVariable";
			keywords[SCRIPT_CMD_GETVARIABLE] = "getVariable";
			keywords[SCRIPT_CMD_SHAREDLOCAL] = "sharedLocal";
			keywords[SCRIPT_CMD_SETSECONDARY] = "setSecondary";
			keywords[SCRIPT_CMD_GETSECONDARY] = "getSecondary";
			keywords[SCRIPT_CMD_ADDSECONDARY] = "addSecondary";
			keywords[SCRIPT_CMD_DECREASESECONDARY] = "decreaseSecondary";
			keywords[SCRIPT_CMD_MULTIPLYSECONDARY] = "multiplySecondary";
			keywords[SCRIPT_CMD_DIVIDESECONDARY] = "divideSecondary";
			keywords[SCRIPT_CMD_LOOP] = "loop";
			keywords[SCRIPT_CMD_ENDLOOP] = "endLoop";
			keywords[SCRIPT_CMD_BREAKLOOP] = "breakLoop";
			keywords[SCRIPT_CMD_RESTARTLOOP] = "restartLoop";
			keywords[SCRIPT_CMD_SUBEXISTS] = "subExists";
			keywords[SCRIPT_CMD_CALLIFEXISTS] = "callIfExists";
			keywords[SCRIPT_CMD_ADDVARIABLETOVALUE] = "addVariableToValue";
			keywords[SCRIPT_CMD_DECREASEVARIABLEFROMVALUE] = "decreaseVariableFromValue";
			keywords[SCRIPT_CMD_ADDVALUETOVARIABLE] = "addValueToVariable";
			keywords[SCRIPT_CMD_DECREASEVALUEFROMVARIABLE] = "decreaseValueFromVariable";
			keywords[SCRIPT_CMD_EXTERNCALLRETURNMULTIPLE] = "_externCallReturnMultiple";
			keywords[SCRIPT_CMD_PERMANENTGLOBAL] = "permanentGlobal";
			keywords[SCRIPT_CMD_SETARRAYVARIABLEVALUES] = "setArrayVariableValues";
			keywords[SCRIPT_CMD_RETURNMULTIPLE] = "_returnMultiple";
			keywords[SCRIPT_CMD_CASES] = "cases";
			keywords[SCRIPT_CMD_UPTOCASE] = "_upToCase";
			keywords[SCRIPT_CMD_PUSHCALLSTACK_INT] = "_pushCallStack_int";
			keywords[SCRIPT_CMD_POPCALLSTACK_INT] = "_popCallStack_int";
			keywords[SCRIPT_CMD_PUSHCALLSTACK_MARKER] = "_pushCallStack_marker";
			keywords[SCRIPT_CMD_POPCALLSTACK_MARKER] = "_popCallStack_marker";
			keywords[SCRIPT_CMD_PARAMGETVARIABLE] = "_paramGetVariable";
			keywords[SCRIPT_CMD_VARIABLEEQUALSVALUE] = "variableEqualsValue";
		}
		name = NULL;

		processorKeywordsAmount = 0;
		processorKeywords = NULL;
		processorDatatypes = NULL;

		processor = NULL;

		allocedSize = SCRIPT_ALLOC_ARRAY_SIZE;
		commandArray = new int[SCRIPT_ALLOC_ARRAY_SIZE];
		intDataArray = new floatint[SCRIPT_ALLOC_ARRAY_SIZE];
		stringDataArray = new char *[SCRIPT_ALLOC_ARRAY_SIZE];
		varOptimizeDataArray = new VarOptimizeData *[SCRIPT_ALLOC_ARRAY_SIZE];
		for (int i = 0; i < SCRIPT_ALLOC_ARRAY_SIZE; i++)
		{ 
			commandArray[i] = SCRIPT_CMD_NOP;
			intDataArray[i].i = 0;
			stringDataArray[i] = NULL;
			varOptimizeDataArray[i] = NULL;
		}
		commandAmount = 0;

		subIPHash = new SubIPHashType();
		variableIPHash = new VariableIPHashType();

		// "null" data, references (jumps) to this are always invalid.
		addCommand("null", NULL);

		loopNumBuf = new LinkedList();
	}

	Script::~Script()
	{
		while (!loopNumBuf->isEmpty())
		{
			loopNumBuf->popLast();
		}
		delete loopNumBuf;

		for (int i = 0; i < allocedSize; i++)
		{ 
			if (stringDataArray[i] != NULL)
			{
				delete [] stringDataArray[i];
			}
			if (varOptimizeDataArray[i] != NULL)
			{
				delete varOptimizeDataArray[i];
			}
		}
		delete [] varOptimizeDataArray;
		delete [] stringDataArray;
		delete [] intDataArray;
		delete [] commandArray;

		subIPHash->clear();
		variableIPHash->clear();
		delete subIPHash;
		delete variableIPHash;

		if (name != NULL)
		{
			delete [] name;
		}

		// TODO...
	}


	void Script::setNoForcedPause(bool noForcedPause)
	{
		this->noForcedPause = noForcedPause;
	}


	void Script::optimizeArrayVariableData(ScriptProcess *sp, int ip, const char *s, int arrIdxStart, int arrIdxEnd, bool arrIsValueIndex, bool arrIsRegIndex, int *varHashCode)
	{
		int arridx = ARRAY_INDEX_VALUE_NONE;
		int arrreg = 0;
		int arrvarhc = 0;
		int arrvarip = 0;

		char tmp[256 + 1];
		strcpy(tmp, s);
		tmp[arrIdxStart] = '\0';
		if (arrIdxEnd != 0)
			tmp[arrIdxEnd] = '\0';

		if (arrIsRegIndex)
		{
			arrreg = 1;
		} 
		else if (arrIsValueIndex)
		{
			arridx = str2int(&tmp[arrIdxStart + 1]);
			if (arridx < 0)
			{
				sp->error("Script::optimizeArrayVariableData - Negative constant array index out of bounds.");
			}
		} else {
			const char *varStr = &tmp[arrIdxStart + 1];
			SCRIPT_HASHCODE_CALC(varStr, &arrvarhc);

			// local variable...? (todo: optimize, some proper hash, or such)
			int stackNegator = 0;
			int localSeek = ip;
			while (localSeek > 0)
			{
				if (commandArray[localSeek] == SCRIPT_CMD_LOCAL)
				{
					stackNegator++;
					// that stringData contains the type too... int, ...
					//if (stringDataArray[localSeek] != NULL
					//	&& strcmp(stringDataArray[localSeek], data) == 0)
					// so using hashcode in intData instead...
					if (arrvarhc == intDataArray[localSeek].i)
					{
						arrvarip = (stackNegator | SCRIPT_LOCAL_VAR_BITMASK); 
						break;
					}
				}
				else if (commandArray[localSeek] == SCRIPT_CMD_SUB)
				{
					break;
				}
				localSeek--;
			}

			if (arrvarip == 0)
			{
				// shared local variable...?
				VariableIPHashType::iterator iter = variableIPHash->find(arrvarhc);
				if (iter != variableIPHash->end())
				{
					arrvarip = (*iter).second; 
					// don't zero this, it is still used to mark a variable type index, even when local...
					//arrvarhc = 0;
				}
			}
		}

		SCRIPT_HASHCODE_CALC(tmp, varHashCode);

		if (arrIdxEnd == 0)
		{
			varOptimizeDataArray[ip] = new VarOptimizeData(*varHashCode, ARRAY_INDEX_VALUE_NONE, 0, 0, 0);
		} else {
			varOptimizeDataArray[ip] = new VarOptimizeData(*varHashCode, arridx, arrvarhc, arrvarip, arrreg);
		}
	}



	char *Script::matchSuitableCommands(int *amount, const char *command,
		int *smallestMatchLength)
	{
		int matches = 0;
		char *ret = NULL;
		int cmdlen = strlen(command);
		const char *firstMatchingStr = NULL;

		int smallest = 999;

		// HACK: HACK HACK!!!
		// FIXME: bad bad dependency and hack to internal data...
		ScriptManager *scrman = ScriptManager::getInstance();

		for (int i = 0; i < keywordsAmount + processorKeywordsAmount + (int)scrman->internalMacros.size(); i++)
		{
			const char *cmpval;
			int cmdtype = SCRIPT_DATATYPE_NONE;
			if (i < keywordsAmount)
			{
				cmpval = keywords[i];
			} else {
				if (i < keywordsAmount + processorKeywordsAmount)
				{
					cmpval = processorKeywords[i - keywordsAmount];
					cmdtype = processorDatatypes[i - keywordsAmount];
				} else {
					// HACK: hack hack...
					cmpval = scrman->internalMacros[i - (keywordsAmount + processorKeywordsAmount)].first.c_str();
				}
			}

			int strcmpval;
#ifdef _MSC_VER
			strcmpval = _strnicmp(cmpval, command, cmdlen);
#else
			strcmpval = strncmp(cmpval, command, cmdlen);
#endif
			// ignore ones starting with two underscores
			if (strncmp(cmpval, "__", 2) == 0)
			{
				strcmpval = -1;
			}
			if (strcmpval == 0)
			{
				if (firstMatchingStr == NULL)
					firstMatchingStr = cmpval;
				
				// solve the max. length of match fill..
				int cmpvallen = strlen(cmpval);
				int firstlen = strlen(firstMatchingStr);
				int cmptolen = cmpvallen;
				if (firstlen < cmptolen)
					cmptolen = firstlen;
				if (smallest < cmptolen)
					cmptolen = smallest;
				smallest = cmptolen;

				for (int sm = 0; sm < cmptolen; sm++)
				{
					if (firstMatchingStr[sm] != cmpval[sm])
					{
						smallest = sm;
						break;
					}
				}
				// ...now smallest should have the length on smallest match
				// fillup so far.

				matches++;
				if (ret == NULL)
				{
					ret = new char[strlen(cmpval) + 2];
					strcpy(ret, cmpval);
					if (cmdtype != SCRIPT_DATATYPE_NONE)
						strcat(ret, " ");
				} else {
					char *oldret = ret;
					ret = new char[strlen(ret) + strlen(cmpval) + 2];
					strcpy(ret, oldret);
					strcat(ret, "\n");
					strcat(ret, cmpval);
					delete [] oldret;
				}
			}
		}

		if (matches == 0 || firstMatchingStr == NULL)
		{
			smallest = 0;
		}

		if (matches == 1)
		{
			smallest = strlen(ret);
		}

		*amount = matches;
		*smallestMatchLength = smallest;
		return ret;
	}


	char *Script::getName()
	{
		return name;
	}

	ScriptProcess *Script::createNewProcess(const char *startSub)
	{
#ifdef _DEBUG
#ifdef FROZENBYTE_DEBUG_MEMORY
		char buf[256 + 30];
		if (name != NULL && startSub != NULL
			&& strlen(startSub) + strlen(name) < 256)
		{
			strcpy(buf, "scriptprocess ");
			strcat(buf, name);
			strcat(buf, ":");
			strcat(buf, startSub);
			frozenbyte::debug::debugSetAllocationInfo(buf);
		}
#endif
#endif
		ScriptProcess *sp = new ScriptProcess();
		sp->script = this;
		sp->ip = getSubIP(startSub);
		if (sp->ip == -1) 
		{
			sp->ip = 0;
			Logger::getInstance()->error("Script::createNewProcess - Start sub was not found.");
		}
#ifdef DEBUG_CHECK_FOR_UNINITIALIZED_SCRIPT_VALUE_USE
		sp->lastValue.clearInitializedFlag();
		sp->secondaryValue.clearInitializedFlag();
#endif
		return sp;
	}

	void Script::run(ScriptProcess *sp, bool pausable)
	{
		assert(sp->script == this);

#ifdef SCRIPT_TIMING_DEBUG
    Timer::update();
    int currentTime = Timer::getTime();
#endif
#ifdef DEBUG_CHECK_FOR_UNINITIALIZED_SCRIPT_VALUE_USE
		::checked_int_value_sp = sp;
#endif

		int failCount = 0;
		int ip = sp->ip;
		bool doPause = false;
		while (!sp->finished && (!pausable || !doPause))
		{
			if (ip >= commandAmount)
			{
				Logger::getInstance()->error("Script::run - Instruction pointer past end of script.");
				sp->finished = true;
			}

			if (commandArray[ip] < keywordsAmount)
			{

				if (commandArray[ip] == SCRIPT_CMD_EXTERNCALLRETURN
					|| commandArray[ip] == SCRIPT_CMD_EXTERNCALLRETURNMULTIPLE)
				{
					bool multi = false;
					if (commandArray[ip] == SCRIPT_CMD_EXTERNCALLRETURNMULTIPLE)
						multi = true;

					int depth = 1;
					if (multi)
					{
						depth = (intDataArray[ip].i >> (32-4));
						if (depth == 0)
						{
							if (stringDataArray[ip] != NULL)
							{
								depth = str2int(stringDataArray[ip])>>(32-4);
								intDataArray[ip].i |= (depth<<(32-4));
							} else {
								sp->error("Script::run - Internal error, _externalCallReturnMultiple parameter missing.");
								sp->finished = true;
							}
						}
						assert(depth >= 2 && depth <= 15);
					}
					int multipop = depth;

					int retIp = ip;

					if (multi)
					{
						for (int i = 1; i < multipop; i++)
						{
							if (sp->ipStack->isEmpty())
							{
								sp->error("Script::run - Call stack empty, internal error while executing _externCallReturnMultiple.");
								sp->finished = true;
							} else {
								// WARNING: void * -> int casts
								if ((intptr_t)sp->ipStack->popLast())
									sp->thenBranch = true;
								else
									sp->thenBranch = false;
								assert(!sp->ipStack->isEmpty());
								sp->ifDepth = (intptr_t)sp->ipStack->popLast();
								assert(!sp->ipStack->isEmpty());
								intptr_t valueCheck = (intptr_t)sp->ipStack->popLast();
								if (valueCheck != -1)
								{
									sp->error("Script::run - Wrong type of stack data, internal error while executing _externCallReturnMultiple.");
									sp->finished = true;
								}
							}
						}
					}

					if ((intDataArray[retIp].i & (0x8fffffff>>4)) != 0)
					{
						ip = intDataArray[retIp].i & (0x8fffffff>>4);
						assert(ip < commandAmount);
						assert(commandArray[ip] == SCRIPT_CMD_EXTERNCALLPOP);
						sp->ifDepth = 0;
					} else {
						while (ip < commandAmount)
						{
							if (commandArray[ip] == SCRIPT_CMD_EXTERNCALLPUSH)
								depth++;
							else if (commandArray[ip] == SCRIPT_CMD_EXTERNCALLPOP)
							{
								if (depth == 1)
								{
#ifndef NDEBUG
									int fooip = ip;
#endif
									if ((intDataArray[retIp].i & (0x8fffffff>>4)) == 0)
										intDataArray[retIp].i |= ip;

									ip = intDataArray[retIp].i & (0x8fffffff>>4);
									assert(ip == fooip);
									assert(commandArray[ip] == SCRIPT_CMD_EXTERNCALLPOP);

									sp->ifDepth = 0;
									break;
								}
								depth--;
							}
							ip++;
						}
					}
					if (ip >= commandAmount)
					{
						if (multi)
							sp->error("Script::run - Internal error, invalid _externalCallReturnMultiple.");
						else
							sp->error("Script::run - Internal error, invalid _externalCallReturn.");
						sp->finished = true;
					}
				}

				switch (commandArray[ip])
				{
				case SCRIPT_CMD_NULL:
					sp->error("Script::run - Encountered null instruction.");
					sp->finished = true;
					break;

				case SCRIPT_CMD_PUSHVALUE:
#ifdef DEBUG_CHECK_FOR_UNINITIALIZED_SCRIPT_VALUE_USE
					// NOTE: special case, never treat values pushed to stack as unitialized.
					// (as it is likely that someone want's to preserve current value by pushing it to stack
					// without any knowledge of the validity of such value)
					if (!sp->lastValue.wasInitialized())
					{
						// (this clears the flag)
						//sp->lastValue = 0;
						// HACK: use some magic value instead of 0 to indicate it is uninitialized...
						sp->lastValue = UNITIALIZED_MAGIC_VALUE;
					}
#endif
					// WARNING: int -> void * casts
					sp->userStack->append((void *)sp->lastValue);
					sp->userStackSize++;

					if (sp->userStackSize == 50)
					{
						static bool stackSizeWarningLogged = false;
						if (!stackSizeWarningLogged)
						{
							stackSizeWarningLogged = true;
							sp->warning("Script::run - User stack size seems high, possibly indicating an error (further warnings of this type omitted).");
						}
					}
					if (sp->userStackSize > USER_STACK_SIZE_LIMIT)
					{
						sp->error("Script::run - User stack size limit reached (assuming the script is buggy and terminating it for fail safety.)");
						sp->finished = true;
					}
					break;

				case SCRIPT_CMD_POPVALUE:
					if (!sp->userStack->isEmpty())
					{
						// WARNING: void * -> int casts
						sp->lastValue = (intptr_t)sp->userStack->popLast();
						sp->userStackSize--;
#ifdef DEBUG_CHECK_FOR_UNINITIALIZED_SCRIPT_VALUE_USE
						// HACK: check some magic value instead of 0 as indication of uninitialized...
						if (sp->lastValue == UNITIALIZED_MAGIC_VALUE)
						{
							sp->lastValue = 0;
							sp->lastValue.clearInitializedFlag();
						}
#endif
					} else {
						sp->error("Script::run - Attempt to popValue on empty stack.");
						sp->finished = true;
					}
					break;

				case SCRIPT_CMD_ENDSUB:
				case SCRIPT_CMD_RETURN:
				case SCRIPT_CMD_RETURNMULTIPLE:
					if (sp->ifDepth != 0
						&& commandArray[ip] != SCRIPT_CMD_RETURN
						&& commandArray[ip] != SCRIPT_CMD_RETURNMULTIPLE)
					{
						sp->error("Script::run - Missing endif after if command.");
						sp->finished = true;
					} else {
						if (sp->ipStack->isEmpty())
						{
							//Logger::getInstance()->debug("Script::run - Call stack empty, script process finished.");
							sp->finished = true;
						} else {
							if (commandArray[ip] == SCRIPT_CMD_RETURNMULTIPLE)
							{
								// copy&pasted and modified from EXTERNCALLRETURNMULTIPLE stuff...
								int depth = 1;
								depth = (intDataArray[ip].i >> (32-4));
								if (depth == 0)
								{
									if (stringDataArray[ip] != NULL)
									{
										depth = str2int(stringDataArray[ip])>>(32-4);
										intDataArray[ip].i |= (depth<<(32-4));
									} else {
										sp->error("Script::run - Internal error, returnMultiple parameter missing.");
										sp->finished = true;
									}
								}
								assert(depth >= 2 && depth <= 15);
								int multipop = depth;

								for (int i = 1; i < multipop; i++)
								{
									if (sp->ipStack->isEmpty())
									{
										sp->error("Script::run - Call stack empty, internal error while executing returnMultiple.");
										sp->finished = true;
									} else {
										// WARNING: void * -> int casts
										if ((intptr_t)sp->ipStack->popLast())
											sp->thenBranch = true;
										else
											sp->thenBranch = false;
										assert(!sp->ipStack->isEmpty());
										sp->ifDepth = (intptr_t)sp->ipStack->popLast();
										assert(!sp->ipStack->isEmpty());
										intptr_t valueCheck = (intptr_t)sp->ipStack->popLast();
										if (valueCheck != -1)
										{
											sp->error("Script::run - Wrong type of stack data, internal error while executing returnMultiple.");
											sp->finished = true;
										}
										//sp->leaveLocalScope();
									}
								}
								// ...end of copy&paste
							}

							if (sp->ipStack->isEmpty())
							{
								// (note, this error cannot happen unless it was returnMultiple)
								//sp->error("Script::run - Call stack empty, internal error while executing returnMultiple (phase 2).");
								// not actually an error, but an indication of highest level return (exit from highest level script sub)
								sp->finished = true;
							} else {
								// WARNING: void * -> int casts
								if ((intptr_t)sp->ipStack->popLast())
									sp->thenBranch = true;
								else
									sp->thenBranch = false;
								sp->ifDepth = (intptr_t)sp->ipStack->popLast();
								ip = (intptr_t)sp->ipStack->popLast();
								if (ip == -1)
								{
									sp->error("Script::run - Wrong type of stack data, internal error while executing return from sub.");
									sp->finished = true;                
								}
								sp->leaveLocalScope();
							}

							//ip++; // don't redo the call, next instruction please
							// damn, that was a bug, it will skip the next instr. anyway
						}
					}
					break;

				case SCRIPT_CMD_IF:
					if (sp->ifDepth != 0)
					{
						sp->error("Script::run - Another if not allowed inside if block.");
						sp->finished = true;
					} else {
						sp->ifDepth++;
					}
					break;

				case SCRIPT_CMD_THEN:
					if (sp->ifDepth != 1)
					{
						if (sp->ifDepth == 0)
							sp->error("Script::run - Missing if before then command.");
						else
							sp->error("Script::run - Internal error, encountered then command at depth > 1.");
						sp->finished = true;
					} else {
						if (sp->lastValue != 0)
						{
							sp->thenBranch = true;
						} else {

							// new: some optimizations, save the endif ip, if one saved, go there directly
							assert(commandArray[ip] == SCRIPT_CMD_THEN);
							int saveToIp = ip;
							if (intDataArray[ip].i != 0)
							{
								ip = intDataArray[ip].i;
								assert(ip < commandAmount);
								assert(commandArray[ip+1] == SCRIPT_CMD_ENDIF
									|| commandArray[ip+1] == SCRIPT_CMD_ELSE);
								saveToIp = 0;
							}

							while (ip < commandAmount)
							{
								if (commandArray[ip] == SCRIPT_CMD_IF)
									sp->ifDepth++;
								else if (commandArray[ip] == SCRIPT_CMD_ENDIF)
								{
									if (sp->ifDepth == 1) 
									{
										ip--;
										if (saveToIp != 0)
										{
											intDataArray[saveToIp].i = ip;
										}
										break;
									}
									sp->ifDepth--;
								} else {
									if (commandArray[ip] == SCRIPT_CMD_ELSE
										&& sp->ifDepth == 1)
									{
										ip--;
										if (saveToIp != 0)
										{
											intDataArray[saveToIp].i = ip;
										}
										break;
									}
								}
								ip++;
							}
							if (ip >= commandAmount)
							{
								sp->error("Script::run - Expected else or endif after then command.");
								sp->finished = true;
							}
							sp->thenBranch = false;
						}
					}
					break;

				case SCRIPT_CMD_ELSE:
					if (sp->ifDepth != 1)
					{
						if (sp->ifDepth == 0)
							sp->error("Script::run - Missing if before else command.");
						else
							sp->error("Script::run - Internal error, encountered else command at depth > 1.");
						sp->finished = true;
					} else {
						if (sp->thenBranch)
						{
							assert(commandArray[ip] == SCRIPT_CMD_ELSE);
							int saveToIp = ip;
							if (intDataArray[ip].i != 0)
							{
								ip = intDataArray[ip].i;
								assert(ip < commandAmount);
								assert(commandArray[ip+1] == SCRIPT_CMD_ENDIF);
								saveToIp = 0;
							}

							while (ip < commandAmount)
							{
								if (commandArray[ip] == SCRIPT_CMD_IF)
									sp->ifDepth++;
								else if (commandArray[ip] == SCRIPT_CMD_ENDIF)
								{
									if (sp->ifDepth == 1) 
									{
										ip--;
										if (saveToIp != 0)
										{
											intDataArray[saveToIp].i = ip;
										}
										break;
									}
									sp->ifDepth--;
								}
								ip++;
							}
							if (ip >= commandAmount)
							{
								sp->error("Script::run - Expected endif after else command.");
								sp->finished = true;
							}
						}
					}
					break;

				case SCRIPT_CMD_ENDIF:
					if (sp->ifDepth != 1)
					{
						if (sp->ifDepth == 0)
							sp->error("Script::run - Missing if before endif command.");
						else
							sp->error("Script::run - Internal error, encountered endif command at depth > 1.");
						sp->finished = true;
					} else {
						sp->thenBranch = false;
						sp->ifDepth--;
					}
					break;

				case SCRIPT_CMD_CASE:
				case SCRIPT_CMD_DEFAULT:
					/*
					while (ip < commandAmount
						&& commandArray[ip] != SCRIPT_CMD_ENDSELECT)
					{
						ip++;
					}
					if (ip >= commandAmount)
					{
						Logger::getInstance()->error("Script::run - Expected endSelect after case of default command.");
						sp->finished = true;
					}
					*/
					{
						int selectDepth = 1;
						int startIp = ip;

						while (ip < commandAmount)
						{
							if (commandArray[ip] == SCRIPT_CMD_SELECT)
							{
								selectDepth++;
							}
							else if (commandArray[ip] == SCRIPT_CMD_ENDSELECT)
							{
								if (selectDepth > 0) 
								{
									// nop
								} else {
									sp->error("Script::run - Internal error while processing select block.");
									sp->finished = true;
									break;
								}
							}

							if (selectDepth > 1)
							{
								if (commandArray[ip] == SCRIPT_CMD_ENDSELECT)
								{
									selectDepth--;
								}
								ip++;
							} else {
								if (commandArray[ip] != SCRIPT_CMD_ENDSELECT)
								{
									ip++;
									if (commandArray[ip] == SCRIPT_CMD_CASE)
									{
										assert(selectDepth == 1);
										// FIXME: this error message was here earlier,
										// i don't know if it works corretly though???
										/*
										if (intDataArray[ip].i == sp->lastValue)
										{
											sp->error("Script::run - Multiple matching select cases.");
											//ip++;
											// must not increment here, done at the end of the loop
											//break;
										}
										*/
									}
								} else {
									assert(selectDepth == 1);
									break;
								}
							}
						}
						if (ip >= commandAmount)
						{
							sp->error("Script::run - Expected endSelect after case of default command.");
							sp->finished = true;
						}
						startIp = startIp;
					}
					break;

				case SCRIPT_CMD_SELECT:
					{
						int selectDepth = 0;

						while (ip < commandAmount)
						{
							if (commandArray[ip] == SCRIPT_CMD_SELECT)
							{
								selectDepth++;
							}
							else if (commandArray[ip] == SCRIPT_CMD_ENDSELECT)
							{
								if (selectDepth > 0) 
								{
									if (selectDepth == 1)
									{
										sp->warning("Script::run - Select block end was reached, matched none of the cases and no default.");
									}
								} else {
									sp->error("Script::run - Internal error while processing select block.");
									sp->finished = true;
									break;
								}
							}

							if (selectDepth > 1)
							{
								if (commandArray[ip] == SCRIPT_CMD_ENDSELECT)
								{
									selectDepth--;
								}
								ip++;
							} else {
								if (commandArray[ip] != SCRIPT_CMD_DEFAULT
									&& commandArray[ip] != SCRIPT_CMD_ENDSELECT)
								{
									ip++;
									if (commandArray[ip] == SCRIPT_CMD_CASE)
									{
										assert(selectDepth == 1);
										if (intDataArray[ip].i == sp->lastValue
											&& (ip < commandAmount - 1 && commandArray[ip + 1] != SCRIPT_CMD_UPTOCASE))
										{
											//ip++;
											// must not increment here, done at the end of the loop
											break;
										}
									}
									else if (commandArray[ip] == SCRIPT_CMD_UPTOCASE)
									{
										assert(selectDepth == 1);
										if (intDataArray[ip].i >= sp->lastValue
											&& ip > 0 && commandArray[ip - 1] == SCRIPT_CMD_CASE
											&& intDataArray[ip - 1].i <= sp->lastValue)
										{
											//ip++;
											// must not increment here, done at the end of the loop
											break;
										}
									}
								} else {
									assert(selectDepth == 1);
									break;
								}
							}
						}
						if (ip >= commandAmount)
						{
							sp->error("Script::run - Expected endSelect after select command.");
							sp->finished = true;
						}
					}
					break;

				case SCRIPT_CMD_GOTO:
					sp->ifDepth = 0;
					if (intDataArray[ip].i != 0)
					{
						ip = intDataArray[ip].i;
					} else {
						if (stringDataArray[ip] != NULL)
						{
							ip = getLabelIP(stringDataArray[ip]);
							if (ip == -1) 
							{
								sp->error("Script::run - Goto target label not found.");
								sp->finished = true;
							}
						} else {
							sp->error("Script::run - Goto has not target label name.");
							sp->finished = true;
						}
					}
					break;

				case SCRIPT_CMD_EXTERNCALLPUSH:
					// WARNING: nasty casts here
					//sp->ipStack->append((void *)ip);
					sp->ipStack->append((void *)-1);
					sp->ipStack->append((void *)sp->ifDepth);
					if (sp->thenBranch)
						sp->ipStack->append((void *)1);
					else
						sp->ipStack->append((void *)0);
					sp->ifDepth = 0;
					sp->thenBranch = false;
					// don't do this here. _externCallPush/Pop should not affect scope
					//sp->enterLocalScope();
					break;

				case SCRIPT_CMD_EXTERNCALLPOP:
					if (sp->ifDepth != 0)
					{
						sp->error("Script::run - Missing endif after if command (inside externCall).");
						sp->finished = true;
					} else {
						if (sp->ipStack->isEmpty())
						{
							sp->error("Script::run - Call stack empty, internal error while executing _externCallPop.");
							sp->finished = true;
						} else {
							// WARNING: void * -> int casts
							if ((intptr_t)sp->ipStack->popLast())
								sp->thenBranch = true;
							else
								sp->thenBranch = false;
							sp->ifDepth = (intptr_t)sp->ipStack->popLast();
							int valueCheck = (intptr_t)sp->ipStack->popLast();
              if (valueCheck != -1)
              {
							  sp->error("Script::run - Wrong type of stack data, internal error while executing _externCallPop.");
							  sp->finished = true;                
              }
							// don't do this here. _externCallPush/Pop should not affect scope
							//sp->leaveLocalScope();
						}
					}
					break;

				case SCRIPT_CMD_CALL:
				case SCRIPT_CMD_CALLIFEXISTS:
					{
						bool makeCall = true;
						if (commandArray[ip] == SCRIPT_CMD_CALLIFEXISTS)
						{
							if (stringDataArray[ip] != NULL)
							{
								// TODO: optimize, could use this tmp value
								// for the actual call value too, would save one
								// unnecessary getSubIP call.
								int tmp = getSubIP(stringDataArray[ip]);
								if (tmp == -1)
								{
									makeCall = false;
								}
							} else {
								makeCall = false;
								sp->error("Script::run - Conditional call (callIfExists) has not target sub name.");
								sp->finished = true;
							}
						}

						if (makeCall)
						{
							// WARNING: int -> void * casts
							sp->ipStack->append((void *)ip);
							sp->ipStack->append((void *)sp->ifDepth);
							if (sp->thenBranch)
								sp->ipStack->append((void *)1);
							else
								sp->ipStack->append((void *)0);
							sp->ifDepth = 0;
							sp->thenBranch = false;

							sp->enterLocalScope();

							if (intDataArray[ip].i != 0)
							{
								ip = intDataArray[ip].i;
							} else {
								if (stringDataArray[ip] != NULL)
								{
									ip = getSubIP(stringDataArray[ip]);
									if (ip == -1) 
									{
										sp->error("Script::run - Call target sub not found.");
										sp->finished = true;
									}
								} else {
									sp->error("Script::run - Call has not target sub name.");
									sp->finished = true;
								}
							}
						}
					}
					break;

				case SCRIPT_CMD_LOCAL:
					// TODO: only if first time running this command!!!
					// or assert that command is in the beginning of a sub
					sp->pushLocalVarStack(0);
					break;

				case SCRIPT_CMD_SETVARIABLE:
				case SCRIPT_CMD_GETVARIABLE:
				case SCRIPT_CMD_ADDVARIABLETOVALUE:
				case SCRIPT_CMD_DECREASEVARIABLEFROMVALUE:
				case SCRIPT_CMD_ADDVALUETOVARIABLE:
				case SCRIPT_CMD_DECREASEVALUEFROMVARIABLE:
				case SCRIPT_CMD_VARIABLEEQUALSVALUE:
					if (intDataArray[ip].i != 0)
					{
						if (intDataArray[ip].i & SCRIPT_LOCAL_VAR_BITMASK)
						{
							int negator = (intDataArray[ip].i ^ SCRIPT_LOCAL_VAR_BITMASK);
							int localVarPos = sp->getLocalVarStackSize() - negator;
							if (localVarPos >= 0 && localVarPos < sp->getLocalVarStackSize())
							{
								if (commandArray[ip] == SCRIPT_CMD_SETVARIABLE)
									sp->setLocalVarStackEntryValue(localVarPos, sp->lastValue);
								else if (commandArray[ip] == SCRIPT_CMD_ADDVALUETOVARIABLE)
									sp->setLocalVarStackEntryValue(localVarPos, sp->getLocalVarStackEntryValue(localVarPos) + sp->lastValue);
								else if (commandArray[ip] == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE)
									sp->setLocalVarStackEntryValue(localVarPos, sp->getLocalVarStackEntryValue(localVarPos) - sp->lastValue);
								else if (commandArray[ip] == SCRIPT_CMD_ADDVARIABLETOVALUE)
									sp->lastValue += sp->getLocalVarStackEntryValue(localVarPos);
								else if (commandArray[ip] == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE)
									sp->lastValue -= sp->getLocalVarStackEntryValue(localVarPos);
								else if (commandArray[ip] == SCRIPT_CMD_VARIABLEEQUALSVALUE)
									sp->lastValue = ((sp->lastValue == sp->getLocalVarStackEntryValue(localVarPos)) ? 1 : 0);
								else
									sp->lastValue = sp->getLocalVarStackEntryValue(localVarPos);
							} else {
								sp->error("Script::run - setVariable or getVariable or variant, local variable stack size smaller than it should be (internal error).");
							}
						} else {
							if (commandArray[ip] == SCRIPT_CMD_SETVARIABLE)
								intDataArray[intDataArray[ip].i].i = sp->lastValue;
							else if (commandArray[ip] == SCRIPT_CMD_ADDVALUETOVARIABLE)
								intDataArray[intDataArray[ip].i].i += sp->lastValue;
							else if (commandArray[ip] == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE)
								intDataArray[intDataArray[ip].i].i -= sp->lastValue;
							else if (commandArray[ip] == SCRIPT_CMD_ADDVARIABLETOVALUE)
								sp->lastValue += intDataArray[intDataArray[ip].i].i;
							else if (commandArray[ip] == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE)
								sp->lastValue -= intDataArray[intDataArray[ip].i].i;
							else if (commandArray[ip] == SCRIPT_CMD_VARIABLEEQUALSVALUE)
								sp->lastValue = ((sp->lastValue == intDataArray[intDataArray[ip].i].i) ? 1 : 0);
							else
								sp->lastValue = intDataArray[intDataArray[ip].i].i;
						}
					} else {
						if (stringDataArray[ip] != NULL)
						{
							int hc = 0;
							
							if (varOptimizeDataArray[ip] != NULL)
							{
								hc = varOptimizeDataArray[ip]->hashCode;
							} else {
								bool isArr = false;
								bool arrIsValueIndex = false;
								bool arrIsRegIndex = false;
								int arrIdxStart = 0;
								int arrIdxEnd = 0;

								char *s = stringDataArray[ip];
								int slen = strlen(stringDataArray[ip]);
								for (int chi = 1; chi < slen; chi++)
								{
									if (s[chi] == '[')
									{
										if (arrIdxStart != 0)
										{
											sp->error("Script::run - setVariable or getVariable or variant, variable name / array reference syntax error.");
										}
										isArr = true;
										arrIdxStart = chi;
										if (s[chi + 1] >= '0' && s[chi + 1] <= '9')
										{
											arrIsValueIndex = true;
										}
										else if ((s[chi + 1] >= 'A' && s[chi + 1] <= 'Z')
											|| (s[chi + 1] >= 'a' && s[chi + 1] <= 'z')
											|| s[chi + 1] == '_')
										{
											// variable
										}
										else if (s[chi + 1] == ']')
										{
											arrIsRegIndex = true;
										} else {
											sp->error("Script::run - setVariable or getVariable or variant, variable name / array reference syntax error.");
										}
									}
									if (s[chi] == ']')
									{
										if (arrIdxEnd != 0
											|| !isArr)
										{
											sp->error("Script::run - setVariable or getVariable or variant, variable name / array reference syntax error.");
										}
										arrIdxEnd = chi;
										if (s[chi - 1] == ' ')
											arrIdxEnd--;
									}
								}

								if (slen >= 256 && isArr)
								{
									sp->error("Script::run - setVariable or getVariable or variant, variable name too long.");
								}

								if (isArr && arrIdxStart != 0 && arrIdxEnd != 0
									&& slen < 256)
								{
									this->optimizeArrayVariableData(sp, ip, s, arrIdxStart, arrIdxEnd, arrIsValueIndex, arrIsRegIndex, &hc);
								} else {
									SCRIPT_HASHCODE_CALC(stringDataArray[ip], &hc);
									varOptimizeDataArray[ip] = new VarOptimizeData(hc, ARRAY_INDEX_VALUE_NONE, 0, 0, 0);
								}
							}

							VariableHashType::iterator iter = globalVariableHash->find(hc);
							if (iter != globalVariableHash->end())
							{
								if ((*iter).second.dataType == SCRIPT_DATATYPE_INT)
								{
									if (varOptimizeDataArray[ip] != NULL
										&& (varOptimizeDataArray[ip]->arrayIndexValue != ARRAY_INDEX_VALUE_NONE
										|| varOptimizeDataArray[ip]->arrayIndexVariableHC != 0
										|| varOptimizeDataArray[ip]->arrayIndexRegister != 0))
									{
										sp->error("Script::run - setVariable or getVariable or variant, attempt to use non-array (int) variable with array index.");
									} else {
										if (commandArray[ip] == SCRIPT_CMD_SETVARIABLE)
											(*iter).second.intValue = sp->lastValue; 
										else if (commandArray[ip] == SCRIPT_CMD_ADDVALUETOVARIABLE)
											(*iter).second.intValue += sp->lastValue; 
										else if (commandArray[ip] == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE)
											(*iter).second.intValue -= sp->lastValue; 
										else if (commandArray[ip] == SCRIPT_CMD_ADDVARIABLETOVALUE)
											sp->lastValue += (*iter).second.intValue; 
										else if (commandArray[ip] == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE)
											sp->lastValue -= (*iter).second.intValue; 
										else if (commandArray[ip] == SCRIPT_CMD_VARIABLEEQUALSVALUE)
											sp->lastValue = ((sp->lastValue == (*iter).second.intValue) ? 1 : 0);
										else
											sp->lastValue = (*iter).second.intValue; 
									}
								}
								else if ((*iter).second.dataType == SCRIPT_DATATYPE_ARRAY)
								{
									if (varOptimizeDataArray[ip] != NULL)
									{
										int arrayIndexValue = varOptimizeDataArray[ip]->arrayIndexValue;
										if (arrayIndexValue == ARRAY_INDEX_VALUE_NONE)
										{
											int arrayIndexVariableHC = varOptimizeDataArray[ip]->arrayIndexVariableHC;

											if (arrayIndexVariableHC != 0)
											{
												if (varOptimizeDataArray[ip]->arrayIndexVariableLocalIP != 0)
												{
													if ((varOptimizeDataArray[ip]->arrayIndexVariableLocalIP & SCRIPT_LOCAL_VAR_BITMASK) != 0)
													{
														int negator = (varOptimizeDataArray[ip]->arrayIndexVariableLocalIP ^ SCRIPT_LOCAL_VAR_BITMASK);
														int localVarPos = sp->getLocalVarStackSize() - negator;
														if (localVarPos >= 0 && localVarPos < sp->getLocalVarStackSize())
														{
															arrayIndexValue = sp->getLocalVarStackEntryValue(localVarPos);
														} else {
															int tmp = sp->ip;
															sp->ip = ip;
															sp->error("Script::run - setVariable or getVariable or variant, internal error (reference to invalid local array index variable).");
															sp->ip = tmp;
														}
													} else {
														arrayIndexValue = intDataArray[varOptimizeDataArray[ip]->arrayIndexVariableLocalIP].i;
													}
												} else {
													VariableHashType::iterator iter2 = globalVariableHash->find(arrayIndexVariableHC);
													if (iter2 != globalVariableHash->end())
													{
														if ((*iter2).second.dataType == SCRIPT_DATATYPE_INT)
														{
															arrayIndexValue = (*iter2).second.intValue; 
														} else {
															int tmp = sp->ip;
															sp->ip = ip;
															sp->error("Script::run - setVariable or getVariable or variant, array index variable is not integer type.");
															sp->ip = tmp;
															Logger::getInstance()->debug(stringDataArray[ip]);
														}
													} else {
														int tmp = sp->ip;
														sp->ip = ip;
														sp->error("Script::run - setVariable or getVariable or variant, array index variable not found.");
														sp->ip = tmp;
														Logger::getInstance()->debug(stringDataArray[ip]);
													}
												}
											} else {
												if (varOptimizeDataArray[ip]->arrayIndexRegister != 0)
												{
													arrayIndexValue = sp->lastValue; 
												}
											}
										}

										if (arrayIndexValue == ARRAY_INDEX_VALUE_NONE)
										{
											int tmp = sp->ip;
											sp->ip = ip;
											sp->error("Script::run - setVariable or getVariable or variant, attempt to use array variable without array index.");
											sp->ip = tmp;
										} else {
											if (arrayIndexValue >= 0 && arrayIndexValue < (*iter).second.arraySize)
											{
												if (commandArray[ip] == SCRIPT_CMD_SETVARIABLE)
													(*iter).second.arrayValue[arrayIndexValue] = sp->lastValue; 
												else if (commandArray[ip] == SCRIPT_CMD_ADDVALUETOVARIABLE)
													(*iter).second.arrayValue[arrayIndexValue] += sp->lastValue; 
												else if (commandArray[ip] == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE)
													(*iter).second.arrayValue[arrayIndexValue] -= sp->lastValue; 
												else if (commandArray[ip] == SCRIPT_CMD_ADDVARIABLETOVALUE)
													sp->lastValue += (*iter).second.arrayValue[arrayIndexValue]; 
												else if (commandArray[ip] == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE)
													sp->lastValue -= (*iter).second.arrayValue[arrayIndexValue]; 
												else if (commandArray[ip] == SCRIPT_CMD_VARIABLEEQUALSVALUE)
													sp->lastValue = ((sp->lastValue == (*iter).second.arrayValue[arrayIndexValue]) ? 1 : 0);
												else
													sp->lastValue = (*iter).second.arrayValue[arrayIndexValue]; 
											} else {
												int tmp = sp->ip;
												sp->ip = ip;
												sp->error("Script::run - setVariable or getVariable or variant, array index out of bounds.");
												sp->ip = tmp;
												Logger::getInstance()->debug(stringDataArray[ip]);
												Logger::getInstance()->debug(int2str(arrayIndexValue));
											}
										}
									}
								} else {
									sp->error("Script::run - setVariable or getVariable or variant, attempt to manipulate variable declared non-int, use appropriate specialized command instead (i.e. setStringVariable).");
								}

								break;
							}

							if (commandArray[ip] == SCRIPT_CMD_SETVARIABLE)
								sp->error("Script::run - setVariable, No variable with given name.");
							else if (commandArray[ip] == SCRIPT_CMD_ADDVALUETOVARIABLE)
								sp->error("Script::run - addValueToVariable, No variable with given name.");
							else if (commandArray[ip] == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE)
								sp->error("Script::run - decreaseValueFromVariable, No variable with given name.");
							else if (commandArray[ip] == SCRIPT_CMD_ADDVARIABLETOVALUE)
								sp->error("Script::run - addVariableToValue, No variable with given name.");
							else if (commandArray[ip] == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE)
								sp->error("Script::run - decreaseVariableFromValue, No variable with given name.");
							else if (commandArray[ip] == SCRIPT_CMD_VARIABLEEQUALSVALUE)
								sp->error("Script::run - variableEqualsValue, No variable with given name.");
							else
								sp->error("Script::run - getVariable, No variable with given name.");
							Logger::getInstance()->debug(stringDataArray[ip]);
							//sp->finished = true;
						} else {
							if (commandArray[ip] == SCRIPT_CMD_SETVARIABLE)
								sp->error("Script::run - setVariable parameter missing.");
							else if (commandArray[ip] == SCRIPT_CMD_ADDVALUETOVARIABLE)
								sp->error("Script::run - addValueToVariable parameter missing.");
							else if (commandArray[ip] == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE)
								sp->error("Script::run - decreaseValueFromVariable parameter missing.");
							else if (commandArray[ip] == SCRIPT_CMD_ADDVARIABLETOVALUE)
								sp->error("Script::run - addVariableToValue parameter missing.");
							else if (commandArray[ip] == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE)
								sp->error("Script::run - decreaseVariableFromValue parameter missing.");
							else if (commandArray[ip] == SCRIPT_CMD_VARIABLEEQUALSVALUE)
								sp->error("Script::run - variableEqualsValue parameter missing.");
							else
								sp->error("Script::run - getVariable parameter missing.");
							//sp->finished = true;
						}
					}
					break;

				case SCRIPT_CMD_SETARRAYVARIABLEVALUES:
					if (intDataArray[ip].i != 0)
					{
						//sp->lastValue = 0;
						sp->error("Script::run - setArrayVariableValues, shared local variables not supported by command.");
					} else {
						if (stringDataArray[ip] != NULL)
						{
							int hc = 0;
							
							if (varOptimizeDataArray[ip] != NULL)
							{
								hc = varOptimizeDataArray[ip]->hashCode;
							} else {
								bool isArr = false;
								bool isComma = false;
								bool arrIsValueIndex = false;
								bool arrIsRegIndex = false;
								int arrIdxStart = 0;
								int arrIdxEnd = 0;

								char *s = stringDataArray[ip];
								int slen = strlen(stringDataArray[ip]);
								for (int chi = 1; chi < slen; chi++)
								{
									if (s[chi] == ',')
									{
										isComma = true;
										if (arrIdxStart == 0)
										{
											arrIdxStart = chi;
										}
										break;
									}
									if (s[chi] == '[')
									{
										if (arrIdxStart != 0)
										{
											sp->error("Script::run - setArrayVariableValues, variable name / array reference syntax error.");
										}
										isArr = true;
										arrIdxStart = chi;
										if (s[chi + 1] >= '0' && s[chi + 1] <= '9')
										{
											arrIsValueIndex = true;
										}
										else if ((s[chi + 1] >= 'A' && s[chi + 1] <= 'Z')
											|| (s[chi + 1] >= 'a' && s[chi + 1] <= 'z')
											|| s[chi + 1] == '_')
										{
											// variable
										}
										else if (s[chi + 1] == ']')
										{
											arrIsRegIndex = true;
										} else {
											sp->error("Script::run - setArrayVariableValues, variable name / array reference syntax error.");
										}
									}
									if (s[chi] == ']')
									{
										if (arrIdxEnd != 0
											|| !isArr)
										{
											sp->error("Script::run - setArrayVariableValues, variable name / array reference syntax error.");
										}
										arrIdxEnd = chi;
										if (s[chi - 1] == ' ')
											arrIdxEnd--;
									}
								}

								if (slen >= 256 && (isArr || isComma))
								{
									sp->error("Script::run - setArrayVariableValues, variable name too long.");
								}

								if (((isArr && arrIdxStart != 0 && arrIdxEnd != 0)
									|| (isComma && arrIdxStart != 0))
									&& slen < 256)
								{
									this->optimizeArrayVariableData(sp, ip, s, arrIdxStart, arrIdxEnd, arrIsValueIndex, arrIsRegIndex, &hc);
								} else {
									SCRIPT_HASHCODE_CALC(stringDataArray[ip], &hc);
									varOptimizeDataArray[ip] = new VarOptimizeData(hc, ARRAY_INDEX_VALUE_NONE, 0, 0, 0);
								}
							}

							VariableHashType::iterator iter = globalVariableHash->find(hc);
							if (iter != globalVariableHash->end())
							{
								if ((*iter).second.dataType == SCRIPT_DATATYPE_ARRAY)
								{
									if (varOptimizeDataArray[ip] != NULL)
									{
										int arrayIndexValue = varOptimizeDataArray[ip]->arrayIndexValue;
										if (arrayIndexValue == ARRAY_INDEX_VALUE_NONE)
										{
											int arrayIndexVariableHC = varOptimizeDataArray[ip]->arrayIndexVariableHC;

											if (arrayIndexVariableHC != 0)
											{
												if (varOptimizeDataArray[ip]->arrayIndexVariableLocalIP != 0)
												{
													arrayIndexValue = intDataArray[varOptimizeDataArray[ip]->arrayIndexVariableLocalIP].i;
												} else {
													VariableHashType::iterator iter2 = globalVariableHash->find(arrayIndexVariableHC);
													if (iter2 != globalVariableHash->end())
													{
														if ((*iter2).second.dataType == SCRIPT_DATATYPE_INT)
														{
															arrayIndexValue = (*iter2).second.intValue; 
														} else {
															sp->error("Script::run - setArrayVariableValues, array index variable is not integer type.");
														}
													} else {
														sp->error("Script::run - setArrayVariableValues, array index variable not found.");
													}
												}
											} else {
												if (varOptimizeDataArray[ip]->arrayIndexRegister != 0)
												{
													arrayIndexValue = sp->lastValue; 
												}
											}
										}

										bool nonIndexed = false;
										if (arrayIndexValue == ARRAY_INDEX_VALUE_NONE)
										{
											arrayIndexValue = 0;
											nonIndexed = true;
										}

										int arrsize = (*iter).second.arraySize;
										int elementListSize = -1;
										int *elementListValues = new int[arrsize];
										assert(arrsize > 0);

										int slen = strlen(stringDataArray[ip]);
										char *tmp = new char[slen + 1];
										strcpy(tmp, stringDataArray[ip]);
										bool ltrimming = true;
										int lastValPos = 0;

										for (int si = 0; si < slen + 1; si++)
										{
											if (ltrimming)
											{
												if (tmp[si] != ' ' && tmp[si] != '\t')
												{
													ltrimming = false;
													lastValPos = si;
												}
											}
											if (!ltrimming)
											{
												if (tmp[si] == ',' || tmp[si] == '\0')
												{
													tmp[si] = '\0';
													if (elementListSize >= 0 && elementListSize < arrsize)
													{
														elementListValues[elementListSize] = str2int(&tmp[lastValPos]);
													}
													elementListSize++;
													ltrimming = true;
													lastValPos = si + 1;
												}
											}
										}

										if (elementListSize >= 0)
										{
											if (nonIndexed && arrayIndexValue + elementListSize < arrsize)
											{
												sp->warning("Script::run - setArrayVariableValues, array is longer than amount of given elements, values for the rest of array is not set (if intended, use [0] array index to get rid of this message).");
											}

											if (arrayIndexValue >= 0 && arrayIndexValue + elementListSize <= arrsize)
											{
												for (int eidx = 0; eidx < elementListSize; eidx++)
												{
													(*iter).second.arrayValue[arrayIndexValue + eidx] = elementListValues[eidx]; 
												}
											} else {
												sp->error("Script::run - setArrayVariableValues, array range (start index / start index + element list size) out of bounds.");
												Logger::getInstance()->debug(stringDataArray[ip]);
												Logger::getInstance()->debug(int2str(arrayIndexValue));
											}
										} else {
											sp->error("Script::run - setArrayVariableValues, invalid parameter.");
										}

										delete [] tmp;
										delete [] elementListValues;

									}
								} else {
									sp->error("Script::run - setArrayVariableValues, attempt to manipulate non-array variable.");
								}

								break;
							}

							sp->error("Script::run - setArrayVariableValues, No variable with given name.");
							Logger::getInstance()->debug(stringDataArray[ip]);
							sp->finished = true;
						} else {
							sp->error("Script::run - setArrayVariableValues parameter missing.");
							Logger::getInstance()->debug(stringDataArray[ip]);
							sp->finished = true;
						}
					}
					break;

				case SCRIPT_CMD_SETSECONDARY:
					sp->secondaryValue = sp->lastValue;
					break;

				case SCRIPT_CMD_GETSECONDARY:
					sp->lastValue = sp->secondaryValue;
					break;

				case SCRIPT_CMD_ADDSECONDARY:
					sp->secondaryValue += sp->lastValue;
					break;

				case SCRIPT_CMD_DECREASESECONDARY:
					sp->secondaryValue -= sp->lastValue;
					break;

				case SCRIPT_CMD_MULTIPLYSECONDARY:
					sp->secondaryValue *= sp->lastValue;
					break;

				case SCRIPT_CMD_DIVIDESECONDARY:
					if (sp->lastValue == 0)
					{
						sp->secondaryValue = 0;
						sp->error("Script::process - divideSecondary, division by zero.");
					} else {
						sp->secondaryValue /= sp->lastValue;
					}
					break;

				case SCRIPT_CMD_PUSHCALLSTACK_INT:
					sp->pushCallParamStack(sp->lastValue);
					break;

				case SCRIPT_CMD_POPCALLSTACK_INT:
					if (!sp->isCallParamStackEmpty())
					{
						sp->lastValue = sp->popCallParamStack();
					} else {
						sp->error("Script::process - popCallStack_int, call parameter stack empty (more parameters expected or internal error).");
						sp->finished = true;
					}
					break;

				case SCRIPT_CMD_PUSHCALLSTACK_MARKER:
					sp->pushCallParamStack(0xF00BABE1);
					break;

				case SCRIPT_CMD_POPCALLSTACK_MARKER:
					if (!sp->isCallParamStackEmpty())
					{
						unsigned int tmp = sp->popCallParamStack();
						if (tmp != 0xF00BABE1)
						{
							sp->error("Script::process - popCallStack_marker, bad magic number in call parameter stack (internal error).");
							sp->finished = true;
						}
					} else {
						sp->error("Script::process - popCallStack_marker, call parameter stack empty (internal error).");
						sp->finished = true;
					}
					break;

				case SCRIPT_CMD_SUBEXISTS:
					if (stringDataArray[ip] != NULL)
					{
						int tmp = getSubIP(stringDataArray[ip]);
						if (tmp == -1)
						{
							sp->lastValue = 0;
						} else {
							sp->lastValue = 1;
						}
					} else {
						sp->error("Script::run - subExists parameter missing, sub name expected.");
						sp->finished = true;
					}
					break;

				case SCRIPT_CMD_CASES:
					// all cases commands should have been converted to case + _upToCase pair at parse time
					sp->error("Script::run - Instruction pointer reached invalid cases command (cases parameter was bad?).");
					sp->finished = true;
					break;

				case SCRIPT_CMD_UPTOCASE:
					// _upToCase should never be directly reached?
					sp->error("Script::run - Instruction pointer reached _upToCase command (internal error).");
					sp->finished = true;
					break;

				//default:
					// do nothing?
				}

				ip++;
			} else {
				if (processor != NULL)
				{
					sp->ip = ip;
					doPause = processor->process(sp, commandArray[ip] - keywordsAmount, 
						intDataArray[ip], stringDataArray[ip], &sp->lastValue);
				} else {
					Logger::getInstance()->error("Script::run - Encountered external command, but no processor object.");
				}
				ip++;
			}

			failCount++;
			if ((failCount > SCRIPT_FORCED_PAUSE_LIMIT
				&& !noForcedPause)
				|| failCount > SCRIPT_FORCED_PAUSE_LIMIT * 50)
			{
				doPause = true;
				pausable = true;
				sp->misbehaveCounter++;
				if (sp->misbehaveCounter > 100 || noForcedPause)
				{
					sp->warning("Script::run - Script process did not finish within limit, terminating it.");
					sp->warning("Script::run - Proper cleanup will not be executed, side-effects may appear.");
					Logger::getInstance()->debug(sp->getScript()->getName());
					sp->finished = true;
				} else {
					Logger::getInstance()->debug("Script::run - Script process did not pause within step limit, forcing pause.");
				}
			}
		}
		sp->ip = ip;
#ifdef SCRIPT_TIMING_DEBUG
    Timer::update();
    int timeDiff = Timer::getTime() - currentTime;
    if (timeDiff >= 10 || failCount > 500)
    {
      char tbuf[256];
      sprintf(tbuf, "script %s - cmds %d, time %d ", name, failCount, timeDiff);
      Logger::getInstance()->debug(tbuf);
    }
#endif
#ifdef DEBUG_CHECK_FOR_UNINITIALIZED_SCRIPT_VALUE_USE
		::checked_int_value_sp = NULL;
#endif
	}

	int Script::getSubIP(const char *subName)
	{
		assert(subName != NULL);
		
		int hc;
		SCRIPT_HASHCODE_CALC(subName, &hc);

		SubIPHashType::iterator iter = subIPHash->find(hc);
		if (iter != subIPHash->end())
		{
			int ip = (*iter).second; 
			return ip;
		}
		return -1;		
	}

	int Script::getLabelIP(char *labelName)
	{
		for (int i = 0; i < commandAmount; i++)
		{
			if (commandArray[i] == SCRIPT_CMD_LABEL)
			{
				if (stringDataArray[i] != NULL)
				{
					if (strcmp(labelName, stringDataArray[i]) == 0)
					{
						return i;
					}
				}
			}
		} 
		return -1;
	}

	bool Script::hasSub(const char *subName)
	{
		if (getSubIP(subName) != -1) 
			return true;
		else
			return false;
	}

	bool Script::addCommand(const char *cmd, const char *data)
	{
		if (commandAmount >= allocedSize)
		{
			Logger::getInstance()->debug("Script::addCommand - Script size greater than allocated, reallocating.");
			assert(allocedSize > 0);
			int oldSize = allocedSize;
			int *oldCommandArray = commandArray;
			floatint *oldIntDataArray = intDataArray;
			char **oldStringDataArray = stringDataArray;
			VarOptimizeData **oldVarOptimizeDataArray = varOptimizeDataArray;
			allocedSize *= 2;
			commandArray = new int[allocedSize];
			intDataArray = new floatint[allocedSize];
			stringDataArray = new char *[allocedSize];
			varOptimizeDataArray = new VarOptimizeData *[allocedSize];

			for (int i = 0; i < allocedSize; i++)
			{
				if (i < oldSize)
				{
					commandArray[i] = oldCommandArray[i];
					intDataArray[i].i = oldIntDataArray[i].i;
					stringDataArray[i] = oldStringDataArray[i];
					varOptimizeDataArray[i] = oldVarOptimizeDataArray[i];
				} else {
					commandArray[i] = SCRIPT_CMD_NOP;
					intDataArray[i].i = 0;
					stringDataArray[i] = NULL;
					varOptimizeDataArray[i] = NULL;
				}
			}
			delete [] oldCommandArray;
			delete [] oldIntDataArray;
			delete [] oldStringDataArray;
			delete [] oldVarOptimizeDataArray;
		}

		// basic flow control commands...
		for (int keyw = 0; keyw < keywordsAmount; keyw++)
		{
			if (strcmp(cmd, keywords[keyw]) == 0)
			{
				commandArray[commandAmount] = keyw;
				intDataArray[commandAmount].i = 0;
				if(stringDataArray[commandAmount] != NULL)
				{
					delete [] stringDataArray[commandAmount];
				}
				stringDataArray[commandAmount] = NULL;
				varOptimizeDataArray[commandAmount] = NULL;
				if (data != NULL)
				{
					stringDataArray[commandAmount] = new char[strlen(data) + 1];
					strcpy(stringDataArray[commandAmount], data);
				}
				if (keyw == SCRIPT_CMD_LABEL)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Label name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {

						if (strncmp(data, "ADD!", 4) == 0)
						{
							assert(stringDataArray[commandAmount] != NULL);
							delete [] stringDataArray[commandAmount];
							stringDataArray[commandAmount] = new char[strlen(data) + 1];
							strcpy(stringDataArray[commandAmount], &data[4]);
						}

						if (strncmp(data, "_loop_", 6) == 0
							|| strncmp(data, "_endloop_", 9) == 0)
						{
							// ignore... was created by loop.
							// will be recreated properly by loop.
							commandAmount--;
						} else {
							int ip = getLabelIP(stringDataArray[commandAmount]);
							if (ip != -1)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Label already defined within script.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data); 
							}
						}
					}
				}
				else if (keyw == SCRIPT_CMD_PARAMGETVARIABLE)
				{
					// replace with appropriate "getVariable (param_variable_name_here)"
					if (!dont_convert_paramgetvariable)
					{
						bool isOk = false;
						if (data != NULL
							&& strncmp(data, "$_param_", 8) == 0)
						{
							int paramNum = str2int(&data[8]);
							if (str2int_errno() == 0
								&& paramNum >= 0)
							{
								if (paramNum < (int)script_paramVars.size())
								{
									addCommand("getVariable", script_paramVars[paramNum].c_str());
									script_paramVars[paramNum] = "_param_used";
									commandAmount--;
								} else {
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - More parameters expected.", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data); 
								}
								isOk = true;
							}
						}
						if (!isOk)
						{
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - _paramGetVariable parameter invalid (internal error).", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data); 
						}
					}
				}
				else if (keyw == SCRIPT_CMD_GETVARIABLE
					|| keyw == SCRIPT_CMD_SETVARIABLE
					|| keyw == SCRIPT_CMD_ADDVARIABLETOVALUE
					|| keyw == SCRIPT_CMD_DECREASEVARIABLEFROMVALUE
					|| keyw == SCRIPT_CMD_ADDVALUETOVARIABLE
					|| keyw == SCRIPT_CMD_DECREASEVALUEFROMVARIABLE
					|| keyw == SCRIPT_CMD_VARIABLEEQUALSVALUE)
				{
					if (data == NULL)
					{
						intDataArray[commandAmount].i = 0;
						if (keyw == SCRIPT_CMD_SETVARIABLE)
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - setVariable parameter missing.", true);
						else if (keyw == SCRIPT_CMD_GETVARIABLE)
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - getVariable parameter missing.", true);
						else
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Variable add/decrease command parameter missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						int hc;

						int datastringlen = strlen(data);
						if (datastringlen < 256)
						{
							char arrtmp[256+1];
							strcpy(arrtmp, data);
							for (int arri = 0; arri < datastringlen; arri++)
							{
								if (arrtmp[arri] == '[')
									arrtmp[arri] = '\0';
							}
							SCRIPT_HASHCODE_CALC(arrtmp, &hc);
						} else {
							SCRIPT_HASHCODE_CALC(data, &hc);
						}


						int ip = -1;

						// local variable...? (todo: optimize, some proper hash, or such)
						if (ip == -1)
						{
							int stackNegator = 0;
							int localSeek = commandAmount;
							while (localSeek > 0)
							{
								if (commandArray[localSeek] == SCRIPT_CMD_LOCAL)
								{
									stackNegator++;
									// that stringData contains the type too... int, ...
									//if (stringDataArray[localSeek] != NULL
									//	&& strcmp(stringDataArray[localSeek], data) == 0)
									// so using hashcode in intData instead...
									if (hc == intDataArray[localSeek].i)
									{
										ip = (stackNegator | SCRIPT_LOCAL_VAR_BITMASK); 
										break;
									}
								}
								else if (commandArray[localSeek] == SCRIPT_CMD_SUB)
								{
									break;
								}
								localSeek--;
							}
						}

						// shared local variable...?
						if (ip == -1)
						{
							VariableIPHashType::iterator iter = variableIPHash->find(hc);
							if (iter != variableIPHash->end())
							{
								ip = (*iter).second; 
							}
						}

						if (ip != -1)
						{
							intDataArray[commandAmount].i = ip;
						} else {
							intDataArray[commandAmount].i = 0;

							// global variable...?
							VariableHashType::iterator iter = globalVariableHash->find(hc);
							if (iter != globalVariableHash->end())
							{
								// was global, ok.
							} else {
								// variable was not properly defined...
								if (keyw == SCRIPT_CMD_SETVARIABLE)
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - setVariable, no variable defined with given name.", true);
								else if (keyw == SCRIPT_CMD_GETVARIABLE)
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - getVariable, no variable defined with given name.", true);
								else
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Variable add/decrease command, no variable defined with given name.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}
						}
					}
				}
				else if (keyw == SCRIPT_CMD_SETARRAYVARIABLEVALUES)
				{
					if (data == NULL)
					{
						intDataArray[commandAmount].i = 0;
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - setArrayVariableValues parameter missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						int hc;

						int datastringlen = strlen(data);
						if (datastringlen < 256)
						{
							char arrtmp[256+1];
							strcpy(arrtmp, data);
							for (int arri = 0; arri < datastringlen; arri++)
							{
								if (arrtmp[arri] == '[')
								{
									arrtmp[arri] = '\0';
									break;
								}
								if (arrtmp[arri] == ',')
								{
									arrtmp[arri] = '\0';
									break;
								}
							}
							SCRIPT_HASHCODE_CALC(arrtmp, &hc);
						} else {
							SCRIPT_HASHCODE_CALC(data, &hc);
						}

						// local variable...?
						VariableIPHashType::iterator iter = variableIPHash->find(hc);
						int ip = -1;
						if (iter != variableIPHash->end())
						{
							ip = (*iter).second; 
						}

						if (ip != -1)
						{
							intDataArray[commandAmount].i = ip;

							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - setArrayVariableValues not supported for shared local variables.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						} else {
							intDataArray[commandAmount].i = 0;

							// global variable...?
							VariableHashType::iterator iter = globalVariableHash->find(hc);
							if (iter != globalVariableHash->end())
							{
								if ((*iter).second.dataType == SCRIPT_DATATYPE_ARRAY)
								{
									// was global, was array, ok.
								} else {
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - setArrayVariableValues, given variable is not of array type.", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data);
								}
							} else {
								// variable was not properly defined...
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - setArrayVariableValues, no variable defined with given name.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}
						}
					}
				}
				else if (keyw == SCRIPT_CMD_LOOP)
				{
					int num = commandAmount;

					// WARNING: int -> void * cast
					loopNumBuf->append((void *)num);

					// don't overwrite this one.
					commandAmount++;

					char buf[32];
					strcpy(buf, "ADD!");
					strcat(buf, "_loop_");
					strcat(buf, int2str(num));
					addCommand("label", buf);

					commandAmount--; // will be incremented later... 
					// (see end of this function)
				}
				else if (keyw == SCRIPT_CMD_ENDLOOP)
				{
					// WARNING: void * -> int cast
					if (loopNumBuf->isEmpty())
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Missing loop before endLoop.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						intptr_t num = (intptr_t)loopNumBuf->popLast();

						// don't overwrite this one.
						commandAmount++;

						char buf[32];
						strcpy(buf, "ADD!");
						strcat(buf, "_loop_");
						strcat(buf, int2str(num));
						addCommand("goto", buf);

						strcpy(buf, "ADD!");
						strcat(buf, "_endloop_");
						strcat(buf, int2str(num));
						addCommand("label", buf);

						commandAmount--; // will be incremented later... 
						// (see end of this function)
					}
				}
				else if (keyw == SCRIPT_CMD_BREAKLOOP)
				{
					// WARNING: void * -> int cast
					if (loopNumBuf->isEmpty())
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Missing loop before breakLoop.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						intptr_t num = (intptr_t)loopNumBuf->peekLast();

						// don't overwrite this one.
						commandAmount++;

						char buf[32];
						strcpy(buf, "ADD!");
						strcat(buf, "_endloop_");
						strcat(buf, int2str(num));
						addCommand("goto", buf);

						commandAmount--; // will be incremented later... 
						// (see end of this function)
					}
				}
				else if (keyw == SCRIPT_CMD_RESTARTLOOP)
				{
					// WARNING: void * -> int cast
					if (loopNumBuf->isEmpty())
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Missing loop before restartLoop.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						intptr_t num = (intptr_t)loopNumBuf->peekLast();

						// don't overwrite this one.
						commandAmount++;

						char buf[32];
						strcpy(buf, "ADD!");
						strcat(buf, "_loop_");
						strcat(buf, int2str(num));
						addCommand("goto", buf);

						commandAmount--; // will be incremented later... 
						// (see end of this function)
					}
				}
				else if (keyw == SCRIPT_CMD_CASE)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Case number missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						intDataArray[commandAmount].i = str2int(data);
						if (str2int_errno() != 0)
						{
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Number expected.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						}
					}
				}
				else if (keyw == SCRIPT_CMD_UPTOCASE)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Up To Case number missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						intDataArray[commandAmount].i = str2int(data);
						if (str2int_errno() != 0)
						{
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Number expected.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						}
					}
				}
				else if (keyw == SCRIPT_CMD_CASES)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Cases number range missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						char buf[64 + 1];
						int slen = strlen(data);
						int splitpos = -1;
						if (slen < 64)
						{
							strcpy(buf, data);
							for (int bufp = 0; bufp < slen; bufp++)
							{
								if (buf[bufp] == 't' && buf[bufp+1] == 'o')
								{
									splitpos = bufp + 2;
									buf[bufp] = '\0';

									bufp += 2;
									for (; bufp < slen; bufp++)
									{
										if (buf[bufp] != ' ' && buf[bufp] != '\t')
										{
											splitpos = bufp;
											break;
										}
									}
									break;
								}
							}
						}
						bool numRangeOk = false;
						int from = 0;
						int upto = 0;
						if (splitpos != -1)
						{
							numRangeOk = true;
							for (int trimi = strlen(buf) - 1; trimi >= 0; trimi--)
							{
								if (buf[trimi] == ' ' || buf[trimi] == '\t')
									buf[trimi] = '\0';
								else
									break;
							}
							from = str2int(buf);
							if (str2int_errno() != 0)
							{
								numRangeOk = false;
							}
							for (int trimi = strlen(&buf[splitpos]) - 1; trimi >= 0; trimi--)
							{
								if ((&buf[splitpos])[trimi] == ' ' || (&buf[splitpos])[trimi] == '\t')
									(&buf[splitpos])[trimi] = '\0';
								else
									break;
							}
							upto = str2int(&buf[splitpos]);
							if (str2int_errno() != 0)
							{
								numRangeOk = false;
							}
						}
						if (!numRangeOk)
						{
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Number range expected.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						} else {
							// don't overwrite this one. (why not?)
							//commandAmount++;

							if (from > upto)
							{
								// swap lower/upper limit (as they seem to be incorrectly from upper to lower)
								int tmp = upto;
								upto = from;
								from = tmp;
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Number range should be from lower to upper value.", false);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}

							char addbuf[32];
							strcpy(addbuf, int2str(from));
							addCommand("case", addbuf);
							strcpy(addbuf, int2str(upto));
							addCommand("_upToCase", addbuf);

							commandAmount--; // will be incremented later... 
							// (see end of this function)
						}
					}
				}
				else if (keyw == SCRIPT_CMD_GOTO)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Goto label name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						if (strncmp(data, "ADD!", 4) == 0)
						{
							assert(stringDataArray[commandAmount] != NULL);
							delete [] stringDataArray[commandAmount];
							stringDataArray[commandAmount] = new char[strlen(data) + 1];
							strcpy(stringDataArray[commandAmount], &data[4]);
						}

						if (strncmp(data, "_loop_", 6) == 0
							|| strncmp(data, "_endloop_", 9) == 0)
						{
							// ignore... was created by loop.
							// will be recreated properly by loop.
							commandAmount--;
						}
					}
				}
				else if (keyw == SCRIPT_CMD_CALL
					|| keyw == SCRIPT_CMD_CALLIFEXISTS)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Call sub name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						// HACK: ...
						static bool dont_process_call_subs = false;

						bool withParams = false;
						int datalen = strlen(data);
						int dataSubNameCutPos = -1;
						int dataSubNameCutPosEnd = -1;

						for (int j = 0; j < datalen; j++)
						{
							if (data[j] == '(')
							{
								if (data[j + 1] != ')')
								{
									withParams = true;
									for (int k = j; k < datalen; k++)
									{
										if (data[k] == ')')
										{
											dataSubNameCutPosEnd = k;
											break;
										}
									}
								}
								dataSubNameCutPos = j;
								break;
							}
						}

						const char *subname = data;
						std::string cuttedSubname;
						if (dataSubNameCutPos != -1)
						{
							cuttedSubname = data;
							cuttedSubname	= cuttedSubname.substr(0, dataSubNameCutPos);
							subname = cuttedSubname.c_str();
						}

						// has entry sub?
						bool hasEntrySub = false;
						std::string p_enter = std::string("_call_enter_") + subname;
						if (this->hasSub(p_enter.c_str()))
						{
							hasEntrySub = true;
						}

						// FIXME: this bugs, if...
						// a sub is called without params
						// and the sub is void returning sub
						// and the sub is declared after this call.

						if (!dont_process_call_subs && (withParams || hasEntrySub))
						{
							if (!this->hasSub(subname))
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Subs with parameters must be declared before calling.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							} else {
								if (withParams && !hasEntrySub)
								{
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub does not take in any parameters (but call has parameters).", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data);
								}
							}

							// has entry sub?
							//std::string p_enter = std::string("_call_enter_") + subname;
							//if (this->hasSub(p_enter.c_str()))
							if (hasEntrySub)
							{
								if (withParams && dataSubNameCutPos != -1
									&& dataSubNameCutPosEnd != -1)
								{
									std::string ptmp = data;
									ptmp = ptmp.substr(dataSubNameCutPos + 1, dataSubNameCutPosEnd - (dataSubNameCutPos + 1));

									bool parseOk = parse_script_params(ptmp, script_paramVars);
									if (!parseOk)
									{
										ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Call parameter parse error.", true);
										Logger::getInstance()->debug(cmd);
										Logger::getInstance()->debug(data);
									}
								}

								std::string p_inc = std::string(this->getName()) + ":" + p_enter;
								dont_add_externcallpushpop = true;
								addCommand("externInclude", p_inc.c_str());
								dont_add_externcallpushpop = false;

								for (int i = 0; i < (int)script_paramVars.size(); i++)
								{
									if (script_paramVars[i] != "_param_used")
									{
										ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Too many call parameters.", true);
										Logger::getInstance()->debug(cmd);
										Logger::getInstance()->debug(data);
										break;
									}
								}

								script_paramVars.clear();
							}

							dont_process_call_subs = true;
							std::string datacopy = subname;
							addCommand("call", datacopy.c_str());
							dont_process_call_subs = false;

							// has leave sub?
							std::string p_leave = std::string("_call_leave_") + subname;
							if (this->hasSub(p_leave.c_str()))
							{
								std::string p_inc = std::string(this->getName()) + ":" + p_leave;
								dont_add_externcallpushpop = true;
								addCommand("externInclude", p_inc.c_str());
								dont_add_externcallpushpop = false;
							}

							commandAmount--;
						}
					}
				}
				else if (keyw == SCRIPT_CMD_SUBEXISTS)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - subExists parameter sub name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					}
				}
				else if (keyw == SCRIPT_CMD_PUSHCALLSTACK_INT
					|| keyw == SCRIPT_CMD_POPCALLSTACK_INT)
				{
					if (data != NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - _pushCallStack_int/_popCallStack_int, no parameter expected.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					}
				}
				else if (keyw == SCRIPT_CMD_PUSHCALLSTACK_MARKER
					|| keyw == SCRIPT_CMD_POPCALLSTACK_MARKER)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - _pushCallStack_marker/_popCallStack_marker expected integer parameter.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						intDataArray[commandAmount].i = str2int(data);
						// TODO: check str2int_errno.
					}
				}
				else if (keyw == SCRIPT_CMD_SUB)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						int containsComma = -1;
						int containsParenthesis = -1;
						int containsParenthesisEnd = -1;
						int datalen = strlen(data);
						int lastParamStart = -1;
						// type, name pair
						std::vector<std::pair<std::string, std::string> > params;

						for (int j = 0; j < datalen; j++)
						{
							bool parseParamFromHere = false;

							if (data[j] == ',')
							{
								if (containsParenthesis == -1)
								{
									if (containsComma == -1)
									{
										containsComma = j;
									} else {
										ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter format bad (expected type,subname(parameters...).", true);
										Logger::getInstance()->debug(cmd);
										Logger::getInstance()->debug(data); 										
									}
								} else {
									parseParamFromHere = true;
								}
							}
							if (data[j] == '(' && data[j + 1] != ')')
							{
								if (containsParenthesis != -1)
								{
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter format bad (multiple starting parenthesis).", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data); 										
								}
								containsParenthesis = j;
								lastParamStart = j + 1;
							}
							if (data[j] == ')'
								&& j > 0 && data[j - 1] != '(')
							{
								if (containsParenthesis == -1)
								{
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter format bad (starting parenthesis missing before ending parenthesis).", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data); 										
								}
								if (containsParenthesisEnd != -1)
								{
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter format bad (multiple ending parenthesis).", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data); 										
								}
								containsParenthesisEnd = j;
								parseParamFromHere = true;
							}
							if (parseParamFromHere)
							{
								bool wasOk = false;

								assert(lastParamStart != -1);

								int tmpsize = (j - lastParamStart);

								if (tmpsize > 0)
								{
									char *tmpbuf = new char[tmpsize + 1];
									strncpy(tmpbuf, &data[lastParamStart], tmpsize);
									tmpbuf[tmpsize] = '\0';
									std::string tmp = tmpbuf;
									std::string tmp2 = StringRemoveWhitespace(tmp);
									assert((int)tmp2.size() <= tmpsize);
									strcpy(tmpbuf, tmp2.c_str());

									tmpsize = tmp2.size();

									// note, intentional inclusion of terminating null.
									for (int k = 0; k < tmpsize + 1; k++)
									{
										if (tmpbuf[k] == ' ' || tmpbuf[k] == '\t'
											|| tmpbuf[k] == '\0')
										{
											tmpbuf[k] = '\0';

											if (k < tmpsize)
											{
												std::string tmp_type = tmpbuf;
												std::string tmp_type2 = StringRemoveWhitespace(tmp_type);
												std::string tmp_name = &tmpbuf[k + 1];
												std::string tmp_name2 = StringRemoveWhitespace(tmp_name);
												if (!tmp_type2.empty() 
													&& !tmp_name2.empty())
												{
													if (tmp_type2 != "int")
													{
														ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter type unsupported (only int type supported).", true);
														Logger::getInstance()->debug(cmd);
														Logger::getInstance()->debug(data);
													}
													params.push_back(std::pair<std::string, std::string>(tmp_type2, tmp_name2));
													wasOk = true;
												}
											}
										}
									}
									delete [] tmpbuf;
								}
								if (!wasOk)
								{
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter list missing type definition or name.", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data);
								}
								lastParamStart = j + 1;
							}
						}
						if (containsParenthesis != -1)
						{
							if (containsParenthesisEnd == -1)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter format bad (missing ending parenthesis).", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data); 										
							}
						}
						// WARNING: casting const char * to char *
						char *subname = (char *)data;
						char *rettypename = NULL;

						bool allocedNameBufs = false;
						bool createEnterAndLeaveSubs = false;
						if (containsComma != -1
							|| containsParenthesis != -1)
						{
							allocedNameBufs = true;
							createEnterAndLeaveSubs = true;

							subname = new char[strlen(data) + 1];
							rettypename = new char[strlen(data) + 1];
							int subnamelen = 0;
							if (containsParenthesis != -1)
							{
								subnamelen = containsParenthesis - (containsComma + 1);
							} else {
								subnamelen = strlen(&data[containsComma + 1]) - (containsComma + 1);
							}
							strncpy(subname, &data[containsComma + 1], subnamelen);
							subname[subnamelen] = '\0';
							
							{
								std::string tmp = subname;
								std::string tmp2 = StringRemoveWhitespace(tmp);
								assert(tmp2.size() <= strlen(data));
								strcpy(subname, tmp2.c_str());
							}

							if (containsComma == 0)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub parameter format bad (return datatype missing).", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data); 										
							}
							if (containsComma > 0)
							{
								strncpy(rettypename, data, containsComma);
								rettypename[containsComma] = '\0';
								
								std::string tmp = rettypename;
								std::string tmp2 = StringRemoveWhitespace(tmp);
								assert(tmp2.size() <= strlen(data));
								strcpy(rettypename, tmp2.c_str());
							} else {
								strcpy(rettypename, "int");
							}
						}

						int ip = getSubIP(subname);
						if (ip != -1)
						{
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Sub already defined within script.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data); 
						}
						int hc;
						SCRIPT_HASHCODE_CALC(subname, &hc);

						if (createEnterAndLeaveSubs)
						{
							const char *pushTypeCmd = "_pushCallStack_int";
							const char *popTypeCmd = "_popCallStack_int";

							if (strcmp(rettypename, "int") == 0)
							{
								// no push/pop, let the lastValue pass through...
								pushTypeCmd = NULL;
								popTypeCmd = NULL;
							}
							else if (strcmp(rettypename, "void") == 0)
							{
								// ok. use the int push/pop
							} else {
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Unsupported sub return value type (only void or int supported).", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data); 
							}

							std::string enterSubName = std::string("_call_enter_") + subname;
							std::string leaveSubName = std::string("_call_leave_") + subname;
							addCommand("sub", enterSubName.c_str());
							std::string paramAmountStr = int2str((int)params.size());
							addCommand("_pushCallStack_marker", paramAmountStr.c_str());
							if (pushTypeCmd != NULL)
								addCommand(pushTypeCmd, NULL);
							for (int i = 0; i < (int)params.size(); i++)
							{
								dont_convert_paramgetvariable = true;
								std::string tmp = std::string("$_param_") + int2str(i);
								addCommand("_paramGetVariable", tmp.c_str());
								dont_convert_paramgetvariable = false;
								addCommand("_pushCallStack_int", NULL);
							}
							addCommand("endSub", NULL);

							addCommand("sub", leaveSubName.c_str());
							if (popTypeCmd != NULL)
								addCommand(popTypeCmd, NULL);
							addCommand("_popCallStack_marker", paramAmountStr.c_str());
							addCommand("endSub", NULL);
						}

						subIPHash->insert(std::pair<int, int> (hc, commandAmount));

						script_currently_parsing_sub = subname;

						commandArray[commandAmount] = keyw;
						intDataArray[commandAmount].i = 0;
						if(stringDataArray[commandAmount] != NULL)
						{
							delete [] stringDataArray[commandAmount];
						}
						stringDataArray[commandAmount] = NULL;
						varOptimizeDataArray[commandAmount] = NULL;
						if (data != NULL)
						{
							stringDataArray[commandAmount] = new char[strlen(data) + 1];
							strcpy(stringDataArray[commandAmount], data);
						}

						if (allocedNameBufs)
						{
							delete [] subname;
							delete [] rettypename;
						}

						// don't override the current sub command...
						commandAmount++;

						for (int i = (int)params.size() - 1; i >= 0; i--)
						{
							assert(params[i].first == "int");
							std::string tmp = std::string("int,") + params[i].second.c_str();
								addCommand("local", tmp.c_str());
							addCommand("_popCallStack_int", NULL);
							addCommand("setVariable", params[i].second.c_str());
						}

						commandAmount--;

					}
				}
				else if (keyw == SCRIPT_CMD_LOCAL)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - local variable name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						if (strncmp(data, "int,", 4) == 0)
						{
							int hc;

							// skip leading spaces...
							int skipsp = 4;
							int datalen = strlen(data);
							for (; skipsp < datalen; skipsp++)
							{
								if (data[skipsp] != ' '
									&& data[skipsp] != '\t')
								{
									break;
								}
							}

							SCRIPT_HASHCODE_CALC((&data[skipsp]), &hc);

							// check for global variable shadowing...
							VariableHashType::iterator globalIter = globalVariableHash->find(hc);
							if (globalIter != globalVariableHash->end())
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - local variable shadows global variable with same name.", false);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}

							// check for shared local variable shadowing...
							VariableIPHashType::iterator shaiter = variableIPHash->find(hc);
							if (shaiter != variableIPHash->end())
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - local variable shadows sharedLocal variable with same name.", false);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}

							// duplicates in locals?
							int localSeek = commandAmount;
							while (localSeek > 0)
							{
								if (commandArray[localSeek] == SCRIPT_CMD_LOCAL)
								{
									// that stringData contains the type too... int, ...
									//if (stringDataArray[localSeek] != NULL
									//	&& strcmp(stringDataArray[localSeek], data) == 0)
									// so using hashcode in intData instead...
									if (hc == intDataArray[localSeek].i)
									{
										ScriptManager::getInstance()->scriptCompileError("Script::addCommand - duplicate local variable name.", true);
										Logger::getInstance()->debug(cmd);
										Logger::getInstance()->debug(data);
										break;
									}
								}
								else if (commandArray[localSeek] == SCRIPT_CMD_SUB)
								{
									break;
								}
								localSeek--;
							}

							intDataArray[commandAmount].i = hc;
						} else {
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - local variable type bad (only int type supported).", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						}
					}
				}
				else if (keyw == SCRIPT_CMD_SHAREDLOCAL)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - sharedLocal variable name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						if (strncmp(data, "int,", 4) == 0)
						{
							int hc;

							// skip leading spaces...
							int skipsp = 4;
							int datalen = strlen(data);
							for (; skipsp < datalen; skipsp++)
							{
								if (data[skipsp] != ' '
									&& data[skipsp] != '\t')
								{
									break;
								}
							}

							SCRIPT_HASHCODE_CALC((&data[skipsp]), &hc);

							// check for global variable shadowing...
							VariableHashType::iterator globalIter = globalVariableHash->find(hc);
							if (globalIter != globalVariableHash->end())
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - sharedLocal variable shadows global variable with same name.", false);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}

							// check for local variables that might be shadowing this one?
							int localSeek = commandAmount;
							while (localSeek > 0)
							{
								if (commandArray[localSeek] == SCRIPT_CMD_LOCAL)
								{
									// that stringData contains the type too... int, ...
									//if (stringDataArray[localSeek] != NULL
									//	&& strcmp(stringDataArray[localSeek], data) == 0)
									// so using hashcode in intData instead...
									if (hc == intDataArray[localSeek].i)
									{
										ScriptManager::getInstance()->scriptCompileError("Script::addCommand - local variable shadows sharedLocal variable with same name.", false);
										Logger::getInstance()->debug(cmd);
										Logger::getInstance()->debug(data);
										break;
									}
								}
								else if (commandArray[localSeek] == SCRIPT_CMD_SUB)
								{
									break;
								}
								localSeek--;
							}

							VariableIPHashType::iterator iter = variableIPHash->find(hc);
							int ip = -1;
							if (iter != variableIPHash->end())
							{
								ip = (*iter).second; 
							}

							if (ip == -1)
								variableIPHash->insert(std::pair<int, int> (hc, commandAmount));
						} else {
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - sharedLocal variable type bad (only int type supported).", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						}
					}
				}
				else if (keyw == SCRIPT_CMD_GLOBAL
					|| keyw == SCRIPT_CMD_PERMANENTGLOBAL)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - global or permanent global variable name missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						int createDatatype = SCRIPT_DATATYPE_NONE;
						int skipstart = 0;
						int arraySize = 0;

						if (strncmp(data, "int,", 4) == 0)
						{
							createDatatype = SCRIPT_DATATYPE_INT;
							skipstart = 4;
						} 
						else if (strncmp(data, "string,", 7) == 0)
						{
							createDatatype = SCRIPT_DATATYPE_STRING;
							skipstart = 7;
						}
						else if (strncmp(data, "float,", 6) == 0)
						{
							createDatatype = SCRIPT_DATATYPE_FLOAT;
							skipstart = 6;
						}
						else if (strncmp(data, "position,", 9) == 0)
						{
							createDatatype = SCRIPT_DATATYPE_POSITION;
							skipstart = 9;
						}
						else if (strncmp(data, "int[", 4) == 0)
						{
							char *tmp = new char[strlen(data) + 1];
							strcpy(tmp, data);
							for (int tmpi = 4; tmpi < (int)strlen(tmp); tmpi++)
							{
								if (tmp[tmpi] == ']'
									|| (tmp[tmpi] == ' ' && tmp[tmpi+1] == ']'))
								{
									if (tmp[tmpi + 1] == ',' || 
										(tmp[tmpi+1] == ']' && tmp[tmpi + 2] == ','))
									{
										tmp[tmpi] = '\0';
										arraySize = str2int(&tmp[4]);
										if (arraySize > 0)
										{
											createDatatype = SCRIPT_DATATYPE_ARRAY;
											skipstart = tmpi + 1 + 1;
											if (tmp[tmpi+1] == ']')
												skipstart++;
										} else {
											ScriptManager::getInstance()->scriptCompileError("Script::addCommand - array variable size must be a positive integer", true);
										}
										break;
									}
								}
							}
							delete [] tmp;
						}

						if (createDatatype != SCRIPT_DATATYPE_NONE)
						{
							int hc;

							// skip leading spaces... and the datatype!
							int skipsp = skipstart;
							int datalen = strlen(data);
							for (; skipsp < datalen; skipsp++)
							{
								if (data[skipsp] != ' '
									&& data[skipsp] != '\t')
								{
									break;
								}
							}

							const char *varname = &data[skipsp];

							SCRIPT_HASHCODE_CALC(varname, &hc);

							VariableHashType::iterator iter = globalVariableHash->find(hc);
							bool didExist = false;
							bool existIsPermanent = false;
							int existType = SCRIPT_DATATYPE_NONE;
							if (iter != globalVariableHash->end())
							{
								didExist = true;
								existIsPermanent = (*iter).second.permanent; 
								existType = (*iter).second.dataType; 
							}

							bool createPermanent = false;
							if (keyw == SCRIPT_CMD_PERMANENTGLOBAL)
								createPermanent = true;

							if (didExist && existIsPermanent != createPermanent)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - global or permanent global variable already declared with different persistance.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}

							if (didExist && existType != createDatatype)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - global or permanent global variable already declared with different type.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}

							if (!didExist)
							{
								VariableDataType data;
								data.dataType = createDatatype;
								data.floatValue = 0.0f;
								data.xValue = 0.0f;
								data.yValue = 0.0f;
								data.zValue = 0.0f;
								data.stringData = NULL;
								data.intValue = 0;
								data.arrayValue = NULL;
								data.arraySize = 0;
								if (arraySize > 0)
								{
									data.arraySize = arraySize;
									data.arrayValue = new int[arraySize];
									for (int arrinit = 0; arrinit < arraySize; arrinit++)
									{
										data.arrayValue[arrinit] = 0;
									}
								}
								data.permanent = false;
								if (createPermanent)
									data.permanent = true;

								data.name = new char[strlen(varname) + 1];
								strcpy(data.name, varname);

								globalVariableHash->insert(std::pair<int, VariableDataType> (hc, data));
							} else {
								// check that hashcodes are ok...
								if (strcmp((*iter).second.name, varname) != 0)
								{
									ScriptManager::getInstance()->scriptCompileError("Script::addCommand - global variable hash failure.", true);
									Logger::getInstance()->debug(cmd);
									Logger::getInstance()->debug(data);
								}
							}
						} else {
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - global or permanent global variable type bad.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						}
					}
				}
				else if (keyw == SCRIPT_CMD_EXTERNCALL)
				{
					if (data == NULL)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - externInclude parameter missing.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					} else {
						if (strlen(data) < 256)
						{
							char splitbuf[256];
							strcpy(splitbuf, data);
							int buflen = strlen(splitbuf);
							bool paramOk = false;
							for (int splitp = 0; splitp < buflen; splitp++)
							{
								if (splitbuf[splitp] == ':')
								{
									splitbuf[splitp] = '\0';
									for (int spl = splitp-1; spl > 0; spl--)
									{
										if (splitbuf[spl] != ' ')
											break;
										splitbuf[spl] = '\0';
									}
									paramOk = true;
									
									char *p1 = splitbuf;
									char *p2 = &splitbuf[splitp + 1];

									bool selfInc = false;

									Script *s = ScriptManager::getInstance()->getScript(p1);
									if (s == this)
									{
										//Logger::getInstance()->debug("Script::addCommand - externInclude within script. Possible problems ahead.");
										if (script_currently_parsing_sub == p2)
										{
											// oh noes...
											Logger::getInstance()->error("Script::addCommand - externInclude attempting to include itself (not allowed as it would result in infinite recursion).");
											Logger::getInstance()->debug(cmd);
											Logger::getInstance()->debug(data);
											selfInc = true;
											s = NULL;
										}								
									}
									if (s != NULL)
									{
										if (s->hasSub(p2))
										{
											int subip = s->getSubIP(p2) + 1;

											bool includedSubIsEmpty = false;
											if (subip < s->commandAmount 
												&& (s->commandArray[subip] == SCRIPT_CMD_ENDSUB
												|| s->commandArray[subip] == SCRIPT_CMD_RETURN))
											{
												includedSubIsEmpty = true;
											}

											if (includedSubIsEmpty)
											{
												// chomp empty sub includes.
											} else {
												if (!dont_add_externcallpushpop)
													addCommand("_externCallPush", NULL);
												for (; subip < s->commandAmount; subip++)
												{ 
													if (s->commandArray[subip] == SCRIPT_CMD_ENDSUB)
													{
														break;
													}
													if (s->commandArray[subip] < keywordsAmount)
													{
														if (s->commandArray[subip] == SCRIPT_CMD_RETURN)
														{
															assert(s->stringDataArray[subip] == NULL);
															assert(!dont_add_externcallpushpop);
															addCommand("_externCallReturn", s->stringDataArray[subip]);
														}
														else if (s->commandArray[subip] == SCRIPT_CMD_RETURNMULTIPLE)
														{
															//char numbuf[16];
															//strcpy(numbuf, int2str(s->intDataArray[subip].i))
															//addCommand("_externCallReturnMultiple", numbuf);
															assert(!dont_add_externcallpushpop);
															addCommand("_externCallReturnMultiple", s->stringDataArray[subip]);
														} else {
															addCommand(keywords[s->commandArray[subip]], s->stringDataArray[subip]);
														}
													} else {
														int pkey = s->commandArray[subip] - keywordsAmount;
														if (s->processorDatatypes[pkey] == SCRIPT_DATATYPE_INT)
														{
															// WARNING: passing pointer to static variable here...
															char *numstr = int2str(s->intDataArray[subip].i);
															addCommand(s->processorKeywords[pkey], numstr);
														}
														else if (s->processorDatatypes[pkey] == SCRIPT_DATATYPE_FLOAT)
														{
															char numstr[16];
															sprintf(numstr, "%f", s->intDataArray[subip].f);
															addCommand(s->processorKeywords[pkey], numstr);
														} 
														else 
														{
															script_doublequotes_warning = false;
															addCommand(s->processorKeywords[pkey], s->stringDataArray[subip]);
														}
													}
												}
												if (!dont_add_externcallpushpop)
													addCommand("_externCallPop", NULL);
											}

											commandAmount--; // cos it will be incremented after this
										} else {
											ScriptManager::getInstance()->scriptCompileError("Script::addCommand - externInclude failed, given script has no given sub.", true);
											Logger::getInstance()->debug(cmd);
											Logger::getInstance()->debug(data);
										}
									} else {
										if (!selfInc)
										{
											ScriptManager::getInstance()->scriptCompileError("Script::addCommand - externInclude failed, given script not loaded.", true);
											Logger::getInstance()->debug(cmd);
											Logger::getInstance()->debug(data);
										}
									}
									break;
								}
							}
							if (!paramOk)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - externCall parameter format bad.", true);
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}
						} else {
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - externCall parameter too long.", true);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);
						}
					}
				}

				commandAmount++;
				return true;	
			}
		}

		// external processor commands...
		for (int i = 0; i < processorKeywordsAmount; i++)
		{
			if (strcmp(cmd, processorKeywords[i]) == 0)
			{
				commandArray[commandAmount] = keywordsAmount + i;
				bool dataTypeOk = false;
				if (processorDatatypes[i] == SCRIPT_DATATYPE_NONE)
				{
					if (data != NULL
						&& strcmp(data, "") != 0)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Parameter data ignored as no data expected.", false);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					}
					intDataArray[commandAmount].i = 0;
					if (stringDataArray[commandAmount] != NULL)
					{
						delete [] stringDataArray[commandAmount];
					}
					stringDataArray[commandAmount] = NULL;
					if (varOptimizeDataArray[commandAmount] != NULL)
					{
						delete varOptimizeDataArray[commandAmount];
					}
					varOptimizeDataArray[commandAmount] = NULL;
					dataTypeOk = true;
				}
				if (processorDatatypes[i] == SCRIPT_DATATYPE_INT)
				{
					bool numexp = false;
					if (data != NULL)
					{
						intDataArray[commandAmount].i = str2int(data);
						if (str2int_errno() != 0)	
							numexp = true;
					} else {
						intDataArray[commandAmount].i = 0;
						numexp = true;
					}
					if (numexp)
					{
						ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Number expected.", true);
						Logger::getInstance()->debug(cmd);
						Logger::getInstance()->debug(data);
					}
					if (stringDataArray[commandAmount] != NULL)
					{
						delete [] stringDataArray[commandAmount];
						//assert(!"stringDataArray was not null, should be.");
					}
					stringDataArray[commandAmount] = NULL;
					if (varOptimizeDataArray[commandAmount] != NULL)
					{
						delete varOptimizeDataArray[commandAmount];
						//assert(!"varOptimizeDataArray was not null, should be.");
					}
					varOptimizeDataArray[commandAmount] = NULL;
					dataTypeOk = true;
				}
				if (processorDatatypes[i] == SCRIPT_DATATYPE_FLOAT)
				{
					float val = 0;
					if (data != NULL)
					{
						val = (float)atof(data);
						// TODO: a better float check... :)
						if ((data[0] < '0' || data[0] > '9') && data[0] != '-')
						{
							ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Float expected.", false);
							Logger::getInstance()->debug(cmd);
							Logger::getInstance()->debug(data);							
						}
					}
					intDataArray[commandAmount].f = val;
					if (stringDataArray[commandAmount] != NULL)
					{
						delete [] stringDataArray[commandAmount];
						//assert(!"stringDataArray was not null, should be.");
					}
					stringDataArray[commandAmount] = NULL;
					if (varOptimizeDataArray[commandAmount] != NULL)
					{
						delete varOptimizeDataArray[commandAmount];
						//assert(!"varOptimizeDataArray was not null, should be.");
					}
					varOptimizeDataArray[commandAmount] = NULL;
					dataTypeOk = true;
				}
				if (processorDatatypes[i] == SCRIPT_DATATYPE_STRING)
				{
					intDataArray[commandAmount].i = 0;
					if (stringDataArray[commandAmount] != NULL)
					{
						delete [] stringDataArray[commandAmount];
						//assert(!"stringDataArray was not null, should be.");
					}
					stringDataArray[commandAmount] = NULL;
					if (varOptimizeDataArray[commandAmount] != NULL)
					{
						delete varOptimizeDataArray[commandAmount];
						//assert(!"varOptimizeDataArray was not null, should be.");
					}
					varOptimizeDataArray[commandAmount] = NULL;
					if (data != NULL)
					{
						stringDataArray[commandAmount] = new char[strlen(data) + 1];
						strcpy(stringDataArray[commandAmount], data);

						// check that this looks like a string...
						int slen = strlen(data);
						if (slen > 0 && script_doublequotes_warning)
						{
							int strchkpos;
							for (strchkpos = 0; strchkpos < slen; strchkpos++)
							{
								if (data[strchkpos] == '"')
									break;
								if ((data[strchkpos] < '0' || data[strchkpos] > '9')
									&& data[strchkpos] != '-' && data[strchkpos] != ' ')
									break;
							}
							if (strchkpos == slen)
							{
								ScriptManager::getInstance()->scriptCompileError("Script::addCommand - String expected, but parameter seems like integer, possibly an error.", false);
								Logger::getInstance()->debug("Either use doublequotes to avoid this message, or use float number notation (example: 1.0f) instead of integer number notation (example: 1).");
								Logger::getInstance()->debug(cmd);
								Logger::getInstance()->debug(data);
							}
						}
					}
					dataTypeOk = true;
				}
				if (!dataTypeOk)
				{
					ScriptManager::getInstance()->scriptCompileError("Script::addCommand - Unknown data type.", true);
					Logger::getInstance()->debug(cmd);
					Logger::getInstance()->debug(data);
				}
				commandAmount++;
				return true;	
			}
		}

		return false;
	}

	void Script::optimizeJumps()
	{
		// a flag to mark "subExists" conditional calls 
		// (just so that there will be no warning about them)
		// not a flawless logic, but better than nothing.
		bool conditionalCall = false;

		for (int i = 0; i < commandAmount; i++)
		{
			if (commandArray[i] == SCRIPT_CMD_SUBEXISTS)
			{
				conditionalCall = true;
				Logger::getInstance()->debug("Script::optimizeJumps - subExists encountered, assuming next call to be conditional.");
			}
			if (commandArray[i] == SCRIPT_CMD_GOTO)
			{
				if (stringDataArray[i] == NULL)
				{
					scrman_currentline = 0;
					ScriptManager::getInstance()->scriptCompileError("Script::optimizeJumps - Goto command has no target label name.", false);
				} else {
					int badip = getSubIP(stringDataArray[i]);
					if (badip != -1)
					{
						scrman_currentline = 0;
						ScriptManager::getInstance()->scriptCompileError("Script::optimizeJumps - Goto command target name specifies a sub.", false);
						Logger::getInstance()->debug(stringDataArray[i]);
					}
					int ip = getLabelIP(stringDataArray[i]);
					if (ip == -1)
					{
						scrman_currentline = 0;
						ScriptManager::getInstance()->scriptCompileError("Script::optimizeJumps - Requested label was not found.", false);
						ScriptManager::getInstance()->scriptCompileError(stringDataArray[i], false);
						//Logger::getInstance()->debug(stringDataArray[i]);
					}
					intDataArray[i].i = ip;
				}
			}

			if (commandArray[i] == SCRIPT_CMD_CALL
				|| commandArray[i] == SCRIPT_CMD_CALLIFEXISTS)
			{
				if (commandArray[i] == SCRIPT_CMD_CALLIFEXISTS)
					conditionalCall = true;

				if (stringDataArray[i] == NULL)
				{
					scrman_currentline = 0;
					ScriptManager::getInstance()->scriptCompileError("Script::optimizeJumps - Call command has no target sub name.", false);
				} else {
					int badip = getLabelIP(stringDataArray[i]);
					if (badip != -1)
					{
						scrman_currentline = 0;
						ScriptManager::getInstance()->scriptCompileError("Script::optimizeJumps - Call command target name specifies a label.", false);
						Logger::getInstance()->debug(stringDataArray[i]);
					}
					int ip = getSubIP(stringDataArray[i]);
					if (ip == -1)
					{
						if (conditionalCall)
						{
							Logger::getInstance()->debug("Script::optimizeJumps - Requested sub was not found, but this is probably a conditional call.");
						} else {
							scrman_currentline = 0;
							ScriptManager::getInstance()->scriptCompileError("Script::optimizeJumps - Requested sub was not found.", false);
							ScriptManager::getInstance()->scriptCompileError(stringDataArray[i], false);
						}
						Logger::getInstance()->debug(stringDataArray[i]);
					}
					intDataArray[i].i = ip;
				}
				conditionalCall = false;
			}
		}

		/*
		Logger::getInstance()->debug("Script::optimizeJumps - Done.");
		Logger::getInstance()->debug(name);
		*/
	}


	void Script::logMessage(ScriptProcess *sp, const char *message, int loglevel)
	{
		if (loglevel == LOGGER_LEVEL_ERROR)
			Logger::getInstance()->error(message);
		else if (loglevel == LOGGER_LEVEL_WARNING)
			Logger::getInstance()->warning(message);	
		else if (loglevel == LOGGER_LEVEL_INFO)
			Logger::getInstance()->info(message); 
		else if (loglevel == LOGGER_LEVEL_DEBUG)
			Logger::getInstance()->debug(message);	
		else
		{
			assert(0);
		}

		// TODO: based on some option, do these or skip them.
		if (loglevel != LOGGER_LEVEL_DEBUG)
		{
			if (name != NULL)
			{
				Logger::getInstance()->debug(name);
			}
			char *dbg = getDebugDump(sp, false);
			if (dbg != NULL)
			{
				Logger::getInstance()->debug(dbg);
				delete [] dbg; 
			}
		}
	}


	char *Script::getDebugDump(ScriptProcess *sp, bool fullDump)
	{
		assert(sp != NULL);

		char *buf = new char[65536*2+1];

		sprintf(buf, "\r\n");
		sprintf(&buf[strlen(buf)], "Script: %s\r\n", name);
		if (fullDump)
		{
			sprintf(&buf[strlen(buf)], "Size: %d\r\n", commandAmount);
			sprintf(&buf[strlen(buf)], "Allocated: %d\r\n", allocedSize);
			sprintf(&buf[strlen(buf)], "Process IP: %d\r\n", sp->ip);
			if (!sp->userStack->isEmpty())
				sprintf(&buf[strlen(buf)], "Process user stack: not empty\r\n");
			else
				sprintf(&buf[strlen(buf)], "Process user stack: empty\r\n");
			if (!sp->ipStack->isEmpty())
				sprintf(&buf[strlen(buf)], "Process IP stack: not empty\r\n");
			else
				sprintf(&buf[strlen(buf)], "Process IP stack: empty\r\n");
			sprintf(&buf[strlen(buf)], "Process if-depth: %d\r\n", sp->ifDepth);
		}
		sprintf(&buf[strlen(buf)], "Process last value: %d\r\n", int(sp->lastValue));
		sprintf(&buf[strlen(buf)], "Process secondary value: %d\r\n", int(sp->secondaryValue));

		int start = 0; // inclusive
		int stop = commandAmount; // exclusive
		if (!fullDump)
		{
			start = sp->ip - 10;
			stop = sp->ip + 10;
			if (start < 0) start = 0;
			if (stop > commandAmount) stop = commandAmount;
		}
		int blen = strlen(buf);
		char ind[40+2+1];
		char intv[40+1];
		char atChar;
		const char *cmd;
		const char *data;
		int indent = 0;
		bool addIndent;
		bool ignoreLastIndent;
		for (int i = start; i < stop; i++)
		{
			addIndent = false;
			ignoreLastIndent = false;
			atChar = ' ';
			if (i == sp->ip) atChar = '@';
			if (commandArray[i] < keywordsAmount)
			{
				cmd = keywords[commandArray[i]];
				if (commandArray[i] == SCRIPT_CMD_THEN
					|| commandArray[i] == SCRIPT_CMD_ELSE
					|| commandArray[i] == SCRIPT_CMD_CASE
					|| commandArray[i] == SCRIPT_CMD_CASES
					|| commandArray[i] == SCRIPT_CMD_UPTOCASE
					|| commandArray[i] == SCRIPT_CMD_DEFAULT)
				{
					ignoreLastIndent = true;
				}
				if (commandArray[i] == SCRIPT_CMD_IF
					|| commandArray[i] == SCRIPT_CMD_SELECT
					|| commandArray[i] == SCRIPT_CMD_LOOP
					|| commandArray[i] == SCRIPT_CMD_SUB)
				{
					addIndent = true;
				}
				if (commandArray[i] == SCRIPT_CMD_ENDIF
					|| commandArray[i] == SCRIPT_CMD_ENDSELECT
					|| commandArray[i] == SCRIPT_CMD_ENDLOOP
					|| commandArray[i] == SCRIPT_CMD_ENDSUB)
				{
					if (indent > 0)
						indent--;
				}
			} else {
				if (commandArray[i] < keywordsAmount + processorKeywordsAmount)
				{
					cmd = processorKeywords[commandArray[i] - keywordsAmount];
				} else {
					cmd = "(invalid command id)";
				}
			}
			data = stringDataArray[i];
			intv[0] = '\0';
			if (data == NULL)
			{
				data = "";
				if (commandArray[i] >= keywordsAmount)
				{
					if (processorDatatypes[commandArray[i] - keywordsAmount] == SCRIPT_DATATYPE_INT)
						strcpy(intv, int2str(intDataArray[i].i));
					if (processorDatatypes[commandArray[i] - keywordsAmount] == SCRIPT_DATATYPE_FLOAT)
					{
						char floatbuf[16];
						sprintf(floatbuf, "%f", *((float *)&intDataArray[i]));
						strcpy(intv, floatbuf);
					}
				} else {
					if (commandArray[i] == SCRIPT_CMD_ENDSUB)
					{
						strcpy(intv, "\r\n");
					}
					if (commandArray[i] == SCRIPT_CMD_THEN
						|| commandArray[i] == SCRIPT_CMD_ELSE)
					{
						if (intDataArray[i].i == 0)
						{
							strcpy(intv, " (never skipped)");
						} else {
							strcpy(intv, " (skip addr ");
							strcat(intv, int2str(intDataArray[i].i));
							strcat(intv, ")");
						}
					}
				}
			} else {
				if (intDataArray[i].i != 0)
				{
					strcpy(intv, " (");
					strcat(intv, int2str(intDataArray[i].i));
					strcat(intv, ")");
				}
			}
				
			ind[0] = '\0';
			for (int j = 0; j < indent; j++)
			{
				if (j >= 20 || (j == (indent - 1) && ignoreLastIndent))
					break;
				ind[j*2] = ' ';
				ind[j*2+1] = ' ';
				ind[j*2+2] = '\0';
			}
			// WARNING: unsafe sprintf!!! (overflow possible)
			sprintf(&buf[blen], "[ %.5d ]%c %s%s %s%s\r\n", i, atChar, ind, cmd, data, intv);
			blen += strlen(&buf[blen]);
			if (blen > 60000*2)
			{
				if (blen < 65000*2)
				{
					sprintf(&buf[blen], "--continues--\r\n");
				}
				break;
			}

			if (addIndent)
			{
				indent++;
			}
		}
		if (blen < 60000*2)
			sprintf(&buf[blen], "\r\n");
		return buf;
	}

	void Script::newGlobalIntVariable( const std::string& name, bool permanent )
	{
		if (Script::getGlobalVariableType(name.c_str()) != SCRIPT_DATATYPE_NONE)
		{
			//assert(!"Script::newGlobalIntVariable - Variable with given name already exists.");
			return;
		}

		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		
		VariableDataType data;
		data.dataType = SCRIPT_DATATYPE_INT;
		data.permanent = permanent;

		// Here was a nasty char pointer bug.
		// added the +1 to make it stable. Pete
		data.name = new char[ name.size() + 1 ];
		strcpy(data.name, name.c_str() );
		
		int hc;
		SCRIPT_HASHCODE_CALC( (name.c_str()), &hc);

		globalVariableHash->insert(std::pair<int, VariableDataType> (hc, data));		
	}

	void Script::newGlobalStringVariable( const std::string& name, bool permanent )
	{
		if (Script::getGlobalVariableType(name.c_str()) != SCRIPT_DATATYPE_NONE)
		{
			//assert(!"Script::newGlobalStringVariable - Variable with given name already exists.");
			return;
		}

		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		
		VariableDataType data;
		data.dataType = SCRIPT_DATATYPE_STRING;
		data.intValue = 0;
		data.stringData = NULL;
		data.permanent = permanent;
		// TODO: reset the rest of the fields...

		// Here was a nasty char pointer bug.
		// added the +1 to make it stable. Pete
		data.name = new char[ name.size() + 1 ];
		strcpy(data.name, name.c_str() );
		
		int hc;
		SCRIPT_HASHCODE_CALC( (name.c_str()), &hc);

		globalVariableHash->insert(std::pair<int, VariableDataType> (hc, data));
	}

	void Script::newGlobalArrayVariable(const std::string& name, int size, bool permanent)
	{
		if (Script::getGlobalVariableType(name.c_str()) != SCRIPT_DATATYPE_NONE)
		{
			//assert(!"Script::newGlobalArrayVariable - Variable with given name already exists.");
			return;
		}

		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		
		VariableDataType data;
		data.dataType = SCRIPT_DATATYPE_ARRAY;
		data.arraySize = size;
		data.arrayValue = new int[size];
		for (int arrinit = 0; arrinit < size; arrinit++)
		{
			data.arrayValue[arrinit] = 0;
		}
		data.permanent = permanent;

		data.name = new char[name.size() + 1];
		strcpy(data.name, name.c_str());
		
		int hc;
		SCRIPT_HASHCODE_CALC((name.c_str()), &hc);

		globalVariableHash->insert(std::pair<int, VariableDataType> (hc, data));
	}

	bool Script::setGlobalIntVariableValue(const char *variablename, int value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::setGlobalIntVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_INT)
			{
				(*iter).second.intValue = value;
				return true;
			} else {
				assert(!"Script::setGlobalIntVariableValue - Variable is not int type.");
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::getGlobalIntVariableValue(const char *variablename, int *value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalIntVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_INT)
			{
				*value = (*iter).second.intValue;
				return true;
			} else {
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	int Script::getGlobalVariableType(const char *variablename)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalVariableType - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			return (*iter).second.dataType;
			// was global, ok.
			/*
			if ((*iter).second.dataType == SCRIPT_DATATYPE_INT)
			{
				return SCRIPT_DATATYPE_INT;
			}
			if ((*iter).second.dataType == SCRIPT_DATATYPE_STRING)
			{
				return SCRIPT_DATATYPE_STRING;
			}
			if ((*iter).second.dataType == SCRIPT_DATATYPE_FLOAT)
			{
				return SCRIPT_DATATYPE_FLOAT;
			}
			if ((*iter).second.dataType == SCRIPT_DATATYPE_POSITION)
			{
				return SCRIPT_DATATYPE_POSITION;
			}
			// TODO: other types!
			*/
		}
		return SCRIPT_DATATYPE_NONE;
	}

	bool Script::setGlobalStringVariableValue(const char *variablename, const char *value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::setGlobalStringVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_STRING)
			{
				if (value != NULL
					&& (*iter).second.stringData != NULL
					&& strlen(value) <= strlen((*iter).second.stringData))
				{
					strcpy((*iter).second.stringData, value);
				} else {
					if ((*iter).second.stringData != NULL)
					{
						delete [] (*iter).second.stringData;
					}
					if (value != NULL)
					{
						(*iter).second.stringData = new char[strlen(value) + 1];					
						strcpy((*iter).second.stringData, value);
					} else {
						(*iter).second.stringData = NULL;
					}
				}
				return true;
			} else {
				assert(!"Script::setGlobalStringVariableValue - Variable is not string type.");
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::getGlobalStringVariableValue(const char *variablename, const char **value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalStringVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_STRING)
			{
				*value = (*iter).second.stringData;
				return true;
			} else {
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	void Script::newGlobalFloatVariable( const std::string& name, bool permanent )
	{
		if (Script::getGlobalVariableType(name.c_str()) != SCRIPT_DATATYPE_NONE)
		{
			//assert(!"Script::newGlobalFloatVariable - Variable with given name already exists.");
			return;
		}

		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		
		VariableDataType data;
		data.dataType = SCRIPT_DATATYPE_FLOAT;
		data.permanent = permanent;

		// Here was a nasty char pointer bug.
		// added the +1 to make it stable. Pete
		data.name = new char[ name.size() + 1 ];
		strcpy(data.name, name.c_str() );
		
		int hc;
		SCRIPT_HASHCODE_CALC( (name.c_str()), &hc);

		globalVariableHash->insert(std::pair<int, VariableDataType> (hc, data));		
	}

	bool Script::setGlobalFloatVariableValue(const char *variablename, float value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::setGlobalFloatVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_FLOAT)
			{
				(*iter).second.floatValue = value;
				return true;
			} else {
				assert(!"Script::setGlobalFloatVariableValue - Variable is not float type.");
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::getGlobalFloatVariableValue(const char *variablename, float *value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalFloatVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_FLOAT)
			{
				*value = (*iter).second.floatValue;
				return true;
			} else {
				assert(!"Script::getGlobalFloatVariableValue - Variable is not float type.");
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	void Script::newGlobalPositionVariable( const std::string& name, bool permanent )
	{
		if (Script::getGlobalVariableType(name.c_str()) != SCRIPT_DATATYPE_NONE)
		{
			//assert(!"Script::newGlobalFloatVariable - Variable with given name already exists.");
			return;
		}

		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		
		VariableDataType data;
		data.dataType = SCRIPT_DATATYPE_POSITION;
		data.permanent = permanent;

		// Here was a nasty char pointer bug.
		// added the +1 to make it stable. Pete
		data.name = new char[ name.size() + 1 ];
		strcpy(data.name, name.c_str() );
		
		int hc;
		SCRIPT_HASHCODE_CALC( (name.c_str()), &hc);

		globalVariableHash->insert(std::pair<int, VariableDataType> (hc, data));		
	}

	bool Script::setGlobalPositionVariableValue(const char *variablename, float xValue, float yValue, float zValue)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::setGlobalPositionVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_POSITION)
			{
				(*iter).second.xValue = xValue;
				(*iter).second.yValue = yValue;
				(*iter).second.zValue = zValue;
				return true;
			} else {
				assert(!"Script::setGlobalPositionVariableValue - Variable is not position type.");
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::getGlobalPositionVariableValue(const char *variablename, float *xValue, float *yValue, float *zValue)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalPositionVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_POSITION)
			{
				*xValue = (*iter).second.xValue;
				*yValue = (*iter).second.yValue;
				*zValue = (*iter).second.zValue;
				return true;
			} else {
				assert(!"Script::getGlobalPositionVariableValue - Variable is not position type.");
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::setGlobalArrayVariableValue(const char *variablename, int entry, int value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::setGlobalArrayVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_ARRAY)
			{
				if (entry >= 0 && entry < (*iter).second.arraySize)
				{
					(*iter).second.arrayValue[entry] = value;
					return true;
				} else {
					Logger::getInstance()->error("Script::setGlobalArrayVariableValue - Array index out of bounds.");
					Logger::getInstance()->debug(variablename);
					Logger::getInstance()->debug(int2str(entry));
					return false;
				}
			} else {
				assert(!"Script::setGlobalArrayVariableValue - Variable is not array type.");
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::getGlobalArrayVariableSize(const char *variablename, int *size)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalArrayVariableSize - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_ARRAY)
			{
				*size = (*iter).second.arraySize;
				return true;
			} else {
				assert(!"Script::getGlobalPositionVariableValue - Variable is not array type.");
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	bool Script::getGlobalArrayVariableValue(const char *variablename, int entry, int *value)
	{
		if (globalVariableHash == NULL)
		{
			globalVariableHash = new VariableHashType();
		}
		if (variablename == NULL)
		{
			assert(!"Script::getGlobalArrayVariableValue - Null variablename parameter.");
			return false;
		}

		int hc;
		SCRIPT_HASHCODE_CALC(variablename, &hc);

		// global variable...?
		VariableHashType::iterator iter = globalVariableHash->find(hc);
		if (iter != globalVariableHash->end())
		{
			// was global, ok.
			if ((*iter).second.dataType == SCRIPT_DATATYPE_ARRAY)
			{
				if (entry >= 0 && entry < (*iter).second.arraySize)
				{
					*value = (*iter).second.arrayValue[entry];
					return true;
				} else {
					Logger::getInstance()->error("Script::getGlobalArrayVariableValue - Array index out of bounds.");
					Logger::getInstance()->debug(variablename);
					Logger::getInstance()->debug(int2str(entry));
					return false;
				}
			} else {
				assert(!"Script::getGlobalPositionVariableValue - Variable is not array type.");
				return false;
			}
		}

		// variable was not properly defined... (did not exist)
		return false;
	}

	void Script::resetNonPermanentGlobalVariables()
	{
		VariableHashType::iterator iter = globalVariableHash->begin();

		for (; iter != globalVariableHash->end(); ++iter)
		{
			if (!(*iter).second.permanent)
			{
				(*iter).second.intValue = 0;
				(*iter).second.floatValue = 0.0f;
				(*iter).second.xValue = 0;
				(*iter).second.yValue = 0;
				(*iter).second.zValue = 0;
				if ((*iter).second.stringData != NULL)
				{
					delete [] (*iter).second.stringData;
					(*iter).second.stringData = NULL;
				}
				if ((*iter).second.arrayValue != NULL)
				{
					for (int i = 0; i < (*iter).second.arraySize; i++)
					{
						(*iter).second.arrayValue[i] = 0;
					}
				}
			}
		}
	}

	// NOTE: returns a list to _const_ char pointers...
	LinkedList *Script::getGlobalVariableList(bool permanentOnly)
	{
		LinkedList *ret = new LinkedList();

		VariableHashType::iterator iter = globalVariableHash->begin();

		for (; iter != globalVariableHash->end(); ++iter)
		{
			if ((*iter).second.permanent || !permanentOnly)
			{
				ret->append((*iter).second.name);
			}
		}

		return ret;
	}

	VariableHashType *Script::getGlobalVariableHash()
	{
		return globalVariableHash;
	}

}

