
#include "precompiled.h"

#include <stdlib.h> // for NULL
#include <cmath>

#ifndef M_PI
#define M_PI PI
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <Storm3D_UI.h>
#include <igios.h>

#include "gui_configuration.h"

#include "GameUI.h"
#include "DHLocaleManager.h"
#include "GameScene.h"
#include "GameMap.h"
#include "scripting/GameScripting.h"
#include "scripting/MapScripting.h"
#include "UnitType.h"
#include "unittypes.h"
#include "UnitList.h"
#include "ProjectileList.h"
#include "Projectile.h"
#include "ProjectileActor.h"
#include "BuildingList.h"
#include "gamedefs.h"
#include "SimpleOptions.h"
#include "options/options_players.h"
#include "options/options_cheats.h"
#include "options/options_graphics.h"
#include "options/options_controllers.h"
#include "options/options_camera.h"
#include "options/options_game.h"
#include "options/options_sounds.h"
#include "options/options_gui.h"
#include "options/options_physics.h"
#include "options/options_debug.h"
#include "ItemManager.h"
#include "createparts.h"
#include "OptionApplier.h"
#include "Flashlight.h"
#include "EnvironmentalEffectManager.h"
#include "WaterManager.h"
#include "GameProfiles.h"
#include "physics/GamePhysics.h"
#include "Weapon.h"
#include "GameWorldFold.h"

#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../system/Miscellaneous.h"
#include "../system/Timer.h"
#include "../sound/sounddefs.h"
#include "../sound/SoundMixer.h"
#include "../sound/SoundLooper.h"
#include "../sound/MusicPlaylist.h"
#include "../sound/AmbientAreaManager.h"
#include "../util/CursorRayTracer.h"
#include "../util/ScreenCapturer.h"
#include "../util/ColorMap.h"
#include "../util/TextureCache.h"
#include "../util/TextureSwitcher.h"
#include "../util/LipsyncManager.h"
#include "../util/HelperPositionCalculator.h"

#include "../ui/DecalPositionCalculator.h"
#include "../ui/cursordefs.h"
#include "../ui/UIState.h"
#include "../ui/Terrain.h"
#include "../ui/TerrainCreator.h"
#include "../ui/ArmorConstructWindow.h"
#include "../ui/MenuCollection.h"
#include "../ui/StorageWindow.h"
#include "../ui/LoadingWindow.h"
#include "../ui/AniRecorderWindow.h"
#include "../ui/CombatWindow.h"
#include "../ui/ICombatSubWindow.h"
#include "../ui/MessageBoxWindow.h"
#include "../ui/GameController.h"
#include "../ui/SelectionBox.h"
#include "../ui/UIEffects.h"
#include "../ui/VisualEffectManager.h"
#include "../ui/VisualEffect.h"
#include "../ui/PlayerUnitCameraBoundary.h"
#include "../ui/ErrorWindow.h"
#include "../ui/UpgradeWindow.h"
#include "../ui/MapWindow.h"
#ifdef GUI_BUILD_LOG_WINDOW
#include "../ui/LogWindow.h"
#endif
#include "../ui/LogManager.h"
#include "../ui/TerminalManager.h"
#include "../ui/CinematicScreen.h"
#ifdef GUI_BUILD_INGAME_GUI_TABS
#include "../ui/IngameGuiTabs.h"
#endif
#include "../ui/Spotlight.h"
#include "../ui/GameConsole.h"
#include "../ui/ScoreWindow.h"
#include "../util/AI_PathFind.h"
#include "../util/Debug_MemoryManager.h"
#include "../ui/AmbientSoundManager.h"
#include "../ui/LoadingMessage.h"
#include "../ui/JoystickAimer.h"
#include "../ui/LightManager.h"
#include "../ui/DynamicLightManager.h"
#include "../util/PositionsDirectionCalculator.h"
#include "../util/AngleRotationCalculator.h"
#include "../util/assert.h"
#include "../ogui/OguiStormDriver.h"
#include "../ui/CameraAutozoomer.h"
#include "../ui/DebugTrackerVisualizer.h"
#include "../ui/DebugProjectileVisualizer.h"
#include "../ui/DebugUnitVisualizer.h"
#include "../ui/DebugMapVisualizer.h"
#include "../ui/SelectionVisualizer.h"
#include "../util/GridOcclusionCuller.h"
#include "../ui/camera_system/CameraSystem.h"
#ifdef PROJECT_SURVIVOR
	#include "../ui/CoopMenu.h"
	#include "../ui/SurvivalMenu.h"
	#include "../ui/ScoreWindow.h"
	#include "../ui/MissionSelectionWindow.h"
	#include "../ui/SurvivorUpgradeWindow.h"
	#include "../ui/VehicleWindow.h"
	#include "../ui/CharacterSelectionWindow.h"
	#include "../ui/MissionFailureWindow.h"
//	#include <keyb3.h>
#endif
#include <keyb3.h>
#include <istorm3D_terrain_renderer.h>
#include <istorm3d_terrain_decalsystem.h>

#include "materials.h"

#include "physics/PhysicsContactSoundManager.h"
#include "physics/PhysicsContactScriptManager.h"
#include "physics/PhysicsContactEffectManager.h"
#include "physics/PhysicsContactDamageManager.h"
#ifdef PHYSICS_FEEDBACK
#include "physics/PhysicsContactFeedback.h"
#endif
#include "tracking/AnyBurnableTrackableObjectFactory.h"

#include "direct_controls.h"


#include <boost/lexical_cast.hpp>

#include "userdata.h"



// HACK:
extern IStorm3D_Scene *disposable_scene;

GAMEUI_SOUND_EFFECT_ERRORCODE gameui_sound_effect_errorcode = GAMEUI_SOUND_EFFECT_ERRORCODE_NONE;

#define AIM_UP_CURSOR_X (1024/2)
#define AIM_UP_CURSOR_Y (768*2/5)
//#define AIM_UP_CURSOR_Y (768/2)

#define SCREEN_CENTER_X (1024/2)
#define SCREEN_CENTER_Y (768/2)


// some really mysterious and unused constant?
//#define CAMERA_EXTRA_ROTATION 0

#define MAX_AIM_OFFSET 10
#define MIN_AIM_OFFSET -50

#define MSGBOX_NOARMORS 1
#define MSGBOX_INCOMPLETEARMORS 2
#define MSGBOX_ABORTMISSION 3
#define MSGBOX_QUITGAME 4

// 25 meters max distance to hear sounds.
#ifdef PROJECT_CLAW_PROTO
#define GAMEUI_MAX_SOUND_DISTANCE_SQ (50 * 50)
#else
#define GAMEUI_MAX_SOUND_DISTANCE_SQ (30 * 30)
#endif

// how much is the camera moved upward when using aim_upward mode
#define CAMERA_AIM_UPWARD_OFFSET 3.0f

// camera smoothing based on player's velocity
#define POSITION_OFFSET_AMOUNT 0.0f

// obstaclemap:gridocclusion scale factor... (in bit shift)
// say, grid occlusion map res x/y is 32 times smaller than obstacle, thus, this value is 5 (1<<5)
#define GRIDOCCLUSIONCULLER_OBSTACLE_DIV_SHIFT 5


extern int lipsync_start_time;

namespace ui
{
extern int visual_object_allocations;
extern int visual_object_model_allocations;
}


extern game::tracking::AnyBurnableTrackableObjectFactory *game_anyBurnableTrackableObjectFactory;


using namespace sfx;

namespace game
{

/*
	bool foofoo_diag1 = false;
	bool foofoo_diag2 = false;
*/

#ifdef PHYSICS_FEEDBACK
	PhysicsContactFeedback *gameui_physicsFeedback = NULL;
#endif
	PhysicsContactSoundManager *gameui_physicsSoundsManager = NULL;
	PhysicsContactDamageManager *gameui_physicsDamageManager = NULL;
	PhysicsContactEffectManager *gameui_physicsEffectManager = NULL;
	//PhysicsContactScriptManager *gameui_physicsScriptManager = NULL;


	///////////////////////////////////////////////////////////////////////////

	bool GameUI::wasKeyClicked( int key )
	{
		bool result = false;

		int i = 0;
		for( i = 0; i < MAX_PLAYERS_PER_CLIENT; i++ )
		{
			result |= gameController[ i ]->wasKeyClicked( key );
		}

		return result;
	}

	///////////////////////////////////////////////////////////////////////////

	GameUI::GameUI(Ogui *ogui, Game *game, IStorm3D *s3d, IStorm3D_Scene *scene, SoundMixer *soundMixer)
		:
	loadingWindow()
	{
		this->ogui = ogui;
		this->game = game;
		this->storm3d = s3d;
		this->scene = scene;
		this->soundMixer = soundMixer;
		this->renderMap = NULL;
		this->renderTerrain = NULL;
		this->terrainCreator = new TerrainCreator(s3d, scene);

		this->oguiStormDriver = NULL;

		this->musicPlaylist = new MusicPlaylist(soundMixer);
		this->ambientAreaManager = new AmbientAreaManager(soundMixer);
		this->soundLooper = new SoundLooper(soundMixer);

		this->ambientSoundManager = new AmbientSoundManager(this, soundLooper);

		this->uiStateStack = new LinkedList();

		this->effects = new UIEffects(ogui, s3d);

		this->aimOffset = 0;
		this->oldAimOffset = 0;

		this->errorWindow = NULL;

		this->thirdPersonView = true;

		this->cameraSystem = new CameraSystem();

		this->forceCursorVisible = false;

		visualEffectManager = new VisualEffectManager(s3d, scene);

		assert(game->getEnvironmentalEffectManager() == NULL);
		EnvironmentalEffectManager *enveffman = new EnvironmentalEffectManager(game, visualEffectManager);
		game->setEnvironmentalEffectManager(enveffman);

		armorConstructWindows = new ArmorConstructWindow *[ABS_MAX_PLAYERS];
		commandWindows = new MenuCollection *[ABS_MAX_PLAYERS];
		storageWindows = new StorageWindow *[ABS_MAX_PLAYERS];
		combatWindows = new CombatWindow *[ABS_MAX_PLAYERS];
		armorIncompleteConfirmWindows = new MessageBoxWindow *[ABS_MAX_PLAYERS];
		quitBox = new MessageBoxWindow *[ABS_MAX_PLAYERS];

		int i;
		for (i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			commandWindows[i] = NULL;
			storageWindows[i] = NULL;
			armorConstructWindows[i] = NULL;
			combatWindows[i] = NULL;
			armorIncompleteConfirmWindows[i] = NULL;
			quitBox[i] = NULL;
		}

		quitRequested = false;
		abortMission = false;

		aniRecorderWindow = NULL;
		upgradeWindow = NULL;
		vehicleWindow = NULL;
		characterSelectionWindow = NULL;
#ifdef GUI_BUILD_MAP_WINDOW
		mapWindow = NULL;
#endif
#ifdef GUI_BUILD_INGAME_GUI_TABS
		ingameGuiTabs = NULL;
#endif
#ifdef GUI_BUILD_LOG_WINDOW
		logWindow = NULL;
#endif
		logManager = new LogManager();

		
#ifdef PROJECT_SURVIVOR
		useScoreWindow = true;
#else
		useScoreWindow = false;
#endif
		scoreWindow = NULL;
		missionSelectionWindow = NULL;

		missionFailureWindow = NULL;


		terminalManager = new TerminalManager( ogui, game );

		cinematicScreen = NULL;

		unitCameraBoundary = new PlayerUnitCameraBoundary(game->units, 0);

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				gameController[c] = new GameController(ogui);
				
				bool load_options = false;

				if( c == 0 ) 
					load_options = SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED );
				else if ( c == 1 )
					load_options = SimpleOptions::getBool( DH_OPT_B_2ND_PLAYER_ENABLED );
				else if ( c == 2 )
					load_options = SimpleOptions::getBool( DH_OPT_B_3RD_PLAYER_ENABLED );
				else if ( c == 3 )
					load_options = SimpleOptions::getBool( DH_OPT_B_4TH_PLAYER_ENABLED );
				

				/*
				if (c == 0)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS));
				else if (c == 1)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS));
				else if (c == 2)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_3RD_PLAYER_KEYBINDS));
				else if (c == 3)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_4TH_PLAYER_KEYBINDS));
				*/

				if( load_options )
				{
#ifdef LEGACY_FILES
					std::string tmp = igios_mapUserDataPrefix("Profiles/");
					tmp += game->getGameProfiles()->getCurrentProfile( c );
					tmp += "/Config/keybinds.txt";
#else
					std::string tmp = igios_mapUserDataPrefix("profiles/");
					tmp += game->getGameProfiles()->getCurrentProfile( c );
					tmp += "/config/keybinds.txt";
#endif
					gameController[c]->loadConfiguration(tmp.c_str());
				}
				gameController[c]->setMouseJoystickAutodetection( true );
			}
		}

		createCameras();

		cursorRayTracer = NULL;

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				sceneSelection[c] = new SceneSelection();
			}
		}

		pointersChanged = false;
		unitsDestroyed = false;
		unitsDamaged = false;

		meterUpdateTime = 0;
		guiAnimationUpdateTime = 0;

		buildingHandlerUpdateTime = 0;

		msgBoxIsOpen = false;

		lastRunUITime = 0;

		{
			for (int msg = 0; msg < MESSAGE_TYPES_AMOUNT; msg++)
			{
				lastGameMessageCounter[msg] = 0;
				lastGameMessageDuration[msg] = 0;
				lastGameMessagePriority[msg] = 0;
			}
		}

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				firstPerson[c] = NULL;
			}
		}

		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			cameras[GAMEUI_CAMERA_NORMAL]->setThirdPersonView(true);
			thirdPersonView = true;
			controlModeDirect = true;
		} else {
			thirdPersonView = false;
			controlModeDirect = false;
		}

		cameraTimeFactor = 0;
		originalTimeFactor = 1.0f;

		visualEffectManager->loadParticleEffects();

		keyreaderId = -1;
		console = NULL;

		leftDirectRotation = false;
		rightDirectRotation = false;

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				joystickAimer[c] = NULL;
				oldJoystickXY[c] = VC2(0.0f, 0.0f);

				leftMovementEnabled[c] = true;
				rightMovementEnabled[c] = true;
				upMovementEnabled[c] = true;
				downMovementEnabled[c] = true;

				fireKeyDownPreviously[c] = false;
				fireSecondaryKeyDownPreviously[c] = false;

				lastPrimaryWeapon[c] = -1;
				lastSecondaryWeapon[c] = -1;

				clientUnitScreenPos[c] = VC2(SCREEN_CENTER_X, SCREEN_CENTER_Y);
			}
		}

		this->lightningTime = 0;
		this->lightningVisualEffect = NULL;

#ifdef PROJECT_SURVIVOR
		this->scrollyEnabled = false;
		this->scrollyTemporarilyDisabled = false;
		this->setScrollyEnabled(false);
#else
		this->scrollyEnabled = true;
		this->scrollyTemporarilyDisabled = false;
		this->setScrollyEnabled(true);
#endif

		this->textureCache = new frozenbyte::TextureCache(*storm3d);
		this->textureSwitcher = new util::TextureSwitcher(*textureCache);

		VisualObject::setTextureCache(this->textureCache);

		this->lightManager = NULL;
		this->dynamicLightManager = NULL;

		this->listenerPosition = VC3(0,0,0);

		this->playerSelfIllumEnabled = true;

		effects->startFadeIn(500);

		this->gridOcclusionCuller = NULL;

		this->movieAspectRatioGUIVisible = -1;


		// TODO: rather no static shit like this...
#ifdef PHYSICS_FEEDBACK
		if (gameui_physicsFeedback == NULL)
		{
			gameui_physicsFeedback = new PhysicsContactFeedback(this->game);
		}
		game->getGamePhysics()->addPhysicsContactListener(gameui_physicsFeedback);
#endif
		if (gameui_physicsSoundsManager == NULL)
		{
			gameui_physicsSoundsManager = new PhysicsContactSoundManager(this);
		}
		game->getGamePhysics()->addPhysicsContactListener(gameui_physicsSoundsManager);
		if (gameui_physicsEffectManager == NULL)
		{
			gameui_physicsEffectManager = new PhysicsContactEffectManager(this->game);
		}
		game->getGamePhysics()->addPhysicsContactListener(gameui_physicsEffectManager);
		if (gameui_physicsDamageManager == NULL)
		{
			gameui_physicsDamageManager = new PhysicsContactDamageManager(this->game);
		}
		game->getGamePhysics()->addPhysicsContactListener(gameui_physicsDamageManager);
		//if (gameui_physicsScriptManager == NULL)
		//{
		//	gameui_physicsScriptManager = new PhysicsContactScriptManager(this->game);
		//}
		//game->getGamePhysics()->addPhysicsContactListener(gameui_physicsScriptManager);

		for(int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
		{
			SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME + c, gameController[c]->getControllerType() );
		}
		setCursorControllers();
	}


	GameUI::~GameUI()
	{
		// TODO: rather no static shit like this...
#ifdef PHYSICS_FEEDBACK
		delete gameui_physicsFeedback;
		gameui_physicsFeedback = NULL;
#endif

		delete gameui_physicsSoundsManager;
		gameui_physicsSoundsManager = NULL;

		delete gameui_physicsEffectManager;
		gameui_physicsEffectManager = NULL;

		delete gameui_physicsDamageManager;
		gameui_physicsDamageManager = NULL;

		//delete gameui_physicsScriptManager;
		//gameui_physicsScriptManager = NULL;


		assert(lightManager == NULL);
		if (lightManager != NULL)
		{
			delete lightManager;
			lightManager = NULL;
		}
		assert(dynamicLightManager == NULL);
		if (dynamicLightManager != NULL)
		{
			delete dynamicLightManager;
			dynamicLightManager = NULL;
		}

		delete console;
		console = NULL;

		delete textureSwitcher;
		delete textureCache;

		while (!uiStateStack->isEmpty())
		{
			UIState *tmp = (UIState *)uiStateStack->popLast();
			char str[256];
			sprintf(str, "UIState with ID %i left unpopped", tmp->id);
			Logger::getInstance()->error(str);
			delete tmp;
		}
		delete uiStateStack;

		if (lightningVisualEffect != NULL)
		{
			lightningVisualEffect->setDeleteFlag();
			lightningVisualEffect->freeReference();
			lightningVisualEffect = NULL;
		}

		if (effects != NULL)
		{
			delete effects;
		}

		detachVisualEffects();

		assert(game->getEnvironmentalEffectManager() != NULL);
		delete game->getEnvironmentalEffectManager();
		game->setEnvironmentalEffectManager(NULL);

		visualEffectManager->freeParticleEffects(); 
		visualEffectManager->freeDecalEffects();
		delete visualEffectManager;

		delete ambientSoundManager;
		
		delete soundLooper;

		delete musicPlaylist;
		delete ambientAreaManager;
		ogui = NULL;
		if (renderTerrain != NULL)
		{
			//scene->RemoveTerrain(renderTerrain);
			game_anyBurnableTrackableObjectFactory->removeImplementation(renderTerrain);
			delete renderTerrain;
			renderTerrain = NULL;
		}
		renderMap = NULL;
		scene = NULL;
		storm3d = NULL;

		game = NULL;

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				delete sceneSelection[c];
			}
		}

		deleteCameras();
		delete cameraSystem;
		//delete gameCamera;

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				delete gameController[c];
			}
		}

		delete terrainCreator;

		delete unitCameraBoundary;

		if (cursorRayTracer != NULL)
		{
			delete cursorRayTracer;
		}

		{
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				if (joystickAimer[c] != NULL)
				{
					delete joystickAimer[c];
					joystickAimer[c] = NULL;
				}
			}
		}

		if (upgradeWindow != NULL)
		{
			delete upgradeWindow;
			upgradeWindow = NULL;
		}

#ifdef PROJECT_SURVIVOR
		if (vehicleWindow != NULL)
		{
			delete vehicleWindow;
			vehicleWindow = NULL;
		}
		if (characterSelectionWindow != NULL)
		{
			delete characterSelectionWindow;
			characterSelectionWindow = NULL;
		}
#endif


#ifdef GUI_BUILD_MAP_WINDOW
		if (mapWindow != NULL)
		{
			delete mapWindow;
			mapWindow = NULL;
		}
#endif

#ifdef GUI_BUILD_LOG_WINDOW
		if ( logWindow != NULL )
		{
			delete logWindow;
			logWindow = NULL;
		}
