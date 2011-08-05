
#include "precompiled.h"

#include "LoadingWindow.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string>
#include <sstream>

#include <IStorm3D.h>
#include <istorm3d_videostreamer.h>
#include <istorm3D_terrain_renderer.h>
#include "../util/assert.h"

#include "uidefaults.h"
#include "../game/Game.h"
#include "../game/GameProfiles.h"
#include "../game/GameUI.h"
#include "../game/GameScene.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "../game/options/options_video.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../system/Logger.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "../game/scripting/GameScripting.h"
#include "../system/Timer.h"
#include "../sound/sounddefs.h"
#include "../sound/SoundMixer.h"
#include "../ui/UIEffects.h"
#include "../filesystem/input_stream_wrapper.h"

#ifdef PROJECT_SURVIVOR
	#include "SurvivalMenu.h"
#endif

#include "../game/DHLocaleManager.h"
#include "../util/Debug_MemoryManager.h"

#define LOADINGW_CLOSE 1
#define LOADINGW_PROGRESS 2
#define LOADINGW_UPGRADEMENU 3

using namespace game;

namespace ui
{

	// ugh.. these are static variables because we need to
	// set their state before the window is even created?
	bool LoadingWindow::showCharacterSelection = false;
	bool LoadingWindow::showUpgradeWindowOnClose = false;
	bool LoadingWindow::autoCloseEnabled = false;

	LoadingWindow::LoadingWindow( Ogui *ogui, game::Game *game, int player ) :
      ogui( ogui ),
	  win(NULL),
	  game( game ),
	  player( player ),
	  fadingOut( false ),
		closeEnabled( false ),
		closeEnabledTime( 0 ),
	  briefingArea( NULL ),
	  closebut( NULL ),
	  loadingbut( NULL ),
	  upgradeMenuBut( NULL ),
	  headerText( NULL ),
	  missionText( NULL ),
	  headerFont( NULL ),
	  missionFont( NULL ),
	  fontNormal( NULL ),
	  fontBold( NULL ),
	  fontItalic( NULL ),
	  fontUnderline( NULL ),
	  fontButton( NULL ),
	  fontButtonDisabled( NULL ),
	  fontButtonHighlighted( NULL ),
		scrollingText( NULL ),
		scrollingFader( NULL ),
		lastScrollTime( 0 ),
		scrollTimeDeltaAvg( 0.0f ),
		scrollingStarted( false ),
		nextScrollAmount( 0.0f ),
		totalScrollAmount( 0 ),
		speechSound( -1 ),
		speechSoundStartTime( -1 ),
		playedMusic( false ),
		scrollingLimit( 0 ),
		briefVideo( NULL ),
		briefVideoStream( NULL ),
		videoStarted( false ),
		cinematicStarted( false ),
		cinematicFinished( true ),
		lastUpdateTime( 0 ),
		updateStarted( false ),
	  scrolling_font_file( NULL ),
		scrolling_font_bold_file( NULL ),
		scrolling_font_italic_file( NULL ),
		scrolling_font_h1_file( NULL ),
		background_image( NULL )
	{

		FB_ASSERT( ogui != NULL );
		FB_ASSERT( game != NULL );

		createWindows();
		FB_ASSERT( win != NULL );
	}

	LoadingWindow::~LoadingWindow()
	{
		destroyWindows();
	}

	void LoadingWindow::createWindows()
	{
		std::string mission = game->getMissionId()?game->getMissionId():"";
		std::string bgpic;

		std::string custom_survival_mission_desc;
		
#ifdef PROJECT_SURVIVOR
		if(!::DHLocaleManager::getInstance()->hasString( ::DHLocaleManager::BANK_GUI, ( "gui_loadingwindow_" + mission + "_background" ).c_str() ))
		{
			// black survivor image
			SurvivalMenu::MissionInfo mi;
			if(SurvivalMenu::loadMissionInfo(mission, mi))
			{
				custom_survival_mission_desc = mi.description;
				bgpic = "Data/GUI/Menus/blackscreen.tga";
			}
		}
#endif

		if(bgpic.empty())
		{
			bgpic = getLocaleGuiString( ( "gui_loadingwindow_" + mission + "_background" ).c_str() );
		}

		{
			win = ogui->CreateSimpleWindow( 0, 0, 1024, 768, bgpic.c_str() );
			win->SetEffectListener( this );
		}

		// header label
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_headertext_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_headertext_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_headertext_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_headertext_h", 0 );

			std::string header_font = getLocaleGuiString( "gui_loadingwindow_header_font" );

			headerFont = ogui->LoadFont( header_font.c_str() );

			std::string header_text;

			if(custom_survival_mission_desc.empty())
			{
				header_text = getLocaleGuiString( ( mission + "_header_text" ).c_str() );
			}

			headerText = ogui->CreateTextLabel( win, x, y, w, h, header_text.c_str() );
			headerText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
			if( headerFont ) headerText->SetFont( headerFont );

		}

