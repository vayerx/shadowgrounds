
#include "precompiled.h"

#include "StorageWindow.h"

#include "../convert/str2int.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"

#include "uidefaults.h"
#include "cursordefs.h"
#include "../game/PartTypeAvailabilityList.h"
#include "../game/PartList.h"
#include "../game/Part.h"
#include "../game/GameUI.h"

#include "../util/Debug_MemoryManager.h"

// button id's for storage window
#define STORAGEW_EXIT 1
#define STORAGEW_PARTTYPE 2
#define STORAGEW_BUY 3
#define STORAGEW_SELL 4

// select list id's
#define STORAGEW_LEVELSEL 1

#define STORAGEW_CURSOR_MODE_BUY 1
#define STORAGEW_CURSOR_MODE_SELL 2


using namespace game;


namespace ui
{

  StorageWindow::StorageWindow(Ogui *ogui, game::Game *game, int player)
  {
    this->player = player;
    this->game = game;
    this->ogui = ogui;

    this->partType = NULL;
    this->infoPartType = NULL;
    this->levelMask = 0;

    this->cursorMode = STORAGEW_CURSOR_MODE_BUY;

    buttons = new LinkedList();

    win = ogui->CreateSimpleWindow(0, 0, 1024, 768, "Data/GUI/Windows/storagewindow.dds");
    win->Hide();

    closeBut = ogui->CreateSimpleTextButton(win, 800, 695, 200, 50, 
      "Data/Buttons/storageexit.tga", "Data/Buttons/storageexit_down.tga", 
      "Data/Buttons/storageexit_highlight.tga", "Exit", STORAGEW_EXIT);
    closeBut->SetListener(this);

    buyBut = ogui->CreateSimpleTextButton(win, 800, 595, 200, 50, 
      "Data/Buttons/purchase1.tga", "Data/Buttons/purchase1_down.tga", 
      "Data/Buttons/purchase1_highlight.tga", "Buy", STORAGEW_BUY);
    buyBut->SetListener(this);

    sellBut = ogui->CreateSimpleTextButton(win, 800, 645, 200, 50, 
      "Data/Buttons/purchase1.tga", "Data/Buttons/purchase1_down.tga", 
      "Data/Buttons/purchase1_highlight.tga", "Sell", STORAGEW_SELL);
    sellBut->SetListener(this);

    OguiButton *b;

    b = ogui->CreateSimpleTextButton(win, 800, 695, 200, 50,
      "Data/Buttons/storageexit.tga", "Data/Buttons/storageexit_down.tga",
      "Data/Buttons/storageexit_highlight.tga", "Exit", STORAGEW_EXIT);
    b->SetListener(this);
    buttons->append(b);

    storageTitle = ogui->CreateTextLabel(win, 390, 90, 100, 16, "Storage");
    storageTitle->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    buyTitle = ogui->CreateTextLabel(win, 500, 90, 100, 16, "Buy");
    buyTitle->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    sellTitle = ogui->CreateTextLabel(win, 610, 90, 100, 16, "Sell");
    sellTitle->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);

    // part type selection buttons
    partTypeSelectImage = NULL; //ogui->LoadOguiImage("Data/Pictures/Parts/unitselect1.tga");
    partTypeSelectActiveImage = ogui->LoadOguiImage("Data/GUI/Buttons/parttypeselect_active.tga");
    partTypeSelectDisabledImage = NULL; //ogui->LoadOguiImage("Data/Pictures/Parts/unitselect3.tga");
    partTypeSelectStyle = new OguiButtonStyle(partTypeSelectImage, 
      partTypeSelectActiveImage, partTypeSelectDisabledImage, partTypeSelectImage,
      defaultFont, 116, 80);
    partTypeSelectActiveStyle = new OguiButtonStyle(partTypeSelectActiveImage, 
      partTypeSelectActiveImage, partTypeSelectDisabledImage, partTypeSelectActiveImage,
      defaultFont, 116, 80);

    // invisible scroll buttons
    invisibleStyle = new OguiButtonStyle(NULL, NULL, NULL, NULL, defaultFont, 0, 0);