#endif

		delete logManager;
		logManager = NULL;

		delete terminalManager;
		terminalManager = NULL;

		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			if (armorConstructWindows[i] != NULL)
			{
				delete armorConstructWindows[i];
				armorConstructWindows[i] = NULL;
			}
			if (storageWindows[i] != NULL)
			{
				delete storageWindows[i];
				storageWindows[i] = NULL;
			}
			if (commandWindows[i] != NULL)
			{
				delete commandWindows[i];
				commandWindows[i] = NULL;
			}
			if (combatWindows[i] != NULL)
			{
				delete combatWindows[i];
				combatWindows[i] = NULL;
			}
			if (quitBox[i] != NULL)
			{
				delete quitBox[i];
				quitBox[i] = NULL;
			}
			if (armorIncompleteConfirmWindows[i] != NULL)
			{
				delete armorIncompleteConfirmWindows[i];
				armorIncompleteConfirmWindows[i] = NULL;
			}
		}
		delete [] armorIncompleteConfirmWindows;
		delete [] quitBox;
		delete [] storageWindows;
		delete [] armorConstructWindows;
		delete [] commandWindows;
		delete [] combatWindows;
	}

	float GameUI::getVideoVolume() const
	{
		float masterVolume = SimpleOptions::getInt(DH_OPT_I_MASTER_VOLUME) / 100.f;
		float fxVolume = SimpleOptions::getInt(DH_OPT_I_FX_VOLUME)  / 100.f;
		float volume = masterVolume * fxVolume;
		if(!soundMixer || !SimpleOptions::getBool(DH_OPT_B_FX_ENABLED))
			volume = 0;

		return volume;
	}

	void GameUI::setCursorControllers(bool allowJoystick)
	{
		bool mouse_given_to_anyone = false;
		int c;
		for( c = 0; c < MAX_PLAYERS_PER_CLIENT; c++ )
		{
			if ( game::SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + c ) )
			{
				int controller_scheme = game::SimpleOptions::getInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME + c );

				// player using cursor (mouse)
				//
				if(controller_scheme >= GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 && controller_scheme <= GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE4)
				{
					// give cursor
					game::SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c, true );

					mouse_given_to_anyone = true;

					int mousecontroller = OGUI_CURSOR_CTRL_MOUSE;

					bool multmouse = game::SimpleOptions::getBool( DH_OPT_B_CONTROLLER_MULTIPLE_INPUT_DEVICES_ENABLED );
					if(multmouse)
					{
						// Reads mouse device ID's from configuration and assigns them to the player.
						//int cID = game::SimpleOptions::getInt ( DH_OPT_I_CONTROLLER_PLAYER1_MOUSE_ID + c );
						int cID = controller_scheme - GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;
						mousecontroller = (OGUI_CURSOR_CTRL_MOUSE0 << cID);
					}

					if(!allowJoystick)
						ogui->SetCursorController( c, mousecontroller | OGUI_CURSOR_CTRL_DISABLE_JOYSTICK_WARP );
					else
						ogui->SetCursorController( c, mousecontroller );
				}

				// player using joystick or keyboard only
				//
				else if(allowJoystick)
				{
					// disable cursor
					game::SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c, false );

					int joy = controller_scheme - GameController::CONTROLLER_TYPE_JOYSTICK1;
					if ( joy == 0 )
						ogui->SetCursorController( c, OGUI_CURSOR_CTRL_JOYSTICK1 );
					else if ( joy == 1 )
						ogui->SetCursorController( c, OGUI_CURSOR_CTRL_JOYSTICK2 );
					else if ( joy == 2 )
						ogui->SetCursorController( c, OGUI_CURSOR_CTRL_JOYSTICK3 );
					else if ( joy == 3 )
						ogui->SetCursorController( c, OGUI_CURSOR_CTRL_JOYSTICK4 );
					else if ( controller_scheme == GameController::CONTROLLER_TYPE_KEYBOARD_ONLY )
						ogui->SetCursorControllerKeyboard( c, gameController[c]->getBoundKey(DH_CTRL_CAMERA_MOVE_LEFT, 0),
																									gameController[c]->getBoundKey(DH_CTRL_CAMERA_MOVE_RIGHT, 0),
																									gameController[c]->getBoundKey(DH_CTRL_CAMERA_MOVE_FORWARD, 0),
																									gameController[c]->getBoundKey(DH_CTRL_CAMERA_MOVE_BACKWARD, 0),
																									gameController[c]->getBoundKey(DH_CTRL_ATTACK, 0),
																									gameController[c]->getBoundKey(DH_CTRL_ATTACK_SECONDARY, 0));
					else
						ogui->SetCursorController( c, 0);
				}
			}
			else
			{
				ogui->SetCursorController( c, 0 );
			}
		}

		// failsafe: always use mouse in at least one controller
		if(!mouse_given_to_anyone)
		{
			ogui->SetCursorController( 0, ogui->GetCursorController(0) | OGUI_CURSOR_CTRL_MOUSE );
		}
	}

	void GameUI::missionStarted()
	{
		{
			for (int msg = 0; msg < MESSAGE_TYPES_AMOUNT; msg++)
			{
				lastGameMessageCounter[msg] = 0;
				lastGameMessageDuration[msg] = 0;
				lastGameMessagePriority[msg] = 0;
			}
		}

		for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
		{
			if (gameController[c] != NULL)
			{
				/*
				if (c == 0)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS));
				else if (c == 1)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS));
				else if (c == 2)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_3RD_PLAYER_KEYBINDS));
				else if (c == 3)
					gameController[c]->loadConfiguration(SimpleOptions::getString(DH_OPT_S_4TH_PLAYER_KEYBINDS));
			    */
			}
		}


		// hack: character selection window doesn't use per-player controllers..
		if(characterSelectionWindow == NULL)
		{
			setCursorControllers();
		}


		/*
		// TEMP HACK:
		if (game::SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED))
		{
			int cnum = 0;
			if (game::SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_HAS_CURSOR))
			{
				ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_MOUSE);	
			} else {
				// EXTRA HACK! :)
				if (!game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED)
					|| !game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_HAS_CURSOR))
				{
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_MOUSE);
				} else {
					int joy = game::SimpleOptions::getInt(DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME) / 3;
					if (joy == 0)
						ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK1);
					if (joy == 1)
						ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK2);
				}
			}
			
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
		{
			int cnum = 1;
			if (game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_HAS_CURSOR))
			{
				ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_MOUSE);	
			} else {
				int joy = game::SimpleOptions::getInt(DH_OPT_I_2ND_PLAYER_CONTROL_SCHEME) / 3;
				if (joy == 0)
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK1);
				if (joy == 1)
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK2);
			}
		} else {
			ogui->SetCursorController(1, 0);
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
		{
			int cnum = 2;
			if (game::SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_HAS_CURSOR))
			{
				ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_MOUSE);	
			} else {
				int joy = game::SimpleOptions::getInt(DH_OPT_I_3RD_PLAYER_CONTROL_SCHEME) / 3;
				if (joy == 0)
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK1);
				if (joy == 1)
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK2);
			}
		} else {
			ogui->SetCursorController(2, 0);
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED))
		{
			int cnum = 3;
			if (game::SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_HAS_CURSOR))
			{
				ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_MOUSE);	
			} else {
				int joy = game::SimpleOptions::getInt(DH_OPT_I_4TH_PLAYER_CONTROL_SCHEME) / 3;
				if (joy == 0)
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK1);
				if (joy == 1)
					ogui->SetCursorController(cnum, OGUI_CURSOR_CTRL_JOYSTICK2);
			}
		} else {
			ogui->SetCursorController(3, 0);
		}
		*/
		// MORE HACK...
#if defined(PROJECT_SHADOWGROUNDS)
		ogui->SetCursorImageState(1, DH_CURSOR_AIM_PLAYER2);
		ogui->SetCursorImageState(2, DH_CURSOR_AIM_PLAYER3);
		ogui->SetCursorImageState(3, DH_CURSOR_AIM_PLAYER4);
#endif

		// EXTRA HACK
#ifdef GUI_BUILD_MAP_WINDOW
		FB_ASSERT(!mapWindow);
		mapWindow = new MapWindow(*game, *ogui, game->map);
#endif

#ifdef GUI_BUILD_INGAME_GUI_TABS
		FB_ASSERT( ingameGuiTabs == NULL );
		ingameGuiTabs = new IngameGuiTabs( ogui, game ) ;
		ingameGuiTabs->hide();
#endif
#ifdef GUI_BUILD_MAP_WINDOW
		// MEGA HACK
		MapScripting::applyPortals(game);
#endif

#ifdef PROJECT_SURVIVOR
		SurvivorUpgradeWindow::preloadTextures(this);
#endif

		getTerrain()->GetTerrain()->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0);
	}


	void GameUI::missionEnded()
	{
		// clear uistack to be safe
		while (!uiStateStack->isEmpty())
		{
			UIState *tmp = (UIState *)uiStateStack->popLast();
			char str[256];
			sprintf(str, "UIState with ID %i left unpopped", tmp->id);
			Logger::getInstance()->error(str);
			delete tmp;
		}

		movieAspectRatioGUIVisible = -1;

#ifdef PROJECT_SURVIVOR
		if(ogui)
			ogui->ResetSwappedCursorImages();

		game->gameUI->getStorm3D()->setGlobalTimeFactor(1.0f);
		game->gameUI->setSoundFrequencyFactor(1.0f);
		Timer::setTimeFactor(1.0f);
		originalTimeFactor = 1.0f;
#endif

		this->effects->clearFilterEffects();
		this->effects->setMovieAspectRatio(false);

		this->effects->clearAllMaskPictures();
		if (this->aniRecorderWindow != NULL)
			this->closeAniRecorderWindow(0);
		if (this->upgradeWindow != NULL)
			this->closeUpgradeWindow(this->firstPerson[0]);
		if (this->vehicleWindow != NULL)
			this->closeVehicleGUI();

#ifdef PROJECT_SURVIVOR
		this->scrollyEnabled = false;
		this->setScrollyEnabled(false);
#endif

#ifdef GUI_BUILD_MAP_WINDOW
		if(mapWindow)
		{
			mapWindow->hide();
			delete mapWindow;
			mapWindow = 0;
		}
#endif

#ifdef GUI_BUILD_INGAME_GUI_TABS
		if( ingameGuiTabs )
		{
			delete ingameGuiTabs;
			ingameGuiTabs = NULL;
		}
#endif

		//soundMixer->stopAllSounds();
		loopingSoundEffectHandles.clear();

		// disable after each mission
#ifdef PROJECT_SURVIVOR
		enableAlphaTestPass(false);
#endif
	}


	void GameUI::detachVisualEffects()
	{
		// need to get rid of lightning too.. do it here..
		if (lightningVisualEffect != NULL)
		{
			lightningVisualEffect->setDeleteFlag();
			lightningVisualEffect->freeReference();
			lightningVisualEffect = NULL;
		}

		// detach projectiles from visual effects
		LinkedList *projlist = game->projectiles->getAllProjectiles();
		LinkedListIterator projiter = LinkedListIterator(projlist);
		while (projiter.iterateAvailable())
		{
			Projectile *proj = (Projectile *)projiter.iterateNext();
			proj->setVisualEffect(NULL);
		}

		// detach units from visual effects
		LinkedList *unitlist = game->units->getAllUnits();
		LinkedListIterator unititer = LinkedListIterator(unitlist);
		while (unititer.iterateAvailable())
		{
			Unit *u = (Unit *)unititer.iterateNext();
			u->setMuzzleflashVisualEffect(NULL, 0);
			u->setPointerVisualEffect(NULL);
			u->setPointerHitVisualEffect(NULL);
		}
		// and the other way dependencies too... 
		visualEffectManager->detachVisualEffectsFromUnits();

	}


	void GameUI::createCameras()
	{
		for (int i = 0; i < GAMEUI_CAMERA_AMOUNT; i++)
		{
			cameras[i] = new GameCamera(scene, game->gameMap, gameController);
		}
		cameras[GAMEUI_CAMERA_NORMAL]->setMode(GameCamera::CAMERA_MODE_TARGET_CENTRIC);
		cameras[GAMEUI_CAMERA_NORMAL]->setMaxZoom(80);
		cameras[GAMEUI_CAMERA_NORMAL]->setMiddleZoom(40);
		cameras[GAMEUI_CAMERA_NORMAL]->setMinZoom(1);
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			int players = 1;
			if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
			{
				players++;
			}
			if (SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
			{
				players++;
			}
			if (SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED))
			{
				players++;
			}
			cameras[GAMEUI_CAMERA_NORMAL]->setFOV((float)SimpleOptions::getInt(DH_OPT_I_CAMERA_DEFAULT_FOV));
			cameras[GAMEUI_CAMERA_NORMAL]->setZoom(SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM) + (float)players * SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM_PLAYER_INC));
		} else {
			cameras[GAMEUI_CAMERA_NORMAL]->setZoom(20);
		}
		cameras[GAMEUI_CAMERA_NORMAL]->setFollowingUnit(false);
		if (!SimpleOptions::getBool(DH_OPT_B_NO_CAMERA_BOUNDARIES))
			cameras[GAMEUI_CAMERA_NORMAL]->setBoundaries(unitCameraBoundary);
		cameras[GAMEUI_CAMERA_TACTICAL]->setMaxZoom(80);
		cameras[GAMEUI_CAMERA_TACTICAL]->setMiddleZoom(40);
		cameras[GAMEUI_CAMERA_TACTICAL]->setMinZoom(4);
		cameras[GAMEUI_CAMERA_TACTICAL]->setZoom(100);
		cameras[GAMEUI_CAMERA_TACTICAL]->setFollowingUnit(false);
		cameras[GAMEUI_CAMERA_CINEMATIC1]->setMaxZoom(200);
		cameras[GAMEUI_CAMERA_CINEMATIC1]->setMiddleZoom(50);
		cameras[GAMEUI_CAMERA_CINEMATIC1]->setMinZoom(4);
		cameras[GAMEUI_CAMERA_CINEMATIC1]->setZoom(4);
		cameras[GAMEUI_CAMERA_CINEMATIC1]->setFollowingUnit(false);
		cameras[GAMEUI_CAMERA_CINEMATIC2]->setMaxZoom(200);
		cameras[GAMEUI_CAMERA_CINEMATIC2]->setMiddleZoom(50);
		cameras[GAMEUI_CAMERA_CINEMATIC2]->setMinZoom(4);
		cameras[GAMEUI_CAMERA_CINEMATIC2]->setZoom(4);
		cameras[GAMEUI_CAMERA_CINEMATIC2]->setFollowingUnit(false);
		gameCamera = cameras[GAMEUI_CAMERA_NORMAL];
	
		if (thirdPersonView)
			cameras[GAMEUI_CAMERA_NORMAL]->setThirdPersonView(true);

		// NOTE: actually, this call sets the upvector for _all_ cameras!
		if (SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS))
		{
			cameras[GAMEUI_CAMERA_NORMAL]->setUpVector(VC3(0,0,1));
		} else {
			cameras[GAMEUI_CAMERA_NORMAL]->setUpVector(VC3(0,1,0));
		}

		CameraAutozoomer::resetForNewMission();
	}


	void GameUI::deleteCameras()
	{
		for (int i = 0; i < GAMEUI_CAMERA_AMOUNT; i++)
		{
			if (cameras[i] != NULL)
			{
				delete cameras[i];
				cameras[i] = NULL;
			}
		}
	}


	void GameUI::openArmorConstructWindow(int player)
	{
		if (armorConstructWindows[player] == NULL)
		{
			armorConstructWindows[player] = 
				new ArmorConstructWindow(ogui, game, player);
			armorConstructWindows[player]->show();
		} else {
			armorConstructWindows[player]->show();
			// already open
			//#ifdef _DEBUG
			//	abort();
			//#endif
		}
	}

	void GameUI::closeArmorConstructWindow(int player)
	{
		if (armorConstructWindows[player] == NULL)
		{
			// already closed
			//#ifdef _DEBUG
			//	abort();
			//endif
		} else {
			delete armorConstructWindows[player];
			armorConstructWindows[player] = NULL;
		}
	}

	void GameUI::refreshArmorConstructWindow(int player)
	{
		if (armorConstructWindows[player] == NULL)
		{
			// not opened
			#ifdef _DEBUG
				abort();
			#endif
		} else {
			armorConstructWindows[player]->refresh();
		}
	}

	void GameUI::openCommandWindow(int player)
	{
		// close abort mission / quit box if it is open...
		if (quitBox[player] != NULL)
		{
			delete quitBox[player];
			quitBox[player] = NULL;
			// TODO: what if some other messagebox is open?
			msgBoxIsOpen = false;
		}
		// then open command window...
		if (commandWindows[player] == NULL)
		{
			commandWindows[player] = 
				new MenuCollection(ogui, game, player, false);

			commandWindows[player]->show();
		} else {
			commandWindows[player]->show();
			// already open
			//#ifdef _DEBUG
			//	abort();
			//#endif
		}
	}

	void GameUI::closeCommandWindow(int player)
	{
		if (commandWindows[player] == NULL)
		{
			// already closed
			//#ifdef _DEBUG
			//	abort();
			//endif
		} else {
			delete commandWindows[player];
			commandWindows[player] = NULL;
			
			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true );
			
		}
	}

	void GameUI::startCommandWindow(int player)
	{
		if( commandWindows[player] != NULL )
			commandWindows[player]->start();
	}

	// added by Pete to open main menu from game, when pressed esc
	void GameUI::openMainmenuFromGame( int menu )
	{
		int player = 0;

		// then open command window...
		if (commandWindows[player] == NULL)
		{
			game->setPaused( true );
			ambientAreaManager->fadeOut(500);

			commandWindows[player] = 
				new MenuCollection( ogui, game, player, true, menu );

			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);
			
			commandWindows[player]->show();
		} else {
			commandWindows[player]->show();
		}
	}

	// added by Pete to close main menu and resuma game 
	void GameUI::resumeGame()
	{
		int player = 0;

		if( commandWindows[ player ] != NULL )
		{
			delete commandWindows[player];
			commandWindows[player] = NULL;
			
			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true );

			game->setPaused( false );
			ambientAreaManager->fadeIn();
		}
	}


	void GameUI::openStorageWindow(int player)
	{
		if (storageWindows[player] == NULL)
		{
			storageWindows[player] = 
				new StorageWindow(ogui, game, player);
			storageWindows[player]->show();
		} else {
			storageWindows[player]->show();
			// already open
			//#ifdef _DEBUG
			//	abort();
			//#endif
		}
	}

	void GameUI::closeStorageWindow(int player)
	{
		if (storageWindows[player] == NULL)
		{
			// already closed
			//#ifdef _DEBUG
			//	abort();
			//endif
		} else {
			delete storageWindows[player];
			storageWindows[player] = NULL;
		}
	}

	void GameUI::doneLoading(int player)
	{
		// TODO: split screen: solve player's cursor id first
		ogui->SetCursorImageState(0, DH_CURSOR_ARROW);

		if (loadingWindow != NULL)
		{
			setCursorControllers();
			loadingWindow->enableClose();
		}
	}

	void GameUI::openLoadingWindow(int player)
	{
		// TODO: split screen: solve player's cursor id first
		ogui->SetCursorImageState(0, DH_CURSOR_INVISIBLE);
		ogui->SetCursorImageState(1, DH_CURSOR_INVISIBLE);
		ogui->SetCursorImageState(2, DH_CURSOR_INVISIBLE);
		ogui->SetCursorImageState(3, DH_CURSOR_INVISIBLE);

		// player is meaningless as long as it on in this client...
		// TODO, NETGAME: check that
		if (loadingWindow == NULL)
		{
			loadingWindow.reset(new LoadingWindow(ogui, game, player));
		} else {
			// already open
			//assert(!"GameUI::openLoadingWindow - Attempt to reopen loading window (when it is already open).");
		}

		// this looks like a bad idea...
		ogui->Run(GAME_TICK_MSEC);
		// this would be better (to just draw the windows not cursors, and no events...)
		//ogui->DrawScreen();
		scene->RenderScene();
		game->gameUI->setGUIVisibility(game->singlePlayerNumber, false);
	}

	/*
	void GameUI::stopLoading(int player)
	{
		game->setPaused(false);
		game->gameUI->setGUIVisibility(game->singlePlayerNumber, true);
	}
	*/

	void GameUI::closeLoadingWindow(int player)
	{
		if (loadingWindow == NULL)
		{
			/*
			// not opened
			#ifdef _DEBUG
				abort();
			#endif
			*/
		} else {
			loadingWindow.reset();
		}

		// show combat windows
		//if(combatWindows && combatWindows[player]) combatWindows[player]->endGUIModeTempInvisible();
		// unpause
		//game->setPaused(false);
		//game->gameUI->setGUIVisibility(game->singlePlayerNumber, true);

#ifdef PROJECT_SURVIVOR
		if(vehicleWindow != NULL)
		{
			vehicleWindow->setCombatWindowVisibility();
		}
#endif
	}

	bool GameUI::isScoreWindowInUse() const
	{
		return useScoreWindow;
	}

	bool GameUI::isScoreWindowOpen() const
	{
		return ( scoreWindow != NULL );
	}

	void GameUI::openScoreWindow( int player )
	{
#ifndef PROJECT_SHADOWGROUNDS
		scoreWindow = new ui::ScoreWindow( ogui, game, player );
#endif

		if(combatWindows[game->singlePlayerNumber])
		{
			combatWindows[game->singlePlayerNumber]->clearMessage();
			combatWindows[game->singlePlayerNumber]->clearCenterMessage();
			combatWindows[game->singlePlayerNumber]->clearExecuteTipMessage();
			combatWindows[game->singlePlayerNumber]->clearHintMessage();
		}

		ogui->Run(GAME_TICK_MSEC);
		// this would be better (to just draw the windows not cursors, and no events...)
		//ogui->DrawScreen();
		scene->RenderScene();
		// game->gameUI->setGUIVisibility(game->singlePlayerNumber, false);

		{
			game->setPaused( true );

			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);
		}
	}

	void GameUI::closeScoreWindow( int player )
	{
#ifdef PROJECT_SURVIVOR
		if(game->isCooperative())
		{
			// open next score window
			player = scoreWindow->getPlayer() + 1;

			if( player < MAX_PLAYERS_PER_CLIENT
				&& game::SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + player ) )
			{
				effects->startFadeOutIfNotFaded(300);
				scoreWindow->setPlayer(player);
				return;
			}			
		}
#endif

		{
			game->setPaused( false );

			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);
		}

#ifndef PROJECT_SHADOWGROUNDS

#ifdef PROJECT_SURVIVOR
		if(scoreWindow->shouldRestart())
		{
			if(game->isCooperative())
			{
				SurvivalMenu::startAsCoop = true;
			}
			SurvivalMenu::reloadLastMission(game);
		}
		else
#endif
		{
			// force end game
			game->gameUI->setAbortingMission(true);
		}

		delete scoreWindow;
#endif
		scoreWindow = NULL;

	}

	bool GameUI::scoreWindowAllowsLoading() const
	{
		if( scoreWindow == NULL )
		{
			return true;
		}
		else 
		{
#ifdef PROJECT_SHADOWGROUNDS
			return true;
#else
			return scoreWindow->AllowLoading();
#endif
		}
	}

	//=========================================================================

	void GameUI::openMissionSelectionWindow()
	{
#ifdef PROJECT_SURVIVOR
		if( missionSelectionWindow == NULL )
			missionSelectionWindow = new MissionSelectionWindow( ogui, game );
#endif
		ogui->Run(GAME_TICK_MSEC);
		// this would be better (to just draw the windows not cursors, and no events...)
		//ogui->DrawScreen();
		scene->RenderScene();
		// game->gameUI->setGUIVisibility(game->singlePlayerNumber, false);

		{
			game->setPaused( true );

			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);
		}
	}

	//=========================================================================

	void GameUI::closeMissionSelectionWindow()
	{
		{
			game->setPaused( false );

			if( renderTerrain )
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);
		}

#ifdef PROJECT_SURVIVOR
		delete missionSelectionWindow;
#endif
		missionSelectionWindow = NULL;
	}

	MissionSelectionWindow* GameUI::getMissionSelectionWindow() const
	{
		return missionSelectionWindow;
	}

	//=========================================================================

	void GameUI::openAniRecorderWindow(int player)
	{
		// TODO: split screen: solve player's cursor id first
		ogui->SetCursorImageState(0, DH_CURSOR_ARROW);

		// player is meaningless as long as it on in this client...
		// TODO, NETGAME: check that
		if (aniRecorderWindow == NULL)
		{
			aniRecorderWindow = new AniRecorderWindow(ogui, game);
		} else {
			// already open
			#ifdef _DEBUG
				abort();
			#endif
		}
	}

	void GameUI::closeAniRecorderWindow(int player)
	{
		if (aniRecorderWindow == NULL)
		{
			/*
			// not opened
			#ifdef _DEBUG
				abort();
			#endif
			*/
		} else {
			delete aniRecorderWindow;
			aniRecorderWindow = NULL;
		} 	 
	}

	void GameUI::openWindow( WINDOW_TYPE type, int player )
	{
		if(scoreWindow || characterSelectionWindow || missionFailureWindow) return;

		if(vehicleWindow && type != WINDOW_TYPE_MAP)
			return;

#ifdef GUI_BUILD_INGAME_GUI_TABS
		updateIngameTabs();
#endif
		switch( type )
		{
#ifdef GUI_BUILD_MAP_WINDOW
		case WINDOW_TYPE_MAP:
			{
				if(game->inCombat && firstPerson[player] && mapWindow && commandWindows[ player ] == NULL )
				{
					if(mapWindow->isVisible() == false)
					{
#ifdef GUI_BUILD_INGAME_GUI_TABS
						if( openMapWindow() && ingameGuiTabs )
							ingameGuiTabs->setActive( type );
#else
						openMapWindow();
#endif
					}
				}
			}

			break;
#endif

		case WINDOW_TYPE_UPGRADE:
			{
				if ( game->inCombat && firstPerson[player] != NULL && commandWindows[ player ] == NULL )
				{
					if (this->upgradeWindow == NULL)
					{
#ifdef GUI_BUILD_INGAME_GUI_TABS
						if( this->openUpgradeWindow(firstPerson[player]) && ingameGuiTabs )
							ingameGuiTabs->setActive( type );
						else
							FB_ASSERT( false );
#else
						this->openUpgradeWindow(firstPerson[player]);
#endif
					}
					else
					{
						FB_ASSERT( false );
					}
				}
			}
			break;

#ifdef GUI_BUILD_LOG_WINDOW
		case WINDOW_TYPE_LOG:
			{
				if( game->inCombat && firstPerson[player] != NULL && commandWindows[ player ] == NULL )
				{
					if( this->logWindow == NULL )
					{
#ifdef GUI_BUILD_INGAME_GUI_TABS
						if( this->openLogWindow() && ingameGuiTabs )
							ingameGuiTabs->setActive( type );
#else
						this->openLogWindow();
#endif
					}
				}
			}
			break;
#endif

		default:
			break;
		}
#ifdef GUI_BUILD_INGAME_GUI_TABS
		updateIngameTabs();
#endif
	}

	void GameUI::closeWindow( WINDOW_TYPE type, bool save_changes, int player )
	{
		switch( type )
		{
#ifdef GUI_BUILD_MAP_WINDOW
		case WINDOW_TYPE_MAP:
			{
				if(game->inCombat && firstPerson[player] && mapWindow && commandWindows[ player ] == NULL )
				{
					if(mapWindow->isVisible())
						closeMapWindow();
				}
			}
			break;
#endif

		case WINDOW_TYPE_UPGRADE:
			{
				if ( game->inCombat && firstPerson[player] != NULL && commandWindows[ player ] == NULL )
				{
					if (this->upgradeWindow != NULL 
						&& this->upgradeWindow->isVisible() != 2) // is not already fading out
					{
						game->setPaused(false);
						if( save_changes )
							this->upgradeWindow->applyUpgrades();

						//this->closeUpgradeWindow(firstPerson[player]);
						this->upgradeWindow->fadeOut();
					}
					else
					{
						FB_ASSERT( false );
					}
				}
			}
			break;

#ifdef GUI_BUILD_LOG_WINDOW
		case WINDOW_TYPE_LOG:
			{
				if( game->inCombat && firstPerson[player] != NULL && commandWindows[ player ] == NULL )
				{
					if( this->logWindow != NULL )
					{
						game->setPaused( false );
						this->logWindow->hide();
					}
				}
			}
			break;
#endif

		default:
			break;
		}
#ifdef GUI_BUILD_INGAME_GUI_TABS
		updateIngameTabs();
#endif
	}

#ifdef GUI_BUILD_INGAME_GUI_TABS
	void GameUI::updateIngameTabs()
	{
		if( this->ingameGuiTabs )
		{
			bool visible = false;
#ifdef GUI_BUILD_MAP_WINDOW
			if( ( mapWindow && mapWindow->isVisible() == 1 ) ||
#else
			if (false ||
#endif
				( upgradeWindow && upgradeWindow->isVisible() == 1  ) ||
#ifdef GUI_BUILD_LOG_WINDOW
				( logWindow && logWindow->isVisible() == 1 ) )
#else
				false)
#endif
			{
				visible = true;
			}

			if( vehicleWindow )
			{
				visible = false;
			}

			if( ingameGuiTabs->isVisible() != visible )
			{
				if( visible )
					ingameGuiTabs->show();
				else ingameGuiTabs->hide();
			}

		}
	}
#endif

	bool GameUI::openUpgradeWindow(Unit *unit)
	{
		// TODO: split screen: solve player's cursor id first
		ogui->SetCursorImageState(0, DH_CURSOR_ARROW);

		// player is meaningless as long as it on in this client...
		// TODO, NETGAME: check that
		if ( ( upgradeWindow == NULL ) && 
#ifdef GUI_BUILD_MAP_WINDOW
			 ( mapWindow == NULL || mapWindow->isVisible() == false ) && 
#else
			 ( true ) && 
#endif
#ifdef GUI_BUILD_LOG_WINDOW
			 ( logWindow == NULL || logWindow->isVisible() == false ) &&
#else
			 ( true ) && 
#endif
			 ( terminalManager == NULL || terminalManager->isWindowOpen() == false ) )
		{
			game->setPaused( true );
			this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);
#ifdef PROJECT_SURVIVOR
			upgradeWindow = new SurvivorUpgradeWindow(ogui, game, unit);
#else
			upgradeWindow = new UpgradeWindow(ogui, game, unit);
#endif

			// don't fade twice
			if(!combatWindows[game->singlePlayerNumber]->isGUIModeTempInvisible())
				combatWindows[game->singlePlayerNumber]->startGUIModeTempInvisible(upgradeWindow->getFadeInTime());

			return true;
		} else {
			// already open
			#ifdef _DEBUG
				// assert( false && "this replaced a debug abort" );
				// abort();
			#endif

			return false;
		}
	}

	void GameUI::prepareCloseUpgradeWindow(Unit *unit)
	{
		if (upgradeWindow == NULL)
		{
			/*
			// not opened
			#ifdef _DEBUG
				abort();
			#endif
			*/
		} else {
			if( upgradeWindow->isVisible() )
			{
				if (combatWindows[game->singlePlayerNumber] != NULL)
				{
					combatWindows[game->singlePlayerNumber]->endGUIModeTempInvisible(upgradeWindow->getFadeOutTime());
				}	
				if (this->renderTerrain != NULL)
				{
					this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);
				}
			}

		} 	 
#ifdef GUI_BUILD_INGAME_GUI_TABS
		updateIngameTabs();
#endif
	}

	void GameUI::closeUpgradeWindow(Unit *unit)
	{
		if (upgradeWindow == NULL)
		{
			/*
			// not opened
			#ifdef _DEBUG
				abort();
			#endif
			*/
			FB_ASSERT( false );
		} else {
			if (combatWindows[game->singlePlayerNumber] != NULL)
			{
				//combatWindows[game->singlePlayerNumber]->endGUIModeTempInvisible();
			}
			delete upgradeWindow;
			upgradeWindow = NULL;
		} 	 
#ifdef GUI_BUILD_INGAME_GUI_TABS
		updateIngameTabs();
#endif

		if(LoadingWindow::showUpgradeWindowOnClose)
		{
			// find current player
			int player = -1;
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				if(getFirstPerson(c) == unit)
				{
					player = c;
					break;
				}
			}

			// open upgrade window for next player
			if(player >= 0 && player + 1 < MAX_PLAYERS_PER_CLIENT 
				&& SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + player + 1))
			{
				openWindow( WINDOW_TYPE_UPGRADE, player + 1 );
			}
			else
			{
				LoadingWindow::showUpgradeWindowOnClose = false;
			}
		}
	}

#ifdef GUI_BUILD_MAP_WINDOW
	bool GameUI::openMapWindow()
	{
		if(mapWindow)
		{
			if( ( upgradeWindow == NULL || upgradeWindow->isVisible() != 1 ) &&
				( mapWindow->isVisible() == false ) &&
#ifdef GUI_BUILD_LOG_WINDOW
				( logWindow == NULL || logWindow->isVisible() == false ) &&
#else
				true &&
#endif
				( terminalManager == NULL || terminalManager->isWindowOpen() == false ) )
			{
				// FB_ASSERT( false );
				if(vehicleWindow == NULL)
					combatWindows[game->singlePlayerNumber]->startGUIModeTempInvisible(mapWindow->getFadeInTime());

				game->setPaused(true);
				renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);

				ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
				mapWindow->show();
				return true;
			}
		}

		return false;
	}

	void GameUI::closeMapWindow()
	{
		if(mapWindow && mapWindow->isVisible())
		{
			if(vehicleWindow == NULL)
				combatWindows[game->singlePlayerNumber]->endGUIModeTempInvisible(mapWindow->getFadeOutTime());

			game->setPaused(false);
			renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);

			mapWindow->hide();
		}
#ifdef GUI_BUILD_INGAME_GUI_TABS
		updateIngameTabs();
#endif
	}
#endif

#ifdef GUI_BUILD_LOG_WINDOW
	bool GameUI::openLogWindow()
	{
		if ( logWindow == NULL &&
			( upgradeWindow == NULL || upgradeWindow->isVisible() != 1 ) && 
#ifdef GUI_BUILD_MAP_WINDOW
			( mapWindow == NULL || mapWindow->isVisible() == false ) && 
#else
			( true ) && 
#endif
			( terminalManager == NULL || terminalManager->isWindowOpen() == false ) )
		{
			// FB_ASSERT( false );
			game->setPaused( true );
			ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
			combatWindows[ game->singlePlayerNumber ]->startGUIModeTempInvisible( upgradeWindow->getFadeInTime() );
			this->renderTerrain->GetTerrain()->getRenderer().enableFeature( IStorm3D_TerrainRenderer::RenderTargets, false );
			logWindow = new LogWindow( *game, *ogui, *logManager );

			return true;
		} else {
			return false;
		}
	}
	
	void GameUI::prepareCloseLogWindow()
	{
		if ( logWindow == NULL )
		{
		} else {
			if ( combatWindows[game->singlePlayerNumber] != NULL )
			{
				combatWindows[game->singlePlayerNumber]->endGUIModeTempInvisible( upgradeWindow->getFadeOutTime() );
			}
			if ( this->renderTerrain != NULL )
			{
				this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);
			}

		}
		// ..\game\GameUI.cpp(1920) : error C3861: 'updateIngameTabs': identifier not found
