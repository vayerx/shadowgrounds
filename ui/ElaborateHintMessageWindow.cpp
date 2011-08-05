#include "precompiled.h"

#include "ElaborateHintMessageWindow.h"
#include "CombatSubWindowFactory.h"

#include "../game/GameStats.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_game.h"
#include "../game/DHLocaleManager.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"

#include "../util/fb_assert.h"
#include "../util/StringUtil.h"

#include "../system/Timer.h"
#include <sstream>

namespace ui { 
///////////////////////////////////////////////////////////////////////////////

namespace {
REGISTER_COMBATSUBWINDOW( ElaborateHintMessageWindow );
}

///////////////////////////////////////////////////////////////////////////////

const std::string fadingTextWindowName = "elaborate_hint";
bool elaborateLoggingEnabled = false;

class CFadingTextMessage
{
public:

	enum FadeStatus {
		FS_NoFade = 0,
		FS_FadeIn = 1,
		FS_FadeOut = 2
	};

	struct QueuedMessage
	{
		int timeShow;
		std::string message;
	};

	CFadingTextMessage( game::Game *game, Ogui* ogui, OguiWindow* window, const std::string& style_name ) :
		dead( false ),

		ogui( ogui ),
		window( window ),
		formattedText( NULL ),
		oguiLoader( ogui ),

		fadeStatus( FS_FadeIn ),
		transparency( 1.0f ),

		immortal( true ),
		fading( true ),
		creationTime( getTime() ),
		fadeInStartTime( getTime() ),
		fadeTime( 1000 ),
		lifeTime( 20000 ),
		queueTime( 0 ),
		game(game)

	{
		// oguiLoader.SetLogging( elaborateLoggingEnabled, "missing_locales.txt" );
		oguiLoader.SetWindowName( fadingTextWindowName + "_" + style_name );
		formattedText = oguiLoader.LoadFormattedText( "formatted_text", window, 0 );

		{
			std::string temp = "gui_" + fadingTextWindowName + "_" + style_name + "_" + "fading";
			bool fade = game::getLocaleGuiInt( temp.c_str(), 0  )==0?false:true;

			temp = "gui_" + fadingTextWindowName + "_" + style_name + "_" + "fading_time";
			fadeTime = game::getLocaleGuiInt( temp.c_str(), 1000 );

			temp = "gui_" + fadingTextWindowName + "_" + style_name + "_" + "life_time";
			lifeTime = game::getLocaleGuiInt( temp.c_str(), 10000 );

			temp = "gui_" + fadingTextWindowName + "_" + style_name + "_" + "immortal";
			immortal = game::getLocaleGuiInt( temp.c_str(), 0  )==0?false:true;

			temp = "gui_" + fadingTextWindowName + "_" + style_name + "_" + "queue_time";
			queueTime = game::getLocaleGuiInt( temp.c_str(), 0  );	

			SetFade( fade );
		}
		ResetCreationTime();
		creationTime = 0;
		Update();
	}

	~CFadingTextMessage() 
	{ 
		delete formattedText;
		formattedText = NULL;
	}

	void Update() 
	{ 
		// bool fadeIn = false;
		switch( fadeStatus )
		{
		case FS_FadeIn:
			{
				float t = 1.0f - ( (float)( getTime() - fadeInStartTime ) / (float)fadeTime );

				if( ( getTime() - fadeInStartTime ) > fadeTime )
					t = -0.01f;

				if( t > 0.0f )
				{
					transparency = t;
				}
				else
				{
					fadeStatus = FS_NoFade;
					transparency = 0.0f;
				}

				formattedText->setTransparency( (int)( transparency * 100.0f + 0.5f ) );
			}
			break;

		case FS_FadeOut: 
			if( immortal == false )
			{
				float t = 1.0f - ( (float)( ( creationTime + lifeTime ) - getTime() ) / (float)fadeTime );
				
				if( getTime() > ( creationTime + lifeTime ) )
					t = 1.01f;

				if( t > 0.0f && t < 1.0f )
				{
					transparency = t;
				}
				else if( t >= 1.0f )
				{
					fadeStatus = FS_NoFade;
					transparency = 1.0f;
						dead  = true;
				}
				formattedText->setTransparency( (int)( transparency * 100.0f + 0.5f ) );
			}
			break;
		default:
			break;
		}

		if( immortal == false && dead == false && fadeStatus == FS_NoFade && fading )
		{
			if( getTime() >= ( creationTime + lifeTime - fadeTime) )
				fadeStatus = FS_FadeOut;
		}	

		if( immortal == false && fading == false )
		{
			if( getTime() >= ( creationTime + lifeTime ) )
				dead = true;
		}

		// check first entry in queue
		if(messageQueue.size() > 0)
		{
			dead = false;

			QueuedMessage &qm = messageQueue.front();
			if(qm.timeShow <= getTime())
			{
				SetText_internal( qm.message );
				// pop first to avoid infinite recursive loop
				messageQueue.pop_front();
				ResetCreationTime();
			}
		}
	}

