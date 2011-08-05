
#ifndef IAIDIRECTCONTROL_H
#define IAIDIRECTCONTROL_H

#include "AIDirectControlActions.h"

namespace game
{
	class Unit;
	class UnitActor;

  /**
	 * Interface for direct AI control of units.
	 * 
	 * @author Jukka Kokkonen <jukka.kokkonen@postiloota.net>
	 */

	class IAIDirectControl
	{
	public:
		virtual ~IAIDirectControl() { }

		virtual void doDirectControls(AIDirectControlActions &actionsOut) = 0;

		/**
		 * This method should return a pointer to the name of the AI direct control class. 
		 * (such as "scriptable" in case of ScriptableAIDirectControl) 
		 * @return name of the AI direct control class. Null value should be never returned.
		 */
		virtual const char *getDirectControlName() = 0;

	};
}

#endif

