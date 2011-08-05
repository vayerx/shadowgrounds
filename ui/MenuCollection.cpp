
#include "precompiled.h"

#include "MenuCollection.h"
#include <stack>
#include <sstream>

#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"
#include "../game/GameOptionManager.h"
#include "../system/Timer.h"
#include "cursordefs.h"
#include "../game/SimpleOptions.h"
#include "GameVideoPlayer.h"
#include "../game/options/options_sounds.h"
#include "../game/options/options_gui.h"
#include "../game/GameScene.h"

#include "../system/Logger.h"
#include "../util/Debug_MemoryManager.h"
#include "../sound/SoundMixer.h"

#include <boost/lexical_cast.hpp>

#include "IMenuBase.h"

#include "Mainmenu.h"
#include "LoadGameMenu.h"
#include "ProfilesMenu.h"
#include "OptionsMenu.h"
#include "CreditsMenu.h"
#include "NewGameMenu.h"
#ifdef PROJECT_SURVIVOR
	#include "SurvivorLoadGameMenu.h"
	#include "SurvivalMenu.h"
	#include "CoopMenu.h"
#endif
#ifdef PROJECT_SHADOWGROUNDS
#include "../ogui/OguiStormDriver.h"
#include <istorm3d_videostreamer.h>
#endif

namespace ui {
	

const static bool videoDisabledDuringGame = true;
const static bool hidePreviouslyOpenedMenus = false;
const static bool playLogoVideoAlways = false;
const static bool raiseLowerMenu = true;

using namespace game;

class MenuCollectionImpl
{
private:
	Ogui*				ogui;
	Game*				game;
	MenuCollection*		self;		

	float				transparencyFadeSpeed;
	float				transparency;
	bool				fadeIn;
	int					lastUpdate;

	IOguiImage* logoVideo;
	IOguiImage*	backgroundVideo;
	OguiWindow* background;		// the background image
	OguiWindow* foreground;		// the foreground planet

	OguiButton*	logoButton;
	OguiButton* videoButton;
	OguiButton* backgroundButton; // the changeable background image 
	IOguiImage* backgroundButtonData;

	IMenuBase*			activeMenu;

	MenuCollection::Fonts	fonts;

	// alternative backgrounds
	struct BackgroundAlternative
	{
		IOguiImage *img;
		int time;
		int fade;
		BackgroundAlternative() :
			img(NULL)
			,time(0)
			,fade(0) {}
		~BackgroundAlternative() {
			if (img != NULL) {
				delete img;
				img = NULL;
			}
		}
	};
	std::vector<BackgroundAlternative> backgroundAlternatives;
	OguiButton *backgroundAlternativeFader;
	OguiButton *backgroundAlternativeButton;
	int currentBackgroundAlternative;
	int previousBackgroundAlternative;
	int lastBackgroundAlternativeSwitch;

	std::stack< int >			openedMenus;
	std::stack< IMenuBase* >	openedMenusData;
	
	///////////////////////////////////////////////////////////////////////////

