
#include "precompiled.h"

#include "HealthWindow.h"

#include <assert.h>
#include "Visual2D.h"
#include "uidefaults.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../convert/str2int.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"
#include "../ogui/OguiAligner.h"

#include "../util/Debug_MemoryManager.h"

#include "CombatSubWindowFactory.h"

#include <boost/lexical_cast.hpp>

using namespace game;

namespace ui
{
///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( HealthWindow );
///////////////////////////////////////////////////////////////////////////////

	HealthWindow::HealthWindow() :
		ogui( NULL ),
		game( NULL ),
		clientNum( 0 ),
		win( NULL ),
		font( NULL ),

		fillupImage( NULL ),
		fillupLowImage( NULL ),
		fillupButton( NULL ),

		healthBackgroundImage( NULL ),
		healthBackgroundLowImage( NULL ),
		healthBackgroundButton( NULL ),

		curveImage( NULL ),
		curveLife( NULL ),
		curveDead( NULL ),
		curveButton( NULL ),
		curveFrames( 0 ),

		healthAmountText( NULL ),

		lastUpdateValue( 0 ),
		lastUpdateValue2( 0 ),
		healthDropBlinkCounter( 0 ),
		healthTextMultiplier( 0.5f )
	{
	}

	HealthWindow::HealthWindow(Ogui *ogui, game::Game *game, int clientNum, bool coop ) :
		ogui( NULL ),
		game( NULL ),
		clientNum( 0 ),
		win( NULL ),
		font( NULL ),

		fillupImage( NULL ),
		fillupLowImage( NULL ),
		fillupButton( NULL ),

		healthBackgroundImage( NULL ),
		healthBackgroundLowImage( NULL ),
		healthBackgroundButton( NULL ),

		curveImage( NULL ),
		curveLife( NULL ),
		curveDead( NULL ),
		curveButton( NULL ),
		curveFrames( 0 ),

		healthAmountText( NULL ),

