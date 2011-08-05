
#ifndef COMBATRADAR_H
#define COMBATRADAR_H

#include "../ogui/Ogui.h"

//#define COMBAT_RADAR_DIRECTION_IMAGES 8
#define COMBAT_RADAR_DIRECTION_IMAGES 1

namespace game
{
  class Game;
  class Unit;
}

namespace ui
{

  class CombatRadar : public IOguiButtonListener
  {
  public:
    CombatRadar(Ogui *ogui, game::Game *game, int player, OguiWindow *window);

    ~CombatRadar();

    void update();

    void setOrigo(float x, float y);

    void setAngle(float angle);

    virtual void CursorEvent(OguiButtonEvent *eve);

  private:
    Ogui *ogui;
    game::Game *game;
    int player;
    OguiWindow *win;

    float angle;
    float radarX;
    float radarY;

    IOguiImage *friendly1Image;
    IOguiImage *friendly2Image;
    IOguiImage *hostile1Image;
    IOguiImage *hostile2Image;
    IOguiImage *radarBackground;

    OguiButton *radarBut;

    OguiButton *radarScanBut;

    IOguiImage *directionImages[COMBAT_RADAR_DIRECTION_IMAGES];
		OguiButton *directionBut;

    int radarUnitsAmount;
    OguiButton **radarUnitButs;
    game::Unit **radarUnits;
    float *radarUnitDist;
    float *radarUnitAngle;
    int *radarUnitType;

		bool radarBeeped;
		int radarSweepNumber;

		void updateDirectionPointer();

    void drawImpl();

  };

}

#endif

