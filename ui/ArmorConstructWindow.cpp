
#include "precompiled.h"

#include "ArmorConstructWindow.h"

#include "../system/Logger.h"
#include "../convert/str2int.h"
#include "../container/LinkedList.h"
#include "../ogui/Ogui.h"
#include "uidefaults.h"
#include "cursordefs.h"
#include "../game/Torso.h"
#include "../game/Unit.h"
#include "../game/UnitList.h"
#include "../game/Part.h"
#include "../game/Weapon.h"
#include "../game/WeaponObject.h"
#include "../game/AmmoPackObject.h"
#include "../game/PartList.h"
#include "../game/Character.h"
#include "../game/GameUI.h"
#include "../game/Unit.h"

#include "../util/Debug_MemoryManager.h"

// button ids...
// unit (armor) selection
#define ACW_UNIT 3
// armor construction page selection
#define ACW_SEL_ARMORPAGE 4
// character page selection
#define ACW_SEL_CHARPAGE 5
// purchase all button
#define ACW_PURCHASE_ALL 6
// repair all button
#define ACW_REPAIR_ALL 7
// purchase button
#define ACW_PURCHASE 8
// repair button
#define ACW_REPAIR 9
// autocomplete button
#define ACW_AUTOCOMPLETE 10

// empty part slot (ids start from this, 
// the id is 100 + slot number)
#define ACW_SLOT_START 100
#define ACW_SLOT_END 199
// used part slot (+ slot number like above)
#define ACW_PART_START 200
#define ACW_PART_END 299

// page selection (armor construction or character)
#define ACW_PAGE_ARMOR 1
#define ACW_PAGE_CHAR 2

// how many units are visible in the construction window 
#define UNITS_PER_ACW 6

// where to draw the armor slots in the window
#define ARMOR_ROOT_X 455
#define ARMOR_ROOT_Y 377

// cursor modes
#define ACW_CURSOR_MODE_NORMAL 1
#define ACW_CURSOR_MODE_REPAIR 2
#define ACW_CURSOR_MODE_PURCHASE 3


using namespace game;

namespace ui
{
  ArmorConstructWindow::ArmorConstructWindow(Ogui *ogui, Game *game, 
    int player)
  {
    this->player = player;
    this->game = game;
    this->ogui = ogui;
    this->unit = NULL;
    this->page = ACW_PAGE_ARMOR;
    this->cursorMode = ACW_CURSOR_MODE_NORMAL;
    this->infoPartType = NULL;

    // window
    win = ogui->CreateSimpleWindow(0, 0, 1024, 768, "Data/GUI/Windows/armor_construct.dds");
    win->Hide();

    // close button
    //closeBut = ogui->CreateSimpleImageButton(win, 8, 8, 16, 16, NULL, NULL, NULL);
    //closeBut->SetStyle(defaultCloseButton);
    //closeBut->SetListener(this);

    closeBut = ogui->CreateSimpleTextButton(win, 922, 722, 96, 27, 
      "Data/GUI/Buttons/armorexit.tga", "Data/GUI/Buttons/armorexit_down.tga", 
      "Data/GUI/Buttons/armorexit_highlight.tga", "");
    closeBut->SetListener(this);

    // empty armor slots
    slot1Image = ogui->LoadOguiImage("Data/Pictures/Parts/slot1.tga");
    slot1Style = new OguiButtonStyle(slot1Image, slot1Image, slot1Image, slot1Image, NULL, 
      176, 156);

    slot2Image = ogui->LoadOguiImage("Data/Pictures/Parts/slot2.tga");
    slot2Style = new OguiButtonStyle(slot2Image, slot2Image, slot2Image, slot2Image, NULL, 
      75, 156);

    slot3Image = ogui->LoadOguiImage("Data/Pictures/Parts/slot3.tga");
    slot3Style = new OguiButtonStyle(slot3Image, slot3Image, slot3Image, slot3Image, NULL, 
      75, 92);

    moneyValText = NULL;
    repairValText = NULL;
    purchaseValText = NULL;
    moneyText = NULL;
    repairText = NULL;
    purchaseText = NULL;

    // purchase, repair and autocomplete buttons
    purchaseButtonImage = ogui->LoadOguiImage("Data/GUI/Buttons/armormod_purchase.tga");
    purchaseButtonDownImage = ogui->LoadOguiImage("Data/GUI/Buttons/armormod_purchase_down.tga");
    purchaseButtonHighlightedImage = ogui->LoadOguiImage("Data/GUI/Buttons/armormod_purchase_highlight.tga");
    purchaseButtonDisabledImage = NULL;
    //purchaseButtonImage = NULL;
    //purchaseButtonDownImage = NULL;
    //purchaseButtonHighlightedImage = NULL;
    //purchaseButtonDisabledImage = NULL;
    purchaseButtonStyle = new OguiButtonStyle(purchaseButtonImage, 
      purchaseButtonDownImage, purchaseButtonDisabledImage, purchaseButtonHighlightedImage, 
      defaultFont, 170, 20);

    // armor/character page selection buttons
    armorPageActiveImage = ogui->LoadOguiImage("Data/GUI/Buttons/armorpage_active.tga");
    armorPageStyle = new OguiButtonStyle(NULL, armorPageActiveImage, 
      armorPageActiveImage, NULL, NULL, 
      69, 20);
    armorPageButton = ogui->CreateSimpleImageButton(win, 80, 248, 69, 20, NULL, NULL, NULL, ACW_SEL_ARMORPAGE);
    armorPageButton->SetStyle(armorPageStyle);
    armorPageButton->SetListener(this);
    armorPageButton->SetDisabled(true);

    charPageActiveImage = ogui->LoadOguiImage("Data/GUI/Buttons/charpage_active.tga");
    charPageStyle = new OguiButtonStyle(NULL, 
      charPageActiveImage, charPageActiveImage, NULL, NULL, 
      69, 20);
    charPageButton = ogui->CreateSimpleImageButton(win, 80, 248, 69, 20, NULL, NULL, NULL, ACW_SEL_CHARPAGE);
    charPageButton->SetStyle(charPageStyle);
    charPageButton->SetListener(this);

    // unit selection buttons
    unitSelectImage = ogui->LoadOguiImage("Data/GUI/Buttons/unitselect_inactive.tga");
    unitSelectActiveImage = ogui->LoadOguiImage("Data/GUI/Buttons/unitselect_active.tga");
    unitSelectDisabledImage = NULL; //ogui->LoadOguiImage("Data/GUI/Buttons/unitselect3.tga");
    unitSelectStyle = new OguiButtonStyle(unitSelectImage, unitSelectActiveImage, unitSelectDisabledImage, unitSelectImage,
      defaultThinWhiteFont, 94, 29);
    unitSelectActiveStyle = new OguiButtonStyle(unitSelectActiveImage, unitSelectActiveImage, unitSelectDisabledImage, unitSelectActiveImage,
      defaultThinWhiteFont, 94, 29);
    //unitSelectFont = ogui->LoadFont("Data/GUI/Fonts/armor_construct1.ogf");
    //unitSelectStyle = new OguiButtonStyle(unitSelectImage, unitSelectImage, unitSelectImage, 
    //  unitSelectFont, 120, 80);

    // title
    title = NULL;
    //title = ogui->CreateTextLabel(win, 32, 8, 256, 16, "ARMOR BUILD TEST");
    //title->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);

    // for character view...
    charBioImageButton = NULL;
    charBioArea = NULL;
    charNameLabel = NULL;

    descriptionButton = ogui->CreateSimpleImageButton(win, 74, 657, 80, 100, NULL, NULL, NULL);
    descriptionButton->SetDisabled(true);
    descriptionArea = NULL;

    // all the other buttons get stored here
    buttons = new LinkedList();

    // part selection window
    selectWindow = new ArmorPartSelectWindow(ogui, game, player);
  }

