#include "precompiled.h"

#include "GenericBarWindow.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../ui/CombatWindow.h"
#include "../ui/ElaborateHintMessageWindow.h"
#include "../game/Unit.h"
#include "../ogui/OguiSlider.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../system/Timer.h"
#include <boost/lexical_cast.hpp>

#include "../game/DHLocaleManager.h"
#include "CombatSubWindowFactory.h"

using namespace game;

namespace ui {

const int default_fadeOffTime = 5000;
const int default_fadingTime = 1000;
///////////////////////////////////////////////////////////////////////////////

GenericBarWindow::GenericBarWindow( Ogui* ogui, game::Game* game, int player ) :
	ogui( ogui ),
	game( game ),
	player( player ),
	win( NULL ),
	win_hidden( NULL ),
	slider( NULL ),
	updator( NULL ),
	isOfFadingType( false ),
	sliderValue( 1.0f ),
	lastTime( 0 ),
	hidden( false ),
	reallyHidden( false ),
	messageOnHide(),
	messageOnHideStyle( ),
	messageOnShow( "" ),
	fadeOffTime( default_fadeOffTime ),
	fadeOutTime( default_fadingTime ),
	fadeInTime( default_fadingTime )
{
}

//=============================================================================

GenericBarWindow::~GenericBarWindow()
{
	Release();
}

//=============================================================================

void GenericBarWindow::loadDataFromLocales( const std::string& locale_name )
{
	const std::string prefix = locale_name + "_";
	{
		int x = getLocaleGuiInt( ( prefix + "x" ).c_str(), 0 );
		int y = getLocaleGuiInt( ( prefix + "y" ).c_str(), 0 );
		int w = getLocaleGuiInt( ( prefix + "w" ).c_str(), 0 );
		int h = getLocaleGuiInt( ( prefix + "h" ).c_str(), 0 );

		win = ogui->CreateSimpleWindow( x, y, w, h, NULL, 0 );
		win->SetUnmovable();
		win->SetEffectListener(this);


		const char *hidden_img = NULL;
		if( DHLocaleManager::getInstance()->getString(DHLocaleManager::BANK_GUI, ( prefix + "img_hidden" ).c_str(), &hidden_img))
		{
			win_hidden = ogui->CreateSimpleWindow( x, y, w, h, hidden_img, 0 );
			win_hidden->SetUnmovable();
			win_hidden->Hide();
			win_hidden->SetEffectListener(this);
		}
	}
	{
		int x = getLocaleGuiInt( ( prefix + "x" ).c_str(), 0 );
		int y = getLocaleGuiInt( ( prefix + "y" ).c_str(), 0 );
		int w = getLocaleGuiInt( ( prefix + "w" ).c_str(), 0 );
		int h = getLocaleGuiInt( ( prefix + "h" ).c_str(), 0 );

		const std::string& background = getLocaleGuiString( ( prefix + "background" ).c_str() );
		const std::string& foreground = getLocaleGuiString( ( prefix + "foreground" ).c_str() );

		slider = new OguiSlider( win, ogui, x, y, w, h, 
								background, background, background,
								foreground, foreground, foreground );

		slider->setDisabledImages( background, foreground );
		slider->setDisabled( true );

		bool is_vertical = getLocaleGuiInt( ( prefix + "is_vertical" ).c_str() , 0 ) == 1;
		if(is_vertical)
		{
			slider->setSliderDirection(false);
		}
		

		int max = is_vertical ? h : w;
		int start = getLocaleGuiInt( ( prefix + "foreground_start" ).c_str() , 0 );
		int end = getLocaleGuiInt( ( prefix + "foreground_end" ).c_str() , max );

		valueStart = start / (float)max;
		valueScale = (end - start) / (float)max;

	}
	{
		// 
		isOfFadingType = getLocaleGuiInt( (prefix + "fading").c_str(), 0 )==0?false:true;

		messageOnShowStyle = getLocaleGuiString( (prefix + "message_on_show_style" ).c_str() );
		messageOnHideStyle = getLocaleGuiString( (prefix + "message_on_hide_style" ).c_str() );

		{
			int x = getLocaleGuiInt(("gui_elaborate_hint_" + messageOnShowStyle + "_formatted_text_x").c_str(), 0);
			int y = getLocaleGuiInt(("gui_elaborate_hint_" + messageOnShowStyle + "_formatted_text_y").c_str(), 0);
			showTextOffsetX = x - win->GetPositionX();
			showTextOffsetY = y - win->GetPositionY();
		}

		{
			int x = getLocaleGuiInt(("gui_elaborate_hint_" + messageOnHideStyle + "_formatted_text_x").c_str(), 0);
			int y = getLocaleGuiInt(("gui_elaborate_hint_" + messageOnHideStyle + "_formatted_text_y").c_str(), 0);
			hideTextOffsetX = x - win->GetPositionX();
			hideTextOffsetY = y - win->GetPositionY();
		}

		messageOnHide = getLocaleGuiString( (prefix + "message_on_hide" ).c_str() );
		if( DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, (prefix + "message_on_show" ).c_str() ) )
		{
			messageOnShow = getLocaleGuiString( (prefix + "message_on_show" ).c_str() );

			if( messageOnShowStyle.empty() == false && game && game->gameUI && game->gameUI->getCombatWindow( player ) &&
				game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ) )
			{
				((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ))->showMessage( messageOnShowStyle, messageOnShow );
				// game->gameUI->getCombatWindow( player )->
			}
		}

