
#include "precompiled.h"

#include <assert.h>

#include "ArmorPartSelectWindow.h"

#include "../convert/str2int.h"
#include "../system/Logger.h"

#include "../game/Part.h"
#include "../game/PartList.h"
#include "../game/PartTypeAvailabilityList.h"
#include "../game/Unit.h"
#include "../game/GameUI.h"
#include "uidefaults.h"

#include "../util/Debug_MemoryManager.h"


using namespace game;


namespace ui
{

  ArmorPartSelectWindow::ArmorPartSelectWindow(Ogui *ogui, game::Game *game, 
    int player)
  {
    this->player = player;
    this->game = game;
    this->ogui = ogui;
    this->partType = NULL;
    this->parentPart = NULL;
    this->slotNumber = -1;
    this->parentUnit = NULL;
    this->infoPartType = NULL;
    this->levelMask = 0;

    win = ogui->CreateSimpleWindow(1024/2-350, 768/2-300, 700, 600, "Data/GUI/Windows/part_select.dds");
    win->Hide();

    closeBut = ogui->CreateSimpleImageButton(win, 8, 8, 16, 16, NULL, NULL, NULL);
    closeBut->SetStyle(defaultCloseButton);
    closeBut->SetListener(this);

    title = NULL;
    //title = ogui->CreateTextLabel(win, 32, 8, 256, 16, "PART SELECTION TEST");
    //title->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);

    storageTitle = ogui->CreateTextLabel(win, 328, 30, 100, 16, "Storage");
    storageTitle->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    repairTitle = ogui->CreateTextLabel(win, 438, 30, 100, 16, "Repair");
    repairTitle->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
    buyTitle = ogui->CreateTextLabel(win, 558, 30, 100, 16, "Buy price");
    buyTitle->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);