    // level list
    levelUnselectedImage = ogui->LoadOguiImage("Data/GUI/Buttons/checkbut.tga");
    levelSelectedImage = ogui->LoadOguiImage("Data/GUI/Buttons/checkbut_selected.tga");
    levelDownImage = ogui->LoadOguiImage("Data/GUI/Buttons/checkbut_down.tga");
    levelSelectedDownImage = ogui->LoadOguiImage("Data/GUI/Buttons/checkbut_selected_down.tga");

    levelUnselStyle = new OguiButtonStyle(levelUnselectedImage, levelDownImage, NULL, levelUnselectedImage, defaultFont, 160, 20);
    levelSelStyle = new OguiButtonStyle(levelSelectedImage, levelSelectedDownImage, NULL, levelSelectedImage, defaultFont, 160, 20);

    levelStyle = new OguiSelectListStyle(levelUnselStyle, levelSelStyle, invisibleStyle, invisibleStyle, 160, 100, 0, 0);

    levelList = ogui->CreateSelectList(win, 800, 110, levelStyle, 0, NULL, NULL, true, -1, STORAGEW_LEVELSEL);
    levelList->setListener(this);
    levelList->addItem("1", "  Level 1");
    levelList->addItem("2", "  Level 2");
    levelList->addItem("3", "  Level 3");
    levelList->addItem("4", "  Level 4");
    levelList->addItem("5", "  Level 5");
    levelList->setSelected(0, true);
    levelList->setSelected(1, true);
    levelList->setSelected(2, true);
    levelList->setSelected(3, true);
    levelList->setSelected(4, true);
    levelMask = 1 | 2 | 4 | 8 | 16;  // (1 << 0) | (1 << 1) | ...

