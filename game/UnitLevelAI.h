
#ifndef UNITLEVELAI_H
#define UNITLEVELAI_H

#include "gamedefs.h"


#define UNITLEVELAI_EVENT_MASK_FIRE_COMPLETE 1
#define UNITLEVELAI_EVENT_MASK_JUMP_START 2
#define UNITLEVELAI_EVENT_MASK_JUMP_END 4
#define UNITLEVELAI_EVENT_MASK_JUMP_CONTINUE 8
#define UNITLEVELAI_EVENT_MASK_RESERVED4 16


namespace util
{
  class ScriptProcess;
}

namespace game
{
  class Game;
  class Unit;

	// for debugging...
	class DevScripting;

  class UnitLevelAI
  {
  public:
    UnitLevelAI(Game *game, Unit *unit);
    ~UnitLevelAI();
    void runUnitAI();
    void reScriptMain();
    void prepareMainScript();

		// re-script main script once the current process has
		// finished.
		void requestReScriptMain();

		void setEnabled(bool aiEnabled, bool force_hitscript_enabled=false);

		void setTempDisabled(bool tempDisabled);

		static void setAllEnabled(bool allEnabled);

		static void setPlayerAIEnabled(int player, bool enabled);

		bool isThisAndAllEnabled();

		void skipMainScriptWait();

		void terminateMainScript();

		void addEventListener(int eventMask);

		void copyStateFrom(UnitLevelAI *otherAI);

		bool isScriptProcessMainScriptProcess(util::ScriptProcess *sp);

		void setRunContinueJumpEvent() { runContinueJumpEvent = true; }

  private:
    game::Game *game;
    Unit *unit;
    util::ScriptProcess *mainScriptProcess;
		bool enabled;
		bool force_hitscript_enabled;
		bool tempDisabled;
		bool rescriptRequested;

		int eventMask;

		// for jump event handling...
		int lastSpeed;
		bool runContinueJumpEvent;

		static bool allEnabled;

		static bool playerAIEnabled[ABS_MAX_PLAYERS];

		friend class DevScripting;
  };

}

#endif

