#ifndef INC_SURVIVORLOADGAMEMENU_H
#define INC_SURVIVORLOADGAMEMENU_H

#include "MenuBaseImpl.h"
#include "MenuCollection.h"

#include <vector>

class IOguiImage;
class OguiCheckBox;
class OguiTextLabel;
class OguiFormattedText;
class OguiLocaleWrapper;

namespace game
{
	class Game;
}

namespace ui
{

class SurvivorLoadGameMenu : public MenuBaseImpl
{
public:
	enum COMMANDS {
		COMMANDS_LOAD = 101,
		COMMANDS_NEXT,
		COMMANDS_PREV,
		COMMANDS_MISSION,
		COMMANDS_BONUSOPTIONS,
		COMMANDS_APPLYBONUSOPTIONS,
		COMMANDS_BONUSOPTION_BUTTONS // note: this must remain last
	};

	SurvivorLoadGameMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* game );
	~SurvivorLoadGameMenu();

	int getType() const;
	void update();
	void openMenu( int menu );
	void closeMenu();
	void applyChanges();

	void CursorEvent( OguiButtonEvent *eve );

private:
	void clipMissionButtons();
	void scrollMissions(int dir);
	void menuLoad();
	void menuBonusOptions();
	void menuCloseBonusOptions();

	void updateHighlightButtons();

	struct MissionInfo;
	bool loadMissionInfo(MissionInfo *mi, int savegame_id);

	MenuCollection* menuCollection;
	MenuCollection::Fonts* fonts;

	std::vector<OguiButton *> bonusOptionButtons;
	std::vector<OguiCheckBox *> bonusOptionBoxes;
	std::list<OguiTextLabel *> bonusOptionTexts;
	OguiWindow *bonusOptionWindow;

	int doubleClickHack;
	int doubleClickTimeHack;

	int fadeLeftStart;
	int fadeRightStart;
	OguiButton::Vertex vertices[12];

	struct MissionInfo
	{
		OguiButton *button;
		std::string title;
		std::string desc;
		std::string gamestats;
		bool empty;

		bool highlighted;
		OguiButton *highlight_button;
	};
	MissionInfo missionInfos[4];
	int lastValidMission;
	MissionInfo *centerMission;



	// id of the rightmost mission
	int firstMission;

	int missionButtonStartPosX;
	int missionButtonStartPosY;
	int missionButtonOffsetX;

	int missionButtonScrollAmount;

	int missionButtonClipLeft;
	int missionButtonClipRight;

	int missionButtonSwapRight;
	int missionButtonSwapLeft;

	OguiButton *nextButton;
	OguiButton *prevButton;

	OguiLocaleWrapper *loader;
	OguiButton *missionTitleText;
	OguiFormattedText *missionText;
	OguiButton *infoTitle;
	OguiFormattedText *infoText;
	std::string originalInfoText;

	bool holdingScroll;
	int startedHoldingScroll;

	OguiFormattedText *profileText;

	struct Thumbnail
	{
		IOguiImage *normal;
	};
	std::vector<Thumbnail> thumbnails;
};

}

#endif
