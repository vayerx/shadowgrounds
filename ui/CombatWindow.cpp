
#include "precompiled.h"

#include <assert.h>

#include "gui_configuration.h"

#include "CombatWindow.h"

#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../system/Timer.h"
#include "../game/Game.h"
#include "../game/GameStats.h"
#include "../game/GameScene.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/UnitList.h"
#include "../game/UnitInventory.h"
#include "../game/options/options_gui.h"
#include "../sound/sounddefs.h"
#include "../game/Item.h"
#include "../game/Part.h"
#include "../game/PartList.h"
#include "../game/UnitActor.h"
#include "../game/unittypes.h"
#include "../game/Character.h"
#include "../game/UnitFormation.h"
#include "../game/Weapon.h"
#include "../game/GameUI.h"
#include "../game/options/options_tactical.h"
#include "../game/options/options_cheats.h"
#include "../game/options/options_game.h"
#include "../game/SimpleOptions.h"
#include "../game/scaledefs.h"
#include "../game/ItemManager.h"
#include "../game/scripting/GameScripting.h"
#include "../game/Flashlight.h"
#include "../game/PlayerWeaponry.h"
// #include "FlashlightWindow.h"
#include "HealthWindow.h"
// #include "HealthWindowCoop.h"
// #include "ItemWindow.h"
// #include "ItemWindowUpdator.h"

// #include "TargetDisplayWindow.h"
#include "TargetDisplayWindowUpdator.h"
/*
#include "WeaponWindow.h"
#include "WeaponWindowCoop.h"
#include "AmmoWindow.h"
#include "AmmoWindowCoop.h"
#include "UpgradeAvailableWindow.h"
*/
#include "cursordefs.h"
#include "WeaponSelectionWindow.h"
#include "GameCamera.h"
#include "GameController.h"
#include "IPointableObject.h"
#include "GamePointers.h"
#include "uidefaults.h"
#include "CombatRadar.h"
#include "OffscreenUnitPointers.h"
//#include "CombatUnitWindow.h"
#include "CombatMessageWindow.h"
#include "CombatMessageWindowWithHistory.h"
#include "AnimationSet.h"
#ifdef PROJECT_SHADOWGROUNDS
#include "TacticalUnitWindow.h"
#endif
#include "Visual2D.h"
#include "SelectionBox.h"
#include "UnitHighlight.h"
#include "SelectionBox.h"
#include "AniRecorderWindow.h"
#include "Spotlight.h"
#include "../game/options/options_players.h"
#include "UnitHealthBarWindow.h"
#include "../util/Script.h"

#ifdef GUI_BUILD_AOV_CROSSHAIR
#include "AOVCrosshair.h"
#include <Storm3D_UI.h>
extern IStorm3D_Scene *disposable_scene;
#endif


#ifndef PROJECT_SHADOWGROUNDS
// #include "ComboWindow.h"	
#endif

#ifdef PROJECT_SURVIVOR
	#include "GenericTextWindow.h"
#endif

#include "CombatSubWindowFactory.h"

#include <string>


// TEMP
#include "../game/UnitLevelAI.h"

// this should be handled by a game request
#include "../util/AI_PathFind.h"

#include "../util/Debug_MemoryManager.h"


// button id's
#define COMBATW_AREA_SCREEN 1
#define COMBATW_AREA_FWD 2
#define COMBATW_AREA_BACK 3
#define COMBATW_AREA_LEFT 4
#define COMBATW_AREA_RIGHT 5
#define COMBATW_AREA_ROTATELEFT 6
#define COMBATW_AREA_ROTATERIGHT 7
#define COMBATW_AREA_ORBITLEFT 8
#define COMBATW_AREA_ORBITRIGHT 9
#define COMBATW_AREA_UPLEFT 10
#define COMBATW_AREA_UPRIGHT 11
#define COMBATW_AREA_DOWNLEFT 12
#define COMBATW_AREA_DOWNRIGHT 13

// last area id + 1
#define COMBATW_AREAS 14

#define COMBATW_TACTICAL_MODE 15

//#define COMBATW_UNIT_START 100
//#define COMBATW_UNIT_END 199

//#define COMBATW_UNITMODE_START 200
//#define COMBATW_UNITMODE_END 299

//#define COMBATW_UNITWEAP_START 300
//#define COMBATW_UNITWEAP_END 399

// id's 400 - 599 reserved for radar!

// id's 600 - 799 reserved for offscreen unit pointers!


//#define UNITSTAT_BOTTOM_START_X 130
//#define UNITSTAT_BOTTOM_START_Y (768 - 82 - 2)
//#define UNITSTAT_BOTTOM_PAD_X 150
#define UNITSTAT_BOTTOM_START_X 2
#define UNITSTAT_BOTTOM_START_Y (768 - 86 - 4)
#define UNITSTAT_BOTTOM_PAD_X 190

//#define UNITSTAT_RIGHT_START_X (1024-147 - 2)
//#define UNITSTAT_RIGHT_START_Y 0
//#define UNITSTAT_RIGHT_PAD_Y 82
#define UNITSTAT_RIGHT_START_X (1024-188 - 4)
#define UNITSTAT_RIGHT_START_Y 2
#define UNITSTAT_RIGHT_PAD_Y 86

//#define COMBATW_HEAT_IMAGES 21

//#define COMBATW_HP_IMAGES 21
//#define COMBATW_HP_IMGTYPES 4
//#define COMBATW_HP_BARS 6

//#define GUI_MODE_INVALID 0
#define GUI_MODE_INVISIBLE 1
#define GUI_MODE_BOTTOM 2
#define GUI_MODE_RIGHT 3

#define MIN_GUI_MODE 1
#define MAX_GUI_MODE 3

//#define RADAR_MODE_INVALID 0
#define RADAR_MODE_INVISIBLE 1
#define RADAR_MODE_RADAR 2
#define RADAR_MODE_MAP 3

#define MIN_RADAR_MODE 1
#define MAX_RADAR_MODE 3


// msec from clip empty to autoreload (affects click shot weapons only)
#define CLIPEMPTY_AUTORELOAD_TIME_LIMIT 400

using namespace game;


namespace ui
{

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

	class CombatWindowImpl
	{
	private:
#ifdef GUI_BUILD_AOV_CROSSHAIR
		AOVCrosshair *aovCrosshairs[MAX_PLAYERS_PER_CLIENT];
#endif

		CombatWindowImpl()
		{
#ifdef GUI_BUILD_AOV_CROSSHAIR
			for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				aovCrosshairs[i] = NULL;
			}
#endif
		}

