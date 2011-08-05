
#ifndef GAMEUI_H
#define GAMEUI_H

//
// The game user interface connectivity.
//
// No other game classes than this should depend on UI.
// This class provides the methods needed to change the user
// interface when the game needs to do that.
//
// The UI may modify the game via GameRequests.
//
// Neither UI nor game should directly call each others methods as that
// might result into problems with network game synchronization.
//

#include "gamedefs.h"

#include "gui_configuration.h"

#include "GameMap.h"
#include "../ui/GameCamera.h"
#include "../ui/IMessageBoxListener.h"
#include "../ui/IGameControllerKeyreader.h"
#include "../ui/LoadingWindow.h"
#include "../util/CursorRayTracer.h"
#include "../util/BuildingHandler.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>




#define GAMEUI_CAMERA_NORMAL 0
#define GAMEUI_CAMERA_TACTICAL 1
#define GAMEUI_CAMERA_CINEMATIC1 2
#define GAMEUI_CAMERA_CINEMATIC2 3

#define GAMEUI_CAMERA_AMOUNT 4



// NOTE: this value must equal the MESSAGE_TYPE enum last value + 1
#define MESSAGE_TYPES_AMOUNT 6


// HACK: some way to detect real errors versus sounds being too far.
enum GAMEUI_SOUND_EFFECT_ERRORCODE
{
	GAMEUI_SOUND_EFFECT_ERRORCODE_NONE,
	GAMEUI_SOUND_EFFECT_ERRORCODE_TOO_FAR,
	GAMEUI_SOUND_EFFECT_ERRORCODE_NO_SOUNDS,
	GAMEUI_SOUND_EFFECT_ERRORCODE_OTHER
};
extern GAMEUI_SOUND_EFFECT_ERRORCODE gameui_sound_effect_errorcode;



class Ogui;
class OguiStormDriver;
class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Terrain;
class CursorRayTracer;

namespace sfx {
	class SoundLooper;
	class SoundMixer;
	class MusicPlaylist;
	class AmbientAreaManager;
}


class Terrain;

namespace frozenbyte
{
	class TextureCache;
}

namespace util
{
	class TextureSwitcher;
	class LipsyncManager;
	class GridOcclusionCuller;
}

namespace ui
{
	class VisualEffectManager;
	class TerrainCreator;
	class ArmorConstructWindow;
	class MenuCollection;
	class StorageWindow;
	class LoadingWindow;
	class AniRecorderWindow;
	class CombatWindow;
	class UpgradeWindow;
	class GameController;
	class MessageBoxWindow;
	class Visual2D;
	class SelectionBox;
	class UIEffects;
	class PlayerUnitCameraBoundary;
	class ErrorWindow;
	class GameConsole;
	class AmbientSoundManager;
	class JoystickAimer;
	class VisualEffect;
	class LightManager;
	class DynamicLightManager;
#ifdef GUI_BUILD_MAP_WINDOW
	class MapWindow;
#endif
#ifdef GUI_BUILD_LOG_WINDOW
	class LogWindow;
#endif
	class LogManager;
#ifdef GUI_BUILD_INGAME_GUI_TABS
	class IngameGuiTabs;
#endif
	class TerminalManager;
	class CinematicScreen;
	class ICameraSystem;
	// for survivor
	class ScoreWindow;
	class MissionFailureWindow;
	class MissionSelectionWindow;
	class SurvivorUpgradeWindow;
	class VehicleWindow;
}


// TODO: using namespace in header - REMOVE!!!
using namespace ui;


namespace game
{

	class Game;
	class BuildingAdder;
	class Unit;


	class SceneSelection
	{
	public:
		SceneSelection() { hit=false; scaledMapX=0; scaledMapY=0; unit=NULL; };
		bool hit; // hit somewhere on map or some unit
		float scaledMapX; // map coordinates (of unit if not null, else just map)
		float scaledMapY;
		Unit *unit;
	};


	class GameUI : public IMessageBoxListener, ui::IGameControllerKeyreader
	{
	public:

		// NOTE: MESSAGE_TYPE_AMOUNT define must equal this enum's last value + 1
		enum MESSAGE_TYPE
		{
			MESSAGE_TYPE_NORMAL = 0,
			MESSAGE_TYPE_RADIO = 1,
			MESSAGE_TYPE_RADIO2 = 2,
			MESSAGE_TYPE_CENTER_BIG = 3,
			MESSAGE_TYPE_HINT = 4,
			MESSAGE_TYPE_EXECUTE_TIP = 5
		};