		lastUpdateValue( 0 ),
		lastUpdateValue2( 0 ),
		healthDropBlinkCounter( 0 ),
		healthTextMultiplier( 0.5f )
	{
		this->ogui = ogui;
		this->game = game;
		this->clientNum = clientNum;

		std::string prefix = "gui_health_";

		if( coop )
		{
			prefix = "gui_health_coop" + boost::lexical_cast< std::string >( clientNum ) + "_";
		}

		int xPosition = getLocaleGuiInt( ( prefix + "position_x" ).c_str(), 0);
		int yPosition = getLocaleGuiInt( ( prefix + "position_y" ).c_str(), 0);


		this->win = ogui->CreateSimpleWindow(xPosition, yPosition, getLocaleGuiInt( (prefix + "size_x" ).c_str(), 0), getLocaleGuiInt( (prefix + "size_y" ).c_str(), 0), NULL);
		this->win->SetUnmovable();

		this->healthBackgroundImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix  + "image" ).c_str()));
		this->healthBackgroundLowImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix  + "low_image" ).c_str()));

		this->fillupImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix  + "fillup_image" ).c_str()));
		this->fillupLowImage = ogui->LoadOguiImage(getLocaleGuiString( (prefix  + "fillup_low_image" ).c_str()));

		this->curveLife = ogui->LoadOguiImage(getLocaleGuiString( (prefix  + "curve_image" ).c_str()));
		this->curveDead = ogui->LoadOguiImage(getLocaleGuiString( (prefix  + "curve_image_dead" ).c_str()));
		this->curveFrames = getLocaleGuiInt( (prefix  + "curve_frames" ).c_str(), 0);

		curveImage = curveLife;

		healthBackgroundButton = ogui->CreateSimpleImageButton(win, 0, 0, getLocaleGuiInt( (prefix  + "size_x" ).c_str(), 0), getLocaleGuiInt( (prefix  + "size_y" ).c_str(), 0), NULL, NULL, NULL);
		healthBackgroundButton->SetDisabled(true);
		healthBackgroundButton->SetDisabledImage(healthBackgroundImage);

		fillupButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt( (prefix  + "fillup_offset_x" ).c_str(), 0), getLocaleGuiInt( (prefix  + "fillup_offset_y" ).c_str(), 0), getLocaleGuiInt( (prefix  + "fillup_size_x" ).c_str(), 0), getLocaleGuiInt( (prefix  + "fillup_size_y" ).c_str(), 0), NULL, NULL, NULL);
		fillupButton->SetDisabled(true);
		fillupButton->SetDisabledImage(fillupImage);

		curveButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt( (prefix  + "curve_offset_x" ).c_str(), 0), getLocaleGuiInt( (prefix  + "curve_offset_y" ).c_str(), 0), getLocaleGuiInt( (prefix  + "curve_size_x" ).c_str(), 0), getLocaleGuiInt( (prefix  + "curve_size_y" ).c_str(), 0), NULL, NULL, NULL);
		curveButton->SetDisabled(true);
		curveButton->SetDisabledImage(curveImage);

		font = ogui->LoadFont( getLocaleGuiString( (prefix + "font" ).c_str() ) );

		healthAmountText = ogui->CreateTextLabel(win, 0, 0, getLocaleGuiInt( (prefix  + "size_x" ).c_str(), 0), getLocaleGuiInt( (prefix  + "size_y" ).c_str(), 0), "");
		healthAmountText->SetFont( font?font: ui::defaultIngameNumbersBoldFont );

		lastUpdateValue = -1;
		lastUpdateValue2 = -1;
		healthDropBlinkCounter = 0;
		win->SetEffectListener(this);

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(win, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
		OguiAligner::align(healthBackgroundButton, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
		OguiAligner::align(fillupButton, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
		OguiAligner::align(curveButton, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
		OguiAligner::align(healthAmountText, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
#endif
	}


	HealthWindow::~HealthWindow()
	{
		if (healthAmountText != NULL)
		{
			delete healthAmountText;
			healthAmountText = NULL;
		}
		if (healthBackgroundButton != NULL)
		{
			delete healthBackgroundButton;
			healthBackgroundButton = NULL;
		}
		if (fillupButton != NULL)
		{
			delete fillupButton;
			fillupButton = NULL;
		}
		if (curveButton != NULL)
		{
			delete curveButton;
			curveButton = NULL;
		}
		if (win != NULL)
		{
			delete win;
			win = NULL;
		}

		delete healthBackgroundImage;

		delete curveLife;
		delete curveDead;
		// delete curveImage;
		delete fillupImage;

		delete font;
	}


	void HealthWindow::hide(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		else
			win->Hide();
	}


	void HealthWindow::show(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);

		win->Show();
	}



	void HealthWindow::update()
	{
		// clientNum = 0;
		// Logger::getInstance()->debug(int2str(clientNum));

		if (game->gameUI->getFirstPerson( clientNum ) == NULL)
		{
			// Logger::getInstance()->debug("crap");
			return;
		}

		float multiplier = healthTextMultiplier;
		if( true && game && game->gameUI && game->gameUI->getFirstPerson(clientNum) && game->gameUI->getFirstPerson(clientNum )->getUnitType()  )
		{
			multiplier = game->gameUI->getFirstPerson(clientNum )->getUnitType()->getHealthTextMultiplier();
		}

		float health_rounded = (float)game->gameUI->getFirstPerson(clientNum )->getHP() * multiplier;
		if(health_rounded > 0)
			health_rounded = ceilf(health_rounded);
		else
			health_rounded = 0;

		float max_health_rounded = (float)game->gameUI->getFirstPerson(clientNum )->getMaxHP() * multiplier;
		if(max_health_rounded > 0)
			max_health_rounded = ceilf(max_health_rounded);
		else
			max_health_rounded = 0;

		int health = (int)( health_rounded );
		int max_health = (int)( max_health_rounded );

		if (healthDropBlinkCounter > 0)
		{
			healthDropBlinkCounter--;
			if (healthDropBlinkCounter == 0)
			{
				lastUpdateValue = -1;
				lastUpdateValue2 = -1;
			}
		}
		if (health != lastUpdateValue || max_health != lastUpdateValue2)
		{
			if (lastUpdateValue > health)
			{
				healthDropBlinkCounter = 5;
				// WARNING: magic number here
				// this happens to be 50ms * 5 = 250 ms (or at least should be)
			}

			lastUpdateValue = health;
			lastUpdateValue2 = max_health;

			
			float hpPercentage = 100.0f * health / (float) max_health;
			float fillTo = (float)ceil(hpPercentage);

			if (fillTo < 0) fillTo = 0;
			if (fillTo > 100) fillTo = 100;

			fillupButton->SetClip(0, 0, fillTo, 100);

			healthAmountText->SetText(int2str((int)health) );

			//if (fillTo < 30 || healthDropBlinkCounter > 0)
			if (fillTo < 30)
			{
				healthBackgroundButton->SetDisabledImage(healthBackgroundLowImage);
				fillupButton->SetDisabledImage(fillupLowImage);
			} else {
				healthBackgroundButton->SetDisabledImage(healthBackgroundImage);
				fillupButton->SetDisabledImage(fillupImage);
			}

			if( fillTo <= 0 )
			{
				curveImage = curveDead;
				curveButton->SetDisabledImage( curveImage );
			}
			else
			{
				curveImage = curveLife;
				curveButton->SetDisabledImage( curveImage );
			}

		}
	}

	void HealthWindow::updateCurve()
	{
		if (curveFrames > 0 && curveImage != NULL
			&& game && game->gameUI && game->gameUI->getFirstPerson(clientNum )) // yes, this is necessary..
		{
			float health_percentage = 100.0f * game->gameUI->getFirstPerson(clientNum )->getHP() / (float)game->gameUI->getFirstPerson(clientNum )->getMaxHP();

			float frameSize = 1.0f / (float)curveFrames;
			float curveSpeed = 3.0f;
			if (health_percentage < 50)
			{
				if (health_percentage < 30)
				{
					curveSpeed = 2.0f;
				} else {
					curveSpeed = 2.5f;
				}
			}

			int currentFrame = ((int)(game->gameTimer / curveSpeed) % curveFrames);
			float frameStart = frameSize * currentFrame;
			//float frameStop = frameSize * (currentFrame + 1);

			/*if (health_percentage <= 0)
			{
				currentFrame = 0;
				frameStart = 0;
			}*/

			//curveButton->SetClip(0, frameStartPerc, 100, frameStopPerc);
			curveButton->SetScroll(0, frameStart);
			curveButton->SetRepeat(1.0f, frameSize);
		}
	}


	void HealthWindow::EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
			win->Hide();
	}

	/*void HealthWindow::setHealthTextMultiplier( int clientNum, float m )
	{
		// healthTextMultiplier = m;
	}*/

}
