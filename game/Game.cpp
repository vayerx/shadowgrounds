
#include "precompiled.h"

#include <stdio.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#include <malloc.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "IStorm3D_Terrain.h"

#include "Game.h"

// debugging only...
#include "../system/Timer.h"
#include "../convert/str2int.h"

// unwanted dependencies... 
// (well actually terrain should be gameplay, but visual effects not)
#include "../ui/Terrain.h"
#include "../ui/VisualEffectManager.h"
#include "../ui/Map.h"
#include "../ui/GameController.h"	// Probably breaks the design here but is used a to fix a bug. -Pete
#include "../particle_editor2/particleeffect.h"

#ifdef PROJECT_SHADOWGROUNDS
#include "../shadowgrounds/version.h"
#elif PROJECT_AOV
#include "../aov/version.h"
#elif PROJECT_CLAW_PROTO
#include "../claw_proto/version.h"
#elif PROJECT_SURVIVOR
#include "../survivor/version.h"
#else 
#error "Game.cpp, Unknown project, don't know which version.h to include."
#endif

#include "../ui/LoadingMessage.h"


#include "../system/Logger.h"
#include "../system/SystemTime.h"
#include "gamedefs.h"
#include "scaledefs.h"
#include "savegamevars.h"
#include "GameObject.h"
#include "DHLocaleManager.h"
#include "MissionParser.h"
#include "unittypes.h"
#include "ArmorUnitActor.h"
#include "ProjectileActor.h"
#include "WaterManager.h"
#include "LightBlinker.h"
#include "GameCollisionInfo.h"
#include "UnitList.h"
#include "PartList.h"
#include "Part.h"
#include "Unit.h"
#include "Flashlight.h"
#include "WeaponObject.h"
#include "AmmoPackObject.h"
#include "Character.h"
#include "GameUI.h"
#include "GameRequest.h"
#include "Building.h"
#include "BuildingList.h"
#include "Weapon.h"
#include "UnitType.h"
#include "ItemList.h"
#include "ItemManager.h"
#include "UpgradeManager.h"
#include "Projectile.h"
#include "ProjectileList.h"
#include "PlayerPartsManager.h"
#include "GameMap.h"
#include "GameScene.h"
#include "GameRandom.h"
#include "VisualObjectModelStorage.h"
#include "UnitSelections.h"
#include "UnitSpawner.h"
#include "UnitVisibilityChecker.h"
#include "Checkpoints.h"
#include "PartTypeAvailabilityList.h"
#include "../util/AI_PathFind.h"
#include "UnitLevelAI.h"
#include "scripting/GameScripting.h"
#include "../util/ScriptManager.h"
#include "../util/Script.h"
#include "../util/ScriptProcess.h"
#include "../util/SimpleParser.h"
#include "../sound/MusicPlaylist.h"
#include "../sound/playlistdefs.h"
#include "../util/BuildingMap.h"
#include "../util/LightAmountManager.h"
#include "../ui/AmbientSoundManager.h"
#include "SimpleOptions.h"
#include "options/options_players.h"
#include "options/options_cheats.h"
#include "options/options_game.h"
#include "options/options_graphics.h"
#include "options/options_physics.h"
#include "options/options_precalc.h"
#include "options/options_camera.h"
#include "AniManager.h"
#include "DifficultyManager.h"
#include "ParticleSpawnerManager.h"
#include "EnvironmentalEffectManager.h"
#include "../ui/AniRecorderWindow.h"
#include "../ui/UIEffects.h"
#include "../util/HelperPositionCalculator.h"
#include "../util/TextFileModifier.h"
#include "../sound/AmbientAreaManager.h"
#include "GameProfiles.h"
#include "GameProfilesEnumeration.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"
#include "AlienSpawner.h"
#include "GameStats.h"
#include "../util/FBCopyFile.h"
#include "../system/FileTimestampChecker.h"
#include "physics/GamePhysics.h"
#include "UnitPhysicsUpdater.h"
#include "UnifiedHandleManager.h"
#include "tracking/TrackableUnifiedHandleObject.h"
#include "tracking/ObjectTracker.h"
#include "tracking/AnyBurnableTrackableObjectFactory.h"
#include "tracking/ScriptableTrackerObjectType.h"
#include "ProjectileTrackerObjectType.h"
#include "IProjectileTrackerFactory.h"
#include "../ui/camera_system/ICameraSystem.h"
#ifdef PHYSICS_ODE
#include "../physics_ode/OdeStormModelCooker.h"
#endif

#ifdef PROJECT_CLAW_PROTO
#ifndef USE_CLAW_CONTROLLER
#define USE_CLAW_CONTROLLER
#endif
#endif

#ifdef USE_CLAW_CONTROLLER
#include "ClawController.h"
extern game::ClawController *gamephysics_clawController;
#endif

#ifdef PROJECT_SURVIVOR
	#include "BonusManager.h"
#endif

#include "../system/Miscellaneous.h"

#include "userdata.h"

#define GAME_UNIT_LOW_ACT_RANGE_SQ (25*25)

#define TOUCHBULLET_INTERVAL 4

// HACK: HACK...
static game::Game *developerGame = NULL;
bool isThisDeveloper()
{
	if (developerGame != NULL
		&& developerGame->getGameProfiles() != NULL
		&& developerGame->getGameProfiles()->getCurrentProfile(0) != NULL
		&& strcmp(developerGame->getGameProfiles()->getCurrentProfile(0), "Developer") == 0)
	{
		return true;
	}
	return false;
}

extern bool next_interface_generation_request;

bool dontLoadMainMenu = false;

using namespace frozenbyte;

// TEMP: for debugging...
namespace ui
{
	extern int visual_object_allocations;
	extern int visual_object_model_allocations;
}

// for perf stats
int game_actedAmount = 0;
int game_quickNoActAmount = 0;
int game_slowNoActAmount = 0;

// HACK: ...
game::tracking::AnyBurnableTrackableObjectFactory *game_anyBurnableTrackableObjectFactory = NULL;

// EXTRA HACK: ...
extern game::Unit *hackhack_trackerunit;


// MORE HACK: ...
namespace game
{
	extern std::string gs_trackerProjectileBullet;
}

class GameProjectileTrackerFactory : public game::IProjectileTrackerFactory
{
public:
	virtual game::Projectile *createNewProjectileTrackerInstance()
	{
		const char *idstr = game::gs_trackerProjectileBullet.c_str();

		assert(this->game != NULL);

		game::Bullet *bullet = NULL;

		if (PARTTYPE_ID_STRING_VALID(idstr))
		{
			game::PartType *pt = game::getPartTypeById(PARTTYPE_ID_STRING_TO_INT(idstr));
			if (pt == NULL) 
			{ 
				LOG_ERROR_W_DEBUG("GameProjectileTrackerFactory::createNewProjectileTrackerInstance - reference to unloaded part type.", idstr);
			} else {
				if (pt->isInherited(game::getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				{ 
					// WARNING: unsafe cast!
					bullet = (game::Bullet *)pt;
				} else {
					LOG_ERROR_W_DEBUG("GameProjectileTrackerFactory::createNewProjectileTrackerInstance - part type is not bullet type.", idstr);
				}
			}
		} else {
			LOG_ERROR_W_DEBUG("GameProjectileTrackerFactory::createNewProjectileTrackerInstance - invalid part type id string.", idstr);
		}

		if (bullet == NULL)
		{
			return NULL;
		}

		game::Projectile *proj = new game::Projectile(NULL, bullet);
		game->projectiles->addProjectile(proj);

		VC3 pos = VC3(0,0,0);

		proj->setDirectPath(pos, pos, bullet->getVelocity());

		game::ProjectileActor pa = game::ProjectileActor(game);
		pa.createVisualForProjectile(proj, true, hackhack_trackerunit);

		// oops, we're using bullets directly, not weapons..
		/*
		const char *fireSound = weapon->getFireSound();
		if (fireSound != NULL)
		{
			bool looped = false;
			int handleDummy = -1;
			int key = 0;

			float range = bullet->getChainSoundRange(HITCHAIN_NOTHING);
			int priority = bullet->getChainSoundPriority(HITCHAIN_NOTHING);
			int handle = game->gameUI->parseSoundFromDefinitionString(fireSound, projpos.x, projpos.y, projpos.z, &looped, &handleDummy, &key, false, range, priority);

			if (handle == -1)
			{
				LOG_WARNING_W_DEBUG("GameProjectileTrackerFactory::createNewProjectileTrackerInstance - Failed to create firesound.", fireSound);
			}
		}

		if (looped)
		{
			Logger::getInstance()->warning("GameProjectileTrackerFactory::createNewProjectileTrackerInstance - Firesound definition was a looping sound, not properly supported.");
			// TODO: looped sounds not yet supported
			assert(!"looped GameProjectileTrackerFactory::createNewProjectileTrackerInstance sounds not supported.");
		}
		*/

		proj->setProjectileIsPointedBy(this);

		return proj;
	}

	void projectileDeleted(game::Projectile *projectile)
	{
		assert(this->game != NULL);
		if (this->game->objectTracker != NULL)
		{
			this->game->objectTracker->releaseTracker(projectile);
		}
	}

	void setGame(game::Game *game) { this->game = game; }
private:
	game::Game *game;
};

GameProjectileTrackerFactory game_projTrackerFactory = GameProjectileTrackerFactory();


namespace game
{
namespace {

	int getNumberOfPlayers()
	{
		int result = 0;
		int c;
		for( c = 0; c < MAX_PLAYERS_PER_CLIENT; c++ )
		{
			if( SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + c ) )
			{
				result++;
			}
		}

		return result;
	}
}

	class SaveParentEntry
	{
	public:
		SaveParentEntry(int s, int children, GameObject *r) 
		{ 
			childleft = children; saved = s; real = r; 
		}
		int saved;				// the ptr value saved to file
		int childleft;		// amount of children expected, once 0 pop off
		GameObject *real; // real object pointer
	};


	Game::Game()
	{
		missionSuccessCounter = 0;
		missionFailureCounter = 0;

		developerGame = this;

		UnitVariables::init();

		visualObjectModelStorage = new VisualObjectModelStorage();

		objectList = new GameObjectList();
		objectFactories = new GameObjectFactoryList();
		GameObject::setConstructorList(objectList);

		units = new UnitList();
		parts = new PartList();
		buildings = new BuildingList();
		projectiles = new ProjectileList();
		partTypesAvailable = new PartTypeAvailabilityList();

		orderQueue = new LinkedList();

		gameMap = new GameMap();
		formations.setGameMap(gameMap);

		gameScripting = new GameScripting(this);
		AniManager::createInstance(gameScripting);

		gameRandom = new GameRandom();

		singlePlayerNumber = 0;

		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			unitSelections[i] = new UnitSelections(units, i);
			money[i] = 0;
			spawnX[i] = 0;
			spawnY[i] = 0;
			for (int j = 0; j < ABS_MAX_PLAYERS; j++)
			{
				if (i == j)
					hostile[i][j] = false;
				else
					hostile[i][j] = true;
			}
			//computerAI[i] = NULL; //new FoobarAI(this, i);
		}

		visibilityChecker = new UnitVisibilityChecker(this);

		currentMap = NULL;

		currentMission = NULL;

		const char *mdefs = SimpleOptions::getString(DH_OPT_S_GAME_MISSIONDEFS);
		assert(mdefs != NULL);
		if (mdefs == NULL)
		{
#ifdef LEGACY_FILES
			mdefs = "Data/Missions/missiondefs.txt";
#else
			mdefs = "data/mission/missiondefs.txt";
#endif
		}

		util::SimpleParser sp;
		if (sp.loadFile(mdefs))
		{
			while (sp.next())
			{
				if (sp.getKey() != NULL
					&& strcmp(sp.getKey(), "firstmission") == 0)
				{
					setCurrentMission(sp.getValue());
				} else {
					sp.error("Unknown key encountered in mission definitions file.");
				}
			}
		} else {
			Logger::getInstance()->error("Game - Could not open missiondefs file.");
		}
		if (currentMission == NULL)
		{
			Logger::getInstance()->error("Game - Missing firstmission definition.");
			assert(!"No firstmission defined");
		}

		nextMissionOnSuccess = NULL;
		nextMissionOnFailure = NULL;

		missionStartTime = 0;
		pauseStartTime = 0;
		missionPausedTime = 0;
		paused = false;

		lastSaveNumber = 0;

		script = NULL;
		missionId = NULL;
		buildingsScript = NULL;
		sectionScript = NULL;

		cinematicScriptProcess = NULL;
		skippingCinematic = false;

		customScriptProcesses = NULL;

		gameScene = NULL;

		decorationManager = NULL;
		waterManager = NULL;

		lightBlinker = NULL;
		outdoorLightBlinker = NULL;

		checkpoints = new Checkpoints();

		playerPartsManager = new PlayerPartsManager(this);

		items = new ItemList();

		itemManager = new ItemManager(this);
		upgradeManager = new UpgradeManager(this);

		devUnit = NULL;

		particleSpawnerManager = NULL;

		difficultyManager = new DifficultyManager(this);

		environmentalEffectManager = NULL;

		temporarySaveBuffer = NULL;

		pendingLoad = NULL;
		automaticallyStartMission = false;
		automaticallyStartMission_loop2 = false;

		endingCombat = false;
		shouldResetAimUpwardMode = false;

		profiles = new GameProfiles();

		alienSpawner = new AlienSpawner(this, this->singlePlayerNumber);

		/*
		// TEMP HACK: for printing what savegames we have...
		for (int si = 1; si < 11; si++)
		{
			char foo[8];
			strcpy(foo, int2str(si));
			bool val = this->getInfoForSavegame(foo, "savegame");

			Logger::getInstance()->error(savegame_type.c_str());
			Logger::getInstance()->error(savegame_version.c_str());
			Logger::getInstance()->error(savegame_description.c_str());
			Logger::getInstance()->error(savegame_time.c_str());
			if (val)
			{
				Logger::getInstance()->error("return value true (savegame exists/loadable)");
			} else {
				Logger::getInstance()->error("return value false (savegame does not exist/unloadable");
			}
		}
		*/
		cooperative = false;

		{
			for (int stats = 0; stats < MAX_PLAYERS_PER_CLIENT; stats++)
			{
				game::GameStats::instances[stats] = new GameStats(this, stats);
			}
		}

#ifdef LEGACY_FILES
		// ...
#else
		bool user_autoexec_exists = false;
		FILE *autoexecf = fopen(igios_mapUserDataPrefix("config/user_autoexec.dhs").c_str(), "rb");
		if (autoexecf != NULL)
		{
			fclose(autoexecf);
			user_autoexec_exists = true;
		}
		if (!user_autoexec_exists)
		{
			char *f1_cstr = "data/misc/default_user_autoexec.dhs";
			std::string f1 = std::string(f1_cstr);
			std::string f2 = std::string(igios_mapUserDataPrefix("config/user_autoexec.dhs"));
			util::FBCopyFile::copyFile(f1, f2);
		}
#endif

#ifdef USE_CLAW_CONTROLLER
		clawController = new ClawController();
		clawController->setGame(this);
		gamephysics_clawController = clawController;
#endif
		this->physics = new GamePhysics();

#ifdef PHYSICS_ODE
		odeModelCooker.setStorage(this->visualObjectModelStorage);
#endif

		tracking::TrackableUnifiedHandleObject::setGame(this);
		objectTracker = new tracking::ObjectTracker();
		unifiedHandleManager = new UnifiedHandleManager(this);

		std::vector<tracking::ITrackableUnifiedHandleObjectImplementationManager *> burnableImplementations;
		burnableImplementations.push_back(this->units);
		game_anyBurnableTrackableObjectFactory = new tracking::AnyBurnableTrackableObjectFactory(burnableImplementations);
		objectTracker->addTrackableObjectFactory(game_anyBurnableTrackableObjectFactory);

		game_projTrackerFactory.setGame(this);
		ProjectileTrackerObjectType *ptt = new ProjectileTrackerObjectType();
		ProjectileTrackerObjectType::setProjectileTrackerFactory(&game_projTrackerFactory);
		objectTracker->addTrackerType(ptt);

#ifdef PROJECT_SURVIVOR
		bonusManager = new BonusManager(this);
#else
		bonusManager = NULL;
#endif

		missionAborting = false;
	}


	Game::~Game()
	{
		delete game_anyBurnableTrackableObjectFactory;

		delete unifiedHandleManager;
		delete objectTracker; 
		objectTracker = NULL;
		tracking::TrackableUnifiedHandleObject::cleanupPool();

		if (this->physics != NULL)
		{
			delete this->physics;
			this->physics = NULL;
		}

#ifdef USE_CLAW_CONTROLLER
		delete clawController;
		clawController = NULL;
#endif

		{
			for (int stats = 0; stats < MAX_PLAYERS_PER_CLIENT; stats++)
			{
				delete game::GameStats::instances[stats];
			}
		}

		delete alienSpawner;

		delete profiles;

		stopAllCustomScriptProcesses();

		LinkedList *uretlist = units->getAllUnits();
		SafeLinkedListIterator uretiter = SafeLinkedListIterator(uretlist);
		while (uretiter.iterateAvailable())
		{
			Unit *u = (Unit *)uretiter.iterateNext();
			UnitSpawner::deleteUnit(this, u);
		}

		delete gameScripting;

		delete orderQueue;

		delete visibilityChecker;

		if (decorationManager != NULL)
		{
			delete decorationManager;
		}
		if (waterManager != NULL)
		{
			delete waterManager;
		}
		if (lightBlinker != NULL)
		{
			delete lightBlinker;
		}
		if (outdoorLightBlinker != NULL)
		{
			delete outdoorLightBlinker;
		}

		if (gameScene != NULL)
		{
			delete gameScene;
		}

		/*
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			if (computerAI[i] != NULL) delete computerAI[i];
		}
		*/

		setPendingLoad(NULL);

		if (currentMission != NULL)
		{
			delete [] currentMission;
			currentMission = NULL;
		}

		if (script != NULL)
		{
			delete [] script;
			script = NULL;
		}

		if (missionId != NULL)
		{
			delete [] missionId;
			missionId = NULL;
		}

		if (buildingsScript != NULL)
		{
			delete [] buildingsScript;
			buildingsScript = NULL;
		}

		if (sectionScript != NULL)
		{
			delete [] sectionScript;
			sectionScript = NULL;
		}

		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			if (unitSelections[i] != NULL)
				delete unitSelections[i];
		}