		enum WINDOW_TYPE
		{
			WINDOW_TYPE_MAP = 0,
			WINDOW_TYPE_UPGRADE = 1,
			WINDOW_TYPE_LOG = 2
		};

		GameUI(Ogui *ogui, Game *game, IStorm3D *s3d, IStorm3D_Scene *scene, sfx::SoundMixer *soundMixer);
		~GameUI();

		// by Pete
		IStorm3D_Scene* getStormScene() const { return scene; }

		float getVideoVolume() const;
		void detachVisualEffects();	

		void createCameras();
		void deleteCameras();

		void openArmorConstructWindow(int player);
		void closeArmorConstructWindow(int player);
		void refreshArmorConstructWindow(int player);

		void openCommandWindow(int player);
		void closeCommandWindow(int player);
		void startCommandWindow(int player);
		// unnecessary(?): void refreshCommandWindow(int player);

		// added by Pete to open main menu from game, when pressed esc
		void openMainmenuFromGame( int menu = 0 );

		// added by Pete to close main menu and resuma game 
		void resumeGame();


		void openStorageWindow(int player);
		void closeStorageWindow(int player);

		void openLoadingWindow(int player);
		void closeLoadingWindow(int player);
		void redrawLoadingWindow(int player);

		void doneLoading(int player);

		// scoreWindow methods by Pete to allow score window haxoring
		// tells us if a score window should open at the end of a level
		bool isScoreWindowInUse() const;	
		// tells us if a score window is visible
		bool isScoreWindowOpen() const;
		bool scoreWindowAllowsLoading() const;
		void openScoreWindow( int player );
		void closeScoreWindow( int player );

		void openMissionFailureWindow();
		void closeMissionFailureWindow();
		bool isMissionFailureWindowOpen() const;


		void openCharacterSelectionWindow(const char *params);
		void closeCharacterSelectionWindow();
		bool isCharacterSelectionWindowOpen() const;

		// added for survivor by Pete
		void openMissionSelectionWindow();
		void closeMissionSelectionWindow();
		MissionSelectionWindow* getMissionSelectionWindow() const;

		void openAniRecorderWindow(int player);
		void closeAniRecorderWindow(int player);

		void openWindow( WINDOW_TYPE type, int player = 0 );
		void closeWindow( WINDOW_TYPE type, bool save_changes = true, int player = 0);
#ifdef GUI_BUILD_INGAME_GUI_TABS
		void updateIngameTabs();
#endif

		bool openUpgradeWindow(Unit *unit);
		void prepareCloseUpgradeWindow(Unit *unit);
		void closeUpgradeWindow(Unit *unit);
#ifdef GUI_BUILD_MAP_WINDOW
		bool openMapWindow();
		void closeMapWindow();
#endif
#ifdef GUI_BUILD_LOG_WINDOW
		bool openLogWindow();
		void prepareCloseLogWindow();
		void closeLogWindow();
#endif

		void openTerminalWindow( const std::string& name );
		void closeTerminalWindow();

		void openCinematicScreen( const std::string& name );
		bool isCinematicScreenOpen() const;
		void closeCinematicScreen();

		void			openCombatWindow(int player, bool invisible = false);
		void			closeCombatWindow(int player);
		CombatWindow*	getCombatWindow( int player ) const;
		// MenuCollection*	getCommandWindow( int player ) const;

		void openArmorIncompleteConfirm(int player, bool notAnyArmor, bool incompleteArmor, bool noArmor, bool notPaid);

		virtual void messageBoxClosed(MessageBoxWindow *msgbox, int id, int choice);

		// TODO: get rid of this, should be done with request
		//bool readyToRockNRoll();

		// Sets the map (terrain) to render, NULLs for not to render any terrain
		void setRenderMap(GameMap *gameMap, char *configFile);

		// runs the game ui
		void runUI(int gameTimer);

		// tell the UI that a mission has just started
		void missionStarted();

		// cursor stuff
		void setCursorControllers(bool allowJoystick = true);

		// tell the UI that a mission has just ended
		void missionEnded();

