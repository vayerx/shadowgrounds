
#ifndef GAME_H
#define GAME_H

//
// The game.
//

#include <c2_vectors.h>

#include "../container/LinkedList.h"
#include "GameObjectFactoryList.h"
#include "GameObjectList.h"
#include "gamedefs.h"
#include "unified_handle_type.h"
#include "UnitFormation.h"
#include "../ui/DecorationManager.h"
#include <boost/shared_ptr.hpp>

#ifdef PROJECT_CLAW_PROTO
#ifndef USE_CLAW_CONTROLLER
#define USE_CLAW_CONTROLLER
#endif
#endif



// 20 tick interval for unit act check (about 15*20=300 msec)
#define GAME_UNIT_ACT_CHECK_COUNTER_INTERVAL 20


namespace frozenbyte 
{
  namespace ai
  {
    class PathFind;
    class Path;
  }
}

namespace util
{
  class Script;
  class ScriptProcess;
}

namespace ui
{
	class Map;
} 

namespace game
{

  class GameCollisionInfo;
  class UnitVisibilityChecker;
	class GameProfiles;

  class ItemList;
	class ItemManager;
	class UpgradeManager;
  class UnitList;
  class Unit;
  class ProjectileList;
  class Projectile;
  class BuildingList;
  class Building;
  class PartList;
  class Part;
  class GameRandom;
  class GameRequest;
  class GameUI;
  class GameMap;
  class GameScene;
  class PartTypeAvailabilityList;
  class GameScripting;
  class UnitSelections;
	class Checkpoints;
	class PlayerPartsManager;
	class VisualObjectModelStorage;
	class WaterManager;
	class DifficultyManager;
	class ParticleSpawnerManager;
	class EnvironmentalEffectManager;
	class LightBlinker;
	class AlienSpawner;
	class GamePhysics;
	class UnifiedHandleManager;
	class BonusManager;

	namespace tracking
	{
		class ObjectTracker;
	}

#ifdef USE_CLAW_CONTROLLER
	class ClawController;
#endif

  class Game
  {
  public:
    Game();
    ~Game();

    // sets the user interface connection
    void setUI(GameUI *gameUI);

    // starts a new game
    void newGame(bool multiplayer);

    // joins an existing game via network, returns true on success
    bool join();

		bool saveVariablesToMemory(char **memoryBuffer, const char *saveId, const char *saveType);
		bool loadVariablesFromMemory(const char *memoryBuffer, const char *saveId, const char *saveType, const char *applySub = "apply_variables");
		bool saveVariablesToTemporary();
		bool loadVariablesFromTemporary();

    // save this game to file, returns true on success
    bool saveVariables(const char *filename, const char *saveId, const char *saveType);

    // replace everything in this game with data read from the file
    // returns true on success, false on failure -> unstable state!
    bool loadVariables(const char *filename, const char *saveId, const char *saveType, const char *applySub = "apply_variables");

		// load and save the game (do save at mission start, load loads to mission start)
		bool loadGame(const char *saveId);
		bool saveGame(const char *saveId);
		// return true if savegame exists and correct type/loadable, else returns false
		// (sets the global savegame_... variables)
		bool getInfoForSavegame(const char *saveId, const char *saveType);


		// REALLY OLD STUFF (but possibly the future save)
		bool save(const char *filename);
		bool load(const char *filename);

    
    // advance one game tick 
    void run();

    // or...?
    // (msec time elapsed is taken as param, but should not 
    // affect the gameplay - ui may be affected by it though)
    //void run(int deltaTime);

    // game request handling 
    void sendRequest(GameRequest *request);
    void receiveRequest(GameRequest *request);
    void sendOrder(GameRequest *request);
    void receiveOrder(GameRequest *request);

    // returns true if this is the server (master)
    bool isServer(); 

    // returns true if this is a multiplayer game
    bool isMultiplayer(); 

    // exit menus and start the actual combat
    void startCombat();

    // end combat and return to menus
    void endCombat();

    // returns true if the player is active (has units in combat)
    bool isActivePlayer(int player);

    // returns true if this player is played by the computer
    bool isComputerOpponent(int player);

    // returns true if vertical/horizontal autoaim is on
    bool isAutoAimVertical();
    bool isAutoAimHorizontal();