//		updateIngameTabs();
	}

	void GameUI::closeLogWindow()
	{
		if ( logWindow == NULL )
		{
		} else {
			if ( combatWindows[ game->singlePlayerNumber ] != NULL )
			{
				// combatWindows[ game->singlePlayerNumber ]->endGUIModeTempInvisible();
			}
			delete logWindow;
			logWindow = NULL;
		} 
	}
#endif

	void GameUI::openTerminalWindow( const std::string& name )
	{
		
		// combatWindows[game->singlePlayerNumber]->startGUIModeTempInvisible(mapWindow->getFadeInTime());
		
		terminalManager->openTerminalWindow( name );

		if( terminalManager->isWindowOpen() )
		{
			game->setPaused(true);
			
			if( this->game->inCombat )
				renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);

			ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
		}
	}

	void GameUI::closeTerminalWindow()
	{
		// combatWindows[game->singlePlayerNumber]->endGUIModeTempInvisible(mapWindow->getFadeOutTime());

		game->setPaused(false);
		
		if( this->game->inCombat )
			renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);

		terminalManager->closeTerminalWindow();
	}

	void GameUI::openCinematicScreen( const std::string& name )
	{
		if( cinematicScreen )
			closeCinematicScreen();

		cinematicScreen = new CinematicScreen( ogui, game, name, getSoundMixer(), storm3d );

		if( this->game->inCombat && cinematicScreen->hasVideo() )
			renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false);

	}

	bool GameUI::isCinematicScreenOpen() const
	{
		return ( cinematicScreen != NULL && cinematicScreen->isOpen() );
	}

	void GameUI::closeCinematicScreen()
	{
		/*
		if( this->game->inCombat )
			renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);
		*/

		if( cinematicScreen )
		{
			if( this->game->inCombat && cinematicScreen->hasVideo() )
				renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true);

			delete cinematicScreen;
			cinematicScreen = NULL;
		}
	}


	void GameUI::openCombatWindow(int player, bool invisible)
	{
		// close quitbox if open...
		if (quitBox[player] != NULL)
		{
			delete quitBox[player];
			quitBox[player] = NULL;
			// TODO: what if some other messagebox is open?
			msgBoxIsOpen = false;
		}
		// close incomplete message box if still open (should never happen?)
		if (armorIncompleteConfirmWindows[player] != NULL)
		{
			delete armorIncompleteConfirmWindows[player];
			armorIncompleteConfirmWindows[player] = NULL;
		}

		if (combatWindows[player] == NULL)
		{
			combatWindows[player] = 
				new CombatWindow(ogui, game, player);
			combatWindows[player]->show();
#ifndef PROJECT_SHADOWGROUNDS
			if(invisible) combatWindows[player]->startGUIModeTempInvisible();
#endif
		} else {
			combatWindows[player]->show();
#ifndef PROJECT_SHADOWGROUNDS
			if(invisible) combatWindows[player]->startGUIModeTempInvisible();
#endif
			// already open
			//#ifdef _DEBUG
			//	abort();
			//#endif
		}
#ifdef PROJECT_SURVIVOR
		if(vehicleWindow != NULL)
		{
			vehicleWindow->setCombatWindowVisibility();
		}
#endif
	}

	void GameUI::closeCombatWindow(int player)
	{
		if (combatWindows[player] == NULL)
		{
			// already closed
			//#ifdef _DEBUG
			//	abort();
			//endif
		} else {
			delete combatWindows[player];
			combatWindows[player] = NULL;
		}
	}

	CombatWindow* GameUI::getCombatWindow( int player ) const
	{
		return combatWindows[ player ];
	}

	/*MenuCollection* GameUI::getCommandWindow( int player ) const
	{
		return commandWindows[ player ];
	}*/

	void GameUI::openArmorIncompleteConfirm(int player, bool notAnyArmor, bool incompleteArmor, bool noArmor, bool notPaid)
	{
		if (armorIncompleteConfirmWindows[player] != NULL)
		{
			Logger::getInstance()->debug("GameUI::openArmorIncompleteConfirm - Already open."); 
		} else {
			if (notAnyArmor)
			{
				const char *choices[1] = 
				{ 
					getLocaleGuiString("gui_back") 
				};
				const char *imgs0[3] = { "Data/GUI/Buttons/incomplete_back.tga", "Data/GUI/Buttons/incomplete_back_down.tga", "Data/GUI/Buttons/incomplete_back_highlight.tga" };
				const char **images[1];
				images[0] = imgs0;
				armorIncompleteConfirmWindows[player] = new MessageBoxWindow(ogui, 
					"All of your armors are incomplete or have unpurchased parts. You need to have at least one complete armor to continue.",
					1, choices, images, 512, 256, 150, 50, "Data/GUI/Windows/incomplete_confirm.dds",
					MSGBOX_NOARMORS, this);
				msgBoxIsOpen = true;
				ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
			} else {
				//char *choices[2] = { "Back", "Continue" };
				const char *choices[2] = 
				{ 
					getLocaleGuiString("gui_back"),
					getLocaleGuiString("gui_continue")
				};

				const char *imgs0[3] = { "Data/Buttons/incomplete_back.tga", "Data/Buttons/incomplete_back_down.tga", "Data/Buttons/incomplete_back_highlight.tga" };
				const char *imgs1[3] = { "Data/Buttons/incomplete_continue.tga", "Data/Buttons/incomplete_continue_down.tga", "Data/Buttons/incomplete_continue_highlight.tga" };
				const char **images[2];
				images[0] = imgs0;
				images[1] = imgs1;
				const char *text = 0;
				if (incompleteArmor)
				{
					text = "Some of the armors are incomplete. They will not take part in combat.";
				} else {
					if (noArmor)
					{
						text = "Some of your mercenaries do not have armors. They will not take part in combat.";
					} else {
						text = "Some of the armors have parts that have not been paid for. Unpaid parts will be removed. Any armors that will become incomplete will not take part in combat.";
					}
				}
				armorIncompleteConfirmWindows[player] = new MessageBoxWindow(ogui, 
					text,
					2, choices, images, 512, 256, 150, 50, "Data/GUI/Windows/incomplete_confirm.dds",
					MSGBOX_INCOMPLETEARMORS, this);
				msgBoxIsOpen = true;
				ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
			}
		}
	}

	void GameUI::messageBoxClosed(MessageBoxWindow *msgbox, int id, int choice)
	{
		if (id == MSGBOX_ABORTMISSION
			|| id == MSGBOX_QUITGAME)
		{
			for (int i = 0; i < ABS_MAX_PLAYERS; i++)
			{
				if (msgbox == quitBox[i])
				{
					quitBox[i] = NULL;
					break;
				}
			}
			if (id == MSGBOX_ABORTMISSION)
			{
				if (choice == 1) abortMission = true;
				if (choice == 2) quitRequested = true;
			}
			if (id == MSGBOX_QUITGAME)
			{
				if (choice == 0) quitRequested = true;
			}
		}
		if (id == MSGBOX_INCOMPLETEARMORS || id == MSGBOX_NOARMORS)
		{
			int i = 0;
			for(; i < ABS_MAX_PLAYERS; i++)
			{
				if (msgbox == armorIncompleteConfirmWindows[i])
				{
					armorIncompleteConfirmWindows[i] = NULL;
					if (id == MSGBOX_INCOMPLETEARMORS && choice == 1)
					{
						if (commandWindows[i] != NULL)
						{ 
							openLoadingWindow(game->singlePlayerNumber); 
							commandWindows[i]->hide();
						}
					}
					break;
				}
			}
			if (i == ABS_MAX_PLAYERS)
			{
				Logger::getInstance()->warning("GameUI::messageBoxClosed - Called from unknown message box.");
			}
		}

		// a nasty way to kill the object that called this method.
		msgBoxIsOpen = false;
		delete msgbox;
	}

	// TEMP HACK!
	/*
	bool GameUI::readyToRockNRoll()
	{
		if ((armorConstructWindows[game->singlePlayerNumber] == NULL || !armorConstructWindows[game->singlePlayerNumber]->isVisible())
			&& (commandWindows[game->singlePlayerNumber] == NULL || !commandWindows[game->singlePlayerNumber]->isVisible())
			&& (storageWindows[game->singlePlayerNumber] == NULL || !storageWindows[game->singlePlayerNumber]->isVisible()))
			return true;
		else 
			return false;
	}
	*/

	void GameUI::setRenderMap(GameMap *gameMap, char *configFile)
	{
		if (lightningVisualEffect != NULL)
		{
			lightningVisualEffect->setDeleteFlag();
			lightningVisualEffect->freeReference();
			lightningVisualEffect = NULL;
		}
		if (renderTerrain != NULL)
		{
			//scene->RemoveTerrain(renderTerrain);

			lipsyncManager.reset(0);

			game_anyBurnableTrackableObjectFactory->removeImplementation(renderTerrain);
			visualEffectManager->freeDecalEffects();
			visualEffectManager->setTerrain(NULL);

			assert(dynamicLightManager != NULL);
			delete dynamicLightManager;
			dynamicLightManager = NULL;

			assert(lightManager != NULL);
			delete lightManager;
			lightManager = NULL;

			delete renderTerrain;
			renderTerrain = NULL;
		}

		if (this->gridOcclusionCuller != NULL)
		{
			delete this->gridOcclusionCuller;
			this->gridOcclusionCuller = NULL;
		}

		renderMap = gameMap;
		if (gameMap != NULL)
		{
			terrainCreator->setCameraRange(cameraRange);
			renderTerrain = terrainCreator->createTerrain(gameMap, lightManager, ambientSoundManager, configFile); 
			game_anyBurnableTrackableObjectFactory->addImplementation(renderTerrain);
			scene->AddTerrain(renderTerrain->GetTerrain());
			visualEffectManager->setTerrain(renderTerrain->GetTerrain());
			visualEffectManager->loadDecalEffects();

			assert(lightManager == NULL);
			lightManager = new ui::LightManager(*storm3d, *scene, *renderTerrain->GetTerrain(), gameMap->lightMap, renderTerrain);
			lightManager->setMaxLightAmount(SimpleOptions::getInt(DH_OPT_I_RENDER_MAX_POINTLIGHTS));
			renderTerrain->setLightManager(lightManager);
			dynamicLightManager = new ui::DynamicLightManager(lightManager);
			lipsyncManager.reset(new util::LipsyncManager(storm3d, renderTerrain->GetTerrain()));

			//scene->AddTerrain(renderTerrain);
			OptionApplier::applyDisplayOptions(storm3d, scene, renderTerrain->GetTerrain(), lightManager);
			OptionApplier::applyCameraOptions(scene);
		}
		if (cursorRayTracer != NULL)
		{
			delete cursorRayTracer;
			cursorRayTracer = NULL;
		}
		if (renderTerrain != NULL)
		{
			cursorRayTracer = new CursorRayTracer(storm3d, scene, 
				renderTerrain->GetTerrain(), 1024, 768);
		}
		if (gameMap != NULL)
		{
			this->gridOcclusionCuller = new util::GridOcclusionCuller(gameMap->getScaledSizeX(), gameMap->getScaledSizeY(), (gameMap->getObstacleSizeX() >> GRIDOCCLUSIONCULLER_OBSTACLE_DIV_SHIFT), (gameMap->getObstacleSizeY() >> GRIDOCCLUSIONCULLER_OBSTACLE_DIV_SHIFT));
		}
	}

	void GameUI::runUI(int currentTime)
	{
		if (game->getPendingLoad() != NULL)
		{
			if (commandWindows[game->singlePlayerNumber] != NULL)
			{
				openLoadingWindow(game->singlePlayerNumber); 
				closeCommandWindow(game->singlePlayerNumber);
			}
		}
		
		if( loadingWindow != NULL )
		{
			// for scrolling text and video
			loadingWindow->update();
		}

#ifdef PROJECT_SURVIVOR
		if( characterSelectionWindow != NULL )
		{
			characterSelectionWindow->update();
		}
#endif


		musicPlaylist->run();

		textureCache->update(currentTime - lastRunUITime);

		if (this->lipsyncManager)
		{
			this->lipsyncManager->update();
		}

		game->getGamePhysics()->renderedScene();

		if (game->inCombat)
		{
			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_TRACKERS))
			{
				DebugTrackerVisualizer::visualizeTrackers(game->objectTracker);
			}
			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_PROJECTILES))
			{
				VC3 campos = gameCamera->getActualInterpolatedPosition();
				DebugProjectileVisualizer::visualizeProjectiles(game->projectiles, campos);
			}
			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_UNITS))
			{
				VC3 campos = gameCamera->getActualInterpolatedPosition();
				DebugUnitVisualizer::visualizeUnits(game->units, campos);
			}
			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_OBSTACLEMAP))
			{
				VC3 campos = gameCamera->getActualInterpolatedPosition();
				DebugMapVisualizer::visualizeObstacleMap(game->gameMap, game->getGameScene(), campos);
			}

			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_SELECTIONS))
			{
				SelectionVisualizer::visualizeSelections(game);
			}

		}

		// listener position...
		{
			VC3 pos = gameCamera->getActualInterpolatedPosition();

			listenerPosition = pos;
			if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
				if (this->gameCamera == this->cameras[GAMEUI_CAMERA_NORMAL])
				{
					if (isThirdPersonView(game->singlePlayerNumber))
					{
						if (firstPerson[0] != NULL && !game->isCooperative())
						{
							listenerPosition = firstPerson[0]->getPosition();
						}
					}
				}
			}
		}

		// occlusion culling
		if (SimpleOptions::getBool(DH_OPT_B_OCCLUSION_CULLING_ENABLED))
		{
			if (game->inCombat
				&& gridOcclusionCuller != NULL)
			{
				VC3 camPos = getOcclusionCheckPosition();
				int cameraArea = GRIDOCCLUSIONCULLER_DEFAULT_AREA_MASK;
				if (gridOcclusionCuller->isWellInScaledBoundaries(camPos.x, camPos.z))
				{
					int occx = gridOcclusionCuller->scaledToOcclusionX(camPos.x);
					int occy = gridOcclusionCuller->scaledToOcclusionY(camPos.z);
					cameraArea = gridOcclusionCuller->getCameraArea(occx, occy);
				} else {
					// warn every once a while...
					if ((game->gameTimer & 63) == 0)
					{
						Logger::getInstance()->debug("GameUI::runUI - Camera outside occlusion culling area (using default area).");
					}
				}
				// HACK: static
				// note, 0 is ok for default init, as this is the bit mask, so 0 is "invalid area"
				static int lastRunUIOcclusionArea = 0;
				if (lastRunUIOcclusionArea != cameraArea)
				{
					lastRunUIOcclusionArea = cameraArea;

					// TODO: this may cause a minor framerate glitch... (although tests suggest only ~1 ms or so)
					// should instead update the visibilities a few objects (~50-100) at a time, per game tick)
					// should just start the update process here.
					game->gameUI->getTerrain()->updateOcclusionForAllObjects(gridOcclusionCuller, cameraArea);
				}
			}
		}

		if (soundMixer != NULL)
		{
			soundLooper->run();
			soundMixer->runMixer(currentTime);

			VC3 pos = listenerPosition;
			VC3 playerPos = listenerPosition;

			//VC3 pos = gameCamera->getPosition();
			soundMixer->setPosition(playerPos.x, playerPos.y, playerPos.z);

#ifdef PROJECT_CLAW_PROTO
			pos = gameCamera->getPosition();
			
			float angle = gameCamera->getAngleY();
			float dirX = cosf(angle / 180*3.1415f);
			float dirZ = sinf(angle / 180*3.1415f);
			// gameCamera->getAngleY();
			// gameCamera->getBetaAngle()

			// For some reason at certain places getAngleY() jumps 90 degrees on each frame.
			// For the time being just calculate direction from target.

			VC3 dir = gameCamera->getTargetPosition() - pos;
			dir.Normalize();
			// FIXME: _real_ beta angle rotation!
			//VC3 dir = VC3(dirX, -0.7f, dirZ);
			//dir.Normalize();

			/*
			{
				VC3 pos = getListenerPosition();
				std::string msg = "Camera listener position: ";
				msg += boost::lexical_cast<std::string> (pos.x);
				msg += ", ";
				msg += boost::lexical_cast<std::string> (pos.y);
				msg += ", ";
				msg += boost::lexical_cast<std::string> (pos.z);
				Logger::getInstance()->error(msg.c_str());
			}

			{
				VC3 pos = gameCamera->getPosition();
				std::string msg = "Camera position: ";
				msg += boost::lexical_cast<std::string> (pos.x);
				msg += ", ";
				msg += boost::lexical_cast<std::string> (pos.y);
				msg += ", ";
				msg += boost::lexical_cast<std::string> (pos.z);
				Logger::getInstance()->error(msg.c_str());
			}

			{
				std::string msg = "Camera direction: ";
				msg += boost::lexical_cast<std::string> (dir.x);
				msg += ", ";
				msg += boost::lexical_cast<std::string> (dir.y);
				msg += ", ";
				msg += boost::lexical_cast<std::string> (dir.z);
				Logger::getInstance()->error(msg.c_str());
			}
			*/
#else
			float angle = gameCamera->getAngleY();
			float dirX = cosf(angle / 180*3.1415f);
			float dirZ = sinf(angle / 180*3.1415f);
			
			// FIXME: _real_ beta angle rotation!
			VC3 dir = VC3(dirX, -2.0f, dirZ);
			dir.Normalize();

			// TEMP:
			// HACK: testing result when camera moved a lot upward (backward)...
			// (10 meters backward, well.. actually at fixed angle until beta angle rotation fixed.)
			pos -= dir * 10.0f;
#endif

			soundMixer->setListenerPosition(pos.x, pos.y, pos.z, 0, 0, 0, dir.x, dir.y, dir.z,
				0, 1.0f, 0);
		}

		{
			static int last_container_check = 0;
			static int last_container_hint = 0;
			bool found_container = false;

			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{
				gameController[c]->run();

				if (SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + c))
				{
					if (this->console == NULL || !this->console->isVisible())
					{
						// WARNING: assuming linear run script keybinds / option numbers!!!
						for (int ctrl = 0; ctrl < RUN_SCRIPT_CTRL_AMOUNT; ctrl++)
						{
							if ( gameController[c]->wasKeyClicked(ctrl + DH_CTRL_RUN_SCRIPT_1) )
							{
								game->gameScripting->setGlobalIntVariableValue("script_runner_client", c);
								char *scriptname = SimpleOptions::getString(ctrl + DH_OPT_S_CONTROLLER_SCRIPT_1);
								if (scriptname != NULL
									&& scriptname[0] != '\0')
								{
									game->gameScripting->runMissionScript(scriptname, "main");
								}
#ifdef LEGACY_FILES
								// play it safe. do nothing, break nothing.
#else
								// set the global int to value of -1 to indicate that we're outside a control script
								game->gameScripting->setGlobalIntVariableValue("script_runner_client", -1);
#endif
							}
						}
					}

					if ( gameController[c]->wasKeyClicked(DH_CTRL_OPEN_ANIRECORDER) )
					{
						if (game->inCombat)
						{
							if (this->aniRecorderWindow == NULL)
							{
								this->openAniRecorderWindow(0);
							} else {
								this->closeAniRecorderWindow(0);
							}
						}
					}

					if( gameController[c]->wasKeyClicked(DH_CTRL_OPEN_UPGRADE))
					{
#ifdef PROJECT_SURVIVOR
						// each player has their own upgrade window
						const int player = c;
						const int command_window_player = 0; // only first player has command windows..
#else
						const int player = 0;
						const int command_window_player = 0;
#endif
						if ( game->inCombat
							&& firstPerson[player] != NULL
							&& firstPerson[player]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
							&& commandWindows[ command_window_player ] == NULL )
						{
							if (this->upgradeWindow == NULL)
							{
								openWindow( WINDOW_TYPE_UPGRADE, player );
							}
							else if( this->upgradeWindow->isVisible() ) 
							{
								closeWindow( WINDOW_TYPE_UPGRADE, true, player );
							}
						}
					}

#ifdef GUI_BUILD_MAP_WINDOW
					if( gameController[c]->wasKeyClicked(DH_CTRL_OPEN_MAP)
						  && firstPerson[c] != NULL
							&& firstPerson[c]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS )
					{
						const int player = 0;
						if(game->inCombat && firstPerson[player] && mapWindow && commandWindows[ player ] == NULL )
						{
#ifdef PROJECT_SURVIVOR
							// in survival mode
							int value = 0;
							if(util::Script::getGlobalIntVariableValue("survival_mode_enabled", &value) && value == 1)
							{
								// open upgrade window instead of map
								if (this->upgradeWindow == NULL)
									openWindow( WINDOW_TYPE_UPGRADE, player );
								else if( this->upgradeWindow->isVisible() ) 
									closeWindow( WINDOW_TYPE_UPGRADE, true, player );
							}
							else
#endif
							if(mapWindow->isVisible())
								closeWindow( WINDOW_TYPE_MAP );
							else
								openWindow( WINDOW_TYPE_MAP );
						}
					}
#endif


#ifdef GUI_BUILD_LOG_WINDOW
					if( gameController[c]->wasKeyClicked(DH_CTRL_OPEN_LOG)
						  && firstPerson[c] != NULL
							&& firstPerson[c]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					{
						const int player = 0;
						if( game->inCombat && firstPerson[player] != NULL && commandWindows[ player ] == NULL )
						{
							if( this->logWindow == NULL )
							{
								openWindow( WINDOW_TYPE_LOG );
							}
							else
							{
								closeWindow( WINDOW_TYPE_LOG );
							}
						}
					}
#endif

#ifdef PROJECT_SURVIVOR
					if(renderTerrain && firstPerson[c] && firstPerson[c]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					{
						if(game->gameTimer > last_container_check)
						{
							// get nearest container
							UnifiedHandle uh = renderTerrain->findClosestContainer(firstPerson[c]->getPosition(), 1.0f);
							if(uh != UNIFIED_HANDLE_NONE)
							{
								found_container = true;

								// show hint message
								if(Timer::getTime() - last_container_hint > 5500)
								{
									combatWindows[game->singlePlayerNumber]->showHintMessage(getLocaleGuiString("gui_container_hint"));
									last_container_hint = Timer::getTime();
								}

								// pressing execute
								if( gameController[c]->wasKeyClicked(DH_CTRL_EXECUTE) )
								{
									// break container
									std::vector<TerrainObstacle> removedObjects;
									std::vector<ExplosionEvent> objectEvents;
									renderTerrain->BreakTerrainObject(uh, removedObjects, objectEvents, 1000, false );

									ProjectileActor::handleTerrainBreaking(game, removedObjects, objectEvents);
								}
							}
						}
					}
#endif

				}
			}

			if(game->gameTimer > last_container_check)
			{
				if(found_container)
				{
					// check again next tick
					last_container_check = game->gameTimer;
				}
				else
				{
					// check again in 0.1 secs
					last_container_check = game->gameTimer + GAME_TICKS_PER_SECOND/10;
					last_container_hint = 0;
				}
			}
		}

		if (keyreaderId != -1)
		{
			if (!console->isVisible())
			{
				// console closed elsewhere?... well then make sure
				// no more keyreader.
				console->cancel();
				if (keyreaderId != -1)
				{
					gameController[0]->removeKeyreader(keyreaderId);
					keyreaderId = -1;
				}
			}
		}

		if (wasKeyClicked(DH_CTRL_CONSOLE_TOGGLE))
		{
			assert(console != NULL);
			if (console != NULL)
			{
				if (console->isVisible())
				{
					hideConsole();
				} else {
					showConsole();
				}
			}
		}

		// Quit pressed, at cinematic (but not while still in load screen)
		{

			static bool quit_released = false;

			/*
			for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
			{*/
			if ((wasKeyClicked(DH_CTRL_QUIT) || Keyb3_IsKeyPressed(KEYCODE_MOUSE_BUTTON1) || Keyb3_IsKeyPressed(KEYCODE_MOUSE_BUTTON2) || Keyb3_IsKeyPressed(KEYCODE_MOUSE_BUTTON3)) && cinematicScreen != NULL )
				{
					cinematicScreen->close();
					quit_released = false;
				}

#ifdef PROJECT_SURVIVOR
				if (wasKeyClicked(DH_CTRL_QUIT) && missionFailureWindow != NULL )
				{
					closeMissionFailureWindow();
				}
#endif

				if (wasKeyClicked(DH_CTRL_QUIT)
					&& game->isCinematicScriptRunning()
					&& loadingWindow == NULL)
				{
					game->skipCinematicScript();
				}

#ifdef PROJECT_SURVIVOR
				// character selection window
				if(characterSelectionWindow != NULL)
				{
					// loop through all players
					bool changed = false;
					for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++ )
					{
						if(!SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + i)) continue;

						bool pressed_button1 = false;
						if(gameController[ i ]->getControllerType() == GameController::CONTROLLER_TYPE_JOYSTICK1)
							pressed_button1 = gameController[ i ]->isKeyDownByKeyCode( KEYCODE_JOY_BUTTON1 );
						else if(gameController[ i ]->getControllerType() == GameController::CONTROLLER_TYPE_JOYSTICK2)
							pressed_button1 = gameController[ i ]->isKeyDownByKeyCode( KEYCODE_JOY2_BUTTON1 );
						else if(gameController[ i ]->getControllerType() == GameController::CONTROLLER_TYPE_JOYSTICK3)
							pressed_button1 = gameController[ i ]->isKeyDownByKeyCode( KEYCODE_JOY3_BUTTON1 );
						else if(gameController[ i ]->getControllerType() == GameController::CONTROLLER_TYPE_JOYSTICK4)
							pressed_button1 = gameController[ i ]->isKeyDownByKeyCode( KEYCODE_JOY4_BUTTON1 );

						bool controlsEnabled = gameController[ i ]->getUserControlsEnabled();
						if(!controlsEnabled) gameController[ i ]->setControlsEnabled(true);
						if(gameController[ i ]->wasKeyClicked( DH_CTRL_CAMERA_MOVE_FORWARD ))
						{
							characterSelectionWindow->chooseCharacterInDir(0, i);
							changed = true;
						}
						else if(gameController[ i ]->wasKeyClicked( DH_CTRL_CAMERA_MOVE_LEFT ))
						{
							characterSelectionWindow->chooseCharacterInDir(-1, i);
							changed = true;
						}
						else if(gameController[ i ]->wasKeyClicked( DH_CTRL_CAMERA_MOVE_RIGHT ))
						{
							characterSelectionWindow->chooseCharacterInDir(1, i);
							changed = true;
						}
						else if(gameController[ i ]->wasKeyClicked( DH_CTRL_ATTACK ) || pressed_button1)
						{
							// don't trigger lock through mouse button fire (instead let the cursor click on the icon)
							if(gameController[ i ]->getBoundKey(DH_CTRL_ATTACK, 0) != KEYCODE_MOUSE_BUTTON1
								 && gameController[ i ]->getBoundKey(DH_CTRL_ATTACK, 0) != KEYCODE_MOUSE0_BUTTON1
								 && gameController[ i ]->getBoundKey(DH_CTRL_ATTACK, 0) != KEYCODE_MOUSE1_BUTTON1
								 && gameController[ i ]->getBoundKey(DH_CTRL_ATTACK, 0) != KEYCODE_MOUSE2_BUTTON1
								 && gameController[ i ]->getBoundKey(DH_CTRL_ATTACK, 0) != KEYCODE_MOUSE3_BUTTON1)
							{
								characterSelectionWindow->lockChosenCharacter(i);
								changed = true;
							}
						}
						if(!controlsEnabled) gameController[ i ]->setControlsEnabled(false);
					}

					// update buttons
					if(changed)
					{
						characterSelectionWindow->updateProfileButtons();
					}

					// safe to close now
					if(characterSelectionWindow->shouldClose())
					{
						closeCharacterSelectionWindow();
					}
				}
