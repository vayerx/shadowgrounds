// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#include "ScriptProcess.h"

#include <assert.h>
#include "Script.h"
#include "../system/Logger.h"
#include "../util/Debug_MemoryManager.h"


#define INITIAL_CALL_PARAM_STACK_ALLOC 16
#define MAX_CALL_PARAM_STACK_USE 65530

#define INITIAL_LOCAL_VAR_STACK_ALLOC 16
#define MAX_LOCAL_VAR_STACK_USE 65530

#define INITIAL_SCOPE_STACK_ALLOC 8
#define MAX_SCOPE_STACK_USE 8190


namespace util
{

	static int next_script_process_id = SCRIPTPROCESS_MIN_ID;

 
	ScriptProcess::ScriptProcess()
	{
		ipStack = new LinkedList();
		userStack = new LinkedList();
		userStackSize = 0;
		script = NULL;
		ip = 0;
		lastValue = 0;
		finished = false;
		ifDepth = 0;
		thenBranch = false;
		data = NULL;
		secondaryValue = 0;
		misbehaveCounter = 0;

		pid = next_script_process_id;
		// TODO: should check for next _free_ pid, not just assume the next one is free.
		next_script_process_id++;
		if (next_script_process_id > SCRIPTPROCESS_MAX_ID)
		{
			next_script_process_id = SCRIPTPROCESS_MIN_ID;
		}

		callParamStack = NULL;
		callParamStackUsed = 0;
		callParamStackAlloced = 0;

		localVarStack = new int[INITIAL_LOCAL_VAR_STACK_ALLOC];
		localVarStackUsed = 0;
		localVarStackAlloced = INITIAL_LOCAL_VAR_STACK_ALLOC;

		scopeStack = new int[INITIAL_SCOPE_STACK_ALLOC];
		scopeStackUsed = 0;
		scopeStackAlloced = INITIAL_SCOPE_STACK_ALLOC;

	}

	ScriptProcess::~ScriptProcess()
	{
		if (this->finished)
		{
			if (!ipStack->isEmpty())
			{
				Logger::getInstance()->warning("ScriptProcess - IP stack not empty at delete although process has finished.");
			}
		}
		while (!ipStack->isEmpty())
		{
			// notice, the pop result is actually a int, so no need to delete it
			// if it would be a pointer to an object, should delete it.
			ipStack->popLast();
		}
		delete ipStack;

		if (this->finished)
		{
			if (!userStack->isEmpty())
			{
				Logger::getInstance()->warning("ScriptProcess - User stack not empty at delete although process has finished.");

				// FIXME: this "empty message queue"-loop managed to loop forever.
				// reason unknown. (and since this is not so important, just ignoring that)
				//MSG msg = { 0 };
				//while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE));

				//assert(!"ScriptProcess - User stack not empty at delete although process has finished.");
				// notice, the pop result is actually a int, so no need to delete it
				// if it would be a pointer to an object, should delete it.
			}
		}

		int internalStackCheck = 0;
		while (!userStack->isEmpty())
		{
			userStack->popLast();
			internalStackCheck++;
		}
		delete userStack;
		assert(internalStackCheck == this->userStackSize);

		if (scopeStack != NULL)
		{
			delete [] scopeStack;
		}
		if (localVarStack != NULL)
		{
			delete [] localVarStack;
		}
		if (callParamStack != NULL)
		{
			delete [] callParamStack;
		}
	}

	void ScriptProcess::error(const char *message)
	{
		assert(script != NULL);
		script->logMessage(this, message, LOGGER_LEVEL_ERROR);
	}


	void ScriptProcess::warning(const char *message)
	{
		assert(script != NULL);
		script->logMessage(this, message, LOGGER_LEVEL_WARNING);
	}


	void ScriptProcess::debug(const char *message)
	{
		assert(script != NULL);
		script->logMessage(this, message, LOGGER_LEVEL_DEBUG);
	}

	bool ScriptProcess::isUserStackEmpty()
	{
		return userStack->isEmpty();
	}

