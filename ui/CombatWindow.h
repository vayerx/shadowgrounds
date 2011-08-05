
#ifndef COMBATWINDOW_H
#define COMBATWINDOW_H

#include <c2_vectors.h>
#include "../ogui/Ogui.h"
#include "../game/gamedefs.h"
#include "../game/UnitSelections.h"

#include <string>
#include <map>

#define COMBATW_UNITS 1
//#define COMBATW_UNITS 5

namespace game
{
	class Game;
	class Unit;
}

namespace ui
{
	int getNumberOfPlayers();
	
	class GamePointers;
	class CombatRadar;
	class OffscreenUnitPointers;
	class CombatUnitWindow;
	class CombatMessageWindow;
	class CombatMessageWindowWithHistory;
	class Visual2D;
	class TacticalUnitWindow;
	class SelectionBox;
	class UnitHighlight;
	class WeaponSelectionWindow;
	// class FlashlightWindow;
	class HealthWindow;
	/*class ItemWindow;
	class ItemWindowUpdator;
	class TargetDisplayWindow;
	*/
	class TargetDisplayWindowUpdator;
	/*class AmmoWindow;
	class WeaponWindow;
	class UpgradeAvailableWindow;
	class UnitHealthBarWindow;
	*/
	// class ComboWindow;

	class CombatWindowImpl;
	

	class CombatSubWindowFactory;
	class ICombatSubWindow;
	/**
	*
	* Combat window class. 
	* Invisible overlay to be used when in combat mode (not in menus).
	* Has invisible buttons and stuff to react when user clicks mouse 
	* on scene.
	* Contains a collection of CombatUnitWindows.
	*
	* @version 0.9, 29.9.2002
	* @author Jukka Kokkonen <jukka@frozenbyte.com>
	* @see GameUI
	* @see Game
	* @see Ogui
	* @see CombatRadar
	* @see CombatUnitWindow
	*
	*/
	