#endif

				bool customMenuKeyPressed = false;
				if( game->isCooperative() && (
					(SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED ) && game->getGameUI()->getController(0)->wasKeyClicked( DH_CTRL_OPEN_MENU ) ) ||
				   (SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + 1 ) && game->getGameUI()->getController(1)->wasKeyClicked( DH_CTRL_OPEN_MENU ) ) ||
				   (SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + 2 ) && game->getGameUI()->getController(2)->wasKeyClicked( DH_CTRL_OPEN_MENU ) ) ||
				   (SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + 3 ) && game->getGameUI()->getController(3)->wasKeyClicked( DH_CTRL_OPEN_MENU ) )
					) )
				{
					customMenuKeyPressed = true;
				}

#ifdef PROJECT_SURVIVOR
				// failsafe: esc is always bound to quit
				{
					static bool holding_esc = false;
					if( game->getGameUI()->getController(0)->isKeyDownByKeyCode(1))
					{
						if(!holding_esc)
						{
							customMenuKeyPressed = true;
							holding_esc = true;
						}
					}
					else
					{
						holding_esc = false;
					}
				}
#endif

				// Quit pressed, not at cinematic
				if ((( ( wasKeyClicked(DH_CTRL_QUIT) || customMenuKeyPressed )
					&& (!game->isCinematicScriptRunning() || loadingWindow != NULL || characterSelectionWindow != NULL))
					|| (commandWindows[game->singlePlayerNumber] != NULL 
					&& commandWindows[game->singlePlayerNumber]->wasQuitPressed())))
				{
					// not already at some message box
					if (!msgBoxIsOpen)
					{
						// if in armor construction/storage/etc. menus back to main menu
						// else quit confirm message box.
						// these should actually be handled inside the menu windows themselves.
						if ((armorConstructWindows[game->singlePlayerNumber] != NULL && armorConstructWindows[game->singlePlayerNumber]->isVisible())
							|| (storageWindows[game->singlePlayerNumber] != NULL && storageWindows[game->singlePlayerNumber]->isVisible()))
						{
							if (armorConstructWindows[game->singlePlayerNumber] != NULL && armorConstructWindows[game->singlePlayerNumber]->isVisible())
							{
								if (armorConstructWindows[game->singlePlayerNumber]->isPartSelectionVisible())
								{
									armorConstructWindows[game->singlePlayerNumber]->cancelPartSelection();
								} else {
									closeArmorConstructWindow(game->singlePlayerNumber);
									openCommandWindow(game->singlePlayerNumber);
									ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
								}
							} else {
								if (storageWindows[game->singlePlayerNumber] != NULL && storageWindows[game->singlePlayerNumber]->isVisible())
								{
									closeStorageWindow(game->singlePlayerNumber);
									openCommandWindow(game->singlePlayerNumber);
									ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
								}
							}
						} else {
							if (game->inCombat)
							{
							/*	const char *choices[3] = { "", "", "" };
								const char *imgs0[3] = { "Data/GUI/Buttons/abort_cancel.tga", "Data/GUI/Buttons/abort_cancel_down.tga", "Data/GUI/Buttons/abort_cancel_highlight.tga" };
								const char *imgs1[3] = { "Data/GUI/Buttons/abort_abort.tga", "Data/GUI/Buttons/abort_abort_down.tga", "Data/GUI/Buttons/abort_abort_highlight.tga" };
								const char *imgs2[3] = { "Data/GUI/Buttons/abort_quit.tga", "Data/GUI/Buttons/abort_quit_down.tga", "Data/GUI/Buttons/abort_quit_highlight.tga" };
								const char **images[3];
								images[0] = imgs0;
								images[1] = imgs1;
								images[2] = imgs2;
								//const char *text = "Abort mission or quit game?";
								const char *text = getLocaleGuiString("gui_abort_abort_mission_or_quit");
								quitBox[game->singlePlayerNumber] = new MessageBoxWindow(ogui, text,
									3, choices, images, 512, 92, 160, 27, "Data/GUI/Windows/abort_confirm.tga",
									MSGBOX_ABORTMISSION, this);
								quitBox[game->singlePlayerNumber]->setFadeoutTime(200);
								msgBoxIsOpen = true;
								ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
								*/
								// msgBoxIsOpen = true;
								// game->setPaused( true );

								// added the esc quits the upgrade and map windows
								// by Pete
								
								if( upgradeWindow ) 
								{
									closeWindow( WINDOW_TYPE_UPGRADE );
									/*closeUpgradeWindow( firstPerson[0] );*/
									/*game->setPaused(false);
									this->upgradeWindow->applyUpgrades();
								
									this->upgradeWindow->fadeOut();*/
								} 
#ifdef GUI_BUILD_MAP_WINDOW
								else if( mapWindow && mapWindow->isVisible() )
								{
									closeWindow( WINDOW_TYPE_MAP );
									// closeMapWindow();
								}
#endif
#ifdef GUI_BUILD_LOG_WINDOW
								else if( logWindow != NULL )
								{
									/*game->setPaused( false );
									logWindow->hide();
									*/
									closeWindow( WINDOW_TYPE_LOG );
								}
#endif
								else if( terminalManager != NULL && terminalManager->isWindowOpen() )
								{
									closeTerminalWindow();
								}
#ifdef PROJECT_SURVIVOR
								else if(characterSelectionWindow != NULL)
								{
									characterSelectionWindow->forceStart();
								}
#endif
								else if( loadingWindow != NULL )
								{
									if(cinematicScreen == NULL)
									{
										// quit was not held down in last frame
										// (prevents skipping cinematic and loading intro with one push)
										if(quit_released)
										{
											loadingWindow->closeWindow();
										}
									}									
								}
								else if( missionSelectionWindow != NULL )
								{
									closeMissionSelectionWindow();
								}
#ifdef PROJECT_SURVIVOR
								else if( scoreWindow != NULL )
								{
									scoreWindow->PleaseClose();
								}
#endif
								else if( loadingWindow == NULL )
								{
									int player = 0;

									if( commandWindows[player] == NULL ) 
										openMainmenuFromGame();
									else commandWindows[player]->escPressed(); 
								} 


								
								// upgradeWindow == NULL && !mapWindow->isVisible()
								
							} else {

								/*
								const char *choices[2] = { "", "" };
								const char *imgs0[3] = { "Data/GUI/Buttons/quit_yes.tga", "Data/GUI/Buttons/quit_yes_down.tga", "Data/GUI/Buttons/quit_yes_highlight.tga" };
								const char *imgs1[3] = { "Data/GUI/Buttons/quit_no.tga", "Data/GUI/Buttons/quit_no_down.tga", "Data/GUI/Buttons/quit_no_highlight.tga" };
								const char **images[2];
								images[0] = imgs0;
								images[1] = imgs1;
								//const char *text = "Quit game?";
								const char *text = getLocaleGuiString("gui_abort_quit_game");
								quitBox[game->singlePlayerNumber] = new MessageBoxWindow(ogui, text,
									2, choices, images, 256, 92, 96, 27, "Data/GUI/Windows/quit_confirm.tga",
									MSGBOX_QUITGAME, this);
								quitBox[game->singlePlayerNumber]->setFadeoutTime(200);
								msgBoxIsOpen = true;
								ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
								*/
							}
						}
					} else {
						// some message box already open... maybe it was the quit
						// box? if so, raise it to top (it may be under some other
						// window)
						if (quitBox[game->singlePlayerNumber] != NULL)
						{
							quitBox[game->singlePlayerNumber]->raise();
						}
					}
				}

				if(wasKeyClicked(DH_CTRL_QUIT))
				{
					quit_released = false;
				}
				else
				{
					quit_released = true;
				}


				// gui toggles...
				// radar
				if (wasKeyClicked(DH_CTRL_GUI_RADAR_TOGGLE))
				{
					if (combatWindows[game->singlePlayerNumber] != NULL)
					{
						combatWindows[game->singlePlayerNumber]->toggleRadarMode();
					}
				}
				// units
				if (wasKeyClicked(DH_CTRL_GUI_UNITS_TOGGLE))
				{
					if (combatWindows[game->singlePlayerNumber] != NULL)
					{
						combatWindows[game->singlePlayerNumber]->toggleGUIMode();
					}
				}
			/*}*/
		}

		// stuff to do while in combat (in mission, not menus)
		if (game->inCombat)
		{ 		

// TEST HACK
/*
if (SimpleOptions::getInt(DH_OPT_I_CAMERA_CULLING_RATE) > 1)
{
	static int cullTick = 0;

	int cullrate = SimpleOptions::getInt(DH_OPT_I_CAMERA_CULLING_RATE);

	cullTick++;
	if (cullTick >= cullrate)
		cullTick = 0;

	bool cullOn = false;
	if (cullTick == 0)
	{
		cullOn = true;
	}
	IStorm3D_TerrainRenderer &r = game->gameScene->getTerrain()->getRenderer();
	r.enableFeature(IStorm3D_TerrainRenderer::FreezeCameraCulling, !cullOn);
}
*/

			// raytrace to scene and terrain, what's under cursor
			if (cursorRayTracer != NULL)
			{
				// psd
				buildingHandler.setCollisions(false);

				for(int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
				{
					if(!SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + c))
						break;

					if (gameController[c]->isKeyDown(DH_CTRL_CAMERA_LOOK_MODE)
						|| gameController[c]->isKeyDown(DH_CTRL_CAMERA_POSITION_OFFSET_MODE)
						|| gameController[c]->isKeyDown(DH_CTRL_CAMERA_TARGET_OFFSET_MODE)) 
					{
						ogui->skipCursorMovement();
					}
				}

				LinkedList restoreColl; 

				// disable hits to units (if specific unit hitting option is not on)
				if (!SimpleOptions::getBool(DH_OPT_B_GUI_CURSOR_RAYTRACE_HITS_UNITS))
				{
					LinkedList *ulist = game->units->getAllUnits();
					LinkedListIterator iter = LinkedListIterator(ulist);
					while (iter.iterateAvailable())
					{
						Unit *u = (Unit *)iter.iterateNext();
						if (u->isActive())
						{
							if (u->getVisualObject() != NULL)
							{
								u->getVisualObject()->setCollidable(false);
								restoreColl.append(u);
							}
						}
					}

					/*
					// unnecessary, done above.
					// disable hits to first person (if one)
					for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
					{
						if (firstPerson[i] != NULL)
						{
							if (firstPerson[i]->getVisualObject() != NULL)
							{
								firstPerson[i]->getVisualObject()->setCollidable(false);
								restoreColl.append(firstPerson[i]);
							}
						}
					}
					*/
				}

				// cursor raytrace.
				for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
				{
					*(sceneSelection[i]) = cursorRayTracePlayer(i, false, false);
				}

				if (firstPerson[0] != NULL && gameCamera->isFirstPersonMode()
					&& gameCamera->isThirdPersonView() && !game->isPaused()
					&& !msgBoxIsOpen)
				{
					for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
					{
						// HACK: keep players inside screen...
						if (firstPerson[c] != NULL
							&& SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
						{
							IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
							IStorm3D_Camera *cam = scene->GetCamera();

							VC3 pos = firstPerson[c]->getPosition();
							VC3 result = VC3(0,0,0);
							float rhw = 0;
							float real_z = 0;
							bool infront = cam->GetTransformedToScreen(pos, result, rhw, real_z);
							if (infront)
							{
								int scrx = (int)(result.x * 1024.0f);
								int scry = (int)(result.y * 768.0f);

								clientUnitScreenPos[c].x = (float)scrx;
								clientUnitScreenPos[c].y = (float)scry;

								if (scrx < 256)
									leftMovementEnabled[c] = false;
								else
									leftMovementEnabled[c] = true;
								if (scrx > 1024-256)
									rightMovementEnabled[c] = false;
								else
									rightMovementEnabled[c] = true;
								if (scry < 128)
									upMovementEnabled[c] = false;
								else
									upMovementEnabled[c] = true;
								if (scry > 768-64)
									downMovementEnabled[c] = false;
								else
									downMovementEnabled[c] = true;
							} 						
						} else {
							leftMovementEnabled[c] = true;
							rightMovementEnabled[c] = true;
							upMovementEnabled[c] = true;
							downMovementEnabled[c] = true;
						}
					}
				}

				if (firstPerson[0] != NULL && (gameCamera->isFirstPersonMode() || SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS))
					&& gameCamera->isThirdPersonView() && !game->isPaused()
					&& !msgBoxIsOpen)
				{
					// handle joystick/keyboard-only controllers
					for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
					{
						if (SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + c))
						{
							if(SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c))
							{
								// switched from joystick to mouse
								if (joystickAimer[c] != NULL)
								{
									delete joystickAimer[c];
									joystickAimer[c] = NULL;
								}							
							}
							else if(!SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c))
							{
								if (joystickAimer[c] == NULL)
								{
									joystickAimer[c] = new JoystickAimer(firstPerson[c], ogui, game, gameController[c], c);
								}
								joystickAimer[c]->aimWithJoystick(currentTime - lastRunUITime);
							}
						}
					}
				}

				// restore destroyed units collidable
				while (!restoreColl.isEmpty())
				{
					Unit *u = (Unit *)restoreColl.popLast();
					u->getVisualObject()->setCollidable(true);
				}

#ifdef PROJECT_SURVIVOR
				if( vehicleWindow != NULL)
				{
					vehicleWindow->update();
				}
#endif


				// update windows and stuff...
				if (combatWindows[game->singlePlayerNumber] != NULL)
				{
					combatWindows[game->singlePlayerNumber]->update(currentTime - lastRunUITime);

					// don't update cursors in character selection window
					if(characterSelectionWindow == NULL)
					{
						combatWindows[game->singlePlayerNumber]->updateCursorImage();
					}
					combatWindows[game->singlePlayerNumber]->doCombatControls(currentTime - lastRunUITime);

					combatWindows[game->singlePlayerNumber]->updatePointers();

					const Unit *highu = sceneSelection[0]->unit;
					if (highu != NULL && !highu->visibility.isSeenByPlayer(game->singlePlayerNumber))
						highu = NULL;
					combatWindows[game->singlePlayerNumber]->setUnitHighlight(highu);
					combatWindows[game->singlePlayerNumber]->updateUnitHighlight();

					// TODO: update selection box only if mouse button down...
					SelectionBox *sbox = combatWindows[game->singlePlayerNumber]->getSelectionBox();
					sbox->selectionPositionUpdate(ogui->getCursorScreenX(0), 
						ogui->getCursorScreenY(0));

					if (currentTime > meterUpdateTime + 50)
					{
						meterUpdateTime = currentTime;

						// update heat and energy bars
						combatWindows[game->singlePlayerNumber]->updateMeters();

						combatWindows[game->singlePlayerNumber]->updateModeInfo();

					}

					
					// update radar
					{
						VC3 pos;
						if (firstPerson[0] != NULL)
							pos = firstPerson[0]->getPosition();
						else
							pos = gameCamera->getPosition();
						combatWindows[game->singlePlayerNumber]->updateRadar(pos.x, pos.z);
					}

					combatWindows[game->singlePlayerNumber]->setRadarAngle(gameCamera->getAngleY());

					// update offscreen unit pointers
					combatWindows[game->singlePlayerNumber]->updateOffscreenUnitPointers();

					if (pointersChanged)
					{
						pointersChanged = false;
						combatWindows[game->singlePlayerNumber]->recreatePointers();
					}
					if (unitsDestroyed)
					{
						unitsDestroyed = false;
						combatWindows[game->singlePlayerNumber]->recreateUnitSelections();
						combatWindows[game->singlePlayerNumber]->recreatePointers();
					}
					if (unitsDamaged)
					{
						unitsDamaged = false;
						combatWindows[game->singlePlayerNumber]->updateHPMeters();
					}
				}

				// psd
				buildingHandler.setCollisions(true);
			}

			// prepare units, projectiles and game pointers for render

			float interpolate_factor = 1.0f;

			if(Timer::getTimeFactor() != 1.0f)
			{
				interpolate_factor = (Timer::getTime() - game->lastTickTime) / (1000.0f / (float) GAME_TICKS_PER_SECOND);
			}

			LinkedList *ulist = game->units->getAllUnits();
			LinkedListIterator unititer = LinkedListIterator(ulist);
			while (unititer.iterateAvailable())
			{
				Unit *unit = (Unit *)unititer.iterateNext();
				VisualObject *vo = unit->getVisualObject();
				if (vo != NULL)
				{
					// Shadow decals
					if (unit->hasActed() )
					{
						VC3 shadowPosition = vo->getRenderPosition();//unit->getPosition();
						UnitType *type = unit->getUnitType();

						VC2 shadowSize(type->getShadowSizeX(), type->getShadowSizeZ()); //unit->getFakeShadowSize();
						float shadowStrength = type->getShadowStrength();

						// Fade by bone position
						VC3 bonePosition;
						if(vo && vo->getStormModel())
							bonePosition = vo->getStormModel()->GetApproximatedPosition();

						if(shadowStrength > 0.05f && shadowSize.x > 0.1f && shadowSize.y > 0.1f)
						{
							float floor = game->gameMap->getScaledHeightAt(shadowPosition.x, shadowPosition.z);
							static const float maxHeightDifference = 2.f;

							float delta = fabsf(shadowPosition.y - floor);
							if(delta < maxHeightDifference)
							{
								float alpha = 1.f - (delta / maxHeightDifference);
								alpha *= shadowStrength;
								shadowPosition.y = floor;

								float sizeFactor = 1.f + (1.75f * (delta / maxHeightDifference));
								shadowSize *= sizeFactor;

								float yAngle = UNIT_ANGLE_TO_RAD(vo->getRenderYAngle());
								QUAT rotation;
								VC3 terrainNormal;
								DecalPositionCalculator::calculateDecalRotation(game->gameScene, shadowPosition, rotation, yAngle, terrainNormal);

								shadowPosition.y += (1.f - terrainNormal.y) * 0.3f;
								float normalFadeFactor = terrainNormal.y;
								alpha *= normalFadeFactor;
								if(alpha < 0)
									alpha = 0.f;

								alpha *= 1.f - lightManager->getFakelightFactor(shadowPosition);

								float sqRange = bonePosition.GetSquareRangeTo(shadowPosition);
								if(sqRange > 4.f)
									alpha = 0.f;
								else if(sqRange > 1.f)
								{
									// Fade 1-2m
									float range = sqrtf(sqRange);
									alpha *= 1.f - (range - 1.f);
								}

								if(vo->getVisibilityFactor() < 0.01f)
									alpha = 0.f;

								IStorm3D_Terrain *terrain = renderTerrain->GetTerrain();
								if(terrain)
								{

									if(!unit->getVisualObject()->isVisible())
										alpha = 0.0f;

									if (alpha < 0) alpha = 0;
									if (alpha >= 1.0f) alpha = 1.0f;
									if(alpha > 0.005f)
										terrain->getDecalSystem().setShadowDecal(shadowPosition, rotation, shadowSize, alpha);
								}
							}
						}
					}

					// Directional light
					if (unit->hasActed())
					{
						const VC3 &position = unit->getPosition();
						const VC3 &sunDir = renderTerrain->getSunDirection();
						int ox = game->gameMap->scaledToObstacleX(position.x);
						int oy = game->gameMap->scaledToObstacleY(position.z);

						util::DirectionalLight &directionalLight = unit->getDirectionalLight();
						if (game->gameMap->isWellInScaledBoundaries(position.x, position.z))
						{
							if(game->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
								directionalLight.setSun(VC3(), 0.f);
							else
								directionalLight.setSun(sunDir, 1.f);
						} else {
							directionalLight.setSun(sunDir, 1.f);
						}

						VC3 dir;
						float strength = 1.f;

						if(!game->isPaused())
							directionalLight.update(currentTime - lastRunUITime);

						directionalLight.getResult(dir, strength);

						vo->getStormModel()->SetDirectional(dir, strength);
						vo->getStormModel()->useAlwaysDirectional(true);
					}

					// hack test
					if( Unit::getVisualizationOffsetInUse() )
					{
						VC3 position;
						position = unit->getPosition();
					
						// should write my own boundry check
						if( game->gameMap->isWellInScaledBoundaries( position.x, position.z ) )
						{
							const int x = game->gameMap->scaledToObstacleX( position.x ) / 2 * 2;
							const int y = game->gameMap->scaledToObstacleY( position.z ) / 2 * 2;

							// <new implementation>
							// int material = getMaterialByPalette( game->gameMap->getAreaMap()->getAreaValue( x, y, AREAMASK_MATERIAL ) >> AREASHIFT_MATERIAL );

							const float v1 = (float)((getMaterialByPalette( game->gameMap->getAreaMap()->getAreaValue( x - 1, y - 1, AREAMASK_MATERIAL ) >> AREASHIFT_MATERIAL ) == MATERIAL_SAND)?1:0);
							const float v2 = (float)((getMaterialByPalette( game->gameMap->getAreaMap()->getAreaValue( x + 1, y - 1, AREAMASK_MATERIAL ) >> AREASHIFT_MATERIAL ) == MATERIAL_SAND)?1:0);
							const float v3 = (float)((getMaterialByPalette( game->gameMap->getAreaMap()->getAreaValue( x - 1, y + 1, AREAMASK_MATERIAL ) >> AREASHIFT_MATERIAL ) == MATERIAL_SAND)?1:0);
							const float v4 = (float)((getMaterialByPalette( game->gameMap->getAreaMap()->getAreaValue( x + 1, y + 1, AREAMASK_MATERIAL ) >> AREASHIFT_MATERIAL ) == MATERIAL_SAND)?1:0);
							// </new implementation>
			
							// <old implementation>
							/*
							const float v1 = (float)(game->gameMap->getAreaMap()->isAreaAnyValue( x - 1, y - 1, AREAMASK_INBUILDING )?0:1);
							const float v2 = (float)(game->gameMap->getAreaMap()->isAreaAnyValue( x + 1, y - 1, AREAMASK_INBUILDING )?0:1);
							const float v3 = (float)(game->gameMap->getAreaMap()->isAreaAnyValue( x - 1, y + 1, AREAMASK_INBUILDING )?0:1);
							const float v4 = (float)(game->gameMap->getAreaMap()->isAreaAnyValue( x + 1, y + 1, AREAMASK_INBUILDING )?0:1);
							*/
							// </old implementation>

							//float fx = ( ( position.x + game->gameMap->scaledSizeHalvedX ) * game->gameMap->scaledToPathfindMultiplierX ) - (float)x;
							//float fy = ( ( position.z + game->gameMap->scaledSizeHalvedY ) * game->gameMap->scaledToPathfindMultiplierY ) - (float)y;
							float fx = game->gameMap->getPositionOffsetFactorInsideObstacleX(position.x);
							float fy = game->gameMap->getPositionOffsetFactorInsideObstacleY(position.z);
							fx /= 2.0f;
							fy /= 2.0f;

							const float s1 = v1 * ( 1.0f - fx ) + (fx) * v2;
							const float s2 = v3 * ( 1.0f - fx ) + (fx) * v4;
							
							const float value = s1 * (1.0f - fy ) + fy * s2;
							unit->setVisualizationOffsetInterpolation( value );
						}

					}

#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto
if (unit->isPhysicsObjectLock() && !unit->isDestroyed()
		&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
{
	VC3 frompos = unit->getPosition();
	VC3 topos = firstPerson[0]->getPosition();
	float angleY = util::PositionDirectionCalculator::calculateDirection(frompos, topos);
	unit->setRotation(0, angleY, 0);
}
#endif
					vo->prepareForRender(interpolate_factor);

					// lighting and lighting fading...
					if (unit->hasActed())
					{
						VC3 movevec = unit->getPosition();
						movevec -= unit->getLastLightUpdatePosition();
						// NOTE: magic number here (10cm)
						if (movevec.GetSquareLength() > 0.1f * 0.1f)
						{
							VC3 tmppos = unit->getPosition();
							unit->setLastLightUpdatePosition(tmppos);

							if(game->gameMap->colorMap)
							{
								GameMap *gameMap = game->gameMap;

								// HACK: if door execute unit (door type unit),
								// don't update color changes, use spawn position
								// colormap always
								VC3 position;
								if (unit->getUnitType()->hasDoorExecute())
								{
									position = unit->getSpawnCoordinates();
									position.y = unit->getPosition().y;
								} else {
									position = unit->getPosition();
								}

								position.x = position.x / gameMap->getScaledSizeX() + .5f;
								position.z = position.z / gameMap->getScaledSizeY() + .5f;

								COL color = gameMap->colorMap->getColor(position.x, position.z);
								// HACK: players get a minimum illum.
								if (playerSelfIllumEnabled 
									&& (unit == firstPerson[0]
									|| unit == firstPerson[1]
									|| unit == firstPerson[2]
									|| unit == firstPerson[3]))
								{
									if (color.r < 0.1f) color.r = 0.1f;
									if (color.g < 0.1f) color.g = 0.1f;
									if (color.b < 0.1f) color.b = 0.1f;
								} else {
									// HACK: units that go way below/above player, get darker...
									// currently 2-7 meters (is that good for now?)
									if (firstPerson[0] != NULL)
									{
										VC3 fpPos = firstPerson[0]->getPosition();
										float depthDiff = (float)fabs(position.y - fpPos.y);
										if (depthDiff > 2.0f)
										{
											float depthFactor = (depthDiff - 2.0f) / 5.0f;
											if (depthFactor > 1.0f) depthFactor = 1.0f;
											depthFactor = 1.0f - depthFactor;

											color.r *= depthFactor;
											color.g *= depthFactor;
											color.b *= depthFactor;
										}
									}
								}

								//VC3 lightPosition;
								//COL ambient = color;
								//COL lightColor;
								//float lightRange = 5.f;
								ui::PointLights lights;
								lights.ambient = color;

								int ox = game->gameMap->scaledToObstacleX(position.x);
								int oy = game->gameMap->scaledToObstacleY(position.z);
								if (game->gameMap->isWellInScaledBoundaries(position.x, position.z))
								{
									if (vo->isSkyModel())
									{
										lights.ambient = COL(0,0,0);
									} else {
										if(game->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
											lightManager->getLighting(unit->getPosition(), lights, getRadius(vo->getStormModel()), true, true, vo->getStormModel());
										else
											lightManager->getLighting(unit->getPosition(), lights, getRadius(vo->getStormModel()), true, false, vo->getStormModel());
									}
								}

								// apply lighting fade effect
								// apply only when changed since last call
								float lightFadeFactor = unit->getCurrentLightingFadeValue();
								if (lightFadeFactor != vo->getLightingFactor())
								{
									/*
									for(int i = 0; i < 2; ++i)
									{
										lights.lights[i].color.r *= lightFadeFactor;
										lights.lights[i].color.g *= lightFadeFactor;
										lights.lights[i].color.b *= lightFadeFactor;
									}
									*/

									lights.ambient.r *= lightFadeFactor;
									lights.ambient.g *= lightFadeFactor;
									lights.ambient.b *= lightFadeFactor;
								}

								for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
									vo->getStormModel()->SetLighting(i, lights.lightIndices[i]);
								vo->setSelfIllumination(lights.ambient);

								// apply visibility fade effect
								float visFadeFactor = unit->getCurrentVisibilityFadeValue();

								if (visFadeFactor != vo->getVisibilityFactor())
								{
									vo->setVisibilityFactor(visFadeFactor);
								}
							}
						}
					}


// HACK TEST:
//VC3 reflectPos = unit->getPosition();
//reflectPos.z = 88.000f - 0.03f;
//vo->setCAReflect(true, reflectPos, VC3(1, -1, 1));
//vo->setCAMotionBlur(true, 1.0f);

					// unit's flashlight...
					Flashlight *fl = unit->getFlashlight();
					if (fl != NULL)
					{
						VC3 position = unit->getPosition();
						VC3 rot = unit->getRotation();

						float angle = UNIT_ANGLE_TO_RAD(rot.y + unit->getFlashlightDirection() + unit->getUnitType()->getAimBoneDirection());

						//fl->setRotationToward(angle, currentTime - lastRunUITime);
						// HACK: NO MORE FLASHLIGHT LAG...
						fl->setRotation(angle);

						// DOES THIS WORK???
						float betaAngle = 0;
						if (!game->isCinematicScriptRunning()) {
							if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD)) {
							//	betaAngle = UNIT_ANGLE_TO_RAD(rot.x) + aimOffset*0.01f+0.2f;
							} else {
								// new: up/down rotation for flashlight only for keyb+mouse players
								for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
								{
									if (unit == firstPerson[c]
										&& SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c))
									{
										float curx = float(ogui->getCursorScreenX(c));
										float cury = float(ogui->getCursorScreenY(c));

										// note, these are at screen scale (0-1024)
										float u_scr_posx = SCREEN_CENTER_X;
										float u_scr_posy = SCREEN_CENTER_Y;

										// HACK:
										// if coop, use player screen position, else just use the screen center
										// this check must be here, as the unit screen position are not calculated
										// for players if single player.
										if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
										{
											u_scr_posx = clientUnitScreenPos[c].x;
											u_scr_posy = clientUnitScreenPos[c].y;
										}

										betaAngle = PI*0.15f-sqrtf((curx-u_scr_posx)*(curx-u_scr_posx)+(cury-u_scr_posy)*(cury-u_scr_posy))*0.002f;
										if (betaAngle<-0.2) betaAngle = -0.2f;
									}
								}
							}
						}

						fl->setBetaRotation(betaAngle);

						if (unit->getJumpCounter() > 0
							&& unit->getUnitType()->doesRollJump())
						{
							float jumpNormTime = (float)(unit->getJumpTotalTime() - unit->getJumpCounter()) / (float)unit->getJumpTotalTime();

							bool forward, backward, left, right;
							forward = backward = left = right = false;

							unit->getUnitRelativeJumpDirections(&forward, &backward, &left, &right);

#ifdef PROJECT_SURVIVOR
							if(jumpNormTime > 0.125f)
							{
								jumpNormTime = (1.0f - jumpNormTime) / 7.0f;
								if(jumpNormTime < 0)
									jumpNormTime = 0;
							}
#endif

							// some interesting formulas for getting the flashlight
							// go out when roll jumping...
							if (left)
							{
								if (backward)
								{
									fl->setBetaRotation(betaAngle - 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f));
								} else {
									fl->setBetaRotation(betaAngle + 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f));
								}
								fl->setRotation(angle - 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f));
							} 
							else if (right)
							{
								if (backward)
								{
									fl->setBetaRotation(betaAngle - 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f));
								} else {
									fl->setBetaRotation(betaAngle + 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f));
								}
								fl->setRotation(angle + 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f));
							}
							else if (backward)
							{
								fl->setBetaRotation(betaAngle - 3.1415927f * 2.0f * jumpNormTime);
								fl->setRotation(angle - 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f * 2.0f));
							}
							else
							{
								fl->setBetaRotation(betaAngle + 3.1415927f * 2.0f * jumpNormTime);
								fl->setRotation(angle + 0.3f * (float)sinf(jumpNormTime * 3.1415927f * 2.0f * 2.0f));
							}

							if (left || right)
								fl->setTemporaryBrightnessFactor(1.0f - (float)sqrtf((float)sinf(jumpNormTime * 3.1415927f)));
							else
								fl->setTemporaryBrightnessFactor(1.0f - (float)sinf(jumpNormTime * 3.1415927f));

						} else {
							fl->setTemporaryBrightnessFactor(1.0f);
						}

						fl->prepareForRender();
					}

					// and halo...
					ui::Spotlight *halo = unit->getSecondarySpotlight();
					if (halo != NULL)
					{
						// halo changes based on colormap brightness
						VC3 colposition = unit->getPosition();
						colposition.x = colposition.x / game->gameMap->getScaledSizeX() + .5f;
						colposition.z = colposition.z / game->gameMap->getScaledSizeY() + .5f;
						COL color = game->gameMap->colorMap->getColor(colposition.x, colposition.z);
						float haloColormapMultiplier = 1.0f - (color.r + color.g + color.b) / 3.0f;
						if (haloColormapMultiplier < 0.0f)
							haloColormapMultiplier = 0.0f;

						// HACK: change halo color if default halo type only...
						if (unit->getUnitType()->getHaloType() == NULL)
						{
							// halo intensity/color changes a bit based on flashlight
							// and muzzleflash
							if (fl != NULL
								&& fl->isFlashlightOn())
							{
								if (unit->isMuzzleflashVisible())
								{
									//halo->setFakelightParams(20, COL(haloColormapMultiplier * 0.25f, haloColormapMultiplier * 0.25f, haloColormapMultiplier * 0.2f));
									halo->setFakelightParams(14, COL(haloColormapMultiplier * 0.20f, haloColormapMultiplier * 0.20f, haloColormapMultiplier * 0.15f));
								} else {
									//halo->setFakelightParams(10.0f + 10.0f * fl->getFlashlightIlluminationFactor(), COL(haloColormapMultiplier * 0.2f, haloColormapMultiplier * 0.2f, haloColormapMultiplier * 0.2f));
									halo->setFakelightParams(8.0f + 6.0f * fl->getFlashlightIlluminationFactor(), COL(haloColormapMultiplier * 0.15f, haloColormapMultiplier * 0.15f, haloColormapMultiplier * 0.15f));
								}
							} else {
								if (unit->isMuzzleflashVisible())
								{
									halo->setFakelightParams(14, COL(haloColormapMultiplier * 0.20f, haloColormapMultiplier * 0.20f, haloColormapMultiplier * 0.15f));
								} else {
									halo->setFakelightParams(8, COL(haloColormapMultiplier * 0.15f, haloColormapMultiplier * 0.15f, haloColormapMultiplier * 0.15f));
								}
							}
						}
						VC3 position = unit->getPosition();
						halo->setPosition(position);

						halo->prepareForRender();
					}


				}
			}

			LinkedList *blist = game->buildings->getAllBuildings();
			LinkedListIterator biter = LinkedListIterator(blist);
			while (biter.iterateAvailable())
			{
				Building *building = (Building *)biter.iterateNext();
				VisualObject *vo = building->getVisualObject();
				if (vo != NULL)
				{
					vo->prepareForRender();
				}
			}

			// items
			game->itemManager->prepareForRender();

			VC3 lightUpdPos = gameCamera->getActualInterpolatedPosition();
			game->gameMap->keepWellInScaledBoundaries(&lightUpdPos.x, &lightUpdPos.z);
			lightUpdPos.y = game->gameMap->getScaledHeightAt(lightUpdPos.x, lightUpdPos.z);
			VC3 playerPos = lightUpdPos;

			if (gameCamera == cameras[GAMEUI_CAMERA_NORMAL])
			{
				if (firstPerson[0] != NULL)
				{
					lightUpdPos = firstPerson[0]->getPosition();
					playerPos = lightUpdPos;
					if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
					{
						VC3 rotation = firstPerson[0]->getRotation();
						VC3 dir = VC3(0,0,0);
						dir.x = -sinf(UNIT_ANGLE_TO_RAD(rotation.y));
						dir.z = -cosf(UNIT_ANGLE_TO_RAD(rotation.y));
						lightUpdPos.x += dir.x * 6.0f;
						lightUpdPos.z += dir.z * 6.0f;
					}
				}
			}

			int lightUpdTime = 0;
			if (!game->isPaused())
				lightUpdTime = currentTime - lastRunUITime;
			lightManager->update(playerPos, lightUpdPos, lightUpdTime);

			// particles and projectiles' visual effects
			visualEffectManager->prepareForRender(game->gameMap, game->gameMap->colorMap, lightManager);

			if (combatWindows[game->singlePlayerNumber] != NULL)
				combatWindows[game->singlePlayerNumber]->renderPointers();

			// pause and tactical modes...
			if ((wasKeyClicked(DH_CTRL_PAUSE)
				|| wasKeyClicked(DH_CTRL_TACTICAL_MODE))
				&& !game->isMissionAboutToEnd()
				&& !this->isAnyIngameWindowVisible()) 
			{
				if (wasKeyClicked(DH_CTRL_PAUSE))
				{
					if (game->isPaused()) 
					{
						this->setScrollyTemporarilyDisabled(false);
						game->setPaused(false);
						gameMessage("", NULL, 3, 100, MESSAGE_TYPE_HINT);
					} else {
						this->setScrollyTemporarilyDisabled(true);
						game->setPaused(true);
						std::string paused_text = getLocaleGuiString("gui_paused");
						gameMessage(paused_text.c_str(), NULL, 2, 10000, MESSAGE_TYPE_HINT);
					}
				}
				if (wasKeyClicked(DH_CTRL_TACTICAL_MODE)) 
				{
					if (game->isTacticalMode()) 
					{
						//selectCamera(GAMEUI_CAMERA_NORMAL);
						//gameCamera->interpolateFrom(cameras[GAMEUI_CAMERA_TACTICAL]);
						game->setTacticalMode(false);
						combatWindows[game->singlePlayerNumber]->setTacticalModeButton(false);
						pointersChanged = true;
						if (firstPerson[0] != NULL)
						{
							firstPerson[0]->setDirectControl(true);
							firstPerson[0]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER);
							combatWindows[game->singlePlayerNumber]->setCrosshair(true);
							if (!thirdPersonView)
							{
								if (firstPerson[0]->getVisualObject() != NULL)
									firstPerson[0]->getVisualObject()->setVisible(false);
							}
						}
					} else {

						game->setTacticalMode(true);
						combatWindows[game->singlePlayerNumber]->setTacticalModeButton(true);
						pointersChanged = true;
						if (firstPerson[0] != NULL)
						{
							firstPerson[0]->setDirectControl(false);
							if (firstPerson[0]->getUnitType()->isDirectControl())
								firstPerson[0]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_AI);
							else
								firstPerson[0]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_NONE);
							combatWindows[game->singlePlayerNumber]->setCrosshair(false);
							if (firstPerson[0]->getVisualObject() != NULL)
								firstPerson[0]->getVisualObject()->setVisible(true);
						}
					}
				}
			}

			if (wasKeyClicked(DH_CTRL_CONTROL_MODE_SWITCH)) 
			{
				if (controlModeDirect)
					controlModeDirect = false;
				else
					controlModeDirect = true;
			}

			// first person
			if (wasKeyClicked(DH_CTRL_UNIT_MODE_TOGGLE))
