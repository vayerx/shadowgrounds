
#ifndef STORAGEWINDOW_H
#define STORAGEWINDOW_H

#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/PartType.h"

namespace ui
{

  /**
   * Storage window class. 
   * Presents the storage inventory of the player.
   *
   * @version 0.5, 25.6.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   */

  class StorageWindow : public IOguiButtonListener, 
    public IOguiSelectListListener
  {
  public:
    StorageWindow(Ogui *ogui, game::Game *game, int player);
    ~StorageWindow();

    void refresh();

    void hide();
    void show();
    bool isVisible();

    void setPartType(game::PartType *partType);
    void setLevelMask(int levelMask);

    void setInfoPartType(game::PartType *partType);

    virtual void CursorEvent(OguiButtonEvent *eve);
    virtual void SelectEvent(OguiSelectListEvent *eve);

  private:
    Ogui *ogui;
    game::Game *game;
    int player;

    game::PartType *partType;
    game::PartType *infoPartType;
    int levelMask;

    int cursorMode;

    OguiWindow *win;
    LinkedList *buttons;

    OguiButton *closeBut;
    OguiButton *buyBut;
    OguiButton *sellBut;

    OguiTextLabel *moneyText;
    OguiTextLabel *moneyValText;

    OguiTextLabel *storageTitle;
    OguiTextLabel *sellTitle;
    OguiTextLabel *buyTitle;

    OguiButtonStyle *partTypeSelectStyle;
    OguiButtonStyle *partTypeSelectActiveStyle;
    IOguiImage *partTypeSelectImage;
    IOguiImage *partTypeSelectActiveImage;
    IOguiImage *partTypeSelectDisabledImage;

    IOguiImage *selImage;
    IOguiImage *selDownImage;
    OguiButtonStyle *unselStyle;
    OguiButtonStyle *selStyle;
    OguiButtonStyle *numUnselStyle;
    OguiButtonStyle *invisibleStyle;

    OguiSelectListStyle *listStyle;
    OguiSelectListStyle *buyPriceStyle;
    OguiSelectListStyle *sellPriceStyle;
    OguiSelectListStyle *storageAmountStyle;

    OguiSelectList *selectList;

    IOguiImage *levelUnselectedImage;
    IOguiImage *levelSelectedImage;
    IOguiImage *levelDownImage;
    IOguiImage *levelSelectedDownImage;
    OguiButtonStyle *levelUnselStyle;
    OguiButtonStyle *levelSelStyle;

    OguiSelectListStyle *levelStyle;

    OguiSelectList *levelList;

    OguiSelectList *buyPriceList;
    OguiSelectList *sellPriceList;
    OguiSelectList *storageAmountList;

    OguiButton *descriptionButton;
    OguiTextLabel *descriptionArea;
  };

}

#endif