    // returns true if these players are hostile to each other
    //bool isHostile(int player, int otherPlayer);
		bool isHostile(int player, int otherPlayer)
		{
			return hostile[player][otherPlayer];
		}

    void abortMission();

    void setPendingLoad(const char *saveId);
    const char *getPendingLoad();

    void setMissionScript(char *scriptname);

    void setMissionId(const char *missionid);
    const char *getMissionId();

    void setCurrentMission(char *missionfile);
    void setSuccessMission(char *missionfile);
    void setFailureMission(char *missionfile);

    void setBuildingsScript(char *scriptname);
    void setSectionScript(char *scriptname);

    char *getCurrentMission();
    char *getSuccessMission();
    char *getFailureMission();

		GameProfiles *getGameProfiles() { return profiles; }

    void requestEndCombat();

		// returns pid of the started script process or zero if failed.
    int addCustomScriptProcess(const char *script, Unit *unit, const std::vector<int> *paramStack);
    int addCustomScriptProcess(const char *script, UnifiedHandle uh, const std::vector<int> *paramStack);

    void stopAllCustomScriptProcesses();
    int getCustomScriptProcessAmount();

    void stopCustomScriptProcessById(int pid);
		bool isRunningCustomScriptProcess(const char *script, Unit *unit);

    void runCustomScriptProcesses();
    void runCustomUIScriptProcesses();

		// returns pid of given script process if it is a custom script process owned
		// by this game instance (else, returns zero)
    //int getCustomScriptProcessId(util::ScriptProcess *sp);

    void setCinematicScriptProcess(char *script);

		bool isCinematicScriptRunning();

		void skipCinematicScript();

		bool isSkippingCinematic();

    void setTacticalMode(bool tacticalModeOn);

    bool isTacticalMode();

    void setPaused(bool pauseOn);

    bool isPaused() const;

		void advanceMissionStartState();

		int getCinematicScriptProcessId();

		// mission time during which the game was not paused
		int getGameplayTime();

		// return true if missionsuccess/failure counter below zero
		// (that is mission failed or success on the screen, and fading out)
		bool isMissionAboutToEnd();

		Unit *createGhostsOfFuture(Unit *unit);

    GameScene *getGameScene() { return gameScene; }
    //GameMap *getGameMap() { return gameMap; } // use gamescene instead
    GameUI *getGameUI() const { return gameUI; }
    GameRandom *getGameRandom() { return gameRandom; }

		void setEnvironmentalEffectManager(EnvironmentalEffectManager *effman);
		EnvironmentalEffectManager *getEnvironmentalEffectManager();

    //void addMapObstacle(int x, int y);

    //void removeMapObstacle(int x, int y);

		GamePhysics *getGamePhysics() { return physics; }

    // TODO, lotsa stuff...


  // TODO, monkey code!
  //private:
    GameUI *gameUI;

		int singlePlayerNumber;

    GameObjectFactoryList *objectFactories;
    GameObjectList *objectList;

    // units (armors and stuff) for each player
    UnitList *units;

    // parts in storage for each player
    PartList *parts;

		PlayerPartsManager *playerPartsManager;

    // projectiles
    ProjectileList *projectiles;

		// items
		ItemList *items;
		ItemManager *itemManager;

		// upgrades
		UpgradeManager *upgradeManager;

    // buildings
    BuildingList *buildings;

		// visual object models
		VisualObjectModelStorage * visualObjectModelStorage;

    // money and stuff for each player
    //ResourceList *resources;
    int money[ABS_MAX_PLAYERS];
    
    // spawns
    int spawnX[ABS_MAX_PLAYERS];
    int spawnY[ABS_MAX_PLAYERS];

    // if the player is hostile towards the other...
    bool hostile[ABS_MAX_PLAYERS][ABS_MAX_PLAYERS];

    //FoobarAI *computerAI[ABS_MAX_PLAYERS];

    UnitSelections *unitSelections[ABS_MAX_PLAYERS];

    // part types available for each player
    PartTypeAvailabilityList *partTypesAvailable;

    // game requests that have been timed by master (thus being orders)
    LinkedList *orderQueue;

    GameMap *gameMap;

    GameRandom *gameRandom;

    GameScene *gameScene;

    UnitVisibilityChecker *visibilityChecker;

    UnitFormation formations;

