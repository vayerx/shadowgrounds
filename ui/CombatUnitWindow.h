
#ifndef COMBATUNITWINDOW_H
#define COMBATUNITWINDOW_H

#include "../ogui/IOguiButtonListener.h"
#include "../ogui/Ogui.h"

// some constants
#define COMBATUNITWINDOW_MAX_WEAPONS 2
#define COMBATUNITWINDOW_MAX_PARTS 6

#define COMBATUNITWINDOW_PART_HP_IMAGES 4
#define COMBATUNITWINDOW_PART_HP_IMGTYPES 6

#define COMBATUNITWINDOW_MAX_HPBLOCKS 10

#define COMBATUNITWINDOW_MAX_ENERGYBLOCKS 22

#define COMBATUNITWINDOW_HEAT_IMAGES 10

//#define COMBATUNITWINDOW_UNIT_HP_IMAGES 1

// none, red, blue, yellow groups.
#define COMBATUNITWINDOW_UNIT_GROUPS 4


namespace game
{
  class Game;
  class Unit;
}


namespace ui
{
  /**
   *
   * Combat _UNIT_ window class. 
   * (one unit's stuff inside the combat window)
   *
   * @version 0.5, 23.6.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see CombatWindow
   *
   */

  class CombatUnitWindow : public IOguiButtonListener
  {
  public:
    enum WEAPON_STATE
    {
      WEAPON_STATE_INACTIVE,
      WEAPON_STATE_ACTIVE,
      WEAPON_STATE_NOAMMO
    };

    CombatUnitWindow(Ogui *ogui, game::Game *game, game::Unit *unit, int unitNumber);

    ~CombatUnitWindow();

    void CursorEvent(OguiButtonEvent *eve);

    void hide();
    void show();
    bool isVisible();

    /**
     * Move the unit window to given screen coordinates.
     * @param x  int, x-coordinate of upper left corner
     * @param y  int, y-coordinate of upper left corner
     */
    void moveTo(int x, int y);

    /**
     * Set's the unit selected or unselected.
     * Does not affect gameplay, only visual side...?!
     * @param selected  bool, true if the unit is to be selected,
     *                        false if the unit is to be unselected.
     */
    void setUnitSelection(bool selected);
    
    void setWeaponState(int weaponNumber, WEAPON_STATE state);
    void setWeaponAmmoAmount(int weaponNumber, int ammoAmount);

    void updateWeaponInfo();

    void updateMiscInfo();

    void updateSelectionInfo();

    void updateHPInfo();

    void updateEnergyInfo();

    void updateModeInfo();

    void doUnitSelection();

    /**
     * Returns the unit that this status window is bound to.
     */
    game::Unit *getUnit();

  private:
    Ogui *ogui;
    game::Game *game;
    game::Unit *unit;
    OguiWindow *win;
    int unitNumber;

    // NOTICE!!!
    // There is no point in loading these images seperately for each
    // unit window, as they all use the same images.
    // For now, we'll just trust in storm3d to handle texture sharing 
    // properly.

    // background
    IOguiImage *unitBackImages[COMBATUNITWINDOW_UNIT_GROUPS];
    IOguiImage *unitBackSelectedImages[COMBATUNITWINDOW_UNIT_GROUPS];
    IOguiImage *unitBackDeadImages[COMBATUNITWINDOW_UNIT_GROUPS];
    IOguiImage *unitBackHitImages[COMBATUNITWINDOW_UNIT_GROUPS];
    IOguiImage *unitBackHitSelectedImages[COMBATUNITWINDOW_UNIT_GROUPS];

    // weapon
    IOguiImage *weapImage;
    IOguiImage *weapSelectedImage;
    IOguiImage *weapDownImage;
    IOguiImage *weapSelectedDownImage;
    IOguiImage *weapNoAmmoImage;

    // mode
    IOguiImage *modeDefensiveImage;
    IOguiImage *modeDefensiveDownImage;
    IOguiImage *modeAggressiveImage;
    IOguiImage *modeAggressiveDownImage;
    IOguiImage *modeHoldFireImage;
    IOguiImage *modeHoldFireDownImage;

    // energy, heat
    //IOguiImage *energyImage;
    //IOguiImage *maxEnergyImage;
    //IOguiImage **heatImages;
    //IOguiImage *heatOverlayImage;
    //IOguiImage *energyOverlayImage;
    
    //IOguiImage *unitHPImages[COMBATUNITWINDOW_UNIT_HP_IMAGES];
    IOguiImage *unitHPBlockImage;
    IOguiImage *unitEmptyHPBlockImage;

    IOguiImage *unitEnergyBlockImage;
    IOguiImage *unitEmptyEnergyBlockImage;

    IOguiImage *unitClipBlockImage;
    IOguiImage *unitEmptyClipBlockImage;

    IOguiImage *heatImages[COMBATUNITWINDOW_HEAT_IMAGES]; 

    IOguiImage *partHPImages[COMBATUNITWINDOW_PART_HP_IMGTYPES 
      * COMBATUNITWINDOW_PART_HP_IMAGES];

    // the background
    OguiButton *unitBackBut;

		// face icon
    OguiButton *characterFaceBut;

    // unit name, group, mode and hp
    OguiButton *unitNameBut;
    OguiButton *unitModeBut;
    //OguiButton *unitHPBut;

    // shields
    OguiButton *unitShieldBut;
    OguiButton *unitHeatBut;

    // weapons
    OguiButton *unitWeapButs[COMBATUNITWINDOW_MAX_WEAPONS];
    int lastUnitWeapAmmo[COMBATUNITWINDOW_MAX_WEAPONS];

    // part HPs
    OguiButton *unitPartHPButs[COMBATUNITWINDOW_MAX_PARTS];
    int lastUnitPartHP[COMBATUNITWINDOW_MAX_PARTS];

    // unit HP
    OguiButton *unitHPBlockButs[COMBATUNITWINDOW_MAX_HPBLOCKS];
    int lastUnitHP;

    // unit energy
    OguiButton *unitEnergyBlockButs[COMBATUNITWINDOW_MAX_ENERGYBLOCKS];
    int lastUnitEnergy;

    int unitSelectTime;
    int blinkUnit;

    //OguiButton *unitMaxEnergyBut;
    //OguiButton *unitEnergyBut;
    //OguiButton *unitHeatBut;
    //OguiButton *unitEnergyOverlayBut;
    //OguiButton *unitHeatOverlayBut;


  };

}

#endif