		// delete map
		if (gameMap != NULL)
			delete gameMap;

		// nice clean-up for the lightamountmanager...
		util::LightAmountManager::cleanInstance();

		// delete random number generator
		if (gameRandom != NULL)
			delete gameRandom;

		// delete all units
		LinkedList *ulist = units->getAllUnits();
		SafeLinkedListIterator uiter = SafeLinkedListIterator(ulist);
		while (uiter.iterateAvailable())
		{
			// NOTE: this should now be handled by UnitSpawner!
			assert(!"Units still left alive at quit.");

			Unit *u = (Unit *)uiter.iterateNext();
			// get rid of character
			if (u->getCharacter() != NULL)
			{
				Character *c = u->getCharacter();
				u->setCharacter(NULL);
				delete c;
			}
			// get rid of parts first
			if (u->getRootPart() != NULL) 
			{
				deleteVisualOfParts(u, u->getRootPart());
				detachParts(u, u->getRootPart());
			}
			units->removeUnit(u);
			delete u;
		}
		// delete parts in storage
		LinkedList *plist = parts->getAllParts();
		SafeLinkedListIterator piter = SafeLinkedListIterator(plist);
		while (piter.iterateAvailable())
		{
			Part *p = (Part *)piter.iterateNext();
			parts->removePart(p);
			delete p;
		}
		// delete projectiles
		LinkedList *projlist = projectiles->getAllProjectiles();
		SafeLinkedListIterator projiter = SafeLinkedListIterator(projlist);
		while (projiter.iterateAvailable())
		{
			Projectile *proj = (Projectile *)projiter.iterateNext();
			projectiles->removeProjectile(proj);
			delete proj;
		}
		// delete buildings
		LinkedList *blist = buildings->getAllBuildings();
		SafeLinkedListIterator biter = SafeLinkedListIterator(blist);
		while (biter.iterateAvailable())
		{
			Building *b = (Building *)biter.iterateNext();
			buildings->removeBuilding(b);
			delete b;
		}

		// delete items
		assert(itemManager != NULL);
		delete itemManager;
		delete items;

		assert(upgradeManager != NULL);
		delete upgradeManager;

		delete buildings;
		delete partTypesAvailable;
		delete projectiles;
		delete parts;
		delete units;

		GameObject::setConstructorList(NULL);
		delete objectList;
		delete objectFactories;

		if (currentMap != NULL)
		{
			delete [] currentMap;
		}

		assert(particleSpawnerManager == NULL);

		UnitVariables::uninit();

		delete checkpoints;

		delete playerPartsManager;

		delete visualObjectModelStorage;

		delete difficultyManager;

		if (temporarySaveBuffer != NULL)
		{
			delete [] temporarySaveBuffer;
			temporarySaveBuffer = NULL;
		}

		delete[] nextMissionOnSuccess;
		nextMissionOnSuccess = 0;
		delete[] nextMissionOnFailure;
		nextMissionOnFailure = 0;

#ifdef PROJECT_SURVIVOR
		if(bonusManager)
		{
			delete bonusManager;
		}
#endif
	}


	void Game::setUI(GameUI *gameUI)
	{
		this->gameUI = gameUI;
	}


	void Game::newGame(bool multiplayer)
	{
		// TODO: clear all game objects... redo the lists
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			money[i] = 0;
			spawnX[i] = 0;
			spawnY[i] = 0;
			for (int j = 0; j < ABS_MAX_PLAYERS; j++)
			{
				if (i == j)
					hostile[i][j] = false;
				else
					hostile[i][j] = true;
			}
		}

		server = true;
		this->multiplayer = multiplayer;

		if (multiplayer)
		{
			syncInterval = DEFAULT_MULTIPLAYER_SYNC_INTERVAL;
		} else {
			syncInterval = DEFAULT_SINGLEPLAYER_SYNC_INTERVAL;
		}
		gameTimer = 0;
		lastTickTime = Timer::getTime();
		syncToTimer = 0;

		inCombat = false;
		endingCombat = false;

		Timer::update();
		gameRandom->seed(Timer::getTime());

		// initial units for players and stuff...
		singlePlayerNumber = 0;
		givePlayerBeginningStuff(singlePlayerNumber);

#ifdef LEGACY_FILES
		gameScripting->loadScripts("Config/user_autoexec.dhs", NULL);
#else
		gameScripting->loadScripts("config/user_autoexec.dhs", NULL);
#endif
		gameScripting->runMissionScript("user_autoexec", "runbefore");

		MissionParser mp = MissionParser();
		mp.parseMission(this, currentMission, 
			MISSIONPARSER_SECTION_BEFORE);

		if (sectionScript != NULL)
		{
			gameScripting->runMissionScript(sectionScript, "runbefore");
			setSectionScript(NULL);
		}

		// open ship's command brigde window
		gameUI->openCommandWindow(singlePlayerNumber);

		// and some music!
#ifdef LEGACY_FILES
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_MENUS, 
			"Data/Music/Playlists/default_menus.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_ACTION, 
			"Data/Music/Playlists/default_action.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_CALM, 
			"Data/Music/Playlists/default_calm.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_FAILURE, 
			"Data/Music/Playlists/default_failure.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_SUCCESS, 
			"Data/Music/Playlists/default_success.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_INTRO, 
			"Data/Music/Playlists/default_intro.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_CREDITS, 
			"Data/Music/Playlists/default_credits.txt");
#else
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_MENUS, 
			"data/audio/music/playlist/default_menus.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_ACTION, 
			"data/audio/music/playlist/default_action.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_CALM, 
			"data/audio/music/playlist/default_calm.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_FAILURE, 
			"data/audio/music/playlist/default_failure.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_SUCCESS, 
			"data/audio/music/playlist/default_success.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_INTRO, 
			"data/audio/music/playlist/default_intro.txt");
		gameUI->getMusicPlaylist(singlePlayerNumber)->loadPlaylist(PLAYLIST_CREDITS, 
			"data/audio/music/playlist/default_credits.txt");
#endif

		gameUI->getMusicPlaylist(singlePlayerNumber)->setBank(PLAYLIST_MENUS);
		gameUI->getMusicPlaylist(singlePlayerNumber)->nextTrack();
		gameUI->getMusicPlaylist(singlePlayerNumber)->play();
	}


	// TODO, need a server param...
	bool Game::join()
	{
		// TODO, clear all game objects...

		// TODO, connect and load snapshot

		server = false;
		multiplayer = true;

		return true;
	}

