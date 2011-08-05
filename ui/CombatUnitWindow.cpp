
#include "precompiled.h"

#include "CombatUnitWindow.h"

#include "../system/Logger.h"
#include "../system/Timer.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/Unit.h"
#include "../game/Part.h"
#include "../game/Weapon.h"
#include "../game/Character.h"
#include "../game/UnitSelections.h"
#include "../game/DHLocaleManager.h"
#include "../convert/str2int.h"
#include "GameController.h"
#include "uidefaults.h"


//#define COMBATUNITWINDOW_SIZE_X 147
//#define COMBATUNITWINDOW_SIZE_Y 81
#define COMBATUNITWINDOW_SIZE_X 188
#define COMBATUNITWINDOW_SIZE_Y 86

// button id's...
#define COMBATUW_UNIT 1 
#define COMBATUW_UNITMODE 2
#define COMBATUW_FACE 3

// weapon buttons (for weapon amount of 2, ids 10 - 11)
#define COMBATUW_WEAP_START 10
#define COMBATUW_WEAP_END (COMBATUW_WEAP_START + COMBATUNITWINDOW_MAX_WEAPONS)

// part hp locations...
// NOTICE: unit's left/right arm and leg sides are mirrored! 
#define COMBATUW_PART_TORSO 0
#define COMBATUW_PART_TORSO_X (44+13)
#define COMBATUW_PART_TORSO_Y (2+17)
#define COMBATUW_PART_TORSO_SIZE_X 25
#define COMBATUW_PART_TORSO_SIZE_Y 23

#define COMBATUW_PART_HEAD 1
#define COMBATUW_PART_HEAD_X (44+19)
#define COMBATUW_PART_HEAD_Y (2+17)
#define COMBATUW_PART_HEAD_SIZE_X 13
#define COMBATUW_PART_HEAD_SIZE_Y 11

#define COMBATUW_PART_ARM_R 2
#define COMBATUW_PART_ARM_R_X (44+10)
#define COMBATUW_PART_ARM_R_Y (2+23)
#define COMBATUW_PART_ARM_R_SIZE_X 11
#define COMBATUW_PART_ARM_R_SIZE_Y 23

#define COMBATUW_PART_ARM_L 3
#define COMBATUW_PART_ARM_L_X (44+29)
#define COMBATUW_PART_ARM_L_Y (2+23)
#define COMBATUW_PART_ARM_L_SIZE_X 11
#define COMBATUW_PART_ARM_L_SIZE_Y 23

#define COMBATUW_PART_LEG_R 4
#define COMBATUW_PART_LEG_R_X (44+16)
#define COMBATUW_PART_LEG_R_Y (2+34)
#define COMBATUW_PART_LEG_R_SIZE_X 11
#define COMBATUW_PART_LEG_R_SIZE_Y 24

#define COMBATUW_PART_LEG_L 5
#define COMBATUW_PART_LEG_L_X (44+24)
#define COMBATUW_PART_LEG_L_Y (2+34)
#define COMBATUW_PART_LEG_L_SIZE_X 11
#define COMBATUW_PART_LEG_L_SIZE_Y 24

#define COMBATUW_WEAP_X (8)
//#define COMBATUW_WEAP_X (44+48)
#define COMBATUW_WEAP_Y (6+32)

#define COMBATUW_HP_BAR_X (8)
//#define COMBATUW_HP_BAR_X (44+49)
#define COMBATUW_HP_BAR_Y 18

#define COMBATUW_ENERGY_BAR_X (8)
//#define COMBATUW_ENERGY_BAR_X (44+49)
#define COMBATUW_ENERGY_BAR_Y 32


#define COMBATUW_FACE_X 8
#define COMBATUW_FACE_Y 19


#define COMBATUW_SELECT_DOUBLECLICK_TIME 400


using namespace game;


namespace ui
{