	IMenuBase* menuFactory( int menu )
	{
		switch( menu )
		{
		case MenuCollection::MENU_TYPE_MAINMENU:
			return new MainMenu( self, &fonts, ogui, game );
			break;

		case MenuCollection::MENU_TYPE_LOADGAMEMENU:
#ifdef PROJECT_SURVIVOR
			if(!game::SimpleOptions::getBool(DH_OPT_B_USE_OLD_LOADGAMEMENU))
				return new SurvivorLoadGameMenu( self, &fonts, ogui, game );
#endif
			return new LoadGameMenu( self, &fonts, ogui, game );
			break;

#ifdef PROJECT_SURVIVOR
		case MenuCollection::MENU_TYPE_SURVIVALMENU:
			return new SurvivalMenu( self, &fonts, ogui, game );
			break;
		case MenuCollection::MENU_TYPE_COOPMENU:
			return new CoopMenu( self, &fonts, ogui, game );
			break;
#endif

		case MenuCollection::MENU_TYPE_PROFILESMENU:
			return new ProfilesMenu( self, &fonts, ogui, game );
			break;

		case MenuCollection::MENU_TYPE_OPTIONSMENU:
			return new OptionsMenu( self, &fonts, ogui, game );
			break;
		
		case MenuCollection::MENU_TYPE_CREDITSMENU:
			return new CreditsMenu( self, &fonts, ogui, game );
			break;

		case MenuCollection::MENU_TYPE_NEWGAMEMENU:
			return new NewGameMenu( self, &fonts, ogui, game ); 
			break;

		default:
			assert( false && "Unknown menu" );
			break;
		}

		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////


public:
	MenuCollectionImpl( MenuCollection* self, Ogui* ogui, Game* game, int player ) :
	  ogui( ogui ),
	  game( game ),
	  self( self ),
	  transparency( 100 ),
	  fadeIn( true ),
	  logoVideo( NULL ),
	  backgroundVideo( NULL ),
	  background( NULL ),
	  foreground( NULL ),
	  logoButton( NULL ),
	  videoButton( NULL ),
	  backgroundButton( NULL ),
	  backgroundButtonData( NULL ),
	  activeMenu( NULL ),
	  backgroundAlternativeFader( NULL ),
	  backgroundAlternativeButton( NULL )
	{
		background = ogui->CreateSimpleWindow(	getLocaleGuiInt( "gui_menu_background_x", 0 ), 
												getLocaleGuiInt( "gui_menu_background_y", 0 ), 
												getLocaleGuiInt( "gui_menu_background_w", 1 ), 
												getLocaleGuiInt( "gui_menu_background_h", 1 ), 
												getLocaleGuiString( "gui_menu_background_img" ) );

		const char *alternatives_str = NULL;
		if(DHLocaleManager::getInstance()->getString(DHLocaleManager::BANK_GUI, "gui_menu_background_alternatives", &alternatives_str))
		{
			int alternatives = atoi(alternatives_str);
			if(alternatives > 0)
			{
				backgroundAlternativeButton = ogui->CreateSimpleImageButton(background,
					getLocaleGuiInt("gui_menu_background_alternatives_x", 0), getLocaleGuiInt("gui_menu_background_alternatives_y", 0),
					getLocaleGuiInt("gui_menu_background_alternatives_w", 0), getLocaleGuiInt("gui_menu_background_alternatives_h", 0),
					NULL, NULL, NULL, NULL, 0, NULL, false);
				backgroundAlternativeButton->SetDisabled(true);

				backgroundAlternativeFader = ogui->CreateSimpleImageButton(background,
					getLocaleGuiInt("gui_menu_background_alternatives_x", 0), getLocaleGuiInt("gui_menu_background_alternatives_y", 0),
					getLocaleGuiInt("gui_menu_background_alternatives_w", 0), getLocaleGuiInt("gui_menu_background_alternatives_h", 0),
					NULL, NULL, NULL, NULL, 0, NULL, false);
				backgroundAlternativeFader->SetDisabled(true);

				backgroundAlternatives.resize(alternatives);
				for(int i = 0; i < alternatives; i++)
				{
					std::string prefix = "gui_menu_background_alt" + boost::lexical_cast<std::string>(i);
					backgroundAlternatives[i].img = ogui->LoadOguiImage( getLocaleGuiString( (prefix+"_img").c_str() ) );
					backgroundAlternatives[i].time = getLocaleGuiInt( (prefix+"_time").c_str() , 0 );
					backgroundAlternatives[i].fade = getLocaleGuiInt( (prefix+"_fade").c_str() , 0 );
				}

				backgroundAlternativeButton->SetDisabledImage(backgroundAlternatives[0].img);
				currentBackgroundAlternative = 0;
				previousBackgroundAlternative = 0;
				lastBackgroundAlternativeSwitch = Timer::getTime();
			}
		}
	
		foreground = ogui->CreateSimpleWindow(	getLocaleGuiInt( "gui_menu_foreground_x", 0 ), 
												getLocaleGuiInt( "gui_menu_foreground_y", 0 ), 
												getLocaleGuiInt( "gui_menu_foreground_w", 1 ), 
												getLocaleGuiInt( "gui_menu_foreground_h", 1 ), 
												getLocaleGuiString( "gui_menu_foreground_img" ) );
		foreground->SetReactMask( 0 );
		
		


		videoButton = ogui->CreateSimpleImageButton( background, 
							getLocaleGuiInt( "gui_menu_video_x", 0 ), getLocaleGuiInt( "gui_menu_video_y", 0 ),
							getLocaleGuiInt( "gui_menu_video_w", 1024 ), getLocaleGuiInt( "gui_menu_video_h", 768 ), NULL, NULL, NULL );
		videoButton->SetDisabled( true );

		backgroundButton = ogui->CreateSimpleImageButton( background, 0, 0, 1024, 768, NULL, NULL, NULL );
		backgroundButton->SetDisabled( true );

		background->Hide();
		foreground->Hide();
		background->SetUnmovable();
		foreground->SetUnmovable();

		ogui->SetCursorImageState( 0, DH_CURSOR_ARROW );

		fonts.big.normal =			ogui->LoadFont( getLocaleGuiString( "gui_menu_font_big" ) );
		fonts.medium.normal =		ogui->LoadFont( getLocaleGuiString( "gui_menu_font_medium" ) );
		fonts.medium.highlighted =	ogui->LoadFont( getLocaleGuiString( "gui_menu_font_medium_high" ) );
		fonts.little.normal =		ogui->LoadFont( getLocaleGuiString( "gui_menu_font_little" ) );
		fonts.little.highlighted =	ogui->LoadFont( getLocaleGuiString( "gui_menu_font_little_high" ) );

		

		background->StartEffect( OGUI_WINDOW_EFFECT_FADEIN, 3000 );
		foreground->StartEffect( OGUI_WINDOW_EFFECT_FADEIN, 3000 );

		if( game->inCombat == false )
			transparencyFadeSpeed = 50.0f;
		else transparencyFadeSpeed = 20.0f;
		
		const int menu_to_be_opened = MenuCollection::MENU_TYPE_MAINMENU;
		openMenu( menu_to_be_opened );
	}

	~MenuCollectionImpl()
	{
		if( hidePreviouslyOpenedMenus == false )
		{
			while( !openedMenus.empty() )
			{
				closeMenu();
			}
		}

		delete activeMenu;

		delete logoButton;
		delete videoButton;
		delete logoVideo;
		delete backgroundVideo;
		delete backgroundAlternativeFader;
		delete backgroundAlternativeButton;
		delete background;
		delete foreground;
		

		delete fonts.big.normal;
		delete fonts.medium.normal;
		delete fonts.medium.highlighted;
		delete fonts.little.normal;
		delete fonts.little.highlighted;
	}

	void start()
	{
		setFadeIn( true );
		initVideos();
	}

	void initVideos()
	{
		
		bool video_enabled = GameOptionManager::getInstance()->getOptionByName( "menu_video_enabled" )? GameOptionManager::getInstance()->getOptionByName( "menu_video_enabled" )->getBooleanValue() : true;

		if( video_enabled && game->inCombat && videoDisabledDuringGame )
			video_enabled = false;

		sfx::SoundMixer *mixer = game->gameUI->getSoundMixer();
		IStorm3D_StreamBuilder *builder = 0;
		if(mixer)
			builder = mixer->getStreamBuilder();

		if( video_enabled )
			backgroundVideo = ogui->LoadOguiVideo( getLocaleGuiString( "gui_menu_video_video" ), builder );
		
		if( backgroundVideo == NULL )
		{
			// background->setBackgroundImage( );
			backgroundVideo = ogui->LoadOguiImage( getLocaleGuiString( "gui_menu_video_image" ) );
		}

#ifdef PROJECT_SHADOWGROUNDS
		float normal_aspect = (background->GetSizeX() * ogui->GetScaleX()) / (float)(background->GetSizeY() * ogui->GetScaleY());

		float video_aspect = (float)atof( getLocaleGuiString("gui_menu_video_video_aspect") );

		float texcoord_multiplier = (video_aspect / normal_aspect);
		float texcoord_offset = (1.0f - texcoord_multiplier) * 0.5f;

		float xm = 0.0f, ym = 0.0f;
		OguiStormImage* bv = (OguiStormImage*)backgroundVideo;
		if (bv->video != NULL)
		{
			bv->video->getTextureCoords(xm, ym);
			if(texcoord_multiplier > 1.0f)
			{
				videoButton->SetRepeat(xm, texcoord_multiplier * ym);
				videoButton->SetScroll(0.0f, texcoord_offset);
			}
		}
#endif

		videoButton->SetDisabledImage( backgroundVideo );

		{
			bool playLogoVideo = GameOptionManager::getInstance()->getOptionByName( "menu_logo_video_enabled" )? GameOptionManager::getInstance()->getOptionByName( "menu_logo_video_enabled" )->getBooleanValue() : true;
			OguiButton* button;
			button = ogui->CreateSimpleImageButton( foreground, 
													getLocaleGuiInt( "gui_menu_logo_x", 0 ), 
													getLocaleGuiInt( "gui_menu_logo_y", 0 ),
													getLocaleGuiInt( "gui_menu_logo_w", 1 ),
													getLocaleGuiInt( "gui_menu_logo_h", 1 ), 
													NULL, NULL, NULL, 0 );

			if( ( video_enabled || playLogoVideoAlways ) && playLogoVideo )
				logoVideo = ogui->LoadOguiVideo( getLocaleGuiString( "gui_menu_logo_video" ), builder );
			
			if( logoVideo == NULL ) 
				logoVideo = ogui->LoadOguiImage( getLocaleGuiString( "gui_menu_logo_img" ) );

#ifdef PROJECT_SHADOWGROUNDS
			normal_aspect = (button->GetSizeX() * ogui->GetScaleX()) / (float)(button->GetSizeY() * ogui->GetScaleY());
			video_aspect = (float)atof( getLocaleGuiString("gui_menu_logo_video_aspect") );

			texcoord_multiplier = (video_aspect / normal_aspect);
			texcoord_offset = (1.0f - texcoord_multiplier) * 0.5f;

			OguiStormImage* lv = (OguiStormImage*)logoVideo;
			if (lv->video != NULL)
			{
				lv->video->getTextureCoords(xm, ym);
				if(texcoord_multiplier > 1.0f)
				{
					button->SetRepeat(xm, texcoord_multiplier * ym);
					button->SetScroll(0.0f, texcoord_offset);
				}
			}
#endif

			button->SetDisabled( true );
			button->SetDisabledImage( logoVideo );

			logoButton = button;
		}
	}

	void setFadeIn( bool value )
	{
		/*if( value )
		{
			fadeIn = true;
			transparency = 0.0f;
			foreground->SetTransparency( 0 );
			lastUpdate = Timer::getTime();
		}
		else
		{
			fadeIn = false;
			transparency = 100.0f;
			foreground->SetTransparency( 100 );
		}*/
	}

	void update()
	{
		/*if( fadeIn )
		{
			transparency += ( Timer::getTime() - lastUpdate ) / transparencyFadeSpeed;
			lastUpdate = Timer::getTime();
			if( transparency > 100.0f )
			{
				transparency = 100.0f;
				fadeIn = false;
				foreground->SetTransparency( 100 );
			}
			else
			{
				foreground->SetTransparency( (int)transparency );
			}

			
		}*/

		if(backgroundAlternativeButton != NULL)
		{
			BackgroundAlternative &abg_current = backgroundAlternatives[currentBackgroundAlternative];
			BackgroundAlternative &abg_previous = backgroundAlternatives[previousBackgroundAlternative];

			// time to switch
			if(Timer::getTime() - lastBackgroundAlternativeSwitch >= abg_current.time)
			{
				lastBackgroundAlternativeSwitch = Timer::getTime();

				// store current
				previousBackgroundAlternative = currentBackgroundAlternative;

				// switch to next
				currentBackgroundAlternative++;
				if(currentBackgroundAlternative >= (int)backgroundAlternatives.size())
					currentBackgroundAlternative = 0;

				BackgroundAlternative &abg_next = backgroundAlternatives[currentBackgroundAlternative];

				// next to background
				backgroundAlternativeButton->SetDisabledImage(abg_next.img);
				// current to fader
				backgroundAlternativeFader->SetDisabledImage(abg_current.img);
				backgroundAlternativeFader->SetTransparency(0);
			}
			else if(Timer::getTime() - lastBackgroundAlternativeSwitch <= abg_previous.fade)
			{
				// fading
				float factor = (Timer::getTime() - lastBackgroundAlternativeSwitch) / (float)abg_previous.fade;
				if(factor >= 1.0f)
					backgroundAlternativeFader->SetDisabledImage(NULL);
				else
					backgroundAlternativeFader->SetTransparency((int)(factor * 100.0f));
			}
		}
	}

	void hide()
	{
		background->Hide();
		if( activeMenu ) activeMenu->hide();
		foreground->Hide();
	}

	void show()
	{
		background->Show();
		if( activeMenu ) activeMenu->show();
		foreground->Show();
	}

	bool isVisible() const
	{
		return true;
		if ( activeMenu )	return activeMenu->isVisible();
		else				return background->IsVisible();	
	}

	bool wasQuitPressed() // const
	{
		update();

		if ( activeMenu ) activeMenu->update();
		if ( activeMenu ) return activeMenu->wasQuitPressed();
		else			  return false; // this is a bit of a hack
	}

	void changeMenu( int menu )
	{	
		openedMenus.pop();
		delete activeMenu;
		activeMenu = NULL;

		IMenuBase *lower_menu = openedMenusData.empty()?NULL:openedMenusData.top();

		activeMenu = menuFactory( menu );
		
		if(raiseLowerMenu && lower_menu) lower_menu->raise();
		
		assert( foreground );
		foreground->Raise();

		openedMenus.push( menu );
		activeMenu->show();
	}

	void openMenu( int menu )
	{
		ogui->SetCursorImageState( 0, DH_CURSOR_ARROW );

		if( activeMenu && hidePreviouslyOpenedMenus ) 
		{
			activeMenu->hide();
			delete activeMenu;
		}
		
		if( activeMenu && hidePreviouslyOpenedMenus == false )
		{
			openedMenusData.push( activeMenu );
		}

		if( activeMenu && raiseLowerMenu )
		{
			IMenuBase* temp = activeMenu;
			activeMenu = menuFactory( menu );
			temp->raise();
		}
		else
		{
			activeMenu = menuFactory( menu );
		}
		
		assert( foreground );
		foreground->Raise();
		
		openedMenus.push( menu );
		activeMenu->show();
	}

	void closeMenu()
	{
		openedMenus.pop();
		int menu = openedMenus.empty()?0:openedMenus.top();
		
		activeMenu->applyChanges();

		delete activeMenu;
		activeMenu = NULL;

		if( hidePreviouslyOpenedMenus )
		{
			if ( !openedMenus.empty() )
			{
				activeMenu = menuFactory( menu );
				activeMenu->show();
				assert( foreground );
				foreground->Raise();
			}
		}
		else
		{
			activeMenu = openedMenusData.empty() ? NULL : openedMenusData.top();
			if( !openedMenusData.empty() )
				openedMenusData.pop();

			if( activeMenu ) 
				activeMenu->show();
			assert( foreground );
			foreground->Raise();
		}
	}

	void loadMission( int n )
	{
		std::stringstream ss;
		ss << n;

		game->loadGame( ss.str().c_str() );	
		// assert( false && "TODO" );
	}

	void newMission()
	{
		/*
		{
			IStorm3D_StreamBuilder *builder = 0;
			sfx::SoundMixer *mixer = game->gameUI->getSoundMixer();
			if(mixer)
				builder = mixer->getStreamBuilder();

			ui::GameVideoPlayer::playVideo( game->getGameUI()->getStormScene(), getLocaleGuiString( "newgame_video" ), builder);
		}
		*/
#ifdef PROJECT_SURVIVOR
		game->gameUI->openCinematicScreen("SGS_intro");
#else
		game->loadGame("new");
#endif
	}

	void escPressed()
	{
		if( activeMenu ) 
			activeMenu->escPressed();
	}

	void setBackgroundImage( const std::string& background )
	{
		delete backgroundButtonData;
		backgroundButtonData = NULL;

		backgroundButtonData = ogui->LoadOguiImage( background.c_str() );
		backgroundButton->SetDisabledImage( backgroundButtonData );
	}

};


//=============================================================================

MenuCollection::MenuCollection( Ogui* ogui, Game* game, int player, bool start_automagicly, int menu_to_be_opened )
{
	this->impl = new MenuCollectionImpl( this, ogui, game, player );
	if( start_automagicly )
		this->impl->start();

	if( menu_to_be_opened != 0 )
	{
		impl->openMenu( menu_to_be_opened );
	}
}

MenuCollection::~MenuCollection()
{
	delete this->impl;
}

void MenuCollection::start()
{
	this->impl->start();
}

void MenuCollection::hide()
{
	this->impl->hide();
}

void MenuCollection::show()
{
	this->impl->show();
}

bool MenuCollection::isVisible() const
{
	return this->impl->isVisible();
}

bool MenuCollection::wasQuitPressed()
{
	return this->impl->wasQuitPressed();
}

void MenuCollection::openMenu( int menu )
{
	this->impl->openMenu( menu );
}

void MenuCollection::closeMenu()
{
	this->impl->closeMenu();
}

void MenuCollection::loadMission( int n )
{
	this->impl->loadMission( n );
}

void MenuCollection::newMission()
{
	this->impl->newMission();
}

void MenuCollection::escPressed()
{
	this->impl->escPressed();
}

void MenuCollection::setBackgroundImage( const std::string& background )
{
	this->impl->setBackgroundImage( background );
}


void MenuCollection::changeMenu( int menu )
{
	this->impl->changeMenu( menu );
}



//=============================================================================
}