// TEMP
static VC3 last_attemptedPos = VC3(0,0,0);


	void Game::run()
	{
		// process game requests
		while (!orderQueue->isEmpty() 
			&& ((GameRequest *)orderQueue->peekFirst())->executeTime 
			<= gameTimer)
		{
			GameRequest *req = (GameRequest *)orderQueue->popFirst();
			if (req->executeTime != gameTimer)
			{
				// we have skipped in time???
				assert(!"Game skipped in time");
				break;
			}
			req->execute();
			delete req;
		}

		// if this is the server, send sync data to all clients
		if (server)
		{
			if ((gameTimer % syncInterval) == 0)
			{
				syncToTimer = gameTimer + syncInterval;
				if (multiplayer)
				{
					// TODO: NETGAME, echo this sync to all clients
				}
			}
		}

		// if we are allowed to continue (not yet reached sync time),
		// advance one game tick...
		if (gameTimer < syncToTimer)
		{ 		 
			if (inCombat)
			{
				if ( gameUI->getLoadingWindow() && paused && missionSuccessCounter != 0 )
				{
					missionSuccessCounter--;
					if (missionSuccessCounter == 0)
					{
//						gameUI->closeLoadingWindow( singlePlayerNumber );
						gameUI->getLoadingWindow()->closeWindow ();
						this->setPaused( false );
						missionSuccessCounter = 1; // Skip the mission as well as the briefing.
					}
				}
				if (!paused && !tacticalMode)
				{
					if (SimpleOptions::getBool(DH_OPT_B_PHYSICS_UPDATE))
					{
						std::vector<TerrainObstacle> removedObjects;
						util::GridOcclusionCuller *culler = NULL;
						if (SimpleOptions::getBool(DH_OPT_B_OCCLUSION_CULLING_ENABLED))
						{
							culler = gameUI->gridOcclusionCuller;
						}
						gameUI->getTerrain()->updatePhysics(physics, removedObjects, culler);
						int removedAmount = removedObjects.size();
						if (removedAmount > 0)
						{
							gameScene->removeTerrainObstacles(removedObjects);
						}
					}

#ifdef USE_CLAW_CONTROLLER
					if(inCombat && getClawController() && gameUI->firstPerson[0])
					{
						if(gameUI->firstPerson[0]->getVisualObject())
						{
							getClawController()->setModel(gameUI->firstPerson[0]->getVisualObject()->getStormModel());
						}

						getClawController()->setSpawnPosition(gameUI->firstPerson[0]->getPosition());
					}
#endif

					physics->runPhysics(gameUI->getStormScene(), gameUI->getVisualEffectManager()->getParticleEffectManager());

					objectTracker->run(GAME_TICK_MSEC);

					// update ground focus height for physics (fluid containment) every once a while...
					if ((gameTimer & 15) == 3)
					{
						VC3 gfocuspos = gameUI->getGameCamera()->getPosition();
						if (gameUI->isThirdPersonView(singlePlayerNumber))
						{
							if (gameUI->getFirstPerson(0) != NULL)
							{
								gfocuspos = gameUI->getFirstPerson(0)->getPosition();
							}
						}
						if (gameMap->isWellInScaledBoundaries(gfocuspos.x, gfocuspos.z))
						{
							gfocuspos.y = gameMap->getScaledHeightAt(gfocuspos.x, gfocuspos.z);
						}
						physics->setGroundFocusHeight(gfocuspos.y);
					}
	
					// NEW: visibility check run only every 2nd tick!
					if ((gameTimer & 1) == 0)
					{
#ifdef _DEBUG
#ifdef FROZENBYTE_DEBUG_MEMORY
						frozenbyte::debug::debugSetAllocationInfo("visibility check");
#endif
#endif
						visibilityChecker->runCheck();
#ifdef _DEBUG
#ifdef FROZENBYTE_DEBUG_MEMORY
						frozenbyte::debug::debugSetAllocationInfo("visibility check over");
#endif
#endif
					}

					// computer ai
					/*
					for (int i = 0; i < ABS_MAX_PLAYERS; i++)
					{
						// do it only once a while, otherwise this is an overkill...
						if (isComputerOpponent(i))
						{
							if ((gameTimer % (ABS_MAX_PLAYERS * 16)) == (i * 16))
	//						if ((gameTimer % ABS_MAX_PLAYERS) == i)
									if (computerAI[i] != NULL)
										computerAI[i]->doStuff();
						}
					}
					*/

					// HACK...
					game::unitactor_acttargeting_counter = 0;

					// move units
					// unit level ai
					int unitActNum = 0;

					VC3 disableCenterPos = gameUI->getGameCamera()->getPosition();
					if (!isCooperative()
						&& gameUI->getFirstPerson(0) != NULL
						&& !this->isCinematicScriptRunning()
						&& gameUI->getCameraNumber() == GAMEUI_CAMERA_NORMAL)
					{
						disableCenterPos = gameUI->getFirstPerson(0)->getPosition();
					}

					UnitPhysicsUpdater::startStats();

					LinkedList *ulist = units->getAllUnits();
					LinkedListIterator unititer = LinkedListIterator(ulist);
					while (unititer.iterateAvailable())
					{
						bool lessActing = false;

						unitActNum++;

						Unit *unit = (Unit *)unititer.iterateNext();
						if (unit->isActive())
						{
							// has been in acting range? (no act check counter value set)
							if (unit->getActCheckCounter() == 0)
							{
								// the unit has been in acting range, so now we're gonna either
								// use the last acted value or check for new acted value...
								// (every second frame)

								if ((unitActNum & 1) == (this->gameTimer & 1))
								{
									// every second frame just use the old acted flag...
									if (!unit->hasActed())
									{
										continue;
									}
								} else {
									// every second frame actually check for the acting range

									if (unit->getUnitType()->getAIDisableRange() > 0
										&& unit->doesUseAIDisableRange())
									{
										int maxDist = unit->getUnitType()->getAIDisableRange();
										float maxDistSq = float(maxDist * maxDist);
										VC3 udistfromcam = unit->getPosition() - disableCenterPos;
										udistfromcam.y = 0; // (use 2d range, not 3d range)
										float distSq = udistfromcam.GetSquareLength();
										if (distSq > maxDistSq)
										{
											unit->setActCheckCounter(GAME_UNIT_ACT_CHECK_COUNTER_INTERVAL);
											unit->setActed(false);
											game_slowNoActAmount++;
											continue;
										}

										// NOTE: this does not work quite correctly anymore as
										// this check is done only every 2nd frame
										if (distSq > GAME_UNIT_LOW_ACT_RANGE_SQ)
										{
											lessActing = true;
										}
									}
									unit->setActed(true);
								}
							} else {

								// the unit has been out of act range, so gonna just skip acting for a while..
								// (until this counter reaches zero)
								unit->setActCheckCounter(unit->getActCheckCounter() - 1);

								// this if always true? (at least should be if the act check counter in nonzero)
								if (!unit->hasActed())
								{
									game_quickNoActAmount++;
									continue;
								}

							}

							// fade effects advance...
							unit->advanceFade();

							// run ai
							UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
							if (ai != NULL)
								ai->runUnitAI();

							// do not run act for scripting units and such, and run act
							// only every second tick for some units that are quite far away...
							if (unit->getUnitType()->doesAct()
								&& (!lessActing || (unitActNum & 1) == (this->gameTimer & 1)))
							{
								game_actedAmount++;

								// update unit physics...
								// cannot use this, ai-disabled units should have physics too...
								//if (ai->isThisAndAllEnabled())
								//{
								if (SimpleOptions::getBool(DH_OPT_B_PHYSICS_UPDATE))
								{
									int physTimer = gameTimer;
									if (lessActing)
										physTimer = this->gameTimer / 2;
									UnitPhysicsUpdater::updatePhysics(unit, physics, physTimer);
								}
								//}

								UnitActor *ua = getUnitActorForUnit(unit);
								if (ua != NULL)
								{
									// HACK...
									game::unitactor_acttargeting_counter++;

									// ghost do things fast... :)
									/*
									if (unit->isGhostOfFuture())
									{
										if (!unit->isDestroyed())
										{
											//ua->act(unit);
											ua->act(unit);
											unit->setGhostTime(unit->getGhostTime() + 1);
											//if (unit->getGhostTime() > 5*GAME_TICKS_PER_SECOND)
											if (unit->getGhostTime() > 15*GAME_TICKS_PER_SECOND)
											{
												unit->setDestroyed(true);
											}
										}
									}
									*/

									ua->act(unit);

									unit->advanceMuzzleflashVisualEffect();
									unit->advanceEjectVisualEffect();

									Flashlight *fl = unit->getFlashlight();
									if (fl != NULL)
									{
										VC3 position = unit->getPosition();

										// --- FLASHLIGHT ON SHOULDER ---
										VisualObject *unitVisual = unit->getVisualObject();
										IStorm3D_Model *unitModel = 0;
										if(unitVisual)
										{
											// HACK: to remove flashlight from lagging/shaking...
											unitVisual->prepareForRender();

											if(unitVisual)
												unitModel = unitVisual->getStormModel();

											if(unitModel)
												util::getHelperPosition(unitModel, "HELPER_BONE_ShoulderLamp", position);
										}
										// ---

										fl->run(position);

										VC3 vel = unit->getVelocity();
										if (fabs(vel.x) > 0.5f / GAME_TICKS_PER_SECOND 
											|| fabs(vel.z) > 0.5f / GAME_TICKS_PER_SECOND)
										{
											fl->setSwayFactor(SimpleOptions::getFloat(DH_OPT_F_FLASHLIGHT_SWAY_FACTOR));
											fl->setSwayTime(SimpleOptions::getInt(DH_OPT_I_FLASHLIGHT_SWAY_TIME));
										} else {
											//fl->setSwayFactor(0);
											fl->setSwayFactor(SimpleOptions::getFloat(DH_OPT_F_FLASHLIGHT_SWAY_FACTOR) * 0.5f);
											fl->setSwayTime(SimpleOptions::getInt(DH_OPT_I_FLASHLIGHT_SWAY_TIME) * 10);
										}
									}

								} else {
									Logger::getInstance()->debug("Game::run - Unit with no actor encountered.");
									assert(0);
								}
							}
						}
					}

					UnitPhysicsUpdater::endStats();
/*
Logger::getInstance()->error("Units acted:");
Logger::getInstance()->error(int2str(actedAmount));
Logger::getInstance()->error("Quickly discarded acts:");
Logger::getInstance()->error(int2str(quickNoActAmount));
Logger::getInstance()->error("Slowly discarded acts:");
Logger::getInstance()->error(int2str(slowNoActAmount));
*/
//					int projAmount = 0;

					// move projectiles
					LinkedList *projlist = projectiles->getAllProjectiles();
					// projectiles may be deleted during combat, thus need to
					// use safe iterator!
					SafeLinkedListIterator projiter = SafeLinkedListIterator(projlist);
					while (projiter.iterateAvailable())
					{
						Projectile *proj = (Projectile *)projiter.iterateNext();
						ProjectileActor pa = ProjectileActor(this);
						pa.act(proj);
//						projAmount++;
					}

					/*
Logger::getInstance()->error("Projectiles:");
Logger::getInstance()->error(int2str(projAmount));
*/

					itemManager->run();

					/*
					if (gameUI->getFirstPerson(0) != NULL)
						gameUI->getAmbientSoundManager()->setListenerPosition(gameUI->getFirstPerson(0)->getPosition());
					else
						gameUI->getAmbientSoundManager()->setListenerPosition(gameUI->getGameCamera()->getPosition());
					gameUI->getAmbientSoundManager()->run();
					*/
					gameUI->getAmbientSoundManager()->setListenerPosition(gameUI->getListenerPosition());
					gameUI->getAmbientSoundManager()->run();

					gameUI->getVisualEffectManager()->run();
					//gameUI->getParticleManager()->stepForwards(1);

					// mission failure/success counters and stuff...

					// psd: update terrain
					gameUI->renderTerrain->Animate(10);

					// Give the players damage if they're touching damage units
					for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
					{
						if (gameUI->getFirstPerson(c) != NULL)
						{
							IUnitListIterator *iter = units->getNearbyOwnedUnits(2, gameUI->getFirstPerson(c)->getPosition(), 1);
							while (iter->iterateAvailable())
							{
								Storm3D_CollisionInfo collisionInfo;
								Unit *u = iter->iterateNext();
								Bullet *touchBullet = u->getUnitType()->getTouchBullet();
								if ((u->getUnitType()->getTouchBullet() != NULL) && u->isTouchProjectileEnabled()
									&& (units->getIdForUnit(u)%TOUCHBULLET_INTERVAL == gameTimer%TOUCHBULLET_INTERVAL)) 
								{
									if (u->getUnitType()->hasNoCollision())
									{
										u->getVisualObject()->setCollidable(true);
										u->getVisualObject()->setForcedNoCollision(false);
								}

									u->getVisualObject()->getStormModel()->
										RayTrace(gameUI->getFirstPerson(c)->getPosition()+VC3(0, 20.0f, 0),VC3(0, -1.0f, 0),300.0f,collisionInfo, true);

									if (u->getUnitType()->hasNoCollision())
									{
										u->getVisualObject()->setCollidable(false);
										u->getVisualObject()->setForcedNoCollision(true);
									}

									if (collisionInfo.hit) 
									{
										Projectile *touchproj = new Projectile(NULL, touchBullet);
										projectiles->addProjectile(touchproj);

										VC3 touchpos = gameUI->getFirstPerson(c)->getPosition();

										touchproj->setDirectPath(touchpos, touchpos, touchBullet->getVelocity());
										touchproj->setHitTarget(gameUI->getFirstPerson(c), NULL);

										ProjectileActor pa = ProjectileActor(this);
										pa.createVisualForProjectile(touchproj);

										//gameUI->getFirstPerson(c)->setHP(gameUI->getFirstPerson(c)->getHP() - u->getUnitType()->getTouchDamage());
									}
								}
							}
							delete iter;
							
						}
					}		
			
					// run alien spawner
					{
						VC3 playerAvgPos = VC3(0,0,0);
						int plAmount = 0;
						for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
						{
							if (gameUI->getFirstPerson(c) != NULL
								&& gameUI->getFirstPerson(c)->isActive())
							{
								playerAvgPos += gameUI->getFirstPerson(c)->getPosition();
								plAmount++;
							}
						}
						alienSpawner->run(playerAvgPos);
					}

					// run check about every 2,5 secs
					if ((gameTimer % (2500 / GAME_TICK_MSEC)) == 0)
					{
						gameScripting->runMissionScript(script, "check");
#ifdef _DEBUG
#ifdef FROZENBYTE_DEBUG_MEMORY
						frozenbyte::debug::debugSetAllocationInfo("mission check over");
#endif
#endif
					}

					// cinematic script
					if (cinematicScriptProcess != NULL)
					{
						if (cinematicScriptProcess->isFinished())
						{
							skippingCinematic = false;
							delete cinematicScriptProcess;
							cinematicScriptProcess = NULL;
						} else {
							gameScripting->runScriptProcess(cinematicScriptProcess, 
								!skippingCinematic);
						}
					} else {
						skippingCinematic = false;
					}
	
					runCustomScriptProcesses();

					// update triggers....
					gameScripting->updateAreaTracker();

					// update unit list (tree)
// TEMP
//if ((gameTimer % 3) == 0)
					this->units->updateLists();

					assert(lightBlinker != NULL);
					lightBlinker->run();

					assert(outdoorLightBlinker != NULL);
					outdoorLightBlinker->run();

					if (missionFailureCounter != 0)
					{
						missionFailureCounter--;
						if (missionFailureCounter == 0)
						{
							gameUI->gameMessage(getLocaleGuiString("gui_mission_MISSION_FAILED"), NULL, 3, 2*3000,
								GameUI::MESSAGE_TYPE_CENTER_BIG);
							missionFailureCounter = -1; 					 
						}
						if (missionFailureCounter == -2 * GAME_TICKS_PER_SECOND)
						{
							gameUI->getEffects()->startFadeOutIfNotFaded(2000);
							gameUI->getAmbientAreaManager()->fadeOut(2000);
						}
						// 4 seconds passed since "mission failed"
						if (missionFailureCounter < -4 * GAME_TICKS_PER_SECOND)
						{
							assert(inCombat);
#ifdef PROJECT_SURVIVOR
							if(!missionAborting && !endingCombat && gameUI)
							{
								gameUI->openMissionFailureWindow();
							}
#endif
							endingCombat = true;
						}
					}
					if (missionSuccessCounter != 0)
					{
						missionSuccessCounter--;
						if (missionSuccessCounter == 0)
						{
							gameUI->gameMessage(getLocaleGuiString("gui_mission_MISSION_COMPLETE"), NULL, 3, 2*3000, 
								GameUI::MESSAGE_TYPE_CENTER_BIG);
							missionSuccessCounter = -1;
						}
						if (missionSuccessCounter == -2 * GAME_TICKS_PER_SECOND)
						{
							gameUI->getEffects()->startFadeOutIfNotFaded(2000);
							gameUI->getAmbientAreaManager()->fadeOut(2000);
						}

						// sweet hack
						static bool score_window_opened_in_this_game = false;

						// opens score window in the end of a mission
						if( gameUI->isScoreWindowInUse() && ( missionSuccessCounter < -4 * GAME_TICKS_PER_SECOND ) &&
							score_window_opened_in_this_game == false )
						{
							if( gameUI->isScoreWindowOpen() == false )
							{
								//gameUI->openScoreWindow( singlePlayerNumber );

								score_window_opened_in_this_game = true;
							}
						}

						// Opens loading screen and starts loading?
						// 4 seconds passed since "mission success"
						if ( ( missionSuccessCounter < -4 * GAME_TICKS_PER_SECOND ) && 
							( gameUI->isScoreWindowInUse() == false || gameUI->scoreWindowAllowsLoading() ) )
						{
							if( gameUI->isScoreWindowOpen() )
								gameUI->closeScoreWindow( singlePlayerNumber );
							
							score_window_opened_in_this_game = false;

							assert(inCombat);
							endingCombat = true;

// HACK:...loading screen?????????????????
{
char *foocrap = this->missionId;
char *foocrap2 = this->currentMission;
std::string fooid = std::string(this->nextMissionOnSuccess);

// HACK: some real crap here!
// FIX: less crap now
int end = fooid.find_last_of(".");
#ifdef PROJECT_SURVIVOR
int start = fooid.find_last_of("/");
#else
int s1 = fooid.find_last_of("/");
int s2 = fooid.find_last_of("_");
int start = std::max(s1, s2);
#endif
std::string tmp = fooid.substr(start + 1, end - start - 1);

this->missionId = (char *)tmp.c_str();
this->currentMission = this->nextMissionOnSuccess;

gameUI->openLoadingWindow(singlePlayerNumber);

// HACK: save game here so we have a savegame before the random crash in endCombat
// inside PhysX when deleting game physics
saveGame(strPrintf("%d", (lastSaveNumber + 1)).c_str());

this->missionId = foocrap;
this->currentMission = foocrap2;
}

							gameScripting->setGlobalIntVariableValue("save_requested", 1);
						}
					}

					// run decorations and water...
					if (decorationManager != NULL)
						decorationManager->run();
					//waterManager->run();

					// run anis
					AniManager::getInstance()->run();

					// update difficulty variables, do auto balance, etc.
					difficultyManager->run();

					// run environmental effects
					if (environmentalEffectManager != NULL)
					{
						environmentalEffectManager->run();
					}

					if (gameUI->getFirstPerson(0) != NULL
						&& !isCinematicScriptRunning()
						&& gameUI->getGameCamera() == gameUI->cameras[GAMEUI_CAMERA_NORMAL])
					{
						particleSpawnerManager->setPlayerPosition(gameUI->getFirstPerson(0)->getPosition());
					} else {
#ifdef PROJECT_SHADOWGROUNDS
						particleSpawnerManager->setPlayerPosition(gameUI->getGameCamera()->getPosition());
#else
						particleSpawnerManager->setPlayerPosition(gameUI->getGameCamera()->getActualInterpolatedPosition());
#endif
					}
					particleSpawnerManager->run();

					if (SimpleOptions::getBool(DH_OPT_B_CAMERA_SYSTEM_ENABLED))
					{
						// CAMERA SYSTEM STUFF HERE!
						VC3 pos(0, 0, 0);
						// TODO: Add coop support
						if (gameUI->getFirstPerson(0) != NULL)
						{
							pos = gameUI->getFirstPerson(0)->getPosition();
						}
#ifdef PROJECT_CLAW_PROTO
						gameUI->getCameraSystem()->setAimPosition(clawController->getTargetPosition());
						if (clawController->getPrepareAction() != ClawController::ActionNone) {
							gameUI->getCameraSystem()->setMode(1);
						} else {
							gameUI->getCameraSystem()->setMode(0);
						}
#endif

						gameUI->getCameraSystem()->setResponse(SimpleOptions::getFloat(DH_OPT_F_CAMERA_SYSTEM_RESPONSE));
						gameUI->getCameraSystem()->update(pos, GAME_TICK_MSEC);

					}
				} // if (!paused && !tacticalMode)

				// this gets done even in pause mode...

				// run anirecorder (window)
				if (gameUI->getAniRecorderWindow() != NULL)
				{
					gameUI->getAniRecorderWindow()->run();
				}

				for (int stats = 0; stats < MAX_PLAYERS_PER_CLIENT; stats++)
				{
					game::GameStats::instances[stats]->runCollect();
				}

				// ending combat?
				if ((endingCombat && (missionAborting || (gameUI && !gameUI->isMissionFailureWindowOpen())))
					|| gameUI->isAbortingMission())
				{
					if (endingCombat)
					{
						assert(inCombat);
						endingCombat = false;
						missionAborting = false;
					} else {
						gameUI->setAbortingMission(false);
						missionFailureCounter = -1;  // aborting means failure
						missionAborting = false;
					}

					if (missionSuccessCounter < 0)
					{
						dontLoadMainMenu = true;
					}
					else
					{
						// if we aren't changing to next level, we must
						// unapply certain special upgrades
						upgradeManager->unapplyAll();
					}

					SET_LOADING_BAR_TEXT(getLocaleGuiString("gui_loadingbar_unloading"));
					SHOW_LOADING_BAR(0);

					endCombat();

					SHOW_LOADING_BAR(25);

					MissionParser mp = MissionParser();
					mp.parseMission(this, currentMission, 
						MISSIONPARSER_SECTION_AFTER);

#ifdef LEGACY_FILES
					gameScripting->loadScripts("Config/user_autoexec.dhs", NULL);
#else
					gameScripting->loadScripts("config/user_autoexec.dhs", NULL);
#endif
					gameScripting->runMissionScript("user_autoexec", "runafter");

					if (sectionScript != NULL)
					{
						gameScripting->runMissionScript(sectionScript, "runafter");
						setSectionScript(NULL);
					}

					if (missionSuccessCounter >= 0 && missionFailureCounter >= 0)
					{
						Logger::getInstance()->warning("Game::run - Combat ended, but with neither failure nor success.");
						Logger::getInstance()->warning("Game::run - Thus current state is undefined.");
						assert(!"Game::run - Combat ended, but with neither failure nor success.");
					}

					if (missionSuccessCounter < 0)
					{
						gameScripting->runMissionScript(script, "success");

						// skip menus and automagically start next mission...
						automaticallyStartMission = true;
						automaticallyStartMission_loop2 = false;
					} else {
						if (missionFailureCounter < 0)
						{
							gameScripting->runMissionScript(script, "failure");
						}
					}

					// NEW: script cleanup moved here.
					Timer::update();
					int scriptCleanupStartTime = Timer::getTime();

					delete gameScripting;
					util::ScriptManager::cleanInstance();
					gameScripting = new GameScripting(this);

					tracking::ScriptableTrackerObjectType::setGameScripting(gameScripting);

					Timer::update();
					int scriptCleanupEndTime = Timer::getTime();
					int scriptCleanupTotalTime = scriptCleanupEndTime - scriptCleanupStartTime;
					Logger::getInstance()->debug("Game::run - Script cleanup done, time used (msec) follows:");
					Logger::getInstance()->debug(int2str(scriptCleanupTotalTime));

					AniManager::createInstance(gameScripting);
					// end of script cleanup

					mp.parseMission(this, currentMission, 
						MISSIONPARSER_SECTION_BEFORE);

#ifdef LEGACY_FILES
					gameScripting->loadScripts("Config/user_autoexec.dhs", NULL);
#else
					gameScripting->loadScripts("config/user_autoexec.dhs", NULL);
#endif
					gameScripting->runMissionScript("user_autoexec", "runbefore");

					if (sectionScript != NULL)
					{
						gameScripting->runMissionScript(sectionScript, "runbefore");
						setSectionScript(NULL);
					}

					SHOW_LOADING_BAR(40);

					if(gameUI->getEffects())
						gameUI->getEffects()->setNoFade();

				} // if (endingCombat ... )

			} else {
				// TODO, menu stuff...

				// don't do this here, must do it in runUI maybe (in command window)?
				/*
				if (getPendingLoad() != NULL)
				{
					if (gameUI->commandWindows[singlePlayerNumber] != NULL)
					{
						gameUI->closeCommandWindow(singlePlayerNumber);
						gameUI->openLoadingWindow(singlePlayerNumber); 
					}
				}
				*/

				// TODO: this should be done by a game request...
				//if (gameUI->readyToRockNRoll())

				// CANNOT DO THIS HERE, AS GAME RUN MAY BE CALLED SEVERAL TIMES BEFORE FALLING
				// BACK TO WIN MAIN...
				// (MOVED TO SEPERATE advanceMissionStartState CALL)
				//if (automaticallyStartMission)
				//{
					// HACK: need to make sure that the game loops at least once thru the
					// winmain...
				//	automaticallyStartMission_loop2 = true;
				//	automaticallyStartMission = false;
				//} else {
					if (automaticallyStartMission_loop2)
					{
						Logger::getInstance()->debug("Game::run - Automatically starting mission (phase 2).");

						assert(!automaticallyStartMission);

						if (!gameUI->isLoadingWindowVisible())
						{
							gameUI->openLoadingWindow(singlePlayerNumber);
						}
						else
						{
							gameUI->redrawLoadingWindow(singlePlayerNumber);
						}

						SET_LOADING_BAR_TEXT(getLocaleGuiString("gui_loadingbar_loading"));
						SHOW_LOADING_BAR(15);

						automaticallyStartMission_loop2 = false;
						for (int i = 0; i < ABS_MAX_PLAYERS; i++)
						{
							spawnX[i] = 0;
							spawnY[i] = 0;
						}
						MissionParser mp = MissionParser();

						mp.parseMission(this, currentMission, 
							MISSIONPARSER_SECTION_COMBAT);

						SHOW_LOADING_BAR(25);

						startCombat();

						// some special upgrades need to be re-applied after loading
						// a new map (mission success, not loading a savegame)
						//
						upgradeManager->reapplyAllUpgrades(NULL);

						// remove loading message
						ui::LoadingMessage::clearLoadingMessage();
					}
				//}
			}


			// update interpolation history for units...
			{
				LinkedList *ulist = units->getAllUnits();
				LinkedListIterator unititer = LinkedListIterator(ulist);
				while (unititer.iterateAvailable())
				{
					Unit *unit = (Unit *)unititer.iterateNext();
					if (unit->isActive())
					{
						if (unit->getVisualObject() != NULL)
						{
							unit->getVisualObject()->advanceHistory();
						}
					}
				}
			}

			lastTickTime = Timer::getTime();
			gameTimer++;
		}

	}

	bool Game::isCooperative() const
	{
		return cooperative;
	}

	void Game::setCooperative( bool coop )
	{
		cooperative = coop;

		if( coop )
		{
			// remember current camera setting
			if(SimpleOptions::getBool( DH_OPT_B_GAME_MODE_AIM_UPWARD ) == true)
				shouldResetAimUpwardMode = true;

			// force aim_upward 0
			gameUI->getGameCamera()->setGameCameraMode( false );
			SimpleOptions::setBool( DH_OPT_B_GAME_MODE_AIM_UPWARD, false );
		}
		else
		{
			// reset camera mode if needed
			if(shouldResetAimUpwardMode)
			{
				SimpleOptions::setBool( DH_OPT_B_GAME_MODE_AIM_UPWARD, true );
				shouldResetAimUpwardMode = false;
			}

			SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_ENABLED, true );
			SimpleOptions::setBool( DH_OPT_B_2ND_PLAYER_ENABLED, false );
			SimpleOptions::setBool( DH_OPT_B_3RD_PLAYER_ENABLED, false );
			SimpleOptions::setBool( DH_OPT_B_4TH_PLAYER_ENABLED, false );

			if( getGameUI()->getController( 0 )->controllerTypeHasMouse() )
			{
				SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, true );
			}
			else
			{
				SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, false );
			}
			SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME, getGameUI()->getController( 0 )->getControllerType() );

			gameUI->getController( 1 )->unloadConfiguration();
			gameUI->getController( 2 )->unloadConfiguration();
			gameUI->getController( 3 )->unloadConfiguration();
		}
	}


	void Game::advanceMissionStartState()
	{
		if (!endingCombat)
		{
			if (automaticallyStartMission)
			{
				// HACK: need to make sure that the game loops at least once thru the
				// winmain...
				automaticallyStartMission_loop2 = true;
				automaticallyStartMission = false;

				Logger::getInstance()->debug("Game::run - Automatically starting mission requested (phase 1).");
			}
		}
	}

	void Game::sendRequest(GameRequest *request)
	{
		if (server)
		{
			receiveRequest(request);
		} else {
			// TODO: send the data to master via network if multiplayer game
		}
	}

	void Game::receiveRequest(GameRequest *request)
	{
		if (!server)
		{
			// none, but server (master) should receive requests
			// just skip it?
			// TODO: log
			delete request;
		} else {
			request->executeTime = gameTimer + syncInterval;
			sendOrder(request);
		}
	}

	void Game::sendOrder(GameRequest *request)
	{
		if (!server)
		{
			// clients (slaves) cannot send orders, only requests
			// TODO: log
			delete request;
		} else {
			if (multiplayer)
			{
				// TODO: echo the order data via network to all clients
			}
			receiveOrder(request);
		}
	}

	void Game::receiveOrder(GameRequest *request)
	{
		orderQueue->append(request);
	}