		~CombatWindowImpl()
		{
#ifdef GUI_BUILD_AOV_CROSSHAIR
			for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				if (aovCrosshairs[i] != NULL)
					delete aovCrosshairs[i];
			}
#endif
		}

		friend class CombatWindow;
	};


	CombatWindow::CombatWindow(Ogui *ogui, game::Game *game, int player) :
		subWindowMap(),
		healthWindow( NULL ),
		targetDisplayWindow( NULL ),
		windowFactory( CombatSubWindowFactory::GetSingleton() )
	{
		this->impl = new CombatWindowImpl();

		this->player = player;
		this->game = game;
		this->ogui = ogui;
		
		win = ogui->CreateSimpleWindow(0, 0, 1024, 768, NULL);
		win->Hide();
		
		winVisible = false;
		guiMode = GUI_MODE_BOTTOM;
		radarMode = RADAR_MODE_RADAR;
		guiVisible = true;
		guiTempInvisible = false;

		radarTemporarilyDisabled = false;
#ifdef PROJECT_SURVIVOR
		radarDisabled = true;
		radarWasDisabled = true;
#else
		radarDisabled = false;
		radarWasDisabled = false;
#endif
		
		int i;
		for (i = 0; i < COMBATW_UNITS; i++)
		{
			//unitWindows[i] = NULL;
		}
		
		dragStartX = 0;
		dragStartY = 0;
		dragStartTime = 0;

		lastMoveClickTime = 0;
		lastMoveTarget = VC3(0,0,0);
		
		areas = new OguiButton *[COMBATW_AREAS];
		for (i = 0; i < COMBATW_AREAS; i++)
		{
			areas[i] = NULL;
		}

		for (i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			weaponSelectionWindows[i] = NULL;
		}
		
		
		// the whole screen area is clickable...
		OguiButton *b;
		b = ogui->CreateSimpleImageButton(win, 0, 0, 1024, 768, NULL, NULL, NULL, 
			COMBATW_AREA_SCREEN);
		areas[COMBATW_AREA_SCREEN] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_PRESS | OGUI_EMASK_OUT | OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		
		//createUnitWindows();		
		
		//unitsSelected = 0;
		
		cursorOnScene = false;
		
		// gotta do radar before corner areas, or it will block them...
		// RADAR REMOVED!
		if (!radarTemporarilyDisabled && !radarDisabled)
		{
			radar = new CombatRadar(ogui, game, player, win);
		} else {
			radar = NULL;
		}

		offscreenUnitPointers = new OffscreenUnitPointers(ogui, game, player, win);
		
		// then the screen edges that will move the camera
		b = ogui->CreateSimpleImageButton(win, 8, 0, 1024-16, 8, NULL, NULL, NULL, 
			COMBATW_AREA_FWD);
		areas[COMBATW_AREA_FWD] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 8, 768-8, 1024-16, 8, NULL, NULL, NULL, 
			COMBATW_AREA_BACK);
		areas[COMBATW_AREA_BACK] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 0, 8, 8, 768-16, NULL, NULL, NULL, 
			COMBATW_AREA_LEFT);
		areas[COMBATW_AREA_LEFT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 1024-8, 8, 8, 768-16, NULL, NULL, NULL, 
			COMBATW_AREA_RIGHT);
		areas[COMBATW_AREA_RIGHT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 0, 768-8, 8, 8, NULL, NULL, NULL, 
			COMBATW_AREA_DOWNLEFT);
		areas[COMBATW_AREA_DOWNLEFT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 1024-8, 768-8, 8, 8, NULL, NULL, NULL, 
			COMBATW_AREA_DOWNRIGHT);
		areas[COMBATW_AREA_DOWNRIGHT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 0, 0, 8, 8, NULL, NULL, NULL, 
			COMBATW_AREA_UPLEFT);
		areas[COMBATW_AREA_UPLEFT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		b = ogui->CreateSimpleImageButton(win, 1024-8, 0, 8, 8, NULL, NULL, NULL, 
			COMBATW_AREA_UPRIGHT);
		areas[COMBATW_AREA_UPRIGHT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		// HACK: scrollers off
		b->SetDisabled(true);
		
		// orbiting and rotation disabled
		/*
		b = ogui->CreateSimpleImageButton(win, 0, 768-8, 8, 8, NULL, NULL, NULL, 
		COMBATW_AREA_ORBITLEFT);
		areas[COMBATW_AREA_ORBITLEFT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		
		b = ogui->CreateSimpleImageButton(win, 1024-8, 768-8, 8, 8, NULL, NULL, NULL, 
		COMBATW_AREA_ORBITRIGHT);
		areas[COMBATW_AREA_ORBITRIGHT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		
		b = ogui->CreateSimpleImageButton(win, 0, 0, 8, 8, NULL, NULL, NULL, 
		COMBATW_AREA_ROTATELEFT);
		areas[COMBATW_AREA_ROTATELEFT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		
		b = ogui->CreateSimpleImageButton(win, 1024-8, 0, 8, 8, NULL, NULL, NULL, 
		COMBATW_AREA_ROTATERIGHT);
		areas[COMBATW_AREA_ROTATERIGHT] = b;
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE);
		*/
		
		crosshair = NULL;
		crosshairImage = NULL;
		
		bool cooperative = game->isCooperative();


		bool survivor = false;

#ifdef PROJECT_SURVIVOR
		survivor = true;
#endif

#ifdef GUI_BUILD_INGAME_GUI
		{
			if( cooperative  )
			{
				if(survivor)
				{
					openSubWindow( "HealthWindowCoop", getNumberOfPlayers() );
					openSubWindow( "AmmoWindowCoop", getNumberOfPlayers() );
				}

				// why are these commented out??

				//openSubWindow( "HealthWindowCoop", getNumberOfPlayers() );
				//openSubWindow( "AmmoWindowCoop", getNumberOfPlayers() );
				//if( survivor == false )
				//	openSubWindow( "WeaponWindowCoop", getNumberOfPlayers() );
			}
			else // basic_shadowgrounds_singleplayer_ui
			{
				openSubWindow( "FlashlightWindow", player );
				openSubWindow( "HealthWindow", player );
				openSubWindow( "AmmoWindow", player  );
				if( survivor == false )
					openSubWindow( "WeaponWindow", player  );
			}

			openSubWindow( "ItemWindow", player  );
			openSubWindow( "TargetDisplayWindow", player  );
			// openSubWindow( "UpgradeAvailableWindowUpdator", player  );
#ifndef PROJECT_SURVIVOR
			openSubWindow( "UpgradeAvailableWindow", player  );
#endif

		}
#endif

#ifdef GUI_BUILD_ELABORATE_HINT_MESSAGE
		{
			openSubWindow( "ElaborateHintMessageWindow", player );
			// openSubWindow( "SurvivorShieldBar", player );
			// openSubWindow( "SurvivorNapalmFlameBar", player );
		}
#endif
		
		messageWindow = new CombatMessageWindow(ogui, game, player, "converse", "face1");
		messageWindow->setBoxed(true);

		timerWindow = new CombatMessageWindow(ogui, game, player, "timer", "");
		timerWindow->setBoxed(true);

		messageWindowRight = new CombatMessageWindow(ogui, game, player, "converse", "face2");
		//messageWindowRight->setBoxed(true);

		centerMessageWindow = new CombatMessageWindow(ogui, game, player, "center", "");
		//centerMessageWindow->moveTo(1024/2-8, 768/2-8);
		centerMessageWindow->setFont(ui::defaultBigIngameFont, true);

		hintMessageWindow = new CombatMessageWindowWithHistory(ogui, "hint", "");
		//hintMessageWindow->moveTo(1024/2-8, 768-16);
		//hintMessageWindow->setFont(ui::defaultSmallIngameFont, true);
		hintMessageWindow->setFont(ui::defaultIngameFont, true);
		//hintMessageWindow->setBoxed(false);
		
		executeTipMessageWindow = new CombatMessageWindow(ogui, game, player, "executetip", "");
		//executeTipMessageWindow->setFont(ui::defaultSmallIngameFont, true);
		executeTipMessageWindow->setFont(ui::defaultIngameFont, true);
		
		gamePointers = new GamePointers(game->getGameScene());
		
		// set us the listener for unit selections
		game->unitSelections[player]->setListener(this);
		
#ifdef PROJECT_SHADOWGROUNDS
		tacticalUnitWindow = new TacticalUnitWindow(ogui, game);
#endif

		selectionBox = new SelectionBox(game, player, ogui->GetScaleX(), ogui->GetScaleY());

		tacticalClickExpected = false;
		tacticalClickCursorType = 0;

		tacticalModeBut = NULL;

		this->unitHighlight = new UnitHighlight(
			game->getGameScene()->getStorm3D(),
			game->getGameScene()->getStormScene());
		highlightLocked = false;

		// HACK: gui and radar options... :)
		if (SimpleOptions::getInt(DH_OPT_I_GUI_UNITS_POSITION) == GUI_MODE_RIGHT)
		{
			this->toggleGUIMode();
		}
		else if (SimpleOptions::getInt(DH_OPT_I_GUI_UNITS_POSITION) == GUI_MODE_INVISIBLE)
		{
			this->toggleGUIMode();
			this->toggleGUIMode();
		}

		if (SimpleOptions::getInt(DH_OPT_I_GUI_RADAR_MODE) == RADAR_MODE_MAP)
		{
			this->toggleRadarMode();
		}
		else if (SimpleOptions::getInt(DH_OPT_I_GUI_RADAR_MODE) == RADAR_MODE_INVISIBLE)
		{
			this->toggleRadarMode();
			this->toggleRadarMode();
		}

		unitHealthBar = NULL;

#ifdef GUI_BUILD_COMBO_WINDOW
		// comboWindow = new ComboWindow( ogui, game, player );
		openSubWindow( "ComboWindow" );
#endif
	}
	

	CombatWindow::~CombatWindow()
	{
		delete impl;

		// we are no-longer listening to unit selections
		game->unitSelections[player]->setListener(NULL);
		
		{
			std::map< std::string, ICombatSubWindow* >::iterator i;
			for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
			{
				delete i->second;
			}
		}

		int i;
		for (i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if (weaponSelectionWindows[i] != NULL)
			{
				delete weaponSelectionWindows[i];
				weaponSelectionWindows[i] = NULL;
			}
		}
		for (i = 0; i < COMBATW_AREAS; i++)
		{
			if (areas[i] != NULL)
			{
				delete areas[i];
				areas[i] = NULL;
			}
		}
		if (crosshair != NULL)
		{
			delete crosshair;
			crosshair = NULL;
		}
		if (crosshairImage != NULL)
		{
			delete crosshairImage;
			crosshairImage = NULL;
		}
		delete [] areas;
		if (tacticalModeBut != NULL)
		{
			delete tacticalModeBut;
			tacticalModeBut = NULL;
		}
		/*
		for (i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				delete unitWindows[i];
				unitWindows[i] = NULL;
			}
		}
		*/
		if (messageWindow != NULL)
		{
			delete messageWindow;
			messageWindow = NULL;
		}
		if (timerWindow != NULL)
		{
			delete timerWindow;
			timerWindow = NULL;
		}
		if (messageWindowRight != NULL)
		{
			delete messageWindowRight;
			messageWindowRight = NULL;
		}
		if (centerMessageWindow != NULL)
		{
			delete centerMessageWindow;
			centerMessageWindow = NULL;
		}
		if (hintMessageWindow != NULL)
		{
			delete hintMessageWindow;
			hintMessageWindow = NULL;
		}
		if (executeTipMessageWindow != NULL)
		{
			delete executeTipMessageWindow;
			executeTipMessageWindow = NULL;
		}
		if (offscreenUnitPointers != NULL)
		{
			delete offscreenUnitPointers;
			offscreenUnitPointers = NULL;
		}
		if (radar != NULL)
		{
			delete radar;
			radar = NULL;
		}
		if (win != NULL)
		{
			delete win;
			win = NULL;
		}
		delete unitHighlight;
		delete gamePointers;
#ifdef PROJECT_SHADOWGROUNDS
		delete tacticalUnitWindow;
#endif
		delete selectionBox;
	
		// delete unitHealthBar;
		// unitHealthBar = NULL;

	}
	

	void CombatWindow::createUnitWindows()
	{
		/*
		int i;

		// get rid of old windows if they exist
		for (i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				delete unitWindows[i];
				unitWindows[i] = NULL;
			}
		}
		*/

		/*
		// create unit statuses for owned active units
		LinkedList *ulist = game->units->getOwnedUnits(player);
		ulist->resetIterate();
		int unitNum = 0;
		for (i = 0; i < COMBATW_UNITS; i++)
		{
			if (ulist->iterateAvailable())
			{
				Unit *u = (Unit *)ulist->iterateNext();
				if (u->isActive())
				{
					unitWindows[unitNum] = new CombatUnitWindow(ogui, game, u, unitNum);
					unitWindows[unitNum]->hide();
					unitNum++;
				}
			}
		}
		*/
		//setUnitWindowPositions();
	}

	
	void CombatWindow::toggleGUIMode()
	{
		guiMode++;
		if (guiMode > MAX_GUI_MODE)
			guiMode = MIN_GUI_MODE;
		setUnitWindowPositions();
		
		if (!guiVisible || guiMode == GUI_MODE_INVISIBLE)
		{
			/*
			for (int i = 0; i < COMBATW_UNITS; i++)
			{
				if (unitWindows[i] != NULL)
					unitWindows[i]->hide();
			}
			*/

			std::map< std::string, ICombatSubWindow* >::iterator i;
			for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
			{
				i->second->hide();
			}

			hintMessageWindow->hide();
			executeTipMessageWindow->hide();
			timerWindow->hide();

		} else {
			/*
			for (int i = 0; i < COMBATW_UNITS; i++)
			{
				if (unitWindows[i] != NULL)
					unitWindows[i]->show();
			}
			*/
			std::map< std::string, ICombatSubWindow* >::iterator i;
			for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
			{
				i->second->show();
			}

			hintMessageWindow->show();
			executeTipMessageWindow->show();
			timerWindow->show();
		}
	}

	void CombatWindow::startGUIModeTempInvisible(int fadeTime)
	{
		guiTempInvisible = true;

		std::map< std::string, ICombatSubWindow* >::iterator i;
		for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
		{
			i->second->hide( fadeTime );
		}

		hintMessageWindow->hide( fadeTime );
		executeTipMessageWindow->hide( fadeTime );
		timerWindow->hide( 0 );

		if (radar != NULL)
		{
			delete radar;
			radar = NULL;
		}
	}

	void CombatWindow::endGUIModeTempInvisible(int fadeTime)
	{
		guiTempInvisible = false;

		if (!guiVisible || guiMode == GUI_MODE_INVISIBLE)
		{
			// nop
		} else {

			std::map< std::string, ICombatSubWindow* >::iterator i;
			for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
			{
				i->second->show( fadeTime );
			}

			hintMessageWindow->show( fadeTime );
			executeTipMessageWindow->show( fadeTime );
			timerWindow->show( 0 );
		
		}
		if(radarMode == RADAR_MODE_RADAR)
		{
			if(!radar)
			{
				if (!radarTemporarilyDisabled && !radarDisabled)
				{
					radar = new CombatRadar(ogui, game, player, win);
				}
			}
		}
	}

	bool CombatWindow::isGUIModeTempInvisible(void)
	{
		return guiTempInvisible;
	}

	void CombatWindow::setConversationNoise(int index, int value)
	{
		if(index == 0 && messageWindow)
			messageWindow->setNoiseAlpha(value / 100.f);
		else if(index == 1 && messageWindowRight)
			messageWindowRight->setNoiseAlpha(value / 100.f);
	}

	void CombatWindow::toggleRadarMode()
	{
		radarMode++;
		if (radarMode > MAX_RADAR_MODE)
			radarMode = MIN_RADAR_MODE;

		if (radar != NULL)
		{
			delete radar;
			radar = NULL;
		}
		if (radarMode == RADAR_MODE_RADAR)
		{
			if (!radarTemporarilyDisabled && !radarDisabled)
			{
				radar = new CombatRadar(ogui, game, player, win);
			}
		}
	}
	
	
	bool CombatWindow::isGUIVisible()
	{
		return guiVisible;
	}
	
	
	void CombatWindow::setGUIVisibility(bool guiVisible)
	{
		this->guiVisible = guiVisible;
		if (!guiVisible)
		{
			win->Hide();
			/*
			for (int i = 0; i < COMBATW_UNITS; i++)
			{
				if (unitWindows[i] != NULL)
					unitWindows[i]->hide();
			}
			*/
			std::map< std::string, ICombatSubWindow* >::iterator i;
			for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
			{
				i->second->hide();
			}

			hintMessageWindow->hide();
			executeTipMessageWindow->hide();
			timerWindow->hide();


		} else {
			win->Show();
			if (guiVisible && guiMode != GUI_MODE_INVISIBLE)
			{
				/*
				for (int i = 0; i < COMBATW_UNITS; i++)
				{
					if (unitWindows[i] != NULL)
						unitWindows[i]->show();
				}
				*/
				std::map< std::string, ICombatSubWindow* >::iterator i;
				for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
				{
					i->second->show();
				}
				hintMessageWindow->show();
				executeTipMessageWindow->show();
				timerWindow->show();

				// can no longer be temp invisible...
				guiTempInvisible = false;

			}
		}
	}
	

	void CombatWindow::setUnitWindowPositions()
	{
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				if (guiMode == GUI_MODE_BOTTOM)
					unitWindows[i]->moveTo(UNITSTAT_BOTTOM_START_X + 
					UNITSTAT_BOTTOM_PAD_X * i, UNITSTAT_BOTTOM_START_Y);
				if (guiMode == GUI_MODE_RIGHT)
					unitWindows[i]->moveTo(UNITSTAT_RIGHT_START_X,
					UNITSTAT_RIGHT_START_Y + UNITSTAT_RIGHT_PAD_Y * i);
			}
		}
		*/
	}



	void CombatWindow::setTacticalClickExpected(int cursorType)
	{
		tacticalClickExpected = true;
		tacticalClickCursorType = cursorType;
	}
	
	
	void CombatWindow::updateOffscreenUnitPointers()
	{
		if (offscreenUnitPointers != NULL)
		{
			offscreenUnitPointers->update();
		}
	}
	

	void CombatWindow::addHostileUnitPointer(game::Unit *unit)
	{
		if (offscreenUnitPointers != NULL)
		{
			offscreenUnitPointers->addUnitForChecklist(unit);
		}
	}


	void CombatWindow::removeHostileUnitPointer(game::Unit *unit)
	{
		if (offscreenUnitPointers != NULL)
		{
			offscreenUnitPointers->removeUnitFromChecklist(unit);
		}
	}


	void CombatWindow::updateRadar(float x, float y)
	{
		if (radar != NULL)
		{
			radar->setOrigo(x, y);
			radar->update();
		}
	}
	
	
	void CombatWindow::setRadarAngle(float angle)
	{
		if (radar != NULL)
		{
			radar->setAngle(angle);
		}
	}
	
	
	void CombatWindow::setCrosshair(bool crosshairVisible)
	{
		if (crosshairVisible)
		{
#ifdef GUI_BUILD_AOV_CROSSHAIR
			for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				if (impl->aovCrosshairs[i] == NULL
					&& game->gameUI->getFirstPerson(i) != NULL)
				{
					impl->aovCrosshairs[i] = new AOVCrosshair(ogui, win, i, game->gameUI->getFirstPerson(i));
				}
				if (impl->aovCrosshairs[i] != NULL)
				{
					impl->aovCrosshairs[i]->show();
				}
			}
#endif
			// NO MORE CROSSHAIR (just the cursor crosshair)
			/*
			if (crosshair == NULL)
			{
				if (crosshairImage == NULL)
				{
					crosshairImage = ogui->LoadOguiImage("Data/GUI/Ingame/crosshair.tga");
				}
				crosshair = ogui->CreateSimpleImageButton(win, 
					(1024-32)/2, (768+8-32)/2, 32, 32, NULL, NULL, NULL);
				crosshair->SetDisabledImage(crosshairImage);
				// the unit seem to shoot a bit lower than this... thus +8
				//	(1024-32)/2, (768-32)/2, 32, 32, "Data/Buttons/Ingame/crosshair.tga", NULL, NULL);
				crosshair->SetReactMask(0);
				crosshair->SetDisabled(true);
			}
			*/
		} else {
#ifdef GUI_BUILD_AOV_CROSSHAIR
			for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				if (impl->aovCrosshairs[i] != NULL)
					impl->aovCrosshairs[i]->hide();
			}