		// mission label
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_missiontext_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_missiontext_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_missiontext_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_missiontext_h", 0 );

			std::string font = getLocaleGuiString( "gui_loadingwindow_mission_font" );

			missionFont = ogui->LoadFont( font.c_str() );

			std::string mission_text = getLocaleGuiString( "gui_loadingwindow_missiontext_text" );

			missionText = ogui->CreateTextLabel( win, x, y, w, h, mission_text.c_str() );
			missionText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
			if( missionFont )
				missionText->SetFont( missionFont );

		}

		// briefing area
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_briefing_area_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_briefing_area_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_briefing_area_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_briefing_area_h", 0 );

			std::string font_file			= getLocaleGuiString( "gui_loadingwindow_font" );
			std::string font_bold_file		= getLocaleGuiString( "gui_loadingwindow_font_bold" ); 
			std::string font_italic_file	= getLocaleGuiString( "gui_loadingwindow_font_italic" );
			std::string font_underline_file = getLocaleGuiString( "gui_loadingwindow_font_underline" );

			if( !font_file.empty() )			fontNormal		= ogui->LoadFont( font_file.c_str() );
			if( !font_bold_file.empty() )		fontBold		= ogui->LoadFont( font_bold_file.c_str() );
			if( !font_italic_file.empty() )		fontItalic		= ogui->LoadFont( font_italic_file.c_str() );
			if( !font_underline_file.empty() )	fontUnderline	= ogui->LoadFont( font_underline_file.c_str() );
			
			briefingArea = new OguiFormattedText( win, ogui, x, y, w, h, 0 );	
			
			if( fontNormal ) 
			{
				briefingArea->setFont( fontNormal );
				
				if( fontBold )		briefingArea->registerFont( "b", fontBold );
				if( fontItalic )	briefingArea->registerFont( "i", fontItalic );
				if( fontUnderline )	briefingArea->registerFont( "u", fontUnderline );

				std::string mission_brief = custom_survival_mission_desc;
				if(mission_brief.empty())
				{
					mission_brief = getLocaleGuiString( ( mission + "_brief" ).c_str() );
				}
				briefingArea->setText( mission_brief );

			} else {
				Logger::getInstance()->error( ( "LoadingWindow::LoadingWindow could not load font: " + font_file ).c_str() );
			}
		}

		// close and loading button fonts...
		{
			std::string font			= getLocaleGuiString( "gui_loadingwindow_button_font" );
			std::string font_disabled	= getLocaleGuiString( "gui_loadingwindow_button_font_disabled" );
			std::string font_high		= getLocaleGuiString( "gui_loadingwindow_button_font_highlighted" );

			fontButton = ogui->LoadFont( font.c_str() );
			if (!font_disabled.empty())
			{
				fontButtonDisabled = ogui->LoadFont( font_disabled.c_str() );
			} else {
				fontButtonDisabled = NULL;
			}

			fontButtonHighlighted = ogui->LoadFont( font_high.c_str() );
		}

		// close (start)
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_closebutton_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_closebutton_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_closebutton_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_closebutton_h", 0 );

			std::string close_normal = getLocaleGuiString( "gui_loadingwindow_closebutton_normal" );
			std::string close_down	 = getLocaleGuiString( "gui_loadingwindow_closebutton_down" );
			std::string close_high	 = getLocaleGuiString( "gui_loadingwindow_closebutton_high" );
			std::string close_text	 = getLocaleGuiString( "gui_loadingwindow_closebutton_text" );

			closebut = ogui->CreateSimpleTextButton( win, x, y, w, h, 
				close_normal.empty()?NULL:close_normal.c_str(), 
				close_down.empty()?NULL:close_down.c_str(),
				close_high.empty()?NULL:close_high.c_str(), close_text.c_str(), LOADINGW_CLOSE );
			closebut->SetListener(this);
			closebut->SetDisabled(true);
			closebut->SetFont(fontButton);
			closebut->SetDisabledFont(fontButtonDisabled);
			closebut->SetHighlightedFont( fontButtonHighlighted );
			if (fontButtonDisabled == NULL)
			{
				closebut->SetText("");
			}
		}

		// loading
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_loadingbutton_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_loadingbutton_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_loadingbutton_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_loadingbutton_h", 0 );

			std::string normal = getLocaleGuiString( "gui_loadingwindow_loadingbutton_normal" );
			std::string down	 = getLocaleGuiString( "gui_loadingwindow_loadingbutton_down" );
			std::string high	 = getLocaleGuiString( "gui_loadingwindow_loadingbutton_high" );
			std::string loadingtext = getLocaleGuiString( "gui_loadingwindow_loadingbutton_text" );

			loadingbut = ogui->CreateSimpleTextButton( win, x, y, w, h, 
				normal.empty()?NULL:normal.c_str(), 
				down.empty()?NULL:down.c_str(),
				high.empty()?NULL:high.c_str(), loadingtext.c_str(), LOADINGW_PROGRESS );
		
			loadingbut->SetReactMask(0);
			loadingbut->SetListener(this);
			loadingbut->SetFont(fontButton);
			loadingbut->SetDisabledFont(fontButtonDisabled);
		}

