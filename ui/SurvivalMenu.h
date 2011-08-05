#ifndef INC_SURVIVALMENU_H
#define INC_SURVIVALMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"

#include "../ogui/OguiCheckBox.h"

#include <vector>

class IOguiImage;
class OguiFormattedText;
class OguiButton;

namespace game
{
	class Game;
}

namespace ui
{

class SurvivalMenu : public MenuBaseImpl, public IOguiCheckBoxListener
{
public:

	enum COMMANDS {
		COMMANDS_LOAD = 101,
		COMMANDS_CLOSEME,
		COMMANDS_ARROWDOWN,
		COMMANDS_ARROWUP
	};

	SurvivalMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* game );
	~SurvivalMenu();

	//.........................................................................

	int getType() const;

	void closeMenu();
	void openMenu( int m );
	void applyChanges();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );
	void checkBoxEvent( OguiCheckBoxEvent* eve );
	void selectButton( int command );

	//.........................................................................

	void menuClose();
	void menuLoad();

	static bool startAsCoop;


	struct MissionInfo
	{
		std::string dir;
		std::string characters;
		std::string description;
		std::vector<int> scoreNumbers;
		std::string scores;
		std::string thumbnail_norm, thumbnail_high, thumbnail_down,
								thumbnail_selected_norm, thumbnail_selected_high;
		bool show_upgradewindow;
		bool locked;
	};

	static void reloadLastMission(game::Game *game);
	static void loadMission(MissionInfo &mi, game::Game *game);

	static bool loadMissionInfo(const std::string &directory, MissionInfo &mi);

	static void unlockMission(const std::string &mission);
	static bool readLockedMissions(std::vector<std::string> &missions);

private:
	void addImageSelectionButton( const std::string& image_norm, const std::string& image_high, const std::string& image_down, const std::string& image_disabled, bool disabled, int command, IOguiFont* font, void* param = NULL );
	void createMissionButtons();
	void createTexts();

	void scrollMissionsUp();
	void scrollMissionsDown();
	void updateArrows();
	OguiButton*			scrollUpArrow;
	OguiButton*			scrollDownArrow;
	OguiCheckBox*			showCustomMissions;

	int						doubleClickHack;
	int						doubleClickTimeHack;

	int						buttonXStart;
	int						buttonXLimit;
	int						scrollCount;
	int						scrollPosition;
	int						buttonYMinLimit; // = 64;
	int						buttonYMaxLimit; //  = 450;

	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;

	IOguiFont *locked_font;

	OguiFormattedText*	infoText;
	OguiFormattedText*	scoreText;

	std::vector<OguiButton *> starButtons;

	std::vector<MissionInfo> missionInfos;

	static MissionInfo lastLoadedMission;

};

}

#endif