		// returns true if the program should terminate
		bool isQuitRequested();

		// requests that the program should terminate
		void setQuitRequested();

		// Sets the background music. It's as simple as that.
		// NULL parameter means no music.
		//void setMusic(char *filename);

		// returns true if music is fading in or out
		//bool isMusicFading();

		// returns camera to be modified... (what about player number???)
		GameCamera *getGameCamera();

		// return the what's under players cursor
		SceneSelection *getSceneSelection(int clientNumber);

		// return the game controller for this _client_
		GameController *getController(int clientNumber);

		int getClientNumberForUnit(Unit *unit);

		VisualEffectManager *getVisualEffectManager();

		void setPointersChangedFlag(int player);

		void setUnitDamagedFlag(int player);

		void setUnitDestroyedFlag(int player);

		bool isAbortingMission();

		void setAbortingMission(bool abortFlag);

		void selectCamera(int camera);
		int getCameraNumber();

		// We need to access this later
		Terrain *getTerrain()
		{
			return renderTerrain;
		}

		bool isLoadingWindowVisible();

		bool isAnyIngameWindowVisible();

		int playGUISound(const char *filename, int relativeVolume = 100);
		
		// added by Pete, for the use of OptionsMenu
		int playGUISpeech( const char* filename, int relativeVolume = 100 ); 

		int playSoundEffect(const char *filename, float x, float y, float z, bool loop, int volume, float range, int priority, bool ambient = false);

		int parseSoundFromDefinitionString(const char *sounddef, 
			float x, float y, float z, bool *looped, int *handle, 
			int *key, bool continueOldSound, float range, int priority, bool muteVolume = false, bool ambient = false);

		int playSpeech(const char *filename, float x, float y, float z, bool loop, int volume, bool volume_adjust = true);

		void stopSound(int handle);

		void setSoundFrequencyFactor(float freqFactor);

		void preloadSound(const char *filename, bool temporaryCache);

		void cleanSoundCache(bool temporaryCache);

		void clearGameMessage(MESSAGE_TYPE messageType);
		void clearGameMessageDuration(MESSAGE_TYPE messageType);

		void gameMessage(const char *message, ui::Visual2D *imagefile = NULL, 
			int priority = 1, int duration = 4000, 
			MESSAGE_TYPE messageType = MESSAGE_TYPE_NORMAL);

		bool isLocalPlayerDirectControlOn(int control, Unit *unit);

		Unit *getFirstPerson(int clientNumber);

		ICameraSystem *getCameraSystem();

		bool isCursorActive(int player);

		bool isThirdPersonView(int player);

		bool isControlModeDirect(int player);

		// trace cursor coordinates to scene
		// return unit and terrain coords.
		SceneSelection cursorRayTrace(int x, int y, bool terrainOnly, bool accurate);
		SceneSelection cursorRayTracePlayer(int player, bool terrainOnly, bool accurate);

		void setCameraRange(float range);
		float getCameraRange();
		void restoreCameraRange();

		sfx::MusicPlaylist *getMusicPlaylist(int player);
		sfx::AmbientAreaManager *getAmbientAreaManager();
		sfx::SoundLooper *getSoundLooper() { return soundLooper; }

		int getCursorScreenX(int clientNumber, bool exact = false);
		int getCursorScreenY(int clientNumber, bool exact = false);

		void setGUIVisibility(int player, bool visible);

		bool pushUIState();
		bool popUIState();

		void setCameraTimeFactor(float factor);
		float getCameraTimeFactor();

		void setUIPauseState(bool paused);

		void setTacticalClickExpected(int player, int cursorType);

		SelectionBox *getSelectionBox();

		UIEffects *getEffects() { return effects; }

		void addHostileUnitPointer(int player, Unit *unit);

		void removeHostileUnitPointer(int player, Unit *unit);

		void doUnitClick(int player, Unit *unit);

		void setUnitHighlight(int player, Unit *unit);
		void setTerrainHighlight(int player, VC3 &position);
		void clearHighlight(int player);
		void lockHighlight(int player);
		void unlockHighlight(int player);

		void setErrorWindow(ui::ErrorWindow *errorWin);
		ErrorWindow *getErrorWindow() { return this->errorWindow; }

