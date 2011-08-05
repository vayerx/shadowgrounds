
#include "precompiled.h"

#include "ScriptableTrackerObjectType.h"

#include "ScriptableTrackerObject.h"
#include "../scripting/GameScripting.h"


namespace game
{
namespace tracking
{
	game::GameScripting *ScriptableTrackerObjectType::gameScripting = NULL;

	ScriptableTrackerObjectType *ScriptableTrackerObjectType::trackerTypeForCurrentlyRunningScript = NULL;

	static int STOTtypeId_data = 0;
	void *ScriptableTrackerObjectType::typeId = &STOTtypeId_data;


	ScriptableTrackerObjectType::ScriptableTrackerObjectType()
	{
		tickInterval = 1000;
		influenceRadius = 0.0f;
		typesOfInterest = 0;
		allowTickBalancing = true;
		script = "";
	}

	ScriptableTrackerObjectType::~ScriptableTrackerObjectType()
	{
		// nop?
	}

	bool ScriptableTrackerObjectType::doesGiveOwnershipToObjectTracker() const
	{
		return true;
	}

	std::string ScriptableTrackerObjectType::getTrackerTypeName() const
	{
		// the script name also is used as the (hopefully unique) tracker type name.
		return this->script;
	}

	TRACKABLE_TYPEID_DATATYPE ScriptableTrackerObjectType::getTrackablesTypeOfInterest() const
	{
		return this->typesOfInterest;
	}

	void ScriptableTrackerObjectType::setScript(const std::string &script)
	{
		this->script = script;
	}

	
	void ScriptableTrackerObjectType::doSelfInit()
	{
		if (!this->script.empty())
		{
			assert(gameScripting != NULL);

			int ret = 0;

			// maybe this was called from a scriptable tracker type's script (would be a bad bad idea)
			assert(trackerTypeForCurrentlyRunningScript == NULL);
			if (trackerTypeForCurrentlyRunningScript != NULL)
			{
				// bad bad...
				return;
			}

			trackerTypeForCurrentlyRunningScript = this;
			ret = gameScripting->runOtherScript(this->script.c_str(), "get_tick_interval", NULL, VC3(0,0,0));
			tickInterval = ret;

			// TODO: in reality, would need a float value returning script run...
			trackerTypeForCurrentlyRunningScript = this;
			ret = gameScripting->runOtherScript(this->script.c_str(), "get_area_of_interest_radius", NULL, VC3(0,0,0));
			influenceRadius = (float)ret;

			trackerTypeForCurrentlyRunningScript = this;
			ret = gameScripting->runOtherScript(this->script.c_str(), "get_types_of_interest", NULL, VC3(0,0,0));
			typesOfInterest = ret;

			trackerTypeForCurrentlyRunningScript = this;
			ret = gameScripting->runOtherScript(this->script.c_str(), "get_allow_tick_balancing", NULL, VC3(0,0,0));
			allowTickBalancing = ret != 0 ? true : false;

			trackerTypeForCurrentlyRunningScript = this;
			ret = gameScripting->runOtherScript(this->script.c_str(), "create_variables", NULL, VC3(0,0,0));
			// ignore ret... the script should have modified this object automagically...

			assert(trackerTypeForCurrentlyRunningScript == this);
			trackerTypeForCurrentlyRunningScript = NULL;

		} else {
			LOG_WARNING("ScriptableTrackerObjectType::doSelfInit - Cannot initialize self, as given script name is empty.");
		}
	}

	void ScriptableTrackerObjectType::setTickInterval(int tickInterval)
	{
		this->tickInterval = tickInterval;
	}

	void ScriptableTrackerObjectType::setAllowTickBalancing(bool allowBalancing)
	{
		this->allowTickBalancing = allowBalancing;
	}

	void ScriptableTrackerObjectType::setAreaOfInterestRadius(float radius)
	{
		this->influenceRadius = radius;
	}

	int ScriptableTrackerObjectType::getTickInterval() const
	{
		return this->tickInterval;
	}

	bool ScriptableTrackerObjectType::doesAllowTickBalancing() const
	{
		return this->allowTickBalancing;
	}

	float ScriptableTrackerObjectType::getAreaOfInterestRadius() const
	{
		return this->influenceRadius;
	}

	ITrackerObject *ScriptableTrackerObjectType::createNewObjectInstance()
	{
		return new ScriptableTrackerObject(this);
	}

	void ScriptableTrackerObjectType::setGameScripting(GameScripting *gameScripting)
	{
		ScriptableTrackerObjectType::gameScripting = gameScripting;
	}

	void ScriptableTrackerObjectType::addTypesOfInterest(TRACKABLE_TYPEID_DATATYPE trackableTypes)
	{
		this->typesOfInterest |= trackableTypes;
	}

	void ScriptableTrackerObjectType::removeTypesOfInterest(TRACKABLE_TYPEID_DATATYPE trackableTypes)
	{
		this->typesOfInterest &= (TRACKABLE_TYPEID_ALL_TYPES_MASK ^ trackableTypes);
	}

	void ScriptableTrackerObjectType::setScriptableTrackerVariables(const std::vector<std::string> &variableNameList)
	{
		this->variableNames = variableNameList;
	}

	void ScriptableTrackerObjectType::addScriptableTrackerVariable(const std::string &variableName)
	{
		this->variableNames.push_back(variableName);
	}

}
}