//				&& !game->isPaused()) 
			{
				if (firstPerson[0] != NULL)
				{
					if (!game->isTacticalMode())
					{
						firstPerson[0]->setDirectControl(false);
						if (firstPerson[0]->getUnitType()->isDirectControl())
							firstPerson[0]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_AI);
						else
							firstPerson[0]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_NONE);
						if (firstPerson[0]->getVisualObject() != NULL)
							firstPerson[0]->getVisualObject()->setVisible(true);
						combatWindows[game->singlePlayerNumber]->setCrosshair(false);
					}
					cameras[GAMEUI_CAMERA_NORMAL]->setFirstPersonMode(false);

					firstPerson[0]->targeting.clearTarget();

					firstPerson[0] = NULL;

				} else {
					LinkedList *ulist = game->units->getOwnedUnits(game->singlePlayerNumber);
					LinkedListIterator iter = LinkedListIterator(ulist);
					while (iter.iterateAvailable())
					{
						Unit *u = (Unit *)iter.iterateNext();
						if (u->isActive() && !u->isDestroyed())
						{
							if (u->isSelected())
							{
								setFirstPerson(game->singlePlayerNumber, u, 0);
								break;
							}
						}
					}
				}
			}

			if (firstPerson[0] != NULL && !game->isTacticalMode()
				&& gameCamera->isFirstPersonMode())
			{
				gameCamera->resetForScene();

				if (wasKeyClicked(DH_CTRL_CAMERA_FIRST_PERSON_TOGGLE))
				{
					if (thirdPersonView)
					{
						thirdPersonView = false;
						cameras[GAMEUI_CAMERA_NORMAL]->setThirdPersonView(false);
					} else {
						thirdPersonView = true;
						cameras[GAMEUI_CAMERA_NORMAL]->setThirdPersonView(true);
						//combatWindows[game->singlePlayerNumber]->setCrosshair(false);
					}
					combatWindows[game->singlePlayerNumber]->setCrosshair(true);
				}
				VC3 pos = firstPerson[0]->getPosition();
				VC3 rot = firstPerson[0]->getRotation();
				
				float lookAngle = firstPerson[0]->getLookBetaAngle();
				int mdx = 0;
				int mdy = 0;
				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
				{
					if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
					{
						if (
							(combatWindows[game->singlePlayerNumber]->isGUIVisible() || forceCursorVisible)
							&& !game->isPaused())
						{
							// removing this breaks turning with mouse
							gameController[0]->getMouseDelta(&mdx, &mdy);

							// removing this removes cursor from screen
							if (!this->isAnyIngameWindowVisible() && !msgBoxIsOpen)
							{
								ogui->setCursorScreenX(0, AIM_UP_CURSOR_X);
								ogui->setCursorScreenY(0, AIM_UP_CURSOR_Y);
							}

							// turning with mouse works even with this removed
							combatWindows[game->singlePlayerNumber]->setCrosshairProperties(
								AIM_UP_CURSOR_X, AIM_UP_CURSOR_Y, firstPerson[0]->getFiringSpreadFactor());
						}
					} else {
						if (thirdPersonView)
						{
							combatWindows[game->singlePlayerNumber]->setCrosshairProperties(
								ogui->getCursorScreenX(0), ogui->getCursorScreenY(0), firstPerson[0]->getFiringSpreadFactor());

							// TEMP
							// mouse works even with this removed
							gameController[0]->getMouseDelta(&mdx, &mdy);

							for(int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
							{
								if(!SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + c))
									break;

								// TEMP
								if (gameController[c]->isKeyDown(DH_CTRL_CAMERA_LOOK_MODE)
									|| gameController[c]->isKeyDown(DH_CTRL_CAMERA_POSITION_OFFSET_MODE)
									|| gameController[c]->isKeyDown(DH_CTRL_CAMERA_TARGET_OFFSET_MODE)) 
								{
									ogui->SetCursorImageState(0, DH_CURSOR_INVISIBLE);
									ogui->SetCursorImageState(1, DH_CURSOR_INVISIBLE);
									ogui->SetCursorImageState(2, DH_CURSOR_INVISIBLE);
									ogui->SetCursorImageState(3, DH_CURSOR_INVISIBLE);
									combatWindows[game->singlePlayerNumber]->setCrosshair(false);
								} else {
									//ogui->SetCursorImageState(0, DH_CURSOR_AIM_SPREAD1);
									combatWindows[game->singlePlayerNumber]->setCrosshair(true);
								}
							}


						} else {
							// mouse works even with this removed
							gameController[0]->getMouseDelta(&mdx, &mdy);
						}
					}
				} else {
					gameController[0]->getMouseDelta(&mdx, &mdy);
				}

				// what the heck was this??? --jpk
				/*
				int ticks = (currentTime - lastRunUITime)/GAME_TICK_MSEC;
				if (ticks==0) ticks = 1;
				switch(ticks){
				case 1:
					rot.y += mdx / 5.0f + CAMERA_EXTRA_ROTATION*3;
					break;
				case 2:
					rot.y += mdx / 5.0f + CAMERA_EXTRA_ROTATION*2;
					break;
				default:
					rot.y += mdx / 5.0f + CAMERA_EXTRA_ROTATION;
				}
				*/
				if(SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD)
					&& firstPerson[0]->getUnitType()->hasMechControls())
				{
					// when using the mech, the angle does not depend
					// on the unit's rotation but instead the camera's
					rot.y = 270.0f - gameCamera->getAngleY();
				}

				float lookrotx = lookAngle;

				if (gameController[0]->controllerTypeHasMouse())
				{
					rot.y += mdx / 5.0f;

					while (rot.y >= 360) rot.y -= 360;
					while (rot.y < 0) rot.y += 360;
					lookrotx = lookAngle + mdy / 5.0f;
					if (lookAngle > 180)
					{
						if (lookrotx < 360-85) lookrotx = 360-85;
						if (lookrotx > 360+85) lookrotx = 360-85;
					} else {
						if (lookrotx < -85) lookrotx = -85;
						if (lookrotx > 85) lookrotx = 85;
					}
					while (lookrotx >= 360) lookrotx -= 360;
					while (lookrotx < 0) lookrotx += 360; 

					// Aiming offset moves flashlight and camera along the vertical mouse motion
					oldAimOffset = aimOffset;
					this->aimOffset += mdy*0.4f;
					if (this->aimOffset > MAX_AIM_OFFSET) aimOffset = MAX_AIM_OFFSET;
					if (this->aimOffset < MIN_AIM_OFFSET) aimOffset = MIN_AIM_OFFSET;
	//				lookAngle += aimOffset*0.2;

				} else {
					// joystick mode

                    // this is used when free camera mode is off

					int x = 0, y = 0;
					gameController[0]->getJoystickValues(NULL, NULL, &x, &y);

                    if (x != 0 || y != 0)
					{
						VC2 dir((float)x, (float)y);
						dir.Normalize();
						if (x > 0)
						{
							//  0 < angle < 180
							rot.y = ((M_PI * 0.5f) + asinf(dir.y)) * 180.0f / M_PI;
						} else {
                            // 180 < angle < 360
							rot.y = (2.0f * M_PI + -(M_PI * 0.5f + asinf(dir.y))) * 180.0f / M_PI;
						}
					}
					// LOG_DEBUG(strPrintf("direction: %d %d rot.y %f lookrotx %f", x, y, rot.y, lookrotx).c_str());

					// TODO: do something to lookrotx?
				}

				VC3 vel2 = firstPerson[0]->getVelocity();
				if (vel2.GetSquareLength() > positionOffset.GetSquareLength())
				{
					positionOffset = positionOffset*0.95f+vel2*0.5f;
				} else {
					positionOffset = positionOffset*0.98f+vel2*0.2f;
				}

				// this seems like some weird hack... assuming it is only meant for upward...
				// this actually turns the player
				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
				{
					if (!firstPerson[0]->isDestroyed()
						&& firstPerson[0]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
						&& !firstPerson[0]->isAnimated()
						&& !firstPerson[0]->getUnitType()->hasMechControls())
					{
						firstPerson[0]->setRotation(rot.x, rot.y, rot.z);
					}
				}

#ifdef PROJECT_SURVIVOR
				///////////////////////////////////////////////
				// custom per-weapon camera settings

				/*int players = 1;
				if (SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
					players++;
				if (SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
					players++;
				if (SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED))
					players++;
				float default_zoom = SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM) + (float)players * SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_ZOOM_PLAYER_INC);*/

				float default_zoom = CameraAutozoomer::getZoom(gameCamera);

				float default_angle = (float)game::SimpleOptions::getInt(DH_OPT_I_CAMERA_DEFAULT_BETA_ANGLE);
			
				static int last_started_custom = 0;
				static int last_used_custom = 0;
				bool using_custom = false;
				for(int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
				{
					if(!firstPerson[c])
						continue;

					int weapNum = firstPerson[c]->getSelectedWeapon();
					if(weapNum == -1)
						continue;

					Weapon *w = firstPerson[c]->getWeaponType(weapNum);
					if(!w)
						continue;

					if(w->usesCustomCamera())
					{
						int delta = Timer::getTime() - last_started_custom;
						if(last_started_custom == 0)
						{
							delta = 0;
							last_started_custom = Timer::getTime();
						}

						float lerp_factor = delta / 2000.0f;
						if(lerp_factor > 1)
							lerp_factor = 1;

						float current_angle = gameCamera->getBetaAngle();
						float current_zoom = gameCamera->getZoom();
						
						float new_angle = default_angle + w->getCustomCameraAngle();
						float new_zoom = default_zoom + w->getCustomCameraZoom();

						gameCamera->setBetaAngle(new_angle * lerp_factor + current_angle * (1.0f - lerp_factor));
						gameCamera->setZoom(new_zoom * lerp_factor + current_zoom * (1.0f - lerp_factor));
						using_custom = true;
						last_used_custom = Timer::getTime();
						break;
					}
				}

				if(!using_custom && last_used_custom != 0)
				{
					int delta = Timer::getTime() - last_used_custom;

					float lerp_factor = delta / 2000.0f;
					if(lerp_factor > 1)
					{
						lerp_factor = 1;
						last_started_custom = 0;
						last_used_custom = 0;
					}

					float current_angle = gameCamera->getBetaAngle();
					float current_zoom = gameCamera->getZoom();

					gameCamera->setBetaAngle(current_angle * (1.0f - lerp_factor) + default_angle * lerp_factor);
					gameCamera->setZoom(current_zoom * (1.0f - lerp_factor) + default_zoom * lerp_factor);
					last_started_custom = 0;
				}

				// end of custom per-weapon camera settings
				///////////////////////////////////////////////
#endif

				float frametime = float(currentTime - lastRunUITime);
				// Rotate the camera using mouse
				if (!isAnyIngameWindowVisible() && !(game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
					&& !game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
				{
					float a = gameCamera->getAngleY();
					float b = float(ogui->getCursorScreenX(0) - AIM_UP_CURSOR_X);
					float my = float(ogui->getCursorScreenY(0));
					
					float safe = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_ROTATION_SAFE);
					float strength = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_ROTATION_STRENGTH)*0.001f;
					float spring = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_ROTATION_SPRING);
					float fade_start = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_ROTATION_FADE_START);
					float fade_end = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_ROTATION_FADE_END);

					if ((my>fade_start) && (fade_start<fade_end)) {
						safe = safe + ((my-fade_start)/(fade_end-fade_start))*(512-safe);
					}

					if (b>safe) {
						a+= -(b-safe)*strength*frametime;

						// new: if spring value is zero, make sure it never springs...
						// (otherwise it will spring back by the minimum value of 1 pixel or something)
						// - jpk
						if (fabs(spring) > 0.001f)
						{
							ogui->setCursorScreenOffsetX(0, int(-(b-safe*0.8f)*0.04f*spring-1));
						}
					} else if (b<-safe) {
						a+= -(b+safe)*strength*frametime;

						// new: if spring value is zero, make sure it never springs...
						// (otherwise it will spring back by the minimum value of 1 pixel or something)
						// - jpk
						if (fabs(spring) > 0.001f)
						{
							ogui->setCursorScreenOffsetX(0, int(-(b+safe*0.8f)*0.04f*spring+1));
						}
					}
					
//					ogui->setCursorScreenOffsetX(0, -b*0.1f);
//					ogui->setCursorScreenX(0, AIM_UP_CURSOR_X+b*0.9);
					//if (b<1) ogui->setCursorScreenX(0, ogui->getCursorScreenX(0)-2);
