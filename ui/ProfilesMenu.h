#ifndef INC_PROFILESMENU_H
#define INC_PROFILESMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"

#include <vector>

class IOguiImage;
class OguiFormattedText;

namespace game
{
	class Game;
	class GameProfiles;
}

namespace ui
{

class ProfilesMenu : public MenuBaseImpl
{
public:
	enum COMMANDS {
		COMMANDS_PROFILE1 = 1,
		COMMANDS_PROFILE2 = 2,
		COMMANDS_PROFILE3 = 3,
		COMMANDS_PROFILE4 = 4,
		COMMANDS_PROFILE5 = 5,
		COMMANDS_PROFILE6 = 6,
		COMMANDS_PROFILE7 = 7,
		COMMANDS_PROFILE8 = 8,
		COMMANDS_PROFILE9 = 9,
		COMMANDS_PROFILE10 = 10,
		COMMANDS_SELECT = 101,
		COMMANDS_NEW,
		COMMANDS_DELETE,
		COMMANDS_CLOSEME
	};

	ProfilesMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* game );
	~ProfilesMenu();

	//.........................................................................

	int getType() const;

	// bool wasQuitPressed() const;

	void closeMenu();
	void openMenu( int m );
	void applyChanges();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );

	//.........................................................................

	void menuClose();
	void menuSelect();
	void menuNew();
	void menuDelete( bool delete_from_profiles = true );
	/*void menuResume();
	void menuContinue();
	void menuNewGame();
	void menuLoadGame();
	void menuProfiles();
	void menuOptions();
	void menuCredits();
	void menuQuit();*/

	void readKey( char ascii, int keycode, const char *keycodeName );
	void handleEsc();
	void escPressed();

private:



	// Call this after a update the profile structure
	void createProfileButtons();

	// void editButton( OguiButton* b, const std::string& temp );
	void editButtonEnter( const std::string& string );
	bool editButtonEnterCheck( const std::string& string );

	void createProfileInfoText();
	void updateProfileInfo(int profile);

	void setProfile(int profile, bool safetyChecks = true);

	game::GameProfiles*		gameProfiles;

	std::string				getProfileName( int i );

	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;

	OguiButton*				textEditButton;

	int						profilesMax;
	int						profilesCnt;
	int						profilesCurrent;

	int						doubleClickHack;
	int						doubleClickTimeHack;

	std::vector< std::string >	profileData;
	std::vector< std::string >	profileInfoCache;
	
	bool					fromGame;
	int						loadGameCnt;
	int						loadButtonCnt;

	std::string				profileEmpty;
	std::string				profilePlayer;

	OguiFormattedText *profileInfoText;
	std::string profileInfoString;


};

} // end of namespace

#endif
