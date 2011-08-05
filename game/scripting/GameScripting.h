
#ifndef GAMESCRIPTING_H
#define GAMESCRIPTING_H

#include "../../util/IScriptProcessor.h"
#include "../../util/ScriptProcess.h"
#include "GameScriptData.h"

#include "../physics/IGamePhysicsScriptRunner.h"
#include "../../util/ITriggerListener.h"


namespace util
{
  class ScriptProcess;
	class CircleAreaTracker;
}

namespace game
{
  class Game;
  class Unit;
	class MiscScripting;
	class Bullet;
	namespace tracking
	{
		class ITrackableObjectIterator;
	}
  
  class GameScripting : private util::IScriptProcessor, private util::ITriggerListener,
		public IGamePhysicsScriptRunner
  {
  public:
    GameScripting(Game *game);

		~GameScripting();

    virtual bool process(util::ScriptProcess *sp, int command, floatint intFloat,
      char *stringData, ScriptLastValueType *lastValue);

    void loadScripts(const char *filename, const char *relativeToFilenamePath);

    void loadMemoryScripts(const char *memoryFilename, char *buf, int buflen);

    void runScriptProcess(util::ScriptProcess *sp, bool pausable);

    util::ScriptProcess *startUnitScript(Unit *unit, const char *script, const char *sub, const std::vector<int> *paramStack = NULL);

    util::ScriptProcess *startNonUnitScript(const char *script, const char *sub, const std::vector<int> *paramStack = NULL);

		void deleteGameScript(util::ScriptProcess *sp);

    void makeAlert(Unit *unit, int distance, const VC3 &position);
    void runHitScript(Unit *unit, Unit *shooter, Bullet *hitBulletType);
    bool runExecuteScript(Unit *unit, Unit *shooter);
    void runSpottedScript(Unit *unit, Unit *spotted);
    void runHitMissScript(Unit *unit, Unit *shooter);
    void runPointedScript(Unit *unit, Unit *shooter);
    void runEventScript(Unit *unit, const char *event);
    void runHearNoiseScript(Unit *unit, Unit *noisy);

    void runItemUseScript(Unit *unit, Item *item);
    void runItemPickupScript(Unit *unit, Item *item);
    void runItemProgressBarScript(Item *item, Unit *unit, const char *subName);
    bool runItemExecuteScript(Unit *unit, Item *item);

    void runMissionScript(const char *scriptname, const char *subname);

    int runOtherScript(const char *scriptname, const char *subname, Unit *unit, const VC3 &position);
    int runOtherScriptForUnifiedHandle(const char *scriptname, const char *subname, UnifiedHandle uh, const VC3 &position);

    int runTrackerScript(const char *scriptname, const char *subname, UnifiedHandle trackerUnifiedHandle, const std::vector<int> *params = NULL);

		void runHitChainScript(const char *scriptname, Projectile *origin, 
			Unit *hitUnit, Unit *shooter, Bullet *chainBullet,
			const VC3 &position, int hitchain, const VC3 &direction, const VC3 &hitPlaneNormal);

		void newGlobalIntVariable( const char* variablename, bool permanent );
		void setGlobalIntVariableValue(const char *variablename, int value);
		int getGlobalIntVariableValue(const char *variablename);

		// delete the returned array once done with it.
		char *matchSuitableCommands(util::ScriptProcess *sp, int *matches, const char *command,
			int *smallestMatchLength);

		void updateAreaTracker();

		void addAreaTrigger(Unit *unit, const VC3 &position, float range, int clipMask, const LinkedList *unitToTrack);

		virtual void activate(int circleId, void *data);

		virtual bool runGamePhysicsScript(const char *scriptname, const char *subname);
		virtual void *getGamePhysicsScriptRunnerImplementation();

		// methods for transition to other scripting languages or for effective simple runtime script command running
		void runSingleCommand(int command, floatint intFloat, const char *stringData, int *lastValue, int *secondaryValue, GameScriptData *gsd);
		void runSingleSimpleCommand(int command, floatint intFloat, const char *stringData, int *lastValue = 0, int *secondaryValue = 0);
		bool runSingleSimpleStringCommand(const char *command, const char *param, int *lastValue = 0, int *secondaryValue = 0);
		void runMultipleSimpleCommands(int commandAmount, int *command, floatint *intFloat, const char **stringData, int *lastValue = 0, int *secondaryValue = 0);
		bool runMultipleSimpleStringCommands(int commandAmount, const char **command, const char **param, int *lastValue = 0, int *secondaryValue = 0);


  private:
    Game *game;

    bool runItemScriptImpl(Unit *unit, Item *item, const char *subName, bool ignoreMissingSub);

		bool itemMarkedForRemove;
		bool itemMarkedForDisable;
		int itemMarkedForDisableTime;

		util::CircleAreaTracker *areaTracker;

		void *gamePhysicsScriptRunnerImplementation;

		// to access itemMarkedForRemove
		friend class ItemScripting;

  };

}

#endif