		Checkpoints *checkpoints;

		ui::DecorationManager *decorationManager;

		WaterManager *waterManager;

		LightBlinker *lightBlinker;
		LightBlinker *outdoorLightBlinker;

		ParticleSpawnerManager *particleSpawnerManager;
		boost::shared_ptr<ui::Map> map;

		void clearVisualObjectModelStorage();

		AlienSpawner *alienSpawner;

		bool isCooperative() const;
		void setCooperative( bool coop );

		char *getGameSceneGraphDump();
		void deleteGameSceneGraphDump(char *buf);

		UnifiedHandleManager *unifiedHandleManager;
		tracking::ObjectTracker *objectTracker;

		BonusManager *bonusManager;

  private:
    bool server;
    bool multiplayer;
		bool cooperative;

    char *currentMission;
    char *nextMissionOnSuccess;
    char *nextMissionOnFailure;
    char *script;
		char *missionId;

		GameProfiles *profiles;

		// if set to other than null, tells the game to start loading the savegame
		char *pendingLoad;

		// if set, tells the game to automatically skip menu mode and start next mission
		bool automaticallyStartMission;
		// HACK: need to make sure that the game loops at least once thru the winmain...
		bool automaticallyStartMission_loop2;

		char *buildingsScript;
		char *sectionScript;

		char *temporarySaveBuffer;

    util::ScriptProcess *cinematicScriptProcess;
		bool skippingCinematic;
    
    bool endingCombat;
		bool shouldResetAimUpwardMode;

    bool paused;
    
    bool tacticalMode;

		DifficultyManager *difficultyManager;

		EnvironmentalEffectManager *environmentalEffectManager;

		LinkedList *customScriptProcesses;

		void applyPendingLoad();

		game::GamePhysics *physics;

		// For savegame hack
		int lastSaveNumber;

  public:

    bool inCombat;

    int gameTimer;
    int syncToTimer; 
    int syncInterval; // defines sync send interval (causes artificial lag)

    // current map filename read from mission data
    char *currentMap;

		// last loaded/saved savegame
		std::string lastSaveId;

    GameScripting *gameScripting;

    int missionSuccessCounter;
    int missionFailureCounter;
    int missionStartTime;
		bool missionAborting;

		int pauseStartTime;
		// length of time during which game was paused
		int missionPausedTime;

		int lastTickTime;

		Unit *devUnit;

    void givePlayerBeginningStuff(int player);

    // return price needed to purchase this part and all of its subparts
    int calculatePurchasePrice(Part *part);

    // return price needed to repair this part and all of its subparts
    int calculateRepairPrice(Part *part);

    // return price needed to reload this part and all of its subparts
    int calculateReloadPrice(Part *part);

    // reload part and all subparts (if they are weapons or ammopacks)
    // does not check for sufficient money, do it before calling this
    void reloadParts(Part *part);

    // purchases part and all subparts attached to it (for unpurchased only)
    // does not check for sufficient money, do it before calling this
    void purchaseParts(Part *part);

    // repair part and all subparts attached to it
    // does not check for sufficient money, do it before calling this
    void repairParts(Part *part);

    // pay for this part (purchase or repair, whichever needed)
    void payForPart(Part *part);

    // find parts that have not been paid for (recurse to subparts)
    // if one found, it will be detached with detachParts()
    void removeUnpurchasedParts(Unit *unit, Part *part);

    // detaches a part and all of it's sub parts, any unpurchased parts
    // will be deleted others put to players part storage
    void detachParts(Unit *unit, Part *part);

    // removes a part and all of it's sub parts
    void removeParts(Unit *unit, Part *part);

    // create visual object for the part, recurse to subparts
    void createVisualForParts(Unit *unit, Part *part, bool keepBones = false);

    // delete visual object of the part, recurse to subparts
    void deleteVisualOfParts(Unit *unit, Part *part, bool keepBones = false);

	// delete & create visual object of the part, recurse to subparts (maintains renderpos)
    void recreateVisualOfParts(Unit *unit, Part *part, bool keepBones = false);

		// returns the difficultymanager
		DifficultyManager *getDifficultyManager();

#ifdef USE_CLAW_CONTROLLER
	private:
		ClawController *clawController;
	public:
		ClawController *getClawController() { return clawController; }
#endif

};

}

#endif