	class CombatWindow : public IOguiButtonListener, 
		private game::IUnitSelectionListener
	{
	public:
		
		CombatWindow(Ogui *ogui, game::Game *game, int player);
		
		~CombatWindow();
		
		virtual void CursorEvent(OguiButtonEvent *eve);
		
		virtual void unitSelectionEvent(game::Unit *unit);

		void createUnitWindows();
		
		void hide();
		void show();
		// notice! returns the _window_ visibility, not the _gui_ visibility!
		// (they are a different things)
		bool isWindowVisible();
		
		// affects gui visibility, not window visibility 
		// (well, the actual implementation may use window visibility to do 
		// that, but externally window visibility is unchanged)
		void setGUIVisibility(bool guiVisible);
		void toggleGUIMode();
		void toggleRadarMode();
		
		bool isGUIVisible();
		
		void update(int delta);
		void updateCursorImage();
		void updateMeters();
		void updateGUIAnimations();
		void updateHPMeters();
		void updatePointers();
		void updateModeInfo();
		void updateCameraDependedElements();
		void renderPointers();
		void updateUnitPointers();
		
		void updateRadar(float x, float y);
		void setRadarAngle(float angle);

		void addHostileUnitPointer(game::Unit *unit);
		void removeHostileUnitPointer(game::Unit *unit);
		void updateOffscreenUnitPointers();
		
		void recreatePointers();
		
		void recreateUnitSelections();
		
		void doCombatControls(int timeDelta);
		
		void doUnitSelectionByNumber(int unitNum);
		
		void doUnitSelection(game::Unit *unit);
		
		void doAllUnitSelection();
		
		void doUnitClick(game::Unit *unit);

		void doUnitAttack(const VC3 &target, game::Unit *targetUnit);
		
		void showMessage(const char *message, Visual2D *image, bool rightSide);
		void showCenterMessage(const char *message);
		void showHintMessage(const char *message);
		void showExecuteTipMessage(const char *message);

		void clearMessage();
		void clearCenterMessage();
		void clearHintMessage();
		void clearExecuteTipMessage();
		
		void setCrosshair(bool crosshairVisible);

		void setCrosshairProperties(int screenX, int screenY, float sizeFactor);

		SelectionBox *getSelectionBox() { return selectionBox; }

		void setTacticalClickExpected(int cursorType);
		
		void setTacticalModeButton(bool tactical);
		
		void updateUnitHighlight();
		void setUnitHighlight(const game::Unit *unit);
		void setTerrainHighlight(VC3 &position);
		void clearHighlight();
		void lockHighlight();
		void unlockHighlight();

		void openUnitHealthBar( game::Unit* unit);
		void closeUnitHealthBar();
		void setUnitHealthBarFlashing(int value);

		HealthWindow* getHealthWindow() const;

		void raiseMessages();

		// for overlay gui, make unit/flashlight guis invisible... (temporarily)
		void startGUIModeTempInvisible(int fadeTime = 0);
		void endGUIModeTempInvisible(int fadeTime = 0);
		bool isGUIModeTempInvisible(void);

		void setConversationNoise(int index, int value);
		
		void hideFlashlight();
		void showFlashlight();

		void openSubWindow( const std::string& window_name, int player_num = 0 );
		void closeSubWindow( const std::string& window_name );
		ICombatSubWindow* getSubWindow( const std::string& window ) const;
		void setSubWindowsVisible( bool visible, bool radar_visible );

		void setRadarDisabled(bool disabled);

		bool hasMessageWindow() const;

	private:
		Ogui *ogui;
		game::Game *game;
		int player;
		OguiWindow *win;
		
		int dragStartX;
		int dragStartY;
		int dragStartTime;

		int lastMoveClickTime;
		VC3 lastMoveTarget;
		
		int *unitSelectTime;
		int *blinkUnit;
		
		bool winVisible;
		int radarMode;
		int guiMode;
		bool guiVisible;
		bool guiTempInvisible;

		bool radarTemporarilyDisabled;
		bool radarDisabled;
		bool radarWasDisabled;
		
		bool tacticalClickExpected;
		int tacticalClickCursorType;

		WeaponSelectionWindow *weaponSelectionWindows[MAX_PLAYERS_PER_CLIENT];

		//CombatUnitWindow *unitWindows[COMBATW_UNITS];
		
		// screen areas (scene and areas that move/rotate camera)
		OguiButton **areas;
		
		CombatMessageWindow *messageWindow;
		CombatMessageWindow *messageWindowRight;
		CombatMessageWindow *centerMessageWindow;
		CombatMessageWindowWithHistory *hintMessageWindow;
		CombatMessageWindow *executeTipMessageWindow;
		CombatMessageWindow *timerWindow;
		
		OguiButton *crosshair;
		IOguiImage *crosshairImage;

		OguiButton *tacticalModeBut;
		
		game::Unit *unitsByNumber[COMBATW_UNITS];
		
		CombatRadar *radar;

		OffscreenUnitPointers *offscreenUnitPointers;
		
		TacticalUnitWindow *tacticalUnitWindow;
		
		SelectionBox *selectionBox;

		std::map< std::string, ICombatSubWindow* > subWindowMap;

		// ICombatSubWindow* flashlightWindow;
		ICombatSubWindow* healthWindow;
		ICombatSubWindow* targetDisplayWindow;
		//ICombatSubWindow* ammoWindow;
		//ICombatSubWindow* weaponWindow;
		//ICombatSubWindow* upgradeAvailableWindow;
		//ICombatSubWindow* itemWindow;

		// TargetDisplayWindowUpdator* targetDisplayWindowUpdator;

		ICombatSubWindow*	unitHealthBar;
		
		UnitHighlight *unitHighlight;
		bool highlightLocked;
		
		// ComboWindow* comboWindow;
		// if cursor is on the scene (not on any unit status)
		bool cursorOnScene;
		
		// the waypoint, selection and other pointers
		GamePointers *gamePointers;
		
		// returns button number for unit or -1 if no button for that unit
		int solveNumberForUnit(game::Unit *unit);
		
		// returns unit for the given button number of null, if no unit
		game::Unit *solveUnitForNumber(int number);
		
		// updates the unit window positions based on current gui mode
		void setUnitWindowPositions();

		CombatSubWindowFactory* windowFactory;

		CombatWindowImpl *impl;
	};
  
}

#endif