	bool QueueText( const std::string& text )
	{
		if(queueTime > 0)
		{
			QueuedMessage qm;

			// the queue is not empty
			if(messageQueue.size() > 0)
			{
				// add to end of queue
				qm.timeShow = messageQueue.back().timeShow + queueTime;
			}
			else
			{
				// current message should be visible for queueTime
				qm.timeShow = creationTime + queueTime;

				// has already been?
				if(qm.timeShow <= getTime())
				{
					// just show it directly
					return false;
				}
			}

			qm.message = text;
			messageQueue.push_back( qm );
			return true;
		}
		return false;
	}

	void SetText( const std::string& text ) 
	{ 
		if( formattedText )
		{
			// try to queue
			if(!QueueText( text ))
			{
				// otherwise just show it directly
				SetText_internal( text );
				ResetCreationTime();
			}
		}
	}

	void SetFade( bool fade )
	{
		if( fade )
		{
			fading = true;
			fadeStatus = FS_FadeIn;
			transparency = 1.0f;
		}
		else
		{
			fading = false;
			fadeStatus = FS_NoFade;
			transparency = 0.0f;
		}
	}

	void ResetCreationTime()
	{
		creationTime = getTime();

		if( fadeStatus != FS_NoFade )
		{
			if( fading )
			{
				fadeStatus = FS_FadeIn;
				fadeInStartTime = Timer::getTime() - (int)( ( 1.0f - transparency ) * (float)fadeTime + 0.5f );
				Update();
			}
		}
	}

	void SetImmortal( bool value )
	{
		if( immortal == true && value == false )
		{
			immortal = value;
			// lifeTime = 0;
			// fadeStatus = FS_FadeOut;
			// lifeTime = fadeTime;
			// lifeTime = ( getTime() - creationTime );
		}
	}

	bool IsDead() const { return dead; }

	int getTime() const 
	{
		return Timer::getTime();
	}

	OguiFormattedText* GetFormattedText()
	{
		return formattedText;
	}

private:

	inline bool stringStartsWith(const std::string &str, const std::string id)
	{
		return strncmp(str.c_str(), id.c_str(), id.length()) == 0;
	}

	void SetText_internal(std::string text)
	{
		// find extra tags
		for(unsigned int i = 0; i < text.size(); i++)
		{
			if(text[i] == '<')
			{
				int tag_size = text.find('>', i) - i;
				std::string tag = util::StringRemoveWhitespace(text.substr(i+1, tag_size - 1));
				
				bool remove_tag = true;

				// audiofile
				if(stringStartsWith(tag, "audiofile:"))
				{
					std::string file = tag.substr(strlen("audiofile:"));
					game->gameUI->playGUISound(file.c_str(), 100);
				}

				// audiostream
				else if(stringStartsWith(tag, "audiofilestream:"))
				{
					std::string file = tag.substr(strlen("audiofilestream:"));
					game->gameUI->playStreamedSound(file.c_str());
				}

				else
				{
					remove_tag = false;
				}

				if(remove_tag)
				{
					text.erase(i, tag_size + 1);
				}
				else
				{
					i += tag_size;
				}
			}
		}
		formattedText->setText(text);
	}

	bool dead;

	Ogui* ogui;

	OguiWindow* window;
	OguiFormattedText*	formattedText;

	OguiLocaleWrapper	oguiLoader;

	FadeStatus			fadeStatus;
	float				transparency;

	bool				immortal;
	bool				fading;
	int					creationTime;
	int					fadeInStartTime;
	int					fadeTime;
	int					lifeTime;

	int queueTime;
	std::list<QueuedMessage> messageQueue;

	game::Game *game;

};

///////////////////////////////////////////////////////////////////////////////

class ElaborateHintMessageWindow::ElaborateHintMessageWindowImpl : private IOguiEffectListener
{
public:
	//-------------------------------------------------------------------------

