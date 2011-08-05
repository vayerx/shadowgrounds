#include "CommandWindow.h"

#ifdef TURHACRAP



#include "OptionsWindow.h"
#include "../game/Unit.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/Part.h"
#include "../game/UnitList.h"
#include "../game/DHLocaleManager.h"
#include "cursordefs.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"
#include "../util/Checksummer.h"

#include "..\util\Debug_MemoryManager.h"


// button id's for command window
#define COMMANDW_MAINMENU 1
#define COMMANDW_COMBAT 2
#define COMMANDW_ARMOR 3
#define COMMANDW_STORAGE 4
#define COMMANDW_QUIT 5

using namespace game;

namespace ui
{

  CommandWindow::CommandWindow(Ogui *ogui, game::Game *game, int player)
  {
    this->player = player;
    this->game = game;
    this->ogui = ogui;
    this->quitPressed = false;
    buttons = new LinkedList();

    win = ogui->CreateSimpleWindow(0, 0, 1024, 768, "Data/GUI/Windows/menu_nebula_background.tga");
    win->Hide();

	

    OguiButton *b;

    /*
    b = ogui->CreateSimpleTextButton(win, 16, 16, 200, 50, 
      "Data/Buttons/mainmenu.tga", "Data/Buttons/mainmenu_down.tga",
      "Data/Buttons/mainmenu_highlight.tga", "Main menu", COMMANDW_MAINMENU);
    b->SetListener(this);
    buttons->append(b);
    */

    //b = ogui->CreateSimpleTextButton(win, 512-250, 700, 250, 50, 
    //  "Data/GUI/Buttons/armormod.tga", "Data/GUI/Buttons/armormod_down.tga",
    //  "Data/GUI/Buttons/armormod_highlight.tga", "Modify armors", COMMANDW_ARMOR);
    //b->SetListener(this);
		//b->SetDisabled(true); // disable armor modification button
    //buttons->append(b);

	

    //b = ogui->CreateSimpleTextButton(win, 512, 700, 250, 50, 
    //  "Data/GUI/Buttons/storage.tga", "Data/GUI/Buttons/storage_down.tga",
    //  "Data/GUI/Buttons/storage_highlight.tga", "Storage", COMMANDW_STORAGE);
    //b->SetListener(this);
		//b->SetDisabled(true); // disable storage button
    //buttons->append(b);

	
	font = ogui->LoadFont( "Data/Fonts/mainmenu_big.ogf" );

	b = ogui->CreateSimpleTextButton( win, 65, 315, 573, 27, 
		"Data/GUI/Buttons/mainmenu_button_normal.tga", "Data/GUI/Buttons/mainmenu_button_high.tga",
		"Data/GUI/Buttons/mainmenu_button_high.tga", getLocaleGuiString( "gui_mm_new_game" ), COMMANDW_COMBAT );
	b->SetListener( this );
	buttons->append( b );
	b->SetFont( font );
	startButton = b;
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	int i;
	for( i = 1; i <= 3; i++ )
	{
		b = ogui->CreateSimpleTextButton( win, 65, 315+(28*i), 573, 27, 
			"Data/GUI/Buttons/mainmenu_button_normal.tga", "Data/GUI/Buttons/mainmenu_button_high.tga",
			"Data/GUI/Buttons/mainmenu_button_high.tga", getLocaleGuiString( "gui_mm_options" ), COMMANDW_COMBAT );
		b->SetListener( this );
		buttons->append( b );
	    b->SetFont( font );
		b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}

	b = ogui->CreateSimpleTextButton( win, 65, 462, 573, 27, 
		"Data/GUI/Buttons/mainmenu_button_normal.tga", "Data/GUI/Buttons/mainmenu_button_high.tga",
		"Data/GUI/Buttons/mainmenu_button_high.tga", getLocaleGuiString( "gui_mm_credits" ), COMMANDW_COMBAT );
	b->SetListener( this );
	buttons->append( b );
	b->SetFont( font );
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	b = ogui->CreateSimpleTextButton( win, 65, 462+28, 573, 27, 
		"Data/GUI/Buttons/mainmenu_button_normal.tga", "Data/GUI/Buttons/mainmenu_button_high.tga",
		"Data/GUI/Buttons/mainmenu_button_high.tga", getLocaleGuiString( "gui_mm_quit" ), COMMANDW_QUIT );
	b->SetListener( this );
	buttons->append( b );
	b->SetFont( font );
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	/*
    b = ogui->CreateSimpleTextButton(win, 512-250*2, 710, 250, 50, 
      "Data/GUI/Buttons/mainquit.tga", "Data/GUI/Buttons/mainquit_down.tga",
      "Data/GUI/Buttons/mainquit_highlight.tga", getLocaleGuiString("gui_quit"), COMMANDW_QUIT);
    b->SetListener(this);
    buttons->append(b);

    b = ogui->CreateSimpleTextButton(win, 512+250, 710, 250, 50, 
      "Data/GUI/Buttons/combat.tga", "Data/GUI/Buttons/combat_down.tga",
      "Data/GUI/Buttons/combat_highlight.tga", getLocaleGuiString("gui_mission_start_mission"), COMMANDW_COMBAT);
    b->SetListener(this);
    buttons->append(b);
		startButton = b;
	*/
		/*
		int chksum1 = 1280806280;
		int chksize1 = 11701276;
		chksum1 ^= 3000100245;
		chksize1 ^= 11000300;
		if (!util::Checksummer::doesChecksumAndSizeMatchFile(chksum1, chksize1, "Data/GUI/Windows/command.dds"))
		{
			b->SetDisabled(true);
		}
		*/

    ogui->SetCursorImageState(0, DH_CURSOR_ARROW);

	optionsWindow = new OptionsWindow(game, ogui, player);

	// Foreground 
	foreground = ogui->CreateSimpleWindow( 0, 0, 1024, 768, "Data/GUI/Windows/menu_planet_foreground.tga" );
	foreground->SetReactMask( 0 );
  }

