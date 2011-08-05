
#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include "../ogui/IOguiButtonListener.h"
#include "../ogui/IOguiEffectListener.h"

class Ogui;
class OguiWindow;
class OguiTextLabel;
class OguiFormattedText;
class IOguiFont;
class IOguiImage;
class IStorm3D_VideoStreamer;

namespace game
{
	class Game;
}

namespace ui
{
	class CharacterSelectionWindow;

	class LoadingWindow 
		: private IOguiButtonListener, private IOguiEffectListener
	{
	public:
		LoadingWindow(Ogui *ogui, game::Game *game, int player);
		~LoadingWindow();

		virtual void CursorEvent(OguiButtonEvent *eve);

		virtual void EffectEvent(OguiEffectEvent *eve);

		void reloadWindows();

		void enableClose();
		bool isCloseEnabled();

		void raise();

		bool isFadingOut();

		void closeWindow();
		void update();

		bool shouldAutoClose();

	private:
		void startScrolling();

		void createWindows();
		void destroyWindows();

		void createUpgradeButton();

		Ogui		*ogui;
		OguiWindow	*win;
		game::Game	*game;

		int	player;

		bool fadingOut;

		bool closeEnabled;
		int closeEnabledTime;

		OguiFormattedText* briefingArea;
		// OguiTextLabel *briefingArea;
		OguiButton *closebut;
		OguiButton *loadingbut;
		OguiButton* upgradeMenuBut;

		OguiTextLabel*	headerText;
		OguiTextLabel*	missionText;
	
		IOguiFont*	headerFont;
		IOguiFont*	missionFont;
		IOguiFont*	fontNormal;
		IOguiFont*	fontBold;
		IOguiFont*	fontItalic;
		IOguiFont*	fontUnderline;

		// start and close button font
		IOguiFont*	fontButton;
		IOguiFont*	fontButtonDisabled;
		IOguiFont*	fontButtonHighlighted;
		

		std::string mission_brief2_locale;
		OguiFormattedText *scrollingText;
		OguiWindow *scrollingFader;
		float scrollingSpeed;
		int lastScrollTime;
		float scrollTimeDeltaAvg;
		bool scrollingStarted;
		float nextScrollAmount;
		int totalScrollAmount;
		int speechSound;
		int speechSoundStartTime;
		bool playedMusic;
		int scrollingLimit;

		IOguiImage* briefVideo;
		IStorm3D_VideoStreamer*	briefVideoStream;
		bool videoStarted;

		std::string cinematic;
		bool cinematicStarted;
		bool cinematicFinished;

		int lastUpdateTime;
		bool updateStarted;


	  const char *scrolling_font_file;
		const char *scrolling_font_bold_file;
		const char *scrolling_font_italic_file;
		const char *scrolling_font_h1_file;


		IOguiImage* background_image;


	public:
		static bool showCharacterSelection;
		static bool showUpgradeWindowOnClose;
		static bool autoCloseEnabled;
	};

}

#endif