#endif
			if (crosshair != NULL)
			{
				delete crosshair;
				crosshair = NULL;
			}
		}
	}
	

	void CombatWindow::setCrosshairProperties(int screenX, int screenY, float sizeFactor)
	{
#ifdef GUI_BUILD_AOV_CROSSHAIR
			for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				if (game->gameUI->getFirstPerson(i) != NULL)
				{
					int originX = 1024/2;
					int originY = 768/2;

					VC3 pos = game->gameUI->getFirstPerson(i)->getPosition();

					// HACK: ...
					pos.z += 1.15f;

					VC3 result = VC3(0,0,0);
					float rhw = 0;
					float real_z = 0;
					IStorm3D_Camera *cam = disposable_scene->GetCamera();
					bool infront = cam->GetTransformedToScreen(pos, result, rhw, real_z);

					if (infront)
					{
						int x = (int)(result.x * 1024);
						int y = (int)(result.y * 768);

						originX = x;
						originY = y;
						if (impl->aovCrosshairs[i] != NULL)
							impl->aovCrosshairs[i]->setScreenPosition(screenX, screenY, originX, originY);
					}
				} else {
					if (impl->aovCrosshairs[i] != NULL)
					{
						delete impl->aovCrosshairs[i];
						impl->aovCrosshairs[i] = NULL;
					}
				}
			}
#endif
		if (crosshair != NULL)
		{
			if (sizeFactor < 1.2f) sizeFactor = 1.2f;
			sizeFactor = (1.0f + sizeFactor) / 2.0f;
			int size = (int)(sizeFactor * 32.0f);
			size = (size & (0xffff - 1));
			crosshair->Move(screenX - size / 2, screenY - size / 2);
			crosshair->Resize(size, size);
		}		
	}

	
	void CombatWindow::hide()
	{
		winVisible = false;
		win->Hide();
		win->SetReactMask( 0 );
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
				unitWindows[i]->hide();
		}
		*/
	}
	
	
	void CombatWindow::show()
	{
		winVisible = true;
		win->Show();
		win->SetReactMask( OGUI_WIN_REACT_MASK_ALL );
		if (guiVisible && guiMode != GUI_MODE_INVISIBLE)
		{
			/*
			for (int i = 0; i < COMBATW_UNITS; i++)
			{
				if (unitWindows[i] != NULL)
					unitWindows[i]->show();
			}
			*/
		}
	}
	
	
	bool CombatWindow::isWindowVisible()
	{
		return winVisible;
		//return win->IsVisible();
	}
	
	
	void CombatWindow::update(int delta)
	{
		if(messageWindow)
			messageWindow->update(delta);
		if(messageWindowRight)
			messageWindowRight->update(delta);
		if(timerWindow)
			timerWindow->update(delta);

#ifdef GUI_BUILD_AOV_CROSSHAIR
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if (impl->aovCrosshairs[i] != NULL)
				impl->aovCrosshairs[i]->update();
		}
#endif

#ifdef PROJECT_SURVIVOR
		// hack to update certain windows a bit more often
		ICombatSubWindow* windows[] =
		{
		 getSubWindow("GrenadeLaunchBar_0"),
		 getSubWindow("GrenadeLaunchBar_1"),
		 getSubWindow("GrenadeLaunchBar_2"),
		 getSubWindow("GrenadeLaunchBar_3"),
		 getSubWindow("CountdownBar"),
		 getSubWindow("CountupBar")
		};
		for(int i = 0; i < 6; i++)
		{
			if(windows[i] == NULL)
				continue;

			windows[i]->update();
		}