//					a+= -b*0.01;
//					Logger::getInstance()->error(int2str(ogui->getCursorScreenX(0)));
					gameCamera->setAngleY(a);
				}

				if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
				{
					if ((combatWindows[game->singlePlayerNumber]->isGUIVisible() || forceCursorVisible)
						&& !firstPerson[0]->isDestroyed()
						&& firstPerson[0]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
						&& !game->isPaused())
					{
						// umm...??? --jpk
						//gameCamera->setAngleY(270 - rot.y + CAMERA_EXTRA_ROTATION);

						gameCamera->setAngleY(270 - rot.y);

						/*
						// TEMP!!!
						// oh, ffs, this is such a crap...
						if (foofoo_diag1)
						{
							fooroty = rot.y-45;
							if (fooroty < 0) fooroty += 360.0f;
							assert(fooroty >= 0);
							if (firstPerson[0]->getVisualObject() != NULL)
								firstPerson[0]->getVisualObject()->rotateBone(firstPerson[0]->getUnitType()->getAimBone(), +45);
						}
						if (foofoo_diag2)
						{
							fooroty = rot.y+45;
							if (fooroty >= 360) fooroty -= 360.0f;
							assert(fooroty <= 360);
							if (firstPerson[0]->getVisualObject() != NULL)
								firstPerson[0]->getVisualObject()->rotateBone(firstPerson[0]->getUnitType()->getAimBone(), -45);
						}
						firstPerson[0]->setRotation(rot.x, fooroty, rot.z);

						//gameCamera->setAngleY(270 - fooroty);
						// HACK: attempt to reduce lag...
						if (firstPerson[0]->getVisualObject() != NULL)
						{
							firstPerson[0]->getVisualObject()->prepareForRender();
						}
						*/

						/*
						// TEMP!!!
						firstPerson[0]->setRotation(rot.x, rot.y, rot.z);
						*/
					}
				}

				firstPerson[0]->setLookBetaAngle(lookrotx);
				float alpha = rot.y + 90;
				float heightOffset = 1.7f;
				VC3 vel = firstPerson[0]->getVelocity();
				if (firstPerson[0]->getUnitType()->isSticky())
				{ 					
					if (vel.x != 0 || vel.z != 0)
					{
						if (!thirdPersonView)
							heightOffset += 0.05f * sinf(((float)(game->gameTimer % 360) * (3*3.1415f/180.0f)));
					}
				}
				if (gameCamera->isFirstPersonMode())
				{
					if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
					{
						int camplayers = 0;
						for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
						{
							if (firstPerson[c] != NULL
								&& firstPerson[c]->isActive()
								&& !firstPerson[c]->isDestroyed()
								&& firstPerson[c]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
							{
								// don't rotate to player's view...
								if (thirdPersonView)
								{
									lookrotx = 0;
									alpha = 360 - gameCamera->getAngleY();

									// move the camera a bit toward the cursor
									// (don't center directly on the person)
									VC3 unitpos = firstPerson[c]->getPosition();
									VC3 aimpos = firstPerson[c]->targeting.getAimingPosition();
									VC3 aimvec = VC3(aimpos.x - unitpos.x, 0, aimpos.z - unitpos.z);
									if (aimvec.GetLength() > gameCamera->getZoom() / 1.6f)
										aimvec = aimvec / aimvec.GetLength() * gameCamera->getZoom() / 1.6f;
									//unitpos += aimvec / 3;
									aimvec = aimvec * (float)SimpleOptions::getInt(DH_OPT_I_CAMERA_SCROLLY_AMOUNT) / 100.0f;

									VC3 scrolly = gameCamera->getScrollPosition(c);
									if (!scrollyEnabled || game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
									{
										aimvec = VC3(0,0,0);
									}

									float speedfact = (float)SimpleOptions::getInt(DH_OPT_I_CAMERA_SCROLLY_SPEED) / 100.0f;
									if (scrolly.x < aimvec.x - 0.5f) scrolly.x += (0.02f + (float)fabs(scrolly.x - aimvec.x) / 20.0f) * speedfact;
									if (scrolly.x > aimvec.x + 0.5f) scrolly.x -= (0.02f + (float)fabs(scrolly.x - aimvec.x) / 20.0f) * speedfact;
									if (scrolly.z < aimvec.z - 0.5f) scrolly.z += (0.02f + (float)fabs(scrolly.z - aimvec.z) / 20.0f) * speedfact;
									if (scrolly.z > aimvec.z + 0.5f) scrolly.z -= (0.02f + (float)fabs(scrolly.z - aimvec.z) / 20.0f) * speedfact;

									// NEW: follow the interpolated model position!
									VC3 interppos = unitpos;
									if (firstPerson[c]->getVisualObject() != NULL)
									{
										VC3 renderpos = firstPerson[c]->getVisualObject()->getRenderPosition();
										// HACK: umm... prevent camera glitch when visual object gets recreated...?
										if (renderpos.x >= -99999.0f)
										{
											interppos = renderpos;
										}
									}

									gameCamera->setScrollPosition(scrolly, c);
									VC3 scrolledPos = interppos + scrolly;

									// move camera a bit backward from player position
									// if game mode is aim_upward
									if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
									{
										//float a = fooroty;
										float a = rot.y;
										float xoff = -CAMERA_AIM_UPWARD_OFFSET * sinf(a * 3.1415f / 180.0f);
										float yoff = -CAMERA_AIM_UPWARD_OFFSET * cosf(a * 3.1415f / 180.0f);
										scrolledPos.x += xoff;
										scrolledPos.z += yoff;
									}

									gameCamera->setFirstPersonPosition(interppos.x, interppos.z, scrolledPos.x+positionOffset.x*POSITION_OFFSET_AMOUNT, scrolledPos.z+positionOffset.z*POSITION_OFFSET_AMOUNT, scrolledPos.y + heightOffset, alpha, lookrotx+(aimOffset-oldAimOffset)*0.0f);

									//gameCamera->setFirstPersonPosition(unitpos.x, unitpos.z, scrolledPos.x+positionOffset.x*POSITION_OFFSET_AMOUNT, scrolledPos.z+positionOffset.z*POSITION_OFFSET_AMOUNT, scrolledPos.y + heightOffset, alpha, lookrotx+(aimOffset-oldAimOffset)*0.0f);
									//gameCamera->setFirstPersonPosition(unitpos.x, unitpos.z, scrolledPos.x, scrolledPos.z, scrolledPos.y + heightOffset, alpha, rot.x + lookrotx);
									camplayers++;
								}
							}
						}
						// no player is conscious
						if (camplayers == 0)
						{
							// in coop, don't move the camera (to fix death cinematic)
							if(game->isCooperative())
							{
								pos.x = gameCamera->getTargetPosition().x;
								pos.z = gameCamera->getTargetPosition().z;
							}
							lookrotx = 0;
							alpha = 360 - gameCamera->getAngleY();
							gameCamera->setFirstPersonPosition(pos.x, pos.z, pos.x, pos.z, pos.y + heightOffset, alpha, lookrotx);
							//gameCamera->setFirstPersonPosition(pos.x, pos.z, pos.x, pos.z, pos.y + heightOffset, alpha, rot.x + lookrotx);
						}
					}

					// autozoom - inside building zoom in (and outside zoom out)
					if (firstPerson[0] != NULL
						&& !game->isCinematicScriptRunning()
						&& gameCamera == cameras[GAMEUI_CAMERA_NORMAL]
						&& SimpleOptions::getBool(DH_OPT_B_CAMERA_AUTOZOOM_ENABLED))
					{
						VC3 position = firstPerson[0]->getPosition();
						if (game->gameMap->isWellInScaledBoundaries(position.x, position.z))
						{
							int x = game->gameMap->scaledToObstacleX(position.x);
							int y = game->gameMap->scaledToObstacleY(position.z);

							if (game->gameMap->getAreaMap()->isAreaAnyValue(x, y, AREAMASK_INBUILDING)
								&& game->gameMap->getAreaMap()->isAreaAnyValue(x, y-1, AREAMASK_INBUILDING)
								&& game->gameMap->getAreaMap()->isAreaAnyValue(x, y+1, AREAMASK_INBUILDING)
								&& game->gameMap->getAreaMap()->isAreaAnyValue(x-1, y, AREAMASK_INBUILDING)
								&& game->gameMap->getAreaMap()->isAreaAnyValue(x+1, y, AREAMASK_INBUILDING))
							{
								CameraAutozoomer::setCurrentArea(CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_INDOOR);
							} else {
								CameraAutozoomer::setCurrentArea(CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_OUTDOOR);
							}
							CameraAutozoomer::run(gameCamera);
						}
					}
				}
				if (!msgBoxIsOpen)
				{
					if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
					{
						if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
						{
							if (!this->isAnyIngameWindowVisible())
								ogui->skipCursorMovement();
						} else {
							if (!thirdPersonView)
								ogui->skipCursorMovement();
						}
					} else {
						ogui->skipCursorMovement();
					}
				}
			}

			if (firstPerson[0] != NULL && !game->isTacticalMode()
				&& (gameCamera->isFirstPersonMode() || SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS)))
			{
				// update aiming position
				for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
				{
					if (firstPerson[c] != NULL)
					{
						if (sceneSelection[c]->hit)
						{
							if (sceneSelection[c]->unit != NULL)
							{
								firstPerson[c]->targeting.setAimingPosition(sceneSelection[c]->unit->getPosition());

							} else {
								float aimHeight = game->gameMap->getScaledHeightAt(sceneSelection[c]->scaledMapX, sceneSelection[c]->scaledMapY);
								VC3 aimPos = VC3(sceneSelection[c]->scaledMapX, aimHeight, sceneSelection[c]->scaledMapY);
								float waterDepth = game->waterManager->getWaterDepthAt(aimPos);
								if (waterDepth > 0.5f)
								{
									// keep aim at water surface (never below surface).
									// that is: 0.5m below surface to be exact
									aimPos.y += (waterDepth - 0.5f);
								}
								// FIXME: this really does not affect anything...
								// as the firing still uses just the 2d coordinate
								// and then adjusts the height based on ground height...
								// dont shoot above/below 2m of unit height.
								if (aimPos.y < firstPerson[c]->getPosition().y - 2.0f)
								{
									aimPos.y = firstPerson[c]->getPosition().y - 2.0f;
								}
								else if (aimPos.y > firstPerson[c]->getPosition().y + 2.0f)
								{
									aimPos.y = firstPerson[c]->getPosition().y + 2.0f;
								}
								if (firstPerson[c]->isSideways())
								{
									VC3 diff = aimPos - firstPerson[c]->getPosition();
									diff.y = 0;
									if (fabs(diff.x) < 1.0f && diff.z > -0.5f && diff.z < 2.0f)
									{
										if (fabs(diff.x) < 0.25f)
											aimPos = firstPerson[c]->targeting.getAimingPosition();
										else
											aimPos.z = firstPerson[c]->targeting.getAimingPosition().z;
									}
								}
								firstPerson[c]->targeting.setAimingPosition(aimPos);
							}
						}
					}

					// DUH, gotta do this to make sure the flashlight won't
					// "shake".
					// but don't override the rotation with roll jumps.
					if (combatWindows[game->singlePlayerNumber] != NULL
						&& (combatWindows[game->singlePlayerNumber]->isGUIVisible() || forceCursorVisible)
						&& firstPerson[c] != NULL
						&& !firstPerson[c]->isDestroyed()
						&& firstPerson[c]->isActive()
						&& firstPerson[c]->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
						&& !game->isPaused()
						&& sceneSelection[c]->hit
						&& !firstPerson[c]->isAnimated())
					{
						if (firstPerson[c]->getJumpCounter() == 0
							|| !firstPerson[c]->getUnitType()->doesRollJump())
						{
							if (firstPerson[c]->getFlashlight() != NULL)
							{
								// FIXME: something wrong with this, makes flashlight go all wonky
								igios_unimplemented();
                                /*
								Flashlight *fl = firstPerson[c]->getFlashlight();

								VC3 unitpos = firstPerson[c]->getPosition();
								VC3 selpos = VC3(sceneSelection[c]->scaledMapX, 0, sceneSelection[c]->scaledMapY);
								float angle = util::PositionDirectionCalculator::calculateDirection(unitpos, selpos);

								fl->setRotation(UNIT_ANGLE_TO_RAD(angle));
                                */
							}
						}
					}
				}

			}


			// camera unit locking/releasing...
			if (wasKeyClicked(DH_CTRL_CAMERA_UNIT_LOCK_TOGGLE)) 
			{
				if (gameCamera->isFollowingUnit())
				{
					gameCamera->setFollowingUnit(false);
					gameCamera->setDisableUserMovement(false);
				} else {
					gameCamera->setFollowingUnit(true);
				}
			}
			if (gameCamera->isFollowingUnit())
			{
				LinkedList *ulist = game->units->getOwnedUnits(game->singlePlayerNumber);
				LinkedListIterator iter = LinkedListIterator(ulist);
				int amount = 0;
				int amount2 = 0;
				float x = 0; // selected units
				float y = 0;
				float x2 = 0; // for unselected units, if none selected
				float y2 = 0;
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive() && !u->isDestroyed())
					{
						if (u->isSelected())
						{
							amount++;
							VC3 pos = u->getPosition();
							x += pos.x;
							y += pos.z;
						} else {
							amount2++;
							VC3 pos = u->getPosition();
							x2 += pos.x;
							y2 += pos.z;
						}
					}
				}
				if (amount > 0)
				{
					x /= amount;
					y /= amount;
					gameCamera->setFollowPosition(x, y);
					gameCamera->setDisableUserMovement(true);
				} else {
					if (amount2 > 0)
					{
						x2 /= amount2;
						y2 /= amount2;
						gameCamera->setFollowPosition(x2, y2);
						gameCamera->setDisableUserMovement(true);
					} 
					// else all dead?
				}
			}

			if (SimpleOptions::getBool(DH_OPT_B_CAMERA_SYSTEM_ENABLED) && (gameCamera==cameras[GAMEUI_CAMERA_NORMAL]))
			{
/*				// CAMERA SYSTEM STUFF HERE!
				VC3 pos(0, 0, 0);
				// TODO: Add coop support
				if (firstPerson[0] != NULL)
				{
					pos = firstPerson[0]->getPosition();
				}
				cameraSystem->update(pos, currentTime - lastRunUITime);*/

				 // HACK: Detect change in camera attributes and update them to the camera system to
				 // enable old style camera adjustments on the new camera system
				static float oldAngle = 0;
				static float oldBetaAngle = 0;
				static float oldFOV = 0;
				static float oldDistance = 0;
				static bool oldSet = false;
				if (oldSet) {
/*					float curAngle = gameCamera->getAngleY();
					float curBetaAngle = gameCamera->getBetaAngle();
					float curFOV = gameCamera->getFOV();
					float curDistance = gameCamera->getZoom();
					cameraSystem->moveCameraAngle(oldAngle-curAngle);
					cameraSystem->moveCameraBetaAngle(oldBetaAngle-curBetaAngle);
					cameraSystem->moveCameraFOV(curFOV-oldFOV);
					cameraSystem->moveCameraDistance(curDistance-oldDistance);*/
				}

				 // Set up the game camera based on the camera system
				 // This is not a hack...
				gameCamera->applyToSceneAbsolute(cameraSystem->getCameraPosition(),
												 cameraSystem->getTargetPosition(),
												 cameraSystem->getUpVector(),
												 cameraSystem->getFOV());
				 // ...And the hack continues!
				oldSet = true;
				oldAngle = gameCamera->getAngleY();
				oldBetaAngle = gameCamera->getBetaAngle();
				oldFOV = gameCamera->getFOV();
				oldDistance = gameCamera->getZoom();

			} else {
				gameCamera->applyToScene();
			}

			combatWindows[game->singlePlayerNumber]->updateUnitPointers();
		}

		// added for the loading window close with 360's start button hack
		if( this->loadingWindow != NULL )
		{
			if( loadingWindow->shouldAutoClose() || wasKeyClicked( DH_CTRL_CLOSE_LOADING_WINDOW ) )
			{
				loadingWindow->closeWindow();
			}
		}
#ifdef PROJECT_SURVIVOR
		ogui->SetMenuIndexMode(0, false);
#else
		// menu button warping with joystick?
		if (game->inCombat && !msgBoxIsOpen 
			&& loadingWindow == NULL)
		{
			// TODO: cursor number...
			ogui->SetMenuIndexMode(0, false);
		} else {
			// TODO: cursor number...
			ogui->SetMenuIndexMode(0, true);
		}
#endif

		// standard screenshot
		if (wasKeyClicked(DH_CTRL_SCREENSHOT) && !game::SimpleOptions::getBool(DH_OPT_B_MAGIC_SCREENSHOT)) 
		{
			util::ScreenCapturer::captureScreen(storm3d);
		}

		// magic screenshot
		//
		if (game::SimpleOptions::getBool(DH_OPT_B_MAGIC_SCREENSHOT))
		{
			static bool magic_screenshot_queued = false;
			static bool magic_screenshot_gui_was_visible = false;
			static bool magic_screenshot_game_was_paused = false;

			// pressed shot
			if(!magic_screenshot_queued && wasKeyClicked(DH_CTRL_SCREENSHOT))
			{
				// queue screenshot to next frame
				magic_screenshot_queued = true;
				magic_screenshot_game_was_paused = game->isPaused();
				magic_screenshot_gui_was_visible = combatWindows[game->singlePlayerNumber] && combatWindows[game->singlePlayerNumber]->isGUIVisible();

				// game not paused
				if(!magic_screenshot_game_was_paused)
				{
					// pause
					game->setPaused(true);
				}

				// gui is visible
				if(magic_screenshot_gui_was_visible)
				{
					// make invisible
					setGUIVisibility(game->singlePlayerNumber, false);
					if(combatWindows[game->singlePlayerNumber]) combatWindows[game->singlePlayerNumber]->updateCursorImage();
				}
			}
			else if(magic_screenshot_queued)
			{
				// take shot without gui
				util::ScreenCapturer::captureScreen(storm3d);
				// render gui
				setGUIVisibility(game->singlePlayerNumber, true);
				if(combatWindows[game->singlePlayerNumber]) combatWindows[game->singlePlayerNumber]->updateCursorImage();
				ogui->Run(GAME_TICK_MSEC);
				scene->RenderScene();
				// take 2nd shot
				util::ScreenCapturer::captureScreenWithLastName(storm3d, "_GUI");

				// reset original state

				if(!magic_screenshot_gui_was_visible)
					setGUIVisibility(game->singlePlayerNumber, false);
				
				if(!magic_screenshot_game_was_paused)
					game->setPaused(false);

				magic_screenshot_queued = false;
			}
		}

		// message showing counter...
		for (int msg = 0; msg < MESSAGE_TYPES_AMOUNT; msg++)
		{
			if (lastGameMessageCounter[msg] > 0)
			{
				lastGameMessageCounter[msg] -= (currentTime - lastRunUITime);
				if (lastGameMessageCounter[msg] <= 0)
				{
					lastGameMessageCounter[msg] = 0;
					if (combatWindows[game->singlePlayerNumber] != NULL)
					{
						if (msg == MESSAGE_TYPE_NORMAL
							|| msg == MESSAGE_TYPE_RADIO
							|| msg == MESSAGE_TYPE_RADIO2)
						{
							combatWindows[game->singlePlayerNumber]->clearMessage();
						}
						if (msg == MESSAGE_TYPE_HINT)
						{
							combatWindows[game->singlePlayerNumber]->clearHintMessage();
						}
						if (msg == MESSAGE_TYPE_CENTER_BIG)
						{
							combatWindows[game->singlePlayerNumber]->clearCenterMessage();
						}
						if (msg == MESSAGE_TYPE_EXECUTE_TIP)
						{
							combatWindows[game->singlePlayerNumber]->clearExecuteTipMessage();
						}
					} 			 
				}
			}
		}

		// psd. show/hide building tops
		if(game->inCombat)
		{
			// now done only 10 times a second. (100ms update interval) 
			if (currentTime > buildingHandlerUpdateTime + 100)
			{
				buildingHandlerUpdateTime = currentTime;

				buildingHandler.beginUpdate();
			
				LinkedList *unitList = game->units->getOwnedUnits(game->singlePlayerNumber);
				// hostiles seen inside buildings no longer remove roof
				//LinkedList *unitList = game->units->getAllUnits();
				LinkedListIterator unitIterator = LinkedListIterator(unitList);
				bool removeRoofs = false;

				while(unitIterator.iterateAvailable())
				{
					Unit *unit = (Unit *)unitIterator.iterateNext();

					if (unit->getOwner() == game->singlePlayerNumber)
					// hostiles seen inside buildings no longer remove roof
					// || (unit->visibility.isSeenByPlayer(game->singlePlayerNumber)
					//	&& !unit->isDestroyed()))
					{
						const VC3 &unitPosition = unit->getPosition();
				
						int x = game->gameMap->scaledToPathfindX(unitPosition.x);
						int y = game->gameMap->scaledToPathfindY(unitPosition.z);
				
						IStorm3D_Model *model = game->getGameScene()->getBuildingModelAtPathfind(x, y);
						if(model)
						{
							//buildingHandler.removeTopFrom(model);
							removeRoofs = true;
							break;
						}
					}
				}
			
				if(removeRoofs)
					buildingHandler.removeAllTops();

				buildingHandler.endUpdate();

			}
		}

		// ambient sound areas
		if(game->inCombat)
		{
			if (firstPerson[0] != NULL)
			{
				VC3 pos = firstPerson[0]->getPosition();
				game->gameMap->keepWellInScaledBoundaries(&pos.x, &pos.z);
				int x = game->gameMap->scaledToObstacleX(pos.x);
				int y = game->gameMap->scaledToObstacleY(pos.z);
				if (game->gameMap->getAreaMap()->isAreaValue(x, y, AREAMASK_INBUILDING, AREAVALUE_INBUILDING_YES))
				{
#ifdef PROJECT_SURVIVOR
					// no indoor sounds for survivor
					this->ambientAreaManager->enterArea(sfx::AmbientAreaManager::Outdoor);
#else
					this->ambientAreaManager->enterArea(sfx::AmbientAreaManager::Indoor);
#endif
				} else {
					this->ambientAreaManager->enterArea(sfx::AmbientAreaManager::Outdoor);
				}
			}
		}
		
		if (lightningVisualEffect != NULL)
		{
			if (lightningTime > 0)
			{
				lightningTime -= (currentTime - lastRunUITime);
				if (lightningTime <= 0)
				{
					lightningTime = 0;
					lightningVisualEffect->setDeleteFlag();
					lightningVisualEffect->freeReference();
					lightningVisualEffect = NULL;
				}
			}
		}

		if (scoreWindow || !game->isPaused() || !game->inCombat)
		{
			IStorm3D_TerrainRenderer *rend = NULL;
			if (renderTerrain != NULL)
				rend = &renderTerrain->GetTerrain()->getRenderer();
			effects->run(currentTime - lastRunUITime, rend, this->scene->GetCamera());
		}
		ambientAreaManager->update(currentTime - lastRunUITime);

		if (combatWindows[game->singlePlayerNumber] != NULL && 
			// all the rest if cases added by Pete to prevent the talking heads
			// from raising above the opened window
#ifdef GUI_BUILD_MAP_WINDOW
			( mapWindow == NULL || mapWindow->isVisible() == false ) && 
#else
			( true ) && 
#endif
			upgradeWindow == NULL && 
#ifdef GUI_BUILD_LOG_WINDOW
			logWindow == NULL && 
#else
			true &&
#endif
			commandWindows[0] == NULL )
		{
			combatWindows[game->singlePlayerNumber]->raiseMessages();
		}

#ifdef GUI_BUILD_MAP_WINDOW
		if (mapWindow != NULL)
		{
			mapWindow->effectUpdate(currentTime - lastRunUITime);

			if (mapWindow->isVisible())
			{
				mapWindow->raise();
			}
		}
#endif
		
		if (upgradeWindow != NULL)
		{
			upgradeWindow->effectUpdate(currentTime - lastRunUITime);
			upgradeWindow->raise();
		}

#ifdef GUI_BUILD_LOG_WINDOW
		if (logWindow != NULL )
		{
			logWindow->update( currentTime - lastRunUITime );
			// logWindow->raise();
		}
#endif

		if( cinematicScreen )
		{
			cinematicScreen->update();
			if( cinematicScreen->shouldBeDeleted() )
			{
				closeCinematicScreen();
			}
		}

		if( terminalManager && !game->isCinematicScriptRunning() )
		{
			terminalManager->update();
		}
#ifdef PROJECT_SURVIVOR
		if( scoreWindow )
		{
			// just a quick and dirty hack to close the scoreWindow
			if( this->isScoreWindowOpen() && scoreWindow->CloseMePlease() )
				closeScoreWindow( game->singlePlayerNumber );
			else
				scoreWindow->Update( currentTime - lastRunUITime );
		}

		if( missionSelectionWindow )
		{
			missionSelectionWindow->Update( currentTime - lastRunUITime );
		}

		if( missionFailureWindow )
		{
			if(missionFailureWindow->closeMePlease())
			{
				closeMissionFailureWindow();
			}
		}
#endif

#ifdef PROJECT_SURVIVOR
		if (upgradeWindow != NULL)
		{
			upgradeWindow->raise();
		}
		if (characterSelectionWindow != NULL)
		{
			characterSelectionWindow->raise();
		}
#endif

#ifdef GUI_BUILD_INGAME_GUI_TABS
		if ( ingameGuiTabs != NULL && ingameGuiTabs->isVisible() )
		{
			ingameGuiTabs->raise();
		}
#endif

		if (loadingWindow != NULL )
		{
			loadingWindow->raise();
			if( cinematicScreen )
				cinematicScreen->raise();
		}

#ifdef GUI_BUILD_MAP_WINDOW
		if(mapWindow)
		{
			VC3 pos;
			VC3 rot;

			if(firstPerson[0])
			{
				pos = firstPerson[0]->getPosition();
				rot = firstPerson[0]->getRotation();

				rot.y += firstPerson[0]->getLastBoneAimDirection();
			}

			VC2 playerPos(pos.x, pos.z);
			float rotation = UNIT_ANGLE_TO_RAD(rot.y);
			mapWindow->setEntity(MapWindow::Player, playerPos, rotation);
			mapWindow->update(currentTime - lastRunUITime);
		}