		virtual void readKey(char ascii, int keycode, 
			const char *keycodeName);

		void hideConsole();
		void showConsole();

		void setLeftDirectRotation(bool rotationOn);
		void setRightDirectRotation(bool rotationOn);

		void setControlsEnabled(int player, bool enabled);

		ui::AmbientSoundManager* getAmbientSoundManager();

		void setCamerasWaterManager();

		void setFirstPerson(int player, Unit *unit, int clientNumber);

		void nextInterfaceGeneration();
		void setOguiStormDriver(OguiStormDriver *driver);
		OguiStormDriver *getOguiStormDriver();
		
		void setUnitsChangedFlag(int player);

		void setScrollyEnabled(bool enabled);
		void setScrollyTemporarilyDisabled(bool disabled);

    // TODO: environmentEffects class or something??
    void setEnvironmentLightning(VC3 &fromPosition);

		frozenbyte::TextureCache *getTextureCache();

		util::TextureSwitcher *getTextureSwitcher();

		IStorm3D *getStorm3D();

		ui::LightManager *getLightManager();
		ui::DynamicLightManager *getDynamicLightManager();

		ui::AniRecorderWindow *getAniRecorderWindow();

		ui::GameConsole *getConsole();

		VC3 getListenerPosition();

		void playStreamedSound(const char *filename);
		void stopAllStreamedSounds();

#ifdef GUI_BUILD_MAP_WINDOW
		MapWindow *getMapWindow();
#endif

		void forceBuildingRoofHide();
		void forceBuildingRoofShow();
		void endForcedBuildingRoof();

		void updateUnitLighting(bool onlyNearPlayer);
		void updateCameraDependedElements();

		void setPlayerSelfIlluminationEnabled(bool enabled);
		util::LipsyncManager *getLipsyncManager();

		util::GridOcclusionCuller *getGridOcclusionCuller() { return this->gridOcclusionCuller; }

		VC3 getOcclusionCheckPosition();

		void createBuildingLighting();

		// for option applier :)
		sfx::SoundMixer *getSoundMixer() { return this->soundMixer; }

		void setFiresPreviously(Unit *unit, bool primaryPressed, bool secondaryPressed);
		void setConversationNoise(int index, int value);

		void openVehicleGUI(const char *params);
		void closeVehicleGUI();
		bool isVehicleGUIOpen();

		void forceCursorVisibility(bool enabled);

		void enableAlphaTestPass(bool enabled);

		inline boost::shared_ptr<LoadingWindow> getLoadingWindow() const { return loadingWindow; };

		void setTimeFactor(float factor);

		void setMovieAspectRatio(bool enabled);

		int getLastRunUITime() const;

		void SwapCursorImages(int cursor1, int cursor2);
		void ResetSwappedCursorImages();
	private:

		bool wasKeyClicked( int key );

		OguiStormDriver *oguiStormDriver;

		Ogui *ogui;
		Game *game;
		IStorm3D *storm3d;
		IStorm3D_Scene *scene;
		GameMap *renderMap;
		//IStorm3D_Terrain *renderTerrain;
		
		GameCamera *gameCamera; // the camera in use
		GameCamera *cameras[GAMEUI_CAMERA_AMOUNT];

		Terrain *renderTerrain;
		sfx::SoundMixer *soundMixer;
		CursorRayTracer *cursorRayTracer;

		sfx::MusicPlaylist *musicPlaylist;
		sfx::AmbientAreaManager *ambientAreaManager;

		sfx::SoundLooper *soundLooper;

		ICameraSystem *cameraSystem;

		bool quitRequested;

		bool msgBoxIsOpen;

		int movieAspectRatioGUIVisible;

		bool forceCursorVisible;

		float aimOffset; // Rotating flashlight/camera with the mouse
		float oldAimOffset; // Rotating flashlight/camera with the mouse
		VC3 positionOffset;
		
		TerrainCreator *terrainCreator;

		SceneSelection *sceneSelection[MAX_PLAYERS_PER_CLIENT];

		ArmorConstructWindow **armorConstructWindows;
		MenuCollection **commandWindows;
		StorageWindow **storageWindows;
		CombatWindow **combatWindows;
		boost::shared_ptr<LoadingWindow> loadingWindow;
		AniRecorderWindow *aniRecorderWindow;
		MessageBoxWindow **armorIncompleteConfirmWindows;
		MessageBoxWindow **quitBox;
		TerminalManager* terminalManager;
		CinematicScreen* cinematicScreen;