  CommandWindow::~CommandWindow()
  {
	delete optionsWindow;

	delete foreground;
		
    while (!buttons->isEmpty())
    {
      delete (OguiButton *)buttons->popLast();
    }
    delete buttons;
    if (win != NULL)
    {
      delete win;
      win = NULL;
    }

	delete font;
  }

  void CommandWindow::hide()
  {
    // remove the parts player has not paid for!
    // ...done in startCombat.

    // TODO: make proper game request!

    win->Hide();
	foreground->Hide();
  }

  void CommandWindow::show()
  {
    //refresh();
    win->Show();
	foreground->Show();
	

		// TODO: cursor number!
		startButton->Focus(0);
  }

  bool CommandWindow::isVisible()
  {
    return win->IsVisible();
  }

  bool CommandWindow::wasQuitPressed()
  {
    if (quitPressed)
    {
      quitPressed = false;
      return true;
    } else {
      return false;
    }
  }

  void CommandWindow::startMission()
  {
    // WARNING: COPY & PASTE PROGRAMMING AHEAD (SEE Game.cpp)

    bool noArmor = false;
    bool notPaid = false;
    bool incompleteArmor = false;
    bool notAnyArmor = true;
    LinkedList *ulist = game->units->getOwnedUnits(player);
    LinkedListIterator iter = LinkedListIterator(ulist);
    while (iter.iterateAvailable())
    {
      Unit *u = (Unit *)iter.iterateNext();
      if (u->getRootPart() == NULL && u->getCharacter() != NULL)
      {
        noArmor = true;
      } else {
        bool wasOk = true;
        if (u->getCharacter() == NULL)
        {
          wasOk = false;
        } else {
          if (game->calculatePurchasePrice(u->getRootPart()) > 0)
          {
            notPaid = true;
            wasOk = false;
          }
          int slotAmount = u->getRootPart()->getType()->getSlotAmount();
          for (int i = 0; i < slotAmount; i++) 
          {
            if (u->getRootPart()->getSubPart(i) == NULL)
            {
              if (
                u->getRootPart()->getType()->getSlotType(i)->getPartTypeId() 
                == PARTTYPE_ID_STRING_TO_INT("Head")
                || u->getRootPart()->getType()->getSlotType(i)->getPartTypeId() 
                == PARTTYPE_ID_STRING_TO_INT("Arm")
                || u->getRootPart()->getType()->getSlotType(i)->getPartTypeId() 
                == PARTTYPE_ID_STRING_TO_INT("Leg")
                || u->getRootPart()->getType()->getSlotType(i)->getPartTypeId() 
                == PARTTYPE_ID_STRING_TO_INT("Reac")
                )
              {
                incompleteArmor = true;
                wasOk = false;
              }
            }
          }
        }
        if (wasOk) 
        {
          // at least one armor is complete and purchased!
          notAnyArmor = false;
        }
      }
    }
    if (noArmor || notPaid || incompleteArmor || notAnyArmor)
    {
      game->gameUI->openArmorIncompleteConfirm(player, notAnyArmor,
        incompleteArmor, noArmor, notPaid); 
    } else {
      // TODO: open loading window for players on this client in netgame...
      game->gameUI->openLoadingWindow(game->singlePlayerNumber); 
      hide();
    }
	}


  void CommandWindow::CursorEvent(OguiButtonEvent *eve)
  {
    if (eve->triggerButton->GetId() == COMMANDW_MAINMENU)
    {
      // TODO
      (Logger::getInstance())->debug("MAIN MENU TODO, PLEASE DO ME! :)");
    }
    if (eve->triggerButton->GetId() == COMMANDW_STORAGE)
    {
      // TODO
      hide();
      game->gameUI->openStorageWindow(player);
    }
    if (eve->triggerButton->GetId() == COMMANDW_QUIT)
    {
      quitPressed = true;
    }
    if (eve->triggerButton->GetId() == COMMANDW_COMBAT)
    {
      // TODO: proper game request!

      // TODO: won't work properly if for example one complete armor, but 
      // it has an unpurchased weapon or something...

			startMission();
    }
    if (eve->triggerButton->GetId() == COMMANDW_ARMOR)
    {
      // TODO: NETGAME, replace this with proper request!
      // (...not needed maybe - does not affect game state?)

      hide();
      game->gameUI->openArmorConstructWindow(player);
    }
  }

}



#endif