#endif

		if ( game->inCombat && currentTime > guiAnimationUpdateTime + 20 )
		{
			guiAnimationUpdateTime = currentTime;
			combatWindows[game->singlePlayerNumber]->updateGUIAnimations();
		}


		game->runCustomUIScriptProcesses();

		// not actually last time, but last time that was over 10 ms ago...
		// ...or whatever the timer resolution is
		if (currentTime - lastRunUITime != 0)
			lastRunUITime = currentTime;
	}

	void GameUI::updateCameraDependedElements()
	{
		if( game->inCombat )
		{
			combatWindows[ game->singlePlayerNumber ]->updateCameraDependedElements();
		}
	}

	void GameUI::setPointersChangedFlag(int player)
	{
		// TODO: proper impl for multiplayer
		if (player == game->singlePlayerNumber)
		{
			pointersChanged = true;
		}
	}

	void GameUI::setUnitDestroyedFlag(int player)
	{
		// TODO: proper impl for multiplayer
		if (player == game->singlePlayerNumber)
		{
			unitsDestroyed = true;
		}
	}

	void GameUI::setUnitDamagedFlag(int player)
	{
		// TODO: proper impl for multiplayer
		if (player == game->singlePlayerNumber)
		{
			unitsDamaged = true;
		}
	}

	SceneSelection *GameUI::getSceneSelection(int clientNumber)
	{
		assert(clientNumber >= 0 && clientNumber < MAX_PLAYERS_PER_CLIENT);
		return sceneSelection[clientNumber];
	}

	void GameUI::selectCamera(int camera)
	{
		assert(camera >= 0 && camera < GAMEUI_CAMERA_AMOUNT);
		gameCamera = this->cameras[camera];
	}

	int GameUI::getCameraNumber()
	{
		for (int i = 0; i < GAMEUI_CAMERA_AMOUNT; i++)
		{
			if (gameCamera == cameras[i])
				return i;
		}
		assert(0);
		return -1;
	}

	/*
	void GameUI::setMusic(char *filename)
	{
		if (soundMixer != NULL)
			soundMixer->setMusic(filename);
	}

	void GameUI::setMusicPlaylist(char *filename)
	{
		if (musicPlaylist != NULL)
		{
			musicPlaylist->setMusic(filename);
		}
	}

	bool GameUI::isMusicFading()
	{
		if (soundMixer != NULL)
			return soundMixer->isMusicFading();
		return false;
	}
	*/

	bool GameUI::isQuitRequested()
	{
		return quitRequested;
	}

	void GameUI::setQuitRequested()
	{
		quitRequested = true;
	}

	bool GameUI::isAbortingMission()
	{
		return abortMission;
	}

	void GameUI::setAbortingMission(bool abortFlag)
	{
		abortMission = abortFlag;
	}

	GameCamera *GameUI::getGameCamera()
	{
		return gameCamera;
	}

	GameController *GameUI::getController(int clientNumber)
	{
		assert(clientNumber >= 0 && clientNumber < MAX_PLAYERS_PER_CLIENT);

		return gameController[clientNumber];
	}

	SceneSelection GameUI::cursorRayTracePlayer(int player, bool terrainOnly, bool accurate)
	{
		if (cursorRayTracer == NULL || !SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + player))
		{
			// return empty selection
			return SceneSelection();
		}

		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
		{
			return cursorRayTrace(AIM_UP_CURSOR_X, AIM_UP_CURSOR_Y, terrainOnly, accurate);
		}
		else if (SimpleOptions::getBool(DH_OPT_B_GUI_CURSOR_IS_RAYTRACING))
		{
			// HACK: attempt to minimize the error caused by camera angle
			// WARNING: this will screw up the aim in case on different camera angles

			int x, y;

			// free-camera mode joystick HACK
			GameController *ctrl = getController(player);
			if (ctrl->controllerTypeHasJoystick())
			{
				ctrl->getJoystickValues(NULL, NULL, &x, &y);

				if (x != 0 || x != 0)
				{
					// only if joystick is pointing in some directon
					// we compare to 0 because GameController has taken care of dead zones
					x = 512 + (x * 512 / 1000);
					y = 384 + (y * 384 / 1000);
					oldJoystickXY[player] = VC2(x, y);

					// Don't let ogui update cursor position if we are not in menus
					if (game->inCombat && !game->isPaused())
					{
						ogui->setCursorScreenX(player, x);
						ogui->setCursorScreenX(player, y);
						ogui->skipCursorMovement();
					}
				} else {
					// joystick centered, use old position so character direction does not "reset"
					x = oldJoystickXY[player].x;
					y = oldJoystickXY[player].y;
				}
			} else {
				x = ogui->getCursorScreenX(player);
				y = ogui->getCursorScreenY(player);
			}

			int rayY = y + SimpleOptions::getInt(DH_OPT_I_CURSOR_RAYTRACE_OFFSET_Y);
			if (rayY < 0) rayY = 0;
			if (rayY >= 768) rayY = 768-1;
			return cursorRayTrace(x, rayY, terrainOnly, accurate);
		}
		else
		{
			return cursorRayTrace(ogui->getCursorScreenX(player), ogui->getCursorScreenY(player), true, accurate);
		}
	}

	SceneSelection GameUI::cursorRayTrace(int x, int y, bool terrainOnly, bool accurate)
	{
		SceneSelection ret;
		Storm3D_CollisionInfo cinfo;
		cursorRayTracer->rayTrace(x, y, cinfo, SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS), terrainOnly, accurate);
		if (cinfo.hit)
		{
			Unit *hitu = NULL;

			// go through all units, see if model matches one of em
			// the model should equal one of the unit visualizations.
			// if not, just convert to map coordinates

			// TODO: isn't this just a horrible waste of performance...?
			// optimize, use the model's custom data to solve the visual object or something!
			LinkedList *ulist = game->units->getAllUnits();
			ulist->resetIterate();
			while (ulist->iterateAvailable())
			{
				Unit *u = (Unit *)ulist->iterateNext();
				if (u->getVisualObject() != NULL)
				{
					if (u->getVisualObject()->model == cinfo.model)
					{
						hitu = u;
						break;
					}
				}
			}

			if (hitu != NULL)
			{
				ret.hit = true;
				VC3 pos = hitu->getPosition();
				ret.scaledMapX = pos.x;
				ret.scaledMapY = pos.z;
				ret.unit = hitu;
			} else {
				// TODO: check that position is in map boundaries!
				ret.hit = true;
				ret.scaledMapX = cinfo.position.x;
				ret.scaledMapY = cinfo.position.z;
				ret.unit = NULL;
			}
		} else {
			ret.hit = false;
			ret.scaledMapX = 0;
			ret.scaledMapY = 0;
			ret.unit = NULL;
		}
		return ret;
	}

	VisualEffectManager *GameUI::getVisualEffectManager() 
	{
		return visualEffectManager;
	}

	int GameUI::parseSoundFromDefinitionString(const char *sounddef, 
		float x, float y, float z, bool *looped, int *handle, int *key,
		bool continueOldSound, float range, int priority, bool muteVolume, bool ambient)
	{
		if (soundMixer != NULL)
		{
			if (sounddef == NULL)
				return -1;

			if (sounddef[0] == '!')
			{
				*looped = true; 
				char foo[256];
				int slen = strlen(sounddef);
				if (slen < 256)
				{
					strcpy(foo, sounddef);
				} else {
					slen = 0;
					assert(0);
				}
				foo[0] = '\0';
				int seppos[3] = { 0, -1, -1 };
				int splitpos[3] = { -1, -1, -1 };
				int splnum = 0;
				bool invalidStr = false;
				for (int i = 1; i < slen; i++)
				{
					if (foo[i] == ',')
					{
						if (splnum < 2)
						{
							if (splitpos[splnum] == -1)
								invalidStr = true;
							foo[i] = '\0';
							splnum++;
							seppos[splnum] = i;
						} else {
							invalidStr = true;
						}
					}
					if (foo[i] == '*')
					{
						if (splnum < 3)
						{
							if (splitpos[splnum] != -1)
								invalidStr = true;
							foo[i] = '\0';
							splitpos[splnum] = i;
						} else {
							invalidStr = true;
						}
					}
				}
				if (splitpos[0] > 0 && !invalidStr)
				{
					// TODO... a lot more things... (start/end)

					int loopDuration = str2int(&foo[seppos[0] + 1]);
					int startDuration = str2int(&foo[seppos[1] + 1]);
					int endDuration = str2int(&foo[seppos[2] + 1]);
					if (continueOldSound)
					{
						assert(*handle != -1);
						bool contOk = soundLooper->continueLoopedSound(*handle, *key, loopDuration);
						if (contOk)
						{
							soundLooper->setSoundPosition(*handle, *key, x, y, z);
							return *handle;
						} else {
							return -1;
						}
					} else {
						SoundSample *loopSample = NULL;
						SoundSample *startSample = NULL;
						SoundSample *endSample = NULL;

						if (foo[splitpos[0] + 1] != '\0') 
							loopSample = soundMixer->loadSample(&foo[splitpos[0] + 1], false);
						if (foo[splitpos[1] + 1] != '\0') 
							startSample = soundMixer->loadSample(&foo[splitpos[1] + 1], false);
						if (foo[splitpos[2] + 1] != '\0') 
							endSample = soundMixer->loadSample(&foo[splitpos[2] + 1], false);

						if (loopSample != NULL)
						{
							//VC3 campos = gameCamera->getPosition();
							VC3 campos = gameCamera->getActualInterpolatedPosition();
							VC3 spos = VC3(x, y, z);
							VC3 distVector = spos - campos;
							if (distVector.GetSquareLength() <= GAMEUI_MAX_SOUND_DISTANCE_SQ)
							{
								int h = soundLooper->playLoopedSound(startSample, 
									loopSample, endSample, startDuration, loopDuration, 
									endDuration, x, y, z, key, muteVolume, range, priority, ambient);
								*handle = h;
								return h;
							}
						}
					}
				} else {
					Logger::getInstance()->warning("GameUI::parseSoundFromDefinitionString - Invalid looping sound definition.");
				}
				return -1;
			} else {
				*looped = false;
				return playSoundEffect(sounddef, x, y, z, false, DEFAULT_SOUND_EFFECT_VOLUME, range, priority, ambient);
			}
		} else {
			return -1;
		}
	}

	int GameUI::playSoundEffect(const char *filename, float x, float y, float z, bool loop, int volume, float range, int priority, bool ambient)
	{
		if (soundMixer != NULL)
		{
			SoundSample *s = soundMixer->loadSample(filename, false);
			if (s == NULL)
			{
				gameui_sound_effect_errorcode = GAMEUI_SOUND_EFFECT_ERRORCODE_OTHER;
				return -1;
			} else {
				//VC3 campos = gameCamera->getPosition();
				VC3 campos = gameCamera->getActualInterpolatedPosition();
				VC3 spos = VC3(x, y, z);
				VC3 distVector = spos - campos;
				if (distVector.GetSquareLength() <= GAMEUI_MAX_SOUND_DISTANCE_SQ)
				{
					int handle;
					if(ambient) handle = soundMixer->playAmbientSound(s, loop, range, priority);
					else handle = soundMixer->playSoundEffect(s, loop, range, priority);
					if (handle != -1)
						soundMixer->setSoundPosition(handle, x, y, z, 0, 0, 0, volume, 0);
					if(loop)
						loopingSoundEffectHandles.push_back(handle);
					gameui_sound_effect_errorcode = GAMEUI_SOUND_EFFECT_ERRORCODE_NONE;
					return handle;
				} else {
					gameui_sound_effect_errorcode = GAMEUI_SOUND_EFFECT_ERRORCODE_TOO_FAR;
					return -1;
				}
			}
		} else {
			gameui_sound_effect_errorcode = GAMEUI_SOUND_EFFECT_ERRORCODE_NO_SOUNDS;
			return -1;
		}
	}

	int GameUI::playSpeech(const char *filename, float x, float y, float z, bool loop, int volume, bool volume_adjust)
	{
		std::string identifier = filename;
		std::string::size_type pos_start = identifier.find_last_of("\\/") + 1;
		std::string::size_type pos_end = identifier.find_last_of('.');
		identifier = identifier.substr(pos_start, pos_end - pos_start);

		std::string radiospeechlocale = "radiospeecheffect_" + identifier;

		const char *radio_id = NULL;
		::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI, radiospeechlocale.c_str(), &radio_id );


		if (soundMixer != NULL)
		{
			SoundSample *s = soundMixer->loadSample(filename, false);
			
			Timer::update();
			lipsync_start_time = Timer::getTime() - 200;

			if (s == NULL)
			{
				return -1;
			} else {
				//VC3 campos = gameCamera->getPosition();
				VC3 campos = gameCamera->getActualInterpolatedPosition();
				VC3 spos = VC3(x, y, z);
				VC3 distVector = spos - campos;
				//if (distVector.GetSquareLength() <= GAMEUI_MAX_SOUND_DISTANCE_SQ)
				//{
					int handle = soundMixer->playSpeech(s, loop);
					if (handle != -1)
					{
						soundMixer->setSoundPosition(handle, x, y, z, 0, 0, 0, volume, 0);

#ifdef PROJECT_SURVIVOR
						// turn down other volumes while speech is playing
						if(volume_adjust && s->getLength() > 0)
						{
							int speech_over = lastRunUITime + s->getLength();
							SoundMixer::SoundEvent se;

							// fade out
							se.type = SoundMixer::Event_SpeechStart;
							se.time = 0;
							soundMixer->runSoundEvent(se);

							// get existing fade-in
							SoundMixer::SoundEvent *se_fadein = soundMixer->getLastSoundEventOfType(SoundMixer::Event_SpeechStop);
							// if it fades in sooner than this
							if(se_fadein != NULL && se_fadein->time < speech_over)
							{
								// remove the event
								se_fadein->type = SoundMixer::Event_Nop;
								se_fadein = NULL;
							}

							if(se_fadein == NULL)
							{
								// fade in
								se.type = SoundMixer::Event_SpeechStop;
								se.time = speech_over;
								soundMixer->queueSoundEvent(se);
							}
						}
#endif

						if(radio_id)
						{
							const char *click1 = getLocaleGuiString(("radiospeecheffect_" + std::string(radio_id) + "_startclick_sound").c_str());
							const char *click2 = getLocaleGuiString(("radiospeecheffect_" + std::string(radio_id) + "_endclick_sound").c_str());
							const char *noise = getLocaleGuiString(("radiospeecheffect_" + std::string(radio_id) + "_noise_sound").c_str());
							int click1_time = getLocaleGuiInt(("radiospeecheffect_" + std::string(radio_id) + "_startclick_starttime").c_str(), 0);
							int click2_time = getLocaleGuiInt(("radiospeecheffect_" + std::string(radio_id) + "_endclick_starttime").c_str(), 0);
							int noise_starttime = getLocaleGuiInt(("radiospeecheffect_" + std::string(radio_id) + "_noise_starttime").c_str(), 0);
							int noise_endtime = getLocaleGuiInt(("radiospeecheffect_" + std::string(radio_id) + "_noise_endtime").c_str(), 0);
							int speech_delay = getLocaleGuiInt(("radiospeecheffect_" + std::string(radio_id) + "_speech_starttime").c_str(), 0);

							int speech_length = s->getLength();

							SoundSample *s_click1 = soundMixer->loadSample(click1, false);
							SoundSample *s_click2 = soundMixer->loadSample(click2, false);
							SoundSample *s_noise = soundMixer->loadSample(noise, false);

							{
								// resume current speech
								SoundMixer::SoundEvent se;
								se.type = SoundMixer::Event_Resume;
								se.soundHandle = handle;
								se.time = lastRunUITime + speech_delay;
								soundMixer->queueSoundEvent(se);
								soundMixer->setSoundPaused(handle, true);
							}

							{
								SoundMixer::SoundEvent se;
								se.type = SoundMixer::Event_PlaySpeech;
								se.volume = volume;
								se.loop = false;
								se.x = x;
								se.y = y;
								se.z = z;
								

								// start click1
								se.sample = s_click1;
								se.time = lastRunUITime + click1_time;
								soundMixer->queueSoundEvent(se);

								// start click2
								se.sample = s_click2;
								se.time = lastRunUITime + speech_length + speech_delay + click2_time;
								soundMixer->queueSoundEvent(se);
							}

							// start noise
							int h_noise = soundMixer->playSpeech(s_noise, true);
							if(h_noise != -1)
							{
								soundMixer->setSoundPosition(h_noise, x, y, z, 0, 0, 0, volume, 0);
								soundMixer->setSoundPaused(h_noise, true);

								SoundMixer::SoundEvent se1;
								se1.time = lastRunUITime + noise_starttime;
								se1.soundHandle = h_noise;
								se1.type =  SoundMixer::Event_Resume;
								soundMixer->queueSoundEvent(se1);

								SoundMixer::SoundEvent se2;
								se2.time = lastRunUITime + speech_length + speech_delay + noise_endtime;
								se2.soundHandle = h_noise;
								se2.type =  SoundMixer::Event_Stop;
								soundMixer->queueSoundEvent(se2);
							}
						}
					}
					return handle;
				//} else {
				//	return -1;
				//}
			}
		} else {
			return -1;
		}
	}

	void GameUI::stopSound(int handle)
	{
		assert(handle != -1);
		soundMixer->stopSound(handle);
	}

	void GameUI::preloadSound(const char *filename, bool temporaryCache)
	{
		if (soundMixer != NULL)
		{
			if (temporaryCache)
				Logger::getInstance()->debug("GameUI::preloadSound - Preloading temporary sound.");
			else
				Logger::getInstance()->debug("GameUI::preloadSound - Preloading sound.");
			Logger::getInstance()->debug(filename);

			soundMixer->loadSample(filename, temporaryCache);
		}
	}


	void GameUI::cleanSoundCache(bool temporaryCache)
	{
		if (soundMixer != NULL)
		{
			if (temporaryCache)
				soundMixer->cleanTemporarySampleCache();
			else
				soundMixer->cleanSampleCache();
		}
	}

	void GameUI::clearGameMessage(MESSAGE_TYPE messageType)
	{
		if (lastGameMessageCounter[messageType] > 0)
			lastGameMessageCounter[messageType] = 1;

		// NEW: clear the radio (conversation) messages immediately
		// NOTE: does not apply to other type of messages (trying to keep as stable as possible)
		if (combatWindows[game->singlePlayerNumber] != NULL)
		{
			if (messageType == MESSAGE_TYPE_NORMAL
				|| messageType == MESSAGE_TYPE_RADIO
				|| messageType == MESSAGE_TYPE_RADIO2)
			{
				lastGameMessageCounter[messageType] = 0;
				combatWindows[game->singlePlayerNumber]->clearMessage();
			}
		}
	}

	// HACK: !!!
	void GameUI::clearGameMessageDuration(MESSAGE_TYPE messageType)
	{
		if (lastGameMessageCounter[messageType] > 0)
			lastGameMessageCounter[messageType] = 1;
	}

	void GameUI::gameMessage(const char *message, ui::Visual2D *image, 
		int priority, int duration, MESSAGE_TYPE messageType)
	{
		bool showIt = false;

		bool messageRightSide = false;

		// HACK: radio messages and normal messages are just the same...
		if (messageType == MESSAGE_TYPE_RADIO
			|| messageType == MESSAGE_TYPE_RADIO2)
		{
			if (messageType == MESSAGE_TYPE_RADIO2)
			{
				messageRightSide = true;
			}
			messageType = MESSAGE_TYPE_NORMAL;
		}

		/*
		if (lastGameMessageCounter[messageType] > 0)
		{
			if (priority > lastGameMessagePriority[messageType]
				|| (priority == lastGameMessagePriority[messageType]
				&& lastGameMessageCounter[messageType] < lastGameMessageDuration[messageType] / 2))
			{
				showIt = true;
			}
		} else {
			showIt = true;
		}
		*/
		showIt = true;

		if (messageType == MESSAGE_TYPE_EXECUTE_TIP)
		{
			if (!(2 - priority < SimpleOptions::getInt(DH_OPT_I_GUI_TIP_MESSAGE_LEVEL)))
			{
				showIt = false;
			}
		}

		if (showIt)
		{
			lastGameMessagePriority[messageType] = priority;
			lastGameMessageDuration[messageType] = duration;
			lastGameMessageCounter[messageType] = duration;
			if (combatWindows[game->singlePlayerNumber] != NULL) 
			{
				if (messageType == MESSAGE_TYPE_HINT)
				{
					combatWindows[game->singlePlayerNumber]->showHintMessage(message);
				} else {
					if (messageType == MESSAGE_TYPE_CENTER_BIG)
					{
						combatWindows[game->singlePlayerNumber]->showCenterMessage(message);
					} else {
						if (messageType == MESSAGE_TYPE_EXECUTE_TIP)
						{
							combatWindows[game->singlePlayerNumber]->showExecuteTipMessage(message);
						} else {
							if (messageRightSide)
							{
								combatWindows[game->singlePlayerNumber]->showMessage(message, image, true);
							} else {
								combatWindows[game->singlePlayerNumber]->showMessage(message, image, false);
							}
						}
					}
				}
			}
		}
	}

	bool GameUI::isLocalPlayerDirectControlOn(int control, Unit *unit)
	{
		int clnum = getClientNumberForUnit(unit);
		if (clnum != -1)
		{
			if (gameController[clnum]->isKeyDown(DH_CTRL_WEAPON_SELECT_MODE))
			{
				return false;
			}

			// joystick hack
			if (gameController[clnum]->controllerTypeHasJoystick()
				&& control >= DIRECT_CTRL_FORWARD
				&& control <= DIRECT_CTRL_RIGHT)
			{
				// get joystick movement values
				int x = 0, y = 0;
				gameController[clnum]->getJoystickValues(&x, &y, NULL, NULL);

				// if not in free camera mode we should move relative to world
				// so we need to "unspin" x and y with player rotation
				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
				{
					// calculate angle of joystick
					float angle = 0.0f;
					if (x != 0 || y != 0)
					{
						VC2 dir(x, y);
						dir.Normalize();
						if (x > 0)
						{
							//  0 < angle < 180
							angle = ((M_PI * 0.5) + asinf(dir.y)) * 180.0 / M_PI;
						} else {
							// 180 < angle < 360
							angle = (2*M_PI + -(M_PI * 0.5 + asinf(dir.y))) * 180.0 / M_PI;
						}

						LOG_DEBUG(strPrintf("joystick x y angle: %d %d %f", x, y, angle).c_str());
						// add player unit angle
						angle = -angle + unit->getRotation().y;

						// angle += unit->getRotation().y;

						// TODO: calculate new x and y
						// angle is in degrees, goes clockwise and 0 is up
						float radAngle = -(M_PI / 2 + angle * 2 * M_PI / 360.f);

						x = 1000 * cosf(radAngle);
						y = 1000 * sinf(radAngle);
						LOG_DEBUG(strPrintf("unit rot: %f  new angle: %f", unit->getRotation().y, angle).c_str());
					}
				}

				// do movement
				if (x != 0 || y != 0)
				{
					// this is not joystick dead zone
					// because this is applied after rotation, normalization etc.
					// FIXME: still an ugly hack
					// should be made configurable
					const int MAGIC_THRESHOLD = 200;
				LOG_DEBUG(strPrintf("movement x y: %d %d", x, y).c_str());
				switch(control)
				{
				case DIRECT_CTRL_FORWARD:
					return (upMovementEnabled[clnum] && y < -MAGIC_THRESHOLD);
					break;

				case DIRECT_CTRL_BACKWARD:
					return (downMovementEnabled[clnum] && y > MAGIC_THRESHOLD);
					break;

				case DIRECT_CTRL_LEFT:
					return (leftMovementEnabled[clnum] && x < -MAGIC_THRESHOLD);
					break;

				case DIRECT_CTRL_RIGHT:
					return (rightMovementEnabled[clnum] && x > MAGIC_THRESHOLD);
					break;

				}
				}
			}

			switch(control)
			{
			case DIRECT_CTRL_FORWARD:
				if (!upMovementEnabled[clnum])
					return false;
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_FORWARD))
					return true;
				break;
			case DIRECT_CTRL_BACKWARD:
				if (!downMovementEnabled[clnum])
					return false;
				// NEW HACK: if forward pressed, ignore backward.
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_FORWARD))
					return false;
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_BACKWARD))
					return true;
				break;
			case DIRECT_CTRL_LEFT:
				if (!leftMovementEnabled[clnum])
					return false;
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT))
					return true;
				break;
			case DIRECT_CTRL_RIGHT:
				if (!rightMovementEnabled[clnum])
					return false;
				// NEW HACK: if left pressed, ignore right.
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT))
					return false;
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT))
					return true;
				break;
			case DIRECT_CTRL_TURN_RIGHT:
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_ROTATE_RIGHT)
					|| rightDirectRotation)
					return true;
				break;
			case DIRECT_CTRL_TURN_LEFT:
				if (gameController[clnum]->isKeyDown(DH_CTRL_CAMERA_MOVE_ROTATE_LEFT)
					|| leftDirectRotation)
					return true;
				break;
			case DIRECT_CTRL_FIRE:
				{
				if(this->firstPerson[clnum] != NULL)
				{
					// if switched weapon
					int weapNum = this->firstPerson[clnum]->getSelectedWeapon();
					if(weapNum != lastPrimaryWeapon[clnum])
					{
						// fire button must be released once
						if(gameController[clnum]->isKeyDown(DH_CTRL_ATTACK))
						{
							return false;
						}
					}
					lastPrimaryWeapon[clnum] = weapNum;
				}
				if (gameController[clnum]->isKeyDown(DH_CTRL_ATTACK)
					&& !msgBoxIsOpen)
				{
					// TODO: this should be refactored elsewhere (adds GameUI's dependecy to Weapon)
					if (this->firstPerson[clnum] != NULL)
					{
						int weapNum = this->firstPerson[clnum]->getSelectedWeapon();
						if (weapNum != -1
							&& this->firstPerson[clnum]->getWeaponType(weapNum) != NULL
							&& this->firstPerson[clnum]->getWeaponType(weapNum)->doesFireByClick())
						{
							if (fireKeyDownPreviously[clnum])
							{
								return false;
							}
						}
					}

					// if reloading shotgun, requires the player to click the button,
					// holding it down is not sufficient
					if (this->firstPerson[clnum] == NULL
						|| !this->firstPerson[clnum]->doesKeepReloading()
						|| gameController[clnum]->wasKeyClicked(DH_CTRL_ATTACK))
					{
						return true;
					}
				}
				}
				break;
			case DIRECT_CTRL_FIRE_SECONDARY:
				{
				Weapon *secondaryWType = NULL;
				if(this->firstPerson[clnum] != NULL)
				{
					// get secondary weapon type
					int weapNum = this->firstPerson[clnum]->getSelectedWeapon();
					if (weapNum >= 0) {
						Weapon *wType = this->firstPerson[clnum]->getWeaponType(weapNum);
						if (wType != NULL)
						{
							secondaryWType = wType->getAttachedWeaponType();
						}
					}

					// if switched weapon
					weapNum = -1;
					if (secondaryWType != NULL)
					{
						weapNum = this->firstPerson[clnum]->getWeaponByWeaponType(secondaryWType->getPartTypeId());
					}
					if(weapNum != lastSecondaryWeapon[clnum])
					{
						// fire button must be released once
						if(gameController[clnum]->isKeyDown(DH_CTRL_ATTACK_SECONDARY))
						{
							return false;
						}
					}
					lastSecondaryWeapon[clnum] = weapNum;
				}
				if (gameController[clnum]->isKeyDown(DH_CTRL_ATTACK_SECONDARY)
					&& !msgBoxIsOpen)
				{
					// TODO: this should be refactored elsewhere (adds GameUI's dependecy to Weapon)
					if (this->firstPerson[clnum] != NULL)
					{
						//int weapNum = this->firstPerson[clnum]->getSelectedSecondaryWeapon();
						// FIXME: selected secondary is not normally set so the above bugs.
						if (secondaryWType != NULL && secondaryWType->doesFireByClick())
						{
							if (fireSecondaryKeyDownPreviously[clnum])
							{
								return false;
							}
						}
					}

					// if reloading shotgun, requires the player to click the button,
					// holding it down is not sufficient
					if (this->firstPerson[clnum] == NULL
						|| !this->firstPerson[clnum]->doesKeepReloading()
						|| gameController[clnum]->wasKeyClicked(DH_CTRL_ATTACK_SECONDARY))
					{
						return true;
					}
				}
				}
				break;
			case DIRECT_CTRL_FIRE_GRENADE:
				if (this->firstPerson[clnum] == NULL || this->firstPerson[clnum]->getLaunchSpeed() >= 0.0f)
				{
					if (gameController[clnum]->isKeyDown(DH_CTRL_GRENADE)
						&& !msgBoxIsOpen)
					{
						return true;
					}
				}
				// hack: grenade key must be released after switching weapon
				else if(this->firstPerson[clnum] != NULL && this->firstPerson[clnum]->getLaunchSpeed() < 0.0f)
				{
					if(!gameController[clnum]->isKeyDown(DH_CTRL_GRENADE))
					{
						this->firstPerson[clnum]->setLaunchSpeed(0.0f);
					}
					return false;
				}
				break;
			case DIRECT_CTRL_SPECIAL_MOVE:
				if (gameController[clnum]->isKeyDown(DH_CTRL_SPECIAL_MOVE)
					&& !msgBoxIsOpen)
					return true;
				break;
			default:
				Logger::getInstance()->warning("GameUI::isDirectControl - Unknown control."); 
				break;
			}
			return false;
		} else {
			return false;
		}
	}

	int GameUI::getClientNumberForUnit(Unit *unit)
	{
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if (unit == firstPerson[i])
				return i;
		}

		return -1;
	}

	Unit *GameUI::getFirstPerson(int clientNumber)
	{
		assert(clientNumber >= 0 && clientNumber < MAX_PLAYERS_PER_CLIENT);

		return firstPerson[clientNumber];
	}

	ICameraSystem *GameUI::getCameraSystem()
	{
		return cameraSystem;
	}


	bool GameUI::isCursorActive(int player)
	{
		if (combatWindows[game->singlePlayerNumber] != NULL)
		{
			if (!forceCursorVisible && !combatWindows[game->singlePlayerNumber]->isGUIVisible())
				return false;
		}

		if (player == game->singlePlayerNumber && firstPerson[0] != NULL && !msgBoxIsOpen
			&& !game->isTacticalMode())
		{
			if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
				if (!thirdPersonView)
					return false;
			} else {
				return false;
			}
		}
		return true;
	}

	bool GameUI::isThirdPersonView(int player)
	{
		if (player == game->singlePlayerNumber)
			return thirdPersonView;
		else
			return false;
	}

	bool GameUI::isControlModeDirect(int player)
	{
		if (player == game->singlePlayerNumber)
			return controlModeDirect;
		else
			return false;
	}

	void GameUI::setCameraRange(float range)
	{
#ifdef LEGACY_FILES
		if (range < 20.0f)
			cameraRange = 20.0f;
#else
		if (range < 5.0f)
			cameraRange = 5.0f;
#endif
		else if (range > 2500.0f)
			cameraRange = 2500.0f;
		else
			cameraRange = range;

		if (this->renderTerrain != NULL)
		{
			scene->GetCamera()->SetVisibilityRange(cameraRange);
		}
	}

	void GameUI::restoreCameraRange()
	{
		cameraRange = (float)SimpleOptions::getInt(DH_OPT_I_CAMERA_RANGE);

		if (this->renderTerrain != NULL)
		{
			float visRange = cameraRange;
			if (visRange > this->renderTerrain->getCameraRange() && this->renderTerrain->getCameraRange() > 0)
				visRange = this->renderTerrain->getCameraRange();
			scene->GetCamera()->SetVisibilityRange((float)visRange);
		}
	}

	float GameUI::getCameraRange()
	{
		return cameraRange;
	}

	MusicPlaylist *GameUI::getMusicPlaylist(int player)
	{
		return musicPlaylist;
	}

	AmbientAreaManager *GameUI::getAmbientAreaManager()
	{
		return ambientAreaManager;
	}

	void GameUI::setGUIVisibility(int player, bool visible)
	{
		if (combatWindows[player] != NULL)
		{
			combatWindows[player]->setGUIVisibility(visible);
		}
		// disable controls too if gui not visible.
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			getController(i)->setControlsEnabled(visible);
		}
#ifdef PROJECT_SURVIVOR
		if (vehicleWindow != NULL)
		{
			if(!visible) vehicleWindow->hide();
			if(visible) vehicleWindow->show();
		}