	void ScriptProcess::copyFrom(ScriptProcess *otherScriptProcess)
	{
		if (otherScriptProcess == NULL)
		{
			assert(!"ScriptProcess::copyFrom - Other script process parameter is null.");
			return;
		}
		if (this->script != otherScriptProcess->script)
		{
			// actually, this should not be a problem, just copy the
			// script pointer too...
			assert(!"ScriptProcess::copyFrom - Cannot copy from script process that is running another script.");
			return;
		}

		this->ip = otherScriptProcess->ip;
		this->lastValue = otherScriptProcess->lastValue;
		{
			LinkedListIterator iter(otherScriptProcess->ipStack);
			while (iter.iterateAvailable())
			{
				// WARNING: unsafe cast - is this really an int??? (i think it is :)
				intptr_t val = (intptr_t)iter.iterateNext();
				this->ipStack->append((void *)val);
			}
		}
		{
			LinkedListIterator iter(otherScriptProcess->userStack);
			while (iter.iterateAvailable())
			{
				// WARNING: unsafe cast - is this really an int??? (i think it is :)
				intptr_t val = (intptr_t)iter.iterateNext();
				this->userStack->append((void *)val);
			}
			this->userStackSize = otherScriptProcess->userStackSize;
		}
		this->finished = otherScriptProcess->finished;
		this->secondaryValue = otherScriptProcess->secondaryValue;
		this->misbehaveCounter = otherScriptProcess->misbehaveCounter;
		
		this->ifDepth = otherScriptProcess->ifDepth;
		this->thenBranch = otherScriptProcess->thenBranch;

		if (callParamStack != NULL)
		{
			this->callParamStack = new int[otherScriptProcess->callParamStackAlloced];
			this->callParamStackAlloced = otherScriptProcess->callParamStackAlloced;
			this->callParamStackUsed = otherScriptProcess->callParamStackUsed;
			for (int i = 0; i < this->callParamStackUsed; i++)
			{
				this->callParamStack[i] = otherScriptProcess->callParamStack[i];
			}
		} else {
			this->callParamStack = NULL;
			this->callParamStackUsed = 0;
			this->callParamStackAlloced = 0;
		}

		assert(localVarStack != NULL);
		this->localVarStack = new int[otherScriptProcess->localVarStackAlloced];
		this->localVarStackAlloced = otherScriptProcess->localVarStackAlloced;
		this->localVarStackUsed = otherScriptProcess->localVarStackUsed;
		for (int i = 0; i < this->localVarStackUsed; i++)
		{
			this->localVarStack[i] = otherScriptProcess->localVarStack[i];
		}

		assert(scopeStack != NULL);
		this->scopeStack = new int[otherScriptProcess->scopeStackAlloced];
		this->scopeStackAlloced = otherScriptProcess->scopeStackAlloced;
		this->scopeStackUsed = otherScriptProcess->scopeStackUsed;
		for (int i = 0; i < this->scopeStackUsed; i++)
		{
			this->scopeStack[i] = otherScriptProcess->scopeStack[i];
		}
	}

	bool ScriptProcess::isCallParamStackEmpty()
	{
		if (this->callParamStackUsed == 0)
			return true;
		else
			return false;
	}

	void ScriptProcess::pushCallParamStack(int value)
	{
		if (this->callParamStackUsed >= this->callParamStackAlloced)
		{
			if (this->callParamStackUsed >= MAX_CALL_PARAM_STACK_USE)
			{
				Logger::getInstance()->error("ScriptProcess::pushCallParamStack - Allowed maximum stack size reached.");
				assert(!"ScriptProcess::pushCallParamStack - Allowed maximum stack size reached.");
				return;
			}
			int *prevStack = this->callParamStack;
			if (this->callParamStackAlloced == 0)
			{
				this->callParamStackAlloced = INITIAL_CALL_PARAM_STACK_ALLOC;
			} else {
				this->callParamStackAlloced *= 2;
			}
			this->callParamStack = new int[this->callParamStackAlloced];
			if (prevStack != NULL)
			{
				for (int i = 0; i < this->callParamStackUsed; i++)
				{
					this->callParamStack[i] = prevStack[i];
				}
				delete [] prevStack;
			}
		}
		this->callParamStack[this->callParamStackUsed] = value;
		this->callParamStackUsed++;
	}