    selImage = NULL; //ogui->LoadOguiImage("Data/Buttons/select_selected.tga");
    selDownImage = NULL; //ogui->LoadOguiImage("Data/Buttons/select_down.tga");
    unselStyle = new OguiButtonStyle(NULL, selDownImage, NULL, selImage, defaultFont, 350, 20);
    selStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, defaultFont, 350, 20);
    numUnselStyle = new OguiButtonStyle(selImage, selDownImage, NULL, selImage, defaultFont, 100, 20);
    invisibleStyle = new OguiButtonStyle(NULL, NULL, NULL, NULL, defaultFont, 0, 0);

    listStyle = new OguiSelectListStyle(unselStyle, selStyle, invisibleStyle, invisibleStyle, 350, 320, 0, 0);
    storageAmountStyle = new OguiSelectListStyle(numUnselStyle, numUnselStyle, invisibleStyle, invisibleStyle, 100, 320, 0, 0);
    repairPriceStyle = new OguiSelectListStyle(numUnselStyle, numUnselStyle, invisibleStyle, invisibleStyle, 100, 320, 0, 0);
    buyPriceStyle = new OguiSelectListStyle(numUnselStyle, numUnselStyle, defaultUpButton, defaultDownButton, 116, 320, 16, 16);

    selectList = ogui->CreateSelectList(win, 8, 50, listStyle, 0, NULL, NULL);
    selectList->setListener(this);

    storageAmountList = ogui->CreateSelectList(win, 358, 50, storageAmountStyle, 0, NULL, NULL);
    storageAmountList->setListener(this);

    repairPriceList = ogui->CreateSelectList(win, 468, 50, repairPriceStyle, 0, NULL, NULL);
    repairPriceList->setListener(this);

    buyPriceList = ogui->CreateSelectList(win, 578, 50, buyPriceStyle, 0, NULL, NULL);
    buyPriceList->setListener(this);

    descriptionButton = ogui->CreateSimpleImageButton(win, 8, 380, 80, 80, NULL, NULL, NULL);
    descriptionButton->SetDisabled(true);
    descriptionArea = NULL;
  }

  ArmorPartSelectWindow::~ArmorPartSelectWindow()
  {
    if (closeBut != NULL)
    {
      delete closeBut;
      closeBut = NULL;
    }
    if (title != NULL)
    {
      delete title;
      title = NULL;
    }
    if (storageTitle != NULL)
    {
      delete storageTitle;
      storageTitle = NULL;
    }
    if (repairTitle != NULL)
    {
      delete repairTitle;
      repairTitle = NULL;
    }
    if (buyTitle != NULL)
    {
      delete buyTitle;
      buyTitle = NULL;
    }
    if (selectList != NULL)
    {
      delete selectList;
      selectList = NULL;
    }
    if (repairPriceList != NULL)
    {
      delete repairPriceList;
      repairPriceList = NULL;
    }
    if (buyPriceList != NULL)
    {
      delete buyPriceList;
      buyPriceList = NULL;
    }
    if (storageAmountList != NULL)
    {
      delete storageAmountList;
      storageAmountList = NULL;
    }
    delete unselStyle;
    delete selStyle;
    delete numUnselStyle;
    delete invisibleStyle;
    delete listStyle;
    delete buyPriceStyle;
    delete repairPriceStyle;
    delete storageAmountStyle;
    if (selImage != NULL) 
    {
      delete selImage;
      selImage = NULL;
    }
    if (selDownImage != NULL) 
    {
      delete selDownImage;
      selDownImage = NULL;
    }
    if (descriptionArea != NULL)
    {
      delete descriptionArea;
      descriptionArea = NULL;
    }
    if (descriptionButton != NULL)
    {
      delete descriptionButton;
      descriptionButton = NULL;
    }
    if (win != NULL)
    {
      delete win;
      win = NULL;
    }
  }

  void ArmorPartSelectWindow::CursorEvent(OguiButtonEvent *eve)
  {
    if (eve->triggerButton == closeBut)
    {
      // TODO, stuff...
      hide();
    }
  }

  void ArmorPartSelectWindow::SelectEvent(OguiSelectListEvent *eve)
  {
    if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SCROLL)
    {
      selectList->scrollTo(eve->scrollY);
      storageAmountList->scrollTo(eve->scrollY);
      repairPriceList->scrollTo(eve->scrollY);
      buyPriceList->scrollTo(eve->scrollY);
    }
    if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_CURSOR_OVER)
    {
      if (eve->selectionValue != NULL)
      {
        setInfoPartType(getPartTypeById(
          PARTTYPE_ID_STRING_TO_INT(eve->selectionValue)));
      }
    }
    if (eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT)
    {
      if (eve->selectionValue != NULL)
      {
        // TODO: NETGAME, send proper request to game...

        if (strcmp(eve->selectionValue, "/CAN") != 0)
        {
          PartType *pt = NULL; 

          if (strcmp(eve->selectionValue, "/DEL") != 0)
          {
            // selected a new part
            pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(eve->selectionValue)); 
            if (pt == NULL) 
            {
              (Logger::getInstance())->
                error("ArmorPartSelectWindow::SelectEvent - Part type id unknown.");
              assert(0);
              return;
            }
          } else {
            // remove part (just leave pt null)
          }

          Part *p = NULL;

          if (pt != NULL)
          {
            // if a suitable part in storage, take it from there...
            // use the one least damaged
            int lowestRepairPrice = 999999;
            Part *lowestRPPart = NULL;
            LinkedList *storage = game->parts->getOwnedParts(player);
            storage->resetIterate();
            while (storage->iterateAvailable())
            {
              Part *tmp = (Part *)storage->iterateNext();
              if (tmp->getType() == pt)
              {
                if (tmp->getRepairPrice() < lowestRepairPrice)
                {
                  lowestRepairPrice = tmp->getRepairPrice();
                  lowestRPPart = tmp;
                }
              }
            }
            p = lowestRPPart;

            if (p != NULL)
            {
              // remove from storage
              game->parts->removePart(p);
            } else {
              // else create new part for purchase like this...
              p = pt->getNewPartInstance();
              p->setPurchasePending(true);
              p->setOwner(player);
            }
          }

          // is this a sub part or the root part (torso)?
          if (parentPart == NULL) 
          {
            if (parentUnit == NULL)
            {
              (Logger::getInstance())->error("ArmorPartSelectWindow::SelectEvent - Unit is null.");
              assert(0);
              return;
            } else {
              Part *oldp = parentUnit->getRootPart();
              if (oldp != NULL)
                game->detachParts(parentUnit, oldp);
              parentUnit->setRootPart(p);
            }
          } else {
            // was there an old part in that slot?
            Part *oldp = parentPart->getSubPart(slotNumber);
            if (oldp != NULL)
            {
              // TODO: 
              // can we change for example torso, so that arms,
              // head and legs are kept unremoved... 
              // how about if slot types are wrong...?
              game->detachParts(parentUnit, oldp);
            }
            parentPart->setSubPart(slotNumber, p);
            // done by setSubPart
            //if (p != NULL)
            //  p->setParent(parentPart);
          }

          // TODO: send a request telling that the armor construction window
          // must be refreshed.
          // hack for now...
          game->gameUI->refreshArmorConstructWindow(player);
        } else {
          // cancel
        }
      } else {
        (Logger::getInstance())->
          warning("ArmorPartSelectWindow::SelectEvent - Received null value.");
        assert(0);
      }

      hide();
    }
  }

  void ArmorPartSelectWindow::setParentPart(Part *part)
  {
    this->parentPart = part;
  }

  void ArmorPartSelectWindow::setSlotNumber(int slot)
  {
    this->slotNumber = slot;
  }

  void ArmorPartSelectWindow::setLevelMask(int levelMask)
  {
    this->levelMask = levelMask;
  }

  void ArmorPartSelectWindow::setParentUnit(Unit *unit)
  {
    this->parentUnit = unit;
  }

  // Call this after level mask, etc. because only this one refreshes...
  void ArmorPartSelectWindow::setPartType(PartType *partType)
  {
    this->partType = partType;
    refresh();
  }

  void ArmorPartSelectWindow::setInfoPartType(PartType *partType)
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
        ogui->CreateTextArea(win, 88, 380, 490, 212, 
        partType->getDescription());
    }
    if (partType != NULL && partType->getVisual2D() != NULL)
    {
      // TODO: check for grandparents too!
      if (partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors")))
      {
        descriptionButton->Resize(160, 160);
        descriptionArea->Move(168, 380);
      } else {
        if (partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg"))
          || partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm"))
          || partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap")))
        {
          descriptionButton->Resize(80, 160);
        }  else {
          descriptionButton->Resize(80, 100);
        }
      }
      descriptionButton->SetDisabledImage(partType->getVisual2D()->getImage());
    } else {
      descriptionButton->SetDisabledImage(NULL);
    }
  }

  void ArmorPartSelectWindow::refresh()
  {
    while (!selectList->isEmpty())
    {
      selectList->deleteItem();
      storageAmountList->deleteItem();
      repairPriceList->deleteItem();
      buyPriceList->deleteItem();
    }
    selectList->scrollTo(0);
    storageAmountList->scrollTo(0);
    repairPriceList->scrollTo(0);
    buyPriceList->scrollTo(0);

    if (partType == NULL) 
    {
      #ifdef _DEBUG
        abort();
      #else
        return;
      #endif
    }

    selectList->addItem("/CAN", "(Cancel)");
    storageAmountList->addItem("/CAN", "");
    repairPriceList->addItem("/CAN", "");
    buyPriceList->addItem("/CAN", "");

    // add remove choice only if a part already exists
    Part *oldp = NULL;
    if (parentPart != NULL)
    {
      oldp = parentPart->getSubPart(slotNumber);
    }
    if (oldp != NULL 
      || (parentPart == NULL 
      && parentUnit != NULL && parentUnit->getRootPart() != NULL))
    {
      selectList->addItem("/DEL", "(Remove part)");
      storageAmountList->addItem("/DEL", "");
      repairPriceList->addItem("/DEL", "");
      buyPriceList->addItem("/DEL", "");
    }

    LinkedList *avail = game->partTypesAvailable->
      getAvailablePartTypes(player);

    avail->resetIterate();
    while (avail->iterateAvailable())
    {
      PartType *pt = (PartType *)avail->iterateNext();
      // check if the part is of correct type and level...
      // TODO: this is limited to part type's grand grand parent!
      // need to check further...
      if ((pt == partType 
        || pt->getParentType() == partType 
        || (pt->getParentType() != NULL 
        && (pt->getParentType()->getParentType() == partType
        || (pt->getParentType()->getParentType() != NULL 
        && pt->getParentType()->getParentType()->getParentType() == partType))))
        && (pt->getLevel() & levelMask) != 0)
      {
        // slot type is parent or grandparent of the parttype in list
        selectList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          pt->getName());
        
        // check how much we have these in storage...
        // find out lowest repair price
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

        storageAmountList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          int2str(storageAmount));
        const char *tmp = "";
        if (storageAmount > 0) tmp = int2str(lowestRepairPrice);
        repairPriceList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          tmp);
        tmp = "";
        if (storageAmount == 0) tmp = int2str(pt->getPrice());
        buyPriceList->addItem(PARTTYPE_ID_INT_TO_STRING(pt->getPartTypeId()), 
          tmp);
      }
    }
  }

  void ArmorPartSelectWindow::show()
  {
    refresh();
    win->Show();
    win->SetOnlyActive(); // TODO: SPLIT SCREEN - NO NO...
  }

  void ArmorPartSelectWindow::hide()
  {
    setInfoPartType(NULL);
    win->Hide();
    win->RestoreAllActive(); // TODO: SPLIT SCREEN - NO NO...
  }

  bool ArmorPartSelectWindow::isVisible()
  {
    return win->IsVisible(); 
  }

}