#endif
	}


	void GameUI::setControlsEnabled(int player, bool enabled)
	{
		// FIXME: if gui disabled too, may conflict with control disabling
		// both change controls enabled/disabled without considering the 
		// other...
		// disable controls too if gui not visible.
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			getController(i)->setControlsEnabled(enabled);
		}
	}


	bool GameUI::pushUIState()
	{
		bool ret = true;
		int id = 0;
		if (!uiStateStack->isEmpty())
		{
			UIState *tmp = (UIState*)uiStateStack->peekLast();
			id = tmp->id + 1;

			char str[1024];
			sprintf(str, "GameUI::pushUIState - State stack is not empty (multiple states stored), storing as ID %i", id);
			Logger::getInstance()->error(str);
			ret = false;
		}
		// TODO: push camera range, light fadeout/cull range, etc.
		UIState *tmp = new UIState();
		tmp->id = id;
		tmp->camera = GAMEUI_CAMERA_NORMAL;
		tmp->cameraTimeFactor = cameraTimeFactor;
		tmp->cameraFOV = gameCamera->getFOV();
		tmp->cameraTargDist = gameCamera->getTargetDistance();
		for (int i = 0; i < GAMEUI_CAMERA_AMOUNT; i++)
		{
			if (gameCamera == cameras[i])
			{
				tmp->camera = i;
				break;
			}
		}
		tmp->guiVisible = true;
		if (combatWindows[game->singlePlayerNumber] != NULL)
		{
			tmp->guiVisible = combatWindows[game->singlePlayerNumber]->isGUIVisible();
		}
		uiStateStack->append(tmp);
		return ret;
	}

	bool GameUI::popUIState()
	{
		bool ret = true;
		if (!uiStateStack->isEmpty())
		{
			UIState *tmp = (UIState *)uiStateStack->popLast();
			if(tmp->id != 0)
			{
				char str[1024];
				sprintf(str, "GameUI::popUIState - State stack has multiple states stored, popped state ID %i", tmp->id);
				Logger::getInstance()->error(str);
				ret = false;
			}

			if (combatWindows[game->singlePlayerNumber] != NULL)
			{
				combatWindows[game->singlePlayerNumber]->setGUIVisibility(tmp->guiVisible);
			}
			for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				getController(i)->setControlsEnabled(tmp->guiVisible);
			}

			// TODO: pop camera range, light fadeout/cull range, etc.
			gameCamera = cameras[tmp->camera];
			cameraTimeFactor = tmp->cameraTimeFactor;
			gameCamera->setFOV(tmp->cameraFOV);
			gameCamera->setTargetDistance(tmp->cameraTargDist);
			delete tmp;

			ogui->setCursorScreenX(0, AIM_UP_CURSOR_X);
			ogui->setCursorScreenY(0, AIM_UP_CURSOR_Y);

		} else {
			Logger::getInstance()->error("GameUI::popUIState - State stack is empty.");
			ret = false;
		}
		return ret;
	}

	void GameUI::setCameraTimeFactor(float factor)
	{
		cameraTimeFactor = factor;
	}

	float GameUI::getCameraTimeFactor()
	{
		return cameraTimeFactor;
	}

	void GameUI::setUIPauseState(bool paused)
	{
		for(unsigned int i = 0; i < loopingSoundEffectHandles.size(); i++)
		{
			this->soundMixer->setSoundPaused(loopingSoundEffectHandles[i], paused);
		}
		this->soundLooper->setSoundsPaused(paused, true);
		scene->SetPauseState(paused);

#ifdef PROJECT_SURVIVOR
		if(paused)
		{
			game->gameUI->getStorm3D()->setGlobalTimeFactor(1.0f);
			game->gameUI->setSoundFrequencyFactor(1.0f);
			Timer::setTimeFactor(1.0f);
		}
		else
		{
			game->gameUI->getStorm3D()->setGlobalTimeFactor(originalTimeFactor);
			game->gameUI->setSoundFrequencyFactor(originalTimeFactor);
			Timer::setTimeFactor(originalTimeFactor);
		}
#endif
	}

	SelectionBox *GameUI::getSelectionBox()
	{
		if (combatWindows[game->singlePlayerNumber] == NULL)
			return NULL;
		return combatWindows[game->singlePlayerNumber]->getSelectionBox();
	}

	void GameUI::setTacticalClickExpected(int player, int cursorType)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->setTacticalClickExpected(cursorType);
	}

	void GameUI::addHostileUnitPointer(int player, Unit *unit)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->addHostileUnitPointer(unit);
	}

	void GameUI::removeHostileUnitPointer(int player, Unit *unit)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->removeHostileUnitPointer(unit);
	}

	void GameUI::doUnitClick(int player, Unit *unit)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->doUnitClick(unit);
	}

	void GameUI::setUnitHighlight(int player, Unit *unit)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->setUnitHighlight(unit);		
	}

	void GameUI::setTerrainHighlight(int player, VC3 &position)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->setTerrainHighlight(position);
	}

	void GameUI::clearHighlight(int player)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->clearHighlight();		
	}

	void GameUI::lockHighlight(int player)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->lockHighlight(); 	
	}

	void GameUI::unlockHighlight(int player)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->unlockHighlight(); 	
	}

	void GameUI::setErrorWindow(ErrorWindow *errorWin)
	{
		this->errorWindow = errorWin;

		assert(this->console == NULL);
		this->console = new GameConsole(errorWin, game->gameScripting);
	}

	GameConsole *GameUI::getConsole()
	{
		return this->console;
	}

	void GameUI::readKey(char ascii, int keycode, 
		const char *keycodeName)
	{
		assert(keyreaderId != -1);
		assert(errorWindow != NULL);

		if (strcmp(keycodeName, "esc") == 0)
		{
			console->cancel();
			console->hide();
		}
		else if (strcmp(keycodeName, "tab") == 0)
		{
			console->tab();
		}
		else if (strcmp(keycodeName, "left") == 0)
		{
			console->prevChar();
		}
		else if (strcmp(keycodeName, "right") == 0)
		{
			console->nextChar();
		}
		else if (strcmp(keycodeName, "backspace") == 0)
		{
			console->erasePrev();
		}
		else if (strcmp(keycodeName, "delete") == 0)
		{
			console->eraseNext();
		}
		else if (strcmp(keycodeName, "enter") == 0)
		{
			console->enter();
		}
		else if (strcmp(keycodeName, "up") == 0)
		{
			console->prevHistory();
		}
		else if (strcmp(keycodeName, "down") == 0)
		{
			console->nextHistory();
		}
		else if (ascii != 0)
		{
			console->add(ascii);
		}
	}


	void GameUI::setScrollyEnabled(bool enabled)
	{
		if (!SimpleOptions::getBool(DH_OPT_B_CAMERA_SCROLLY_ENABLED))
			enabled = false;
		this->scrollyEnabled = enabled;
	}

	void GameUI::setScrollyTemporarilyDisabled(bool disabled)
	{
		if(disabled && this->scrollyEnabled)
		{
			this->scrollyEnabled = false;
			this->scrollyTemporarilyDisabled = true;
		}
		else if(this->scrollyTemporarilyDisabled)
		{
			this->scrollyEnabled = true;
			this->scrollyTemporarilyDisabled = false;
		}
	}


	void GameUI::hideConsole()
	{
		//console->cancel();
		console->hide();
		if (keyreaderId != -1)
		{
			gameController[0]->removeKeyreader(keyreaderId);
			keyreaderId = -1;
		}
	}


	void GameUI::showConsole()
	{
		console->show();
		if (keyreaderId == -1)
		{
			// HACK!!!!
			keyreaderId = gameController[0]->addKeyreader(this);
		}
	}


	ui::AmbientSoundManager* GameUI::getAmbientSoundManager() 
	{
		return ambientSoundManager;
	}


	void GameUI::setCamerasWaterManager()
	{
		for (int i = 0; i < GAMEUI_CAMERA_AMOUNT; i++)
		{
			cameras[i]->setWaterManager(game->waterManager);
		}
	}


	void GameUI::setLeftDirectRotation(bool rotationOn)
	{
		leftDirectRotation = rotationOn;
	}

	void GameUI::setRightDirectRotation(bool rotationOn)
	{
		rightDirectRotation = rotationOn;
	}

	void GameUI::setFirstPerson(int player, Unit *unit, int clientNumber)
	{
		if (firstPerson[clientNumber] != NULL)
		{
			firstPerson[clientNumber]->setDirectControl(false);
			if (firstPerson[clientNumber]->getUnitType()->isDirectControl())
				firstPerson[clientNumber]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_AI);
			else
				firstPerson[clientNumber]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_NONE);

			if (joystickAimer[clientNumber] != NULL)
			{
				delete joystickAimer[clientNumber];
				joystickAimer[clientNumber] = NULL;
			}

			// must remove the pointer when quitting
			firstPerson[clientNumber]->targeting.setTarget(NULL);
			firstPerson[clientNumber]->targeting.clearTarget();

			UnitActor *ua = getUnitActorForUnit(firstPerson[clientNumber]);
			ua->stopUnit(firstPerson[clientNumber]);
		}

		if(unit && unit->getVisualObject())
			unit->getVisualObject()->setPerFrameInterpolation(true);

		firstPerson[clientNumber] = unit;
		if (!game->isTacticalMode() && unit != NULL )
		{
			firstPerson[clientNumber]->setDirectControl(true);
			firstPerson[clientNumber]->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER);
			combatWindows[game->singlePlayerNumber]->setCrosshair(true);
			if (!thirdPersonView)
			{
				if (firstPerson[clientNumber]->getVisualObject() != NULL)
					firstPerson[clientNumber]->getVisualObject()->setVisible(false);
			}
		}
		cameras[GAMEUI_CAMERA_NORMAL]->setFirstPersonMode(true);
	}

	void GameUI::nextInterfaceGeneration()
	{
		Timer::update();
		int recreateStartTime = Timer::getTime();

		if (lightningVisualEffect != NULL)
		{
			lightningVisualEffect->setDeleteFlag();
			lightningVisualEffect->freeReference();
			lightningVisualEffect = NULL;
		}

		assert(game->getEnvironmentalEffectManager() != NULL);
		delete game->getEnvironmentalEffectManager();
		game->setEnvironmentalEffectManager(NULL);

		visualEffectManager->freeParticleEffects();
		delete visualEffectManager;

		delete this->terrainCreator;

		deleteCameras();

		game->clearVisualObjectModelStorage();

		reloadPartTypeVisuals();

		assert(ui::visual_object_allocations == 0);
		assert(ui::visual_object_model_allocations == 0);

		SHOW_LOADING_BAR(60);

		delete textureSwitcher;
		delete textureCache;

		oguiStormDriver->prepareForNextStormGeneration(this->scene);

		oguiStormDriver->deleteTextureCache();
		lipsyncManager.reset();

		storm3d->Empty();

		textureCache = new frozenbyte::TextureCache(*storm3d);
		textureSwitcher = new util::TextureSwitcher(*textureCache);

		VisualObject::setTextureCache(this->textureCache);

		this->scene = storm3d->CreateNewScene();
		//scene->SetBackgroundColor(bgCol);

		ui::VisualObjectModel::setVisualStorm(storm3d, this->scene);

		oguiStormDriver->nextStormGeneration(this->scene);
		ogui->ResetData();

		LoadingMessage::setManagers(storm3d, this->scene, ogui);

		::disposable_scene = this->scene;

		SHOW_LOADING_BAR(99);

		ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
		
		this->terrainCreator = new TerrainCreator(storm3d, scene);

		visualEffectManager = new VisualEffectManager(storm3d, scene);
		visualEffectManager->loadParticleEffects();

		assert(game->getEnvironmentalEffectManager() == NULL);
		EnvironmentalEffectManager *enveffman = new EnvironmentalEffectManager(game, visualEffectManager);
		game->setEnvironmentalEffectManager(enveffman);

		createCameras();

		Timer::update();
		int recreateEndTime = Timer::getTime();
		int recreateTotalTime = recreateEndTime - recreateStartTime;
		Logger::getInstance()->debug("GameUI::nextInterfaceGeneration - Interface recreate done, time used (msec) follows:");
		Logger::getInstance()->debug(int2str(recreateTotalTime));

		SHOW_LOADING_BAR(100);
		//this->effects->startFadeIn(500);

	}

	void GameUI::setOguiStormDriver(OguiStormDriver *driver)
	{
		this->oguiStormDriver = driver;
	}

	OguiStormDriver *GameUI::getOguiStormDriver()
	{
		return this->oguiStormDriver;
	}

	void GameUI::setUnitsChangedFlag(int player)
	{
		if (combatWindows[player] != NULL)
			combatWindows[player]->createUnitWindows();
	}

	void GameUI::setEnvironmentLightning(VC3 &fromPosition)
	{
		// TODO: move to environmentEffects class or something??

		if (!SimpleOptions::getBool(DH_OPT_B_WEATHER_EFFECTS))
		{
			return;
		}

		if (lightningVisualEffect == NULL)
		{
			if (firstPerson[0] != NULL)
			{
				VC3 playerPosition = firstPerson[0]->getPosition();
				VC3 position = fromPosition;
				VC3 posDiff = position - playerPosition;
				float posDiffLenSq = posDiff.GetSquareLength();

				bool areaOk = false;
				int tryNum = 0;

				IStorm3D_Model *model = NULL;

				while (!areaOk)
				{
					tryNum++;

					/*
					if (posDiffLenSq < 24 * 24 || posDiffLenSq > 30 * 30)
					{
						posDiff.Normalize();
						posDiff *= 25.0f; 
						position = playerPosition + posDiff;
						posDiffLenSq = posDiff.GetSquareLength();
					}
					*/
					// Damn.. problems with the spotlight if angle is "any"..
					// must be 0,90,180,270 to work properly..

					if (fabs(posDiff.x) > fabs(posDiff.z))
					{
						if (posDiff.x < 0)
						{
							posDiff = VC3(-25.0f, 0, ((rand() % 100) - 50) / 10.0f);
						} else {
							posDiff = VC3(25.0f, 0, ((rand() % 100) - 50) / 10.0f);
						}
					} else {
						if (posDiff.z < 0)
						{
							posDiff = VC3(((rand() % 100) - 50) / 10.0f, 0, -25.0f);
						} else {
							posDiff = VC3(((rand() % 100) - 50) / 10.0f, 0, 25.0f);
						}
					}
					/*
					if (fabs(posDiff.x) < fabs(posDiff.z))
					{
						if (posDiff.x < 0)
						{
							posDiff.x = -25.0f;
						} else {
							posDiff.x = 25.0f;
						}
					} else {
						if (posDiff.z < 0)
						{
							posDiff.z = -25.0f;
						} else {
							posDiff.z = 25.0f;
						}
					}
					*/
					position = playerPosition + posDiff;
					posDiffLenSq = posDiff.GetSquareLength();

					// TODO: check that light not inside building.

					int px = game->gameMap->scaledToPathfindX(position.x);
					int py = game->gameMap->scaledToPathfindY(position.z);
			
					model = game->getGameScene()->getBuildingModelAtPathfind(px, py);

					if (model == NULL)
					{
						px += 3;
						model = game->getGameScene()->getBuildingModelAtPathfind(px, py);
						if (model == NULL)
						{
							px -= 6;
							model = game->getGameScene()->getBuildingModelAtPathfind(px, py);
							px += 3;
						}
					}
					if (model == NULL)
					{
						py += 3;
						model = game->getGameScene()->getBuildingModelAtPathfind(px, py);
						if (model == NULL)
						{
							py -= 6;
							model = game->getGameScene()->getBuildingModelAtPathfind(px, py);
							py += 3;
						}
					}

					areaOk = true;
					if (model != NULL)
					{
						if (tryNum < 4)
						{
							areaOk = false;
							if (tryNum == 1 || tryNum == 3)
							{
								posDiff.x = -posDiff.x;
								posDiff.z = -posDiff.z;
							} else {
								float tmp = posDiff.z;
								posDiff.x = posDiff.z;
								posDiff.z = tmp;
							}
						}
					}
				}

				if (model == NULL)
//					&& posDiffLenSq >= 24 * 24 && posDiffLenSq <= 30 * 30)
				{
					float angle = util::PositionDirectionCalculator::calculateDirection(playerPosition, position);
					VC3 rotation = VC3(0,angle,0);

					lightningVisualEffect =
						visualEffectManager->createNewVisualEffect(
						visualEffectManager->getVisualEffectIdByName("envlightning"), 
						NULL, NULL, position, position, rotation, VC3(0,0,0), game);

					if (lightningVisualEffect != NULL)
					{
						lightningVisualEffect->addReference();

						lightningTime = 150;
					} else {
						Logger::getInstance()->error("GameUI::setEnvironmentLightning - No envlightning visual effect found.");
					}
				}

				// TODO: should make the flash less visible when further away..
				game->gameUI->getEffects()->startFlashEffect(ui::UIEffects::FLASH_EFFECT_TYPE_ENVIRONMENT_LIGHTNING);
			}
		}
	}

	void GameUI::setSoundFrequencyFactor(float freqFactor)
	{
		if (this->soundMixer != NULL)
		{
			soundMixer->setSoundFrequency(freqFactor);
		}
	}


	frozenbyte::TextureCache *GameUI::getTextureCache()
	{
		return textureCache;
	}

	util::TextureSwitcher *GameUI::getTextureSwitcher()
	{
		return textureSwitcher;
	}

	IStorm3D *GameUI::getStorm3D()
	{
		return storm3d;
	}

	ui::LightManager *GameUI::getLightManager()
	{
		return lightManager;
	}

	ui::DynamicLightManager *GameUI::getDynamicLightManager()
	{
		return dynamicLightManager;
	}

	ui::AniRecorderWindow *GameUI::getAniRecorderWindow()
	{
		return this->aniRecorderWindow;
	}

	bool GameUI::isLoadingWindowVisible()
	{
		if (loadingWindow != NULL)
		{
			if (!loadingWindow->isFadingOut())
			{
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

	void GameUI::redrawLoadingWindow(int player)
	{
		if(loadingWindow)
		{
			loadingWindow->reloadWindows();

			// this looks like a bad idea...
			ogui->Run(GAME_TICK_MSEC);
			// this would be better (to just draw the windows not cursors, and no events...)
			//ogui->DrawScreen();
			scene->RenderScene();
			game->gameUI->setGUIVisibility(game->singlePlayerNumber, false);
		}
	}

	int GameUI::getCursorScreenX(int clientNumber, bool exact)
	{
		return ogui->getCursorScreenX(clientNumber, exact);
	}

	int GameUI::getCursorScreenY(int clientNumber, bool exact)
	{
		return ogui->getCursorScreenY(clientNumber, exact);
	}

	bool GameUI::isAnyIngameWindowVisible()
	{
		if (this->isLoadingWindowVisible())
			return true;

		if (characterSelectionWindow)
			return true;

#ifdef GUI_BUILD_MAP_WINDOW
		if(mapWindow && mapWindow->isVisible())
			return true;
#endif

		if (this->upgradeWindow != NULL)
			return true;
		
		if( commandWindows[ 0 ] != NULL ) 
			return true;
		
#ifdef GUI_BUILD_LOG_WINDOW
		if( logWindow != NULL )
			return true;
#endif

		if( terminalManager != NULL && terminalManager->isWindowOpen() )
			return true;

		if( scoreWindow != NULL )
			return true;

		if( missionSelectionWindow != NULL )
			return true;

		if( missionFailureWindow != NULL )
			return true;

		return false;
	}

	VC3 GameUI::getListenerPosition()
	{
		return listenerPosition;
	}

	void GameUI::playStreamedSound(const char *filename)
	{
		if (soundMixer != NULL)
		{
			soundMixer->playStreamedSound(filename);
		}
	}

	void GameUI::stopAllStreamedSounds()
	{
		if (soundMixer != NULL)
		{
			soundMixer->stopStreamedSounds();
		}
	}

#ifdef GUI_BUILD_MAP_WINDOW
	MapWindow *GameUI::getMapWindow()
	{
		return this->mapWindow;
	}
#endif

	void GameUI::forceBuildingRoofHide()
	{
		this->buildingHandler.setUpdateEnabled(false);
		this->buildingHandler.hideAllRoofs();
	}

	void GameUI::forceBuildingRoofShow()
	{
		this->buildingHandler.setUpdateEnabled(false);
		this->buildingHandler.showAllRoofs();
	}

	void GameUI::endForcedBuildingRoof()
	{
		this->buildingHandler.setUpdateEnabled(true);
	}

	int GameUI::playGUISound(const char *filename, int relativeVolume)
	{
		VC3 campos = gameCamera->getActualInterpolatedPosition();
		if (relativeVolume > 100)
			relativeVolume = 100;
		if (relativeVolume < 0)
			relativeVolume = 0;
		return this->playSoundEffect(filename, campos.x, campos.y, campos.z, false, DEFAULT_SOUND_EFFECT_VOLUME * relativeVolume / 100, 999, DEFAULT_SOUND_PRIORITY_HIGH);
	}

	// added by Pete
	int GameUI::playGUISpeech( const char* filename, int relativeVolume )
	{
		VC3 campos = gameCamera->getActualInterpolatedPosition();
		if (relativeVolume > 100)
			relativeVolume = 100;
		if (relativeVolume < 0)
			relativeVolume = 0;
		
		return this->playSpeech( filename, campos.x, campos.y, campos.z, false, DEFAULT_SOUND_EFFECT_VOLUME * relativeVolume / 100 );
	}

	void GameUI::updateUnitLighting(bool onlyNearPlayer)
	{
		VC3 nearPosition = VC3(0,0,0);
		float range = 100000.0f;

		if (onlyNearPlayer)
		{
			// TODO: use camera position instead of player if not direct control?

			if (firstPerson[0] != NULL)
			{
				nearPosition = firstPerson[0]->getPosition();
				range = 50.0f;
			}
		}

		for (int p = 0; p < ABS_MAX_PLAYERS; p++)
		{
			IUnitListIterator *iter = game->units->getNearbyOwnedUnits(p, nearPosition, range);
			while (iter->iterateAvailable())
			{
				Unit *u = iter->iterateNext();
				u->setLastLightUpdatePosition(VC3(-99999, -99999, -99999));
			}
			delete iter;
		}
	}

	void GameUI::setPlayerSelfIlluminationEnabled(bool enabled)
	{
		this->playerSelfIllumEnabled = enabled;
	}


	util::LipsyncManager *GameUI::getLipsyncManager()
	{
		if(lipsyncManager)
			return lipsyncManager.get();
		else
			return 0;
	}

	void GameUI::createBuildingLighting()
	{
		// update pointlights to buildings...
		{
			assert(lightManager != NULL);
			LinkedListIterator builditer(game->buildings->getAllBuildings());
			while (builditer.iterateAvailable())
			{
				Building *b = (Building *)builditer.iterateNext();
				lightManager->setBuildingLights(*b->getVisualObject()->getStormModel());
			}
		}
	}

	void GameUI::setFiresPreviously(Unit *unit, bool primaryPressed, bool secondaryPressed)
	{
		int clnum = getClientNumberForUnit(unit);
		if (clnum != -1)
		{
			//fireKeyDownPreviously[clnum] = primaryPressed;
			//fireSecondaryKeyDownPreviously[clnum] = secondaryPressed;

			fireKeyDownPreviously[clnum] = gameController[clnum]->isKeyDown(DH_CTRL_ATTACK);
			fireSecondaryKeyDownPreviously[clnum] = gameController[clnum]->isKeyDown(DH_CTRL_ATTACK_SECONDARY);

		} else {
			Logger::getInstance()->error("GameUI::setFiresPreviously - Unit has no client number.");
		}
	}

	void GameUI::setConversationNoise(int index, int value)
	{
		combatWindows[game->singlePlayerNumber]->setConversationNoise(index, value);
	}

	VC3 GameUI::getOcclusionCheckPosition()
	{
		return gameCamera->getPosition();
	}

	void GameUI::openVehicleGUI(const char *params)
	{
#ifdef PROJECT_SURVIVOR
		if(vehicleWindow != NULL)
		{
			delete vehicleWindow;
		}
		vehicleWindow = new VehicleWindow(ogui, game, firstPerson[0], params);
#endif
	}

	void GameUI::closeVehicleGUI()
	{
#ifdef PROJECT_SURVIVOR
		if(vehicleWindow)
		{
			delete vehicleWindow;
			vehicleWindow = NULL;
		}
#endif
	}

	bool GameUI::isVehicleGUIOpen()
	{
		return vehicleWindow != NULL;
	}

	void GameUI::forceCursorVisibility(bool enabled)
	{
		forceCursorVisible = enabled;
	}

	void GameUI::openCharacterSelectionWindow(const char *params)
	{
#ifdef PROJECT_SURVIVOR
		// only active in coop
		if(params && strstr(params, "coop_only") && !game->isCooperative()) return;

		// still loading?
		if(loadingWindow && !loadingWindow->isCloseEnabled())
		{
			// show through loading window, kthx
			LoadingWindow::showCharacterSelection = true;
			CharacterSelectionWindow::parseChoices(params);
		}
		else if(characterSelectionWindow == NULL)
		{
			// otherwise create directly
			CharacterSelectionWindow::parseChoices(params);
			characterSelectionWindow = new CharacterSelectionWindow(ogui, game);
			characterSelectionWindow->raise();

			// pause game
			game->setPaused(true);
			// hide combat windows
			if(combatWindows && combatWindows[game->singlePlayerNumber])
				combatWindows[game->singlePlayerNumber]->startGUIModeTempInvisible();

			// hide co-op cursors
			ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
			for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				// except for players with mouse
				if(!gameController[i]->controllerTypeHasMouse())
				{
					ogui->SetCursorImageState(i, DH_CURSOR_INVISIBLE);
					ogui->setCursorScreenX(i, 0);
					ogui->setCursorScreenY(i, 0);
				}
			}
			
		}
#endif
	}

	void GameUI::closeCharacterSelectionWindow()
	{
#ifdef PROJECT_SURVIVOR
		// if loading window is open, close it too
		if(loadingWindow)
		{
			loadingWindow->closeWindow();
		}

		delete characterSelectionWindow;
		characterSelectionWindow = NULL;

		// unpause (except if we have upgradewindow open)
		if(upgradeWindow == NULL)
		{
			game->setPaused(false);
		}

		// show co-op cursors again
		combatWindows[game->singlePlayerNumber]->updateCursorImage();

		// should open upgrade windows?
		if(LoadingWindow::showUpgradeWindowOnClose)
		{
			// open the first upgrade window
			game->gameUI->openWindow( GameUI::WINDOW_TYPE_UPGRADE );
		}
		else
		{
			// otherwise show combat windows
			combatWindows[game->singlePlayerNumber]->endGUIModeTempInvisible();
		}
#endif
	}

	bool GameUI::isCharacterSelectionWindowOpen() const
	{
		if(characterSelectionWindow != NULL) return true;

		// queued after load
		if(loadingWindow != NULL && LoadingWindow::showCharacterSelection)
		{
			return true;
		}

		return false;
	}

	void GameUI::enableAlphaTestPass(bool enabled)
	{
		if(this->renderTerrain)
		{
			this->renderTerrain->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::AdditionalAlphaTestPass, enabled );
		}
	}

	void GameUI::setTimeFactor(float factor)
	{
		originalTimeFactor = factor;
		getStorm3D()->setGlobalTimeFactor(factor);
		setSoundFrequencyFactor(factor);
		Timer::setTimeFactor(factor);
	}

	void GameUI::openMissionFailureWindow()
	{
#ifdef PROJECT_SURVIVOR
		if(missionFailureWindow)
			return;

		if(combatWindows[game->singlePlayerNumber])
		{
			combatWindows[game->singlePlayerNumber]->clearMessage();
			combatWindows[game->singlePlayerNumber]->clearCenterMessage();
			combatWindows[game->singlePlayerNumber]->clearExecuteTipMessage();
			combatWindows[game->singlePlayerNumber]->clearHintMessage();
		}

		game->setPaused( true );
		effects->startFadeIn(500);
		missionFailureWindow = new MissionFailureWindow(ogui, game);
#endif
	}

	void GameUI::closeMissionFailureWindow()
	{
#ifdef PROJECT_SURVIVOR
		if(!missionFailureWindow)
			return;


		if(missionFailureWindow->shouldRestart())
		{
			bool load = true;

			// is coop
			if(game->isCooperative())
			{
				// apply settings
				if(!CoopMenu::prepareCoopGameRestart(game))
				{
					Logger::getInstance()->error("GameUI::closeMissionFailureWindow - Applying coop settings failed");
				}
			}

			if(load)
			{
				// try to reload game
				std::string save = game->lastSaveId;
				if(save.empty() || !game->loadGame(save.c_str()))
				{
					char str[1024];
					sprintf(str, "GameUI::closeMissionFailureWindow - Loading last savegame \"%s\" failed", save.c_str());
					Logger::getInstance()->error(str);
				}
			}
		}

		game->setPaused( false );
		delete missionFailureWindow;
		missionFailureWindow = NULL;
#endif
	}

	bool GameUI::isMissionFailureWindowOpen() const
	{
		return missionFailureWindow != NULL;
	}

	void GameUI::setMovieAspectRatio(bool enabled)
	{
		if(enabled)
		{
			this->renderTerrain->GetTerrain()->getRenderer().setMovieAspectRatio(true, true);

			// disable gui
			if(movieAspectRatioGUIVisible == -1)
			{
				movieAspectRatioGUIVisible = combatWindows[0] != NULL && combatWindows[0]->isGUIVisible() ? 1 : 0;
			}
			setGUIVisibility(0, false);
		}
		else
		{
			this->renderTerrain->GetTerrain()->getRenderer().setMovieAspectRatio(false, true);

			// return gui
			if(movieAspectRatioGUIVisible == 1)
				setGUIVisibility(0, true);
			else if(movieAspectRatioGUIVisible == 0)
				setGUIVisibility(0, false);

			movieAspectRatioGUIVisible = -1;
		}
	}

	int GameUI::getLastRunUITime() const
	{
		return lastRunUITime;
	}

	void GameUI::SwapCursorImages(int cursor1, int cursor2)
	{
		ogui->SwapCursorImages(cursor1, cursor2);
	}

	void GameUI::ResetSwappedCursorImages()
	{
		ogui->ResetSwappedCursorImages();
	}
}

