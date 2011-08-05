#ifndef INC_NEWGAMEMENU_H
#define INC_NEWGAMEMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"
#include "../ogui/IOguiSliderListener.h"
#include "../ogui/IOguiSelectListListener.h"

#include <vector>
#include <list>
#include <map>
#include <string>

class IOguiImage;
class OguiWindow;
class OguiCheckBox;
class OguiSelectList;
class OguiSelectListStyle;

namespace game
{
	class Game;
	class GameProfiles;
}


namespace ui {

///////////////////////////////////////////////////////////////////////////////

void setSinglePlayer( const game::Game* game );

class NewGameMenu : public MenuBaseImpl, private IOguiSelectListListener
{
public:

	//.........................................................................
	
	enum COMMANDS {
		COMMANDS_STARTGAME = 1,

		COMMANDS_SINGLEPLAYER,
		COMMANDS_MULTIPLAYER,

		COMMANDS_PLAYER1,
		COMMANDS_PLAYER2,
		COMMANDS_PLAYER3,
		COMMANDS_PLAYER4,

		COMMANDS_EASY,
		COMMANDS_NORMAL,
		COMMANDS_HARD,
		COMMANDS_VERY_HARD,
		COMMANDS_EXTREMELY_HARD,

		COMMANDS_BONUSOPTION_BUTTONS // note: this must remain last
	};

	//.........................................................................

	NewGameMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* g );
	~NewGameMenu();

	//.........................................................................

	static int convertToRunningNum( int i );
	static NewGameMenu::COMMANDS convertToPlayerNum( int i );

	int getType() const;

	// bool wasQuitPressed() const;

	void closeMenu();
	void openMenu( int m );
	void applyChanges();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );
	void SelectEvent( OguiSelectListEvent *eve );

	//.........................................................................
	
	void menuStartGame();
	
	static void createBonusOptions(	game::Game *game, OguiWindow *win, Ogui *ogui, MenuCollection::Fonts *fonts,
																	std::vector<OguiCheckBox *> &boxes, std::vector<OguiButton *> &buttons, std::list<OguiTextLabel *> &texts,
																	IOguiButtonListener *button_listener, int button_id_offset,
																	int offset_x = 0, int offset_y = 0);
	static void applyBonusOptions(game::Game *game, std::vector<OguiCheckBox *> &boxes);

private:

	void				addText( const std::string& text, int x, int y, int w, int h, IOguiFont* font );
	OguiButtonStyle*	loadStyle( const std::string& button_name );

	// Difficulty buttons
	void createDifficultyButtons();
	void setDifficulty( int difficulty );
	void selectDifficultButton( int i );
	void addDifficultButton( int x, int y, int w, int h, 
		const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, IOguiFont* font_high, IOguiFont* font_down, IOguiFont* font_disa, const std::string& text, int command );
	void showDifficultyToolTip( int difficulty );
	void hideDifficultyToolTip( void );

	// difficulty hack stuff
	std::map< int, OguiButton* >	difficultButtons;
	int								difficultActiveSelection;
	IOguiImage*						difficultImageSelectDown;
	IOguiImage*						difficultImageSelectNorm;
	OguiTextLabel*					difficultToolTipText;
	IOguiFont*								difficultToolTipFont;
	std::map<int, std::string> difficultToolTips;

	OguiCheckBox *tutorialHintsButton;

	// bonus options
	std::vector<OguiCheckBox *> bonusOptionBoxes;
	std::vector<OguiButton *> bonusOptionButtons;

	
	// Gamemode buttons
	void createGameModeButtons();
	void setGameMode( int gamemode );
	void selectGameModeButton( int i );
	void addGameModeButton( int x, int y, int w, int h, const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, IOguiFont* font_high, IOguiFont* font_down, IOguiFont* font_disa, const std::string& text, int command );

	// game mode
	std::map< int, OguiButton* >	gameModeButtons;
	int								gameModeActiveSelection;


	void		createCooperativeMenu();
	void		freeCooperativeMenu();
	std::string getCoopPlayerName( int i );
	void		openCoopProfileMenu( int i );
	void		closeCoopProfileMenu();
	void		setCoopPlayer( int i, const std::string& name );

	OguiTextLabel*					coopBigText;
	OguiSelectList*					coopProfileList;
	OguiButton*						coopProfileListSaver;
	OguiButton*						coopCaptureEvents;
	int								coopCurrentSelection;
	static std::map< int, std::string >	coopPlayerNames;
	
	
	std::string				none;
	game::GameProfiles*		gameProfiles;


	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;

	
	std::list< OguiButtonStyle* >	styles;
	OguiSelectListStyle*			selectListStyle;

	std::list< OguiTextLabel* >		textLabels;
	OguiButton*						startGame;
	

public:
	static int						selectedDifficultSelection;
	static int						selectedGameplaySelection;

};

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#endif