#ifdef PROJECT_SURVIVOR
		mission_brief2_locale = mission + "_brief2";
		int value = -1;
		if(util::Script::getGlobalIntVariableValue("loading_window_next_brief2_index", &value) && value > 0)
		{
			std::string new_locale = mission_brief2_locale + "_alt" + std::string(int2str(value));
			if(::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, new_locale.c_str() ))
			{
				mission_brief2_locale = new_locale;
			}
		}
		// hack: find alt index from savegame
		else if(strncmp(mission.c_str(), "end0", 4) == 0 && game->getPendingLoad())
		{
			std::string filename = std::string(game->getGameProfiles()->getProfileDirectory( 0 )) + "/Save/save_" + std::string(game->getPendingLoad()) + ".dhs";
			frozenbyte::filesystem::FB_FILE *f = frozenbyte::filesystem::fb_fopen(filename.c_str(), "rb");
			if(f != NULL)
			{
				// read whole file
				size_t size = frozenbyte::filesystem::fb_fsize(f);
				char *buf = new char[size + 1];
				frozenbyte::filesystem::fb_fread(buf, size, 1, f);
				frozenbyte::filesystem::fb_fclose(f);
				buf[size] = 0;

				// find index
				const char *str = strstr(buf, "loading_window_next_brief2_index; setValue ");
				if(str != NULL
					&& sscanf(str, "loading_window_next_brief2_index; setValue %i", &value) == 1
					&& value > 0)
				{
					std::string new_locale = mission_brief2_locale + "_alt" + std::string(int2str(value));
					if(::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, new_locale.c_str() ))
					{
						mission_brief2_locale = new_locale;
					}
				}
				delete[] buf;
			}
		}

		bool has_scrolling_text = ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, mission_brief2_locale.c_str() );
		if(has_scrolling_text)
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_scrollingtext_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_scrollingtext_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_scrollingtext_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_scrollingtext_h", 0 );

			scrolling_font_file = getLocaleGuiString( "gui_loadingwindow_scrollingtext_font_normal" );
			scrolling_font_bold_file = getLocaleGuiString( "gui_loadingwindow_scrollingtext_font_bold" ); 
			scrolling_font_italic_file	= getLocaleGuiString( "gui_loadingwindow_scrollingtext_font_italic" );
			scrolling_font_h1_file = getLocaleGuiString( "gui_loadingwindow_scrollingtext_font_h1" );

			scrollingText = new OguiFormattedText( win, ogui, x, y, w, h, 0 );

			scrollingText->setFont( ogui->LoadFont( scrolling_font_file ) );
			scrollingText->registerFont( "b", ogui->LoadFont( scrolling_font_bold_file ) );
			scrollingText->registerFont( "i", ogui->LoadFont( scrolling_font_italic_file ) );
			scrollingText->registerFont( "h1", ogui->LoadFont( scrolling_font_h1_file ) );

			scrollingSpeed = getLocaleGuiInt("gui_loadingwindow_scrollingtext_default_speed", 50 ) * 0.001f;
		}
		if(has_scrolling_text)
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_scrollingfader_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_scrollingfader_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_scrollingfader_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_scrollingfader_h", 0 );
			std::string img = getLocaleGuiString( "gui_loadingwindow_scrollingfader_img" );
			scrollingFader = ogui->CreateSimpleWindow( x, y, w, h, img.c_str() );
			scrollingFader->SetUnmovable();
			scrollingFader->SetEffectListener( this );
			scrollingFader->Hide();
		}
