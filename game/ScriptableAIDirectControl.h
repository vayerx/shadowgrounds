
#ifndef SCRIPTABLEAIDIRECTCONTROL_H
#define SCRIPTABLEAIDIRECTCONTROL_H

#include "IAIDirectControl.h"

namespace game
{
	class Game;
	class Unit;
	class UnitActor;

  /**
	 * A class for units controlled directly by scripts.
	 * (Directly AI controlled unit acts as if it was controlled by a player keybinds, but it is rather 
	 * controlled by AI decision "keybinds")
	 * 
	 * @author Jukka Kokkonen <jukka.kokkonen@postiloota.net>
	 */

	class ScriptableAIDirectControl : public IAIDirectControl
	{

	private:
		// allow only the UnitSpawner to construct this. (for no good reason ;)
		ScriptableAIDirectControl(Game *game, Unit *unit);

	public:
		typedef enum
		{
			EVENT_MASK_INVALID = 0,

			EVENT_MASK_TICK = (1<<0),
			EVENT_MASK_TIMER = (1<<1),

			EVENT_MASK_FALL = (1<<2),
			EVENT_MASK_TOUCHDOWN = (1<<3),

			// (these are sensible to sideways gameplay only)
			EVENT_MASK_OBJECT_TOUCH_LEFT = (1<<4),
			EVENT_MASK_OBJECT_TOUCH_RIGHT = (1<<5),
			EVENT_MASK_OBJECT_TOUCH_BELOW = (1<<6),
			EVENT_MASK_OBJECT_TOUCH_ABOVE = (1<<7),

			EVENT_MASK_DROP_ON_LEFT = (1<<8),
			EVENT_MASK_DROP_ON_RIGHT = (1<<9),

			// (these are sensible to sideways gameplay only)
			EVENT_MASK_UNIT_TOUCH_LEFT = (1<<10),
			EVENT_MASK_UNIT_TOUCH_RIGHT = (1<<11),
			EVENT_MASK_UNIT_TOUCH_BELOW = (1<<12),
			EVENT_MASK_UNIT_TOUCH_ABOVE = (1<<13),

			_EVENT_MASK_DUMMY = 0x7fffffff // (don't use this value).

		} EVENT_MASK;

		static const int NUM_EVENT_MASK = 14;


		typedef enum
		{
			AIM_MODE_INVALID = 0,

			AIM_MODE_OFFSET_SELF,
			AIM_MODE_ABSOLUTE,
			AIM_MODE_OFFSET_UNIT_TARGET,
			AIM_MODE_OFFSET_TARGET_UNIFIED_HANDLE,

			NUM_AIM_MODE
		} AIM_MODE;

		virtual void doDirectControls(AIDirectControlActions &actionsOut);

		virtual const char *getDirectControlName() { return "scriptable"; }

		void enableEvent(EVENT_MASK eventMask);
		void disableEvent(EVENT_MASK eventMask);

		void enableAction(int directCtrl);
		void disableAction(int directCtrl);

		void setActionsToDisableAutomatically(const AIDirectControlActions &actionsToDisable);
		void addActionToDisableAutomatically(int directControl);
		void removeActionToDisableAutomatically(int directControl);

		void setTimerEventParameters(int ticksPerTimerEvent);

		void setAimMode(AIM_MODE aimMode);
		AIM_MODE getAimMode() { return aimMode; }
		void setAimPosition(const VC3 &position);

		// returns the aim mode for given name or AIM_MODE_INVALID
		static AIM_MODE getAimModeByName(const char *name);
		// returns the event mask for given name or EVENT_MASK_INVALID
		static EVENT_MASK getEventMaskByName(const char *name);

	private:
		Game *game;
		Unit *unit;

		int eventMask;

		int previousEventsOnMask;
		AIDirectControlActions actions;
		AIDirectControlActions actionsToDisableAutomatically;

		AIM_MODE aimMode;
		VC3 aimPosition;
		VC3 aimPositionResult;

		int timerTicksAmount;
		int lastTimerCall;

		friend class UnitSpawner;
	};
}

#endif