  CombatUnitWindow::CombatUnitWindow(Ogui *ogui, game::Game *game, game::Unit *unit, int unitNumber)
  {
    this->ogui = ogui;
    this->game = game;
    this->unit = unit;
    this->unitNumber = unitNumber;

    // first load images and initialize variables...

    unitSelectTime = 0;
    blinkUnit = 0;

    int i;
    int j;
    for (j = 0; j < 2; j++)
    {
      unitWeapButs[j] = NULL;
      lastUnitWeapAmmo[j] = -1;
    }
    for (j = 0; j < COMBATUNITWINDOW_MAX_PARTS; j++)
    {
      unitPartHPButs[j] = NULL;
      lastUnitPartHP[j] = -1;
    }
    lastUnitHP = -1;
    lastUnitEnergy = -1;

    // load backgrounds for each group and different selection states
    for (j = 0; j < COMBATUNITWINDOW_UNIT_GROUPS; j++)
    {
      char buf[256];
      strcpy(buf, "Data/GUI/Ingame/status_background_team");
      strcat(buf, int2str(j));
      strcat(buf, ".tga");
      unitBackImages[j] = ogui->LoadOguiImage(buf);
      strcpy(buf, "Data/GUI/Ingame/status_background_team");
      strcat(buf, int2str(j));
      strcat(buf, "_selected.tga");
      unitBackSelectedImages[j] = ogui->LoadOguiImage(buf);
      strcpy(buf, "Data/GUI/Ingame/status_background_team");
      strcat(buf, int2str(j));
      strcat(buf, "_hit.tga");
      unitBackHitImages[j] = ogui->LoadOguiImage(buf);
      strcpy(buf, "Data/GUI/Ingame/status_background_team");
      strcat(buf, int2str(j));
      strcat(buf, "_hit_selected.tga");
      unitBackHitSelectedImages[j] = ogui->LoadOguiImage(buf);
      strcpy(buf, "Data/GUI/Ingame/status_background_team");
      strcat(buf, int2str(j));
      strcat(buf, "_dead.tga");
      unitBackDeadImages[j] = ogui->LoadOguiImage(buf);
    }

    weapImage = NULL;
    weapDownImage = NULL;
    weapSelectedImage = NULL;
    weapSelectedDownImage = NULL;
    weapNoAmmoImage = NULL;
    //weapImage = ogui->LoadOguiImage("Data/Buttons/Ingame/weapon.tga");
    //weapDownImage = ogui->LoadOguiImage("Data/Buttons/Ingame/weapon_down.tga");
    //weapSelectedImage = ogui->LoadOguiImage("Data/Buttons/Ingame/weapon_selected.tga");
    //weapSelectedDownImage = ogui->LoadOguiImage("Data/Buttons/Ingame/weapon_selected_down.tga");
    //weapNoAmmoImage = ogui->LoadOguiImage("Data/Buttons/Ingame/weapon_noammo.tga");

    //modeDefensiveImage = ogui->LoadOguiImage("Data/Buttons/Ingame/mode_defensive.tga");
    //modeDefensiveDownImage = ogui->LoadOguiImage("Data/Buttons/Ingame/mode_defensive_down.tga");
    //modeAggressiveImage = ogui->LoadOguiImage("Data/Buttons/Ingame/mode_aggressive.tga");
    //modeAggressiveDownImage = ogui->LoadOguiImage("Data/Buttons/Ingame/mode_aggressive_down.tga");
    //modeHoldFireImage = ogui->LoadOguiImage("Data/Buttons/Ingame/mode_holdfire.tga");
    //modeHoldFireDownImage = ogui->LoadOguiImage("Data/Buttons/Ingame/mode_holdfire_down.tga");

    /*
    heatOverlayImage = ogui->LoadOguiImage("Data/Buttons/Ingame/heat_overlay.tga");
    energyOverlayImage = ogui->LoadOguiImage("Data/Buttons/Ingame/energy_overlay.tga");
    energyImage = ogui->LoadOguiImage("Data/Buttons/Ingame/energy.tga");
    maxEnergyImage = ogui->LoadOguiImage("Data/Buttons/Ingame/max_energy.tga");
    */
    for (i = 0; i < COMBATUNITWINDOW_HEAT_IMAGES; i++)
    {
      char buf[60];
      if (i == 0)
      {
        heatImages[i] = NULL;
      } else {
        strcpy(buf, "Data/GUI/Ingame/Heat/heat");
        strcat(buf, int2str(i));
        strcat(buf, ".tga");
        heatImages[i] = ogui->LoadOguiImage(buf);
      }
    }
    /*
    for (i = 0; i < COMBATUNITWINDOW_UNIT_HP_IMAGES; i++)
    {
      char buf[60];
      strcpy(buf, "Data/Buttons/Ingame/unitHP/hp_");
      strcat(buf, int2str(i));
      strcat(buf, ".tga");
      unitHPImages[i] = ogui->LoadOguiImage(buf);
    }
    */
    unitHPBlockImage = ogui->LoadOguiImage("Data/GUI/Ingame/UnitHP/hp_block.tga");
    unitEmptyHPBlockImage = ogui->LoadOguiImage("Data/GUI/Ingame/UnitHP/hp_empty_block.tga");

    unitEnergyBlockImage = ogui->LoadOguiImage("Data/GUI/Ingame/Energy/energy_block.tga");
    unitEmptyEnergyBlockImage = ogui->LoadOguiImage("Data/GUI/Ingame/Energy/energy_empty_block.tga");

    unitClipBlockImage = ogui->LoadOguiImage("Data/GUI/Ingame/Energy/clip_block.tga");
    unitEmptyClipBlockImage = ogui->LoadOguiImage("Data/GUI/Ingame/Energy/clip_empty_block.tga");

    for (j = 0; j < COMBATUNITWINDOW_PART_HP_IMGTYPES; j++)
    {
      for (i = 0; i < COMBATUNITWINDOW_PART_HP_IMAGES; i++)
      {
        char buf[60];
        strcpy(buf, "Data/GUI/Ingame/PartHP");
        strcat(buf, int2str(j));
        strcat(buf, "/hp_");
        strcat(buf, int2str(i));
        strcat(buf, ".tga");
        partHPImages[i + (j * COMBATUNITWINDOW_PART_HP_IMAGES)] 
          = ogui->LoadOguiImage(buf);
      }
    }

    // then create window...

    win = ogui->CreateSimpleWindow(0, 0, 
      COMBATUNITWINDOW_SIZE_X, COMBATUNITWINDOW_SIZE_Y, NULL);
		win->SetUnmovable();
    win->Hide();

    // then create buttons...

    OguiButton *b;

    // first the unit "window" background
    b = ogui->CreateSimpleImageButton(win, 0, 0, 
      COMBATUNITWINDOW_SIZE_X, COMBATUNITWINDOW_SIZE_Y, NULL, NULL, NULL,
      COMBATUW_UNIT);
    int group = unit->getGroupNumber();
    if (group >= COMBATUNITWINDOW_UNIT_GROUPS) group = 0;
    b->SetDisabledImage(unitBackImages[group]);
    b->SetReactMask(0);
    b->SetListener(this);
		// HACK! disable unit buttons
		b->SetDisabled(true);
    unitBackBut = b;

		// character's face
		/*
    b = ogui->CreateSimpleImageButton(win, COMBATUW_FACE_X, COMBATUW_FACE_Y, 
      38, 44, NULL, NULL, NULL, COMBATUW_FACE);
    b->SetReactMask(0);
    b->SetListener(this);
    b->SetDisabled(true);
		if (unit->getCharacter() != NULL)
		{
			b->SetDisabledImage(unit->getCharacter()->getIconImage()->getImage());
		}
    characterFaceBut = b;
		*/

    // the unit name
		/*
    char buf[40];
    strcpy(buf, int2str(unitNumber + 1));
    strcat(buf, ". ");
    if (unit->getCharacter() != NULL)
    {
      char *name = unit->getCharacter()->getName();
      if (strlen(name) < 32)
        strcat(buf, unit->getCharacter()->getName());
    }		
    b = ogui->CreateSimpleTextButton(win, 20, 2,
      COMBATUNITWINDOW_SIZE_X - 20, 16, NULL, NULL, NULL, buf, 0);
    b->SetFont(ui::defaultSmallIngameFont);
    b->SetReactMask(0);
    b->SetDisabled(true);
    b->SetListener(this);
    b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
    unitNameBut = b;
		*/

    // the mode button
    b = ogui->CreateSimpleTextButton(win, 4, COMBATUNITWINDOW_SIZE_Y - 20, 
      COMBATUNITWINDOW_SIZE_X-8, 16, NULL, NULL, NULL, NULL,
      COMBATUW_UNITMODE);
    //b->SetImage(modeDefensiveImage);
    //b->SetDownImage(modeDefensiveDownImage);
    b->SetReactMask(0);
    //b->SetReactMask(OGUI_REACT_MASK_BUT_1);
    b->SetDisabled(true);
    b->SetListener(this);
    b->SetFont(ui::defaultSmallIngameFont);
    b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
    unitModeBut = b;

    /*
    // the heat meter
    b = ogui->CreateSimpleImageButton(win, 1024-UNITSTAT_SIZE_X + 16, 
      posY * UNITSTAT_SIZE_Y + 31 + 59, 
      9, 0, NULL, NULL, NULL, 0);
    b->SetDisabledImage(heatImages[0]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitHeatButs[posY] = b;

    // the maxenergy meter
    b = ogui->CreateSimpleImageButton(win, 1024-UNITSTAT_SIZE_X + 77, 
      posY * UNITSTAT_SIZE_Y + 31, 
      9, 59, NULL, NULL, NULL, 0);
    b->SetDisabledImage(maxEnergyImage);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitMaxEnergyButs[posY] = b;

    // the energy meter
    b = ogui->CreateSimpleImageButton(win, 1024-UNITSTAT_SIZE_X + 77, 
      posY * UNITSTAT_SIZE_Y + 31, 
      9, 59, NULL, NULL, NULL, 0);
    b->SetDisabledImage(energyImage);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitEnergyButs[posY] = b;

    // heat overlay (scale dots)
    b = ogui->CreateSimpleImageButton(win, 1024-UNITSTAT_SIZE_X + 16, 
      posY * UNITSTAT_SIZE_Y + 31, 
      9, 59, NULL, NULL, NULL, 0);
    b->SetDisabledImage(heatOverlayImage);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitHeatOverlayButs[posY] = b;

    // energy overlay (scale dots)
    b = ogui->CreateSimpleImageButton(win, 1024-UNITSTAT_SIZE_X + 77, 
      posY * UNITSTAT_SIZE_Y + 31, 
      9, 59, NULL, NULL, NULL, 0);
    b->SetDisabledImage(energyOverlayImage);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitEnergyOverlayButs[posY] = b;
    */

    // unit HP...
    for (i = 0; i < COMBATUNITWINDOW_MAX_HPBLOCKS; i++)
    {
      b = ogui->CreateSimpleImageButton(win, 
        COMBATUW_HP_BAR_X + i * 9, COMBATUW_HP_BAR_Y, 9, 9, 
        NULL, NULL, NULL, 0);
      b->SetDisabled(true);
      b->SetReactMask(0);
      unitHPBlockButs[i] = b;
    }

    // unit energy...
    for (i = 0; i < COMBATUNITWINDOW_MAX_ENERGYBLOCKS; i++)
    {
      b = ogui->CreateSimpleImageButton(win, 
        COMBATUW_ENERGY_BAR_X + i * 4, COMBATUW_ENERGY_BAR_Y, 4, 5, 
        NULL, NULL, NULL, 0);
      b->SetDisabled(true);
      b->SetReactMask(0);
      unitEnergyBlockButs[i] = b;
    }

    // part HPs...

		/*
    // the body hp meter
    b = ogui->CreateSimpleImageButton(win, 
      COMBATUW_PART_TORSO_X, COMBATUW_PART_TORSO_Y, 
      COMBATUW_PART_TORSO_SIZE_X, COMBATUW_PART_TORSO_SIZE_Y, 
      NULL, NULL, NULL, 0);
    b->SetDisabledImage(partHPImages[COMBATUNITWINDOW_PART_HP_IMAGES 
      * COMBATUW_PART_TORSO]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitPartHPButs[COMBATUW_PART_TORSO] = b;

    // the head hp meter
    b = ogui->CreateSimpleImageButton(win, 
      COMBATUW_PART_HEAD_X, COMBATUW_PART_HEAD_Y, 
      COMBATUW_PART_HEAD_SIZE_X, COMBATUW_PART_HEAD_SIZE_Y, 
      NULL, NULL, NULL, 0);
    b->SetDisabledImage(partHPImages[COMBATUNITWINDOW_PART_HP_IMAGES 
      * COMBATUW_PART_HEAD]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitPartHPButs[COMBATUW_PART_HEAD] = b;

    // the right arm hp meter
    b = ogui->CreateSimpleImageButton(win, 
      COMBATUW_PART_ARM_R_X, COMBATUW_PART_ARM_R_Y, 
      COMBATUW_PART_ARM_R_SIZE_X, COMBATUW_PART_ARM_R_SIZE_Y, 
      NULL, NULL, NULL, 0);
    b->SetDisabledImage(partHPImages[COMBATUNITWINDOW_PART_HP_IMAGES 
      * COMBATUW_PART_ARM_R]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitPartHPButs[COMBATUW_PART_ARM_R] = b;

    // the left arm hp meter
    b = ogui->CreateSimpleImageButton(win, 
      COMBATUW_PART_ARM_L_X, COMBATUW_PART_ARM_L_Y, 
      COMBATUW_PART_ARM_L_SIZE_X, COMBATUW_PART_ARM_L_SIZE_Y, 
      NULL, NULL, NULL, 0);
    b->SetDisabledImage(partHPImages[COMBATUNITWINDOW_PART_HP_IMAGES 
      * COMBATUW_PART_ARM_L]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitPartHPButs[COMBATUW_PART_ARM_L] = b;

    // the right leg hp meter
    b = ogui->CreateSimpleImageButton(win, 
      COMBATUW_PART_LEG_R_X, COMBATUW_PART_LEG_R_Y, 
      COMBATUW_PART_LEG_R_SIZE_X, COMBATUW_PART_LEG_R_SIZE_Y, 
      NULL, NULL, NULL, 0);
    b->SetDisabledImage(partHPImages[COMBATUNITWINDOW_PART_HP_IMAGES 
      * COMBATUW_PART_LEG_R]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitPartHPButs[COMBATUW_PART_LEG_R] = b;

    // the left leg hp meter
    b = ogui->CreateSimpleImageButton(win, 
      COMBATUW_PART_LEG_L_X, COMBATUW_PART_LEG_L_Y, 
      COMBATUW_PART_LEG_L_SIZE_X, COMBATUW_PART_LEG_L_SIZE_Y, 
      NULL, NULL, NULL, 0);
    b->SetDisabledImage(partHPImages[COMBATUNITWINDOW_PART_HP_IMAGES 
      * COMBATUW_PART_LEG_L]);
    b->SetDisabled(true);
    b->SetReactMask(0);
    unitPartHPButs[COMBATUW_PART_LEG_L] = b;

    // heat image
    b = ogui->CreateSimpleImageButton(win, 7, 19, 
      39, 39, NULL, NULL, NULL, 0);
    b->SetDisabledImage(NULL);
    b->SetReactMask(0);
    b->SetListener(this);
    b->SetDisabled(true);
    unitHeatBut = b;
		*/

    // the weapon buttons
		/*
    if (unit->getWeaponType(COMBATUNITWINDOW_MAX_WEAPONS) != NULL)
    {
      Logger::getInstance()->warning("CombatWindow - Unit has more weapons than status window can handle.");
    }
		*/

    //char winfobuf[20];
    for (j = 0; j < COMBATUNITWINDOW_MAX_WEAPONS; j++)
    {
      /*
      Weapon *weapType = u->getWeaponType(j);
      char *weapInfoString = NULL;
      winfobuf[0] = '\0';
      if (weapType != NULL)
      {
        strcpy(winfobuf, " ");
        weapInfoString = weapType->getShortName();
        if (weapInfoString != NULL && strlen(weapInfoString) < 8)
        {
          strcat(winfobuf, weapInfoString);
        }
        if (weapType->getAmmoType() != NULL)
        {
          strcat(winfobuf, "\n ");
          strcat(winfobuf, int2str(u->getWeaponAmmoAmount(j)));
        }
      }
      */
      b = ogui->CreateSimpleTextButton(win, COMBATUW_WEAP_X,
        COMBATUW_WEAP_Y + (j * 10), COMBATUNITWINDOW_SIZE_X-COMBATUW_WEAP_X, 10, NULL, NULL, NULL, 
        "", COMBATUW_WEAP_START + j);
      /*
      if (u->isWeaponActive(j))
      {
        if (u->getWeaponAmmoAmount(j) <= 0
          && u->getWeaponAmmoType(j) != NULL)
        {
          b->SetImage(weapNoAmmoImage);
          b->SetDownImage(weapNoAmmoImage);
        } else {
          b->SetImage(weapSelectedImage);
          b->SetDownImage(weapSelectedDownImage);
        }
      } else {
        b->SetImage(weapImage);
        b->SetDownImage(weapDownImage);
      }
      */
      b->SetFont(ui::defaultSmallIngameFont);
      b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
      //b->SetLineBreaks(true);
      b->SetReactMask(OGUI_REACT_MASK_BUT_1);
      b->SetListener(this);
			// no longer activated/deactivated by pressing the weapon button
			// so disable them all
			b->SetDisabled(true);
      /*
      if (weapType == NULL)
        b->SetDisabled(true);
      */

      unitWeapButs[j] = b;
    }
    updateWeaponInfo();
    updateHPInfo();
  }



