
crap;

#ifndef COMMANDWINDOW_H
#define COMMANDWINDOW_H

#include "../ogui/Ogui.h"
#include "../game/Game.h"

#include "MenuCollection.h"

namespace ui
{
	class OptionsWindow;


// #define CommandWindow M;
  /**
   * Command brigde window class. 
   * Presents the command brigde selections of player's mothership.
   *
   * @version 0.5, 25.6.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   */


/*
  class CommandWindow : public IOguiButtonListener
  {
  public:
    CommandWindow(Ogui *ogui, game::Game *game, int player);
    ~CommandWindow();
    
    void hide();
    void show();
    bool isVisible();

    bool wasQuitPressed();

	void startMission();

    virtual void CursorEvent(OguiButtonEvent *eve);

  private:

	IOguiFont* font;

    Ogui *ogui;
    game::Game *game;
    int player;
    OguiWindow *win;
    LinkedList *buttons;
    bool quitPressed;
		OguiButton *startButton;
		OptionsWindow *optionsWindow;
	
	// Test 
	OguiWindow* foreground;
  };*/

}

#endif