#endif

		std::string mission_cinematic_locale = ( mission + "_cinematic" );
		if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI,  mission_cinematic_locale.c_str() ) )
		{
			cinematic =	getLocaleGuiString( mission_cinematic_locale.c_str() );
			cinematicStarted = false;
			cinematicFinished = false;

#ifdef PROJECT_SURVIVOR
			// stupid hack to remove background crap until cinematic over
			background_image = ogui->LoadOguiImage(getLocaleGuiString("gui_loadingwindow_black"));
			win->setBackgroundImage( background_image );
			briefingArea->setText("");
			headerText->SetText("");
			missionText->SetText("");
#endif
		}
	}

	void LoadingWindow::destroyWindows()
	{
#ifdef PROJECT_SURVIVOR
		if(scrollingText)
		{
			scrollingText->deleteRegisteredFonts();
			delete scrollingText->getFont();
			delete scrollingText;
		}
#endif

		delete fontButtonHighlighted;
		delete fontButtonDisabled;
		delete fontButton;

		delete fontNormal;
		delete fontBold;
		delete fontItalic;
		delete fontUnderline;

		delete missionFont;
		delete headerFont;

		delete missionText;
		delete headerText;

		delete upgradeMenuBut;
		delete briefingArea;
		delete loadingbut;
		delete closebut;
		delete win;

#ifdef PROJECT_SURVIVOR
		// close button may be parented to this, so don't delete before
		delete scrollingFader;
#endif

		delete briefVideo;
		delete briefVideoStream;

		delete background_image;
		background_image = NULL;
	}

	void LoadingWindow::reloadWindows()
	{
		destroyWindows();
		createWindows();
	}

	void LoadingWindow::enableClose()
	{
		closeEnabled = true;
		closeEnabledTime = Timer::getTime();

		// stop rendering crap in the background
		game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, false );

		loadingbut->SetDisabled(true);
		if (fontButtonDisabled == NULL) // err.. wtf?
		{
			loadingbut->SetText("");
		}

#ifdef PROJECT_SURVIVOR
		if( !cinematic.empty() || (scrollingFader && !scrollingFader->IsVisible() && !scrollingStarted) )
		{
			// hide briefing
			briefingArea->setText("");
			// hide close button
			if(closebut)
			{
				delete closebut;
				closebut = NULL;
			}
		}
		else
#endif
		{
			closebut->SetDisabled(false);
			if (fontButtonDisabled == NULL) // err.. wtf?
			{
				std::string tmp = getLocaleGuiString( "gui_loadingwindow_closebutton_text" );
				closebut->SetText(tmp.c_str());
			}
		}

		if( upgradeMenuBut )
			upgradeMenuBut->SetDisabled(false);

		// TODO: cursor number
		loadingbut->Focus(0);

#ifndef PROJECT_SURVIVOR
		bool show_upgrade = false;
#else
		bool show_upgrade = false;
		if( game->gameScripting->getGlobalIntVariableValue( "survivor_show_upgrade_button_on_loading_screen" ) == 1 )
			show_upgrade = true;
