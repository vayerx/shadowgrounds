
#ifndef SCRIPTABLETRACKEROBJECT_H
#define SCRIPTABLETRACKEROBJECT_H

#include "ITrackerObject.h"
#include "ScriptableTrackerObjectType.h"
#include "../unified_handle_type.h"
#include <string>

namespace game
{
namespace tracking
{
	class ScriptableTrackerObjectType;

	class ScriptableTrackerObject : public ITrackerObject
	{
	public:
		ScriptableTrackerObject(ScriptableTrackerObjectType *type);
		~ScriptableTrackerObject();

		void initVariableValues();
		void setUnifiedHandle(UnifiedHandle uh);
		//UnifiedHandle getUnifiedHandle() const;

		virtual ITrackerObjectType *getType();

		virtual void tick();

		virtual void setTrackablePosition(const VC3 &globalPosition);

		virtual void setTrackableRotation(const QUAT &rotation);

		virtual void setTrackableVelocity(const VC3 &velocity);

		virtual void lostTracked();

		virtual void trackerSignal(int trackerSignalNumber);

		virtual void trackerDeleted() { }

		virtual void attachedToTrackable(ITrackableObject *trackable);

		virtual void setTrackerPosition(const VC3 &position);

		virtual VC3 getTrackerPosition() const;

		virtual void iterateTrackables(ITrackableObjectIterator *iter);

		void setVariable(int variableNumber, int value);
		int getVariable(int variableNumber);
		int getVariableNumberByName(const std::string &variableName);

		// NOTE: far from multithread safe.
		static UnifiedHandle trackerForCurrentlyRunningScript;
		static ITrackableObjectIterator *trackableIteratorForCurrentlyRunningScript;

	private:
		VC3 position;
		ScriptableTrackerObjectType *type;
		std::vector<int> variables;
		UnifiedHandle unifiedHandle;
		bool initedVariableValues;

		//friend class ScriptableTrackerObjectType;
	};
}
}

#endif