  CombatUnitWindow::~CombatUnitWindow()
  {
    if (unitBackBut != NULL)
    {
      delete unitBackBut;
      unitBackBut = NULL;
    }
		/*
    if (unitNameBut != NULL)
    {
      delete unitNameBut;
      unitNameBut = NULL;
    }
		*/
    if (unitModeBut != NULL)
    {
      delete unitModeBut;
      unitModeBut = NULL;
    }
		/*
    if (unitHeatBut != NULL)
    {
      delete unitHeatBut;
      unitHeatBut = NULL;
    }
		*/
    int i;
    for (i = 0; i < COMBATUNITWINDOW_MAX_WEAPONS; i++)
    {
      if (unitWeapButs[i] != NULL)
      {
        delete unitWeapButs[i];
        unitWeapButs[i] = NULL;
      }
    }
    /*
    if (unitEnergyBut != NULL)
    {
      delete unitEnergyBut;
      unitEnergyBut = NULL;
    }
    if (unitEnergyOverlayBut != NULL)
    {
      delete unitEnergyOverlayBut;
      unitEnergyOverlayBut = NULL;
    }
    if (unitHeatOverlayBut != NULL)
    {
      delete unitHeatOverlayBut;
      unitHeatOverlayBut = NULL;
    }
    if (unitMaxEnergyBut != NULL)
    {
      delete unitMaxEnergyBut;
      unitMaxEnergyBut = NULL;
    }
    if (unitHeatBut != NULL)
    {
      delete unitHeatBut;
      unitHeatBut = NULL;
    }
    */
    for (i = 0; i < COMBATUNITWINDOW_MAX_PARTS; i++)
    {
			/*
      if (unitPartHPButs[i] != NULL)
      {
        delete unitPartHPButs[i];
        unitPartHPButs[i] = NULL;
      }
			*/
    }
    for (i = 0; i < COMBATUNITWINDOW_MAX_HPBLOCKS; i++)
    {
      if (unitHPBlockButs[i] != NULL)
      {
        delete unitHPBlockButs[i];
        unitHPBlockButs[i] = NULL;
      }
    }
    for (i = 0; i < COMBATUNITWINDOW_MAX_ENERGYBLOCKS; i++)
    {
      if (unitEnergyBlockButs[i] != NULL)
      {
        delete unitEnergyBlockButs[i];
        unitEnergyBlockButs[i] = NULL;
      }
    }

    // delete images
    if (unitHPBlockImage != NULL)
    {
      delete unitHPBlockImage;
      unitHPBlockImage = NULL;
    }
    if (unitEmptyHPBlockImage != NULL)
    {
      delete unitEmptyHPBlockImage;
      unitEmptyHPBlockImage = NULL;
    }
    if (unitEnergyBlockImage != NULL)
    {
      delete unitEnergyBlockImage;
      unitEnergyBlockImage = NULL;
    }
    if (unitEmptyEnergyBlockImage != NULL)
    {
      delete unitEmptyEnergyBlockImage;
      unitEmptyEnergyBlockImage = NULL;
    }
    if (unitClipBlockImage != NULL)
    {
      delete unitClipBlockImage;
      unitClipBlockImage = NULL;
    }
    if (unitEmptyClipBlockImage != NULL)
    {
      delete unitEmptyClipBlockImage;
      unitEmptyClipBlockImage = NULL;
    }
    for (i = 0; i < COMBATUNITWINDOW_HEAT_IMAGES; i++)
    {
      if (heatImages[i] != NULL)
      {
        delete heatImages[i];
        heatImages[i] = NULL;
      }
    }
    for (i = 0; i < COMBATUNITWINDOW_UNIT_GROUPS; i++)
    {
      if (unitBackSelectedImages[i] != NULL)
      {
        delete unitBackSelectedImages[i];
        unitBackSelectedImages[i] = NULL;
      }
      if (unitBackImages[i] != NULL)
      {
        delete unitBackImages[i];
        unitBackImages[i] = NULL;
      }
      if (unitBackHitSelectedImages[i] != NULL)
      {
        delete unitBackHitSelectedImages[i];
        unitBackHitSelectedImages[i] = NULL;
      }
      if (unitBackHitImages[i] != NULL)
      {
        delete unitBackHitImages[i];
        unitBackHitImages[i] = NULL;
      }
      if (unitBackDeadImages[i] != NULL)
      {
        delete unitBackDeadImages[i];
        unitBackDeadImages[i] = NULL;
      }
    }
    if (weapSelectedImage != NULL)
    {
      delete weapSelectedImage;
      weapSelectedImage = NULL;
    }
    if (weapImage != NULL)
    {
      delete weapImage;
      weapImage = NULL;
    }
    if (weapSelectedDownImage != NULL)
    {
      delete weapSelectedDownImage;
      weapSelectedDownImage = NULL;
    }
    if (weapDownImage != NULL)
    {
      delete weapDownImage;
      weapDownImage = NULL;
    }
    if (weapNoAmmoImage != NULL)
    {
      delete weapNoAmmoImage;
      weapNoAmmoImage = NULL;
    }
    /*
    if (modeDefensiveImage != NULL)
    {
      delete modeDefensiveImage;
      modeDefensiveImage = NULL;
    }
    if (modeDefensiveDownImage != NULL)
    {
      delete modeDefensiveDownImage;
      modeDefensiveDownImage = NULL;
    }
    if (modeAggressiveImage != NULL)
    {
      delete modeAggressiveImage;
      modeAggressiveImage = NULL;
    }
    if (modeAggressiveDownImage != NULL)
    {
      delete modeAggressiveDownImage;
      modeAggressiveDownImage = NULL;
    }
    if (modeHoldFireImage != NULL)
    {
      delete modeHoldFireImage;
      modeHoldFireImage = NULL;
    }
    if (modeHoldFireImage != NULL)
    {
      delete modeHoldFireDownImage;
      modeHoldFireDownImage = NULL;
    }
    */
    /*
    if (energyImage != NULL)
    {
      delete energyImage;
    }
    if (maxEnergyImage != NULL)
    {
      delete maxEnergyImage;
    }
    for (i = 0; i < COMBATW_HEAT_IMAGES; i++)
    {
      if (heatImages[i] != NULL)
      {
        delete heatImages[i];
        heatImages[i] = NULL;
      }
    }
    delete [] heatImages;
    */
    for (int j = 0; j < COMBATUNITWINDOW_PART_HP_IMGTYPES; j++)
    {
      for (i = 0; i < COMBATUNITWINDOW_PART_HP_IMAGES; i++)
      {
				/*
        if (partHPImages[i + (j * COMBATUNITWINDOW_PART_HP_IMAGES)] != NULL)
          delete partHPImages[i + (j * COMBATUNITWINDOW_PART_HP_IMAGES)];
				*/
      }
    }
		assert(win != NULL);
		if (win != NULL)
		{
			delete win;
			win = NULL;
		}
  }



