
#ifndef ANIRECORDERWINDOW_H
#define ANIRECORDERWINDOW_H

#include "../ogui/IOguiButtonListener.h"
#include "../ogui/IOguiSelectListListener.h"
#include "IMessageBoxListener.h"
#include <string>

class Ogui;
class IOguiImage;
class IOguiFont;
struct OguiButtonStyle;
class OguiWindow;
class OguiTextLabel;
class OguiSelectList;
class OguiSelectListStyle;

namespace game
{
  class Game;
	class AniRecorder;
	class Unit;
}

namespace ui
{
	class MessageBoxWindow;

  class AniRecorderWindow : private IOguiButtonListener,
		private IOguiSelectListListener,
		private IMessageBoxListener
  {
  public:
    AniRecorderWindow(Ogui *ogui, game::Game *game);
    ~AniRecorderWindow();

    virtual void CursorEvent(OguiButtonEvent *eve);

		virtual void SelectEvent(OguiSelectListEvent *eve);

		void updateLists();

		void updateButtons();

		void updateUnitSelections();

    void run();

		void reload();

		void setMinimizedWindowMode(bool minimize);

		void messageBoxClosed(MessageBoxWindow *msgbox, int id, int choice);

		void addAniScriptCommands(const char *scriptCommands);

		void setStatusText(const char *status);
		void sliderStatus();
		std::string getSliderPosOrRangeText();

  private:
		void clearLists();

		void createMiniDependantButtons();

    Ogui *ogui;
    OguiWindow *win;
    game::Game *game;

		IOguiFont *font;
		IOguiFont *smallFont;

    OguiTextLabel *label1;
    OguiButton *closebut;
    OguiButton *minibut;

    OguiButton *cameraModeBut;
    OguiButton *cameraDumpBut;
    OguiButton *cameraTestBut;
    OguiButton *cameraInterpBut;
    OguiButton *cameraDelBut;

    OguiButton *addUnitBut;
    OguiButton *removeUnitBut;
    OguiButton *recBut;
    OguiButton *playBut;
    OguiButton *pauseBut;
    OguiButton *rewindBut;

    OguiButton *positionBut;
    OguiButton *positionEndBut;
    OguiButton *sliderBut;

    OguiButton *recordPathBut;
    OguiButton *reloadBut;

    OguiButton *addAnimBut;
    OguiButton *addFewTicksBut;
    OguiButton *addManyTicksBut;
    OguiButton *addCommandsBut;
    OguiButton *smoothPositionBut;
    OguiButton *smoothRotationBut;
    OguiButton *smoothAimBut;
    OguiButton *undoBut;
    OguiButton *redoBut;
    OguiButton *deletePositionBut;
    OguiButton *dropOnGroundBut;

    IOguiImage *unselImage;
    IOguiImage *selImage;
    IOguiImage *selDownImage;

    IOguiImage *scrollUpImage;
    IOguiImage *scrollUpPressedImage;
    IOguiImage *scrollUpDisabledImage;
    IOguiImage *scrollDownImage;
    IOguiImage *scrollDownPressedImage;
    IOguiImage *scrollDownDisabledImage;
    OguiButtonStyle *scrollUpStyle;
    OguiButtonStyle *scrollDownStyle;

    OguiButtonStyle *unselStyle;
    OguiButtonStyle *selStyle;
    OguiButtonStyle *numUnselStyle;

    OguiSelectListStyle *listStyle;

    OguiButtonStyle *smallUnselStyle;
    OguiButtonStyle *smallSelStyle;
    OguiSelectListStyle *smallListStyle;

    OguiButtonStyle *tinyUnselStyle;
    OguiButtonStyle *tinySelStyle;
    OguiSelectListStyle *tinyListStyle;

    OguiTextLabel *timeLabel;
    OguiTextLabel *statusLabel;

    OguiSelectList *cameraSelectList;
    OguiSelectList *unitSelectList;
    OguiSelectList *animationSelectList;
    OguiSelectList *tickSelectList;

		game::AniRecorder *aniRecorder;

		game::Unit *selectedUnit;
		int selectedCamera;

		bool sliderDown;
		bool endSliderDown;

		bool assumingPlayingOrRecording;

		bool minimized;
		int sliderHeight;

		int recCounter;

		char *currentSelectedAnimation;
		int currentSelectedTicks;

		MessageBoxWindow *cameraDelBox;
  };

}

#endif
