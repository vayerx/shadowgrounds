
#ifndef ARMORPARTSELECTWINDOW_H
#define ARMORPARTSELECTWINDOW_H

//
// This is the armor part selection window.
// Appears when you click a slot in the armor construction window.
//
// v1.0 - 20.5.2002 - jpkokkon
//

#include "../container/LinkedList.h"

#include "../ogui/Ogui.h"

#include "../game/Game.h"
#include "../game/PartType.h"


namespace ui
{

  class ArmorPartSelectWindow : public IOguiButtonListener, 
    public IOguiSelectListListener
  {
  public:
    ArmorPartSelectWindow(Ogui *ogui, game::Game *game, int player);
    ~ArmorPartSelectWindow();

    virtual void CursorEvent(OguiButtonEvent *eve);
    virtual void SelectEvent(OguiSelectListEvent *eve);

    void setPartType(game::PartType *partType);
    void setInfoPartType(game::PartType *partType);

    void setParentPart(game::Part *part);
    void setSlotNumber(int slot);
    void setLevelMask(int levelMask);
    void setParentUnit(game::Unit *unit);

    // nope, not like this... when part selected, send proper request to game
    // the game will then create the part and inform construct window to refresh
    //Part *getSelectedPart();

    void show();
    void hide();

    void refresh();

    bool isVisible();

  private:
    Ogui *ogui;
    game::Game *game;
    int player;

    int levelMask;
    game::PartType *partType;
    game::PartType *infoPartType;
    //game::PartType *selectedPartType;

    game::Part *parentPart;
    game::Unit *parentUnit;
    int slotNumber;

    OguiWindow *win;
    OguiButton *closeBut;
    OguiTextLabel *title;

    OguiTextLabel *storageTitle;
    OguiTextLabel *repairTitle;
    OguiTextLabel *buyTitle;

    IOguiImage *selImage;
    IOguiImage *selDownImage;
    OguiButtonStyle *unselStyle;
    OguiButtonStyle *selStyle;
    OguiButtonStyle *numUnselStyle;
    OguiButtonStyle *invisibleStyle;

    OguiSelectListStyle *listStyle;
    OguiSelectListStyle *buyPriceStyle;
    OguiSelectListStyle *repairPriceStyle;
    OguiSelectListStyle *storageAmountStyle;

    OguiSelectList *selectList;

    OguiSelectList *buyPriceList;
    OguiSelectList *repairPriceList;
    OguiSelectList *storageAmountList;

    //IOguiImage *descriptionImage; // get from part visual...
    OguiButton *descriptionButton;
    OguiTextLabel *descriptionArea;
  };

}

#endif
