
#ifndef HEALTHWINDOW_H
#define HEALTHWINDOW_H

#include "../ogui/Ogui.h"
#include "ICombatSubWindow.h"

namespace game
{
	class Game;
}

namespace ui
{

	/**
	 *
	 * Player health on-screen status window.
	 *
	 * @version 1.0, 23.12.2004
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see CombatWindow
	 *
	 */

	class HealthWindow : public ICombatSubWindow, private IOguiEffectListener
	{
	public:
		HealthWindow();
		HealthWindow(Ogui *ogui, game::Game *game, int clientNum, bool cooperative = false );
		virtual ~HealthWindow();

			//void moveTo(int x, int y);

		virtual void hide(int fadeTime = 0);
		virtual void show(int fadeTime = 0);
		virtual void update();
		virtual void updateAnimation() { updateCurve(); }
		virtual void updateCurve();
		virtual void EffectEvent(OguiEffectEvent *e);

		// added by Pete for survivors, shouldn't break shadowgrounds
		// virtual void setHealthTextMultiplier( int player_num, float m );
	private:
		Ogui *ogui;
		game::Game *game;
		int clientNum;
		OguiWindow *win;

		IOguiFont*	font;

		IOguiImage *fillupImage;
		IOguiImage *fillupLowImage;
		OguiButton *fillupButton;
		

		IOguiImage *healthBackgroundImage;
		IOguiImage *healthBackgroundLowImage;
		OguiButton *healthBackgroundButton;

		IOguiImage *curveImage;
		IOguiImage *curveLife;
		IOguiImage *curveDead;
		OguiButton *curveButton;
		int curveFrames;

		OguiTextLabel *healthAmountText;

		int lastUpdateValue;
		int lastUpdateValue2;
		int healthDropBlinkCounter;
		
		// added by Pete
		float healthTextMultiplier;
	};

}


#endif