		fadeOffTime = getLocaleGuiInt( (prefix + "fade_after" ).c_str(), fadeOffTime );
		fadeOutTime = getLocaleGuiInt( (prefix + "fade_out_time" ).c_str(), fadeOutTime );
		fadeInTime = getLocaleGuiInt( (prefix + "fade_in_time" ).c_str(), fadeInTime );
		// fadeOffTime = getLocaleGuiInt( (prefix + "fade_after" ).c_str(), fadeOffTime );
	}


	if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, (prefix + "window_w").c_str()))
	{
		int w = getLocaleGuiInt( (prefix + "window_w" ).c_str(), 0);
		int h = getLocaleGuiInt( (prefix + "window_h" ).c_str(), 0);
		win->Resize(w, h);
	}

	if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, (prefix + "bar_x").c_str()))
	{
		int x = getLocaleGuiInt( (prefix + "bar_x" ).c_str(), 0);
		int y = getLocaleGuiInt( (prefix + "bar_y" ).c_str(), 0);
		slider->Move(x, y);
	}

	std::string amount_locale = (prefix + "decoration_amount");
	if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, amount_locale.c_str()))
	{
		int amount = getLocaleGuiInt(amount_locale.c_str(), 0);
		for(int i = 0; i < amount; i++)
		{
			std::string prefix2 = prefix + "decoration_" + boost::lexical_cast<std::string>(i);

			int x = getLocaleGuiInt( (prefix2 + "_x" ).c_str(), 0);
			int y = getLocaleGuiInt( (prefix2 + "_y" ).c_str(), 0);
			int w = getLocaleGuiInt( (prefix2 + "_w" ).c_str(), 0);
			int h = getLocaleGuiInt( (prefix2 + "_h" ).c_str(), 0);

			const char *img = getLocaleGuiString( (prefix2 + "_img" ).c_str() );
			const char *texttext = getLocaleGuiString( (prefix2 + "_text" ).c_str() );
			const char *fontname = getLocaleGuiString( (prefix2 + "_font" ).c_str() );

			IOguiFont *font = fontname != NULL && fontname[0] != 0 ? ogui->LoadFont(fontname) : NULL;
			
			OguiButton *button;
			if(texttext != NULL && texttext[0] != 0)
			{
				button = ogui->CreateSimpleTextButton(win, x, y, w, h, NULL, NULL, NULL, texttext, 0, 0, false);
				button->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
			}
			else
			{
				button = ogui->CreateSimpleImageButton(win, x, y, w, h, NULL, NULL, NULL, img, 0, 0, false);
			}
			button->SetDisabled(true);
			if(font) button->SetFont(font);
			button->SetText(texttext);

			decorations.push_back(button);
			if(font) decorationfonts.push_back(font);
		}
	}

}

//=============================================================================

void GenericBarWindow::hide( int fadeTime )
{
	reallyHidden = true;
	fadeHide( fadeTime );
	if(win_hidden) win_hidden->Hide();
}

//=============================================================================

void GenericBarWindow::show( int fadeTime )
{
	reallyHidden = false;
	fadeShow( fadeTime );
}

//=============================================================================

void GenericBarWindow::fadeHide( int fadeTime )
{
	if( win )
	{
		if(fadeTime)
		{
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
			if(win_hidden) win_hidden->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);
		}
		else
		{
			win->EndAllEffects();
			win->Hide();
			if(win_hidden) win_hidden->EndAllEffects();
		}

		if(win_hidden) win_hidden->Show();

		hidden = true;
	}
}

