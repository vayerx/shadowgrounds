
#include "precompiled.h"

#include "ScriptableTrackerObject.h"

#include "ScriptableTrackerObjectType.h"
#include "ITrackableObjectIterator.h"
#include "ITrackableObject.h"
#include "TrackableUnifiedHandleObject.h"
#include "../scripting/GameScripting.h"

#include "../unified_handle.h"

namespace game
{
namespace tracking
{

	UnifiedHandle ScriptableTrackerObject::trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;
	ITrackableObjectIterator *ScriptableTrackerObject::trackableIteratorForCurrentlyRunningScript = NULL;


	ScriptableTrackerObject::ScriptableTrackerObject(ScriptableTrackerObjectType *type)
	{
		this->position = VC3(0,0,0);
		this->type = type;
		this->unifiedHandle = UNIFIED_HANDLE_NONE;
		for (int i = 0; i < (int)type->variableNames.size(); i++)
		{
			this->variables.push_back(0);
		}
		this->initedVariableValues = false;
	}

	ScriptableTrackerObject::~ScriptableTrackerObject()
	{
		// nop?
	}

	void ScriptableTrackerObject::setVariable(int variableNumber, int value)
	{
		assert(this->initedVariableValues);
		assert(variableNumber >= 0 && variableNumber < (int)variables.size());
		variables[variableNumber] = value;
	}

	int ScriptableTrackerObject::getVariable(int variableNumber)
	{
		assert(this->initedVariableValues);
		assert(variableNumber >= 0 && variableNumber < (int)variables.size());
		return variables[variableNumber];
	}

	int ScriptableTrackerObject::getVariableNumberByName(const std::string &variableName)
	{
		for (int i = 0; i < (int)this->type->variableNames.size(); i++)
		{
			if (this->type->variableNames[i] == variableName)
			{
				if (this->type->variableNames.size() == variables.size())
				{
					return i;
				} else {
					LOG_ERROR("ScriptableTrackerObject::getVariableNumberByName - Scriptable tracker object type has different amount of variables than its instance.");
					return -1;
				}
			}
		}
		LOG_WARNING_W_DEBUG("ScriptableTrackerObject::getVariableNumberByName - No variable found with given name.", variableName.c_str());
		return -1;
	}

	ITrackerObjectType *ScriptableTrackerObject::getType()
	{
		return this->type;
	}

	void ScriptableTrackerObject::tick()
	{
		assert(trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE);

		trackerForCurrentlyRunningScript = this->unifiedHandle;
		ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "tick", this->unifiedHandle);		
		trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;
	}

	void ScriptableTrackerObject::trackerSignal(int trackerSignalNumber)
	{
		assert(trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE);

		trackerForCurrentlyRunningScript = this->unifiedHandle;
		std::vector<int> params;
		params.push_back(trackerSignalNumber);
		ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "signal", this->unifiedHandle, &params);
		trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;
	}

	void ScriptableTrackerObject::setTrackablePosition(const VC3 &globalPosition)
	{
		this->position = globalPosition;
	}

	void ScriptableTrackerObject::setTrackableRotation(const QUAT &rotation)
	{
		// nop?
	}

	void ScriptableTrackerObject::setTrackableVelocity(const VC3 &velocity)
	{
		// nop?
	}

	void ScriptableTrackerObject::lostTracked()
	{
		assert(trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE);

		trackerForCurrentlyRunningScript = this->unifiedHandle;
		ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "lost_tracked", this->unifiedHandle);
		trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;
	}

	void ScriptableTrackerObject::attachedToTrackable(ITrackableObject *trackable)
	{
		assert(trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE);

		trackerForCurrentlyRunningScript = this->unifiedHandle;
		if (trackable != NULL)
		{
			ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "attached", this->unifiedHandle);
		} else {
			ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "detached", this->unifiedHandle);
		}
		trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;
	}

	void ScriptableTrackerObject::setTrackerPosition(const VC3 &position)
	{
		this->position = position;
	}

	VC3 ScriptableTrackerObject::getTrackerPosition() const
	{
		return position;
	}

	void ScriptableTrackerObject::iterateTrackables(ITrackableObjectIterator *iter)
	{
		if (iter == NULL)
		{
			assert(!"ScriptableTrackerObject::iterateTrackables - null iterator parameter.");
			return;
		}

		assert(trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE);
		assert(trackableIteratorForCurrentlyRunningScript == NULL);

		assert(this->unifiedHandle != UNIFIED_HANDLE_NONE);

		trackerForCurrentlyRunningScript = this->unifiedHandle;
		trackableIteratorForCurrentlyRunningScript = iter;

		ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "iterate_trackables", this->unifiedHandle);

		trackableIteratorForCurrentlyRunningScript = NULL;
		trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;

		/*
		while (iter->iterateAvailable())
		{
			ITrackableObject *trackable = iter->iterateNext();

			assert(trackable->getTypeId() == TrackableUnifiedHandleObject::typeId);

			if (trackable->getTypeId() == TrackableUnifiedHandleObject::typeId)
			{
				// WARNING: unsafe cast, based on the check above...
				()
			}
		}
		*/
	}

	void ScriptableTrackerObject::setUnifiedHandle(UnifiedHandle uh)
	{
		// assuming this is called only once.
		assert(this->unifiedHandle == UNIFIED_HANDLE_NONE);

		this->unifiedHandle = uh;
	}

	void ScriptableTrackerObject::initVariableValues()
	{
		assert(!this->initedVariableValues);
		this->initedVariableValues = true;
		assert(trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE);
		trackerForCurrentlyRunningScript = this->unifiedHandle;
		ScriptableTrackerObjectType::gameScripting->runTrackerScript(this->type->script.c_str(), "init_variable_values", this->unifiedHandle);
		trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;
	}

}
}