  ArmorConstructWindow::~ArmorConstructWindow()
  {
    delete selectWindow;
    while (!buttons->isEmpty())
    {
      delete (OguiButton *)buttons->popLast();
    }
    delete buttons;
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
    if (moneyText != NULL)
    {
      delete moneyText;
      moneyText = NULL;
    }
    if (repairText != NULL)
    {
      delete repairText;
      repairText = NULL;
    }
    if (purchaseText != NULL)
    {
      delete purchaseText;
      purchaseText = NULL;
    }
    if (moneyValText != NULL)
    {
      delete moneyValText;
      moneyValText = NULL;
    }
    if (repairValText != NULL)
    {
      delete repairValText;
      repairValText = NULL;
    }
    if (purchaseValText != NULL)
    {
      delete purchaseValText;
      purchaseValText = NULL;
    }
    if (purchaseButtonStyle != NULL)
    {
      delete purchaseButtonStyle;
      purchaseButtonStyle = NULL;
    }
    if (purchaseButtonImage != NULL)
    {
      delete purchaseButtonImage;
      purchaseButtonImage = NULL;
    }
    if (purchaseButtonDisabledImage != NULL)
    {
      delete purchaseButtonDisabledImage;
      purchaseButtonDisabledImage = NULL;
    }
    if (purchaseButtonDownImage != NULL)
    {
      delete purchaseButtonDownImage;
      purchaseButtonDownImage = NULL;
    }
    if (purchaseButtonHighlightedImage != NULL)
    {
      delete purchaseButtonHighlightedImage;
      purchaseButtonHighlightedImage = NULL;
    }
    if (armorPageActiveImage != NULL)
    {
      delete armorPageActiveImage;
      armorPageActiveImage = NULL;
    }
    if (armorPageStyle != NULL)
    {
      delete armorPageStyle;
      armorPageStyle = NULL;
    }
    if (armorPageButton != NULL)
    {
      delete armorPageButton;
      armorPageButton = NULL;
    }
    if (charPageActiveImage != NULL)
    {
      delete charPageActiveImage;
      charPageActiveImage = NULL;
    }
    if (charPageStyle != NULL)
    {
      delete charPageStyle;
      charPageStyle = NULL;
    }
    if (charPageButton != NULL)
    {
      delete charPageButton;
      charPageButton = NULL;
    }
    if (charBioImageButton != NULL)
    {
      delete charBioImageButton;
      charBioImageButton = NULL;
    }
    if (charNameLabel != NULL)
    {
      delete charNameLabel;
      charNameLabel = NULL;
    }
    if (charBioArea != NULL)
    {
      delete charBioArea;
      charBioArea = NULL;
    }
    if (slot1Style != NULL)
    {
      delete slot1Style;
      slot1Style = NULL;
    }
    if (slot1Image != NULL)
    {
      delete slot1Image;
      slot1Image = NULL;
    }
    if (slot2Style != NULL)
    {
      delete slot2Style;
      slot2Style = NULL;
    }
    if (slot2Image != NULL)
    {
      delete slot2Image;
      slot2Image = NULL;
    }
    if (slot3Style != NULL)
    {
      delete slot3Style;
      slot3Style = NULL;
    }
    if (slot3Image != NULL)
    {
      delete slot3Image;
      slot3Image = NULL;
    }
    if (unitSelectStyle != NULL)
    {
      delete unitSelectStyle;
      unitSelectStyle = NULL;
    }
    if (unitSelectActiveStyle != NULL)
    {
      delete unitSelectActiveStyle;
      unitSelectActiveStyle = NULL;
    }
    if (unitSelectImage != NULL)
    {
      delete unitSelectImage;
      unitSelectImage = NULL;
    }
    if (unitSelectActiveImage != NULL)
    {
      delete unitSelectActiveImage;
      unitSelectActiveImage = NULL;
    }
    if (unitSelectDisabledImage != NULL)
    {
      delete unitSelectDisabledImage;
      unitSelectDisabledImage = NULL;
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

  void ArmorConstructWindow::CursorEvent(OguiButtonEvent *eve)
  {
    // cannot refresh before no-longer referring to eve->triggerButton as
    // refresh might invalidate that
    bool doRefresh = false;

    if (eve->triggerButton == closeBut)
    {
      // TODO, something maybe...
      hide();
    }

    if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
    {
      if (eve->triggerButton->GetId() >= ACW_PART_START
        && eve->triggerButton->GetId() <= ACW_PART_END)
      {
        Part *p = (Part *)eve->extraArgument;
        if (p == NULL)
          setInfoPartType(NULL);
        else
          setInfoPartType(p->getType());
      }
    }
    if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
    {
      if ((eve->triggerButton->GetId() >= ACW_PART_START
        && eve->triggerButton->GetId() <= ACW_PART_END)
        || (eve->triggerButton->GetId() >= ACW_SLOT_START
        && eve->triggerButton->GetId() <= ACW_SLOT_END))
      {
        Part *p = NULL;  // part
        Part *pp = NULL; // parent part
        int slotNum = 0;
        PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors")); 
        // ...should make a check based on p/pt to be sure this is torso slot

        // solve proper part type for the slot...
        if (eve->triggerButton->GetId() >= ACW_PART_START
          && eve->triggerButton->GetId() <= ACW_PART_END)
        {
          p = (Part *)eve->extraArgument;
          pp = p->getParent();
          if (pp != NULL) 
          {
            pt = NULL;
            int amount = pp->getType()->getSlotAmount();
            for (int i = 0; i < amount; i++)
            {
              if (pp->getSubPart(i) == p)
              {
                // this is valid part type to this slot...
                pt = pp->getType()->getSlotType(i);
              }
            }
            if (pt == NULL) abort();
          }
          slotNum = eve->triggerButton->GetId() - ACW_PART_START;
        }
        if (eve->triggerButton->GetId() >= ACW_SLOT_START
          && eve->triggerButton->GetId() <= ACW_SLOT_END)
        {
          slotNum = eve->triggerButton->GetId() - ACW_SLOT_START;
          // this was the slots parent part
          pp = (Part *)eve->extraArgument;
          // this is valid part type to this slot...
          //pt = (PartType *)eve->extraArgument;
          if (pp != NULL)
          {
            // this is the valid part type for this slot
            pt = pp->getType()->getSlotType(slotNum);
          }
        }

        // normal cursor mode, select new part
        if (cursorMode == ACW_CURSOR_MODE_NORMAL)
        {
          selectWindow->setPartType(pt);
          if (pp != NULL)
            selectWindow->setLevelMask(pp->getType()->getSlotLevelMask(slotNum));
          else
            selectWindow->setLevelMask(PARTTYPE_LEVEL_MASK_ALL);
          selectWindow->setParentPart(pp);
          selectWindow->setSlotNumber(slotNum);
          selectWindow->setParentUnit(unit);

          selectWindow->show();
        } 
        // repair mode
        if (cursorMode == ACW_CURSOR_MODE_REPAIR)
        {
          // TODO: NETGAME, send proper request...
          int reloadPrice = 0;
          if (p != NULL)
          {
            // is ammo...
            if (p->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Ammo"))))
            {
              AmmoPackObject *apo = (AmmoPackObject *)p;
              reloadPrice = apo->getReloadPrice();
            }
            // is weapon...
            if (p->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
            {
              // WARNING: unsafe cast! (based on check above)
              WeaponObject *wo = (WeaponObject *)p;
              reloadPrice = wo->getReloadPrice();
            }
          }
          
          if (p != NULL)
          {
            if (game->money[player] >= 
              p->getRepairPrice() + reloadPrice)
            {
              // repair
              game->payForPart(p);

              // pay for reload
              game->money[player] -= reloadPrice;
              if (p != NULL)
              {
                // reload ammo
                if (p->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Ammo"))))
                {
                  AmmoPackObject *apo = (AmmoPackObject *)p;
                  apo->reload();
                }
                // reload weapon
                if (p->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
                {
                  // WARNING: unsafe cast! (based on check above)
                  WeaponObject *wo = (WeaponObject *)p;
                  wo->reload();
                }
              }
            } else {
              // TODO: not enough money messagebox
            }
            doRefresh = true;
          } else {
            // no part or not damaged
          }
        }
        // purchase mode
        if (cursorMode == ACW_CURSOR_MODE_PURCHASE)
        {
          // TODO: NETGAME, send proper request...
          if (p != NULL && p->isPurchasePending())
          {
            if (game->money[player] >= p->getType()->getPrice())
            {
              game->payForPart(p);
            } else {
              // TODO: not enough money messagebox
            }
            doRefresh = true;
          } else {
            // no part or already puchased
          }
        }

      }
    }
    if (eve->triggerButton->GetId() == ACW_UNIT)
    {
      setUnit((Unit *)eve->extraArgument);
      // TODO: if split screen, solve player cursor number first
      ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
      cursorMode = ACW_CURSOR_MODE_NORMAL;
      doRefresh = true;
      setInfoPartType(NULL);
    }
    if (eve->triggerButton->GetId() == ACW_SEL_ARMORPAGE)
    {
      page = ACW_PAGE_ARMOR;
      armorPageButton->SetDisabled(true);
      charPageButton->SetDisabled(false);
      doRefresh = true;
      // TODO: if split screen, solve player cursor number first
      ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
      cursorMode = ACW_CURSOR_MODE_NORMAL;
      setInfoPartType(NULL);
    }
    if (eve->triggerButton->GetId() == ACW_SEL_CHARPAGE)
    {
      page = ACW_PAGE_CHAR;
      armorPageButton->SetDisabled(false);
      charPageButton->SetDisabled(true);
      doRefresh = true;
      // TODO: if split screen, solve player cursor number first
      ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
      cursorMode = ACW_CURSOR_MODE_NORMAL;
      setInfoPartType(NULL);
    }
    if (eve->triggerButton->GetId() == ACW_REPAIR_ALL)
    {
      if (unit != NULL)
      {
        if (unit->getRootPart() != NULL)
        {
          if (game->money[player] > 
            game->calculateRepairPrice(unit->getRootPart())
            + game->calculateReloadPrice(unit->getRootPart()))
          {
            game->repairParts(unit->getRootPart());
            game->reloadParts(unit->getRootPart());
            doRefresh = true;
          } else {
            // TODO: not enough money
          }
        }
      }
      // TODO: if split screen, solve player cursor number first
      ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
      cursorMode = ACW_CURSOR_MODE_NORMAL;
    }
    if (eve->triggerButton->GetId() == ACW_PURCHASE_ALL)
    {
      if (unit != NULL)
      {
        if (unit->getRootPart() != NULL)
        {
          if (game->money[player] > 
            game->calculatePurchasePrice(unit->getRootPart()))
          {
            game->purchaseParts(unit->getRootPart());
            doRefresh = true;
          } else {
            // TODO: not enough money
          }
        }
      }
      // TODO: if split screen, solve player cursor number first
      ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
      cursorMode = ACW_CURSOR_MODE_NORMAL;
    }
    if (eve->triggerButton->GetId() == ACW_PURCHASE)
    {
      if (cursorMode == ACW_CURSOR_MODE_PURCHASE) 
      {
        // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
        ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
#endif
        cursorMode = ACW_CURSOR_MODE_NORMAL;
      } else {
        // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
        ogui->SetCursorImageState(0, DH_CURSOR_BUY);
#endif
        cursorMode = ACW_CURSOR_MODE_PURCHASE;
      }
    }
    if (eve->triggerButton->GetId() == ACW_REPAIR)
    {
      if (cursorMode == ACW_CURSOR_MODE_REPAIR) 
      {
        // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
        ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
#endif
        cursorMode = ACW_CURSOR_MODE_NORMAL;
      } else {
        // TODO: if split screen, solve player cursor number first
#ifdef PROJECT_SHADOWGROUNDS
        ogui->SetCursorImageState(0, DH_CURSOR_REPAIR);
#endif
        cursorMode = ACW_CURSOR_MODE_REPAIR;
      }
    }

    if (doRefresh)
      refresh();
  }

  void ArmorConstructWindow::hide()
  {
    // TODO: if split screen, solve player cursor number first
    ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
    cursorMode = ACW_CURSOR_MODE_NORMAL;

    // TODO: make proper game request!
    // (not really needed, as game state is not changed at this point?)

    if (selectWindow->isVisible())
    {
      selectWindow->hide();
    }
    win->Hide();
    game->gameUI->openCommandWindow(player);
  }

  void ArmorConstructWindow::show()
  {
    refresh();
    win->Show();
  }

  void ArmorConstructWindow::setUnit(Unit *unit)
  {
    this->unit = unit;
    //refresh();
    // can't refresh here, might invalidate the eve->triggerButtor in 
    // the cursorevent handler
  }

  void ArmorConstructWindow::addPartButton(Part *p, Part *pp, int slotNumber, int x, int y)
  {
    OguiButton *b;
    OguiButton *b3;
    int slotSize = 3;
    if (pp == NULL)
    {
      b = ogui->CreateSimpleImageButton(win, x-88, y-78, 176, 156,
        NULL, NULL, NULL, ACW_SLOT_START + slotNumber, pp);
      b->SetStyle(slot1Style);
      slotSize = 1;

      b3 = ogui->CreateSimpleTextButton(win, x-88, y-78-14, 176, 16,
        NULL, NULL, NULL, "TORSO", 0);
      b3->SetFont(defaultRedInfoFont);
      b3->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b3->SetDisabled(true);
      buttons->append(b3);
    } else {
      if (pp->getType()->getSlotType(slotNumber) == 
        getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm"))
        || pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg"))
        || pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap")))
      {
        b = ogui->CreateSimpleImageButton(win, x-38, y-78, 75, 156, 
          NULL, NULL, NULL, ACW_SLOT_START + slotNumber, pp);
        b->SetStyle(slot2Style);
        slotSize = 2; 

        if (pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg"))
          && pp->getType()->getSlotPosition(slotNumber) == SLOT_POSITION_LEFT_LEG)
        {
          b3 = ogui->CreateSimpleTextButton(win, x-38, y-78-14, 75, 16,
          NULL, NULL, NULL, "LEFT LEG", 0);
          b3->SetFont(defaultRedInfoFont);
          b3->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
          b3->SetDisabled(true);
          buttons->append(b3);
        }
        if (pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg"))
          && pp->getType()->getSlotPosition(slotNumber) == SLOT_POSITION_RIGHT_LEG)
        {
          b3 = ogui->CreateSimpleTextButton(win, x-38, y-78-14, 75, 16,
          NULL, NULL, NULL, "RIGHT LEG", 0);
          b3->SetFont(defaultRedInfoFont);
          b3->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
          b3->SetDisabled(true);
          buttons->append(b3);
        }
        if (pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm"))
          && pp->getType()->getSlotPosition(slotNumber) == SLOT_POSITION_LEFT_ARM)
        {
          b3 = ogui->CreateSimpleTextButton(win, x-38-20, y-78-14, 75, 16,
          NULL, NULL, NULL, "LEFT ARM", 0);
          b3->SetFont(defaultRedInfoFont);
          b3->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
          b3->SetDisabled(true);
          buttons->append(b3);
        }
        if (pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm"))
          && pp->getType()->getSlotPosition(slotNumber) == SLOT_POSITION_RIGHT_ARM)
        {
          b3 = ogui->CreateSimpleTextButton(win, x-38-20, y-78-14, 75, 16,
          NULL, NULL, NULL, "RIGHT ARM", 0);
          b3->SetFont(defaultRedInfoFont);
          b3->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
          b3->SetDisabled(true);
          buttons->append(b3);
        }
      } else {
        b = ogui->CreateSimpleImageButton(win, x-38, y-46, 75, 92, 
          NULL, NULL, NULL, ACW_SLOT_START + slotNumber, pp);
        b->SetStyle(slot3Style);
        slotSize = 3; 

        if (pp->getType()->getSlotType(slotNumber) == 
          getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Head"))
          && pp->getType()->getSlotPosition(slotNumber) == SLOT_POSITION_HEAD)
        {
          b3 = ogui->CreateSimpleTextButton(win, x-38, y-46-14, 75, 16,
          NULL, NULL, NULL, "HEAD", 0);
          b3->SetFont(defaultRedInfoFont);
          b3->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
          b3->SetDisabled(true);
          buttons->append(b3);
        }
      }
    }
    b->SetListener(this);
    buttons->append(b);

    bool showDamaged = false;
    bool showReload = false;
    int priceval = 0;
    if (p != NULL)
    {
      b->SetDisabled(true);
      const char *pricetext = "";
      if (p->isPurchasePending())
      {
        priceval = p->getType()->getPrice();
      } else {
        if (p->getRepairPrice() > 0)
        {
          priceval = p->getRepairPrice();
          showDamaged = true;
        }
      }
      // is ammo, show reload?
      if (p->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Ammo"))))
      {
        AmmoPackObject *apo = (AmmoPackObject *)p;
        if (apo->getReloadPrice() > 0)
        {
          priceval += apo->getReloadPrice();
          showReload = true;
        }
      }
      // is weapon, show reload?
      if (p->getType()->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
      {
        // WARNING: unsafe cast! (based on check above)
        WeaponObject *wo = (WeaponObject *)p;
        if (wo->getReloadPrice() > 0)
        {
          priceval += wo->getReloadPrice();
          showReload = true;
        }
      }

      if (priceval > 0)
        pricetext = int2str(priceval);

      OguiButton *b2;
      if (slotSize == 1)
      {
        b2 = ogui->CreateSimpleTextButton(win, x-80, y-80, 
          160, 160, NULL, NULL, NULL, pricetext, ACW_PART_START + slotNumber, p);
      } else {
        if (slotSize == 2)
        {
          b2 = ogui->CreateSimpleTextButton(win, x-40, y-80, 
            80, 160, NULL, NULL, NULL, pricetext, ACW_PART_START + slotNumber, p);
        } else {
          b2 = ogui->CreateSimpleTextButton(win, x-40, y-40-10, 
            80, 100, NULL, NULL, NULL, pricetext, ACW_PART_START + slotNumber, p);
        }
      }
      b2->SetTextVAlign(OguiButton::TEXT_V_ALIGN_BOTTOM);
      Visual2D *vis = NULL;
      if (x < ARMOR_ROOT_X)
        vis = p->getType()->getMirrorVisual2D();
      else
        vis = p->getType()->getVisual2D();
      if (vis != NULL) 
      {
        b2->SetImage(vis->getImage());
        b2->SetDownImage(vis->getImage());
        b2->SetHighlightedImage(vis->getImage());
      }
      b2->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER);
      b2->SetListener(this);
      buttons->append(b2);

      if (showReload)
      {
        int ty = y - 8;
        if (showDamaged) ty -= 8;
        OguiButton *b4;
        b4 = ogui->CreateSimpleTextButton(win, x-30, ty, 60, 16,
        NULL, NULL, NULL, "RELOAD", 0);
        b4->SetFont(defaultSmallRedFont);
        b4->SetDisabled(true);
        buttons->append(b4);
      }
      if (showDamaged)
      {
        int ty = y - 8;
        if (showReload) ty += 8;
        OguiButton *b4;
        b4 = ogui->CreateSimpleTextButton(win, x-30, ty, 60, 16,
        NULL, NULL, NULL, "DAMAGED", 0);
        b4->SetFont(defaultSmallRedFont);
        b4->SetDisabled(true);
        buttons->append(b4);
      }

      int amount = p->getType()->getSlotAmount();
      for (int i = 0; i < amount; i++)
      {
        int newx = x;
        int newy = y;
        int pos = p->getType()->getSlotPosition(i);
        if (pos == SLOT_POSITION_EXTERNAL_ITEM
          || pos == SLOT_POSITION_INTERNAL_ITEM)
        {
          if (x < ARMOR_ROOT_X) newx = x - 90; else newx = x + 90;
          newy = y + 60 + i * 100;
        } else {
          switch(pos)
          {
          case SLOT_POSITION_HEAD:
            newy = y - 128;
            break;
          case SLOT_POSITION_RIGHT_ARM:
            newx = x - 160;
            break;
          case SLOT_POSITION_LEFT_ARM:
            newx = x + 160;
            break;
          case SLOT_POSITION_RIGHT_LEG:
            newx = x - 45;
            newy = y + 170;
            break;
          case SLOT_POSITION_LEFT_LEG:
            newx = x + 45;
            newy = y + 170;
            break;
          case SLOT_POSITION_RIGHT_WAIST:
            newx = x - 130;
            newy = y + 140;
            break;
          case SLOT_POSITION_LEFT_WAIST:
            newx = x + 130;
            newy = y + 140;
            break;
          case SLOT_POSITION_RIGHT_SHOULDER:
            newx = x - 100;
            newy = y - 128;
            break;
          case SLOT_POSITION_LEFT_SHOULDER:
            newx = x + 100;
            newy = y - 128;
            break;
          case SLOT_POSITION_RIGHT_HEAD:
            newx = x - 250;
            newy = y;
            break;
          case SLOT_POSITION_LEFT_HEAD:
            newx = x + 250;
            newy = y;
            break;
          case SLOT_POSITION_BACK:
            newx = x - 345;
            newy = y + 150;
            break;
          case SLOT_POSITION_CENTER:
            newx = x + 250;
            newy = y + 200;
            break;
          // TODO: rest of the positions
          default:
            Logger::getInstance()->error("ArmorConstructWindow::addPartButton - Unknown slot position.");
            assert(0); // unexpected (unimplemented) position
            return;
          }
        }
        addPartButton(p->getSubPart(i), p, i, 
          newx, newy);
      }
    } 
  }

  void ArmorConstructWindow::refresh()
  {
    // remove old buttons
    while (!buttons->isEmpty())
    {
      delete (OguiButton *)buttons->popLast();
    }
    if (charBioArea != NULL)
    {
      delete charBioArea;
      charBioArea = NULL;
    }
    if (charBioImageButton != NULL)
    {
      delete charBioImageButton;
      charBioImageButton = NULL;
    }
    if (charNameLabel != NULL)
    {
      delete charNameLabel;
      charNameLabel = NULL;
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
    if (repairText != NULL)
    {
      delete repairText;
      repairText = NULL;
    }
    if (repairValText != NULL)
    {
      delete repairValText;
      repairValText = NULL;
    }
    if (purchaseText != NULL)
    {
      delete purchaseText;
      purchaseText = NULL;
    }
    if (purchaseValText != NULL)
    {
      delete purchaseValText;
      purchaseValText = NULL;
    }

    // add unit buttons
    LinkedList *ownu = game->units->getOwnedUnits(player);
    ownu->resetIterate();
    int count = 0;
    while (ownu->iterateAvailable())
    {
      Unit *u = (Unit *)ownu->iterateNext();

      // character name assigned to this unit (armor)
      const char *cname = NULL;
      char *cnamebuf = NULL;
      if (u->getCharacter() != NULL)
      {
        cname = u->getCharacter()->getName();
        // make a linebreak between first name and lastname
        // causes a problem with text center alignment
        /*
        cnamebuf = new char[strlen(cname) + 1];
        strcpy(cnamebuf, cname);
        for (int i = strlen(cname) - 1; i >= 0; i--)
        {
          if (cnamebuf[i] == ' ') 
          {
            cnamebuf[i] = '\n';
            break;
          }
        }
        cname = cnamebuf;
        */
      } else {
        cname = "(NONE)";
      }

      OguiButton *b = ogui->CreateSimpleTextButton(win, 62 + count * 100, 142, 
        94, 29, NULL, NULL, NULL, cname, ACW_UNIT, u);
      if (u == unit)
        b->SetStyle(unitSelectActiveStyle);
      else
        b->SetStyle(unitSelectStyle);
      //b->SetLineBreaks(true);
      b->SetListener(this);
      buttons->append(b);

      if (cnamebuf != NULL)
        delete [] cnamebuf;

      count++;
      if (count >= UNITS_PER_ACW) break;
    }

    // pad with empty unit selection buttons
    for (; count < UNITS_PER_ACW; count++)
    {
      OguiButton *b = ogui->CreateSimpleTextButton(win, 62 + count * 100, 142, 
        94, 29, NULL, NULL, NULL, "(EMPTY)", ACW_UNIT, NULL);
      b->SetStyle(unitSelectStyle);
      b->SetListener(this);
      b->SetDisabled(true);
      buttons->append(b);
    }

    if (page == ACW_PAGE_ARMOR)
    {
      // money and stuff
      int repairval = 0;
      int purchaseval = 0;
      int reloadval = 0;
      if (unit != NULL && unit->getRootPart() != NULL)
      {
        repairval = game->calculateRepairPrice(unit->getRootPart());
        purchaseval = game->calculatePurchasePrice(unit->getRootPart());
        reloadval = game->calculateReloadPrice(unit->getRootPart());
      }
      moneyText = ogui->CreateTextLabel(win, 850, 220, 170, 20, 
        "Money:");
      moneyText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      moneyValText = ogui->CreateTextLabel(win, 850, 240, 170, 20, 
        int2str(game->money[player]));
      //moneyValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);
      moneyValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      moneyValText->SetFont(defaultSmallRedFont);

      repairText = ogui->CreateTextLabel(win, 850, 260, 170, 20, 
        "Total repairs:");
      repairText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      repairValText = ogui->CreateTextLabel(win, 850, 280, 170, 20, 
        int2str(repairval + reloadval));
      //repairValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);
      repairValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      repairValText->SetFont(defaultSmallRedFont);

      purchaseText = ogui->CreateTextLabel(win, 850, 300, 170, 20, 
        "Purchases:");
      purchaseText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      purchaseValText = ogui->CreateTextLabel(win, 850, 320, 170, 20, 
        int2str(purchaseval));
      //purchaseValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);
      purchaseValText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      purchaseValText->SetFont(defaultSmallRedFont);

      bool hasPurchases = true;
      bool hasRepairs = true;
      if (unit == NULL || unit->getRootPart() == NULL 
        || game->calculatePurchasePrice(unit->getRootPart()) == 0)
      {
        hasPurchases = false;
        if (cursorMode == ACW_CURSOR_MODE_PURCHASE) 
        {
          // TODO: if split screen, solve player cursor number first
          ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
          cursorMode = ACW_CURSOR_MODE_NORMAL;
        }
      }
      if (unit == NULL || unit->getRootPart() == NULL 
        || (game->calculateRepairPrice(unit->getRootPart()) == 0
        && game->calculateReloadPrice(unit->getRootPart()) == 0))
      {
        hasRepairs = false; 
        if (cursorMode == ACW_CURSOR_MODE_REPAIR) 
        {
          // TODO: if split screen, solve player cursor number first
          ogui->SetCursorImageState(0, DH_CURSOR_ARROW);
          cursorMode = ACW_CURSOR_MODE_NORMAL;
        }
      }

      // add purchase, repair and autocomplete buttons
      OguiButton *b = ogui->CreateSimpleTextButton(win, 850, 408, 
        170, 20, NULL, NULL, NULL, "PURCHASE", ACW_PURCHASE, NULL);
      b->SetStyle(purchaseButtonStyle);
      b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b->SetListener(this);
      if (!hasPurchases)
      {
        b->SetFont(defaultDisabledFont);
        b->SetDisabled(true);
      }
      buttons->append(b);

      b = ogui->CreateSimpleTextButton(win, 850, 433, 
        170, 20, NULL, NULL, NULL, "PURCHASE ALL", ACW_PURCHASE_ALL, NULL);
      b->SetStyle(purchaseButtonStyle);
      b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b->SetListener(this);
      if (!hasPurchases)
      {
        b->SetFont(defaultDisabledFont);
        b->SetDisabled(true);
      }
      buttons->append(b);

      b = ogui->CreateSimpleTextButton(win, 850, 458, 
        170, 20, NULL, NULL, NULL, "REPAIR", ACW_REPAIR, NULL);
      b->SetStyle(purchaseButtonStyle);
      b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b->SetListener(this);
      if (!hasRepairs)
      {
        b->SetFont(defaultDisabledFont);
        b->SetDisabled(true);
      }
      buttons->append(b);

      b = ogui->CreateSimpleTextButton(win, 850, 483, 
        170, 20, NULL, NULL, NULL, "REPAIR ALL", ACW_REPAIR_ALL, NULL);
      b->SetStyle(purchaseButtonStyle);
      b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b->SetListener(this);
      if (!hasRepairs)
      {
        b->SetFont(defaultDisabledFont);
        b->SetDisabled(true);
      }
      buttons->append(b);

      b = ogui->CreateSimpleTextButton(win, 850, 508, 
        170, 20, NULL, NULL, NULL, "AUTOCOMPLETE", ACW_AUTOCOMPLETE, NULL);
      b->SetStyle(purchaseButtonStyle);
      b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      b->SetListener(this);

      b->SetFont(defaultDisabledFont);
      b->SetDisabled(true);
      
      buttons->append(b);

      // start part recursion from torso
      if (unit != NULL)
      {
        Part *p = unit->getRootPart();
        //PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors"));

        addPartButton(p, NULL, 0, ARMOR_ROOT_X, ARMOR_ROOT_Y);
      }
    }
    if (unit != NULL)
    {
      const char *charname = NULL;
      const char *charbio = NULL;
      Character *c = unit->getCharacter();
      if (c != NULL)
      {
        charname = c->getFullname();
        charbio = c->getBio();
        if (charname == NULL) charname = "?";
        if (charbio == NULL) charbio = "(No bio available)";
      } else {
        charname = "No mercenary for this armor";
        charbio = "";
      }
      charNameLabel = ogui->CreateTextLabel(win, 80, 220, 
        160, 16, charname);
      charNameLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
      if (page == ACW_PAGE_CHAR)
      {
        charBioArea = ogui->CreateTextArea(win, 300, 300, 
          450, 200, charbio);
        charBioImageButton = ogui->CreateSimpleImageButton(win, 80, 300, 
          160, 160, NULL, NULL, NULL, NULL, 0, NULL);
        charBioImageButton->SetDisabled(true);
        if (c != NULL)
        {
          Visual2D *vis = c->getImage();
          if (vis != NULL)
            charBioImageButton->SetDisabledImage(vis->getImage());
        }
      }
    }
  }

  void ArmorConstructWindow::setInfoPartType(PartType *partType)
  {
    this->infoPartType = partType;
    if (descriptionArea != NULL)
    {
      delete descriptionArea;
      descriptionArea = NULL;
    }
    if (partType != NULL)
    {
      if (partType->getDescription() != NULL)
      {
        descriptionArea = 
          ogui->CreateTextArea(win, 74+105, 657, 700, 100, 
          partType->getDescription());
        descriptionArea->SetFont(defaultRedInfoFont);
      }
    }
    if (partType != NULL && partType->getVisual2D() != NULL)
    {
      // TODO: check for grandparents too!
      if (partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors")))
      {
        descriptionButton->Resize(100, 100);
        descriptionButton->Move(74, 657);
      } else {
        if (partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Leg"))
          || partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Arm"))
          || partType->getParentType() == getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap")))
        {
          descriptionButton->Resize(50, 100);
          descriptionButton->Move(104, 657);
        } else {
          descriptionButton->Resize(60, 75);
          descriptionButton->Move(99, 682);
        }
      }
      descriptionButton->SetDisabledImage(partType->getVisual2D()->getImage());
    } else {
      descriptionButton->SetDisabledImage(NULL);
    }
  }

  bool ArmorConstructWindow::isVisible()
  {
    return win->IsVisible();
  }

  bool ArmorConstructWindow::isPartSelectionVisible()
  { 
    return selectWindow->isVisible();
  }

  void ArmorConstructWindow::cancelPartSelection()
  { 
    if (selectWindow->isVisible())
      selectWindow->hide();
  }

}