//=============================================================================

void GenericBarWindow::fadeShow( int fadeTime )
{
	if( reallyHidden ) 
		return;

	if( win )
	{
		if(fadeTime)
		{
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);
			if(win_hidden) win_hidden->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		}
		else
		{
			win->EndAllEffects();
			if(win_hidden) win_hidden->EndAllEffects();
			if(win_hidden) win_hidden->Hide();
		}

		win->Show();

		hidden = false;
	}
}

//=============================================================================

void GenericBarWindow::update()
{
	if( updator )
	{
		setValue( updator->update() );
	}

	if( isOfFadingType )
	{
		if( ( getTime() - lastTime ) > fadeOffTime )
		{
			if( hidden == false )
			{
				fadeHide( fadeOutTime );

				if( messageOnHideStyle.empty() == false && game && game->gameUI && game->gameUI->getCombatWindow( player ) &&
					game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ) )
				{
					((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ))->showMessage( messageOnHideStyle, messageOnHide );

					OguiFormattedText *text = ((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ))->getFormattedTextForStyle(messageOnHideStyle);
					text->move(win->GetPositionX() + hideTextOffsetX, win->GetPositionY() + hideTextOffsetY);

					// game->gameUI->getCombatWindow( player )->
				}
			}
		}
		else
		{
			if( hidden == true )
			{
				fadeShow( fadeInTime );

				if( messageOnHideStyle.empty() == false && game && game->gameUI && game->gameUI->getCombatWindow( player ) &&
					game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ) )
				{
					((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ))->showMessage( messageOnShowStyle, messageOnShow );
					// game->gameUI->getCombatWindow( player )->

					OguiFormattedText *text = ((ElaborateHintMessageWindow*)game->gameUI->getCombatWindow( player )->getSubWindow( "ElaborateHintMessageWindow" ))->getFormattedTextForStyle(messageOnShowStyle);
					text->move(win->GetPositionX() + showTextOffsetX, win->GetPositionY() + showTextOffsetY);
				}

			}
		}
	}
}

//=============================================================================

// NOTE! deletes the given updator, when it's done with it
void GenericBarWindow::setUpdator( IGenericBarWindowUpdator* up )
{
	delete updator;
	updator = up;
	update();
}

//=============================================================================

void GenericBarWindow::EffectEvent( OguiEffectEvent *e )
{
	if( e && win && e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
	{
		if(e->triggerWindow == win_hidden)
			win_hidden->Hide();
		else
			win->Hide();
		hidden = true;
	}
}

//=============================================================================

void GenericBarWindow::Release()
{
	for(unsigned int i = 0; i < decorations.size(); i++)
	{
		delete decorations[i];
	}

	for(unsigned int i = 0; i < decorationfonts.size(); i++)
	{
		delete decorationfonts[i];
	}

	delete slider;
	slider = NULL;

	delete win;
	win = NULL;

	delete win_hidden;
	win_hidden = NULL;
}

//=============================================================================

void GenericBarWindow::setValue( float value )
{
	if( value > 1.0f ) value = 1.0f;
	if( value < 0.0f ) value = 0.0f;
	
	if( value != sliderValue && slider )
	{
		sliderValue = value;
		lastTime = getTime();

		// if( hidden == false )
		slider->setValue( sliderValue * valueScale + valueStart );
	}
}

//=============================================================================

int GenericBarWindow::getTime() const
{
	return Timer::getTime();
}

void GenericBarWindow::moveBy(int x, int y)
{
	if(win == NULL) return;
	x += win->GetPositionX();
	y += win->GetPositionY();
	win->MoveTo(x, y);
	if(win_hidden) win_hidden->MoveTo(x, y);
}

void GenericBarWindow::move(int x, int y)
{
	if(win == NULL) return;
	win->MoveTo(x, y);
	if(win_hidden) win_hidden->MoveTo(x, y);
}

void GenericBarWindow::resize(int x, int y)
{
	if(win == NULL) return;
	win->Resize(x, y);
	slider->resize(x, y);
	if(win_hidden) win_hidden->Resize(x, y);
}

void GenericBarWindow::getWindowRect(int &x, int &y, int &w, int &h)
{
	if(win == NULL) return;
	x = win->GetPositionX();
	y = win->GetPositionY();
	w = win->GetSizeX();
	h = win->GetSizeY();
}

void GenericBarWindow::raise()
{
	if(win == NULL) return;
	win->Raise();
	if(win_hidden) win_hidden->Raise();
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