// HACK: ...
bool game_in_start_combat = false;

	void Game::startCombat()
	{
		// loaded by terrain creator? ( gameUI->setRenderMap() below )
		//gameMap->loadMap("Data/Maps/hmap512_16b.raw");
		game_in_start_combat = true;

		Timer::update();
		int timeAtCombatStart = Timer::getTime();

#ifdef PROJECT_SURVIVOR
		// re-enable AI
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
			UnitLevelAI::setPlayerAIEnabled(i, true);
#endif

#ifdef PROJECT_SURVIVOR
		// apply new options
		bonusManager->applyOptions(true, BonusManager::APPLYING_BEFORE_LOAD);
#endif

		for (int p = 0; p < ABS_MAX_PLAYERS; p++)
		{
			gameUI->closeArmorConstructWindow(p);
			gameUI->closeCommandWindow(p);
		}

		gameUI->setRenderMap(gameMap, currentMap);

		SHOW_LOADING_BAR(45);

		gameScene = new GameScene(gameUI->storm3d, gameUI->scene, 
			gameUI->renderTerrain, gameMap);
		gameUI->getVisualEffectManager()->setGameScene(gameScene);

		// TODO: this size should be halved!
		units->recreateLists(VC2(gameMap->getScaledSizeX(), gameMap->getScaledSizeY()));

		SHOW_LOADING_BAR(50);

		assert(particleSpawnerManager == NULL);
		particleSpawnerManager = new ParticleSpawnerManager(this);

		formations.setGameScene(gameScene);

		// try to load obstaclemap and areamap
		gameMap->loadObstacleAndAreaMap(gameScene->getPathFinder());

		decorationManager = new DecorationManager();
		waterManager = new WaterManager();
		lightBlinker = new LightBlinker(this, false);
		outdoorLightBlinker = new LightBlinker(this, true);
		gameUI->setCamerasWaterManager();

		map.reset(new Map(*this));
		map->setMission(currentMap);

		SHOW_LOADING_BAR(55);

		if (script == NULL)
		{
			Logger::getInstance()->warning("Game::startCombat - No mission script set.");
		}
		gameScripting->runMissionScript(script, "begin");

		SHOW_LOADING_BAR(56);

		// set visual objects for buildings
		// make obstacle maps for them

		// fill area map with terrain material
		gameScene->initTerrainMaterial();

		SHOW_LOADING_BAR(57);

#ifdef LEGACY_FILES
		// nop
#else
		gameScripting->runMissionScript("spotlight_init", "main");
#endif

		gameScripting->runMissionScript(buildingsScript, "create");

		SHOW_LOADING_BAR(58);

		unsigned char *clipMap = gameScene->generateTerrainTexturing();

		SHOW_LOADING_BAR(60);

		// require physics cache recreate?
		std::string obstFilename = std::string(currentMap) + std::string("/scene.bin");
		std::string cacheFilename = std::string(currentMap) + std::string("/pcache.bin");
		bool needPhysCacheRecreate = !FileTimestampChecker::isFileUpToDateComparedTo(cacheFilename.c_str(), obstFilename.c_str());

		bool loadedCache = false;

		if (SimpleOptions::getBool(DH_OPT_B_FORCE_PHYSICS_CACHE_RECREATE)
			|| (SimpleOptions::getBool(DH_OPT_B_AUTO_PHYSICS_CACHE_RECREATE) && needPhysCacheRecreate))
		{
			Logger::getInstance()->debug("About to simulate physics to stable state for cache file.");
			Timer::update();
			int startTimeForPhysSim = Timer::getUnfactoredTime();

			// no hardware accel when recreating physics cache (as it cannot handle the initial 4000+ contacts)
			bool wasHardware = SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_HARDWARE);
			if (wasHardware)
			{
				Logger::getInstance()->debug("Temporarily disabled hardware physics for cached physics state stabilization.");
				SimpleOptions::setBool(DH_OPT_B_PHYSICS_USE_HARDWARE, false);
			}

			physics->createPhysics(gameScripting);
			bool usedDynamicObst = gameUI->getTerrain()->doesUseDynamicObstacles();
			gameUI->getTerrain()->setUseDynamicObstacles(false);
			gameUI->getTerrain()->createPhysics(physics, clipMap, false, currentMap);

			physics->setIgnoreContacts(true);
			// simulate for a while... (about 180 secs or so ok?)
			for (int psim = 0; psim < 180 * GAME_TICKS_PER_SECOND; psim++)
			{
				physics->runPhysics(gameUI->getStormScene(), gameUI->getVisualEffectManager()->getParticleEffectManager());
				std::vector<TerrainObstacle> tmplist;
				gameUI->getTerrain()->updatePhysics(physics, tmplist, NULL);
			}
			physics->setIgnoreContacts(false);
			gameUI->getTerrain()->setUseDynamicObstacles(usedDynamicObst);

			LinkedListIterator biter(buildings->getAllBuildings());
			while (biter.iterateAvailable())
			{
				Building *b = (Building *)biter.iterateNext();
				b->deletePhysics(physics);
			}

			gameUI->getTerrain()->savePhysicsCache(physics, currentMap);

			gameUI->getTerrain()->deletePhysics(physics);
			physics->deletePhysics();

			if (wasHardware)
			{
				SimpleOptions::setBool(DH_OPT_B_PHYSICS_USE_HARDWARE, wasHardware);
			}

			LinkedListIterator biter2(buildings->getAllBuildings());
			while (biter2.iterateAvailable())
			{
				Building *b = (Building *)biter2.iterateNext();
				b->addPhysics(physics);
			}

			Timer::update();
			int endTimeForPhysSim = Timer::getUnfactoredTime();
			Logger::getInstance()->debug("Stabilizing simulation done, time used follows:");
			Logger::getInstance()->debug(int2str(endTimeForPhysSim - startTimeForPhysSim));
		} else {
			Logger::getInstance()->debug("About to load physics state cache file.");
			gameUI->getTerrain()->loadPhysicsCache(physics, currentMap);
			Logger::getInstance()->debug("Physics state cache file loaded.");
			loadedCache = true;
		}

		SHOW_LOADING_BAR(65);

		// terrain obstacles...

		//std::vector<TerrainObstacle> &obstacleList = gameUI->renderTerrain->getObstacles();
		std::vector<TerrainObstacle> obstacleList;
		gameUI->renderTerrain->getObstacles(obstacleList);
		gameScene->addTerrainObstacles(obstacleList);

		gameMap->createCoverMap();

		// now, we should have either loaded the hidemap or created it
		// try saving it. (done only if it was created, not loaded)
//		gameMap->saveHideMap();

		gameMap->saveObstacleAndAreaMap(gameScene->getPathFinder());

		// set visual objects for all parts in units,
		// create ai for units,
		// move units to spawn...

		SHOW_LOADING_BAR(70);

#ifdef LEGACY_FILES
		gameScripting->loadScripts("Config/user_autoexec.dhs", NULL);
#else
		gameScripting->loadScripts("config/user_autoexec.dhs", NULL);
#endif
		gameScripting->runMissionScript("user_autoexec", "runcombat");

		// now, we can run the mission script, not before, cos the unit
		// spawning would not be able to solve proper coordinates before
		// loading the map first.
		if (sectionScript != NULL)
		{
			gameScripting->runMissionScript(sectionScript, "runcombat");
			setSectionScript(NULL);
		}

		if (loadedCache)
		{
			Timer::update();
			int startTimeForRelight = Timer::getUnfactoredTime();
			Logger::getInstance()->debug("About to relight all physics objects loaded from cache...");

			gameUI->getTerrain()->updateAllPhysicsObjectsLighting();

			Timer::update();
			int endTimeForRelight = Timer::getUnfactoredTime();
			Logger::getInstance()->debug("Physics object relighting done, time used follows:");
			Logger::getInstance()->debug(int2str(endTimeForRelight - startTimeForRelight));
		}

		SHOW_LOADING_BAR(75);

		LinkedList *ulist = units->getAllUnits();
		LinkedListIterator uiter = LinkedListIterator(ulist);
		while (uiter.iterateAvailable())
		{
			Unit *u = (Unit *)uiter.iterateNext();
			Part *p = u->getRootPart();
			if (p != NULL)
			{
				// remove parts that have not been paid for
				removeUnpurchasedParts(u, p);
				// need to get the root part again, as it may have been removed above
				p = u->getRootPart();
				// make the unit active or inactive - based on if it is ok for combat
				// also make a visualization for it if active

				// WARNING: COPY & PASTE PROGRAMMING AHEAD (SEE Game.cpp)

				/*
				bool armorOk = true;

				if (p == NULL)
				{
					armorOk = false;
				} else {
					// TODO: this is copy paste programming from commandwindow. 
					// do proper method for this check...
					int slotAmount = p->getType()->getSlotAmount();
					for (int i = 0; i < slotAmount; i++) 
					{
						if (p->getSubPart(i) == NULL)
						{
							if (
								p->getType()->getSlotType(i)->getPartTypeId() 
								== PARTTYPE_ID_STRING_TO_INT("Head")
								|| p->getType()->getSlotType(i)->getPartTypeId() 
								== PARTTYPE_ID_STRING_TO_INT("Arm")
								|| p->getType()->getSlotType(i)->getPartTypeId() 
								== PARTTYPE_ID_STRING_TO_INT("Leg")
								|| p->getType()->getSlotType(i)->getPartTypeId() 
								== PARTTYPE_ID_STRING_TO_INT("Reac")
								)
							{
								armorOk = false;
							}
						}
					}
				}
				if (u->getCharacter() == NULL
					&& strcmp(u->getUnitType()->getName(), "Armor") == 0)
				{
					armorOk = false;
				}
				*/

				/*
				if (armorOk
					|| (p != NULL && isComputerOpponent(u->getOwner())))
				{
				*/
					UnitSpawner::spawnUnit(this, u);
					/*
				} else {
					// these are actually useless, as the retireUnit should
					// have done these things(?)
					u->setActive(false);
					u->setSelected(false);
					u->setAI(NULL);
				}
				*/
			}
			/*
			VC3I pos = VC3I(
				gameMap->getScaledSizeX() / 2, 
				gameMap->, 
				gameMap->getScaledSizeY() / 2);
			u->setPosition(pos);			
			*/
		}


		missionAborting = false;
		missionFailureCounter = 0;
		missionSuccessCounter = 0;

		missionStartTime = gameTimer;
		pauseStartTime = 0;
		missionPausedTime = 0;
		lastTickTime = Timer::getTime();

		endingCombat = false;
		inCombat = true;


		SHOW_LOADING_BAR(80);

		gameUI->createBuildingLighting();
gameUI->getTerrain()->calculateLighting();

		for (int seli = 0; seli < ABS_MAX_PLAYERS; seli++)
		{
			unitSelections[seli]->reset();
		}

		visibilityChecker->restart();

		SHOW_LOADING_BAR(95);

		//gameUI->closeLoadingWindow(singlePlayerNumber); // TODO: for players on this client in netgame...
		gameUI->doneLoading(singlePlayerNumber);

#ifndef PROJECT_SHADOWGROUNDS
		gameScripting->runMissionScript("player_selection", "activation");
#else
		// NEW: now handled by a script... or maybe not...		
		Unit *firstUnit = NULL;
		Unit *secondUnit = NULL;
		Unit *thirdUnit = NULL;
		Unit *fourthUnit = NULL;
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			LinkedList *firstulist = units->getOwnedUnits(singlePlayerNumber);
			LinkedListIterator iter(firstulist);
			if(iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED))
				{
					firstUnit = u;
				} else {
					UnitActor *ua = getUnitActorForUnit(u);
					ua->removeUnitObstacle(u);
					UnitSpawner::retireUnit(this, u);
				}
			} else {
				Logger::getInstance()->warning("Game::startCombat - No first unit available.");
			}
			if(iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
				{
					secondUnit = u;
				} else {
					UnitActor *ua = getUnitActorForUnit(u);
					ua->removeUnitObstacle(u);
					UnitSpawner::retireUnit(this, u);
				}
			} else {
				Logger::getInstance()->warning("Game::startCombat - No second unit available.");
			}
			if(iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
				{
					thirdUnit = u;
				} else {
					UnitActor *ua = getUnitActorForUnit(u);
					ua->removeUnitObstacle(u);
					UnitSpawner::retireUnit(this, u);
				}
			} else {
				Logger::getInstance()->warning("Game::startCombat - No third unit available.");
			}
			if(iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED))
				{
					fourthUnit = u;
				} else {
					UnitActor *ua = getUnitActorForUnit(u);
					ua->removeUnitObstacle(u);
					UnitSpawner::retireUnit(this, u);
				}
			} else {
				Logger::getInstance()->warning("Game::startCombat - No fourth unit available.");
			}
		}
#endif

		// prepare all units' ai...
		LinkedListIterator prepareiter = LinkedListIterator(ulist);
		while (prepareiter.iterateAvailable())
		{
			Unit *u = (Unit *)prepareiter.iterateNext();
			// WARNING: unsafe cast!
			UnitLevelAI *ai = (UnitLevelAI *)u->getAI();
			if (ai != NULL)
			{
				ai->prepareMainScript();
			}
		}

		// TODO: FIXME: why do we need the *2 here? 
		// *1 is not enough, but why is that... it should be. I think.

		for (int visc = 0; visc < units->getAllUnitAmount() 
			* VISIBILITY_CHECK_IN_PASSES * 2; visc++)
		{
			visibilityChecker->runCheck();
		}
		// start in invisible mode
		gameUI->openCombatWindow(singlePlayerNumber, true); // TODO: for players on this client in netgame...


		// to get hp meters updated.
		gameUI->setUnitDamagedFlag(singlePlayerNumber);

		// cameras near player 0's spawn
		for (int cam = 0; cam < GAMEUI_CAMERA_AMOUNT; cam++)
		{
			gameUI->selectCamera(cam);
			int camxi = spawnX[singlePlayerNumber];
			int camyi = spawnY[singlePlayerNumber];
			//camx = (camx - (float)(gameMap->getSizeX()/2)) * gameMap->getScaleX();
			//camy = (camy - (float)(gameMap->getSizeY()/2)) * gameMap->getScaleY();
			float camx = gameMap->configToScaledX(camxi);
			float camy = gameMap->configToScaledY(camyi);
			gameUI->getGameCamera()->setPositionNear(camx, camy);
		}
		// then start with normal camera
		gameUI->selectCamera(GAMEUI_CAMERA_NORMAL);

		// we are not in tactical mode now
		tacticalMode = false;

		// actually we're paused... (until the loading window is closed)
		//paused = true;
		setPaused(true);

		//gameUI->getMusicPlaylist(singlePlayerNumber)->setBank(PLAYLIST_CALM);
		//gameUI->getMusicPlaylist(singlePlayerNumber)->play();

		std::string pendingLoadBackup;
		if (this->getPendingLoad() != NULL)
		{
			pendingLoadBackup = this->getPendingLoad();
			this->applyPendingLoad();
		}

		// NEW: now handled by script...
#ifndef PROJECT_SHADOWGROUNDS
		gameScripting->runMissionScript("player_selection", "control_selection");
#else
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			if (firstUnit != NULL)
				gameUI->setFirstPerson(singlePlayerNumber, firstUnit, 0);
			if (secondUnit != NULL)
				gameUI->setFirstPerson(singlePlayerNumber, secondUnit, 1);
			if (thirdUnit != NULL)
				gameUI->setFirstPerson(singlePlayerNumber, thirdUnit, 2);
			if (fourthUnit != NULL)
				gameUI->setFirstPerson(singlePlayerNumber, fourthUnit, 3);
			//visibilityChecker->setUpdateEnabled(false);
		}
#endif		

		gameUI->missionStarted();

		gameScripting->runMissionScript("user_autoexec", "runcombat2");
		if(environmentalEffectManager)
			environmentalEffectManager->init();

		if(!pendingLoadBackup.empty())
		{
			setPendingLoad(pendingLoadBackup.c_str());
		}
		if (this->getPendingLoad() != NULL)
		{
			this->applyPendingLoad();
		}

#ifdef PROJECT_SURVIVOR
		// override savegame options
		bonusManager->applyOptions(true, BonusManager::APPLYING_AFTER_LOAD);
