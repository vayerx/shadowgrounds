#ifndef INC_MAINMENU_H
#define INC_MAINMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"

class Ogui;
class OguiFormattedText;
class OguiButton;

namespace game
{
	class Game;
}

namespace ui
{

// The main menu the single menu from in the begin of the game, this does 
// not include options / load game / profiles / credits or any other sub window!
class MainMenu : public MenuBaseImpl
{
public:
	enum COMMANDS
	{
		COMMANDS_RESUME = 0,
		COMMANDS_CONTINUE,
		COMMANDS_NEW_GAME,
		COMMANDS_LOAD_GAME,
		COMMANDS_SURVIVAL,
		COMMANDS_COOP,
		COMMANDS_PROFILES,
		COMMANDS_OPTIONS,
		COMMANDS_CREDITS,
		COMMANDS_QUIT,
		COMMANDS_YES,
		COMMANDS_NO,
		COMMANDS_EASY,
		COMMANDS_NORMAL,
		COMMANDS_HARD
	};

	MainMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* game, bool from_game = false );
	~MainMenu();

	//.........................................................................

	int getType() const;

	// bool wasQuitPressed() const;

	void checkContinue();
	void closeMenu();
	void openMenu( int m );
	void applyChanges();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );
	void handleEsc();
	//.........................................................................

	void menuResume();
	void menuContinue();
	void menuNewGame();
	void menuLoadGame();
	void menuSurvival();
	void menuCoop();
	void menuProfiles();
	void menuOptions();
	void menuCredits();
	void menuQuit();

	//.........................................................................

	void hide();
	void show();
	void raise();

	void update();


private:

	void abortCurrentGame();
	void closeAbortMenu();

	OguiButton* addButton( const std::string& text, int command, IOguiFont* font = NULL, IOguiFont* high = NULL, IOguiFont* down = NULL, IOguiFont* disa = NULL, OguiButton::TEXT_H_ALIGN halign = OguiButton::TEXT_H_ALIGN_CENTER );
	void createDifficultyButtons();
	void setDifficulty( int difficulty );
	void selectDifficultButton( int i );
	void addDifficultButton( int x, int y, int w, int h, 
		const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, const std::string& text, int command );

	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;
	// game::Game*				game;

	// for startting a new game if fromGame
	OguiFormattedText*		abortGame;
	OguiButton*				abortGameYes;
	OguiButton*				abortGameNo;
	OguiWindow*				captureAllEvents;
	OguiButton*				continueButton;

	// haxoring
	OguiButton*				currentActive;
	int						numberOfButtons;
	OguiWindow*				win2;

	// difficulty hack stuff
	std::map< int, OguiButton* >	difficultButtons;
	int								difficultActiveSelection;
	IOguiImage*						difficultImageSelectDown;
	IOguiImage*						difficultImageSelectNorm;
	
	bool					fromGame;
	bool firstUpdate;

	mutable bool			quitPressed;
	std::string				playerName;
};

}

#endif
