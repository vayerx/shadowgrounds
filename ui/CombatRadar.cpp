
#include "precompiled.h"

#include "CombatRadar.h"

#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/Checkpoints.h"
#include "../game/scaledefs.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/UnitList.h"
#include "../ui/MapWindow.h"
#include "../convert/str2int.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_gui.h"
#include "../ogui/OguiAligner.h"

#include "../util/Debug_MemoryManager.h"

// button id's
// MUST NOT OVERLAP ONES DEFINED IN COMBATWINDOW!!!
#define COMBATW_RADAR 400
#define COMBATW_RADARUNIT_START 500
#define COMBATW_RADARUNIT_END 599


#define COMBAT_RADAR_MAX_UNITS 50

#define RADAR_SIZE 192
#define RADAR_EDGE 6
//#define RADAR_SIZE 128
//#define RADAR_EDGE 4
#define RADAR_RANGE 40

#define RADAR_UNITTYPE_FRIENDLY_SMALL 1
#define RADAR_UNITTYPE_FRIENDLY_BIG 2
#define RADAR_UNITTYPE_HOSTILE_SMALL 3
#define RADAR_UNITTYPE_HOSTILE_BIG 4

// everything bigger than 2 meters is big, others small
#define RADAR_BIG_SIZE 2.0f
// everything below this size won't be visible in radar
#define RADAR_MIN_SIZE 0.5f

#define RADAR_POINTER_ANGLE_STEP 1

#define RADAR_SWEEP_TICK_INTERVAL 128
#define RADAR_SWEEP_TICK_DURATION 92


using namespace game;

namespace ui
{

