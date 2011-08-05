
#ifndef SCRIPTABLETRACKEROBJECTTYPE_H
#define SCRIPTABLETRACKEROBJECTTYPE_H

#include "ITrackerObjectType.h"
#include <string>

namespace game
{
	class GameScripting;

namespace tracking
{
	class ScriptableTrackerObject;
	class ITrackableObject;
	class ITrackerObject;

	class ScriptableTrackerObjectType : public ITrackerObjectType
	{
	public:
		ScriptableTrackerObjectType();

		~ScriptableTrackerObjectType();

		virtual void *getTypeId() const { return ScriptableTrackerObjectType::typeId; }

		virtual bool doesGiveOwnershipToObjectTracker() const;

		void setScript(const std::string &script);

		void doSelfInit();

		void setTickInterval(int tickInterval);

		void setAllowTickBalancing(bool allowBalancing);

		void setAreaOfInterestRadius(float radius);

		void addTypesOfInterest(TRACKABLE_TYPEID_DATATYPE trackableTypes);

		void removeTypesOfInterest(TRACKABLE_TYPEID_DATATYPE trackableTypes);

		void setScriptableTrackerVariables(const std::vector<std::string> &variableNameList);
		void addScriptableTrackerVariable(const std::string &variableName);

		virtual std::string getTrackerTypeName() const;

		virtual TRACKABLE_TYPEID_DATATYPE getTrackablesTypeOfInterest() const;

		virtual int getTickInterval() const;

		virtual bool doesAllowTickBalancing() const;

		virtual float getAreaOfInterestRadius() const;

		virtual ITrackerObject *createNewObjectInstance();

		static void setGameScripting(GameScripting *gameScripting);

		static ScriptableTrackerObjectType *trackerTypeForCurrentlyRunningScript;

		static void *typeId;

	private:
		int tickInterval;
		bool allowTickBalancing;
		float influenceRadius;
		std::string script;
		TRACKABLE_TYPEID_DATATYPE typesOfInterest;

		static game::GameScripting *gameScripting;

		std::vector<std::string> variableNames;

		friend class ScriptableTrackerObject;
	};

}
}

#endif

