#ifndef INC_LOADGAMEMENU_H
#define INC_LOADGAMEMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"

#include <vector>

class IOguiImage;
class OguiCheckBox;
class OguiTextLabel;

namespace game
{
	class Game;
}

namespace ui
{

extern int mission_max;

class LoadGameMenu : public MenuBaseImpl
{
public:
	class SurvivorInfoScreen;

	enum COMMANDS {
		COMMANDS_LEVEL1 = 1,
		COMMANDS_LEVEL2 = 2,
		COMMANDS_LEVEL3 = 3,
		COMMANDS_LEVEL4 = 4,
		COMMANDS_LEVEL5 = 5,
		COMMANDS_LEVEL6 = 6,
		COMMANDS_LEVEL7 = 7,
		COMMANDS_LEVEL8 = 8,
		COMMANDS_LEVEL9 = 9,
		COMMANDS_LEVEL10 = 10,
		COMMANDS_LOAD = 101,
		COMMANDS_CLOSEME,
		COMMANDS_ARROWDOWN,
		COMMANDS_ARROWUP,
		COMMANDS_BONUSOPTIONS,
		COMMANDS_APPLYBONUSOPTIONS,
		COMMANDS_BONUSOPTION_BUTTONS // note: this must remain last
	};

	LoadGameMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* game );
	~LoadGameMenu();

	//.........................................................................

	int getType() const;

	// bool wasQuitPressed() const;

	void closeMenu();
	void openMenu( int m );
	void applyChanges();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );
	void selectButton( int command );

	//.........................................................................

	void menuClose();
	void menuLoad();
	/*void menuResume();
	void menuContinue();
	void menuNewGame();
	void menuLoadGame();
	void menuProfiles();
	void menuOptions();
	void menuCredits();
	void menuQuit();*/

	
	void menuBonusOptions();
	void menuCloseBonusOptions();


	static bool startAsCoop;
	
private:

	void missionSelected( int m, int ex_selection );
	void addImageSelectionButton( const std::string& image_norm, const std::string& image_high, const std::string& image_down, const std::string& image_disabled, bool disabled, int command, IOguiFont* font, void* param = NULL );

	void scrollMissionsUp();
	void scrollMissionsDown();
	void updateArrows();

	std::vector< std::string > missionDescriptions;
	std::vector< std::string >	missionTimes;
	std::list< SelectionButtonDescs* > selectionButtonDescs;
	std::string				getMissionName( int i );
	//OguiButton*				loadButton;

	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;

	int						doubleClickHack;
	int						doubleClickTimeHack;
	
	bool					hideLoadGame;
	bool					fromGame;
	int						loadGameCnt;
	int						loadButtonCnt;

	int						buttonXStart;
	int						buttonXLimit;
	int						scrollCount;
	int						scrollPosition;
	int						buttonYMinLimit; // = 64;
	int						buttonYMaxLimit; //  = 450;

	SurvivorInfoScreen*		survivorInfoScreen;

	static	int				selectedSelection;

	std::vector<OguiButton *> bonusOptionButtons;
	std::vector<OguiCheckBox *> bonusOptionBoxes;
	std::list<OguiTextLabel *> bonusOptionTexts;
	OguiWindow *bonusOptionWindow;
};

}

#endif