	int ScriptProcess::popCallParamStack()
	{
		if (this->callParamStackUsed > 0)
		{
			this->callParamStackUsed--;
			return this->callParamStack[this->callParamStackUsed];
		} else {
			Logger::getInstance()->error("ScriptProcess::popCallParamStack - Attempt to pop from empty stack.");
			assert(!"ScriptProcess::popCallParamStack - Attempt to pop from empty stack.");
			return 0;
		}
	}

	bool ScriptProcess::isLocalVarStackEmpty()
	{
		if (this->localVarStackUsed == 0)
			return true;
		else
			return false;
	}

	void ScriptProcess::pushLocalVarStack(int value)
	{
		if (this->localVarStackUsed >= this->localVarStackAlloced)
		{
			if (this->localVarStackUsed >= MAX_LOCAL_VAR_STACK_USE)
			{
				Logger::getInstance()->error("ScriptProcess::pushLocalVarStack - Allowed maximum stack size reached.");
				assert(!"ScriptProcess::pushLocalVarStack - Allowed maximum stack size reached.");
				return;
			}
			int *prevStack = this->localVarStack;
			assert(this->localVarStackAlloced > 0);
			this->localVarStackAlloced *= 2;
			this->localVarStack = new int[this->localVarStackAlloced];
			if (prevStack != NULL)
			{
				for (int i = 0; i < this->localVarStackUsed; i++)
				{
					this->localVarStack[i] = prevStack[i];
				}
				delete [] prevStack;
			}
		}
		this->localVarStack[this->localVarStackUsed] = value;
		this->localVarStackUsed++;
	}

	int ScriptProcess::popLocalVarStack()
	{
		if (this->localVarStackUsed > 0)
		{
			this->localVarStackUsed--;
			return this->localVarStack[this->localVarStackUsed];
		} else {
			Logger::getInstance()->error("ScriptProcess::popLocalVarStack - Attempt to pop from empty stack.");
			assert(!"ScriptProcess::popLocalVarStack - Attempt to pop from empty stack.");
			return 0;
		}
	}

	int ScriptProcess::getLocalVarStackSize()
	{
		return localVarStackUsed;
	}

	int ScriptProcess::getLocalVarStackEntryValue(int index)
	{
		assert(index >= 0 && index < localVarStackUsed);
		return localVarStack[index];
	}

	void ScriptProcess::setLocalVarStackEntryValue(int index, int value)
	{
		assert(index >= 0 && index < localVarStackUsed);
		localVarStack[index] = value;
	}


	void ScriptProcess::enterLocalScope()
	{
		if (this->scopeStackUsed >= this->scopeStackAlloced)
		{
			if (this->scopeStackUsed >= MAX_SCOPE_STACK_USE)
			{
				Logger::getInstance()->error("ScriptProcess::enterLocalScope - Allowed maximum stack size reached.");
				assert(!"ScriptProcess::enterLocalScope - Allowed maximum stack size reached.");
				return;
			}
			int *prevStack = this->scopeStack;
			assert(this->scopeStackAlloced != 0);
			this->scopeStackAlloced *= 2;
			this->scopeStack = new int[this->scopeStackAlloced];
			if (prevStack != NULL)
			{
				for (int i = 0; i < this->scopeStackUsed; i++)
				{
					this->scopeStack[i] = prevStack[i];
				}
				delete [] prevStack;
			}
		}
		this->scopeStack[this->scopeStackUsed] = this->localVarStackUsed;
		this->scopeStackUsed++;
	}

	void ScriptProcess::leaveLocalScope()
	{
		if (this->scopeStackUsed > 0)
		{
			this->scopeStackUsed--;			
			int popToUsed = this->scopeStack[this->scopeStackUsed];
			if (this->localVarStackUsed >= popToUsed)
			{
				this->localVarStackUsed = popToUsed;
			} else {
				Logger::getInstance()->error("ScriptProcess::leaveLocalScope - Local variable stack used less than it should be.");
				assert(!"ScriptProcess::leaveLocalScope - Local variable stack used less than it should be.");
			}
		} else {
			Logger::getInstance()->error("ScriptProcess::leaveLocalScope - Attempt to pop from empty scope stack.");
			assert(!"ScriptProcess::leaveLocalScope - Attempt to pop from empty scope stack.");
		}
	}


}