#endif
	}

	void CombatWindow::updateCursorImage()
	{
		if (!game->gameUI->isCursorActive(game->singlePlayerNumber)
#ifdef PROJECT_SURVIVOR
			// hide cursors in cinematic screen
			|| game->gameUI->isCinematicScreenOpen()
#endif
			)
		{
			ogui->SetCursorImageState(0, DH_CURSOR_INVISIBLE);
			ogui->SetCursorImageState(1, DH_CURSOR_INVISIBLE);
			ogui->SetCursorImageState(2, DH_CURSOR_INVISIBLE);
			ogui->SetCursorImageState(3, DH_CURSOR_INVISIBLE);
			return;
		}

#ifdef CRIMSON_MODE
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_SURVIVOR) || defined(PROJECT_CLAW_PROTO)
		if (game->gameUI->getFirstPerson(0) != NULL
			&& game->gameUI->isThirdPersonView(game->singlePlayerNumber)
			&& game->gameUI->isControlModeDirect(game->singlePlayerNumber))
		{
			// TODO: || gameUI->isMessageBoxVisible() ?????
			if (game->gameUI->isAnyIngameWindowVisible())
			{
				ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
				ogui->SetCursorImageState(1, DH_CURSOR_AIM_PLAYER2);
				ogui->SetCursorImageState(2, DH_CURSOR_AIM_PLAYER3);
				ogui->SetCursorImageState(3, DH_CURSOR_AIM_PLAYER4);

			} else {
				ogui->SetCursorImageState(1, DH_CURSOR_AIM_PLAYER2);
				ogui->SetCursorImageState(2, DH_CURSOR_AIM_PLAYER3);
				ogui->SetCursorImageState(3, DH_CURSOR_AIM_PLAYER4);

				if (game->gameUI->getFirstPerson(0)->isClipReloading())
				{
					int cursor = DH_CURSOR_AIM_RELOADING;

#ifdef PROJECT_SURVIVOR
					// get custom cursor from weapon
					int wnum = game->gameUI->getFirstPerson(0)->getSelectedWeapon();
					if(wnum != -1)
					{
						Weapon *w = game->gameUI->getFirstPerson(0)->getWeaponType(wnum);
						if(w && w->getReloadCursor() >= 0 && w->getReloadCursor() < DH_CURSOR_AMOUNT)
						{
							cursor = w->getReloadCursor();
						}
					}
#endif

					ogui->SetCursorImageState(0, cursor);
				}
				else
				{
					int cursor;
#ifdef PROJECT_SURVIVOR
					cursor = DH_CURSOR_AIM_SPREAD1;
#else
					if (game->gameUI->getFirstPerson(0)->getFiringSpreadFactor() < 1.5f)
						cursor = DH_CURSOR_AIM_SPREAD1;
					else if (game->gameUI->getFirstPerson(0)->getFiringSpreadFactor() < 2.5f)
						cursor = DH_CURSOR_AIM_SPREAD2;
					else if (game->gameUI->getFirstPerson(0)->getFiringSpreadFactor() < 3.5f)
						cursor = DH_CURSOR_AIM_SPREAD3;
					else if (game->gameUI->getFirstPerson(0)->getFiringSpreadFactor() < 4.5f)
						cursor = DH_CURSOR_AIM_SPREAD4;
					else
						cursor = DH_CURSOR_AIM_SPREAD5;
#endif

#ifdef PROJECT_SURVIVOR
					// get custom cursor from weapon
					int wnum = game->gameUI->getFirstPerson(0)->getSelectedWeapon();
					if(wnum != -1)
					{
						Weapon *w = game->gameUI->getFirstPerson(0)->getWeaponType(wnum);
						if(w && w->getCursor() >= 0 && w->getCursor() < DH_CURSOR_AMOUNT)
						{
							cursor = w->getCursor();
						}
					}
#endif
					ogui->SetCursorImageState(0, cursor);
				}

#ifdef PROJECT_SURVIVOR
				// potentially unhealthy hack: make dead players cursors invisible
				for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
				{
					if (game->gameUI->getFirstPerson(c) == NULL
						|| game->gameUI->getFirstPerson(c)->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					{
						ogui->SetCursorImageState(c, DH_CURSOR_INVISIBLE);
					}
				}
#endif

			}
			return;
		}
#else
		for (int cl = 0; cl < MAX_PLAYERS_PER_CLIENT; cl++)
		{
			if (game->gameUI->getFirstPerson(cl) != NULL
				&& game->gameUI->isThirdPersonView(game->singlePlayerNumber)
				&& game->gameUI->isControlModeDirect(game->singlePlayerNumber))
			{
				// TODO: || gameUI->isMessageBoxVisible() ?????
				if (game->gameUI->isAnyIngameWindowVisible())
				{
					ogui->SetCursorImageState(cl, DH_CURSOR_ARROW);
				} else {
					if (game->gameUI->getFirstPerson(cl)->isClipReloading())
					{
						ogui->SetCursorImageState(cl, DH_CURSOR_PLAYER1_RELOADING + cl);
					} else {			 
						ogui->SetCursorImageState(cl, DH_CURSOR_PLAYER1_AIM + cl);
					}
				}
				return;
			}
		}
#endif
#endif
		
#ifdef PROJECT_SHADOWGROUNDS
		if (!cursorOnScene) return;
		
		// TODO: client number
		SceneSelection *sel = game->gameUI->getSceneSelection(0);
		
		if (tacticalClickExpected)
		{
			ogui->SetCursorImageState(0, tacticalClickCursorType);
			return;
		}

		if (game->unitSelections[player]->getUnitsSelected() > 0 
			&& game->gameUI->getController(player)->isKeyDown(DH_CTRL_FORCE_ATTACK)) 
		{
			ogui->SetCursorImageState(0, DH_CURSOR_AIM_HEAVY);
		} else {
			if (sel->hit && sel->unit != NULL)
			{
				if (sel->unit->getOwner() != player)
				{
					if (game->isHostile(player, sel->unit->getOwner())
						&& sel->unit->visibility.isSeenByPlayer(player))
					{
						if (SimpleOptions::getBool(DH_OPT_B_TARGET_BASED_WEAPON_CHOOSE)
							&& sel->unit->getUnitType()->isVehicle())
							ogui->SetCursorImageState(0, DH_CURSOR_AIM_ALL);
						else
							ogui->SetCursorImageState(0, DH_CURSOR_AIM);
					} else {
						if (game->unitSelections[player]->getUnitsSelected() > 0)
						{
							if (game->gameUI->getController(player)->isKeyDown(DH_CTRL_SPECIAL_MOVE))
								ogui->SetCursorImageState(0, DH_CURSOR_SNEAK_TO);
							else
								ogui->SetCursorImageState(0, DH_CURSOR_MOVE_TO);
						} else {
							ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
						}
					}
				} else {
					ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
				}
			} else {
				if (game->unitSelections[player]->getUnitsSelected() > 0)
				{
					if (game->gameUI->getController(player)->isKeyDown(DH_CTRL_SPECIAL_MOVE))
						ogui->SetCursorImageState(0, DH_CURSOR_SNEAK_TO);
					else
						ogui->SetCursorImageState(0, DH_CURSOR_MOVE_TO);
				} else {
					ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
				}
			}
		}
#endif

	}
	
	
	void CombatWindow::doCombatControls(int timeDelta)
	{
		GameController *gameController = game->gameUI->getController(player);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_UNIT_1))
			doUnitSelectionByNumber(0);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_UNIT_2))
			doUnitSelectionByNumber(1);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_UNIT_3))
			doUnitSelectionByNumber(2);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_UNIT_4))
			doUnitSelectionByNumber(3);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_UNIT_5))
			doUnitSelectionByNumber(4);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_UNIT_6))
			doUnitSelectionByNumber(5);
		if (gameController->wasKeyClicked(DH_CTRL_SELECT_ALL_UNITS))
			doAllUnitSelection();
		if (gameController->wasKeyClicked(DH_CTRL_STOP)
			|| gameController->wasKeyClicked(DH_CTRL_CEASE_FIRE)
			|| gameController->wasKeyClicked(DH_CTRL_STOP_AND_CEASE_FIRE))
		{
			std::vector<Unit *> unitVector;
			LinkedList *ulist = game->units->getOwnedUnits(player);
			ulist->resetIterate();
			while (ulist->iterateAvailable())
			{
				Unit *u = (Unit *)ulist->iterateNext();
				if (u->isActive() && u->isSelected())
				{
					unitVector.push_back(u);
				}
			}			
			if (gameController->wasKeyClicked(DH_CTRL_STOP)
				|| gameController->wasKeyClicked(DH_CTRL_STOP_AND_CEASE_FIRE))
				game->formations.clearMovePoint(&unitVector);
			if (gameController->wasKeyClicked(DH_CTRL_CEASE_FIRE)
				|| gameController->wasKeyClicked(DH_CTRL_STOP_AND_CEASE_FIRE))
				game->formations.clearTarget(&unitVector);
			game->gameUI->setPointersChangedFlag(game->singlePlayerNumber);
		}

		if (gameController->wasKeyClicked(DH_CTRL_SELECT_NEXT_UNIT))
		{
			LinkedList *ulist = game->units->getOwnedUnits(player);
			ulist->resetIterate();
			bool passedCurrent = false;
			bool foundSome = false;
			Unit *firstOne = NULL;
			while(ulist->iterateAvailable())
			{
				Unit *u = (Unit *)ulist->iterateNext();
				if (u->isActive())
				{					
					if (passedCurrent)
					{
						if (game->gameUI->getFirstPerson(0) != NULL)
						{
							game->gameUI->setFirstPerson(player, u, 0);
						} else {
							game->unitSelections[player]->selectAllUnits(false);
							game->unitSelections[player]->selectUnit(u, true);
						}
						foundSome = true;
						break;
					}
					if (firstOne == NULL)
					{
						firstOne = u;
					}
					if (game->gameUI->getFirstPerson(0) != NULL)
					{
						if (u == game->gameUI->getFirstPerson(0))
							passedCurrent = true;
					} else {
						if (u->isSelected())
							passedCurrent = true;
					}
				}
			}
			if (firstOne != NULL && !foundSome)
			{
				if (game->gameUI->getFirstPerson(0) != NULL)
				{
					game->gameUI->setFirstPerson(0, firstOne, 0);
				} else {
					game->unitSelections[player]->selectAllUnits(false);
					game->unitSelections[player]->selectUnit(firstOne, true);
				}
			}
		}

		
		for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
		{
			Unit *fp = game->gameUI->getFirstPerson(c);

			if (fp != NULL)
			{
				if (!game->isPaused())
				{
					// TODO: more efficient way of doing the autoreload
					bool needAutoReload = false;
					for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
					{
						if (fp->getWeaponType(i) != NULL)
						{
							if (fp->isWeaponActive(i))
							{
								if (fp->getWeaponType(i)->isSharedClipAttachment())
								{
									int sharedW = fp->getWeaponForSharedClip(i);
									if (sharedW != -1)
									{
										if (fp->getWeaponAmmoInClip(sharedW) == 0)
// FIXME: shouldn't there be a  fp->getWeaponClipSize(sharedW) > 0  check here??? (like below)
										{
											if(fp->getWeaponType(sharedW) != NULL
												&& fp->getWeaponType(sharedW)->isSingleReloading())
											{
// FIXME: FRAMERATE DEPENDENT MAGIC NUMBER HERE
												if (fp->getClipEmptyTime() < CLIPEMPTY_AUTORELOAD_TIME_LIMIT)
												{
													fp->setClipEmptyTime(fp->getClipEmptyTime() + timeDelta);
												} else {
													needAutoReload = true;
												}
											} else {
												needAutoReload = true;
											}
										}
									}
								} else {
									if (fp->getWeaponAmmoInClip(i) == 0
										&& fp->getWeaponClipSize(i) > 0)
									{
										if(fp->getWeaponType(i) != NULL
											&& fp->getWeaponType(i)->isSingleReloading())
										{
// FIXME: FRAMERATE DEPENDENT MAGIC NUMBER HERE
											if (fp->getClipEmptyTime() < CLIPEMPTY_AUTORELOAD_TIME_LIMIT)
											{
												fp->setClipEmptyTime(fp->getClipEmptyTime() + timeDelta);
											} else {
												needAutoReload = true;
											}
										} else {
											needAutoReload = true;
										}
									}
								}
							}
						}
					}

					if (fp->doesKeepReloading())
					{
						if (fp->getSelectedWeapon() != -1
							&& fp->getWeaponType(fp->getSelectedWeapon()) != NULL
							&& !fp->getWeaponType(fp->getSelectedWeapon())->isSingleReloading()
							&& (fp->getWeaponType(fp->getSelectedWeapon())->getAttachedWeaponType() == NULL
							    || !fp->getWeaponType(fp->getSelectedWeapon())->getAttachedWeaponType()->isSingleReloading()))
						{
							fp->setKeepReloading(false);
						}
					}

					if (game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_RELOAD)
						|| needAutoReload || fp->doesKeepReloading())
					{
						UnitActor *ua = getUnitActorForUnit(fp);
						ua->reload(fp);
					}			

					// medkit...

					if (game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_USE_MEDIKIT))
					{
						if (fp->getHP() < fp->getMaxHP()
							&& fp->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
							&& !fp->isDestroyed())
						{
							game::UnitInventory::useUnitItem(game, fp, "medikit");
						}
					}			

					// flashlight on/off...

					if (game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_FLASHLIGHT))
					{
						if (fp->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
							&& !fp->isDestroyed())
						{
							Flashlight *fl = fp->getFlashlight();
							if (fl != NULL)
							{
								fl->toggleOn();
								VC3 campos = game->gameUI->getGameCamera()->getPosition();
								//if (fl->isFlashlightOn())
								//{
	#ifdef LEGACY_FILES
									game->gameUI->playSoundEffect("Data/Sounds/Defaultfx/flashlight_switch.wav", campos.x, campos.y, campos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_LOW);
	#else
									game->gameUI->playSoundEffect("data/audio/sound/gui/flashlight_switch.wav", campos.x, campos.y, campos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_LOW);
	#endif
								//} else {
								//	game->gameUI->playSoundEffect("Data/Sounds/Defaultfx/flashlight_switch.wav", campos.x, campos.y, campos.z, false, DEFAULT_SOUND_EFFECT_VOLUME);
								//}
							}
						}
					}			

					if (game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_EXECUTE)
						&& fp->getJumpCounter() == 0)
					{
						if (fp->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
							&& !fp->isDestroyed())
						{
							UnitActor *ua = getUnitActorForUnit(fp);
							bool didUnitExecute = ua->doExecute(fp);

							/*
							// if an item pickup script should be run, run the execute
							// script for it instead.
							// FIXME: this method cannot handle multiple units, 
							// just one at a time.
							game->itemManager->setExecuteUnit(fp);
							*/

							bool didItemExecute = game->itemManager->doItemExecute(fp);

							if (!(didUnitExecute || didItemExecute))					
							{
								// does execute use items too (combined use/execute)?
								if (SimpleOptions::getBool(DH_OPT_B_GAME_EXECUTE_USE_SELECTED_ITEM))
								{
									// run default item execute
									game::UnitInventory::useSelectedUnitItem(game, fp);
								}
							}
						}
					}


					if (game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_USE_SELECTED_ITEM))
					{
						if (fp->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
							&& !fp->isDestroyed())
						{
							game::UnitInventory::useSelectedUnitItem(game, fp);
						}
					}


					// weapon changing...

					int weapId = 0;
					int oldWeapId = 0;
					int prevSelection = fp->getSelectedWeapon();

					bool canChangeWeaponImmediately = false;

					if (fp->getJumpCounter() == 0
						&& (prevSelection == -1
							|| ((fp->getFireReloadDelay(prevSelection) == 0
									|| fp->isClipReloading())
								&& fp->getFireWaitDelay(prevSelection) == 0))
						&& fp->getKeepFiringCount() == 0)
					{
						canChangeWeaponImmediately = true;
					}

					if(prevSelection != -1
						&& fp->getWeaponType(prevSelection)
						&& fp->getWeaponType(prevSelection)->usesFireDelayHack())
					{
						canChangeWeaponImmediately = true;
					}

					if (canChangeWeaponImmediately)
					{
						// pending requests (ones that were not completed 
						// immediately because of jump/firing/reload in progress)

						if (fp->getPendingWeaponChangeRequest() != -1)
						{
							weapId = fp->getPendingWeaponChangeRequest();
						}
					}

					// next/prev weapon change

					if (prevSelection != -1 && fp->getWeaponType(prevSelection) != NULL)
					{
						oldWeapId = fp->getWeaponType(prevSelection)->getPartTypeId();
					}

					bool nextWeap = false;
					bool prevWeap = false;
					if (fp->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
						&& !fp->isDestroyed()
						&& !fp->isWeaponRecharging()
						&& !game->isPaused())
					{
						nextWeap = game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_CHANGE_NEXT_WEAPON);
						prevWeap = game->gameUI->getController(c)->wasKeyClicked(DH_CTRL_CHANGE_PREV_WEAPON);
					}

					if (nextWeap || prevWeap)
					{
						int changeDir = 1;
						if (prevWeap)
						{
							changeDir = -1;
						}

						int failCount = 0;
						int wsel = fp->getSelectedWeapon();
						int wselId = 0;
						if (wsel != -1 && fp->getWeaponType(wsel) != NULL)
						{
							wselId = fp->getWeaponType(wsel)->getPartTypeId();
						}
						int curSelection = PlayerWeaponry::getUINumberByWeaponId(fp, wselId);
						int nextSelection = ((curSelection + changeDir + WEAPON_CTRL_AMOUNT) % WEAPON_CTRL_AMOUNT);
						while (nextSelection != curSelection)
						{
							weapId = PlayerWeaponry::getWeaponIdByUINumber(fp, nextSelection);
							int newweap = -1;
							if (weapId != 0)
							{
								newweap = fp->getWeaponByWeaponType(weapId);
								if (!fp->isWeaponOperable(newweap)
									|| (fp->getWeaponAmmoAmount(newweap) == 0 
										&& fp->getWeaponType(newweap) != NULL
										&& fp->getWeaponType(newweap)->getAmmoUsage() > 0))
								{
									newweap = -1;
								}
							}
							if (newweap != -1)
							{
								break;
							} else { 
								weapId = 0;
							}

							nextSelection = ((nextSelection + changeDir + WEAPON_CTRL_AMOUNT) % WEAPON_CTRL_AMOUNT);
							failCount++;
							if (failCount > WEAPON_CTRL_AMOUNT)
							{
								Logger::getInstance()->warning("CombatWindow::doCombatControls - Failed to change next/prev weapon.");
								break;
							}
						}

					}			

					// direct weapon change

					if (fp->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
						&& !fp->isDestroyed()
						&& !fp->isWeaponRecharging())
					{
						// HACK: use direct weapon change buttons only in single player / for player using (kb+)mouse
						if (!SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED)
							|| SimpleOptions::getInt(DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME + c) < GameController::CONTROLLER_TYPE_JOYSTICK1)
						{
							for (int ctrl = 0; ctrl < WEAPON_CTRL_AMOUNT; ctrl++)
							{
								if (game->gameUI->getController(c)->wasKeyClicked(ctrl + DH_CTRL_WEAPON_1))
								{
									weapId = PlayerWeaponry::getWeaponIdByUINumber(fp, ctrl);
									if (weapId != 0)
									{
										int newweap = -1;
										newweap = fp->getWeaponByWeaponType(weapId);
#ifndef PROJECT_SURVIVOR
										if (!fp->isWeaponOperable(newweap))
										{
											newweap = -1;
											weapId = 0;
										}
#endif
										if (newweap != -1 && fp->getWeaponType(newweap))
										{
											Weapon *w = fp->getWeaponType(newweap);
											// not selectable without ammo and no ammo
											if(!w->isSelectableWithoutAmmo() && fp->getWeaponAmmoAmount(newweap) == 0)
											{
												newweap = -1;
												weapId = 0;
											}
										}
									}
								}
							}
						}
					}

					if (weapId != 0 && weapId != oldWeapId)
					{
						if (canChangeWeaponImmediately)
						{
							// reset launching
							if(fp->getLaunchSpeed() > 0.0f)
							{
								if(game->gameUI->getController(c)->isKeyDown(DH_CTRL_GRENADE))
								{
									// hack: grenade key must be released after switching weapon
									fp->setLaunchSpeed(-1.0f);
									fp->setLaunchNow(false);
								}
								else
								{
									fp->setLaunchSpeed(0.0f);
									fp->setLaunchNow(false);
								}
							}
							fp->setPendingWeaponChangeRequest(-1);
							PlayerWeaponry::selectWeapon(game, fp, weapId);
							if (fp->getSpeed() == Unit::UNIT_SPEED_SLOW)
							{
								fp->setSpeed(Unit::UNIT_SPEED_FAST);
							}
						} else {
							fp->setPendingWeaponChangeRequest(weapId);
						}
					}

					/*
					if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_WEAPON_SELECT_MODE))
					{
						int prevSelection = fp->getSelectedWeapon();

						int selectNumber = -1;
						int weapId = 0;
						if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_FORWARD))
						{
							if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT))
							{
								// TODO
							}
							else if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT))
							{
								// TODO
							} else {
								weapId = PARTTYPE_ID_STRING_TO_INT("W_Pistol");
								selectNumber = 0;
							}
						}
						if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_BACKWARD))
						{
							if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT))
							{
								weapId = PARTTYPE_ID_STRING_TO_INT("W_Rocket");
								selectNumber = 3;
							}
							else if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT))
							{
								// TODO
							} else {
								weapId = PARTTYPE_ID_STRING_TO_INT("W_Shotg");
								selectNumber = 4;
							}
						}
						if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT)
							&& selectNumber == -1)
						{
							weapId = PARTTYPE_ID_STRING_TO_INT("W_Rifle");
							selectNumber = 2;
						}
						if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT)
							&& selectNumber == -1)
						{
							weapId = PARTTYPE_ID_STRING_TO_INT("W_MiniG");
							selectNumber = 6;
						}
						int newweap = -1;
						if (weapId != 0)
						{
							newweap = fp->getWeaponByWeaponType(weapId);
						}

						if (game->gameUI->getController(c)->isKeyDown(DH_CTRL_WEAPON_SELECT_MODE))
						{
							if (weaponSelectionWindows[c] == NULL)
							{
								int weapMask = 0;
								if (fp->getWeaponByWeaponType(PARTTYPE_ID_STRING_TO_INT("W_Pistol")) != -1)
									weapMask |= (1 << 0);
								if (fp->getWeaponByWeaponType(PARTTYPE_ID_STRING_TO_INT("W_Rifle")) != -1)
									weapMask |= (1 << 2);
								if (fp->getWeaponByWeaponType(PARTTYPE_ID_STRING_TO_INT("W_Rocket")) != -1)
									weapMask |= (1 << 3);
								if (fp->getWeaponByWeaponType(PARTTYPE_ID_STRING_TO_INT("W_Shotg")) != -1)
									weapMask |= (1 << 4);
								if (fp->getWeaponByWeaponType(PARTTYPE_ID_STRING_TO_INT("W_MiniG")) != -1)
									weapMask |= (1 << 6);

								weaponSelectionWindows[c] = new WeaponSelectionWindow(game, ogui, c, weapMask);

								if (selectNumber == -1)
								{
									int curWeap = -1;
									int selnum = fp->getSelectedWeapon();
									Weapon *wtype = NULL;
									if (selnum != -1) wtype = fp->getWeaponType(selnum);

									if (wtype != NULL)
									{
										if (wtype->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Pistol"))
											curWeap = 0;
										if (wtype->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Rifle"))
											curWeap = 2;
										if (wtype->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Rocket"))
											curWeap = 3;
										if (wtype->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Shotg"))
											curWeap = 4;
										if (wtype->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_MiniG"))
											curWeap = 6;
									}
									weaponSelectionWindows[c]->setSelectedWeapon(curWeap);
								}
							}
							weaponSelectionWindows[c]->setSelectedWeapon(selectNumber);
						}
						
						if (newweap != -1 && newweap != prevSelection)
						{
							fp->setSelectedWeapon(newweap);
							if (fp->isActive())
							{
								for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
								{
									if (w == fp->getSelectedWeapon())
									{
										fp->setWeaponActive(w, true);
										if (fp->getWeaponType(w) != NULL)
										{
											float firingSpread = fp->getFiringSpreadFactor();
											if (firingSpread > fp->getWeaponType(w)->getMaxSpread())
												firingSpread = fp->getWeaponType(w)->getMaxSpread();
											if (firingSpread < fp->getWeaponType(w)->getMinSpread())
												firingSpread = fp->getWeaponType(w)->getMinSpread();
											fp->setFiringSpreadFactor(firingSpread);
										}
									} else {
										fp->setWeaponActive(w, false);
									}
								}
							}

							// FIXME: oh yeah babe, make the right weapon visible..
							// by recreating the whole visual object!
							// _really_ not a very brilliant idea, not at all.
							// (maybe we should just change the visibilities of the
							// weapon objects - now that would be something nice)
							if (fp->isActive())
							{
								//char *stats = game->getGameScene()->getStorm3D()->GetPrintableStatusInfo();
								//Logger::getInstance()->error(stats);
								//game->getGameScene()->getStorm3D()->DeletePrintableStatusInfo(stats);

								if (fp->isMuzzleflashVisible())
									fp->setMuzzleflashVisualEffect(NULL, 0);

								game->deleteVisualOfParts(fp, fp->getRootPart());
								game->createVisualForParts(fp, fp->getRootPart());
								if (fp->getFlashlight() != NULL)
								{
									fp->getFlashlight()->resetOrigin(fp->getVisualObject());
								}

								//stats = game->getGameScene()->getStorm3D()->GetPrintableStatusInfo();
								//Logger::getInstance()->error(stats);
								//game->getGameScene()->getStorm3D()->DeletePrintableStatusInfo(stats);

								fp->setPosition(fp->getPosition());
								VC3 rot = fp->getRotation();
								fp->setRotation(rot.x, rot.y, rot.z);
								if (fp->getWalkDelay() < 1)
									fp->setWalkDelay(1);
								if (fp->getAnimationSet() != NULL)
								{
									fp->setAnimation(0); // ANIM_NONE
									if (fp->getAnimationSet()->isAnimationInSet(ANIM_STAND))
										fp->getAnimationSet()->animate(fp, ANIM_STAND);
								}
							}
						}

					} else {
						if (weaponSelectionWindows[c] != NULL)
						{
							// TODO: time delta?
							if (weaponSelectionWindows[c]->advanceTimeout())
							{
								delete weaponSelectionWindows[c];
								weaponSelectionWindows[c] = NULL;
							}
						}
					}
					*/

				} // /game is not paused
			} // /first person exists
		} // /for each player
	}
	
	
	void CombatWindow::showMessage(const char *message, Visual2D *image, bool rightSide)
	{
		if (rightSide)
		{
			messageWindow->clearMessageTextOnly();
			messageWindowRight->showMessage(message, image);
			if (radar != NULL)
			{
				radarTemporarilyDisabled = true;
				delete radar;
				radar = NULL;
			}
		} else {
			messageWindowRight->clearMessageTextOnly();
			messageWindow->showMessage(message, image);
		}
	}
	
	
	void CombatWindow::showCenterMessage(const char *message)
	{
		centerMessageWindow->showMessage(message, NULL);
	}
	
	
	void CombatWindow::clearMessage()
	{
		messageWindow->clearMessage();
		messageWindowRight->clearMessage();
		if (radarTemporarilyDisabled)
		{
			radarTemporarilyDisabled = false;
			if (radar == NULL)
			{
				radar = new CombatRadar(ogui, game, player, win);
			}
		}

	}


	void CombatWindow::clearCenterMessage()
	{
		centerMessageWindow->clearMessage();
	}


	void CombatWindow::clearHintMessage()
	{
		hintMessageWindow->clearMessage();
	}


	void CombatWindow::clearExecuteTipMessage()
	{
		executeTipMessageWindow->clearMessage();
	}


	void CombatWindow::showHintMessage(const char *message)
	{
		hintMessageWindow->showMessage(message, NULL);
	}
	
	
	void CombatWindow::showExecuteTipMessage(const char *message)
	{
		executeTipMessageWindow->showMessage(message, NULL);
	}
	
	
	void CombatWindow::unitSelectionEvent(game::Unit *unit)
	{
		/*
		for (int i = 0; i < COMBATW_UNITS; i++) 
		{
			if (unitWindows[i] != NULL)
			{
				if (unitWindows[i]->getUnit() == unit)
				{
					unitWindows[i]->updateSelectionInfo();
				}
			}
		}
		*/
		if (game->gameUI->getAniRecorderWindow() != NULL)
		{
			game->gameUI->getAniRecorderWindow()->updateUnitSelections();
		}
		recreatePointers();
	}
	
	
	void CombatWindow::CursorEvent(OguiButtonEvent *eve)
	{
#ifdef PROJECT_SHADOWGROUNDS
		tacticalUnitWindow->update();
#endif

		if (!game->gameUI->isCursorActive(game->singlePlayerNumber))
		{
			return;
		}

#ifdef CRIMSON_MODE
		if (game->gameUI->getFirstPerson(0) != NULL
			&& game->gameUI->isThirdPersonView(game->singlePlayerNumber))
		{
			/*
			game->gameUI->setLeftDirectRotation(false);
			game->gameUI->setRightDirectRotation(false);
			if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
			{
				if (eve->triggerButton->GetId() == COMBATW_AREA_LEFT)
				{
					game->gameUI->setLeftDirectRotation(true);
				}
				if (eve->triggerButton->GetId() == COMBATW_AREA_RIGHT)
				{
					game->gameUI->setRightDirectRotation(true);
				}
			}
			*/
			if (game->gameUI->isControlModeDirect(game->singlePlayerNumber))
				return;
		}
#endif

		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
		{
			// if we have missed the button release
			// (because it was done on some other window)
			// then make sure the selection box gets disabled
			if ((eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK) == 0)
			{
				selectionBox->selectionEnded(
					eve->cursorScreenX, eve->cursorScreenY);
			}

#ifdef PROJECT_SHADOWGROUNDS
			// TODO: split screen, must solve player's cursor number first
			if (eve->triggerButton->GetId() == COMBATW_AREA_FWD)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_FORWARD);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_FORWARD, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_BACK)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_BACKWARD);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_BACKWARD, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_LEFT)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_LEFT);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_LEFT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_RIGHT)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_RIGHT);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_RIGHT, true);
			}
			/*
			if (eve->triggerButton->GetId() == COMBATW_AREA_ROTATELEFT)
			{
			ogui->SetCursorImageState(0, DH_CURSOR_ROTATE_LEFT);
			game->gameUI->getGameCamera()->setMovement(
			GameCamera::CAMERA_MOVE_ROTATE_LEFT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_ROTATERIGHT)
			{
			ogui->SetCursorImageState(0, DH_CURSOR_ROTATE_RIGHT);
			game->gameUI->getGameCamera()->setMovement(
			GameCamera::CAMERA_MOVE_ROTATE_RIGHT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_ORBITLEFT)
			{
			ogui->SetCursorImageState(0, DH_CURSOR_ORBIT_LEFT);
			game->gameUI->getGameCamera()->setMovement(
			GameCamera::CAMERA_MOVE_ORBIT_LEFT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_ORBITRIGHT)
			{
			ogui->SetCursorImageState(0, DH_CURSOR_ORBIT_RIGHT);
			game->gameUI->getGameCamera()->setMovement(
			GameCamera::CAMERA_MOVE_ORBIT_RIGHT, true);
			}
			*/
			if (eve->triggerButton->GetId() == COMBATW_AREA_UPLEFT)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_UP_LEFT);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_FORWARD, true);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_LEFT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_UPRIGHT)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_UP_RIGHT);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_FORWARD, true);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_RIGHT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_DOWNLEFT)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_DOWN_LEFT);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_BACKWARD, true);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_LEFT, true);
			}
			if (eve->triggerButton->GetId() == COMBATW_AREA_DOWNRIGHT)
			{
				ogui->SetCursorImageState(0, DH_CURSOR_DOWN_RIGHT);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_BACKWARD, true);
				game->gameUI->getGameCamera()->setMovement(
					GameCamera::CAMERA_MOVE_RIGHT, true);
			}