    // select list
    selImage = NULL; //ogui->LoadOguiImage("Data/Buttons/select_selected.tga");
    selDownImage = NULL; //ogui->LoadOguiImage("Data/Buttons/select_down.tga");
    unselStyle = new OguiButtonStyle(NULL, selDownImage, NULL, selImage, defaultFont, 350, 20);
    selStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, defaultFont, 350, 20);
    numUnselStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, defaultFont, 100, 20);

    listStyle = new OguiSelectListStyle(unselStyle, selStyle, invisibleStyle, invisibleStyle, 350, 320, 0, 0);
    storageAmountStyle = new OguiSelectListStyle(numUnselStyle, numUnselStyle, invisibleStyle, invisibleStyle, 100, 320, 0, 0);
    buyPriceStyle = new OguiSelectListStyle(numUnselStyle, numUnselStyle, invisibleStyle, invisibleStyle, 100, 320, 0, 0);
    sellPriceStyle = new OguiSelectListStyle(numUnselStyle, numUnselStyle, defaultUpButton, defaultDownButton, 116, 320, 16, 16);

    selectList = ogui->CreateSelectList(win, 40, 110, listStyle, 0, NULL, NULL);
    selectList->setListener(this);

    storageAmountList = ogui->CreateSelectList(win, 390, 110, storageAmountStyle, 0, NULL, NULL);
    storageAmountList->setListener(this);

    buyPriceList = ogui->CreateSelectList(win, 500, 110, buyPriceStyle, 0, NULL, NULL);
    buyPriceList->setListener(this);

    sellPriceList = ogui->CreateSelectList(win, 610, 110, sellPriceStyle, 0, NULL, NULL);
    sellPriceList->setListener(this);

    // info
    descriptionButton = ogui->CreateSimpleImageButton(win, 40, 550, 80, 80, NULL, NULL, NULL);
    descriptionButton->SetDisabled(true);
    descriptionArea = NULL;

    moneyText = NULL;
    moneyValText = NULL;

  }

  StorageWindow::~StorageWindow()
  {
    while (!buttons->isEmpty())
    {
      delete (OguiButton *)buttons->popLast();
    }
    delete buttons;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    // TODO: delete stuff!!!

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    if (win != NULL)
    {
      delete win;
      win = NULL;
    }
  }

  void StorageWindow::setLevelMask(int levelMask)
  {
    this->levelMask = levelMask;
  }

  // call after level mask, etc. (only one that refreshes)
  void StorageWindow::setPartType(PartType *partType)
  {
    this->partType = partType;
    refresh();
  }

  void StorageWindow::setInfoPartType(PartType *partType)
  {
    this->infoPartType = partType;
    if (descriptionArea != NULL)
    {
      delete descriptionArea;
      descriptionArea = NULL;
    }
    if (partType != NULL)
    {
      descriptionArea = 
        ogui->CreateTextArea(win, 128, 550, 490, 200, 
        partType->getDescription());
    }
    if (partType != NULL && partType->getVisual2D() != NULL)
    {
      // TODO: check for grandparents too!
      if (partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors")))
      {
        descriptionButton->Resize(160, 160);
        descriptionArea->Move(208, 600);
      } else {
        if (partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg"))
          || partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm"))
          || partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap")))
        {
          descriptionButton->Resize(80, 160);
        }  else {
          descriptionButton->Resize(80, 80);
        }
      }
      descriptionButton->SetDisabledImage(partType->getVisual2D()->getImage());
    } else {
      descriptionButton->SetDisabledImage(NULL);
    }
  }

  void StorageWindow::refresh()
  {
    while (!buttons->isEmpty())
    {
      delete (OguiButton *)buttons->popLast();
    }
    if (moneyText != NULL)
    {
      delete moneyText;
      moneyText = NULL;
    }
    if (moneyValText != NULL)
    {
      delete moneyValText;
      moneyValText = NULL;
    }

    moneyText = ogui->CreateTextLabel(win, 800, 514, 200, 20, 
      "Money:");
    moneyText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    moneyValText = ogui->CreateTextLabel(win, 800, 530, 200, 20, 
      int2str(game->money[player]));
    moneyValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);

    // add parttype selection buttons
    OguiButton *b;

    b = ogui->CreateSimpleTextButton(win, 103 + 0 * 116, 4, 
      116, 80, NULL, NULL, NULL, "Torso", STORAGEW_PARTTYPE, "Tors");
    if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors")))
      b->SetStyle(partTypeSelectActiveStyle);
    else
      b->SetStyle(partTypeSelectStyle);
    b->SetListener(this);
    buttons->append(b);

    b = ogui->CreateSimpleTextButton(win, 103 + 1 * 116, 4, 
      116, 80, NULL, NULL, NULL, "Head", STORAGEW_PARTTYPE, "Head");
    if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Head")))
      b->SetStyle(partTypeSelectActiveStyle);
    else
      b->SetStyle(partTypeSelectStyle);
    b->SetListener(this);
    buttons->append(b);

    b = ogui->CreateSimpleTextButton(win, 103 + 2 * 116, 4, 
      116, 80, NULL, NULL, NULL, "Arm", STORAGEW_PARTTYPE, "Arm");
    if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm")))
      b->SetStyle(partTypeSelectActiveStyle);
    else
      b->SetStyle(partTypeSelectStyle);
    b->SetListener(this);
    buttons->append(b);

    b = ogui->CreateSimpleTextButton(win, 103 + 3 * 116, 4, 
      116, 80, NULL, NULL, NULL, "Leg", STORAGEW_PARTTYPE, "Leg");
    if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg")))
      b->SetStyle(partTypeSelectActiveStyle);
    else
      b->SetStyle(partTypeSelectStyle);
    b->SetListener(this);
    buttons->append(b);

    b = ogui->CreateSimpleTextButton(win, 103 + 4 * 116, 4, 
      116, 80, NULL, NULL, NULL, "Weapon", STORAGEW_PARTTYPE, "Weap");
    if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap")))
      b->SetStyle(partTypeSelectActiveStyle);
    else
      b->SetStyle(partTypeSelectStyle);
    b->SetListener(this);
    buttons->append(b);

    b = ogui->CreateSimpleTextButton(win, 103 + 5 * 116, 4, 
      116, 80, NULL, NULL, NULL, "Misc", STORAGEW_PARTTYPE, "Pack");
    if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Pack")))
      b->SetStyle(partTypeSelectActiveStyle);
    else
      b->SetStyle(partTypeSelectStyle);
    b->SetListener(this);
    buttons->append(b);

    // delete old items...
    while (!selectList->isEmpty())
    {
      selectList->deleteItem();
      storageAmountList->deleteItem();
      sellPriceList->deleteItem();
      buyPriceList->deleteItem();
    }
    selectList->scrollTo(0);
    storageAmountList->scrollTo(0);
    sellPriceList->scrollTo(0);
    buyPriceList->scrollTo(0);

    LinkedList *avail = game->partTypesAvailable->
      getAvailablePartTypes(player);

    avail->resetIterate();
    while (avail->iterateAvailable())
    {
      PartType *pt = (PartType *)avail->iterateNext();
      
      // PartType *secondPartType = NULL;
      //if (partType == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Pack")))
      //  PartType *secondPartType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Reac"))

      // check if the part is of correct type and level...
      // TODO: this is limited to part type's grand grand parent!
      // need to check further...
      /*
      if ((pt == partType 
        || pt->getParentType() == partType 
        || (pt->getParentType() != NULL 
        && (pt->getParentType()->getParentType() == partType
        || (pt->getParentType()->getParentType() != NULL 
        && pt->getParentType()->getParentType()->getParentType() == partType))))
        && (pt->getLevel() & levelMask) != 0
        && partType != NULL)
      */
      if (partType != NULL
        && (pt->getLevel() & levelMask) != 0
        && pt->isInherited(partType))
      {
        // check how much we have these in storage...
        int storageAmount = 0;
        int lowestRepairPrice = 999999;
        LinkedList *inStorage = game->parts->getOwnedParts(player);
        inStorage->resetIterate();
        while (inStorage->iterateAvailable())
        {
          Part *p = (Part *)inStorage->iterateNext();
          if (p->getType() == pt)
          {
            storageAmount++;
            if (p->getRepairPrice() < lowestRepairPrice)
              lowestRepairPrice = p->getRepairPrice();
          }
        }

        // slot type is parent or grandparent of the parttype in list
        selectList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          pt->getName());        

        // TODO: add "(damaged)" after a damaged part's name

        storageAmountList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          int2str(storageAmount));
        const char *tmp = "";
        if (storageAmount > 0) tmp = int2str((pt->getPrice() / 2) - lowestRepairPrice);
        sellPriceList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          tmp);
        tmp = "";
        tmp = int2str(pt->getPrice());
        buyPriceList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          tmp);
      }
    }

  }

  void StorageWindow::hide()
  {
    // TODO: gamerequest?
    win->Hide();

    // TODO: if split screen, solve player cursor number first
    ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
    cursorMode = STORAGEW_CURSOR_MODE_BUY;
  }

  void StorageWindow::show()
  {
    refresh();
    win->Show();

    // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
    ogui->SetCursorImageState(0, DH_CURSOR_BUY);
#endif
    cursorMode = STORAGEW_CURSOR_MODE_BUY;
  }

  bool StorageWindow::isVisible()
  {
    return win->IsVisible();
  }

  void StorageWindow::CursorEvent(OguiButtonEvent *eve)
  {
    bool doRefresh = false;

    if (eve->triggerButton->GetId() == STORAGEW_EXIT)
    {
      // TODO: NETGAME, replace this with proper request!
      // (...not needed maybe - does not affect game state?)

      hide();
      game->gameUI->openCommandWindow(player);
    }

    if (eve->triggerButton->GetId() == STORAGEW_BUY)
    {
      // TODO: NETGAME, replace this with proper request!
      // (...not needed maybe - does not affect game state?)

      // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
      ogui->SetCursorImageState(0, DH_CURSOR_BUY);
#endif
      cursorMode = STORAGEW_CURSOR_MODE_BUY;

      /*
      if (infoPartType != NULL)
      { 
        if (game->money[player] >= infoPartType->getPrice())
        {
          game->money[player] -= infoPartType->getPrice();
          Part *p = infoPartType->getNewPartInstance();
          p->setOwner(player);
          game->parts->addPart(p);
          refresh();
        } else {
          // TODO: msgbox, not enough money
        }
      }
      */
    }
    if (eve->triggerButton->GetId() == STORAGEW_SELL)
    {
      // TODO: NETGAME, replace this with proper request!
      // (...not needed maybe - does not affect game state?)

      // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
      ogui->SetCursorImageState(0, DH_CURSOR_SELL);
#endif
      cursorMode = STORAGEW_CURSOR_MODE_SELL;

      /*
      if (infoPartType != NULL)
      { 
        // check how much we have these in storage...
        int storageAmount = 0;
        int lowestRepairPrice = 999999;
        Part *lowestDamagedPart = NULL;
        LinkedList *inStorage = game->parts->getOwnedParts(player);
        inStorage->resetIterate();
        while (inStorage->iterateAvailable())
        {
          Part *p = (Part *)inStorage->iterateNext();
          if (p->getType() == infoPartType)
          {
            storageAmount++;
            lowestDamagedPart = p;
            if (p->getRepairPrice() < lowestRepairPrice)
              lowestRepairPrice = p->getRepairPrice();
          }
        }

        if (storageAmount > 0)
        {
          game->parts->removePart(lowestDamagedPart);
          game->money[player] += infoPartType->getPrice() / 2 - lowestRepairPrice;
        }
        refresh();
      }
      */
    }
    if (eve->triggerButton->GetId() == STORAGEW_PARTTYPE)
    {
      partType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT((char *)eve->extraArgument));
      doRefresh = true;
    }

    if (doRefresh)
      refresh();
  }

  void StorageWindow::SelectEvent(OguiSelectListEvent *eve)
  {
    if (eve->triggerSelectList->getId() == STORAGEW_LEVELSEL)
    {
      if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT)
      {
        int val = str2int(eve->selectionValue);
        levelMask |= (1 << (val - 1));
        refresh();
      }
      if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_UNSELECT)
      {
        int val = str2int(eve->selectionValue);
        levelMask &= (PARTTYPE_LEVEL_MASK_ALL ^ (1 << (val - 1)));
        refresh();
      }
    } else {
      if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT
        || eve->eventType == OguiSelectListEvent::EVENT_TYPE_CURSOR_OVER)
      {
        PartType *pt;
        pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(eve->selectionValue));
        setInfoPartType(pt);
      }
      if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT)
      {
        if (cursorMode == STORAGEW_CURSOR_MODE_BUY)
        {
          // TODO: NETGAME, replace this with proper request!
          // (...not needed maybe - does not affect game state?)

          if (infoPartType != NULL)
          { 
            if (game->money[player] >= infoPartType->getPrice())
            {
              game->money[player] -= infoPartType->getPrice();
              Part *p = infoPartType->getNewPartInstance();
              p->setOwner(player);
              game->parts->addPart(p);
              refresh();
            } else {
              // TODO: msgbox, not enough money
            }
          }
        }
        if (cursorMode == STORAGEW_CURSOR_MODE_SELL)
        {
          // TODO: NETGAME, replace this with proper request!
          // (...not needed maybe - does not affect game state?)

          if (infoPartType != NULL)
          { 
            // check how much we have these in storage...
            int storageAmount = 0;
            int lowestRepairPrice = 999999;
            Part *lowestDamagedPart = NULL;
            LinkedList *inStorage = game->parts->getOwnedParts(player);
            inStorage->resetIterate();

            while (inStorage->iterateAvailable())
            {
              Part *p = (Part *)inStorage->iterateNext();
              if (p->getType() == infoPartType)
              {
                storageAmount++;
                lowestDamagedPart = p;
                if (p->getRepairPrice() < lowestRepairPrice)
                  lowestRepairPrice = p->getRepairPrice();
              }
            }

            if (storageAmount > 0)
            {
              game->parts->removePart(lowestDamagedPart);
              game->money[player] += infoPartType->getPrice() / 2 - lowestRepairPrice;
              delete lowestDamagedPart;
            }
            refresh();
          }
        }

      }
    }
  }

}

