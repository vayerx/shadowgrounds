
#ifndef COMBATMESSAGEWINDOW_H
#define COMBATMESSAGEWINDOW_H

#include "../ogui/Ogui.h"
#include <vector>

namespace game
{
	class Game;
}

namespace ui
{

	class Visual2D;
	class GUIEffectWindow;

	/**
	 *
	 * Combat message window class. 
	 * Shows the on-screen messages while in combat (= in mission).
	 *
	 * @version 0.9, 30.9.2002
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see CombatWindow
	 *
	 */

	class CombatMessageWindow : public IOguiEffectListener
	{
	public:

		CombatMessageWindow(Ogui *ogui, game::Game *game, int player, 
			const char *textConfName, const char *iconConfName);
		virtual ~CombatMessageWindow();

		virtual void showMessage(const char *message, Visual2D *image);
		virtual void clearMessage();

		void setNoiseAlpha(float alpha);
		void clearMessageTextOnly();

		virtual void hide( int fadeTime = 0 );
		virtual void show( int fadeTime = 0 );
		virtual void EffectEvent( OguiEffectEvent *e );

		/*
		void moveTo(int x, int y);
		*/
		void setFont(IOguiFont *font, bool centered);
		void setBoxed(bool textBoxed);

		virtual void raise();
		virtual void update(int ms);

		inline bool hasMessage() const { return messageText != NULL; }

	protected:
		Ogui *ogui;
		game::Game *game;
		int player;
		OguiWindow *win;

		int textXPosition;
		int textYPosition;
		int textXSize;
		int textYSize;

		int iconXPosition;
		int iconYPosition;
		int iconXSize;
		int iconYSize;

		int iconBoxXPosition;
		int iconBoxYPosition;
		int iconBoxXSize;
		int iconBoxYSize;

		int boxXPosition;
		int boxYPosition;
		int boxXSize;
		int boxYSize;
		
		IOguiFont *textFont;
		bool textCentered;
		bool textBoxed;

		char *textConfName;
		char *iconConfName;

		OguiTextLabel *messageText;
		OguiButton *messageImageButton;
		//IOguiImage *messageImage;
		IOguiImage *boxImage;
		OguiButton *messageBoxButton;

		OguiButton *messageImageBGButton;
		IOguiImage *iconBGImage;

		GUIEffectWindow *effectWindow;
		int time;
		float noiseAlpha;
		std::vector<float> noiseArray;
		int noiseTime;
	};

}


#endif
