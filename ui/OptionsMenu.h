#ifndef INC_OPTIONSMENU_H
#define INC_OPTIONSMENU_H

#include <vector>

#include "MenuBaseImpl.h"
#include "MenuCollection.h"
#include "../ogui/IOguiSliderListener.h"
#include "../ogui/IOguiSelectListListener.h"
#include "../ogui/OguiCheckBox.h"
#include "GameController.h"

class IOguiImage;
class OguiWindow;
class OguiCheckBox;
class OguiSelectList;
class OguiSelectListStyle;

extern bool apply_options_request;

namespace game
{
	class Game;
	
}

namespace ui
{

// class GameController;

class OptionsMenu : public MenuBaseImpl, 
					public IOguiSliderListener, 
					public IOguiSelectListListener, 
					public IOguiCheckBoxListener
{
public:
	enum COMMANDS {
		COMMANDS_BUTTON_1 = 1,
		COMMANDS_BUTTON_15 = 15,
		COMMANDS_BUTTON_16 = 16,
		COMMANDS_BUTTON_17 = 17,
		COMMANDS_BUTTON_18 = 18,
		COMMANDS_BUTTON_19 = 10,
		COMMANDS_BUTTON_20 = 20,
		
		COMMANDS_JOYSTICK_MOVE_XAXIS,
		COMMANDS_JOYSTICK_MOVE_YAXIS,
		COMMANDS_JOYSTICK_DIR_XAXIS,
		COMMANDS_JOYSTICK_DIR_YAXIS,
		
		COMMANDS_EASY = 50,
		COMMANDS_NORMAL = 51,
		COMMANDS_HARD = 52,
		
		COMMANDS_SLIDERMUSIC = 30,
		COMMANDS_SLIDERSOUND = 31,
		COMMANDS_SLIDERSPEECH = 32,
		COMMANDS_SLIDERAMBIENT = 33,

		COMMANDS_SLIDERROTATESPEED = 34,
		COMMANDS_SLIDERMOUSESPRING = 35,
		COMMANDS_SLIDERGAMMA = 36,

		COMMANDS_CONTROLLERTYPE = 37,

		COMMANDS_CLOSEME = 101,
		COMMANDS_DEFAULTS,
		
	};

	OptionsMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, game::Game* game );
	~OptionsMenu();

	//.........................................................................

	int getType() const;

	// bool wasQuitPressed() const;
	
	void closeMenu();
	void openMenu( int m );
	void applyChanges();
	void handleEsc();

	//.........................................................................

	void CursorEvent( OguiButtonEvent* eve );
	void sliderEvent( OguiSliderEvent* eve );
	void SelectEvent( OguiSelectListEvent *eve );
	void checkBoxEvent( OguiCheckBoxEvent* eve );

	void update();

	//.........................................................................

	void menuClose();
	void menuDefaults();
	/*void menuResume();
	void menuContinue();
	void menuNewGame();
	void menuLoadGame();
	void menuProfiles();
	void menuOptions();
	void menuCredits();
	void menuQuit();*/

	void setProfile( const std::string& profile );

public:

	static int cooperativeSelection;
	static bool returnToCoopMenu;

private:

	//.........................................................................

	void selectButton( int i );

	void addDifficultButton( int x, int y, int w, int h, 
		const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, const std::string& text, int id );

	void addText( const std::string& text, int x, int y, int w, int h, IOguiFont* font );

	void selectDifficultButton( int id );

	void setDifficulty( int difficulty );
	void addControlDescription(  const std::string& text, int id, int x_add, int y_add, IOguiFont* font, IOguiFont* high = NULL, IOguiFont* down = NULL, IOguiFont* disa = NULL );
	void updateControlDescriptions();
	void readControls();

	std::string				getOptionsButtonName( int i );
	std::string				getOptionsKeyName( int i );

	//.........................................................................

	void createJoystickButtons();
	void setJoystickAxis( int axis, GameController::JOYSTICK_AXIS button );
	void joystickSelection( int i );
	void freeJoystickButtons();
	void determineCurrentController();

	//.........................................................................

	void createMouseButtons();
	void freeMouseButtons();

	//.........................................................................

	OguiButtonStyle*	loadStyle( const std::string& button_name );
	void				openCoopProfileMenu();
	void				closeCoopProfileMenu();

	//.........................................................................

	void setControllerType(int i);
	void updateControllerTypeText();

	

	std::vector< int >				controlNumArray;
	std::vector< int >				keycodeArray;
	std::vector< OguiButton*  >		controlDescriptions;
	std::list< SelectionButtonDescs* >	selectionButtonDescs;
	OguiWindow*						mouseEventCaptureWindow;

	int								controlUpdate;

	std::list< OguiSlider* >		sliderButtons;
	std::list< OguiTextLabel* >		textLabels;

	GameController*					gameController;

	std::map< int, OguiButton* >	difficultButtons;
	int								difficultActiveSelection;
	IOguiImage*						difficultImageSelectDown;
	IOguiImage*						difficultImageSelectNorm;

	float							sliderMusicValue;
    float						sliderSoundValue;
	float							sliderSpeechValue;
	float							sliderAmbientValue;
	bool							ambientOptionAvailable;

	std::string						sliderSoundPlayfile;
	int								sliderSoundLoopTime;
	bool							sliderSoundPlayNow;

	std::string						sliderSpeechPlayfile;
	int								sliderSpeechLoopTime;
	bool							sliderSpeechPlayNow;

	int								lastPlayTime;

	//.........................................................................

	OguiTextLabel*			joystickBigText;
	bool					joystickMenuOpen;
	int						joystickUpdate;
	int						currentController;
	
	//.........................................................................

	
	OguiTextLabel*					cooperativeBigText;
	OguiSelectList*					cooperativeProfileList;
	std::list< OguiButtonStyle* >	styles;
	OguiSelectListStyle*			selectListStyle;
	

	//.........................................................................

	MenuCollection*			menuCollection;
	MenuCollection::Fonts*	fonts;
	
	bool					discartNextCursorEvent;
	bool					fromGame;

	//.........................................................................

	// Only for demo use
	OguiCheckBox*			cameraModeHack;
	OguiCheckBox*			cameraLockYAxis;
	OguiTextLabel*			cameraModeTextHack;
	OguiSlider*				cameraRotateStrength;
	OguiSlider*				cameraSpringStrength;
	float					cameraRotateSpeed;
    float					mouseSpring;

	OguiButton *controllerTypeButton;
	OguiButton *controllerTypeListCaptureEvents;
	OguiSelectList *controllerTypeList;
	OguiSelectListStyle *controllerTypeListStyle;
	void openControllerTypeList();
	void closeControllerTypeList();
	//.........................................................................

	std::string				currentProfile;

};

} // end of namespace

#endif