#endif

		for (int stats = 0; stats < MAX_PLAYERS_PER_CLIENT; stats++)
		{
			game::GameStats::instances[stats]->start(this->missionId);
		}

		if (SimpleOptions::getBool(DH_OPT_B_PHYSICS_ENABLED))
		{
			Logger::getInstance()->debug("Game::startCombat - About to create physics simulation objects...");

			physics->createPhysics(gameScripting);
			gameUI->getTerrain()->createPhysics(physics, clipMap, true, currentMap);

			Logger::getInstance()->debug("Game::startCombat - Physics created.");
		} else {
			Logger::getInstance()->debug("Game::startCombat - Physics are not enabled, so skipping physics object creation.");
		}

		delete[] clipMap;

		int timeAtCombatEnd = Timer::getTime();
		int totalTimeUsed = timeAtCombatEnd - timeAtCombatStart;
		Logger::getInstance()->debug("Game::startCombat - Start done, time used follows (msec):");
		Logger::getInstance()->debug(int2str(totalTimeUsed));

		game_in_start_combat = false;
	}


	void Game::endCombat()
	{
		Logger::getInstance()->debug("Game::endCombat - About to delete physics simulation objects...");

		physics->prepareForDelete();

		if(gameUI && gameUI->getEffects())
		{
			IStorm3D_TerrainRenderer *rend = NULL;
			if(gameUI->getTerrain())
				rend = &gameUI->getTerrain()->GetTerrain()->getRenderer();
			gameUI->getEffects()->endAllFlashEffects(rend);
		}

		if(gameUI && gameUI->getVisualEffectManager() && gameUI->getVisualEffectManager()->getParticleEffectManager())
			gameUI->getVisualEffectManager()->getParticleEffectManager()->releasePhysicsResources();

		if(gameUI && gameUI->getTerrain())
			gameUI->getTerrain()->releasePhysicsResources();

		physics->deletePhysics();

		Logger::getInstance()->debug("Game::endCombat - Physics deleted.");

		for (int stats = 0; stats < MAX_PLAYERS_PER_CLIENT; stats++)
		{
			game::GameStats::instances[stats]->end();
		}

		objectTracker->deleteAllTrackers();
		tracking::TrackableUnifiedHandleObject::cleanupPool();
		tracking::TrackableUnifiedHandleObject::setGame(this);


		alienSpawner->reset();

		AniManager::cleanInstance();

		if (cinematicScriptProcess != NULL)
		{
			delete cinematicScriptProcess;
			cinematicScriptProcess = NULL;
		}

		stopAllCustomScriptProcesses();

		gameUI->endForcedBuildingRoof();

		gameUI->detachVisualEffects();

		// delete projectiles
		LinkedList *projlist = projectiles->getAllProjectiles();
		SafeLinkedListIterator projIter = SafeLinkedListIterator(projlist);
		while (projIter.iterateAvailable())
		{
			Projectile *p = (Projectile *)projIter.iterateNext();
			projectiles->removeProjectile(p);
			delete p;
		}

		assert(particleSpawnerManager != NULL);
		delete particleSpawnerManager;
		particleSpawnerManager = NULL;

		// delete items
		delete itemManager;
		itemManager = new ItemManager(this);

		delete upgradeManager;
		upgradeManager = new UpgradeManager(this);

		gameUI->getAmbientSoundManager()->clearAllAmbientSounds();

		// clear building handler
		gameUI->buildingHandler.clear();
		gameUI->buildingHandler.setUpdateEnabled(true);

		// delete buildings
		LinkedList *blist = buildings->getAllBuildings();
		SafeLinkedListIterator bIter = SafeLinkedListIterator(blist);
		while (bIter.iterateAvailable())
		{
			Building *b = (Building *)bIter.iterateNext();
			buildings->removeBuilding(b);
			delete b;
		}

		if (decorationManager != NULL)
		{
			delete decorationManager;
			decorationManager = NULL;
		}
		if (waterManager != NULL)
		{
			delete waterManager;
			waterManager = NULL;
		}
		if (lightBlinker != NULL)
		{
			delete lightBlinker;
			lightBlinker = NULL;
		}
		if (outdoorLightBlinker != NULL)
		{
			delete outdoorLightBlinker;
			outdoorLightBlinker = NULL;
		}
		gameUI->setCamerasWaterManager();

		// retire units from the battlefield.

		// delete unit level ai
		// delete computer units 
		// clear targets and stuff
		// uninit weapons
		// clear spawn coordinates
		// TODO: loot from destroyed ones?
		// TODO: delete fully damaged parts
		LinkedList *ulist = units->getAllUnits();
		SafeLinkedListIterator uiter = SafeLinkedListIterator(ulist);
		while (uiter.iterateAvailable())
		{
			Unit *u = (Unit *)uiter.iterateNext();
			UnitSpawner::retireUnit(this, u);
		}

		devUnit = NULL;

		// TODO: for players on this client in netgame...
		gameUI->closeCombatWindow(singlePlayerNumber);
		gameUI->closeLoadingWindow(singlePlayerNumber);

		gameUI->getMusicPlaylist(singlePlayerNumber)->setBank(PLAYLIST_MENUS);
		gameUI->getMusicPlaylist(singlePlayerNumber)->play();

		assert(gameUI->getVisualEffectManager() != NULL);
		gameUI->getVisualEffectManager()->setGameScene(NULL);

		gameUI->setRenderMap(NULL, NULL);
		
		if (gameScene != NULL)
		{
			delete gameScene;
			gameScene = NULL;
		}

		inCombat = false;

		// TODO: NETGAME: open for all human players!
		if (this->pendingLoad != NULL
			|| dontLoadMainMenu)
		{
			if (dontLoadMainMenu)
			{
				dontLoadMainMenu = false;
				char *foocrap = this->missionId;
				std::string fooid = std::string(this->nextMissionOnSuccess);

				// HACK: some real crap here!
				// FIX: less crap now
				int end = fooid.find_last_of(".");
#ifdef PROJECT_SURVIVOR
				int start = fooid.find_last_of("/");
#else
				int s1 = fooid.find_last_of("/");
				int s2 = fooid.find_last_of("_");
				int start = std::max(s1, s2);
#endif
				std::string tmp = fooid.substr(start + 1, end - start - 1);
				this->missionId = (char *)tmp.c_str();
				
				gameUI->openLoadingWindow(singlePlayerNumber);
				this->missionId = foocrap;
			} else {
				gameUI->openLoadingWindow(singlePlayerNumber);
			}
		} else {
			gameUI->openCommandWindow(singlePlayerNumber);
			gameUI->startCommandWindow( singlePlayerNumber );
		}
		//for (int p = 0; p < ABS_MAX_PLAYERS; p++)
		//{
		//	gameUI->openCommandWindow(p);
		//}

		assert(!endingCombat);
		endingCombat = false;

		// OLD: game scripting was cleaned up here... 
		// which was a bad place to do it, as the scripts get immediately reloaded by run_after mission scripts and stuff
		/*
		Timer::update();
		int scriptCleanupStartTime = Timer::getTime();

		delete gameScripting;
		util::ScriptManager::cleanInstance();
		gameScripting = new GameScripting(this);

		tracking::ScriptableTrackerObjectType::setGameScripting(gameScripting);

		Timer::update();
		int scriptCleanupEndTime = Timer::getTime();
		int scriptCleanupTotalTime = scriptCleanupEndTime - scriptCleanupStartTime;
		Logger::getInstance()->debug("Game::endCombat - Script cleanup done, time used (msec) follows:");
		Logger::getInstance()->debug(int2str(scriptCleanupTotalTime));

		AniManager::createInstance(gameScripting);
		*/

		::next_interface_generation_request = true;

		gameUI->missionEnded();

		if(gameUI->isQuitRequested() && shouldResetAimUpwardMode)
		{
			// reset camera mode on quit too
			SimpleOptions::setBool( DH_OPT_B_GAME_MODE_AIM_UPWARD, true );
			shouldResetAimUpwardMode = false;
		}

#ifdef PROJECT_SURVIVOR
		// unapply old options
		bonusManager->applyOptions(false, BonusManager::APPLYING_AFTER_LOAD);

		// reset water level
		SimpleOptions::setFloat(DH_OPT_F_PHYSICS_WATER_HEIGHT, 0.0f);
#endif
	}



	bool Game::isActivePlayer(int player)
	{
		// TODO, proper implementation
		return true;
	}

	bool Game::isComputerOpponent(int player)
	{
		// TODO, proper implementation
		if (player == singlePlayerNumber) 
			return false;
		else 
			return true;
	}

	/*
	bool Game::isHostile(int player, int otherPlayer)
	{
		return hostile[player][otherPlayer];
	}
	*/

	bool Game::isServer()
	{
		// TODO, netgame
		return true;
	}


	bool Game::isMultiplayer()
	{
		// TODO, netgame
		return false;
	}

	void Game::runCustomUIScriptProcesses()
	{
		// custom UI scripts
		//runCustomScriptProcesses();
		// TODO: only such custom script processes that are marked for UI cycle...
	}

	void Game::runCustomScriptProcesses()
	{
		// TODO: only such custom script processes that are marked for non-ui (game) cycle...
		if (customScriptProcesses != NULL)
		{
			SafeLinkedListIterator cspiter(customScriptProcesses);
			while (cspiter.iterateAvailable())
			{
				util::ScriptProcess *sp = (util::ScriptProcess *)cspiter.iterateNext();
				fb_assert(sp != NULL);
				if (sp->isFinished())
				{
					delete static_cast<GameScriptData *> (sp->getData());
					delete sp;
					customScriptProcesses->remove(sp);
				} else {
					gameScripting->runScriptProcess(sp, true);
				}
			}
		}
	}

	inline void writeVariable(util::TextFileModifier &tfm, const char *varname)
	{
		if (util::Script::getGlobalVariableType(varname) == SCRIPT_DATATYPE_INT)
		{
			int varval = 0;
			util::Script::getGlobalIntVariableValue(varname, &varval);

			char *tmpbuf = new char[strlen(varname) + 256];
			strcpy(tmpbuf, "permanentGlobal int,");
			strcat(tmpbuf, varname);
			strcat(tmpbuf, "; setValue ");
			strcat(tmpbuf, int2str(varval));
			strcat(tmpbuf, "; setVariable ");
			strcat(tmpbuf, varname);
			strcat(tmpbuf, "\n");

			tfm.setBothSelectionsToEnd();
			tfm.addAfterSelection(tmpbuf);

			delete[] tmpbuf;
		} 
		else if (util::Script::getGlobalVariableType(varname) == SCRIPT_DATATYPE_STRING)
		{
			const char *varval = NULL;
			util::Script::getGlobalStringVariableValue(varname, &varval);

			char *tmpbuf = new char[strlen(varname) + 512];
			strcpy(tmpbuf, "permanentGlobal string,");
			strcat(tmpbuf, varname);
			if (varval == NULL)
			{
				strcat(tmpbuf, "; setNullString");
				strcat(tmpbuf, "; setStringVariable ");
				strcat(tmpbuf, varname);
			} else {
				strcat(tmpbuf, "; setStringValue \"");
				strcat(tmpbuf, varval);
				strcat(tmpbuf, "\"; setStringVariable ");
				strcat(tmpbuf, varname);
			}
			strcat(tmpbuf, "\n");

			tfm.setBothSelectionsToEnd();
			tfm.addAfterSelection(tmpbuf);

			delete[] tmpbuf;
		}
		else if (util::Script::getGlobalVariableType(varname) == SCRIPT_DATATYPE_FLOAT)
		{
			float varval = 0;
			util::Script::getGlobalFloatVariableValue(varname, &varval);

			char *tmpbuf = new char[strlen(varname) + 256];
			strcpy(tmpbuf, "permanentGlobal float,");
			strcat(tmpbuf, varname);
			strcat(tmpbuf, "; setFloatValue ");
			char fbuf[16]; sprintf(fbuf, "%f", varval);
			strcat(tmpbuf, fbuf);
			strcat(tmpbuf, "; setFloatVariable ");
			strcat(tmpbuf, varname);
			strcat(tmpbuf, "\n");

			tfm.setBothSelectionsToEnd();
			tfm.addAfterSelection(tmpbuf);

			delete[] tmpbuf;
		} 
		else if (util::Script::getGlobalVariableType(varname) == SCRIPT_DATATYPE_ARRAY)
		{
			int varsize = 0;
			util::Script::getGlobalArrayVariableSize(varname, &varsize);

			if (varsize > 0)
			{
				char *tmpbuf = new char[strlen(varname) + varsize * 16 + 256];
				strcpy(tmpbuf, "permanentGlobal int[");
				strcat(tmpbuf, int2str(varsize));
				strcat(tmpbuf, "],");
				strcat(tmpbuf, varname);
				strcat(tmpbuf, "; setArrayVariableValues ");
				strcat(tmpbuf, varname);
				strcat(tmpbuf, ",");

				for (int i = 0; i < varsize; i++)
				{
					int varval = 0;
					util::Script::getGlobalArrayVariableValue(varname, i, &varval);

					strcat(tmpbuf, int2str(varval));
					if (i < varsize - 1)
						strcat(tmpbuf, ",");
				}

				strcat(tmpbuf, "\n");

				tfm.setBothSelectionsToEnd();
				tfm.addAfterSelection(tmpbuf);

				delete[] tmpbuf;
			}
		} 
		// TODO: position, etc. types.
	}


	bool Game::saveVariablesToMemory(char **memoryBuffer, const char *saveId, const char *saveType)
	{
		util::TextFileModifier tfm;

		assert(saveId != NULL);
		assert(saveType != NULL);

		tfm.newFile();

		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("#!dhs -nopp\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("// <SAVEGAME>\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("script savegame_");
		tfm.setBothSelectionsToEnd();
#ifdef PROJECT_SURVIVOR
		if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
		{
			tfm.addAfterSelection("coop_");
			tfm.setBothSelectionsToEnd();
		}
#endif
		tfm.addAfterSelection(saveId);
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("sub get_info\n");

		tfm.setBothSelectionsToEnd();
		std::string tmpver = "savegameVersion \"";
		tmpver += get_version_string();
		tmpver += "\"\n";
		tfm.addAfterSelection(tmpver.c_str());
		tfm.setBothSelectionsToEnd();

		std::string tmpsaveloc = "savegame_desc_";
		if (this->getMissionId() != NULL)
		{
			tmpsaveloc += this->getMissionId();
		} else {
			tmpsaveloc += "null";
			Logger::getInstance()->error("Game::saveVariablesToMemory - Null mission id.");
			fb_assert(!"Game::saveVariablesToMemory - Null mission id.");
		}

		std::string tmpsavedesc = "savegameDescription \"";
		tmpsavedesc += getLocaleGuiString(tmpsaveloc.c_str());
		if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
		{
			tmpsavedesc += " (COOP)";
		}
		tmpsavedesc += "\"\n";
		tfm.addAfterSelection(tmpsavedesc.c_str());

		tfm.setBothSelectionsToEnd();

		std::string tmpsavetime = "savegameTime \"";
		tmpsavetime += SystemTime::getSortableDateAndTime();
		tmpsavetime += "\"\n";
		tfm.addAfterSelection(tmpsavetime.c_str());

		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("savegameType \"");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection(saveType);
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\"\n");
		tfm.setBothSelectionsToEnd();

#ifdef PROJECT_SURVIVOR
		{
			// update time
			int timeValue = gameScripting->getGlobalIntVariableValue("player_survival_time");
			timeValue += getGameplayTime() / GAME_TICKS_PER_SECOND;
			gameScripting->setGlobalIntVariableValue("player_survival_time", timeValue);

			// update kills
			int totalKills = 0;
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				if (SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + c))
				{
					int killsValue = 0;
					util::Script::getGlobalArrayVariableValue("player_total_kills", c, &killsValue);
					killsValue += game::GameStats::instances[c]->getTotalKills();
					util::Script::setGlobalArrayVariableValue("player_total_kills", c, killsValue);
					totalKills += killsValue;
				}
			}

			std::string expstats;

#ifdef PROJECT_SURVIVOR
			expstats = " explevel ";
			int last_index = 0;
			util::Script::getGlobalIntVariableValue("exp_array_index", &last_index);
			int level = 0;
			util::Script::getGlobalArrayVariableValue("survivor_characterLevels", last_index, &level);
			expstats += boost::lexical_cast<std::string>(level+1);
#endif

			const char *time = time2str(timeValue);

			std::string kills = boost::lexical_cast<std::string>(totalKills);
			std::string secrets = boost::lexical_cast<std::string>(gameScripting->getGlobalIntVariableValue("secretpart_amount"));

			std::string stats = "savegameStats \"time " + std::string(time) + " kills " + kills + " secrets " + secrets + expstats + "\"\n";

			tfm.addAfterSelection(stats.c_str());
			tfm.setBothSelectionsToEnd();
		}
#endif
		tfm.addAfterSelection("endSub\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\n");
		tfm.setBothSelectionsToEnd();

		tfm.addAfterSelection("sub apply_mission\n");

#ifdef PROJECT_SURVIVOR
		// don't write profile info to savegames in coop
		if (getNumberOfPlayers() <= 1)