	ElaborateHintMessageWindowImpl(  Ogui* ogui, game::Game* game, int player_num  ) :
		ogui( ogui ),
		game( game ),
		playerNum( player_num ),
		window( NULL ),
		oguiLoader( ogui ),
		fadingTextMessages()
	{
		oguiLoader.SetLogging( elaborateLoggingEnabled, "missing_locales.txt" );

		window = oguiLoader.LoadWindow( fadingTextWindowName );
		
		if( window )
			window->SetReactMask( 0 );

		// ShowMessage( "medikit", "Testing the medikit fun" );
	}

	~ElaborateHintMessageWindowImpl()
	{
		ReleaseAll();

		delete window;
		window = NULL;
	}

	//-------------------------------------------------------------------------


	void ShowMessage( const std::string& style, const std::string& message )
	{
		CFadingTextMessage* fadingtext = GetStyleThing( style );
		
		fadingtext->SetText( message );
		//else
		//	fadingtext->SetImmortal( false );
	}

	//-------------------------------------------------------------------------

	void Update()
	{
		std::map< std::string, CFadingTextMessage* >::iterator i;
		for( i = fadingTextMessages.begin(); i != fadingTextMessages.end(); )
		{
			i->second->Update();
			
			if( i->second->IsDead() )
			{
				std::map< std::string, CFadingTextMessage* >::iterator remove = i;
				++i;
				delete remove->second;
				fadingTextMessages.erase( remove );
				
			}
			else
			{
				++i;
			}
		}
	}

	//-------------------------------------------------------------------------

	CFadingTextMessage* GetStyleThing( const std::string& style )
	{	
		std::map< std::string, CFadingTextMessage* >::iterator i;
		i = fadingTextMessages.find( style );
		if( i != fadingTextMessages.end() )
		{
			return i->second;
		}
		else
		{
			CFadingTextMessage* result = new CFadingTextMessage( game, ogui, window, style );
			fadingTextMessages.insert( std::pair< std::string, CFadingTextMessage* >( style, result ) );

			return result;
		}
	}
	
	//-------------------------------------------------------------------------

	void Hide( int fadeTime )
	{
		if(fadeTime)
			window->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		else
			window->Hide();
	}

	//............................................................................

	void Show( int fadeTime )
	{
		if(fadeTime)
			window->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);

		window->Show();
	}

	//-------------------------------------------------------------------------

	void EffectEvent(OguiEffectEvent *e)
	{
		if( e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT )
			window->Hide();
	}

	//-------------------------------------------------------------------------

	OguiFormattedText* GetStyleThingFormattedText( const std::string& style )
	{
		return GetStyleThing(style)->GetFormattedText();
	}

private:

	void ReleaseAll()
	{
		std::map< std::string, CFadingTextMessage* >::iterator i;
		for( i = fadingTextMessages.begin(); i != fadingTextMessages.end(); ++i )
		{
			delete i->second;
		}
		fadingTextMessages.clear();
	}


	Ogui* ogui;
	game::Game* game;

	int playerNum;

	OguiWindow* window;
	OguiLocaleWrapper	oguiLoader;

	std::map< std::string, CFadingTextMessage* >	fadingTextMessages;
};

///////////////////////////////////////////////////////////////////////////////

ElaborateHintMessageWindow::ElaborateHintMessageWindow(  Ogui* ogui, game::Game* game, int player_num ) :
	impl( NULL ),
	style( "none" )
{
	impl = new ElaborateHintMessageWindowImpl( ogui, game, player_num );
}

//=============================================================================

ElaborateHintMessageWindow::~ElaborateHintMessageWindow()
{
	delete impl;
	impl = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void ElaborateHintMessageWindow::hide( int time )
{
	FB_ASSERT( impl );
	impl->Hide( time );
}

//=============================================================================

void ElaborateHintMessageWindow::show( int time )
{
	FB_ASSERT( impl );
	impl->Show( time );

}

//=============================================================================

void ElaborateHintMessageWindow::update()
{
	FB_ASSERT( impl );
	impl->Update();
}

//=============================================================================

void ElaborateHintMessageWindow::setStyle( const std::string& s )
{
	style = s;
}

//=============================================================================

void ElaborateHintMessageWindow::showMessage( const std::string& message )
{
	FB_ASSERT( impl );
	impl->ShowMessage( style, message );
}

//.............................................................................

void ElaborateHintMessageWindow::showMessage( const std::string& s, const std::string& message )
{
	FB_ASSERT( impl );
	impl->ShowMessage( s, message );
}

OguiFormattedText* ElaborateHintMessageWindow::getFormattedTextForStyle( const std::string &style )
{
	FB_ASSERT( impl );
	return impl->GetStyleThingFormattedText( style );
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui
