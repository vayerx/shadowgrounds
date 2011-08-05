
#ifndef ARMORCONSTRUCTWINDOW_H
#define ARMORCONSTRUCTWINDOW_H

#include "../ogui/Ogui.h"

#include "../game/Game.h"
#include "../game/Unit.h"

#include "ArmorPartSelectWindow.h"


namespace ui
{

  class ArmorConstructWindow : public IOguiButtonListener
  {
  public:
    ArmorConstructWindow(Ogui *ogui, game::Game *game, int player);
    ~ArmorConstructWindow();

    virtual void CursorEvent(OguiButtonEvent *eve);

    void show();
    void hide();

    // sets the unit to be currently modified in the window
    void setUnit(game::Unit *unit);

    // redo everything visible in the window
    void refresh();

    // TEMP HACK!
    bool isVisible();

    bool isPartSelectionVisible();

    void cancelPartSelection();

    void setInfoPartType(game::PartType *partType);

  private:
    Ogui *ogui;
    game::Game *game;
    int player;
    game::Unit *unit;
    OguiWindow *win;
    OguiButton *closeBut;

    OguiButtonStyle *slot1Style;  // torso
    IOguiImage *slot1Image;
    OguiButtonStyle *slot2Style;  // leg and arm
    IOguiImage *slot2Image;
    OguiButtonStyle *slot3Style;  // others
    IOguiImage *slot3Image;

    OguiButtonStyle *purchaseButtonStyle;
    IOguiImage *purchaseButtonImage;
    IOguiImage *purchaseButtonDownImage;
    IOguiImage *purchaseButtonHighlightedImage;
    IOguiImage *purchaseButtonDisabledImage;

    IOguiImage *armorPageActiveImage;
    OguiButtonStyle *armorPageStyle;
    OguiButton *armorPageButton;

    IOguiImage *charPageActiveImage;
    OguiButtonStyle *charPageStyle;
    OguiButton *charPageButton;

    OguiButtonStyle *unitSelectStyle;
    OguiButtonStyle *unitSelectActiveStyle;
    //OguiButtonStyle *unitSelectDisabledStyle;
    IOguiImage *unitSelectImage;
    IOguiImage *unitSelectActiveImage;
    IOguiImage *unitSelectDisabledImage;
    //IOguiFont *unitSelectFont;

    OguiTextLabel *title;
    ArmorPartSelectWindow *selectWindow;
    LinkedList *buttons;

    OguiTextLabel *moneyText;
    OguiTextLabel *moneyValText;
    OguiTextLabel *repairText;
    OguiTextLabel *repairValText;
    OguiTextLabel *purchaseText;
    OguiTextLabel *purchaseValText;

    OguiTextLabel *charBioArea;
    OguiTextLabel *charNameLabel;
    OguiButton *charBioImageButton;

    OguiButton *descriptionButton;
    OguiTextLabel *descriptionArea;

    game::PartType *infoPartType;

    int page;  // armor construction or character info

    int cursorMode; // what does the cursor do... (normal, repair, buy)

    void addPartButton(game::Part *p, game::Part *pp, 
      int slotNum, int x, int y);
  };

}

#endif