#endif
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if (i == 0)
			{
				if (SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED))
				{
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 1; setOptionValue 1st_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				} else {
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 0; setOptionValue 1st_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				}
			}
			if (i == 1)
			{
				if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
				{
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 1; setOptionValue 2nd_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				} else {
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 0; setOptionValue 2nd_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				}
			}
			if (i == 2)
			{
				if (SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
				{
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 1; setOptionValue 3rd_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				} else {
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 0; setOptionValue 3rd_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				}
			}
			if (i == 3)
			{
				if (SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED))
				{
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 1; setOptionValue 4th_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				} else {
					tfm.setBothSelectionsToEnd();
					std::string tmpenab = "setValue 0; setOptionValue 4th_player_enabled;\n";
					tfm.addAfterSelection(tmpenab.c_str());
				}
			}

			tfm.setBothSelectionsToEnd();
			std::string tmpprof = "setValue ";
			tmpprof += int2str(i);
			tmpprof += "; setProfile \"";
			if (this->profiles->getCurrentProfile(i) != NULL)
			{
				tmpprof += this->profiles->getCurrentProfile(i);
			} else {
				tmpprof += "";
			}
			tmpprof += "\"\n";
			tfm.addAfterSelection(tmpprof.c_str());
		}

		tfm.setBothSelectionsToEnd();
		std::string tmpmissfile = "setLoadMissionFile \"";
		tmpmissfile += this->currentMission;
		tmpmissfile += "\"\n";
		tfm.addAfterSelection(tmpmissfile.c_str());
		
		tfm.setBothSelectionsToEnd();
		std::string tmpmissid = "setMissionId \"";
		tmpmissid += this->getMissionId();
		tmpmissid += "\"\n";
		tfm.addAfterSelection(tmpmissid.c_str());

		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("endSub\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("sub apply_variables\n");

		{

#ifdef PROJECT_SURVIVOR
			LinkedList *tmp;

			// savegame formatting template
			//
			util::SimpleParser parser;
			if(parser.loadFile("Data/Scripts/savegame_template.dhs"))
			{
				// make a map of pointers to variables
				//
				std::map<int, util::VariableDataType *> variablePointerHash;
				util::VariableHashType *globalVariableHash = util::Script::getGlobalVariableHash();
				util::VariableHashType::iterator iter = globalVariableHash->begin();
				for (; iter != globalVariableHash->end(); ++iter)
				{
					if((*iter).second.permanent)
					{
						variablePointerHash[(*iter).first] = &(*iter).second;
					}
				}

				while(parser.next(true))
				{
					const char *buf = parser.getLine();

					if(buf[0] == '/' && buf[1] == '/')
					{
						tfm.setBothSelectionsToEnd();
						tfm.addAfterSelection(buf);
						tfm.setBothSelectionsToEnd();
						tfm.addAfterSelection("\n");
					}
					else
					{
						// cut
						std::string varname = buf;
						varname = varname.substr(0, varname.find_first_of(" \t\n"));

						// find in hash
						int hc = 0;
						SCRIPT_HASHCODE_CALC(varname.c_str(), &hc);
						std::map<int, util::VariableDataType *>::iterator iter = variablePointerHash.find(hc);
						if (iter != variablePointerHash.end() && iter->second != NULL)
						{
							writeVariable(tfm, varname.c_str());
							// remove from hash
							iter->second = NULL;
						}
						else
						{
							// not empty line - make it a comment
							if(buf[0] != '\n' && buf[0] != 0)
							{
								tfm.setBothSelectionsToEnd();
								tfm.addAfterSelection("//");
							}
							tfm.setBothSelectionsToEnd();
							tfm.addAfterSelection(buf);
							tfm.setBothSelectionsToEnd();
							tfm.addAfterSelection("\n");
						}
					}
				}
				tfm.setBothSelectionsToEnd();
				tfm.addAfterSelection("\n");

				// create linked list of remaining variables
				//
				tmp = new LinkedList();
				{
					std::map<int, util::VariableDataType *>::iterator iter = variablePointerHash.begin();
					for (; iter != variablePointerHash.end(); ++iter)
					{
						if(iter->second != NULL)
							tmp->append((*iter).second->name);
					}
				}
			}
			else
			{
				// no template file - just get all variables
				tmp = util::Script::getGlobalVariableList(true);
			}

#else
			LinkedList *tmp = util::Script::getGlobalVariableList(true);
#endif

			while (!tmp->isEmpty())
			{
				const char *varname = (const char *)tmp->popLast();
				writeVariable(tfm, varname);
			}

			delete tmp;
		}

		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("endSub\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("\n");
		tfm.setBothSelectionsToEnd();
		tfm.addAfterSelection("endScript\n");

		tfm.setStartSelectionToStart();
		tfm.setEndSelectionToEnd();
		char *buf = tfm.getSelectionAsNewBuffer();

		*memoryBuffer = buf;

		return true;
	}

	bool Game::loadVariablesFromMemory(const char *memoryBuffer, const char *saveId, const char *saveType, const char *applySub)
	{
		int buflen = strlen(memoryBuffer);
		char *tmp = new char[buflen + 1];
		strcpy(tmp, memoryBuffer);

		gameScripting->loadMemoryScripts("memory/savegame", tmp, buflen);
		char *scriptname = new char[32 + strlen(saveId) + 1];
		if (strcmp(saveId, "new") == 0)
		{
			strcpy(scriptname, "newgame");
		}
		// full path
		else if(strchr(saveId, '/') != NULL)
		{
			strcpy(scriptname, "savegame");
		}
		else
		{
			strcpy(scriptname, "savegame_");
			strcat(scriptname, saveId);
		}

		savegame_description = "(invalid savegame)";
		savegame_time = "(invalid time)";
		savegame_version = "";
		savegame_type = "invalid";
		savegame_stats = "";

		gameScripting->runMissionScript(scriptname, applySub);

		delete [] scriptname;
		delete [] tmp;
		return true;
	}

	bool Game::saveVariables(const char *filename, const char *saveId, const char *saveType)
	{
		if(!SimpleOptions::getBool(DH_OPT_B_SAVEGAME_ALLOW_OVERWRITE))
		{
			// file exists
			filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
			if(f)
			{
				filesystem::fb_fclose(f);
				// don't overwrite
				return false;
			}
		}

		char *buf = NULL;
		if (!saveVariablesToMemory(&buf, saveId, saveType))
		{
			return false;
		}

		bool saveFailed = false;
		FILE *f = fopen(filename, "wb");
		if (f != NULL)
		{
			int got = fwrite(buf, strlen(buf), 1, f);
			if (got != 1)
			{
				saveFailed = true;
			}
			fclose(f);
		} else {
			saveFailed = true;
		}
		delete [] buf;
		if (saveFailed)
		{
		  return false;
		}

		return true;
	}

	bool Game::loadVariables(const char *filename, const char *saveId, const char *saveType, const char *applySub)
	{
		char *buf = NULL;
		bool loadFailed = false;
		filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		if (f != NULL)
		{
			//fseek(f, 0, SEEK_END);
			//int filelen = ftell(f);
			//fseek(f, 0, SEEK_SET);
			int filelen = filesystem::fb_fsize(f);

			buf = new char[filelen + 1];
			int got = filesystem::fb_fread(buf, filelen, 1, f);
			if (got != 1)
			{
				loadFailed = true;
			}
			buf[filelen] = '\0';
			filesystem::fb_fclose(f);
		} else {
			loadFailed = true;
		}

		if (loadFailed)
		{
			if (buf != NULL)
			{
				delete [] buf;
			}
			return false;
		}

		if (!loadVariablesFromMemory(buf, saveId, saveType, applySub))
		{
			delete [] buf;
			return false;
		}
		delete [] buf;

		return true;
	}

	bool Game::saveVariablesToTemporary()
	{
		if (temporarySaveBuffer != NULL)
		{
			delete [] temporarySaveBuffer;
			temporarySaveBuffer = NULL;
		}
		bool success = saveVariablesToMemory(&temporarySaveBuffer, "temp", "temporary");
		assert(!success || temporarySaveBuffer != NULL);
		return success;
	}

	bool Game::loadVariablesFromTemporary()
	{
		if (temporarySaveBuffer == NULL)
		{
			temporarySaveBuffer = new char[256];
			//temporarySaveBuffer[0] = '\0';
			strcpy(temporarySaveBuffer, "script savegame_temp; sub get_info; endSub; sub apply_mission; endSub; sub apply_variables; endSub; endScript;\n\n");
		}
		bool success = loadVariablesFromMemory(temporarySaveBuffer, "temp", "temporary");
		return success;
	}

	void Game::givePlayerBeginningStuff(int player)
	{
		// TODO, should make these as GameRequests, not do this directly
		// or is this called by a game request... probably...
		// then don't call this directly in new game, make the requests

		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			money[i] = 0;
		}
	}


	int Game::calculatePurchasePrice(Part *part)
	{
		int sum = 0;

		if (part->isPurchasePending())
		{
			sum += part->getType()->getPrice();
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				sum += calculatePurchasePrice(subp);
			}
		} 		 

		return sum;
	}


	int Game::calculateRepairPrice(Part *part)
	{
		int sum = 0;

		if (part->getRepairPrice() > 0)
		{
			sum += part->getRepairPrice();
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				sum += calculateRepairPrice(subp);
			}
		} 		 

		return sum;
	}


	int Game::calculateReloadPrice(Part *part)
	{
		int sum = 0;

		if (part->getType()->
			isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
		{
			// WARNING: unsafe cast! (based on check above)
			WeaponObject *wo = (WeaponObject *)part;
			if (wo->getReloadPrice() > 0)
			{
				sum += wo->getReloadPrice();
			} 		 
		}
		if (part->getType()->
			isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Ammo"))))
		{
			// WARNING: unsafe cast! (based on check above)
			AmmoPackObject *apo = (AmmoPackObject *)part;
			if (apo->getReloadPrice() > 0)
			{
				sum += apo->getReloadPrice();
			} 		 
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				sum += calculateReloadPrice(subp);
			}
		} 		 

		return sum;
	}


	void Game::reloadParts(Part *part)
	{
		if (part->getType()->
			isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
		{
			// WARNING: unsafe cast! (based on check above)
			WeaponObject *wo = (WeaponObject *)part;
			if (wo->getReloadPrice() > 0)
			{
				if (part->getOwner() != NO_PART_OWNER)
					money[part->getOwner()] -= wo->getReloadPrice();
				wo->reload();
			} 		 
		}
		if (part->getType()->
			isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Ammo"))))
		{
			// WARNING: unsafe cast! (based on check above)
			AmmoPackObject *apo = (AmmoPackObject *)part;
			if (apo->getReloadPrice() > 0)
			{
				if (part->getOwner() != NO_PART_OWNER)
					money[part->getOwner()] -= apo->getReloadPrice();
				apo->reload();
			} 		 
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				reloadParts(subp);
			}
		} 		 
	}



	void Game::purchaseParts(Part *part)
	{
		//if (money[player] >= calculatePurchasePrice(part))
		//{
		//} else {
			// TODO: not enough money msgbox!!!
		//}
		if (part->isPurchasePending())
		{
			payForPart(part);
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				purchaseParts(subp);
			}
		} 		 
	}

	void Game::repairParts(Part *part)
	{
		if (part->getRepairPrice() > 0)
		{
			payForPart(part);
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				repairParts(subp);
			}
		} 		 
	}

	void Game::payForPart(Part *part)
	{
		if (part->isPurchasePending())
		{
			if (part->getOwner() != NO_PART_OWNER)
				money[part->getOwner()] -= part->getType()->getPrice();
			part->setPurchasePending(false);
		} else {
			if (part->getRepairPrice() > 0)
			{
				if (part->getOwner() != NO_PART_OWNER)
					money[part->getOwner()] -= part->getRepairPrice();
				part->repair();
			}
		}
	}

	void Game::removeUnpurchasedParts(Unit *unit, Part *part)
	{
		if (part->isPurchasePending())
		{
			// detach this unpurchased part (deletes it), sub parts are deleted
			// or put to storage
			detachParts(unit, part);
		} else {
			int slots = part->getType()->getSlotAmount();
			for (int i = 0; i < slots; i++)
			{
				Part *subp;
				if ((subp = part->getSubPart(i)) != NULL)
				{
					removeUnpurchasedParts(unit, subp);
				}
			} 		 
		}
	}

	// TODO: check that this works: puts purchased items to storage, not delete
	void Game::detachParts(Unit *unit, Part *part)
	{
		// this is depth first algorithm... first all children
		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				detachParts(unit, subp);
			}
		} 		 

		// next detach this part from it's parent
		Part *parent = NULL;
		if ((parent = part->getParent()) != NULL)
		{
			int parentslots = parent->getType()->getSlotAmount();
			for (int i = 0; i < parentslots; i++)
			{
				if (parent->getSubPart(i) == part)
				{
					parent->setSubPart(i, NULL);
					break;
				}
			}
			// done by setSubPart
			//part->setParent(NULL);
		} else {
			// no parent, so must be unit's root part
			unit->setRootPart(NULL);
		}

		// part has been detached, now delete it or put it to storage
		// and for computers, just delete it
		if (part->isPurchasePending()
			|| isComputerOpponent(part->getOwner()))
		{
			delete part;
		} else {
			parts->addPart(part);
		}
	}

	void Game::removeParts(Unit *unit, Part *part)
	{
		// this is depth first algorithm... first all children
		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				removeParts(unit, subp);
			}
		} 		 

		// next detach this part from it's parent
		Part *parent = NULL;
		if ((parent = part->getParent()) != NULL)
		{
			int parentslots = parent->getType()->getSlotAmount();
			for (int i = 0; i < parentslots; i++)
			{
				if (parent->getSubPart(i) == part)
				{
					parent->setSubPart(i, NULL);
					break;
				}
			}
			// done by setSubPart
			//part->setParent(NULL);
		} else {
			// no parent, so must be unit's root part
			unit->setRootPart(NULL);
		}

		// part has been detached, now delete it
		delete part;
	}

	void Game::createVisualForParts(Unit *unit, Part *part, bool keepBones)
	{
		// TODO: this is actually UI stuff, not gameplay... 
		// (should be moved, at least when netgame is being made)

		// if we're at root, create one visual for the unit
		// that will be used to combine all the parts' visual objects
		if (keepBones && unit->getVisualObject() == NULL)
		{
			assert(!"Game::createVisualForParts - Unit has null visual object when keepBones parameter given.");
			Logger::getInstance()->error("Game::createVisualForParts - Unit has null visual object when keepBones parameter given.");
			return;
		}

		if (unit->getRootPart() == part && !keepBones)
		{
			//Logger::getInstance()->error("VO/VOM allocs:");
			//Logger::getInstance()->error(int2str(ui::visual_object_allocations));
			//Logger::getInstance()->error(int2str(ui::visual_object_model_allocations));

			// delete old visual model, if one
			ui::VisualObject *oldv = unit->getVisualObject();
			unit->setVisualObject(NULL);
			if (oldv != NULL) delete oldv;

			if (part == NULL)
			{
				assert(!"Game::createVisualForParts - Given part is null.");
				Logger::getInstance()->error("Game::createVisualForParts - Root part of unit is null.");
				Logger::getInstance()->debug("Attempting to deactivate unit.");
				unit->setActive(false);
				return;
			}

			// create new base model and clear it (keeping only bones)
			UnitType *ut = unit->getUnitType();
			char *bonesFilename = ut->getBonesFilename();

			// possible root part bones-filename will override unit bones-filename
			if (part->getType()->getBonesFilename() != NULL)
			{
				bonesFilename = part->getType()->getBonesFilename();
			}

			VisualObjectModel *vom;
			VisualObject *vo;
			if (bonesFilename == NULL)
			{
				Logger::getInstance()->debug("Game::createVisualForParts - No bones defined for unit.");
			}
			//vom = new VisualObjectModel(bonesFilename);
			if (bonesFilename != NULL)
				vom = visualObjectModelStorage->getVisualObjectModel(bonesFilename);
			else
				vom = visualObjectModelStorage->getVisualObjectModel(NULL);

			vo = vom->getNewObjectInstance();
			unit->setVisualObject(vo);
			vo->setInScene(true);
			//vo->setSphereCollisionOnly(true);
			vo->setSphereCollisionOnly(false);
			vo->setDataObject(unit);
			vo->setStaticRotationYAngle((float)unit->getUnitType()->getBaseRotation());

			float scale = unit->getUnitType()->getScale();
			
			float randomScale = (float)((this->gameRandom->nextInt() % 201) - 100) / 100.0f;
			randomScale *= unit->getUnitType()->getRandomScale();
			scale += randomScale;

			if (scale != 1.0f)
			{
				vo->getStormModel()->SetScale(VC3(scale, scale, scale));
			}

			// NOTE: static treshold values here!
			vo->setPositionInterpolationAmount(unit->getUnitType()->getPositionInterpolation());
			vo->setPositionInterpolationTreshold(0.2f);
			vo->setRotationInterpolationAmount(unit->getUnitType()->getRotationInterpolation());
			vo->setRotationInterpolationTreshold(95.0f);
		}

		// delete old visual object first, if one
		if (part->getVisualObject() != NULL)
		{
			// doing this unnesessarily complex manner just to be sure...
			ui::VisualObject *oldv = part->getVisualObject();
			part->setVisualObject(NULL);
			delete oldv;
		}
		if (part->getType()->getVisualObjectModel() != NULL
			|| part->getType()->getMirrorVisualObjectModel() != NULL)
		{
			// name of the part to be used in the unit model...
			// and sideness

			// TODO: sideness is always left when going beond arms or legs...
			// therefore, weapons and other are always left sided!!!
			// fix that. (add side info to part class or just check grandparents)

			const char *partName = "";
			const char *helperName = NULL;
			bool rightSide = false;
			if (part->getParent() != NULL)
			{
				Part *parent = part->getParent();
				int slots = parent->getType()->getSlotAmount();
				for (int i = 0; i < slots; i++)
				{
					if (parent->getSubPart(i) == part)
					{
						int slotpos = parent->getType()->getSlotPosition(i);
						if (slotpos == SLOT_POSITION_WEAPON
							|| slotpos == SLOT_POSITION_BACK
							|| slotpos == SLOT_POSITION_EXTERNAL_ITEM)
						{
							Part *gparent = part->getParent()->getParent();
							if (gparent != NULL)
							{
								int parentSlotpos = SLOT_POSITION_EXTERNAL_ITEM;
								int gparentslots = gparent->getType()->getSlotAmount();
								for (int j = 0; j < gparentslots; j++)
								{
									if (gparent->getSubPart(j) == parent)
									{
										parentSlotpos = gparent->getType()->getSlotPosition(j);
										break;
									}
								}
								if (parentSlotpos == SLOT_POSITION_LEFT_ARM)
								{ 
									helperName = "HELPER_BONE_WeaponArmLeft";
								}
								if (parentSlotpos == SLOT_POSITION_RIGHT_ARM)
								{ 
									helperName = "HELPER_BONE_WeaponArmRight";
									rightSide = true;
								}
							} else {
								// back?
								if (slotpos == SLOT_POSITION_BACK)
								{
									helperName = "HELPER_BONE_WeaponBack";
								}
								// soldier?
								if (slotpos == SLOT_POSITION_WEAPON)
								{
									helperName = "HELPER_BONE_Weapon";
								}
								// solid armor?
								if (slotpos == SLOT_POSITION_WEAPON_LEFT)
								{
									helperName = "HELPER_BONE_WeaponArmLeft";
								}
								if (slotpos == SLOT_POSITION_WEAPON_RIGHT)
								{
									helperName = "HELPER_BONE_WeaponArmRight";
									rightSide = true;
								}
							}
						}
						if (slotpos == SLOT_POSITION_LEFT_ARM)
						{
							partName = "LeftArm";
						}
						if (slotpos == SLOT_POSITION_RIGHT_ARM)
						{
							partName = "RightArm";
							rightSide = true;
						}
						if (slotpos == SLOT_POSITION_LEFT_LEG)
						{
							partName = "LeftLeg";
						}
						if (slotpos == SLOT_POSITION_RIGHT_LEG)
						{
							partName = "RightLeg";
							rightSide = true;
						}
						if (slotpos == SLOT_POSITION_HEAD)
						{
							partName = "Head";
						}
						break;
					}
				}
			} else {
				partName = "Torso";
			}

			// create model
			VisualObject *vo = NULL;
			if (rightSide)
			{
				if (part->getType()->getMirrorVisualObjectModel() != NULL)
				{
					vo = part->getType()->getMirrorVisualObjectModel()->getNewObjectInstance();
				}
			} else {

				// torso model override
				VisualObjectModel *vom = part->getType()->getVisualObjectModel();
				if(!unit->getTorsoModelOverride().empty() && strcmp(partName, "Torso") == 0)
				{
					vom = visualObjectModelStorage->getVisualObjectModel(unit->getTorsoModelOverride().c_str());
				}

				if (vom != NULL)
				{
					vo = vom->getNewObjectInstance();
				}
			}
			part->setVisualObject(vo);

			// HACK: weapons not always visible (if not selected)
			// FIXME: bugs if several weapons of same type 
			// (all will be set to visible when only one actually selected)
			bool isItVisible = true;
			if (unit->getSelectedWeapon() != -1 
				|| unit->isDirectControl())
			{
				if (part->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
				{
					Weapon *selweap = NULL;
					if (unit->getSelectedWeapon() != -1) 
						selweap = unit->getWeaponType(unit->getSelectedWeapon());
					if (selweap == NULL 
						|| part->getType()->getPartTypeId() != selweap->getPartTypeId()
						|| !unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						isItVisible = false;
					}
				}
			}

			if (isItVisible)
			{
				// combine part model to unit model
				unit->getVisualObject()->combine(part->getVisualObject(), partName, helperName);
			}
		}

		// recursively do for subparts too
		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				createVisualForParts(unit, subp);
			}
		}

		if (unit->getRootPart() == part)
		{
			// done, now make it a nice unique model if stealth armor
			if (unit->getStealthValue() > 0)
			{
				if (unit->getVisualObject() != NULL)
				{
					unit->getVisualObject()->disableSharedObjects();
				}
			}

			// and then some unit effect layers maybe...
			if (unit->getUnitEffectLayerDuration() > 0)
			{
				if (unit->getVisualObject() != NULL)
				{
					unit->getVisualObject()->createEffectLayer();

					if (unit->getUnitEffectLayerType() != Unit::UNIT_EFFECT_LAYER_NONE)
					{
						if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_ELECTRIC)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_ELECTRIC);
						}
						else if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_BURNING)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_BURNING);
						}
						else if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_SLIME)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_SLIME);
						}
						else if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_CLOAK)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_CLOAK);
						}
						else if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_CLOAKHIT)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_CLOAKHIT);
						}
						else if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_PROTECTIVESKIN)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_PROTECTIVESKIN);
						}
						else if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_CLOAKRED)
						{
							unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_CLOAKRED);
						}
					}

					// NOTE: currently used by slime effect only.
					unit->getVisualObject()->setEffectDuration(unit->getUnitEffectLayerDuration() * GAME_TICK_MSEC);
				}
			}

			if (unit->getBurnedCrispyAmount() > 0)
			{
				if (SimpleOptions::getInt(DH_OPT_I_LAYER_EFFECTS_LEVEL) >= 75)
				{
					if (unit->getVisualObject() != NULL)
					{
						unit->getVisualObject()->setBurnedCrispyAmount(unit->getBurnedCrispyAmount());

						unit->getVisualObject()->createEffectLayer2();
						unit->getVisualObject()->setEffect(VISUALOBJECTMODEL_EFFECT_BURNED_CRISPY);
					}
				}
			}



			unit->setLastLightUpdatePosition(VC3(-999999,-999999,-999999));

			if (unit->getUnitType()->hasNoBoneCollision())
			{
				if (unit->getVisualObject() != NULL)
				{
					unit->getVisualObject()->getStormModel()->EnableBoneCollision(false);
				}
			}

			// need to reset fade value, as that has been lost when the model was recreated(?)
			if (keepBones) 
			{
				if (unit->getVisualObject() != NULL)
				{
					unit->getVisualObject()->setVisibilityFactor(unit->getCurrentVisibilityFadeValue());
				}
				// same for forced visibility value
				if (unit->isForcedLightVisibilityFactor())
				{
					if (unit->getVisualObject() != NULL)
					{
						unit->getVisualObject()->setLightVisibilityFactor(unit->getLightVisibilityFactor(), true);
					}
				}
			}

			if (unit->getUnitType()->hasNoCollision())
			{
				unit->getVisualObject()->setCollidable(false);
				unit->getVisualObject()->setForcedNoCollision(true);
			}

			if (unit->getVisualObject() != NULL)
			{
				unit->getVisualObject()->setSideways(unit->isSideways());
			}

			this->units->updateUnitRadius(unit);

		}
		// end of if (unit->getRootPart() == part)

	}

	void Game::deleteVisualOfParts(Unit *unit, Part *part, bool keepBones)
	{
		// delete old visual object first, if one
		if (part->getVisualObject() != NULL)
		{
			// doing this unnesessarily complex manner just to be sure...
			ui::VisualObject *oldv = part->getVisualObject();
			part->setVisualObject(NULL);
			if (oldv != NULL) delete oldv;
		}
		// recursively do for subparts too
		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				deleteVisualOfParts(unit, subp);
			}
		}
		// finally delete the unit visual object, if we're at root part
		if (keepBones)
		{
			if (unit->getVisualObject() != NULL)
				unit->getVisualObject()->clearObjects();

			if (unit->getVisualObject() != NULL)
				unit->getVisualObject()->clearEffects();

			// FIXME: clear helpers too!!! now, we're leaking them?
			// FIXED: done by clearObjects (i think?)

		} else {
			if (unit->getRootPart() == part)
			{
				ui::VisualObject *oldv = unit->getVisualObject();
				unit->setVisualObject(NULL);
				if (oldv != NULL) delete oldv;
			}
		}

	}

	void Game::recreateVisualOfParts(Unit *unit, Part *part, bool keepBones)
	{
		ui::VisualObject *oldv = unit->getVisualObject();
		Vector oldPos;
		if(oldv != NULL) oldPos = oldv->getRenderPosition();

		deleteVisualOfParts(unit, part, keepBones);
		createVisualForParts(unit, part, keepBones);

		if(oldv != NULL && unit->getVisualObject() != NULL) unit->getVisualObject()->setPosition(oldPos);
	}

	/*
	void Game::addMapObstacle(int x, int y)
	{
		assert(pathFinder != NULL);
		pathFinder->addObstacle(x, y);
	}

	void Game::removeMapObstacle(int x, int y)
	{
		assert(pathFinder != NULL);
		pathFinder->removeObstacle(x, y);
	}
	*/

  void Game::setMissionId(const char *missionid_)
	{
		if (missionId != NULL)
		{
			delete [] missionId;
			missionId = NULL;
		}
		if (missionid_ != NULL)
		{
			missionId = new char[strlen(missionid_) + 1];
			strcpy(missionId, missionid_);
		}
	}

  const char *Game::getMissionId()
	{
		return this->missionId;
	}


	void Game::setMissionScript(char *scriptname)
	{
		if (script != NULL)
		{
			delete [] script;
			script = NULL;
		}
		if (scriptname != NULL)
		{
			script = new char[strlen(scriptname) + 1];
			strcpy(script, scriptname);
		}
	}

	void Game::setBuildingsScript(char *scriptname)
	{
		if (buildingsScript != NULL)
		{
			delete [] buildingsScript;
			buildingsScript = NULL;
		}
		if (scriptname != NULL)
		{
			buildingsScript = new char[strlen(scriptname) + 1];
			strcpy(buildingsScript, scriptname);
		}
	}


	void Game::setSectionScript(char *scriptname)
	{
		if (sectionScript != NULL)
		{
			delete [] sectionScript;
			sectionScript = NULL;
		}
		if (scriptname != NULL)
		{
			sectionScript = new char[strlen(scriptname) + 1];
			strcpy(sectionScript, scriptname);
		}
	}

	void Game::setPendingLoad(const char *saveId)
	{
		if (pendingLoad != NULL)
		{
			delete [] pendingLoad;
			pendingLoad = NULL;
		}
		if (saveId != NULL)
		{
			pendingLoad = new char[strlen(saveId) + 1];
			strcpy(pendingLoad, saveId);
		}
	}

	const char *Game::getPendingLoad()
	{
		return pendingLoad;
	}

	void Game::setCurrentMission(char *missionfile)
	{
		if (currentMission != NULL)
		{
			delete [] currentMission;
			currentMission = NULL;
		}
		if (missionfile != NULL)
		{
			currentMission = new char[strlen(missionfile) + 1];
			strcpy(currentMission, missionfile);
		}
	}

	void Game::setSuccessMission(char *missionfile)
	{
		if (nextMissionOnSuccess != NULL)
		{
			delete [] nextMissionOnSuccess;
			nextMissionOnSuccess = NULL;
		}
		if (missionfile != NULL)
		{
			nextMissionOnSuccess = new char[strlen(missionfile) + 1];
			strcpy(nextMissionOnSuccess, missionfile);
		}
	}

	void Game::setFailureMission(char *missionfile)
	{
		if (nextMissionOnFailure != NULL)
		{
			delete [] nextMissionOnFailure;
			nextMissionOnFailure = NULL;
		}
		if (missionfile != NULL)
		{
			nextMissionOnFailure = new char[strlen(missionfile) + 1];
			strcpy(nextMissionOnFailure, missionfile);
		}
	}

	void Game::requestEndCombat()
	{
		// FIXME: for some reason, the mission's failure script is being run when new game/load
		// done in the middle of the mission (inCombat)... this is otherwise correct behaviour, but 
		// mission (inCombat) has already been ended when the script runs... ???
		// resulting into this assert failing. 
		//assert(inCombat);
		// just ignore it, things seems to be working just fine regardless :P  --jpk
		// (possibly some memory leaks or whatever nasty things happening, but don't care about those)

		if (inCombat)
		{
			endingCombat = true;
		}
	}

	char *Game::getCurrentMission()
	{
		return currentMission;
	}

	char *Game::getSuccessMission()
	{
		return nextMissionOnSuccess;
	}

	char *Game::getFailureMission()
	{
		return nextMissionOnFailure;
	}

	void Game::setCinematicScriptProcess(char *script)
	{
		if (cinematicScriptProcess != NULL)
		{
			Logger::getInstance()->warning("Game::setCinematicScriptProcess - Another script already running.");
		} else {
			cinematicScriptProcess = gameScripting->startNonUnitScript(script, "play_cinematic");
		}
	}

	bool Game::isCinematicScriptRunning()
	{
		if (cinematicScriptProcess != NULL)
			return true;
		else
			return false;
	}

	void Game::skipCinematicScript()
	{
		skippingCinematic = true;
		if (cinematicScriptProcess != NULL)
		{
			if (!cinematicScriptProcess->isFinished())
			{
				// WARNING: unsafe cast
				GameScriptData *gsd = (GameScriptData *)cinematicScriptProcess->getData();
				gsd->waitCounter = 0;
				gsd->waitDestination = false;
				gsd->waitCinematicScreen = false;
			}
		}
	}

	bool Game::isSkippingCinematic()
	{
		return skippingCinematic;
	}

	void Game::setTacticalMode(bool tacticalModeOn)
	{
		tacticalMode = tacticalModeOn;
		if (tacticalMode || paused)
		{
			gameUI->setUIPauseState(true);
		} else {
			gameUI->setUIPauseState(false);
		}
	}

	bool Game::isTacticalMode()
	{
		return tacticalMode;
	}

	void Game::setPaused(bool pauseOn)
	{
		paused = pauseOn;
		if (tacticalMode || paused)
		{
			gameUI->setUIPauseState(true);
		} else {
			gameUI->setUIPauseState(false);
		}

		// calculate how long the game was paused
		if(paused && pauseStartTime == 0)
		{
			pauseStartTime = gameTimer;
		}
		else if(!paused && pauseStartTime != 0)
		{
			missionPausedTime += gameTimer - pauseStartTime;
			pauseStartTime = 0;
		}
	}

	bool Game::isPaused() const
	{
		return paused;
	}

	void Game::clearVisualObjectModelStorage()
	{
		visualObjectModelStorage->clear();
	}

	// returns true if horizontal autoaim is on
	bool Game::isAutoAimHorizontal()
	{
		return SimpleOptions::getBool(DH_OPT_B_AUTOAIM_HORIZONTAL);
	}

	// returns true if vertical autoaim is on
	bool Game::isAutoAimVertical()
	{
		return SimpleOptions::getBool(DH_OPT_B_AUTOAIM_VERTICAL);
	}

	DifficultyManager *Game::getDifficultyManager()
	{
		return difficultyManager;
	}

	void Game::setEnvironmentalEffectManager(EnvironmentalEffectManager *effman)
	{
		assert(this->environmentalEffectManager == NULL || effman == NULL);
		this->environmentalEffectManager = effman;
	}

	EnvironmentalEffectManager *Game::getEnvironmentalEffectManager()
	{
		return this->environmentalEffectManager;
	}

	bool Game::isMissionAboutToEnd()
	{
		if ( ( this->missionFailureCounter < 0 
			|| this->missionSuccessCounter < 0 ) )
			return true;
		else
			return false;
	}

	Unit *Game::createGhostsOfFuture(Unit *unit)
	{
		if (unit->isGhostOfFuture())
			return NULL;

		LinkedList *ulist = units->getAllUnits();
		LinkedListIterator uiter = LinkedListIterator(ulist);

		Unit *ghost = NULL;

		while (uiter.iterateAvailable())
		{
			Unit *u = (Unit *)uiter.iterateNext();
			if (u->isGhostOfFuture() && u->getUnitType() == unit->getUnitType()
				&& u->isDestroyed())
			{
				//ghost = u;
				break;
			}
		}

		UnitType *ut = unit->getUnitType();

		if (ghost == NULL)
		{
			// WARNING: reserving all players above max/2 for ghosts.
			ghost = ut->getNewUnitInstance(unit->getOwner() + ABS_MAX_PLAYERS/2);
			{
				for (int i = 0; i < ABS_MAX_PLAYERS/2; i++)
				{
					for (int j = 0; j < ABS_MAX_PLAYERS/2; j++)
					{
						this->hostile[i][j+ABS_MAX_PLAYERS/2] = false;
						this->hostile[i+ABS_MAX_PLAYERS/2][j] = false;
						this->hostile[i+ABS_MAX_PLAYERS/2][j+ABS_MAX_PLAYERS/2] = this->hostile[i][j];
					}
				}
			}
			units->addUnit(ghost);
			ghost->setActive(false);
			ghost->makeGhostOfFuture();
			PartType *pt = unit->getRootPart()->getType();
			Part *part = pt->getNewPartInstance();
			part->setOwner(ghost->getOwner());
			ghost->setRootPart(part);

			for (int slot = 0; slot < pt->getSlotAmount(); slot++)
			{
				if (unit->getRootPart()->getSubPart(slot) != NULL)
				{
					Part *p = unit->getRootPart()->getSubPart(slot)->getType()->getNewPartInstance();
					p->setOwner(ghost->getOwner());
					part->setSubPart(slot, p);
				}
			}

			VC3 spawnpos = unit->getSpawnCoordinates();
			ghost->setSpawnCoordinates(spawnpos);
			ghost->setScript(unit->getScript());
			UnitSpawner::spawnUnit(this, ghost);
			// WARNING: unsafe casts.
			UnitLevelAI *ai = (UnitLevelAI *)ghost->getAI();
			UnitLevelAI *sourceai = (UnitLevelAI *)unit->getAI();
			ai->copyStateFrom(sourceai);
		}

		ghost->setGhostTime(0);
		ghost->setDestroyed(false);
		ghost->setHP(ghost->getMaxHP());

		ghost->setPosition(unit->getPosition());
		ghost->setVelocity(unit->getVelocity());
		VC3 rot = unit->getRotation();
		ghost->setRotation(rot.x, rot.y, rot.z);
		if (ghost->getWeaponType(unit->getSelectedWeapon()) != NULL)
			ghost->setSelectedWeapon(unit->getSelectedWeapon());

		UnitActor *ua = getUnitActorForUnit(ghost);

		if (unit->isDirectControl())
		{
			VC3 velNorm(0,0,0);
			if (unit->getVelocity().GetSquareLength() > 0.0001f)
			{
				velNorm = unit->getVelocity().GetNormalized();
				ua->removeUnitObstacle(unit);
				if (!ua->setPathTo(ghost, ghost->getPosition() + (velNorm * 8.0f)))
				{
					ua->setPathTo(ghost, ghost->getPosition() + (velNorm * 5.0f));
				}
				ua->addUnitObstacle(unit);
			} else {
				ua->stopUnit(ghost);
			}
		} else {
			// todo, should directly copy the path!
			ua->removeUnitObstacle(unit);
			VC3 dest = unit->getFinalDestination();
			ua->setPathTo(ghost, dest);
			ua->addUnitObstacle(unit);
		}

		for (int i = 0; i < MAX_UNIT_VARIABLES; i++)
		{
			ghost->variables.setVariable(i, unit->variables.getVariable(i));
		}

		// TODO: copy weapon states, etc.
		// TODO: copy scriptprocess

		return ghost;
	}

	int Game::addCustomScriptProcess(const char *script, Unit *unit, const std::vector<int> *paramStack)
	{
		if (customScriptProcesses == NULL)
		{
			customScriptProcesses = new LinkedList();
		}
		util::ScriptProcess *sp = NULL;
		if (unit != NULL)
		{
			sp = gameScripting->startUnitScript(unit, script, "main", paramStack);
		} else {
			sp = gameScripting->startNonUnitScript(script, "main", paramStack);
		}
		int pid = 0;
		if (sp != NULL)
		{
			customScriptProcesses->append(sp);
			pid = sp->getId();
		}
		return pid;
	}

  int Game::addCustomScriptProcess(const char *script, UnifiedHandle uh, const std::vector<int> *paramStack)
	{
		if (customScriptProcesses == NULL)
		{
			customScriptProcesses = new LinkedList();
		}
		util::ScriptProcess *sp = NULL;
		sp = gameScripting->startNonUnitScript(script, "main", paramStack);
		int pid = 0;
		if (sp != NULL)
		{
			// WARNING: unsafe cast!
			GameScriptData *gsd = (GameScriptData *)sp->getData();
			gsd->unifiedHandle = uh;
			customScriptProcesses->append(sp);
			pid = sp->getId();
		}
		return pid;
	}

  void Game::stopAllCustomScriptProcesses()
	{
		if (customScriptProcesses != NULL)
		{
			while (!customScriptProcesses->isEmpty())
			{
				util::ScriptProcess *sp = (util::ScriptProcess *)customScriptProcesses->popLast();
				assert(sp != NULL);
				delete static_cast<GameScriptData *> (sp->getData());
				delete sp;
			}
			delete customScriptProcesses;
			customScriptProcesses = NULL;
		}
	}

	int Game::getCustomScriptProcessAmount()
	{
		// TODO: optimize.
		int ret = 0;
		if (customScriptProcesses != NULL)
		{
			LinkedListIterator iter(customScriptProcesses);
			while (iter.iterateAvailable())
			{
				iter.iterateNext();
				ret++;
			}
		}
		return ret;
	}

	bool Game::isRunningCustomScriptProcess(const char *script, Unit *unit)
	{
		if (customScriptProcesses != NULL)
		{
			SafeLinkedListIterator iter(customScriptProcesses);
			while (iter.iterateAvailable())
			{
				util::ScriptProcess *sp = (util::ScriptProcess *)iter.iterateNext();
				assert(sp != NULL);
				util::Script *s = sp->getScript();
				if (!s)
					continue;

				if(strcmp(s->getName(), script) != 0)
					continue;

				game::GameScriptData *gsd = (GameScriptData *)sp->getData();

				if(!gsd)
					continue;

				if(gsd->originalUnit != unit)
					continue;
				
				return true;
			}
		}
		return false;
	}

  void Game::stopCustomScriptProcessById(int pid)
	{
		int deletedProcesses = 0;

		if (customScriptProcesses != NULL)
		{
			SafeLinkedListIterator iter(customScriptProcesses);
			while (iter.iterateAvailable())
			{
				util::ScriptProcess *sp = (util::ScriptProcess *)iter.iterateNext();
				assert(sp != NULL);
				if (sp->getId() == pid)
				{
					customScriptProcesses->remove(sp);
					delete static_cast<GameScriptData *> (sp->getData());
					delete sp;
					deletedProcesses++;
				}
			}
		}
		if (deletedProcesses == 0)
		{
			Logger::getInstance()->warning("Game::stopCustomScriptProcessById - No script process found with given id.");
		}
		if (deletedProcesses > 1)
		{
			Logger::getInstance()->error("Game::stopCustomScriptProcessById - Internal error, multiple script process found with given id.");
		}
	}


	/*
  int Game::getCustomScriptProcessId(util::ScriptProcess *sp)
	{
		if (customScriptProcesses != NULL)
		{
			LinkedListIterator iter(customScriptProcesses);
			while (iter.iterateAvailable())
			{
				util::ScriptProcess *listsp = (util::ScriptProcess *)iter.iterateNext();
				assert(listsp != NULL);
				if (listsp == sp)
				{
					return listsp->getId();
				}
			}
		}
		return 0;
	}
	*/


	bool Game::loadGame(const char *saveId)
	{
		if (gameUI->isAbortingMission())
		{
			assert(!"Game::loadGame - already aborting mission.");
			return false;
		}

		if (saveId == NULL)
		{
			assert(!"Game::loadGame - save id null.");
			return false;
		}

		const char *profiledir = profiles->getProfileDirectory( 0 );

		const char *saveGameType = "invalid";
		char *savefile = NULL;
		// new game
		if (strcmp(saveId, "new") == 0)
		{
			saveGameType = "newgame";
			savefile = new char[64];
#ifdef LEGACY_FILES
			strcpy(savefile, "Data/Missions/newgame.dhs");
#else
			strcpy(savefile, "data/mission/newgame.dhs");
#endif
		}
		// full path
		else if(strchr(saveId, '/') != NULL)
		{
			saveGameType = "savegame";
			int size = strlen(saveId) + 1;
			savefile = new char[size];
			memcpy(savefile, saveId, size);
		}
		// save game
		else
		{
			saveGameType = "savegame";
			savefile = new char[64 + strlen(profiledir) + strlen(saveId) + 1];
			strcpy(savefile, profiledir);
#ifdef LEGACY_FILES
			strcat(savefile, "/Save/save_");
#else
			strcat(savefile, "/save/save_");
#endif
			strcat(savefile, saveId);
			strcat(savefile, ".dhs");
		}

		// TODO: check that file exists here? (or just rely on loadVariable to do that)

		// first, set the mission we want...
		bool success = this->loadVariables(savefile, saveId, saveGameType, "apply_mission");

		delete [] savefile;

		if (!success)
		{
			return false;
		}
		
		if( getNumberOfPlayers() > 1 )
		{
			setCooperative( true );
		}
		else
		{
			setCooperative( false );
		}

		gameUI->openLoadingWindow(singlePlayerNumber);

		// then fall back to menus...
		if (inCombat)
		{
			assert(inCombat);
			endingCombat = true;
			missionFailureCounter = -1;
			missionSuccessCounter = 0;
		}

		// (and once that is done, menus should automagically start mission, after
		// which the global variables are applied...)
		setPendingLoad(saveId);

		// now set the mission to actually start :)
		automaticallyStartMission = true;
		automaticallyStartMission_loop2 = false;

		lastSaveId = saveId;
		lastSaveNumber = atoi(saveId);
		return true;
	}

	void Game::applyPendingLoad()
	{
		char *saveId = pendingLoad;

		if (saveId == NULL)
		{
			assert(!"Game::applyPendingLoad - save id null.");
			return;
		}

		const char *profiledir = profiles->getProfileDirectory( 0 );

		const char *saveGameType = "invalid";
		char *savefile = NULL;
		// new game
		if (strcmp(saveId, "new") == 0)
		{
			saveGameType = "newgame";
			savefile = new char[64];
#ifdef LEGACY_FILES
			strcpy(savefile, "Data/Missions/newgame.dhs");
#else
			strcpy(savefile, "data/mission/newgame.dhs");
#endif
		}
		// full path
		else if(strchr(saveId, '/') != NULL)
		{
			saveGameType = "savegame";
			int size = strlen(saveId) + 1;
			savefile = new char[size];
			memcpy(savefile, saveId, size);
		}
		// save game
		else
		{
			saveGameType = "savegame";
			savefile = new char[64 + strlen(profiledir) + strlen(saveId) + 1];
			strcpy(savefile, profiledir);
			strcat(savefile, "/Save/save_");
			strcat(savefile, saveId);
			strcat(savefile, ".dhs");
		}

		bool success = this->loadVariables(savefile, saveId, saveGameType, "apply_variables");

		if (!success)
		{
			Logger::getInstance()->warning("Game::applyPendingLoad - Failed to load variables (failed to run apply_variables sub).");
		}

		delete [] savefile;

		// ok, game loaded. done.
		setPendingLoad(NULL);

		// finally, need to run a script to actually apply the loaded game...
		gameScripting->runMissionScript("load_game", "apply_loaded");
	}


	bool Game::getInfoForSavegame(const char *saveId, const char *saveType)
	{
		savegame_type = "invalid";
		savegame_version = "";
		savegame_description = "(invalid description)";
		savegame_time = "(invalid time)";
		savegame_stats = "";
		savegame_mission_id = "";

		if (gameUI->isAbortingMission())
		{
			assert(!"Game::getInfoForSavegame - already aborting mission.");
			return false;
		}

		if (saveId == NULL)
		{
			assert(!"Game::getInfoForSavegame - save id null.");
			return false;
		}

		const char *profiledir = profiles->getProfileDirectory( 0 );
		char *savefile;
		// saveid is a full path
		if(strchr(saveId, '/') != NULL)
		{
			int size = strlen(saveId) + 1;
			savefile = new char[size];
			memcpy(savefile, saveId, size);
		}
		else
		{
			savefile = new char[64 + strlen(profiledir) + strlen(saveId) + 1];
			strcpy(savefile, profiledir);
			strcat(savefile, "/Save/save_");
			strcat(savefile, saveId);
			strcat(savefile, ".dhs");
		}

		filesystem::FB_FILE *f = filesystem::fb_fopen(savefile, "rb");
		if (f != NULL)
		{
			// manually find mission id
			//
			const char *command = "setMissionId \"";
			size_t command_length = strlen(command);
			size_t matched = 0;
			bool read_command = true;
			bool done = false;
			size_t id_length = 0;
			size_t max_id_length = 31;
			savegame_mission_id.resize(max_id_length + 1);
			while(!done)
			{
				char buffer[1024];
				size_t numread = filesystem::fb_fread(buffer, 1, 1024, f);

				for(size_t i = 0; i < numread; i++)
				{
					// reading command
					if(read_command)
					{
						if(buffer[i] != command[matched])
						{
							matched = 0;
							continue;
						}

						matched++;

						// found command
						if(matched == command_length)
						{
							read_command = false;
						}
					}
					else
					{
						if(buffer[i] == '"' || id_length == max_id_length)
						{
							done = true;
							break;
						}
						// reading data
						savegame_mission_id[id_length] = buffer[i];
						id_length++;
					}
				}

				if(filesystem::fb_feof(f))
					break;
			}
			
			savegame_mission_id.resize(id_length);

			filesystem::fb_fclose(f);
		} else {
			// file did not exist
			Logger::getInstance()->debug("Game::getInfoForSaveGame - Savegame does not exist.");
			return false;
		}

		// first, set the mission we want...
		bool success = this->loadVariables(savefile, saveId, saveType, "get_info");

		if (savegame_type != saveType)
		{
			Logger::getInstance()->error("Game::getInfoForSavegame - Savegame type mismatch.");
			return false;
		}
		if (savegame_version != "x"
			&& savegame_version != get_version_string())
		{
			Logger::getInstance()->warning("Game::getInfoForSavegame - Savegame version mismatch, game may not load correctly.");
			// TODO: if older version, load anyway, if newer version, don't load?
			//return false;
		}

		delete [] savefile;

		if (!success)
		{
			return false;
		}

		return true;
	}


	bool Game::saveGame(const char *saveId)
	{
		// start by running the script that prepares saving of weapons, etc.
		gameScripting->runMissionScript("save_game", "prepare_for_save");

		if (saveId == NULL)
		{
			assert(!"Game::saveGame - save id null.");
			return false;
		}

#ifdef PROJECT_SURVIVOR
		// remember last saved game, if it's a number
		int save_number = 0;
		if(sscanf(saveId, "%i", &save_number) == 1)
		{
			char buf[32];
#ifdef _MSC_VER
			sprintf_s
#else
			snprintf
#endif
			(buf, 32, "%i", save_number);

			// write for each player
			int numPlayers = getNumberOfPlayers();
			for(int i = 0; i < numPlayers; i++)
			{
				const char *profiledir = profiles->getProfileDirectory( i );
				char *savefile = (char *)alloca(64 + strlen(profiledir) + 1);
				strcpy(savefile, profiledir);

				// different file for coop
				if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
					strcat(savefile, "/Save/lastsave_coop.txt");
				else
					strcat(savefile, "/Save/lastsave.txt");

				FILE *f = fopen(savefile, "wb");
				if (f != NULL)
				{
					fwrite(buf, strlen(buf), 1, f);
					fclose(f);
				}
			}
		}

		// coop hax: savegames are separate and copies are made for all players!
		int numPlayers = getNumberOfPlayers();
		if(numPlayers > 1)
		{
			for(int i = 0; i < numPlayers; i++)
			{
				const char *profiledir = profiles->getProfileDirectory( i );
				char *savefile = new char[64 + strlen(profiledir) + strlen(saveId) + 1];
				strcpy(savefile, profiledir);
				strcat(savefile, "/Save/save_coop_");
				strcat(savefile, saveId);
				strcat(savefile, ".dhs");

				this->saveVariables(savefile, saveId, "savegame");

				delete [] savefile;
			}
			lastSaveId = "coop_" + std::string(saveId);
			return true;
		}
#endif

		const char *profiledir = profiles->getProfileDirectory( 0 );
		char *savefile = new char[64 + strlen(profiledir) + strlen(saveId) + 1];
		strcpy(savefile, profiledir);
		strcat(savefile, "/Save/save_");
		strcat(savefile, saveId);
		strcat(savefile, ".dhs");

		this->saveVariables(savefile, saveId, "savegame");

		delete [] savefile;

		lastSaveId = saveId;
		lastSaveNumber = atoi(saveId);
		return true;
	}

	int Game::getCinematicScriptProcessId()
	{
		if (this->cinematicScriptProcess != NULL)
		{
			return this->cinematicScriptProcess->getId();
		} else {
			return 0;
		}
	}

	bool Game::save(const char *filename)
	{
		FILE *f = fopen(filename, "wb");
		if (f == NULL) return false;

		const char *ver_buf = "SAVE1.0";
		//int hdr_buf[5];
		//BYTE *data_buf;
	
		fwrite(ver_buf, sizeof(char), 8, f);

		// TODODODODO!!!

		// TODO, go through all root level gameobjects to be saved
		// they will give us the roots of the tree structures to be saved
		// each tree node is a GameObject possibly having more children objects...

		fclose(f);
		return true;
	}


	// presuming 32 bit pointers and 32 bit ints
	// presuming intel byte order
	// fix this if not so

	bool Game::load(const char *filename)
	{
		FILE *f = fopen(filename, "rb");
		if (f == NULL) return false;

		char ver_buf[8];
		int hdr_buf[5];
		BYTE *data_buf;

		// would need a hashtable for this really
		LinkedList parents = LinkedList();

		// read version
		fread(ver_buf, sizeof(char), 8, f);
		if (strncmp(ver_buf, "SAVE1.0", 8) != 0) return false;

		// read data chunks
		int gothdr;
		while ((gothdr = fread(hdr_buf, sizeof(int), 5, f)) == 5)
		{
			int chunkid = hdr_buf[0];
			int datasize = hdr_buf[1];
			int selfptr = hdr_buf[2];
			int parentptr = hdr_buf[3];
			int children = hdr_buf[4];

			IGameObjectFactory *fact = objectFactories->getById(chunkid);
			if (fact == NULL)
			{
				// no factory for such data chunk id
	#ifdef _DEBUG
				abort();
	#else
				fclose(f);
				return false;
	#endif
			}
			if (datasize <= 0 || datasize > 99999999) abort();
			if (datasize > 0)
			{
				data_buf = new BYTE[datasize];
				if ((int)fread(data_buf, sizeof(BYTE), datasize, f) != datasize)
				{
					// could not read whole data, got only a part of it
					return false;
				}
			} else {
				data_buf = NULL;
			}

			GameObject *realparent = NULL;
			if (parentptr != 0)
			{
				SaveParentEntry *tmp = (SaveParentEntry *)parents.peekLast();
				if (tmp != NULL && tmp->saved == parentptr)
				{
					realparent = tmp->real;
					tmp->childleft--;
					if (tmp->childleft == 0)
					{
						delete (SaveParentEntry *)parents.popLast();
					}
				} else {
	#ifdef _DEBUG
					abort();
	#else
					fclose(f);
					return false;
	#endif
				}
			}

			GameObject *go = fact->create(chunkid, datasize, data_buf, realparent);
			if (data_buf != NULL) 
			{
				delete [] data_buf;
				data_buf = NULL;
			}
			if (go != NULL)
			{
				if (children > 0) 
				{
					parents.append(new SaveParentEntry(selfptr, children, go));
				}
			}
		}
		if (gothdr != 0)
		{
			// file size mismatch?, expected one chunk or nothing, got part of chunk
			fclose(f);
			return false;
		}

		fclose(f);
		return true;
	}

	char *Game::getGameSceneGraphDump()
	{
		char *ret = new char[65536+1];
		strcpy(ret, "Game scene graph dump follows:\n");

		objectList->resetIterate();
		while (objectList->iterateAvailable())
		{
			GameObject *o = objectList->iterateNext();
			const char *inf = o->getStatusInfo();

			if (strlen(ret)+strlen(inf) > 60000)
			{
				strcat(ret, "--continues--\n");
				break;
			}
			strcat(ret, inf);
			strcat(ret, "\r\n");
		}		
		return ret;
	}

	void Game::deleteGameSceneGraphDump(char *buf)
	{
		delete [] buf;
	}

	int Game::getGameplayTime()
	{
		return (gameTimer - missionStartTime - missionPausedTime);
	}
}