#endif
			if (eve->triggerButton->GetId() == COMBATW_AREA_SCREEN)
			{
				cursorOnScene = true; 
			}
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_LEAVE)
		{
			ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
			if (eve->triggerButton->GetId() == COMBATW_AREA_SCREEN)
			{
				cursorOnScene = false;
			}
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_FORWARD, false);
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_BACKWARD, false);
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_LEFT, false);
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_RIGHT, false);

			/*
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_ORBIT_RIGHT, false);
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_ORBIT_LEFT, false);
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_ROTATE_RIGHT, false);
			game->gameUI->getGameCamera()->setMovement(
				GameCamera::CAMERA_MOVE_ROTATE_LEFT, false);
			*/

		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_PRESS)
		{
			dragStartX = eve->cursorScreenX;
			dragStartY = eve->cursorScreenY;
			dragStartTime = Timer::getTime();
			
			// possible selection box started
			if (eve->cursorButtonMask & OGUI_BUTTON_1_MASK)
			{
				if (!selectionBox->selectionActive())
				{
					selectionBox->selectionStarted(
						eve->cursorScreenX, eve->cursorScreenY);
				}
			}
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OUT)
		{
			// cancel, or do what?
			/*
			if (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK)
			{
				selectionBox->selectionEnded(
					eve->cursorScreenX, eve->cursorScreenY);
			}
			*/
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			// possible selection box ended
			if (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK)
			{
				selectionBox->selectionEnded(
					eve->cursorScreenX, eve->cursorScreenY);
			}

			// this is needed if the tactical mode window was just visible
			// and closed by the popup autoclose...
			// if so, we'll skip this click.
			bool tacticalWinWasVisible = false;
#ifdef PROJECT_SHADOWGROUNDS
			if (!tacticalUnitWindow->isVisible()
				&& !tacticalUnitWindow->isClosedAck())
			{
				int now = Timer::getTime();
				if ((abs(eve->cursorScreenX - dragStartX) < 4
					&& abs(eve->cursorScreenY - dragStartY) < 4
					&& now < dragStartTime + 400)
					|| (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK) != 0)
				{
					tacticalWinWasVisible = true;
					tacticalUnitWindow->setClosedAck();
					tacticalUnitWindow->tacticalDone();
				} else {
					// heck, the autoclose will close the window, even
					// though we would prefer for it to remain visible...
					// thus hacking...
					tacticalUnitWindow->keepVisible();
				}
			}
#endif

			if (eve->triggerButton->GetId() == COMBATW_TACTICAL_MODE)
			{
				// TODO
			}

			if (eve->triggerButton->GetId() < COMBATW_AREAS
				&& (!tacticalWinWasVisible
				|| eve->cursorOldButtonMask & OGUI_BUTTON_2_MASK) != 0)
			{
				// TODO: netgame: do a proper game request!
				
				// psd. these checks should be moved here to avoid a lot of code duplication. 
				// should set all movement/firing through formations
				
				if (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK)
				{
					int now = Timer::getTime();
					if (abs(eve->cursorScreenX - dragStartX) < SELECTION_BOX_MIN_SIZE
						&& abs(eve->cursorScreenY - dragStartY) < SELECTION_BOX_MIN_SIZE)
						//&& now < dragStartTime + 400)
					{ 
						// clicked somewhere on scene, pass on to game ui...	
						// TODO: client number		
						SceneSelection *sel = game->gameUI->getSceneSelection(0);

						// was a tactical menu (3rd) click expected?
#ifdef PROJECT_SHADOWGROUNDS
						if (tacticalClickExpected)
						{
							// forget any possible expected tactical clicks...
							tacticalClickExpected = false;
							if (sel->hit)
							{
								VC3 pos = VC3(sel->scaledMapX, 
									game->gameMap->getScaledHeightAt(sel->scaledMapX, sel->scaledMapY), 
									sel->scaledMapY);
								tacticalUnitWindow->doTacticalClick(pos, sel->unit);
							}
							sel->hit = false;
						}
#endif
						if (sel->hit)
						{
							// if we clicked a unit, select it, attack, or something...
							if (sel->unit != NULL)
							{
								// we clicked a unit...
								Unit *selu = sel->unit;
								doUnitClick(selu);
							} else {
								// we did not click a unit (clicked terrain maybe)...
								if (game->unitSelections[player]->getUnitsSelected() > 0)
								{
									if (game->gameUI->getController(player)->
										isKeyDown(DH_CTRL_FORCE_ATTACK)) 
									{
										// shoot the ground
										VC3 targ = VC3(sel->scaledMapX, 
											game->gameMap->getScaledHeightAt(sel->scaledMapX, sel->scaledMapY), 
											sel->scaledMapY);
										doUnitAttack(targ, NULL);
									} else {
										// move selected units there
										VC3 targ = VC3(sel->scaledMapX, 
											game->gameMap->getScaledHeightAt(sel->scaledMapX, sel->scaledMapY), 
											sel->scaledMapY);
										std::vector<Unit *> unitVector;
										LinkedList *ulist = game->units->getOwnedUnits(player);
										ulist->resetIterate();
										while (ulist->iterateAvailable())
										{
											Unit *u = (Unit *)ulist->iterateNext();
											if (u->isActive() && u->isSelected())
											{
												unitVector.push_back(u);
											}
										}
										if (game->gameUI->getController(player)->
											isKeyDown(DH_CTRL_SPECIAL_MOVE))
										{
											game->formations.addMovePoint(&unitVector, targ, game::Unit::MoveTypeStealth);
										} else {
											VC3 targdiff = targ - lastMoveTarget;
											if (now < lastMoveClickTime + 400
												&& targdiff.GetSquareLength() < 3*3
												&& SimpleOptions::getBool(DH_OPT_B_DOUBLE_CLICK_MOVE_FAST))
											{
												game->formations.addMovePoint(&unitVector, targ, game::Unit::MoveTypeFast);	
											} else {
												game->formations.addMovePoint(&unitVector, targ, game::Unit::MoveTypeNormal);
											}
											lastMoveClickTime = now;
											lastMoveTarget = targ;
										}
										unitVector.clear();

										/*
										LinkedList *ulist = game->units->getOwnedUnits(player);
										ulist->resetIterate();
										int finalPointNum = 0;
										float movePointOffsetX[6] = { 0, 4, 2.5f, -4, -2.5f, 0 };
										float movePointOffsetY[6] = { 0, 0, 4, 0, 4, -4 };
										while (ulist->iterateAvailable())
										{
											Unit *u = (Unit *)ulist->iterateNext();
											if (u->isActive() && u->isSelected())
											{
												// go go go!!!
												if (game->gameUI->getController(player)->
													isKeyDown(DH_CTRL_SPECIAL_MOVE))
												{
													u->setSpeed(Unit::UNIT_SPEED_SLOW);
													u->setMode(Unit::UNIT_MODE_HOLD_FIRE);
												} else {
													u->setSpeed(Unit::UNIT_SPEED_FAST);
													u->setMode(Unit::UNIT_MODE_DEFENSIVE);
												}

												UnitActor *ua = getUnitActorForUnit(u);
												
												ua->setPathTo(u, 
													VC3(sel->scaledMapX + movePointOffsetX[finalPointNum],
													0, sel->scaledMapY + movePointOffsetY[finalPointNum]));
												
												finalPointNum = (finalPointNum + 1) % 6;
											}
										}
										*/
										recreatePointers();
									}
								}
							}
						}
					} 
					else 
					{
						selectionBox->selectionEnded(
							eve->cursorScreenX, eve->cursorScreenY);
						
						recreatePointers();
					}
				}
				if (eve->cursorOldButtonMask & OGUI_BUTTON_2_MASK)
				{
					// check that we were not dragging (rotating/orbiting) the view
					// TODO: proper check in case the cursor end up back to drag 
					// starting location.
					int now = Timer::getTime();
					if (abs(eve->cursorScreenX - dragStartX) < 4
						&& abs(eve->cursorScreenY - dragStartY) < 4
						&& now < dragStartTime + 400)
					{
						// forget any possible expected tactical clicks...
						tacticalClickExpected = false;

						std::vector<Unit *> unitsSelected;
						
						for (int i = 0; i < COMBATW_UNITS; i++)
						{
							Unit *u = solveUnitForNumber(i);
							if (u != NULL)
							{
								if (u->isSelected()) 
									unitsSelected.push_back(u);
							}
						}
						
						if(unitsSelected.empty() == false)
						{
							// TODO: client number
							SceneSelection *selection = game->gameUI->getSceneSelection(0);
							if (selection->hit)
							{
#ifdef PROJECT_SHADOWGROUNDS
								// first make sure the previous tactical menu 
								// "session" has properly ended
								tacticalUnitWindow->tacticalDone();
								// open tactical menu
								tacticalUnitWindow->showAt(eve->cursorScreenX, eve->cursorScreenY, &unitsSelected, selection->unit);
#endif
							} else {
								// it did not hit even ground.
								//	tacticalUnitWindow->showAt(eve->cursorScreenX, eve->cursorScreenY, &unitsSelected, Vector2D(selection->scaledMapX, selection->scaledMapY));
							}
						}
					}
				}
			}
		}
	}


	void CombatWindow::doUnitClick(game::Unit *unit)
	{
		if ((game->isHostile(player, unit->getOwner())
			&& unit->visibility.isSeenByPlayer(player))
			|| game->gameUI->getController(player)->
			isKeyDown(DH_CTRL_FORCE_ATTACK)) 
		{
			if (game->unitSelections[player]->getUnitsSelected() > 0)
			{
				// shoot the unit
				doUnitAttack(unit->getPosition(), unit);
			}
		} else {
			// select or unselect the unit we just clicked
			if (unit->getOwner() == player)
			{
				doUnitSelection(unit);
			} else {
				// do nothing.
			}
		}
	}
	
	
	void CombatWindow::doUnitAttack(const VC3 &target, game::Unit *targetUnit)
	{
		// make all selected units attack the target...
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			Unit *u = solveUnitForNumber(i);
			if (u != NULL)
			{
				if (u->isSelected()) 
				{
					if (targetUnit != NULL)
					{
						if (targetUnit == u)
						{
							u->targeting.clearTarget();
						} else {
							if (game->gameUI->getController(player)->isKeyDown(
								DH_CTRL_FORCE_ATTACK))
								u->setWeaponsActiveByFiretype(Unit::FireTypeHeavy);
							else
								u->setWeaponsActiveByFiretype(Unit::FireTypeBasic);
							u->targeting.setTarget(targetUnit);
						}
					} else {
						if (game->gameUI->getController(player)->isKeyDown(
							DH_CTRL_FORCE_ATTACK))
							u->setWeaponsActiveByFiretype(Unit::FireTypeHeavy);
						else
							u->setWeaponsActiveByFiretype(Unit::FireTypeBasic);
						u->targeting.setTarget(target);
					}
				}
			}
		}
		*/
		std::vector<Unit *> unitVector;
		LinkedList *ulist = game->units->getOwnedUnits(player);
		ulist->resetIterate();
		while (ulist->iterateAvailable())
		{
			Unit *u = (Unit *)ulist->iterateNext();
			if (u->isActive() && u->isSelected())
			{
				unitVector.push_back(u);
			}
		}
		game::Unit::FireType ft;
		if (game->gameUI->getController(player)->isKeyDown(
			DH_CTRL_FORCE_ATTACK))
		{
			ft = game::Unit::FireTypeHeavy;
		} else {
			if (SimpleOptions::getBool(DH_OPT_B_TARGET_BASED_WEAPON_CHOOSE)
				&& targetUnit->getUnitType()->isVehicle())
				ft = game::Unit::FireTypeAll;
			else
				ft = game::Unit::FireTypeBasic;
		}
		if (targetUnit != NULL)
		{
			game->formations.setTarget(&unitVector, targetUnit, ft);
		} else {
			game->formations.setTarget(&unitVector, target, ft);
		}
		unitVector.clear();

		recreatePointers();
	}
	
	void CombatWindow::doUnitSelectionByNumber(int unitNum)
	{
		Unit *u = solveUnitForNumber(unitNum);
		doUnitSelection(u);
	}
	
	void CombatWindow::doUnitSelection(Unit *unit)
	{
		if (unit != NULL)
		{
			int unitNum = solveNumberForUnit(unit);
			
			if (unitNum != -1)
			{
				/*
				assert(unitWindows[unitNum] != NULL);
				unitWindows[unitNum]->doUnitSelection();
				*/
			} else {
				game->unitSelections[player]->selectAllUnits(false);
				game->unitSelections[player]->selectUnit(unit, true);
#ifdef CRIMSON_MODE
				if (game->gameUI->getFirstPerson(0) == unit)
					game->unitSelections[player]->selectUnit(unit, false);
#endif
			}
		}
	}
	
	
	void CombatWindow::doAllUnitSelection()
	{
		bool selectAll = false;
		LinkedList *ulist = game->units->getOwnedUnits(player);
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u != NULL)
			{
				if (!u->isSelected() && !u->isDestroyed()
					&& u->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
				{
					selectAll = true;
					break;
				}
			}
		}
		game->unitSelections[player]->selectAllUnits(selectAll);