  void CombatUnitWindow::hide()
  {
    win->Hide();
  }


  void CombatUnitWindow::show()
  {
    win->Show();
  }


  bool CombatUnitWindow::isVisible()
  {
    return win->IsVisible();
  }



  void CombatUnitWindow::updateWeaponInfo()
  {
    // update weapon info (ammo amount)
    for (int j = 0; j < COMBATUNITWINDOW_MAX_WEAPONS; j++)
    {
      Weapon *weapType = NULL;
			int weapNum = -1;
			if (unit->isDirectControl())
			{
				if (j == 0)
				{
					weapNum = unit->getSelectedWeapon();
				}
				if (j == 1)
				{
					weapNum = unit->getSelectedSecondaryWeapon();
				}
			} else {
				weapNum = j;
			}
			if (weapNum != -1)
				weapType = unit->getWeaponType(weapNum);

      if (weapType != NULL
        && unit->getWeaponAmmoAmount(weapNum) != lastUnitWeapAmmo[j])
      {
        char winfobuf[30];

        char *weapInfoString = NULL;
        winfobuf[0] = '\0';

        //strcpy(winfobuf, " ");
        weapInfoString = weapType->getShortName();
        if (weapInfoString != NULL && strlen(weapInfoString) < 8)
        {
          strcat(winfobuf, weapInfoString);
        }
        if (weapType->getAmmoType() != NULL)
        {
					int ammoAmount = unit->getWeaponAmmoAmount(weapNum);
					/*
					if (weapType->isSharedClipAttachment())
					{
						for (int we = 0; we < UNIT_MAX_WEAPONS; we++)
						{
							if (unit->getWeaponType(we)->getAttachedWeaponType() == weapType)
							{
								ammoAmount = unit->getWeaponAmmoAmount(we);
								break;
							}
						}
					}
					*/

					// TODO: maybe should say "UNLIMITED" or something
          strcat(winfobuf, " ");
					if (ammoAmount < 9999) 
					{
	          strcat(winfobuf, int2str(ammoAmount));
					}
          //strcat(winfobuf, "/");
          //strcat(winfobuf, int2str(unit->getWeaponMaxAmmoAmount(j)));
        }

				unitWeapButs[j]->SetFont(ui::defaultIngameNumbersBoldFont);
        unitWeapButs[j]->SetText(winfobuf);
        lastUnitWeapAmmo[j] = unit->getWeaponAmmoAmount(weapNum);

        if (unit->getWeaponAmmoAmount(j) <= 0)
        {
          unitWeapButs[j]->SetImage(weapNoAmmoImage);
          unitWeapButs[j]->SetDownImage(weapNoAmmoImage);
        }
      }
    }
  }


  
  void CombatUnitWindow::updateSelectionInfo()
  {
    int group = unit->getGroupNumber();
    if (group >= COMBATUNITWINDOW_UNIT_GROUPS) group = 0;

    if (unit->isDestroyed())
    {
      unitBackBut->SetImage(unitBackDeadImages[group]);
    } else {
	    // hit blink
      if (blinkUnit > 0)
      {
        if (unit->isSelected())
          unitBackBut->SetImage(unitBackHitSelectedImages[group]);
        else
          unitBackBut->SetImage(unitBackHitImages[group]);
      } else {
        if (unit->isSelected())
          unitBackBut->SetImage(unitBackSelectedImages[group]);
        else
          unitBackBut->SetImage(unitBackImages[group]);
      }
    }
  }


