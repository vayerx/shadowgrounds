#ifndef INC_CHARACTERSELECTIONWINDOW_H
#define INC_CHARACTERSELECTIONWINDOW_H

#include "../ogui/IOguiButtonListener.h"
#include "../game/gamedefs.h"

class Ogui;
class OguiLocaleWrapper;
class OguiTextLabel;
class IOguiImage;
class IStorm3D_VideoStreamer;


namespace game {
	class Game;
	class Unit;
}

namespace ui {

class CharacterSelectionWindow 
	: private IOguiButtonListener
{
public:
	CharacterSelectionWindow( Ogui *ogui, game::Game *game);
	~CharacterSelectionWindow();

	virtual void CursorEvent(OguiButtonEvent *eve);

	void raise();
	void closeWindow();

	void update();


	static void parseChoices(const char *params);

	void chooseCharacterInDir(int dir, int player);
	void lockChosenCharacter(int player);
	
	void updateProfileButtons();
	bool canStart();

	void forceStart();

	bool shouldClose();

private:
	static const char *characterNames[3];
	static bool characterEnabled[3];
	static int characterForced;

	void updateCharacterButtonChosen(int charNum);

	Ogui		*ogui;
	OguiWindow	*win;
	game::Game	*game;

	OguiButton*	headerText;
	std::vector<OguiButton *> characterImages;
	std::vector<OguiButton *> characterButtons;
	std::vector<IOguiImage *> characterButtonImages;
	std::vector<OguiButton *> profileButtons;
	int profileButtonOffset;
	OguiButton* mustChooseError;

	std::vector<IOguiImage *> characterVideos;
	std::vector<IStorm3D_VideoStreamer *>	characterVideoStreams;

	OguiLocaleWrapper *oguiLoader;

	int chosenCharacter[MAX_PLAYERS_PER_CLIENT];
	bool chosenCharacterLocked[MAX_PLAYERS_PER_CLIENT];

	int profileDisplayOrder[MAX_PLAYERS_PER_CLIENT];
	std::string lockSound, chooseSound, errorSound;

	int readyToCloseTimer;
	unsigned int frame;

	std::vector<unsigned int> switchToVideoInFrame;
}; 

} // end of namespace ui

#endif