		ui::ErrorWindow *errorWindow;

#ifdef GUI_BUILD_MAP_WINDOW
		ui::MapWindow *mapWindow;
#endif
#ifdef GUI_BUILD_LOG_WINDOW
		ui::LogWindow* logWindow;
#endif
		ui::LogManager* logManager;
#ifdef PROJECT_SURVIVOR
		SurvivorUpgradeWindow *upgradeWindow;
#else
		UpgradeWindow *upgradeWindow;
#endif
#ifdef GUI_BUILD_INGAME_GUI_TABS
		ui::IngameGuiTabs*	ingameGuiTabs;
#endif
		VehicleWindow *vehicleWindow;
		CharacterSelectionWindow *characterSelectionWindow;

		// score window variables, added by Pete
		bool useScoreWindow;
		ui::ScoreWindow* scoreWindow;

		ui::MissionFailureWindow* missionFailureWindow;

		// missionSelection window variables
		ui::MissionSelectionWindow* missionSelectionWindow;

		bool abortMission;

		ui::UIEffects *effects;

		PlayerUnitCameraBoundary *unitCameraBoundary;

		ui::VisualEffectManager *visualEffectManager;

		ui::GameController *gameController[MAX_PLAYERS_PER_CLIENT];
		frozenbyte::BuildingHandler buildingHandler;

		bool pointersChanged;
		bool unitsDestroyed;
		bool unitsDamaged;

		int meterUpdateTime;
		int guiAnimationUpdateTime;

		int buildingHandlerUpdateTime;

		int lastRunUITime;

		int lastGameMessagePriority[MESSAGE_TYPES_AMOUNT];
		int lastGameMessageDuration[MESSAGE_TYPES_AMOUNT];
		int lastGameMessageCounter[MESSAGE_TYPES_AMOUNT];

		Unit *firstPerson[MAX_PLAYERS_PER_CLIENT];
		bool thirdPersonView;

		float cameraRange;

		LinkedList *uiStateStack;

		float cameraTimeFactor;

		float originalTimeFactor;

		int keyreaderId;
		ui::GameConsole *console;

		ui::AmbientSoundManager* ambientSoundManager;

		bool controlModeDirect;
		bool leftDirectRotation;
		bool rightDirectRotation;

		JoystickAimer *joystickAimer[MAX_PLAYERS_PER_CLIENT];

		// HACK for keeping character facing the correct direction
		VC2 oldJoystickXY[MAX_PLAYERS_PER_CLIENT];

		bool leftMovementEnabled[MAX_PLAYERS_PER_CLIENT];
		bool rightMovementEnabled[MAX_PLAYERS_PER_CLIENT];
		bool upMovementEnabled[MAX_PLAYERS_PER_CLIENT];
		bool downMovementEnabled[MAX_PLAYERS_PER_CLIENT];

		int lastPrimaryWeapon[MAX_PLAYERS_PER_CLIENT];
		int lastSecondaryWeapon[MAX_PLAYERS_PER_CLIENT];

		bool fireKeyDownPreviously[MAX_PLAYERS_PER_CLIENT];
		bool fireSecondaryKeyDownPreviously[MAX_PLAYERS_PER_CLIENT];

		VC2 clientUnitScreenPos[MAX_PLAYERS_PER_CLIENT];

    // TODO: move this to own class...
    ui::VisualEffect *lightningVisualEffect;
    int lightningTime;

		bool scrollyEnabled;
		bool scrollyTemporarilyDisabled;
		bool playerSelfIllumEnabled;

		frozenbyte::TextureCache *textureCache;
		util::TextureSwitcher *textureSwitcher;

		ui::LightManager *lightManager;
		ui::DynamicLightManager *dynamicLightManager;
		boost::scoped_ptr<util::LipsyncManager> lipsyncManager;

		VC3 listenerPosition;
		std::vector<int> loopingSoundEffectHandles;

		util::GridOcclusionCuller *gridOcclusionCuller;

		// a horrible solution...
		friend class Game;
		friend class BuildingAdder;
		friend class PhysicsContactSoundManager;
	};

}

#endif