#ifdef CRIMSON_MODE
		if (game->gameUI->getFirstPerson(0) != NULL)
			game->unitSelections[player]->selectUnit(game->gameUI->getFirstPerson(0), false);
#endif
	}
	
	
	void CombatWindow::recreateUnitSelections()
	{
		game->unitSelections[player]->reset();
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				unitWindows[i]->updateSelectionInfo();
				//unitWindows[i]->updateWeaponInfo();
			}
		}
		*/
	}
	
	
	void CombatWindow::recreatePointers()
	{
		// TODO: optimize, as usual...
		
		gamePointers->clearPointers();

		if (game->isCinematicScriptRunning())
		{
			return;
		}
		
#ifdef CRIMSON_MODE
		if (game->gameUI->getFirstPerson(0) != NULL
			&& !game->gameUI->isThirdPersonView(game->singlePlayerNumber)
			&& !game->isTacticalMode())
		{
			return;
		}
#else
		if (game->gameUI->getFirstPerson(0) != NULL
			&& !game->isTacticalMode())
		{
			return;
		}
#endif
		
		LinkedList *ulist;
		// cheat?
		if (SimpleOptions::getBool(DH_OPT_B_SHOW_ENEMY_TACTICAL))
		{
			// show pointers for even enemies!
			ulist = game->units->getAllUnits();
		} else {
			ulist = game->units->getOwnedUnits(game->singlePlayerNumber);
		}
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u != NULL && u->isActive())
			{
#ifdef CRIMSON_MODE
				assert(MAX_PLAYERS_PER_CLIENT == 4);
				if (!u->isDestroyed() 
					&& u != game->gameUI->getFirstPerson(0)
					&& u != game->gameUI->getFirstPerson(1)
					&& u != game->gameUI->getFirstPerson(2)
					&& u != game->gameUI->getFirstPerson(3))
#else
				if (!u->isDestroyed())
#endif
				{
					if (u->isSelected())
					{
						gamePointers->addPointer(u->getPosition(), 
							GPOINTER_SELECTED, u, NULL);
					} else {
						if (game->isTacticalMode())
						{
							gamePointers->addPointer(u->getPosition(), 
								GPOINTER_UNSELECTED, u, NULL);
						}
					}

					if (u->targeting.hasTarget()
						&& !u->targeting.isAutoTarget())
					{
						Unit *lineFrom = NULL;
						if (game->isTacticalMode())
							lineFrom = u;

						Weapon *w1 = u->getWeaponType(0);
						Weapon *w2 = u->getWeaponType(1);
						float maxDist = 0;
						if (u->isWeaponActive(0))
						{
							if (w1 != NULL)
							{
								if (w1->getRange() > maxDist)
									maxDist = w1->getRange();
							}
						}
						if (u->isWeaponActive(1))
						{
							if (w2 != NULL)
							{
								if (w2->getRange() > maxDist)
									maxDist = w2->getRange();
							}
						}

						VC3 tpos;
						if (u->targeting.getTargetUnit() != NULL)
						{
							tpos = u->targeting.getTargetUnit()->getPointerPosition();
						} else {
							tpos = u->targeting.getTargetPosition();
							// HACK: lower the target to ground
							// raised back up if heavy weapon.
							tpos.y -= 2.0f;
						}

						if ((w1 != NULL && u->isWeaponActive(0)
							&& w1->isHeavyWeapon())
							|| (w2 != NULL && u->isWeaponActive(1)
							&& w2->isHeavyWeapon()))
						{
							if (u->targeting.getTargetUnit() == NULL)
								tpos.y += 2.0f;
							if (u->isSelected())
							{
								gamePointers->addPointer(tpos, 
									GPOINTER_GROUNDTARGET, u->targeting.getTargetUnit(), lineFrom, (float)maxDist);
							} else {
								gamePointers->addPointer(tpos, 
									GPOINTER_UNSEL_GROUNDTARGET, u->targeting.getTargetUnit(), lineFrom, (float)maxDist);
							}
						} else {
							if (u->isSelected())
							{
								gamePointers->addPointer(tpos, 
									GPOINTER_TARGET, u->targeting.getTargetUnit(), lineFrom, (float)maxDist);
							} else {
								gamePointers->addPointer(tpos, 
									GPOINTER_UNSEL_TARGET, u->targeting.getTargetUnit(), lineFrom, (float)maxDist);
							}
						}
					}
										
					if (!u->atFinalDestination())
					{
						Unit *lineFrom = NULL;
						if (game->isTacticalMode())
							lineFrom = u;

						if (u->getSpeed() == Unit::UNIT_SPEED_SLOW
							|| u->isStealthing())
						{
							if (u->isSelected())
							{
								gamePointers->addPointer(u->getFinalDestination(), 
									GPOINTER_SNEAKPOINT, NULL, lineFrom);
							} else {
								gamePointers->addPointer(u->getFinalDestination(), 
									GPOINTER_UNSEL_SNEAKPOINT, NULL, lineFrom);
							}
						}
						else if (u->getSpeed() == Unit::UNIT_SPEED_SPRINT)
						{
							if (u->isSelected())
							{
								gamePointers->addPointer(u->getFinalDestination(), 
									GPOINTER_SPRINTPOINT, NULL, lineFrom);
							} else {
								gamePointers->addPointer(u->getFinalDestination(), 
									GPOINTER_UNSEL_SPRINTPOINT, NULL, lineFrom);
							}
						} else {
							if (u->isSelected())
							{
								gamePointers->addPointer(u->getFinalDestination(), 
									GPOINTER_FINALPOINT, NULL, lineFrom);
							} else {
								gamePointers->addPointer(u->getFinalDestination(), 
									GPOINTER_UNSEL_FINALPOINT, NULL, lineFrom);
							}
						}
						
						if (SimpleOptions::getBool(DH_OPT_B_SHOW_PATHS))
						{
							if (game->isTacticalMode())
							{
								if (!u->isAtPathEnd())
								{
									frozenbyte::ai::Path *path = u->getPath();
									for (int i = u->getPathIndex(); i < path->getSize(); i += 1)
									{
										float x = game->gameMap->pathfindToScaledX(path->getPointX(i));
										float y = game->gameMap->pathfindToScaledY(path->getPointY(i));
										VC3 wp = VC3(x, game->gameMap->getScaledHeightAt(x,y), y);
										gamePointers->addPointer(wp, GPOINTER_WAYPOINT, NULL, NULL);
									}
								}
							}
						}
					}

				}
			}
		}

		// others than player...				
		ulist = game->units->getAllUnits();
		iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed())
			{
				if (u->getOwner() != player 
					//&& (u->visibility.isInRadarByPlayer(player)
					//|| u->visibility.isSeenByPlayer(player)))
					&& u->visibility.isSeenByPlayer(player))
				{
					// only small units get the torus
					// but not really small.
					float size = u->getUnitType()->getSize();
					if (size > 0.2f && size < 3.0f)
					{
						if (game->isTacticalMode())
						{							
							if (game->isHostile(player, u->getOwner()))
								gamePointers->addPointer(u->getPosition(), 
								GPOINTER_ENEMY, u, NULL);
							else
								gamePointers->addPointer(u->getPosition(), 
								GPOINTER_FRIENDLY, u, NULL);
						} else {
							/*
							if (game->isHostile(player, u->getOwner()))
								gamePointers->addPointer(u->getPosition(), 
								GPOINTER_HOSTILE_SIGHT, u, NULL);
							*/
						}
					}
				}
			}
		}
	}
			
	void CombatWindow::updateUnitPointers()
	{
		if( targetDisplayWindow )
			targetDisplayWindow->update();
	}
	
	void CombatWindow::updatePointers()
	{
		gamePointers->updatePositions();
	}
	
	
	void CombatWindow::renderPointers()
	{
		gamePointers->prepareForRender();
	}
	
	
	void CombatWindow::updateHPMeters()
	{
		// TODO: really need to optimize!!!
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				unitWindows[i]->updateHPInfo();
			}
		}
		*/
	}
	
	void CombatWindow::updateMeters()
	{
		// TODO: really need to optimize!!!
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				unitWindows[i]->updateMiscInfo();
				
				unitWindows[i]->updateWeaponInfo();				
			}
		} 
		*/
