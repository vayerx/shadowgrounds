#ifndef INC_COOPMENU_H
#define INC_COOPMENU_H

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

class CoopMenu : public MenuBaseImpl, private IOguiSelectListListener
{
public:

	//.........................................................................
	
	enum COMMANDS {
		COMMANDS_STARTGAME = 1,
		COMMANDS_LOADGAME,
		COMMANDS_STARTSURVIVAL,

		COMMANDS_PLAYER1,
		COMMANDS_PLAYER2,
		COMMANDS_PLAYER3,
		COMMANDS_PLAYER4,

		COMMANDS_PLAYER1_OPTIONS,
		COMMANDS_PLAYER2_OPTIONS,
		COMMANDS_PLAYER3_OPTIONS,
		COMMANDS_PLAYER4_OPTIONS,

		COMMANDS_EASY,
		COMMANDS_NORMAL,
		COMMANDS_HARD,
		COMMANDS_VERY_HARD,
		COMMANDS_EXTREMELY_HARD
	};

	enum OPTIONS {
		OPTION_FRIENDLY_FIRE = 0,
		NUM_OPTIONS = 1,
	};

	//.........................................................................

	CoopMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* g );
	~CoopMenu();

	//.........................................................................

	static int convertToRunningNum( int i );
	static CoopMenu::COMMANDS convertToPlayerNum( int i );

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
	void menuLoadGame();
	void menuStartSurvival();

	void menuOptions(int player);

	static bool prepareCoopGameRestart(game::Game *game);
	static bool enableCoopGameSettings(game::Game *game, bool test_only = false);
	static void disableCoopGameSettings(game::Game *game);

private:

	void				addText( const std::string& text, int x, int y, int w, int h, IOguiFont* font );
	OguiButtonStyle*	loadStyle( const std::string& button_name );



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
	static std::string				none;

	game::GameProfiles*		gameProfiles;


	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;

	
	std::list< OguiButtonStyle* >	styles;
	OguiSelectListStyle*			selectListStyle;

	std::list< OguiTextLabel* >		textLabels;
	OguiButton* startGame;
	OguiButton* loadGame;
	OguiButton* startSurvival;

	std::vector< OguiButton * > optionsButtons;

	OguiTextLabel* optionsBigText;
	std::vector<OguiCheckBox *> options;
};

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#endif