  CombatRadar::CombatRadar(Ogui *ogui, game::Game *game, int player, OguiWindow *window)
  {
    this->ogui = ogui;
    this->game = game;
    this->player = player;
    this->win = window;

    angle = 0;

    radarUnitsAmount = 0;
    radarUnitButs = new OguiButton *[COMBAT_RADAR_MAX_UNITS];
    radarUnits = new Unit *[COMBAT_RADAR_MAX_UNITS];
    radarUnitDist = new float[COMBAT_RADAR_MAX_UNITS];
    radarUnitAngle = new float[COMBAT_RADAR_MAX_UNITS];
    radarUnitType = new int[COMBAT_RADAR_MAX_UNITS];

		int i;
    for (i = 0; i < COMBAT_RADAR_MAX_UNITS; i++)
    { 
      radarUnitButs[i] = NULL;
			radarUnits[i] = NULL;
    }

#ifdef LEGACY_FILES
    friendly1Image = ogui->LoadOguiImage("Data/GUI/Ingame/radar_friendly1.tga");
    friendly2Image = ogui->LoadOguiImage("Data/GUI/Ingame/radar_friendly2.tga");
    hostile1Image = ogui->LoadOguiImage("Data/GUI/Ingame/radar_hostile1.tga");
    hostile2Image = ogui->LoadOguiImage("Data/GUI/Ingame/radar_hostile2.tga");
		radarBackground = ogui->LoadOguiImage("Data/GUI/Ingame/radar_background.tga");
#else
    friendly1Image = ogui->LoadOguiImage("data/gui/hud/radar/friendly1.tga");
    friendly2Image = ogui->LoadOguiImage("data/gui/hud/radar/friendly2.tga");
    hostile1Image = ogui->LoadOguiImage("data/gui/hud/radar/hostile1.tga");
    hostile2Image = ogui->LoadOguiImage("data/gui/hud/radar/hostile2.tga");
		radarBackground = ogui->LoadOguiImage("data/gui/hud/radar/background.tga");
#endif

    OguiButton *b;
    b = ogui->CreateSimpleImageButton(win, 1024-RADAR_SIZE, 0, RADAR_SIZE, RADAR_SIZE, 
      NULL, NULL, NULL, COMBATW_RADAR);
		b->SetDisabledImage(radarBackground);
    b->SetListener(this);
    b->SetReactMask(0);
    b->SetEventMask(OGUI_EMASK_CLICK);
		b->SetDisabled(true);
    radarBut = b;

#ifdef LEGACY_FILES
    b = ogui->CreateSimpleImageButton(win, 1024-RADAR_SIZE, 0, RADAR_SIZE, RADAR_SIZE, 
      NULL, NULL, NULL, "Data/GUI/Ingame/radar_scan.tga", COMBATW_RADAR);
#else
    b = ogui->CreateSimpleImageButton(win, 1024-RADAR_SIZE, 0, RADAR_SIZE, RADAR_SIZE, 
      NULL, NULL, NULL, "data/gui/hud/radar/scan.tga", COMBATW_RADAR);
#endif
    b->SetListener(this);
    b->SetReactMask(0);
		b->SetDisabled(true);
		b->SetTransparency(0);
    radarScanBut = b;

    for (i = 0; i < COMBAT_RADAR_DIRECTION_IMAGES; i++)
    { 
			char buf[128];
#ifdef LEGACY_FILES
			strcpy(buf, "Data/GUI/Ingame/radarDir/dir_");
#else
			strcpy(buf, "data/gui/hud/radar/dir_");
#endif
			strcat(buf, int2str(i));
      strcat(buf, ".tga");
      directionImages[i] = ogui->LoadOguiImage(buf);
    }

		directionBut = NULL;

		radarBeeped = false;
		radarSweepNumber = 0;

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(radarBut, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		OguiAligner::align(radarScanBut, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
	}

  CombatRadar::~CombatRadar()
  {
    if (directionBut != NULL)
    {
      delete directionBut;
      directionBut = NULL;
    }

    if (radarBut != NULL)
    {
      delete radarBut;
      radarBut = NULL;
    }
    if (radarScanBut != NULL)
    {
      delete radarScanBut;
      radarScanBut = NULL;
    }
    if (radarBackground != NULL) delete radarBackground;
    for (int i = 0; i < COMBAT_RADAR_MAX_UNITS; i++)
    { 
      if (radarUnitButs[i] != NULL)
        delete radarUnitButs[i];
    }
    if (friendly1Image != NULL) delete friendly1Image;
    if (friendly2Image != NULL) delete friendly2Image;
    if (hostile1Image != NULL) delete hostile1Image;
    if (hostile2Image != NULL) delete hostile2Image;

    delete [] radarUnitButs;
    delete [] radarUnits;
    delete [] radarUnitDist;
    delete [] radarUnitAngle;
    delete [] radarUnitType;
  }

  void CombatRadar::CursorEvent(OguiButtonEvent *eve)
  {
    // TODO
  }

  void CombatRadar::update()
  {
		if (game->isPaused())
			return;

    int oldAmount = radarUnitsAmount;
    radarUnitsAmount = 0;

		LinkedList *ulist;

		if (SimpleOptions::getBool(DH_OPT_B_GUI_RADAR_SHOW_ALL))
		{
			ulist = game->units->getAllUnits();
		} else {
			ulist = game->units->getOwnedUnits(1);
		}

    LinkedListIterator iter = LinkedListIterator(ulist);

    while (iter.iterateAvailable())
    {
      Unit *u = (Unit *)iter.iterateNext();
      if (u->isActive() && !u->isDestroyed()
        && u->visibility.isInRadarByPlayer(player)
//        && u->visibility.isSeenByPlayer(player)
        && u->getUnitType()->getSize() >= RADAR_MIN_SIZE)
      {
        VC3 pos = u->getPosition();
        pos.x -= radarX;
        pos.z -= radarY;
        float distSq = pos.x * pos.x + pos.z * pos.z;
        if (distSq < RADAR_RANGE * RADAR_RANGE)
        {
          float dist = sqrtf(distSq);
          radarUnitDist[radarUnitsAmount] = dist * (RADAR_SIZE/2-RADAR_EDGE) / RADAR_RANGE;
					float angle;
					if (dist == 0)
						angle = 0;
					else
            angle = (float)acos(pos.x / dist);
          if (pos.z < 0) angle = -angle;
          radarUnitAngle[radarUnitsAmount] = angle;
          radarUnits[radarUnitsAmount] = u;

          OguiButton *b;
          if (radarUnitButs[radarUnitsAmount] == NULL)
          {
            b = ogui->CreateSimpleImageButton(win, 
              RADAR_SIZE/2-5, RADAR_SIZE/2-5, 9, 9, 
              NULL, NULL, NULL, COMBATW_RADARUNIT_START + radarUnitsAmount);
#ifdef PROJECT_SURVIVOR
						OguiAligner::align(b, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
            radarUnitButs[radarUnitsAmount] = b;
          } else {
            b = radarUnitButs[radarUnitsAmount];
          }
          b->SetListener(this);
          b->SetReactMask(0);
          b->SetEventMask(OGUI_EMASK_CLICK);
					b->SetDisabled(true);
          if (game->isHostile(player, u->getOwner()))
          {
            if (u->getUnitType()->getSize() >= RADAR_BIG_SIZE)
            {
              b->SetDisabledImage(hostile2Image);
              radarUnitType[radarUnitsAmount] = RADAR_UNITTYPE_HOSTILE_BIG;
            } else {
              b->SetDisabledImage(hostile1Image);
              radarUnitType[radarUnitsAmount] = RADAR_UNITTYPE_HOSTILE_SMALL;
            }
          } else {
						if (u->getOwner() == player)
						{
							b->SetDisabledImage(NULL);
							radarUnitType[radarUnitsAmount] = RADAR_UNITTYPE_FRIENDLY_SMALL;
						} else {
							if (u->getUnitType()->getSize() >= RADAR_BIG_SIZE)
							{
								b->SetDisabledImage(friendly2Image);
								radarUnitType[radarUnitsAmount] = RADAR_UNITTYPE_FRIENDLY_BIG;
							} else {
								b->SetDisabledImage(friendly1Image);
								radarUnitType[radarUnitsAmount] = RADAR_UNITTYPE_FRIENDLY_SMALL;
							}
						}
          }

          radarUnitsAmount++;
          if (radarUnitsAmount >= COMBAT_RADAR_MAX_UNITS) break;
        }
      }
    }

    for (int i = radarUnitsAmount; i < oldAmount; i++)
    {
      delete radarUnitButs[i];
      radarUnitButs[i] = NULL;
    }

		if (radarScanBut != NULL)
		{
			int gt = (game->gameTimer % RADAR_SWEEP_TICK_INTERVAL);
			float ratio = (float)gt / RADAR_SWEEP_TICK_DURATION;

			if (ratio > 1.0f) ratio = 1.0f;
			int perc = (int)(ratio * 100.0f);

			int curSweep = game->gameTimer / RADAR_SWEEP_TICK_INTERVAL;
			if (curSweep > radarSweepNumber
				|| curSweep < radarSweepNumber - 5)
			{
				radarSweepNumber = curSweep;
				radarBeeped = false;

				if (win->IsVisible())
				{
#ifdef LEGACY_FILES
#ifndef PROJECT_SURVIVOR
					game->gameUI->playGUISound("Data/Sounds/Defaultfx/Radar/radar_sweep.wav");
#endif
#else
					game->gameUI->playGUISound("data/audio/sound/gui/hud/radar/radar_sweep.wav");
#endif
				}
			}

			int x = 1024 - (int)(RADAR_SIZE * (0.5f + ratio * 0.5f));
			int y = (int)(RADAR_SIZE * (0.5f - ratio * 0.5f));
			int sizeX = (int)(RADAR_SIZE * ratio);
			int sizeY = (int)(RADAR_SIZE * ratio);
			radarScanBut->Move(x, y);
			radarScanBut->Resize(sizeX, sizeY);
			radarScanBut->SetTransparency(perc);
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(radarScanBut, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
		}

    drawImpl();

		updateDirectionPointer();
  }

  void CombatRadar::setAngle(float angle)
  {
    this->angle = angle * (3.1415f/180.0f);
    while (angle > 3.1415f) angle -= 3.1415f;
    drawImpl();
  }

  void CombatRadar::setOrigo(float x, float y)
  {
    this->radarX = x;
    this->radarY = y;
  }

  void CombatRadar::drawImpl()
  {
	int gt = (game->gameTimer % RADAR_SWEEP_TICK_INTERVAL);
	float ratio = (float)gt / RADAR_SWEEP_TICK_DURATION;

	// no clip to 100% here.
	//if (ratio > 1.0f) ratio = 1.0f;
#if (defined LEGACY_FILES && !defined PROJECT_SURVIVOR) || !defined LEGACY_FILES
	int perc = (int)(ratio * 100.0f);
#endif

    for (int i = 0; i < radarUnitsAmount; i++)
    { 
      float a = -(radarUnitAngle[i] - angle);
      int x = (int)(radarUnitDist[i] * sin(a));
      int y = -(int)(radarUnitDist[i] * cos(a));
      //radarUnitButs[i]->Move(RADAR_SIZE/2 + x - 3, 768-RADAR_SIZE/2 + y - 3);
      radarUnitButs[i]->Move(1024-RADAR_SIZE/2 + x - 4, RADAR_SIZE/2 + y - 4);
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(radarUnitButs[i], OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
			if ((float)radarUnitDist[i] > (ratio * (RADAR_SIZE/2-RADAR_EDGE)))
			{
				radarUnitButs[i]->SetTransparency(100);
			} else {
				int vis = (int)(((ratio * (RADAR_SIZE/2-RADAR_EDGE)) - (float)radarUnitDist[i]) / (float)(RADAR_SIZE/2-RADAR_EDGE) * 150.0f);
				if (vis < 0) vis = 0;
				if (vis > 100) vis = 100;
				radarUnitButs[i]->SetTransparency(vis);
				if (!radarBeeped)
				{
					radarBeeped = true;
					if (win->IsVisible())
					{
						if (radarUnitType[i] == RADAR_UNITTYPE_HOSTILE_SMALL
							|| radarUnitType[i] == RADAR_UNITTYPE_HOSTILE_BIG)
						{
#ifdef LEGACY_FILES
#ifndef PROJECT_SURVIVOR
							game->gameUI->playGUISound("Data/Sounds/Defaultfx/Radar/radar_beep.wav", 100 - (perc*3/4));
#endif
#else
							game->gameUI->playGUISound("data/audio/sound/gui/hud/radar/radar_beep.wav", 100 - (perc*3/4));
#endif
						}
					}
				}
			}
    }
  }

	void CombatRadar::updateDirectionPointer()
	{
		bool objExists = false;
		VC3 objPos = VC3(0,0,0);
#ifdef GUI_BUILD_MAP_WINDOW
		if (game->gameUI->getMapWindow() != NULL)
		{
			objExists = game->gameUI->getMapWindow()->getCurrentObjectivePosition(objPos);
		}
#endif

		if (objExists)
		{
			// HACK: set the objective we got from map window to checkpointer.
			game->checkpoints->setCheckpoint(0, objPos.x, objPos.z);

			if (directionBut == NULL)
			{
				directionBut = ogui->CreateSimpleImageButton(win, 0, 0, 16, 16, NULL, NULL, NULL);
				directionBut->SetReactMask(0);
				directionBut->SetDisabled(true);
			}
			VC3 pos = VC3(radarX, 0, radarY);
			float dirangle = game->checkpoints->getAngleToCheckpoint(0, pos);
			float chkpointDist = game->checkpoints->getDistanceToCheckpoint(0, pos);
			if (chkpointDist > RADAR_RANGE)
				chkpointDist = RADAR_RANGE;

      float a = (dirangle - angle);
			// only 8 main angles (xdegrees) accepted...
			a /= (RADAR_POINTER_ANGLE_STEP * 3.1415f/180);
			a = (float)floor(a + 0.5f);
			a *= (RADAR_POINTER_ANGLE_STEP * 3.1415f/180);
      int x = (int)((RADAR_SIZE / 2 - 2) * sin(a));
      int y = (int)((RADAR_SIZE / 2 - 2) * cos(a));

			// NEW: scale by distance...
			x = (int)((x * chkpointDist) / RADAR_RANGE);
			y = (int)((y * chkpointDist) / RADAR_RANGE);

      //directionBut->Move(RADAR_SIZE/2 + x - 8, 768-RADAR_SIZE/2 + y - 8);
      directionBut->Move(1024-RADAR_SIZE/2 + x - 8, RADAR_SIZE/2 + y - 8);
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(directionBut, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
			int inum = (int)(RAD_TO_UNIT_ANGLE(a - (3.1415f / COMBAT_RADAR_DIRECTION_IMAGES)));
			inum = 180 - inum;
			while (inum < 0) inum += 360;
			while (inum >= 360) inum -= 360;
			inum /= (360 / COMBAT_RADAR_DIRECTION_IMAGES);
			assert(inum >= 0 && inum < COMBAT_RADAR_DIRECTION_IMAGES);
			if (inum < 0) inum = 0;
			if (inum >= COMBAT_RADAR_DIRECTION_IMAGES) inum = COMBAT_RADAR_DIRECTION_IMAGES - 1;
			directionBut->SetDisabledImage(directionImages[inum]);
		} else {
			game->checkpoints->disableCheckpoint(0);

			if (directionBut != NULL)
			{
				delete directionBut;
				directionBut = NULL;
			}
		}

		/*
		if (game->checkpoints->isCheckpointEnabled(0))
		{
			if (directionBut == NULL)
			{
				directionBut = ogui->CreateSimpleImageButton(win, 0, 0, 16, 16, NULL, NULL, NULL);
				directionBut->SetReactMask(0);
				directionBut->SetDisabled(true);
			}
			VC3 pos = VC3(radarX, 0, radarY);
			float dirangle = game->checkpoints->getAngleToCheckpoint(0, pos);
			float chkpointDist = game->checkpoints->getDistanceToCheckpoint(0, pos);
			if (chkpointDist > RADAR_RANGE)
				chkpointDist = RADAR_RANGE;

      float a = (dirangle - angle);
			// only 8 main angles (xdegrees) accepted...
			a /= (RADAR_POINTER_ANGLE_STEP * 3.1415f/180);
			a = (float)floor(a + 0.5f);
			a *= (RADAR_POINTER_ANGLE_STEP * 3.1415f/180);
      int x = (int)((RADAR_SIZE / 2 - 2) * sin(a));
      int y = (int)((RADAR_SIZE / 2 - 2) * cos(a));

			// NEW: scale by distance...
			x = (int)((x * chkpointDist) / RADAR_RANGE);
			y = (int)((y * chkpointDist) / RADAR_RANGE);

      //directionBut->Move(RADAR_SIZE/2 + x - 8, 768-RADAR_SIZE/2 + y - 8);
      directionBut->Move(1024-RADAR_SIZE/2 + x - 8, RADAR_SIZE/2 + y - 8);
			int inum = (int)(RAD_TO_UNIT_ANGLE(a - (3.1415f / COMBAT_RADAR_DIRECTION_IMAGES)));
			inum = 180 - inum;
			while (inum < 0) inum += 360;
			while (inum >= 360) inum -= 360;
			inum /= (360 / COMBAT_RADAR_DIRECTION_IMAGES);
			assert(inum >= 0 && inum < COMBAT_RADAR_DIRECTION_IMAGES);
			if (inum < 0) inum = 0;
			if (inum >= COMBAT_RADAR_DIRECTION_IMAGES) inum = COMBAT_RADAR_DIRECTION_IMAGES - 1;
			directionBut->SetDisabledImage(directionImages[inum]);
		} else {
			if (directionBut != NULL)
			{
				delete directionBut;
				directionBut = NULL;
			}
		}
		*/
	}

}