#ifdef PROJECT_SURVIVOR
		int value = 0;
		if(util::Script::getGlobalIntVariableValue("survival_mode_enabled", &value) && value == 1)
		{
			if(!game->isPaused())
			{
				// find longest survival time...
				int longest_time = 0;
				for( int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++ )
				{
					if(!SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + i )) continue;

					int time = game::GameStats::instances[i]->getSurvivalTime();
					if(time > longest_time) longest_time = time;
				}
				timerWindow->showMessage(time2str(longest_time), NULL);
			}
		}
#endif

		std::map< std::string, ICombatSubWindow* >::iterator i;
		for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
		{
			i->second->update();
		}

		hintMessageWindow->update();

		// if( unitHealthBar )
		// {
		// unitHealthBar->update();
		// }
	}


	void CombatWindow::updateGUIAnimations()
	{
		// healthWindow->updateCurve();
		if( healthWindow )
			healthWindow->updateAnimation();
	}
	
	void CombatWindow::updateCameraDependedElements()
	{
		if( targetDisplayWindow )
			targetDisplayWindow->update();
	}

	void CombatWindow::updateModeInfo()
	{
		// TODO: really need to optimize!!!
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i] != NULL)
			{
				unitWindows[i]->updateModeInfo();
			}
		} 
		*/
	}

			
	int CombatWindow::solveNumberForUnit(game::Unit *unit)
	{
		/*
		for (int i = 0; i < COMBATW_UNITS; i++)
		{
			if (unitWindows[i]->getUnit() == NULL) return -1;
			if (unitWindows[i]->getUnit() == unit) return i;
		}
		*/
		return -1;
	}
	
	game::Unit *CombatWindow::solveUnitForNumber(int number)
	{
		/*
		assert(number < COMBATW_UNITS);
		if (unitWindows[number] == NULL)
			return NULL;
		else
			return unitWindows[number]->getUnit();
		*/
		return NULL;
	}

	void CombatWindow::setTacticalModeButton(bool tactical)
	{
		if (tactical)
		{
			if (tacticalModeBut == NULL)
			{
				tacticalModeBut = ogui->CreateSimpleTextButton(win, 
					4, UNITSTAT_BOTTOM_START_Y - 20, 64, 16, NULL, NULL, NULL, 
					"TACTICAL MODE", COMBATW_TACTICAL_MODE);
				tacticalModeBut->SetFont(ui::defaultIngameFont);
				tacticalModeBut->SetLineBreaks(true);
				tacticalModeBut->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
				tacticalModeBut->SetListener(this);
			}
		} else {
			if (tacticalModeBut != NULL)
			{
				delete tacticalModeBut;
				tacticalModeBut = NULL;
			}			
		}
	}
			
	void CombatWindow::updateUnitHighlight()
	{
		unitHighlight->run();
	}
			
	void CombatWindow::setUnitHighlight(const game::Unit *unit)
	{
		if (!highlightLocked)
		{
			float size = 1.0f;
			if (unit != NULL) 
				size = unit->getUnitType()->getSize();
			if (size > 0.2f && size < 3.0f)
			{
				unitHighlight->setHighlightedUnit(unit);
			} else {
				unitHighlight->setHighlightedUnit(NULL);
			}
		}
	}

	void CombatWindow::setTerrainHighlight(VC3 &position)
	{
		if (!highlightLocked)
			unitHighlight->setHighlightedTerrain(position);
	}

	void CombatWindow::clearHighlight()
	{
		if (!highlightLocked)
			unitHighlight->clearHighlightedTerrain();
	}

	void CombatWindow::lockHighlight()
	{
		highlightLocked = true;
	}

	void CombatWindow::unlockHighlight()
	{
		highlightLocked = false;
	}

	void CombatWindow::openUnitHealthBar( Unit* unit )
	{
		if( unitHealthBar == NULL )
		{
			openSubWindow( "UnitHealthBarWindow", player );
			// unitHealthBar = windowFactory->CreateNewWindow( "UnitHealthBarWindow", ogui, game, player );
		}
		
		if( unitHealthBar )
			((UnitHealthBarWindow*)unitHealthBar )->setUnit( unit );
			
	}

	void CombatWindow::setUnitHealthBarFlashing(int value)
	{
		if( unitHealthBar != NULL)
		{
			((UnitHealthBarWindow*)unitHealthBar )->setFlashing(value);
		}
	}

	void CombatWindow::closeUnitHealthBar()
	{
		// delete unitHealthBar;
		closeSubWindow( "UnitHealthBarWindow" );
		unitHealthBar = NULL;
	}

	void CombatWindow::hideFlashlight()
	{
		closeSubWindow( "FlashlightWindow" );
	}
	
	void CombatWindow::showFlashlight()
	{
		openSubWindow( "FlashlightWindow", player );	
	}


	void CombatWindow::raiseMessages()
	{
		messageWindow->raise();
		messageWindowRight->raise();
		hintMessageWindow->raise();
		executeTipMessageWindow->raise();
		centerMessageWindow->raise();
	}
	
	HealthWindow* CombatWindow::getHealthWindow() const
	{
		return (HealthWindow*)healthWindow;
	}

	void CombatWindow::openSubWindow( const std::string& window_name, int player_num )
	{
#ifdef PROJECT_SURVIVOR
		// reset last opened window
		GenericTextWindow::last_opened_window = NULL;
#endif

		std::map< std::string, ICombatSubWindow* >::iterator i;
		i = subWindowMap.find( window_name );
		if( i == subWindowMap.end() )
		{
			ICombatSubWindow* temp = windowFactory->CreateNewWindow( window_name, ogui, game, player_num );
			if( temp != NULL )
			{
				subWindowMap.insert( std::pair< std::string, ICombatSubWindow* >( window_name, temp ) );

				if(guiTempInvisible || !guiVisible)
				{
					temp->hide();
				}

				if( window_name == "HealthWindow" )
				{
					healthWindow = temp;
				}
				else if( window_name == "HealthWindowCoop" )
				{
					healthWindow = temp;
				}
				else if( window_name == "TargetDisplayWindow" )
				{
					targetDisplayWindow = temp;
				}
				else if( window_name == "UnitHealthBarWindow" )
				{
					unitHealthBar = temp;
				}
			}
			else
			{
				if( window_name == "CombatRadar" )
				{
					setRadarDisabled(false);
				}
			}
		}
	}

	void CombatWindow::closeSubWindow( const std::string& window_name )
	{
		std::map< std::string, ICombatSubWindow* >::iterator i;
		i = subWindowMap.find( window_name );
		if( i != subWindowMap.end() )
		{
			delete i->second;
			subWindowMap.erase( i );
		}
		else
		{
			if( window_name == "CombatRadar" )
			{
				setRadarDisabled(true);
			}
		}

		// if( window_name == "UnitHealthBarWindow" )
		// {
		//	unitHealthBar = NULL;
		// }
	}

	ICombatSubWindow* CombatWindow::getSubWindow( const std::string& window_name ) const 
	{
		std::map< std::string, ICombatSubWindow* >::const_iterator i;
		i = subWindowMap.find( window_name );
		if( i != subWindowMap.end() )
		{
			return i->second;
		}
		else
		{
			return NULL;
		}
	}

	void CombatWindow::setSubWindowsVisible(bool visible, bool radar_visible)
	{
		std::map< std::string, ICombatSubWindow* >::iterator i;
		for( i = subWindowMap.begin(); i != subWindowMap.end(); ++i )
		{
			if(!visible) i->second->hide();
			else i->second->show();
		}

		if(!radar_visible)
		{
			radarWasDisabled = radarDisabled;
			radarDisabled = true;

			if (radar != NULL)
			{
				delete radar;
				radar = NULL;
			}
		}
		else
		{
			if(!radarWasDisabled)
			{
				radarDisabled = false;
				if (radar == NULL)
				{
					if (!radarTemporarilyDisabled)
					{
						radar = new CombatRadar(ogui, game, player, win);
					}
				}
			}
		}
	}


	void CombatWindow::setRadarDisabled(bool disabled)
	{
		radarDisabled = disabled;

		if(disabled && radar != NULL)
		{
			delete radar;
			radar = NULL;
		}
		else if(!disabled && radar == NULL && !radarTemporarilyDisabled)
		{
			radar = new CombatRadar(ogui, game, player, win);
		}
	}

	bool CombatWindow::hasMessageWindow() const
	{
		if(messageWindow != NULL && messageWindow->hasMessage())
			return true;

		if(messageWindowRight != NULL && messageWindowRight->hasMessage())
			return true;

		return false;
	}
}