  void CombatUnitWindow::updateEnergyInfo()
  {
		int e, me; // energy/maxenergy (or ammo_in_clip/clip_size)

		// if using clipped weapon, show that, and not the energy
		Weapon *clipWeapon = NULL;
		int clipWNum = -1; 
		
		for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
		{
			Weapon *we = unit->getWeaponType(w);
			if (unit->isWeaponActive(w) && we != NULL
				&& we->getClipSize() > 0)
			{				
				clipWeapon = unit->getWeaponType(w);
				clipWNum = w;
				break;
			}
		}
		if (unit->isDirectControl())
		{
			if (unit->getSelectedWeapon() != -1
				&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
			{
				clipWNum = unit->getSelectedWeapon();
				clipWeapon = unit->getWeaponType(unit->getSelectedWeapon());
			}
		}

		if (clipWeapon != NULL)
		{
			e = unit->getWeaponAmmoInClip(clipWNum);
			me = clipWeapon->getClipSize();
		} else {
			e = unit->getEnergy();
			me = unit->getMaxEnergy();
		}

    if (e != lastUnitEnergy
			|| blinkUnit > 0)
    {
      lastUnitEnergy = e;
			int maxEnergy = me;
      for (int i = 0; i < COMBATUNITWINDOW_MAX_ENERGYBLOCKS; i++)
      {
        if (lastUnitEnergy > i * maxEnergy / COMBATUNITWINDOW_MAX_ENERGYBLOCKS
					&& !unit->isDestroyed())
        {
					if (clipWeapon != NULL)
					{
						unitEnergyBlockButs[i]->SetDisabledImage(unitClipBlockImage);
					} else {
						unitEnergyBlockButs[i]->SetDisabledImage(unitEnergyBlockImage);
					}
        } else {
          if (maxEnergy > i * maxEnergy / COMBATUNITWINDOW_MAX_ENERGYBLOCKS
					&& !unit->isDestroyed())
          {
						if (clipWeapon != NULL)
						{
	            unitEnergyBlockButs[i]->SetDisabledImage(unitEmptyClipBlockImage);
						} else {
	            unitEnergyBlockButs[i]->SetDisabledImage(unitEmptyEnergyBlockImage);
						}
          } else {
            unitEnergyBlockButs[i]->SetDisabledImage(NULL);
          }
        }
      }
    }
	}


  void CombatUnitWindow::updateHPInfo()
  {
    if (unit->getHP() != lastUnitHP)
    {
			if (unit->isDestroyed())
				updateEnergyInfo();

      lastUnitHP = unit->getHP();
			int maxHP = unit->getMaxHP();
      for (int i = 0; i < COMBATUNITWINDOW_MAX_HPBLOCKS; i++)
      {
        if (lastUnitHP > i * (maxHP / COMBATUNITWINDOW_MAX_HPBLOCKS))
        {
          unitHPBlockButs[i]->SetDisabledImage(unitHPBlockImage);
        } else {
          if (unit->getMaxHP() > i * (maxHP / COMBATUNITWINDOW_MAX_HPBLOCKS))
          {
            unitHPBlockButs[i]->SetDisabledImage(unitEmptyHPBlockImage);
          } else {
            unitHPBlockButs[i]->SetDisabledImage(NULL);
          }
        }
      }
    }

    Unit *u = unit;
    if (u != NULL)
    {
			// "clamp down" the parts that are in the best shape if other parts
			// are in a crappy shape. (equalize the unit hp part colors)
			int highestAllowedLevel = COMBATUNITWINDOW_PART_HP_IMAGES;

			// destroyed ones are always on red at highest
			if (u->isDestroyed())
				highestAllowedLevel = 1;

      // THE ÜBERCODE!
      // GET RID OF THIS WHOLE CRAP IF YOU WANT TO DO ANY CHANGES 
      // TO HP BARS!!!
      // update hp meters
      for (int j = 0; j < COMBATUNITWINDOW_MAX_PARTS; j++)
      {
        Part *p = NULL;
        if (j == 0) 
        {
          p = u->getRootPart();
				}

        //} else {
          /*
          int parttypeid = 0;
          if (j == 1) parttypeid = PARTTYPE_ID_STRING_TO_INT("Head");
          else if (j == 2) parttypeid = PARTTYPE_ID_STRING_TO_INT("Arm");
          else if (j == 3) parttypeid = PARTTYPE_ID_STRING_TO_INT("Arm");
          else if (j == 4) parttypeid = PARTTYPE_ID_STRING_TO_INT("Leg");
          else if (j == 5) parttypeid = PARTTYPE_ID_STRING_TO_INT("Leg");
          */
          Part *rootp = u->getRootPart();
          int slotAmount = rootp->getType()->getSlotAmount();
          for (int k = 0; k < slotAmount; k++)
          {
            // CORRECT (SLOWER) CLAUSE:
            // rootp->getSubPart(k)->getType()->isInherited(getPartTypeById(parttypeid))
            // NOW, WE JUST SKIP THE CHECK AND RELY ON POSITION
            //  && rootp->getSubPart(k)->getType()->getPartTypeId() 
            //  == parttypeid)
            if (rootp->getSubPart(k) != NULL)
            {
							// going thru all parts and calculating the highest
							// color level allowed (most damaged part defines
							// the max color shown for other parts)
							Part *subp = rootp->getSubPart(k);
							if (subp != NULL)
							{
				        int maxhp = subp->getType()->getMaxDamage();
								if (maxhp == 0) maxhp = 1;
								int hp = (COMBATUNITWINDOW_PART_HP_IMAGES * 
									(maxhp - subp->getDamage() 
									+ (maxhp / COMBATUNITWINDOW_PART_HP_IMAGES - 1))) 
									/ maxhp;
								if (hp < 0) hp = 0;
								if (hp + 2 < highestAllowedLevel)
									highestAllowedLevel = hp + 2;
							}

							// is this the part we want to show?
              if ((j == 1 && rootp->getType()->getSlotPosition(k) == SLOT_POSITION_HEAD) 
                || (j == 2 && rootp->getType()->getSlotPosition(k) == SLOT_POSITION_RIGHT_ARM)
                || (j == 3 && rootp->getType()->getSlotPosition(k) == SLOT_POSITION_LEFT_ARM)
                || (j == 4 && rootp->getType()->getSlotPosition(k) == SLOT_POSITION_RIGHT_LEG)
                || (j == 5 && rootp->getType()->getSlotPosition(k) == SLOT_POSITION_LEFT_LEG))
              {
                p = rootp->getSubPart(k);
                //break;
              }
            }
          }
        //}

				// no head?
				if (j == 1 && p == NULL)
				{
					// then show torso's HP
          p = u->getRootPart();
				}

        if (p != NULL)
        {
					/*
          int newhp = p->getDamage();
          if (newhp != lastUnitPartHP[j])
          {
            if (lastUnitPartHP[j] != -1)
              blinkUnit = 5;
            lastUnitPartHP[j] = newhp;
          }
					*/
        }

        /*int imgtype = j;
        if (imgtype >= 4) imgtype = 3 - (imgtype - 4);*/
        int maxhp = 0;
        if (p != NULL)
          maxhp = p->getType()->getMaxDamage();
        else
          maxhp = 1;
        if (maxhp == 0) maxhp = 1;
        int hp = 0;
        if (p != NULL)
          hp = (COMBATUNITWINDOW_PART_HP_IMAGES * 
          (maxhp - p->getDamage() 
          + (maxhp / COMBATUNITWINDOW_PART_HP_IMAGES - 1))) 
          / maxhp;
        else
          hp = COMBATUNITWINDOW_PART_HP_IMAGES;
        if (hp < 0) hp = 0;
        // this was totally wrong...
        //if (hp > maxhp) hp = maxhp;
        ///if (hp > 24) hp = 24;
				if (hp > highestAllowedLevel)
					hp = highestAllowedLevel;
        if (hp >= COMBATUNITWINDOW_PART_HP_IMAGES) 
          hp = COMBATUNITWINDOW_PART_HP_IMAGES - 1;
        //int hpimg = hp; //hp * COMBATUW_HP_IMAGES / 24;
        //if (hpimg < 0) hpimg = 0;
        //if (hpimg >= COMBATUW_PART_HP_IMAGES) hpimg = COMBATUW_PART_HP_IMAGES - 1;
				/*
        unitPartHPButs[j]->SetDisabledImage(partHPImages[hpimg + (imgtype * COMBATUNITWINDOW_PART_HP_IMAGES)]);
				*/
      }
    }
  }


  void CombatUnitWindow::updateModeInfo()
  {
		if (unit == game->gameUI->getFirstPerson(0)
			|| unit == game->gameUI->getFirstPerson(1)
			|| unit == game->gameUI->getFirstPerson(2)
			|| unit == game->gameUI->getFirstPerson(3))
		{
			//unitModeBut->SetText("");
			return;
		}

		if (unit->isDestroyed())
		{
			unitModeBut->SetText("DEAD");
			return;
		}

		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		{
			unitModeBut->SetText("UNCONSCIOUS");
			return;
		}

		if (unit->targeting.hasTarget())
		{
			bool fired = false;
			bool reconWeap = false;
			for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
			{
				if (unit->getWeaponType(i) != NULL)
				{
					if (unit->getWeaponType(i)->doesNeedRecon()
						&& unit->isWeaponActive(i))
					{
						reconWeap = true;
					}
					if (unit->getFireWaitDelay(i) > 0
						|| unit->getFireReloadDelay(i) > 0)
					{
						fired = true;
					}
				}
			}
			if (fired)
			{
				// TODO: basic / primary / mines ...
				if (unit->isReconAvailableFlag() || !reconWeap)
				{
					if (unit->getEnergy() < unit->getMaxEnergy() / 10)
					{
						unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_firing_energy_low"));
					} else {
						unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_firing"));
					}
				} else {
					unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_firing_blind"));
				}
			} else {
				if (unit->isReconAvailableFlag() || !reconWeap)
				{
					if (unit->getEnergy() < unit->getMaxEnergy() / 10)
					{
						unitModeBut->SetText(getLocaleGuiString(getLocaleGuiString("gui_unit_combat_firing_energy_low")));
					} else {
						unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_targeting"));
					}
				} else {
					unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_targeting_blind"));
				}
			}
			return;
		}

		if (blinkUnit > 0)
		{
			if (unit->getHP() < 20)
			{
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_losing_conciousness"));
			} else {
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_taking_damage"));
			}
			return;
		}

		if (unit->getHP() < 20)
		{
			unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_bad_condition"));
			return;
		}

		if (unit->isStealthing())
		{
			if (unit->getEnergy() < unit->getMaxEnergy() / 5)
			{
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_stealthing_energy_low"));
				return;
			} else {
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_stealthing"));
				return;
			}
		}
		if (unit->getSpeed() == Unit::UNIT_SPEED_SLOW)
		{
			if (!unit->atFinalDestination())
			{
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_sneaking"));
				return;
			}
		}

		if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
		{
			if (unit->atFinalDestination())
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_prone"));
			else
				unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_crawling"));
			return;
		}

		if (unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
		{
			if (!unit->atFinalDestination())
			{
				if (unit->getReconValue() > 0)
					unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_running_silent"));
				else
					unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_running"));
				return;
			}
		}

		if (unit->getSpeed() == Unit::UNIT_SPEED_FAST)
		{
			if (!unit->atFinalDestination())
			{
				if (unit->getReconValue() > 0)
					unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_moving_silent"));
				else
					unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_moving"));
				return;
			}
		}

		if (unit->getEnergy() < unit->getMaxEnergy() / 10)
		{
			unitModeBut->SetText(getLocaleGuiString("gui_unit_combat_energy_low"));
			return;			
		}

		unitModeBut->SetText("");
	}


  void CombatUnitWindow::updateMiscInfo()
  {
		updateEnergyInfo();

    // update (heat, energy,) hit blinking...
    int group = unit->getGroupNumber();
    if (group >= COMBATUNITWINDOW_UNIT_GROUPS) group = 0;

    if (unit != NULL)
    {
      if (blinkUnit > 0)
      {
        blinkUnit--;
        updateSelectionInfo();
      }

      // update heat meter
			/*
      int maxh = unit->getMaxHeat();
      if (maxh == 0) maxh = 1;
      int h = (COMBATUNITWINDOW_HEAT_IMAGES * unit->getHeat()) / maxh;
      if (unit->isDestroyed()) h = 0;
      int himg = h;
      if (himg < 0) himg = 0;
      if (himg >= COMBATUNITWINDOW_HEAT_IMAGES) 
        himg = COMBATUNITWINDOW_HEAT_IMAGES - 1;
      unitHeatBut->SetDisabledImage(heatImages[himg]);
			*/
    }
  }


  void CombatUnitWindow::CursorEvent(OguiButtonEvent *eve)
  {
    if (!game->gameUI->isCursorActive(game->singlePlayerNumber))
    {
      return;
    }

    if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
    {

      if (eve->triggerButton->GetId() == COMBATUW_UNIT)
      {
        if (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK)
        {
          doUnitSelection();
        }
      }
      if (eve->triggerButton->GetId() >= COMBATUW_UNITMODE)
      {
        bool changeMultiple = false;
        Unit *u = unit;
        int unitNum = unitNumber;
        if (u == NULL)
        {
          // button assigned for null unit? (should never happen)
          assert(0);
        }

        // new mode based on current mode of the unit
        Unit::UNIT_MODE umode = u->getMode();
        if (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK)
        {
          if (umode == Unit::UNIT_MODE_AGGRESSIVE)
          {
            umode = Unit::UNIT_MODE_DEFENSIVE;
          } else {
            if (umode == Unit::UNIT_MODE_DEFENSIVE)
            {
              umode = Unit::UNIT_MODE_HOLD_FIRE;
            } else {
              umode = Unit::UNIT_MODE_AGGRESSIVE;
            }
          }
        } else {
          if (eve->cursorOldButtonMask & OGUI_BUTTON_2_MASK)
          {
            if (umode == Unit::UNIT_MODE_AGGRESSIVE)
            {
              umode = Unit::UNIT_MODE_HOLD_FIRE;
            } else {
              if (umode == Unit::UNIT_MODE_DEFENSIVE)
              {
                umode = Unit::UNIT_MODE_AGGRESSIVE;
              } else {
                umode = Unit::UNIT_MODE_DEFENSIVE;
              }
            }
          }
        }

        // changed mode for unit part of multiple unit selection
        if (u->isSelected() 
          && game->unitSelections[unit->getOwner()]->getUnitsSelected() > 1)
        {
          // change multiple unit modes
          changeMultiple = true;
          unitNum = 0;
        }

        // go thru all selected units or the one unit and apply new mode.
        /*
        bool moreUnits = true;
        while (moreUnits)
        {
          if (changeMultiple)
          {
            u = solveUnitForNumber(unitNum);
          }

          if (u != NULL && (u->isSelected() || !changeMultiple))
          {
            if (!u->isDestroyed())
            {
              // set new mode
              u->setMode(umode);

              // set mode button image
              if (umode == Unit::UNIT_MODE_AGGRESSIVE)
              {
                unitModeButs[unitNum]->SetImage(modeAggressiveImage);
                unitModeButs[unitNum]->SetDownImage(modeAggressiveDownImage);
              } else {
                if (umode == Unit::UNIT_MODE_DEFENSIVE)
                {
                  unitModeButs[unitNum]->SetImage(modeDefensiveImage);
                  unitModeButs[unitNum]->SetDownImage(modeDefensiveDownImage);
                } else {
                  unitModeButs[unitNum]->SetImage(modeHoldFireImage);
                  unitModeButs[unitNum]->SetDownImage(modeHoldFireDownImage);
                }
              }
            }
          }

          if (changeMultiple)
          {
            unitNum++;
            if (unitNum >= COMBATW_UNITS) 
              moreUnits = false;
          } else {
            moreUnits = false;
          }
        }
        */
      }
      if (eve->triggerButton->GetId() >= COMBATUW_WEAP_START
        && eve->triggerButton->GetId() <= COMBATUW_WEAP_END)
      {
        int weapNum = eve->triggerButton->GetId() - COMBATUW_WEAP_START;
        Unit *u = unit;
        if (u != NULL)
        {
          if (!u->isDestroyed())
          {
            if (eve->cursorOldButtonMask & OGUI_BUTTON_1_MASK)
            {
              if (u->isWeaponOperable(weapNum)
                && (u->getWeaponAmmoAmount(weapNum) > 0
                || u->getWeaponAmmoType(weapNum) == NULL))
              {
                if (u->isWeaponActive(weapNum))
                {
                  u->setWeaponActive(weapNum, false);
                  unitWeapButs[weapNum]->SetImage(weapImage);
                  unitWeapButs[weapNum]->SetDownImage(weapDownImage);
                } else {
                  u->setWeaponActive(weapNum, true);
                  unitWeapButs[weapNum]->SetImage(weapSelectedImage);
                  unitWeapButs[weapNum]->SetDownImage(weapSelectedDownImage);
                }
              }
            }
          }
        } else {
          // button assigned for null unit? (should never happen)
          assert(0);
        }
      }

    }

  }


  game::Unit *CombatUnitWindow::getUnit()
  {
    return unit;
  } 


  void CombatUnitWindow::moveTo(int x, int y)
  {
    win->MoveTo(x, y);
  } 


  void CombatUnitWindow::doUnitSelection()
  {
    int player = unit->getOwner();

    if (player == NO_UNIT_OWNER) 
    {
      assert(0);
      return;
    }

//        game->unitSelections[unit->getOwner()]->selectUnit(unit, true);
 
    bool centeredTo = false;
    int now = Timer::getTime();
    if (unitSelectTime != 0
      && now < unitSelectTime + COMBATUW_SELECT_DOUBLECLICK_TIME)
    {
      // doubleclicked, so center to unit
      VC3 upos = unit->getPosition();
      game->gameUI->getGameCamera()->setDestination(upos.x, upos.z);
      centeredTo = true;
    } else {
      unitSelectTime = now;
    }

    if (unit->isSelected())
    {
      // unselect only if this was not doubleclick
      if (!centeredTo)
      {
        // if no shift pressed and multiple selected, 
        // unselect all but this.
        if (game->unitSelections[player]->getUnitsSelected() > 1 
          && !game->gameUI->getController(player)->
          isKeyDown(DH_CTRL_MULTIPLE_UNIT_SELECT))
        {
          // select this unit only.
          if (!unit->isDestroyed())
          {
            game->unitSelections[player]->selectAllUnits(false);
            game->unitSelections[player]->selectUnit(unit, true);
          }
        } else {
          // unselect this unit
          if (!unit->isDestroyed())
          {
            game->unitSelections[player]->selectUnit(unit, false);
          }
        }
      }
    } else {
      if (!unit->isDestroyed()
				&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
      {
        // if no shift pressed, unselect previously selected
        if (game->unitSelections[player]->getUnitsSelected() > 0 
          && !game->gameUI->getController(player)->
          isKeyDown(DH_CTRL_MULTIPLE_UNIT_SELECT))
        {
          game->unitSelections[player]->selectAllUnits(false);
        }
        // select this unit
        game->unitSelections[player]->selectUnit(unit, true);
      }
    }
  }

}