#endif

		if( show_upgrade )
			createUpgradeButton();
	}

	bool LoadingWindow::isCloseEnabled()
	{
		return closeEnabled;
	}

	void LoadingWindow::raise()
	{
		win->Raise();
		if(scrollingFader) scrollingFader->Raise();
	}

	void LoadingWindow::CursorEvent(OguiButtonEvent *eve)
	{
		if (this->fadingOut)
		{
			// no doubleclicks
			return;
		}
		if (eve->triggerButton->GetId() == LOADINGW_CLOSE
			&& eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			closeWindow();
		}
		else if( eve->triggerButton->GetId() == LOADINGW_UPGRADEMENU
			&& eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK )
		{
			game->gameUI->openWindow( GameUI::WINDOW_TYPE_UPGRADE );
		}
	}

	void LoadingWindow::EffectEvent(OguiEffectEvent *eve)
	{
		if (eve->triggerWindow == win && eve->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
		{
			game->gameUI->closeLoadingWindow(player);
		}
		else if (eve->triggerWindow == scrollingFader && eve->eventType == OguiEffectEvent::EVENT_TYPE_FADEDIN)
		{
			startScrolling();
		}
	}

	bool LoadingWindow::isFadingOut()  
	{
		return fadingOut;
	}

	void LoadingWindow::startScrolling()
	{
		scrollingStarted = true;
		
		std::string mission = game->getMissionId()?game->getMissionId():"";

		// play briefing sound
		{
			speechSoundStartTime = Timer::getTime();
			if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, ( mission_brief2_locale + "_sound_time" ).c_str() ) )
			{
				speechSoundStartTime += getLocaleGuiInt(( mission_brief2_locale + "_sound_time" ).c_str(), 0);
			}
		}

		// play briefing music
		std::string mission_music_locale = ( mission_brief2_locale + "_music" );
		if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI,  mission_music_locale.c_str() ) )
		{
			if(game->gameUI->getSoundMixer())
				game->gameUI->getSoundMixer()->setMusic(getLocaleGuiString( mission_music_locale.c_str() ));
			playedMusic = true;
		}

		// set fonts
		std::string font_normal_locale = ( mission_brief2_locale + "_font_normal" );
		std::string font_bold_locale = ( mission_brief2_locale + "_font_bold" );
		std::string font_italic_locale = ( mission_brief2_locale + "_font_italic" );
		std::string font_h1_locale = ( mission_brief2_locale + "_font_h1" );
		if(  ::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI,  font_normal_locale.c_str(), &scrolling_font_file )
			|| ::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI,  font_bold_locale.c_str(), &scrolling_font_bold_file )
			|| ::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI,  font_italic_locale.c_str(), &scrolling_font_italic_file )
			|| ::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI,  font_h1_locale.c_str(), &scrolling_font_h1_file ))
		{
			scrollingText->deleteRegisteredFonts();
			delete scrollingText->getFont();

			scrollingText->setFont( ogui->LoadFont( scrolling_font_file ) );
			scrollingText->registerFont( "b", ogui->LoadFont( scrolling_font_bold_file ) );
			scrollingText->registerFont( "i", ogui->LoadFont( scrolling_font_italic_file ) );
			scrollingText->registerFont( "h1", ogui->LoadFont( scrolling_font_h1_file ) );
		}

		// set briefing text
		scrollingText->setText( getLocaleGuiString(mission_brief2_locale.c_str()) );

		// set scrolling speed
		std::string scroll_speed_locale = mission_brief2_locale + "_speed";
		if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, scroll_speed_locale.c_str() ))
		{
			scrollingSpeed = getLocaleGuiInt(scroll_speed_locale.c_str(), 50) * 0.001f;
		}

		// set scrolling limit
		scrollingLimit = INT_MIN;
		std::string scrolling_limit_locale = mission_brief2_locale + "_scroll_limit";
		if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, scrolling_limit_locale.c_str() ))
		{
			scrollingLimit = getLocaleGuiInt(scrolling_limit_locale.c_str(), INT_MIN);
		}

		// start scrolling from bottom
		totalScrollAmount = getLocaleGuiInt( "gui_loadingwindow_scrollingtext_start_offset", 0 );
		scrollingText->moveBy(0 , totalScrollAmount );

		// recreate close button
		if(closebut == NULL)
		{
			int x = getLocaleGuiInt( "gui_loadingwindow_closebutton_x", 0 ) - scrollingFader->GetPositionX();
			int y = getLocaleGuiInt( "gui_loadingwindow_closebutton_y", 0 ) - scrollingFader->GetPositionY();
			int w = getLocaleGuiInt( "gui_loadingwindow_closebutton_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_closebutton_h", 0 );

			std::string close_normal = getLocaleGuiString( "gui_loadingwindow_closebutton_normal" );
			std::string close_down	 = getLocaleGuiString( "gui_loadingwindow_closebutton_down" );
			std::string close_high	 = getLocaleGuiString( "gui_loadingwindow_closebutton_high" );
			std::string close_text	 = getLocaleGuiString( "gui_loadingwindow_closebutton_text" );

			closebut = ogui->CreateSimpleTextButton( scrollingFader, x, y, w, h, 
				close_normal.empty()?NULL:close_normal.c_str(), 
				close_down.empty()?NULL:close_down.c_str(),
				close_high.empty()?NULL:close_high.c_str(), close_text.c_str(), LOADINGW_CLOSE );
			closebut->SetListener(this);
			closebut->SetFont(fontButton);
			closebut->SetDisabledFont(fontButtonDisabled);
			closebut->SetHighlightedFont( fontButtonHighlighted );
		}

		// focus on it so pressing joystick button starts the game
		// without having to move cursor
		closebut->Focus(0);
	}

	void LoadingWindow::update()
	{
		if(!closeEnabled)
			return;

		int delta_time = Timer::getTime() - lastUpdateTime;
		lastUpdateTime = Timer::getTime();

		// wait until framerate is stabilized (more than 20 fps), or 3 secs have passed
		if( !updateStarted && Timer::getTime() - closeEnabledTime < 3000 && delta_time > 50 )
		{
			return;
		}
		updateStarted = true;

		// cinematic playback
		//
		if(!cinematic.empty())
		{
			// start cinematic
			if(!cinematicStarted)
			{
				cinematicStarted = true;
				game->gameUI->openCinematicScreen( cinematic );
			}
			// cinematic ended
			else if(!game->gameUI->isCinematicScreenOpen() && !cinematicFinished)
			{
				cinematicFinished = true;
				if( background_image )
				{
					// show background
					IOguiImage *bg_old = background_image;
					std::string mission = game->getMissionId()?game->getMissionId():"";
					background_image = ogui->LoadOguiImage(getLocaleGuiString( ( "gui_loadingwindow_" + mission + "_background" ).c_str() ));
					win->setBackgroundImage( background_image );
					delete bg_old;
					std::string mission_brief = getLocaleGuiString( ( mission + "_brief" ).c_str() );
					briefingArea->setText( mission_brief );
					std::string header_text = getLocaleGuiString( ( mission + "_header_text" ).c_str() );
					headerText->SetText(header_text.c_str());
					std::string mission_text = getLocaleGuiString( "gui_loadingwindow_missiontext_text" );
					missionText->SetText(mission_text.c_str());
				}
				if( scrollingFader && !scrollingFader->IsVisible() && !scrollingStarted )
				{
					// fade in the fader
					scrollingFader->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, 1000);
					scrollingFader->Show();
					scrollingFader->SetTransparency(100);
				}
				else if(!scrollingFader)
				{
					// play briefing music
					std::string mission_music_locale = ( mission_brief2_locale + "_music" );
					if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI,  mission_music_locale.c_str() ) )
					{
						if(game->gameUI->getSoundMixer())
							game->gameUI->getSoundMixer()->setMusic(getLocaleGuiString( mission_music_locale.c_str() ));
						playedMusic = true;
					}
					// recreate close button
					if(closebut == NULL)
					{
						int x = getLocaleGuiInt( "gui_loadingwindow_closebutton_x", 0 );
						int y = getLocaleGuiInt( "gui_loadingwindow_closebutton_y", 0 );
						int w = getLocaleGuiInt( "gui_loadingwindow_closebutton_w", 0 );
						int h = getLocaleGuiInt( "gui_loadingwindow_closebutton_h", 0 );

						std::string close_normal = getLocaleGuiString( "gui_loadingwindow_closebutton_normal" );
						std::string close_down	 = getLocaleGuiString( "gui_loadingwindow_closebutton_down" );
						std::string close_high	 = getLocaleGuiString( "gui_loadingwindow_closebutton_high" );
						std::string close_text	 = getLocaleGuiString( "gui_loadingwindow_closebutton_text" );

						closebut = ogui->CreateSimpleTextButton( win, x, y, w, h, 
							close_normal.empty()?NULL:close_normal.c_str(), 
							close_down.empty()?NULL:close_down.c_str(),
							close_high.empty()?NULL:close_high.c_str(), close_text.c_str(), LOADINGW_CLOSE );
						closebut->SetListener(this);
						closebut->SetFont(fontButton);
						closebut->SetDisabledFont(fontButtonDisabled);
						closebut->SetHighlightedFont( fontButtonHighlighted );
					}
				}
			}
		}
		// no cinematic
		else
		{
			// start scrolling text
			if( scrollingFader && !scrollingFader->IsVisible() && !scrollingStarted )
			{
				// fade in the fader
				scrollingFader->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, 1000);
				scrollingFader->Show();
				scrollingFader->SetTransparency(100);
			}
		}

		bool playing_cinematic = game->gameUI->isCinematicScreenOpen() || (!cinematic.empty() && !cinematicStarted);

		// video playback
		//
		if(!playing_cinematic && !videoStarted)
		{
			videoStarted = true;

			// play video
			std::string mission = game->getMissionId()?game->getMissionId():"";
			std::string video_locale = ( mission + "_video" );
			const char *video = NULL;
			if( ::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI,  video_locale.c_str(), &video ) )
			{
				if(SimpleOptions::getBool( DH_OPT_B_VIDEO_ENABLED ))
				{
					sfx::SoundMixer *mixer = game->gameUI->getSoundMixer();
					IStorm3D_StreamBuilder *builder = 0;
					if(mixer)
						builder = mixer->getStreamBuilder();

					bool loop = getLocaleGuiInt( ( mission + "_video_loop_enabled" ).c_str(), 0) == 0 ? false : true;
					briefVideoStream = game->gameScene->getStorm3D()->CreateVideoStreamer( video, builder, loop );
					briefVideo = ogui->ConvertVideoToImage( briefVideoStream, builder );
					win->setBackgroundImage(briefVideo);
				}
			}

			std::string mission_music_locale = ( mission + "_music" );
			if( ::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI,  mission_music_locale.c_str() ) )
			{
				if(game->gameUI->getSoundMixer())
					game->gameUI->getSoundMixer()->setMusic(getLocaleGuiString( mission_music_locale.c_str() ));
				playedMusic = true;
			}
		}

		// scrolling
		//
		if( scrollingStarted && !fadingOut )
		{
			if(lastScrollTime == 0)
			{
				// reset timers
				lastScrollTime = Timer::getTime();
				scrollTimeDeltaAvg = 0;
			}

			int timeDelta = Timer::getTime() - lastScrollTime;
			lastScrollTime = Timer::getTime();

			// since FPS should be pretty constant, we should use a smoothed
			// average of the time delta to avoid ugly jumps
			scrollTimeDeltaAvg = timeDelta * 0.1f + scrollTimeDeltaAvg * 0.9f;

			// accumulate scrolling
			nextScrollAmount += scrollingSpeed * scrollTimeDeltaAvg;

			// only scroll if we have accumulated at least one pixel's length
			if(nextScrollAmount >= 1.0f)
			{
				bool clip_at_top = scrollingLimit < 0;
				scrollingText->moveBy(0, (int)(-nextScrollAmount), clip_at_top, false);
				totalScrollAmount += (int)(-nextScrollAmount);
				nextScrollAmount = 0;

				if(totalScrollAmount <= scrollingLimit)
				{
					// stop scrolling
					scrollingStarted = false;
				}
			}

			if(speechSoundStartTime != -1 && Timer::getTime() > speechSoundStartTime)
			{
				std::string mission_sound = getLocaleGuiString( ( mission_brief2_locale + "_sound" ).c_str() );
				VC3 pos = game->gameUI->getListenerPosition();
				speechSound = game->gameUI->playSpeech(convertLocaleSpeechString(mission_sound.c_str()), pos.x, pos.y, pos.z, false, DEFAULT_SPEECH_VOLUME, false);
				speechSoundStartTime = -1;
			}
		}
	}

	void LoadingWindow::closeWindow()
	{
		if( fadingOut == false )
		{
			if(speechSound >= 0)
			{
				game->gameUI->stopSound(speechSound);
				speechSound = -1;
			}

			if(playedMusic)
			{
				if(game->gameUI->getSoundMixer())
					game->gameUI->getSoundMixer()->setMusic(NULL);
				playedMusic = false;
			}

			// remove video from bg
			if(briefVideo)
			{
				win->setBackgroundImage(NULL);
			}

			//game->gameUI->closeLoadingWindow(player);

#ifdef PROJECT_SURVIVOR
			if(game->gameUI->getEffects())
			{
				game->gameUI->getEffects()->setDefaultFadeImageIfHitImage();
				game->gameUI->getEffects()->startFadeIn(1000);
				win->Raise();
				if(scrollingFader) scrollingFader->Raise();
			}
#endif

			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, 500);
			if(scrollingFader) scrollingFader->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, 500);
			this->fadingOut = true;

			if(!LoadingWindow::showCharacterSelection && !LoadingWindow::showUpgradeWindowOnClose)
			{
				game->setPaused(false);
			}

			game->gameUI->setGUIVisibility(game->singlePlayerNumber, true);
			autoCloseEnabled = false;

			// open character selection window
			if(showCharacterSelection)
			{
				game->gameUI->openCharacterSelectionWindow(NULL);
				showCharacterSelection = false;
			}
			else if(LoadingWindow::showUpgradeWindowOnClose)
			{
				// open the first upgrade window
				game->gameUI->openWindow( GameUI::WINDOW_TYPE_UPGRADE );
			}


			// HAX HAX HAX!
			if (SimpleOptions::getBool(DH_OPT_B_RESET_RENDERER_WHEN_LOADED))
			{
				game->gameScene->getStorm3D()->forceReset();
			}

			// resume rendering crap in the background
			game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::RenderTargets, true );

		}
	}

	void LoadingWindow::createUpgradeButton()
	{
		// upgrade button thingie
		{
			
			int x = getLocaleGuiInt( "gui_loadingwindow_upgrademenubutton_x", 0 );
			int y = getLocaleGuiInt( "gui_loadingwindow_upgrademenubutton_y", 0 );
			int w = getLocaleGuiInt( "gui_loadingwindow_upgrademenubutton_w", 0 );
			int h = getLocaleGuiInt( "gui_loadingwindow_upgrademenubutton_h", 0 );

			std::string normal = getLocaleGuiString( "gui_loadingwindow_upgrademenubutton_normal" );
			std::string down	 = getLocaleGuiString( "gui_loadingwindow_upgrademenubutton_down" );
			std::string high	 = getLocaleGuiString( "gui_loadingwindow_upgrademenubutton_high" );
			std::string upgradetext = getLocaleGuiString( "gui_loadingwindow_upgrademenubutton_text" );

			// y = y - 35;
			// upgradetext = "Upgrade menu";

			upgradeMenuBut = ogui->CreateSimpleTextButton( win, x, y, w, h, 
				normal.empty()?NULL:normal.c_str(), 
				down.empty()?NULL:down.c_str(),
				high.empty()?NULL:high.c_str(), upgradetext.c_str(), LOADINGW_UPGRADEMENU );

			
			upgradeMenuBut->SetListener(this);
			upgradeMenuBut->SetFont(fontButton);
			upgradeMenuBut->SetDisabledFont(fontButtonDisabled);
			upgradeMenuBut->SetHighlightedFont( fontButtonHighlighted );
			upgradeMenuBut->SetDisabled(true);
		}
	}

	bool LoadingWindow::shouldAutoClose()
	{
		return autoCloseEnabled && !closebut->isDisabled();
	}
} // end of namespace game

