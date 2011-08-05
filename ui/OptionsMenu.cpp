
#include "precompiled.h"

#include "OptionsMenu.h"

#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../ui/GameCamera.h"
#include "../game/GameProfiles.h"
#include "MenuCollection.h"
#include "../game/DHLocaleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_sounds.h"
#include "../game/scripting/GameScripting.h"
#include "../ogui/OguiSlider.h"
#include "../sound/SoundMixer.h"
#include "../game/OptionApplier.h"
#include "../ogui/OguiCheckBox.h"
#include "../game/options/options_all.h"
#include "../storm/keyb3/keyb3.h"
#include "CombatWindow.h" // for getNumberOfPlayers()

#include "../util/Debug_MemoryManager.h"

#include <boost/lexical_cast.hpp>
#include <sstream>
#include <fstream>
#include <assert.h>

#include "../game/userdata.h"

using namespace game;

extern int JoyNum;

namespace ui
{

///////////////////////////////////////////////////////////////////////////////
namespace {
	float getCameraRotateRate()
	{
		float result = 0;
		result = SimpleOptions::getFloat( DH_OPT_F_CAMERA_ROTATION_STRENGTH );
		result /= 3.0f;
		// result *= SimpleOptions::getFloat( DH_OPT_F_CAMERA_ROTATION_SPRING );
		// DH_OPT_F_CAMERA_ROTATION_STRENGTH 207
		// DH_OPT_F_CAMERA_ROTATION_SAFE 208
		// DH_OPT_F_CAMERA_ROTATION_SPRING 209
		return result;
	}

	void setCameraRotateRate( float f )
	{
		SimpleOptions::setFloat( DH_OPT_F_CAMERA_ROTATION_STRENGTH, f * 3.0f );
	}

	float getMouseSpring()
	{
		float result = SimpleOptions::getFloat( DH_OPT_F_CAMERA_ROTATION_SPRING );		
		result /= 2.0f;

		return result;
	}

	void setMouseSpring( float f )
	{
		SimpleOptions::setFloat( DH_OPT_F_CAMERA_ROTATION_SPRING, f * 2.0f );
	}


	// returns gamma between 0 and 1
	float getGamma()
	{
		return ( SimpleOptions::getFloat( DH_OPT_F_GAMMA ) / 1.5f - 0.5f );
	}

	// applys the gamma between 0 and 1
	void setGamma( float value )
	{
		SimpleOptions::setFloat( DH_OPT_F_GAMMA, ( value + 0.5f ) * 1.5f );
		::apply_options_request = true;
	}

	static bool camera_lock_y_axis = false;
	static int options_max = 0;

	std::string getKeyNameFromLocales( const std::string& locale_name )
	{
		std::string str = "keycode_name_" + locale_name;

		if( DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, str.c_str() ) )
		{
			return DHLocaleManager::getInstance()->getString( DHLocaleManager::BANK_GUI, str.c_str() );
		}
		else
		{
			std::fstream f( "missing_keycode_locales.txt", std::ios::app );
			f << str << " = " << locale_name << std::endl;
			f.close();
		}

		return locale_name;
	}

}

///////////////////////////////////////////////////////////////////////////////



int	OptionsMenu::cooperativeSelection = 0;
bool OptionsMenu::returnToCoopMenu = false;

///////////////////////////////////////////////////////////////////////////////

OptionsMenu::OptionsMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
  MenuBaseImpl( NULL ),
  controlNumArray(),
  keycodeArray(),
  controlDescriptions(),
  mouseEventCaptureWindow( NULL ),

  controlUpdate( -1 ),
  
  sliderButtons(),
  textLabels(),

  gameController( NULL ),

  difficultButtons(),
  difficultActiveSelection( -1 ),
  difficultImageSelectDown( NULL ),
  difficultImageSelectNorm( NULL )	,
  
  sliderMusicValue( 0.75f ),
  sliderSoundValue( 0.00f ),
  sliderSpeechValue( 0.00f ),
	sliderAmbientValue( 1.0f ),

  sliderSoundPlayfile( "" ),
  sliderSoundLoopTime( 0 ),
  sliderSoundPlayNow( false ),

  sliderSpeechPlayfile( "" ),
  sliderSpeechLoopTime( 0 ),
  sliderSpeechPlayNow( false ),

  lastPlayTime( 0 ),

  joystickBigText( NULL ),
  joystickMenuOpen( false ),
  joystickUpdate( -1 ),
  currentController( -1 ),

  cooperativeBigText( NULL ),
  cooperativeProfileList( NULL ),
  styles(),
  selectListStyle( NULL ),

  menuCollection( menu ),
  fonts( fonts ),
  discartNextCursorEvent( false ),
  fromGame( false ),

  cameraModeHack( NULL ),
  cameraLockYAxis( NULL ),
  cameraModeTextHack( NULL ),
  
  cameraRotateStrength( NULL ),
  cameraSpringStrength( NULL ),
  cameraRotateSpeed( getCameraRotateRate() ),
  mouseSpring( getMouseSpring() ),

	controllerTypeButton( NULL ),
	controllerTypeListCaptureEvents( NULL ),
	controllerTypeList( NULL ),
	controllerTypeListStyle( NULL ),

  currentProfile( "" )
{
	assert( o_gui );
	assert( menu );
	assert( fonts );
	assert( g );

	game = g;
	
	ogui = o_gui;
	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_optionsmenu_window_x", 0 ), getLocaleGuiInt( "gui_optionsmenu_window_y", 0 ), getLocaleGuiInt( "gui_optionsmenu_window_w", 1024 ), getLocaleGuiInt( "gui_optionsmenu_window_h", 768 ), NULL );
	win->Hide();
	win->SetUnmovable();

	menu->setBackgroundImage( getLocaleGuiString( "gui_optionsmenu_background_image" ) );

	fontSelectNorm	= ogui->LoadFont( buttonFontSelectNormal.c_str() );
	fontSelectDown	= ogui->LoadFont( buttonFontSelectDown.c_str() );
	fontDescNorm	= ogui->LoadFont( buttonFontDescNormal.c_str() );
	fontDescDown	= ogui->LoadFont( buttonFontDescDown.c_str() );


	// choose valid coop profile
	{
		int first_valid = -1;
		bool selection_valid = false;
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++ )
		{
			std::string none = getLocaleGuiString( "gui_newgame_text_none" );
			if(game->getGameProfiles()->getCurrentProfile( i ) == none)
				continue;

			if(first_valid == -1)
				first_valid = i;

			if(i == cooperativeSelection)
				selection_valid = true;
		}
		if(!selection_valid)
		{
			cooperativeSelection = first_valid;
		}
	}

	gameController = game->getGameUI()->getController( cooperativeSelection );
	assert( gameController );

	

	imageSelectNorm = ogui->LoadOguiImage( buttonNormal.c_str() );
	imageSelectDown = ogui->LoadOguiImage( buttonDown.c_str() );
	

	// Main menu buttons
	buttonX	= getLocaleGuiInt( "gui_optionsmenu_button_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_optionsmenu_button_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_optionsmenu_button_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_optionsmenu_button_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

	buttonAddX	= getLocaleGuiInt( "gui_optionsmenu_button_add_x", getLocaleGuiInt( "gui_menu_common_button_add_x", 0 ) );
	buttonAddY	= getLocaleGuiInt( "gui_optionsmenu_button_add_y", getLocaleGuiInt( "gui_menu_common_button_add_y", 28 ) );
	// addHeaderText( getLocaleGuiString( "gui_om_header" ), fonts->big.normal );

	currentProfile = game->getGameProfiles()->getCurrentProfile( cooperativeSelection );
	
	///////////////////////////////////////////////////////////////////////////

	// the new read-from-file solution...
	options_max = 0;

	util::SimpleParser sp;
#ifdef LEGACY_FILES
	bool loadSuccess = sp.loadFile("Data/Misc/options_menu_keybinds.txt");
#else
	bool loadSuccess = sp.loadFile("data/misc/options_menu_keybinds.txt");
#endif
	if (loadSuccess)
	{
		while (sp.next())
		{
			char *tmp = sp.getLine();
			// NOTE: always using player 1 controller...
			if (tmp != NULL && tmp[0] != '\0')
			{
				int ctrlNum = game->gameUI->getController(0)->getControlNumberForName(tmp);
				if (ctrlNum != -1)
				{
					options_max++;
					// TODO: optimize, this is not very effective, lots of unnecessary resizes happening...
					// (should alloc appropriate size before the loop)
					controlNumArray.resize( options_max + 1 );
					keycodeArray.resize( options_max + 1 );
					controlDescriptions.resize( options_max + 1 );					
					controlNumArray[options_max] = ctrlNum;
				} else {
					LOG_ERROR_W_DEBUG("OptionsMenu - Options menu keybinds configuration file contained an unknown control name.", tmp);
				}
			}
		}
	}

	/*
	// the old hard-coded solution...
	controlNumArray.resize( options_max + 1 );
	keycodeArray.resize( options_max + 1 );
	controlDescriptions.resize( options_max + 1 );					

	controlNumArray[ 1 ] = DH_CTRL_ATTACK;
	controlNumArray[ 2 ] = DH_CTRL_ATTACK_SECONDARY;
	controlNumArray[ 3 ] = DH_CTRL_RELOAD;
	controlNumArray[ 4 ] = DH_CTRL_CHANGE_NEXT_WEAPON;
	controlNumArray[ 5 ] = DH_CTRL_CHANGE_PREV_WEAPON;
	// controlNumArray[ 6 ] = DH_CTRL_USE_MEDIKIT;
	controlNumArray[ 6 ] = DH_CTRL_FLASHLIGHT;
	controlNumArray[ 7 ] = DH_CTRL_EXECUTE;
	controlNumArray[ 8 ] = DH_CTRL_CAMERA_MOVE_FORWARD;
	controlNumArray[ 9 ] = DH_CTRL_CAMERA_MOVE_BACKWARD;
	controlNumArray[10 ] = DH_CTRL_CAMERA_MOVE_LEFT;
	controlNumArray[11 ] = DH_CTRL_CAMERA_MOVE_RIGHT;
	controlNumArray[12 ] = DH_CTRL_SPECIAL_MOVE;	
	controlNumArray[13 ] = DH_CTRL_OPEN_MAP;	
	controlNumArray[14 ] = DH_CTRL_OPEN_UPGRADE;
	controlNumArray[15 ] = DH_CTRL_OPEN_LOG;
	controlNumArray[16 ] = DH_CTRL_CAMERA_MOVE_ROTATE_LEFT;
	controlNumArray[17 ] = DH_CTRL_CAMERA_MOVE_ROTATE_RIGHT;
	controlNumArray[18 ] = DH_CTRL_CAMERA_LOOK_MODE;
	*/
	

	int i;
	for ( i = 1; i <= options_max; i++ )
	{
		keycodeArray[ i ] = gameController->getBoundKey( controlNumArray[ i ], 0 );
	}

	///////////////////////////////////////////////////////////////////////////

	// The big text's in the options menu
	{
		std::string textSoundSettings =		getLocaleGuiString( "gui_optionsmenu_text_soundsettings" );
		int			textBigX =				getLocaleGuiInt( "gui_optionsmenu_text_soundsettings_x", 0 );
		int			textSoundSettingsY =	getLocaleGuiInt( "gui_optionsmenu_text_soundsettings_y", 0 );
		int			textBigW =				getLocaleGuiInt( "gui_optionsmenu_text_soundsettings_w", 0 );
		int			textBigH =				getLocaleGuiInt( "gui_optionsmenu_text_soundsettings_h", 0 );

		addText( textSoundSettings, textBigX, textSoundSettingsY, textBigW, textBigH, fonts->big.normal );

#ifdef PROJECT_SURVIVOR
		if(!game->isCooperative())
#endif
		{
			std::string	textDifficultyLevel = getLocaleGuiString( "gui_optionsmenu_text_difficultylevel" );
			
			textBigX					= getLocaleGuiInt( "gui_optionsmenu_text_difficultylevel_x", 0 );
			int	textDifficultyLevelY	= getLocaleGuiInt( "gui_optionsmenu_text_difficultylevel_y", 0 );
			textBigW					= getLocaleGuiInt( "gui_optionsmenu_text_difficultylevel_w", 0 );
			textBigH					= getLocaleGuiInt( "gui_optionsmenu_text_difficultylevel_h", 0 );

			addText( textDifficultyLevel, textBigX, textDifficultyLevelY, textBigW, textBigH, fonts->big.normal );
		}

		std::string	textControls = getLocaleGuiString( "gui_optionsmenu_text_controls" );
		
		textBigX			= getLocaleGuiInt( "gui_optionsmenu_text_controls_x", 0 );
		int	textControlsY	= getLocaleGuiInt( "gui_optionsmenu_text_controls_y", 0 );
		textBigW			= getLocaleGuiInt( "gui_optionsmenu_text_controls_w", 0 );
		textBigH			= getLocaleGuiInt( "gui_optionsmenu_text_controls_h", 0 );


		addText( textControls, textBigX, textControlsY, textBigW, textBigH, fonts->big.normal );

		std::string	textGraphics = getLocaleGuiString( "gui_optionsmenu_text_graphics" );
		
		textBigX			= getLocaleGuiInt( "gui_optionsmenu_text_graphics_x", 0 );
		int	textGraphicsY	= getLocaleGuiInt( "gui_optionsmenu_text_graphics_y", 0 );
		textBigW			= getLocaleGuiInt( "gui_optionsmenu_text_graphics_w", 0 );
		textBigH			= getLocaleGuiInt( "gui_optionsmenu_text_graphics_h", 0 );


		addText( textGraphics, textBigX, textGraphicsY, textBigW, textBigH, fonts->big.normal );


	}

	///////////////////////////////////////////////////////////////////////////
	// Slider buttons
	{
		int sliderButtonX = getLocaleGuiInt( "gui_optionsmenu_slider_button_x", 0 );
		int sliderButtonY = getLocaleGuiInt( "gui_optionsmenu_slider_button_y", 0 );
		int sliderButtonW = getLocaleGuiInt( "gui_optionsmenu_slider_button_w", 0 );
		int sliderButtonH = getLocaleGuiInt( "gui_optionsmenu_slider_button_h", 0 );
		
		int sliderClipX = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_x", 0 );
		int sliderClipY = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_y", 0 );
		int sliderClipW = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_w", 0 );
		int sliderClipH = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_h", 0 );

		int sliderButtonAddX = getLocaleGuiInt( "gui_optionsmenu_slider_button_add_x", 0 );
		int sliderButtonAddY = getLocaleGuiInt( "gui_optionsmenu_slider_button_add_y", 0 );
		int sliderTextX		 = getLocaleGuiInt( "gui_optionsmenu_slider_text_x", 0 );
		int sliderTextY		 = getLocaleGuiInt( "gui_optionsmenu_slider_text_y", 0 );
		int	sliderTextW		 = getLocaleGuiInt( "gui_optionsmenu_slider_text_w", 0 );
		int sliderTextH		 = getLocaleGuiInt( "gui_optionsmenu_slider_text_h", 0 );
		int	sliderTextAddX	 = getLocaleGuiInt( "gui_optionsmenu_slider_text_add_x", 0 );
		int sliderTextAddY   = getLocaleGuiInt( "gui_optionsmenu_slider_text_add_y", 0 );

		std::string sliderButtonBackgroundNormal =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_normal" );
		std::string sliderButtonBackgroundDown =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_down" );
		std::string sliderButtonBackgroundHigh =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_high" );

		std::string sliderButtonForegroundNormal =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_normal" );
		std::string sliderButtonForegroundDown =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_down" );
		std::string sliderButtonForegroundHigh =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_high" );

		std::string textMusic = getLocaleGuiString( "gui_optionsmenu_text_music" );
		std::string textSoundFx = getLocaleGuiString( "gui_optionsmenu_text_sound_fx" );
		std::string textSpeech = getLocaleGuiString( "gui_optionsmenu_text_speech" );

		int musicVolume = SimpleOptions::getInt( DH_OPT_I_MUSIC_VOLUME );
		int soundVolume = SimpleOptions::getInt( DH_OPT_I_FX_VOLUME );
		int speechVolume = SimpleOptions::getInt( DH_OPT_I_SPEECH_VOLUME );
		int ambientVolume = SimpleOptions::getInt( DH_OPT_I_AMBIENT_VOLUME );

		sliderMusicValue = (float)musicVolume / 100.0f;
		sliderSoundValue = (float)soundVolume / 100.0f;
		sliderSpeechValue = (float)speechVolume / 100.0f;
		sliderAmbientValue = (float)ambientVolume / 100.0f;

		OguiSlider* slider;

		slider = new OguiSlider( win, ogui, sliderButtonX, sliderButtonY, sliderButtonW, sliderButtonH, 
					sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
					sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
					COMMANDS_SLIDERMUSIC, sliderMusicValue );

		addText( textMusic, sliderTextX, sliderTextY, sliderTextW, sliderTextH, fonts->medium.highlighted ); 

		slider->setBarPosition( sliderButtonX + sliderClipX, sliderButtonY + sliderClipY, sliderClipW, sliderClipH );
		slider->setListener( this );

		sliderButtons.push_back( slider );

		sliderButtonX += sliderButtonAddX;
		sliderButtonY += sliderButtonAddY;
		sliderTextX += sliderTextAddX;
		sliderTextY += sliderTextAddY;


		slider = new OguiSlider( win, ogui, sliderButtonX, sliderButtonY, sliderButtonW, sliderButtonH, 
					sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
					sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
					COMMANDS_SLIDERSOUND, sliderSoundValue );

		addText( textSoundFx, sliderTextX, sliderTextY, sliderTextW, sliderTextH, fonts->medium.highlighted ); 

		slider->setBarPosition( sliderButtonX + sliderClipX, sliderButtonY + sliderClipY, sliderClipW, sliderClipH );
		slider->setListener( this );
		sliderButtons.push_back( slider );

		sliderButtonX += sliderButtonAddX;
		sliderButtonY += sliderButtonAddY;
		sliderTextX += sliderTextAddX;
		sliderTextY += sliderTextAddY;

		slider = new OguiSlider( win, ogui, sliderButtonX, sliderButtonY, sliderButtonW, sliderButtonH, 
					sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
					sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
					COMMANDS_SLIDERSPEECH, sliderSpeechValue );

		addText( textSpeech, sliderTextX, sliderTextY, sliderTextW, sliderTextH, fonts->medium.highlighted ); 

		slider->setBarPosition( sliderButtonX + sliderClipX, sliderButtonY + sliderClipY, sliderClipW, sliderClipH );
		slider->setListener( this );
		sliderButtons.push_back( slider );

		sliderButtonX += sliderButtonAddX;
		sliderButtonY += sliderButtonAddY;
		sliderTextX += sliderTextAddX;
		sliderTextY += sliderTextAddY;


		// create ambient slider
		//
		ambientOptionAvailable = DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, "gui_optionsmenu_text_ambient" );
		if( ambientOptionAvailable )
		{
			std::string textAmbient = getLocaleGuiString( "gui_optionsmenu_text_ambient" );

			slider = new OguiSlider( win, ogui, sliderButtonX, sliderButtonY, sliderButtonW, sliderButtonH, 
						sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
						sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
						COMMANDS_SLIDERAMBIENT, sliderAmbientValue );

			addText( textAmbient, sliderTextX, sliderTextY, sliderTextW, sliderTextH, fonts->medium.highlighted ); 

			slider->setBarPosition( sliderButtonX + sliderClipX, sliderButtonY + sliderClipY, sliderClipW, sliderClipH );
			slider->setListener( this );
			sliderButtons.push_back( slider );

			sliderButtonX += sliderButtonAddX;
			sliderButtonY += sliderButtonAddY;
			sliderTextX += sliderTextAddX;
			sliderTextY += sliderTextAddY;
		}

	}

	///////////////////////////////////////////////////////////////////////////
	// Difficulty buttons
	/*{
		int difficultButtonX = getLocaleGuiInt( "gui_optionsmenu_difficult_button_x", 0 );
		int difficultButtonY = getLocaleGuiInt( "gui_optionsmenu_difficult_button_y", 0 );
		int difficultButtonW = getLocaleGuiInt( "gui_optionsmenu_difficult_button_w", 0 );
		int difficultButtonH = getLocaleGuiInt( "gui_optionsmenu_difficult_button_h", 0 );
		int difficultButtonAddX = getLocaleGuiInt( "gui_optionsmenu_difficult_button_add_x", 0 );
		int difficultButtonAddY = getLocaleGuiInt( "gui_optionsmenu_difficult_button_add_y", 0 );

		std::string optionsDifficultButtonNormal =	getLocaleGuiString( "gui_optionsmenu_difficult_image_normal" );
		std::string optionsDifficultButtonDown =	getLocaleGuiString( "gui_optionsmenu_difficult_image_down" );
		std::string optionsDifficultButtonHigh =	getLocaleGuiString( "gui_optionsmenu_difficult_image_high" );
		
		std::string optionsEasyText =	getLocaleGuiString( "gui_optionsmenu_text_easy" );
		std::string optionsNormalText = getLocaleGuiString( "gui_optionsmenu_text_normal" );
		std::string optionsHardText =	getLocaleGuiString( "gui_optionsmenu_text_hard" );

		difficultImageSelectDown = o_gui->LoadOguiImage( optionsDifficultButtonHigh.c_str() );
		difficultImageSelectNorm = o_gui->LoadOguiImage( optionsDifficultButtonNormal.c_str() );

		addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
			optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
			fonts->little.normal, optionsEasyText, COMMANDS_EASY );

		difficultButtonX += difficultButtonAddX;
		difficultButtonY += difficultButtonAddY;

		addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
			optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
			fonts->little.normal, optionsNormalText, COMMANDS_NORMAL );
		
		difficultButtonX += difficultButtonAddX;
		difficultButtonY += difficultButtonAddY;

		addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
			optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
			fonts->little.normal, optionsHardText, COMMANDS_HARD );

		int diff = game->gameScripting->getGlobalIntVariableValue( "general_difficulty_level" );
		if( diff < 50 )						selectDifficultButton( COMMANDS_EASY );
		else if ( diff >= 50 && diff < 75 )	selectDifficultButton( COMMANDS_NORMAL );
		else if ( diff >= 75 )				selectDifficultButton( COMMANDS_HARD );

	}*/
	///////////////////////////////////////////////////////////////////////////

	{
		buttonDescriptionW = getLocaleGuiInt( "gui_optionsmenu_desc_w", 0 );
		buttonDescriptionH = getLocaleGuiInt( "gui_optionsmenu_desc_h", 0 );

		int key_name_add_x = getLocaleGuiInt( "gui_optionsmenu_keydesc_add_x", 0 );
		int i;
		for( i = 1; i <= options_max; i++ )
		{
			SelectionButtonDescs* descs = new SelectionButtonDescs;

			addControlDescription( getOptionsKeyName( i ), i, key_name_add_x, 0, fonts->little.normal, fonts->little.highlighted, fonts->little.down, fonts->little.disabled );
			descs->first = controlDescriptions[ i ];
			addSelectionButton( getOptionsButtonName( i ), i, fonts->medium.normal, descs );
			// selectButtons[ i ]->SetEventMask( OGUI_EMASK_PRESS );,
			selectionButtonDescs.push_back( descs );
		}

	}
	
	///////////////////////////////////////////////////////////////////////////
	
	// These should be loaded from the locales, and they are. Yay.

	sliderSoundPlayfile = getLocaleGuiString( "gui_optionsmenu_sound_file" );
	sliderSoundLoopTime = getLocaleGuiInt( "gui_optionsmenu_sound_looptime", 0 );
	sliderSoundPlayNow = false;

	sliderSpeechPlayfile = getLocaleGuiString( "gui_optionsmenu_speech_file" );
	sliderSpeechLoopTime = getLocaleGuiInt( "gui_optionsmenu_speech_looptime", 0 );
	sliderSpeechPlayNow = false;

	///////////////////////////////////////////////////////////////////////////

	if( game->inCombat )
	{
		closeMenuByEsc = false;
	}
	else
	{
		closeMenuByEsc = true;
		editHandle = game->gameUI->getController(0)->addKeyreader( this );
		debugKeyreader( editHandle, false, "OptionsMenu::OptionsMenu()" );
	}

	///////////////////////////////////////////////////////////////////////////

	bool value = game->gameUI->getGameCamera()->getGameCameraYAxisLock();

	camera_lock_y_axis = value;
	
	// addSmallButton( getLocaleGuiString( "gui_om_closeme" ), COMMANDS_CLOSEME, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );
	addSeparator();
#ifdef PROJECT_SURVIVOR
	buttonX = getLocaleGuiInt( "gui_optionsmenu_defaults_x", 0 );
	buttonY = getLocaleGuiInt( "gui_optionsmenu_defaults_y", 0 );
#endif
	/*OguiButton* b = */addButton( getLocaleGuiString( "gui_om_defaults" ), COMMANDS_DEFAULTS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );

	if( getNumberOfPlayers() == 1 )
	{

		// Camera mode temp hack for demo
		cameraModeHack = new OguiCheckBox( win, ogui, 
			getLocaleGuiInt( "gui_optionsmenu_camerabox_x", 0 ), 
			getLocaleGuiInt( "gui_optionsmenu_camerabox_y", 0 ), 
			getLocaleGuiInt( "gui_optionsmenu_camerabox_w", 0 ), 
			getLocaleGuiInt( "gui_optionsmenu_camerabox_h", 0 ), 	
			getLocaleGuiString( "gui_optionsmenu_camerabox_img_norm" ), "", "", 
			getLocaleGuiString( "gui_optionsmenu_camerabox_img_fill" ) );

		cameraModeHack->setText( ( (std::string)getLocaleGuiString( "demo_camera_name" ) ).c_str(), OguiCheckBox::TEXT_ALIGN_RIGHT, 200, fonts->little.highlighted );

		cameraModeTextHack = NULL;

		cameraModeHack->setValue( !game->gameUI->getGameCamera()->getGameCameraMode() );
		cameraModeHack->setListener( this );

		if( cameraModeHack->getValue())
		{
			createMouseButtons();
		}

	}

	// controller type
	{
		int x = getLocaleGuiInt( "gui_optionsmenu_controllertype_x", 0 );
		int y = getLocaleGuiInt( "gui_optionsmenu_controllertype_y", 0 );
		int w = getLocaleGuiInt( "gui_optionsmenu_controllertype_w", 0 );
		int h = getLocaleGuiInt( "gui_optionsmenu_controllertype_h", 0 );
		controllerTypeButton = ogui->CreateSimpleTextButton( win, x, y, w, h, NULL, NULL, NULL, "", COMMANDS_CONTROLLERTYPE, 0, false );
		controllerTypeButton->SetFont( fonts->medium.normal );
		controllerTypeButton->SetDownFont( fonts->medium.down );
		controllerTypeButton->SetHighlightedFont( fonts->medium.highlighted );
		controllerTypeButton->SetListener(this);
	}
	
	///////////////////////////////////////////////////////////////////////////
	// GAMMA Ramp

	{
		int x = getLocaleGuiInt( "gui_optionsmenu_gammaslider_x", 400 );
		int y = getLocaleGuiInt( "gui_optionsmenu_gammaslider_y", 100 );
		int w = getLocaleGuiInt( "gui_optionsmenu_gammaslider_w", 90 );
		int h = getLocaleGuiInt( "gui_optionsmenu_gammaslider_h", 15 );

		int clip_x = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_x", 0 );
		int clip_y = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_y", 0 );
		int clip_w = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_w", 0 );
		int clip_h = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_h", 0 );

		std::string sliderButtonBackgroundNormal =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_normal" );
		std::string sliderButtonBackgroundDown =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_down" );
		std::string sliderButtonBackgroundHigh =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_high" );

		std::string sliderButtonForegroundNormal =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_normal" );
		std::string sliderButtonForegroundDown =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_down" );
		std::string sliderButtonForegroundHigh =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_high" );

		float value = getGamma();

		OguiSlider* slider;

		slider = new OguiSlider( win, ogui, x, y, w, h, 
					sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
					sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
					COMMANDS_SLIDERGAMMA, value );


		slider->setBarPosition( x + clip_x, y + clip_y, clip_w, clip_h );
		slider->setListener( this );

		sliderButtons.push_back( slider );

		int text_x = getLocaleGuiInt( "gui_optionsmenu_gammatext_x", 0 );
		int text_y = getLocaleGuiInt( "gui_optionsmenu_gammatext_y", 0 );
		int text_w = getLocaleGuiInt( "gui_optionsmenu_gammatext_w", 0 );
		int text_h = getLocaleGuiInt( "gui_optionsmenu_gammatext_h", 0 );

		addText( getLocaleGuiString( "gui_optionsmenu_gammatext" ), text_x, text_y, text_w, text_h, fonts->medium.highlighted ); 
	}

	///////////////////////////////////////////////////////////////////////////
	// Camera rotation speed
	// DH_OPT_F_CAMERA_ROTATION_STRENGTH 207
	// DH_OPT_F_CAMERA_ROTATION_SAFE 208
	// DH_OPT_F_CAMERA_ROTATION_SPRING 209


	// GameController::JOYSTICK_AXIS tempHack = detectJoystickAxis( gameController );
	// createJoystickButtons();

	determineCurrentController();

	// Logger::getInstance()->warning( std::string( "joy2-up = " + boost::lexical_cast< std::string >( gameController->getKeycodeNumberForName( "joy2-up" ) ) ).c_str() );
	// joy-up : 270
	// joy2-up: 290

	if( game->isCooperative() )
	{
		openCoopProfileMenu();
	}

//	setProfile( "Default" );
}


//.............................................................................

OptionsMenu::~OptionsMenu()
{
	freeJoystickButtons();
	freeMouseButtons();

	delete cameraModeHack;
	delete cameraModeTextHack;

	if(	closeMenuByEsc )
	{
		game->gameUI->getController(0)->removeKeyreader( editHandle  );
		debugKeyreader( editHandle, true, "OptionsMenu::~OptionsMenu()" );
	}

	delete headerText;

	delete imageSelectNorm;
	delete imageSelectDown;

	delete difficultImageSelectDown;
	delete difficultImageSelectNorm;

	delete controllerTypeButton;
	delete controllerTypeListStyle;
	delete controllerTypeList;
	delete controllerTypeListCaptureEvents;


	/*
	std::list< OguiSlider* >		sliderButtons;
	std::list< OguiTextLabel* >		textLabels;
	*/
	while( selectionButtonDescs.empty() == false )
	{
		delete selectionButtonDescs.front();
		selectionButtonDescs.pop_front();
	}

	{
		int i;
		for ( i = 0; i < (int)controlDescriptions.size(); i++ )
		{
			delete controlDescriptions[ i ];
		}
	}

	while( sliderButtons.empty() == false )
	{
		delete sliderButtons.front();
		sliderButtons.pop_front();
	}

	while( textLabels.empty() == false )
	{
		delete textLabels.front();
		textLabels.pop_front();
	}
	
	{
		std::map< int, OguiButton* >::iterator i;
		for( i = difficultButtons.begin(); i != difficultButtons.end(); ++i )
		{
			delete i->second;
		}

		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{
			delete i->second;
		}
	}

	while( !buttons.empty() )
	{
		delete *(buttons.begin());
		buttons.pop_front();
	}

	{
		std::list< OguiButtonStyle* >::iterator i;
		for( i = styles.begin(); i != styles.end(); ++i )
		{
			delete (*i)->image;
			delete (*i)->imageDown;
			delete (*i)->imageDisabled;
			delete (*i)->imageHighlighted;
			delete (*i)->textFont;
			delete (*i)->textFontHighlighted;
			delete (*i)->textFontDown;
			delete (*i)->textFontDisabled;

			delete (*i);
		}

		delete selectListStyle;
	}

	delete cooperativeBigText;
	delete cooperativeProfileList;


	assert( mouseEventCaptureWindow == NULL );

	delete mouseEventCaptureWindow;
	mouseEventCaptureWindow = NULL;

	delete fontSelectNorm;
	delete fontSelectDown;
	delete fontDescNorm;
	delete fontDescDown;

	delete win;

	OptionsMenu::returnToCoopMenu = false;
}

///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::setProfile( const std::string& profile )
{
	applyChanges();
	currentProfile = profile;

#ifdef LEGACY_FILES
	std::string tmp = "Profiles/";
	tmp += currentProfile;
	tmp += "/Config/keybinds.txt";
#else
	std::string tmp = "profiles/";
	tmp += currentProfile;
	tmp += "/config/keybinds.txt";
#endif
	gameController->loadConfiguration( igios_mapUserDataPrefix(tmp).c_str() );
	
	readControls();
	updateControlDescriptions();
}

///////////////////////////////////////////////////////////////////////////////

int OptionsMenu::getType() const
{
	return MenuCollection::MENU_TYPE_OPTIONSMENU;
}

///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::closeMenu()
{
	if(OptionsMenu::returnToCoopMenu)
	{
		applyChanges();
		menuCollection->changeMenu( MenuCollection::MENU_TYPE_COOPMENU );
	}
	else
	{
		// close menu
		menuCollection->closeMenu();
	}
}

//.............................................................................

void OptionsMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

//.............................................................................

void OptionsMenu::applyChanges()
{
	// assert( menuCollection );
	// Save volume 
	int musicVolume = (int) ( sliderMusicValue * 100.0f );
	int soundVolume = (int) ( sliderSoundValue * 100.0f );
	int speechVolume = (int) ( sliderSpeechValue * 100.0f );
	int ambientVolume = (int) ( sliderAmbientValue * 100.0f );

	if(!ambientOptionAvailable)
	{
		ambientVolume = soundVolume;
	}
	
	SimpleOptions::setInt( DH_OPT_I_MUSIC_VOLUME,	musicVolume );
	SimpleOptions::setInt( DH_OPT_I_FX_VOLUME,		soundVolume );
	SimpleOptions::setInt( DH_OPT_I_SPEECH_VOLUME,	speechVolume );
	SimpleOptions::setInt( DH_OPT_I_AMBIENT_VOLUME,	ambientVolume );

	// if( cameraRotateStrength )
	setCameraRotateRate( cameraRotateSpeed );
	
	//if( cameraSpringStrength )
	setMouseSpring( mouseSpring );
	
	// above setInt calls do not yet apply the changes, thus this apply is needed --jpk
	OptionApplier::applySoundOptions(game->gameUI->getSoundMixer());

	if( cameraModeHack )
	{
		game->gameUI->getGameCamera()->setGameCameraMode( !cameraModeHack->getValue() );
		SimpleOptions::setBool( DH_OPT_B_GAME_MODE_AIM_UPWARD, !cameraModeHack->getValue() );
		// game->gameScripting->setGlobalIntVariableValue( "game_mode_aim_upward", cameraModeHack->getValue()?0:1 );
	}

	if( cameraLockYAxis )
		camera_lock_y_axis = cameraLockYAxis->getValue();
	
	game->gameUI->getGameCamera()->setGameCameraYAxisLock( camera_lock_y_axis );
	

	gameController->setControllerType( (GameController::CONTROLLER_TYPE)currentController );

	if( gameController->controllerTypeHasMouse() )
	{
		SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + cooperativeSelection, true );
	}
	else
	{
		SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + cooperativeSelection, false );
	}
	SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME + cooperativeSelection, gameController->getControllerType() );

	// Save difficulty
	// setDifficulty( difficultActiveSelection );
	
	//
#ifdef LEGACY_FILES
	std::string tmp = "Profiles/";
	tmp += currentProfile;
	tmp += "/Config/keybinds.txt";
#else
	std::string tmp = "profiles/";
	tmp += currentProfile;
	tmp += "/config/keybinds.txt";
#endif
	gameController->saveConfiguration( igios_mapUserDataPrefix(tmp).c_str() );

	if(game->gameUI)
		game->gameUI->setCursorControllers();
}

//.............................................................................

void OptionsMenu::menuClose()
{
	applyChanges();
	closeMenu();
}

//.............................................................................

void OptionsMenu::menuDefaults()
{
	// TODO set selection ...
	assert( gameController );
	
#ifdef LEGACY_FILES
	// should not this be default_keybinds.txt? (though not sure if that was the case in sg.) --jpk
	gameController->loadConfiguration( "Data/Misc/keybinds.txt" );
#else
	gameController->loadConfiguration( "data/misc/default_keybinds.txt" );
#endif

	readControls();
	updateControlDescriptions();

}

///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::checkBoxEvent( OguiCheckBoxEvent* eve )
{
	if( eve->value == true )
	{
		freeMouseButtons();
		createMouseButtons();
	}
	else
	{
		freeMouseButtons();
	}
}


void OptionsMenu::CursorEvent( OguiButtonEvent* eve )
{
	if( eve->triggerButton == controllerTypeListCaptureEvents )
	{
		bool hit_scroller = false;
		int x = eve->cursorScreenX - win->GetPositionX();
		int y = eve->cursorScreenY - win->GetPositionY();
		if(controllerTypeList)
		{
			OguiButton *but = controllerTypeList->getUpScrollBut();
			if(but != NULL)
			{
				if(x >= but->GetX() && x <= but->GetX() + but->GetSizeX() && y >= but->GetY() && y <= but->GetY() + but->GetSizeY())
				{
					hit_scroller = true;
				}
			}
			but = controllerTypeList->getDownScrollBut();
			if(but != NULL)
			{
				if(x >= but->GetX() && x <= but->GetX() + but->GetSizeX() && y >= but->GetY() && y <= but->GetY() + but->GetSizeY())
				{
					hit_scroller = true;
				}
			}
		}
		if(!hit_scroller)
		{
			closeControllerTypeList();
		}
		return;
	}

	if( eve->eventType == OGUI_EMASK_CLICK &&
		!(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK) &&
		!(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK))
	{

		if( mouseEventCaptureWindow != NULL )
		{			
			if( joystickUpdate != -1 )
			{

			}
			return; 
		}

		if( discartNextCursorEvent )
		{
			discartNextCursorEvent = false;
			return;
		}
		
		

		switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_CLOSEME:
			game->gameUI->playGUISound( soundClick.c_str() ); 
			menuClose();
			break;

		case COMMANDS_JOYSTICK_MOVE_XAXIS:
		case COMMANDS_JOYSTICK_MOVE_YAXIS:
		case COMMANDS_JOYSTICK_DIR_XAXIS:
		case COMMANDS_JOYSTICK_DIR_YAXIS:
			joystickSelection( eve->triggerButton->GetId() );
			break;

		case COMMANDS_DEFAULTS:
			game->gameUI->playGUISound( soundClick.c_str() ); 
			menuDefaults();
			break;

		case COMMANDS_EASY:
		case COMMANDS_NORMAL:
		case COMMANDS_HARD:
			game->gameUI->playGUISound( soundClick.c_str() ); 
			selectDifficultButton( eve->triggerButton->GetId() );
			break;

		case COMMANDS_CONTROLLERTYPE:
			openControllerTypeList();
			break;

		default:
			MenuBaseImpl::CursorEvent( eve );
			return;
		}
	}
	else
	{
		MenuBaseImpl::CursorEvent( eve );

	}
}

//.............................................................................

void OptionsMenu::sliderEvent( OguiSliderEvent* eve )
{
	if( eve->type == OguiSliderEvent::EVENT_TYPE_MOUSEDOWN )
	{

	}

	bool sounds_updated = false;

	switch( eve->button->getId() )
	{
	case COMMANDS_SLIDERGAMMA:
		setGamma( eve->value );
		break;

	case COMMANDS_SLIDERROTATESPEED:
		cameraRotateSpeed = eve->value;
		break;

	case COMMANDS_SLIDERMOUSESPRING:
		mouseSpring = eve->value;
		break;

	case COMMANDS_SLIDERMUSIC:
		sounds_updated = true;
		sliderMusicValue = eve->value;
		
		break;

	case COMMANDS_SLIDERSOUND:
		sounds_updated = true;
		sliderSoundValue = eve->value;

		if( eve->type == OguiSliderEvent::EVENT_TYPE_MOUSEDOWN )
			sliderSoundPlayNow = true;
		
		if( eve->type == OguiSliderEvent::EVENT_TYPE_RELEASE )
			sliderSoundPlayNow = false,
			lastPlayTime = 0;

		break;

	case COMMANDS_SLIDERSPEECH:
		sounds_updated = true;
		sliderSpeechValue = eve->value;

		if( eve->type == OguiSliderEvent::EVENT_TYPE_MOUSEDOWN )
			sliderSpeechPlayNow = true;
		
		if( eve->type == OguiSliderEvent::EVENT_TYPE_RELEASE )
			sliderSpeechPlayNow = false,
			lastPlayTime = 0;
		break;

	case COMMANDS_SLIDERAMBIENT:
		sounds_updated = true;
		sliderAmbientValue = eve->value;	
		break;
	}

	if( sounds_updated )
	{
		int musicVolume = (int) ( sliderMusicValue * 100.0f );
		int soundVolume = (int) ( sliderSoundValue * 100.0f );
		int speechVolume = (int) ( sliderSpeechValue * 100.0f );
		int ambientVolume = (int) ( sliderAmbientValue * 100.0f );
		if(!ambientOptionAvailable)
		{
			ambientVolume = soundVolume;
		}
		
		SimpleOptions::setInt( DH_OPT_I_MUSIC_VOLUME,	musicVolume );
		SimpleOptions::setInt( DH_OPT_I_FX_VOLUME,		soundVolume );
		SimpleOptions::setInt( DH_OPT_I_SPEECH_VOLUME,	speechVolume );
		SimpleOptions::setInt( DH_OPT_I_AMBIENT_VOLUME,	ambientVolume );

		if( game->getGameUI()->getSoundMixer() )
			game->getGameUI()->getSoundMixer()->setVolume( 100, soundVolume, speechVolume, musicVolume, ambientVolume );

	}
}

//.............................................................................

// Cooperative selection menu event
void OptionsMenu::SelectEvent( OguiSelectListEvent* eve )
{

	if( eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT )
	{
		if( eve->triggerSelectList == controllerTypeList)
		{
			setControllerType(str2int(eve->selectionValue));
			closeControllerTypeList();

		}
		else
		{
			applyChanges();

			cooperativeSelection = eve->selectionNumber;
			currentProfile = game->getGameProfiles()->getCurrentProfile( cooperativeSelection );
			gameController = game->gameUI->getController( cooperativeSelection );

			//game->getGameProfiles()->getCurrentProfile( cooperativeSelection );
			//this->setProfile( game->getGameProfiles()->getCurrentProfile( cooperativeSelection ) );

			readControls();
			updateControlDescriptions();
		}
	}
}

//.............................................................................

void OptionsMenu::update()
{


	if( sliderSoundPlayNow && ( Timer::getTime() - lastPlayTime ) > sliderSoundLoopTime )
	{
		if( !sliderSoundPlayfile.empty() ) 
			game->getGameUI()->playGUISound( sliderSoundPlayfile.c_str(), (int)( 100 ) );
		
		lastPlayTime = Timer::getTime();
	}

	if( sliderSpeechPlayNow && ( Timer::getTime() - lastPlayTime ) > sliderSpeechLoopTime )
	{
		if( !sliderSpeechPlayfile.empty() ) 
			game->getGameUI()->playGUISpeech( sliderSpeechPlayfile.c_str(), (int)( 100 ) );
		
		lastPlayTime = Timer::getTime();
	}

	if( joystickUpdate != -1 )
	{
		// gameController->getDetectedAxis();
		GameController::JOYSTICK_AXIS axis = gameController->getDetectedAxis();
		if( axis != 0 )
		{
			setJoystickAxis( joystickUpdate, axis );

			delete mouseEventCaptureWindow;
			mouseEventCaptureWindow = NULL;

			joystickUpdate = -1;
		}
		else
		{
			return;
		}
	}


	if( controlUpdate == -1 && mouseEventCaptureWindow != NULL )
	{			
		assert( gameController );
		
		if( gameController->isKeyDownByKeyCode( 262 ) == false &&	// mouse button 1
			gameController->isKeyDownByKeyCode( 263 ) == false &&   // mouse button 2
			gameController->isKeyDownByKeyCode( 264 ) == false &&   // mouse button 3
			gameController->isKeyDownByKeyCode( 274 ) == false )	// joy1-button1
		{
			delete mouseEventCaptureWindow;
			mouseEventCaptureWindow = NULL;
		} else {
			discartNextCursorEvent = true;
		}
		return; 
	}

	if( controlUpdate != -1 )
	{
		assert( gameController );
		int i;
		int keycode = -1;
		// go through all the keys to find if some is pressed



		for( i = KEYCODE_NAME_AMOUNT - 1; i >= 0; i-- ) 
		{
			// NOTE: ignore mouse movement, but NOT joystick movement... 
			if( gameController->isKeyDownByKeyCode( i ) 
				&& ( i < 256 || i > 259 )
				&&!( i >=KEYCODE_MOUSE(0, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(0, KEYCODE_MOUSE_RIGHT) ) 
				&&!( i >=KEYCODE_MOUSE(1, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(1, KEYCODE_MOUSE_RIGHT) ) 
				&&!( i >=KEYCODE_MOUSE(2, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(2, KEYCODE_MOUSE_RIGHT) ) 
				&&!( i >=KEYCODE_MOUSE(3, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(3, KEYCODE_MOUSE_RIGHT) ) 
				&&!( i >=KEYCODE_MOUSE(4, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(4, KEYCODE_MOUSE_RIGHT) ) 
				&&!( i >=KEYCODE_MOUSE(5, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(5, KEYCODE_MOUSE_RIGHT) ) 
				&&!( i >=KEYCODE_MOUSE(6, KEYCODE_MOUSE_UP) && i <=KEYCODE_MOUSE(6, KEYCODE_MOUSE_RIGHT) ) 
				)
				//&& ( i < 310 || i > 341 ) )
			{
				keycode = i;
				break;
			}
		}

		// if esc was pressed
		if( keycode == 1 ||
			 keycode == KEYCODE_GEN_KEYBID(0, 1) ||
			 keycode == KEYCODE_GEN_KEYBID(1, 1) ||
			 keycode == KEYCODE_GEN_KEYBID(2, 1) ||
			 keycode == KEYCODE_GEN_KEYBID(3, 1) ||
			 keycode == KEYCODE_GEN_KEYBID(4, 1) 
			)
		{
			updateControlDescriptions();

			selectButton( -1 );
			controlUpdate = -1;
			// closeMenuByEsc = true;
			canWeCloseTheMenuNow = true;
			return;
		}

		// if pressed key found 
		if( keycode != -1 )
		{
			// set the keycodeArray for that key to what it should be
			gameController->unbindKey( controlNumArray[ controlUpdate ],  0 );
			gameController->unbindKey( controlNumArray[ controlUpdate ],  1 );
			gameController->bindKey( controlNumArray[ controlUpdate ], keycode );

#ifdef PROJECT_SURVIVOR
			// hack: if pressed a keyboard button, reassign weapon selection keys
			if(keycode < 256 || keycode > BASIC_KEYCODE_AMOUNT)
			{
				int kb_id = KEYCODE_KEYBID(keycode);
				for(int i = 0; i < 10; i++)
				{
					int original_key = gameController->getBoundKey(DH_CTRL_WEAPON_1 + i, 0);

					gameController->unbindKey(DH_CTRL_WEAPON_1 + i, 0);

					int new_key = original_key;
					if(new_key >= BASIC_KEYCODE_AMOUNT)
					{
						new_key = KEYCODE_KEYID(new_key);
					}

					if(kb_id > 0)
					{
						new_key = KEYCODE_GEN_KEYBID(kb_id, new_key);
					}
					
					gameController->bindKey(DH_CTRL_WEAPON_1 + i, new_key, 0, false);
				}
			}
#endif

			// keycodeArray[ controlUpdate ] = keycode;
			// redraw the descrption button for that key in the
			// controlDescriptions
			readControls();
			updateControlDescriptions();
			// controlDescriptions[ controlUpdate ]->SetText( getOptionsKeyName( controlUpdate ).c_str() );

			selectButton( -1 );
			controlUpdate = -1;
			// closeMenuByEsc = true;
			canWeCloseTheMenuNow = true;

			// To prevent joy button click being interpreted as new rebind request
			if (gameController->isKeyDownByKeyCode(KEYCODE_JOY_BUTTON1)
				|| gameController->isKeyDownByKeyCode(KEYCODE_JOY_BUTTON2)
				|| gameController->isKeyDownByKeyCode(KEYCODE_JOY_BUTTON5)
				|| gameController->isKeyDownByKeyCode(KEYCODE_JOY_BUTTON6))
				discartNextCursorEvent = true;
		}
		
	} 
}

//.............................................................................

void OptionsMenu::selectButton( int i )
{
/*	if( mouseEventCaptureWindow == NULL || i == -1 )
	{*/
		MenuBaseImpl::selectButton( i );
		controlUpdate = i;
	
	// }
	
	if( i != -1 ) 
	{
		if( i >= 0 && i < (int)controlDescriptions.size() )
		{
			// hack me up
			assert( mouseEventCaptureWindow == NULL );
			mouseEventCaptureWindow = ogui->CreateSimpleWindow(	0, 0, 1024, 768, NULL ); 
			mouseEventCaptureWindow->SetUnmovable();
			mouseEventCaptureWindow->Show();
			mouseEventCaptureWindow->SetOnlyActive();
			if( controlDescriptions[ i ] )
				controlDescriptions[ i ]->SetText( "" );
			// closeMenuByEsc = false;
			canWeCloseTheMenuNow = false;
		}
	} 
}

///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::handleEsc()
{
	if( joystickUpdate != -1 )
	{
		canWeCloseTheMenuNow = false;
		delete mouseEventCaptureWindow;
		mouseEventCaptureWindow = NULL;

		joystickUpdate = -1;

		updateControlDescriptions();
	}
	else
	{
		MenuBaseImpl::handleEsc();
	}
}

///////////////////////////////////////////////////////////////////////////////

std::string	OptionsMenu::getOptionsButtonName( int i )
{
	std::string foo = gameController->getControlName( controlNumArray[ i ] );


	std::string result = getLocaleGuiString( ( "key_" + foo ).c_str() );

	if( result == "(LOCALIZATION MISSING)" )
	{
		int i = 1;
		i++;
	}

    return getLocaleGuiString( ( "key_" + foo ).c_str() );

	std::stringstream ss;
	ss << "Button " << i;
	return ss.str();
}

//.............................................................................

std::string	OptionsMenu::getOptionsKeyName( int i )
{
	if( i <= options_max )
		return getKeyNameFromLocales( gameController->getKeycodeName( keycodeArray[ i ] ) );
	else if ( i == COMMANDS_JOYSTICK_MOVE_XAXIS )
		return getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickMoveXAxis() ) );
	else if ( i == COMMANDS_JOYSTICK_MOVE_YAXIS )
		return getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickMoveYAxis() ) );
	else if ( i == COMMANDS_JOYSTICK_DIR_XAXIS )
		return getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickDirXAxis() ) );
	else if ( i == COMMANDS_JOYSTICK_DIR_YAXIS )
		return getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickDirYAxis() ) );

	std::stringstream ss;
	ss << i;
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::addText( const std::string& text, int x, int y, int w, int h, IOguiFont* font )
{
	assert( win );
	assert( ogui );

	OguiTextLabel* foo;
	foo = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
	if ( font ) foo->SetFont( font );

	foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	textLabels.push_back( foo );
}

//.............................................................................

void OptionsMenu::setDifficulty( int difficulty )
{
	// HACK: SET DEFAULT DIFFICULTY TO EASY
	int difficultyLevel = 25;

	switch( difficulty )
	{
	case COMMANDS_EASY:
		difficultyLevel = 25;
		break;

	case COMMANDS_NORMAL:
		difficultyLevel = 50;
		break;

	case COMMANDS_HARD:
		difficultyLevel = 75;
		break;

	default:
		break;
	}

	game->gameScripting->setGlobalIntVariableValue("general_difficulty_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("damage_amount_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("item_amount_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("hostile_amount_level", difficultyLevel);
}

//.............................................................................

void OptionsMenu::selectDifficultButton( int i )
{
	if ( !difficultButtons.empty() )
	{
		if ( difficultButtons.find( i ) != difficultButtons.end() )
		{
			std::map< int, OguiButton* >::iterator it;

			if ( difficultActiveSelection != -1 )
			{
				it = difficultButtons.find( difficultActiveSelection );
				if ( it != difficultButtons.end() )
				{
					assert( difficultImageSelectNorm );
					it->second->SetImage( difficultImageSelectNorm );
				}

			}

			it = difficultButtons.find( i );

			if ( it != difficultButtons.end() )
			{
				assert( difficultImageSelectDown );
				it->second->SetImage( difficultImageSelectDown );	
				difficultActiveSelection = i;
			}
		}
	}	
}

//.............................................................................

void OptionsMenu::addDifficultButton( int x, int y, int w, int h, 
		const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, const std::string& text, int command )
{
	assert( ogui );
	assert( win );
	assert( command >= 0 );

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, x, y, w, h, 
		button_norm.empty()?NULL:button_norm.c_str(), 
		button_down.empty()?NULL:button_down.c_str(), 
		button_high.empty()?NULL:button_high.c_str(), 
		text.c_str(), command );
	
	b->SetListener( this );
	
	if ( font ) b->SetFont( font );

	difficultButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

//.............................................................................

void OptionsMenu::addControlDescription( const std::string& text, int id, int x_add, int y_add, IOguiFont* font, IOguiFont* high, IOguiFont* down, IOguiFont* disa )
{

	int x = buttonX + x_add;
	int y = buttonY + y_add;

	assert( ogui );
	assert( win );

	OguiButton* foo;
	foo = ogui->CreateSimpleTextButton( win, x, y, buttonDescriptionW, buttonDescriptionH, NULL, NULL, NULL, text.c_str() );
	// foo = ogui->CreateTextLabel( win, x, y, buttonDescriptionW, buttonDescriptionH, text.c_str() );
	if( font ) foo->SetFont( font );
	if( down ) foo->SetDownFont( down );
	if( high ) foo->SetHighlightedFont( high );
	// if( disa ) foo->SetDisabledFont( font );
	foo->SetDisabled( true );

	foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	foo->SetText( text.c_str() );

	if( id >= (int)controlDescriptions.size() )
		controlDescriptions.resize( id + 1 );
	
	assert( id >= 0 && id < (int)controlDescriptions.size() );
	controlDescriptions[ id ] = foo;
}

//.............................................................................

void OptionsMenu::updateControlDescriptions()
{
	int i;
	for ( i = 1; i < (int)controlDescriptions.size(); i++ )
	{
		if( controlDescriptions[ i ] )
			controlDescriptions[ i ]->SetText( getOptionsKeyName( i ).c_str() );
	}
	determineCurrentController();
}

//.............................................................................

void OptionsMenu::readControls()
{
	int i;
	for ( i = 1; i < (int)keycodeArray.size(); i++ )
	{
		keycodeArray[ i ] = gameController->getBoundKey( controlNumArray[ i ], 0 );
	}
	
	// updateControlDescriptions();
}


///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::determineCurrentController()
{
	// this was used to determine controller based on how many keys of each were used
	/*GameController::CONTROLLER_TYPE type = GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;

	std::vector< int > type_array( 5 );

	int i;
	for( i = 0; i < (int)type_array.size(); i++ )
	{
		type_array[ i ] = 0;
	}

	for( i = 0; i <= options_max; i++ )
	{
		if( gameController->getControllerTypeByKey( keycodeArray[ i ] ) != -1 )
		{
			type_array[ gameController->getControllerTypeByKey( keycodeArray[ i ] ) ]++;
		}
	}

	int max = 0;
	for( i = 0; i < (int)type_array.size(); i++ )
	{
		if( type_array[ i ] > max ) 
		{
			max = type_array[ i ];
			type = ( GameController::CONTROLLER_TYPE )i;
		}
	}

	currentController = type;*/

	currentController = gameController->getControllerType();

	if( currentController >= GameController::CONTROLLER_TYPE_JOYSTICK1 && currentController <= GameController::CONTROLLER_TYPE_JOYSTICK4 )
	{
		createJoystickButtons();
	}
	else
	{
		freeJoystickButtons();
	}
	updateControllerTypeText();
}

//=============================================================================

void OptionsMenu::joystickSelection( int i )
{
	joystickUpdate = i;
	MenuBaseImpl::selectButton( i );
	
	
	if( i != -1 ) 
	{
		if( i >= 0 && i < (int)controlDescriptions.size() )
		{
			gameController->startJoystickDetection( );

			// hack me up
			assert( mouseEventCaptureWindow == NULL );
			mouseEventCaptureWindow = ogui->CreateSimpleWindow(	0, 0, 1024, 768, NULL ); 
			mouseEventCaptureWindow->SetUnmovable();
			mouseEventCaptureWindow->Show();
			mouseEventCaptureWindow->SetOnlyActive();
			controlDescriptions[ i ]->SetText( "" );
			// closeMenuByEsc = false;
			canWeCloseTheMenuNow = false;
		}
	} 
}

//=============================================================================

void OptionsMenu::setJoystickAxis( int axis, GameController::JOYSTICK_AXIS button )
{
	switch( axis )
	{
	case COMMANDS_JOYSTICK_MOVE_XAXIS:
		gameController->setJoystickMoveXAxis( button );
		break;
	case COMMANDS_JOYSTICK_MOVE_YAXIS:
		gameController->setJoystickMoveYAxis( button );
		break;
	case COMMANDS_JOYSTICK_DIR_XAXIS:
		gameController->setJoystickDirXAxis( button );
		break;
	case COMMANDS_JOYSTICK_DIR_YAXIS:
		gameController->setJoystickDirYAxis( button );
		break;
	}

	controlDescriptions[ axis ]->SetText( getKeyNameFromLocales( gameController->getJoystickAxisName( button ) ).c_str() );
}

//=============================================================================

void OptionsMenu::createJoystickButtons()
{
	if( joystickMenuOpen )
		return;
	{
		buttonX = getLocaleGuiInt( "gui_optionsmenu_joystick_button_x", 0 );
		buttonY = getLocaleGuiInt( "gui_optionsmenu_joystick_button_y", 0 );
		buttonW = getLocaleGuiInt( "gui_optionsmenu_joystick_button_w", 0 );
		buttonH = getLocaleGuiInt( "gui_optionsmenu_joystick_button_h", 0 );

		buttonDescriptionW = getLocaleGuiInt( "gui_optionsmenu_joystick_desc_w", 0 );
		buttonDescriptionH = getLocaleGuiInt( "gui_optionsmenu_joystick_desc_h", 0 );

		buttonAddX = getLocaleGuiInt( "gui_optionsmenu_joystick_button_add_x", 0 );
		buttonAddY = getLocaleGuiInt( "gui_optionsmenu_joystick_button_add_y", 0 );

		int key_name_add_x = getLocaleGuiInt( "gui_optionsmenu_joystick_desc_add_x", 0 );

		//.....................................................................

		{
			SelectionButtonDescs* descs = new SelectionButtonDescs;

			std::string axis_key_name = getLocaleGuiString( "gui_optionsmenu_joystick_x_axis" );
			std::string button_name = getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickMoveXAxis() ) );

			addControlDescription( button_name, COMMANDS_JOYSTICK_MOVE_XAXIS, key_name_add_x, 0, fonts->little.normal, fonts->little.highlighted, fonts->little.down, fonts->little.disabled );
			descs->first = controlDescriptions[ COMMANDS_JOYSTICK_MOVE_XAXIS ];
			addSelectionButton( axis_key_name, COMMANDS_JOYSTICK_MOVE_XAXIS, fonts->medium.normal, descs );
			selectionButtonDescs.push_back( descs );
		}
		//.....................................................................
		{
			SelectionButtonDescs* descs = new SelectionButtonDescs;

			std::string axis_key_name = getLocaleGuiString( "gui_optionsmenu_joystick_y_axis" );
			std::string button_name = getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickMoveYAxis() ) );

			addControlDescription( button_name, COMMANDS_JOYSTICK_MOVE_YAXIS, key_name_add_x, 0, fonts->little.normal, fonts->little.highlighted, fonts->little.down, fonts->little.disabled );
			descs->first = controlDescriptions[ COMMANDS_JOYSTICK_MOVE_YAXIS ];
			addSelectionButton( axis_key_name, COMMANDS_JOYSTICK_MOVE_YAXIS, fonts->medium.normal, descs );
			selectionButtonDescs.push_back( descs );
		}
		//.....................................................................
		{
			SelectionButtonDescs* descs = new SelectionButtonDescs;

			std::string axis_key_name = getLocaleGuiString( "gui_optionsmenu_joystick_dir_x_axis" );
			std::string button_name = getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickDirXAxis() ) );

			addControlDescription( button_name, COMMANDS_JOYSTICK_DIR_XAXIS, key_name_add_x, 0, fonts->little.normal, fonts->little.highlighted, fonts->little.down, fonts->little.disabled );
			descs->first = controlDescriptions[ COMMANDS_JOYSTICK_DIR_XAXIS ];
			addSelectionButton( axis_key_name, COMMANDS_JOYSTICK_DIR_XAXIS, fonts->medium.normal, descs );
			selectionButtonDescs.push_back( descs );
		}
		//.....................................................................
		{
			SelectionButtonDescs* descs = new SelectionButtonDescs;

			std::string axis_key_name = getLocaleGuiString( "gui_optionsmenu_joystick_dir_y_axis" );
			std::string button_name = getKeyNameFromLocales( gameController->getJoystickAxisName( gameController->getJoystickDirYAxis() ) );

			addControlDescription( button_name, COMMANDS_JOYSTICK_DIR_YAXIS, key_name_add_x, 0, fonts->little.normal, fonts->little.highlighted, fonts->little.down, fonts->little.disabled );
			descs->first = controlDescriptions[ COMMANDS_JOYSTICK_DIR_YAXIS ];
			addSelectionButton( axis_key_name, COMMANDS_JOYSTICK_DIR_YAXIS, fonts->medium.normal, descs );
			selectionButtonDescs.push_back( descs );
		}
		//.....................................................................
		{
			
			std::string text =	getLocaleGuiString( "gui_optionsmenu_text_joystickaxis" );
			int			x =		getLocaleGuiInt( "gui_optionsmenu_text_joystickaxis_x", 0 );
			int			y =		getLocaleGuiInt( "gui_optionsmenu_text_joystickaxis_y", 0 );
			int			w =		getLocaleGuiInt( "gui_optionsmenu_text_joystickaxis_w", 0 );
			int			h =		getLocaleGuiInt( "gui_optionsmenu_text_joystickaxis_h", 0 );

			joystickBigText = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
			joystickBigText->SetFont( fonts->big.normal );

			joystickBigText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		}
	}

	joystickMenuOpen = true;

}

//=============================================================================

void OptionsMenu::freeJoystickButtons()
{
	if( joystickMenuOpen == false )
		return;

	delete joystickBigText;
	joystickBigText = NULL;

	delete controlDescriptions[ COMMANDS_JOYSTICK_MOVE_XAXIS ];
	controlDescriptions[ COMMANDS_JOYSTICK_MOVE_XAXIS ] = NULL;

	delete controlDescriptions[ COMMANDS_JOYSTICK_MOVE_YAXIS ];
	controlDescriptions[ COMMANDS_JOYSTICK_MOVE_YAXIS ] = NULL;

	delete controlDescriptions[ COMMANDS_JOYSTICK_DIR_XAXIS ];
	controlDescriptions[ COMMANDS_JOYSTICK_DIR_XAXIS ] = NULL;

	delete controlDescriptions[ COMMANDS_JOYSTICK_DIR_YAXIS ];
	controlDescriptions[ COMMANDS_JOYSTICK_DIR_YAXIS ] = NULL;

	std::map< int, OguiButton* >::iterator i;

	i = selectButtons.find( COMMANDS_JOYSTICK_MOVE_XAXIS );
	if( i != selectButtons.end() )
	{
		SelectionButtonDescs* desc = (SelectionButtonDescs*)i->second->GetArgument();
		delete desc;
		selectionButtonDescs.erase( std::find( selectionButtonDescs.begin(), selectionButtonDescs.end(), desc ) );

		delete i->second;
		selectButtons.erase( i );
	}

	i = selectButtons.find( COMMANDS_JOYSTICK_MOVE_YAXIS );
	if( i != selectButtons.end() )
	{
		SelectionButtonDescs* desc = (SelectionButtonDescs*)i->second->GetArgument();
		delete desc;
		selectionButtonDescs.erase( std::find( selectionButtonDescs.begin(), selectionButtonDescs.end(), desc ) );

		delete i->second;
		selectButtons.erase( i );
	}

	i = selectButtons.find( COMMANDS_JOYSTICK_DIR_XAXIS );
	if( i != selectButtons.end() )
	{
		SelectionButtonDescs* desc = (SelectionButtonDescs*)i->second->GetArgument();
		delete desc;
		selectionButtonDescs.erase( std::find( selectionButtonDescs.begin(), selectionButtonDescs.end(), desc ) );

		delete i->second;
		selectButtons.erase( i );
	}

	i = selectButtons.find( COMMANDS_JOYSTICK_DIR_YAXIS );
	if( i != selectButtons.end() )
	{
		SelectionButtonDescs* desc = (SelectionButtonDescs*)i->second->GetArgument();
		delete desc;
		selectionButtonDescs.erase( std::find( selectionButtonDescs.begin(), selectionButtonDescs.end(), desc ) );

		delete i->second;
		selectButtons.erase( i );
	}

	// assert( false );
	/*
	delete joystickXButton;
	joystickXButton = NULL;

	delete joystickYButton;
	joystickYButton = NULL;
	*/
	joystickMenuOpen = false;
}

///////////////////////////////////////////////////////////////////////////////

OguiButtonStyle* OptionsMenu::loadStyle( const std::string& button_name )
{
	IOguiImage* norm = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_norm" ).c_str() ) );
	IOguiImage* high = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_high" ).c_str() ) );
	IOguiImage* down = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_down" ).c_str() ) );
	IOguiImage* disa = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_disa" ).c_str() ) );
	IOguiFont*  font = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font" ).c_str() ) );
	IOguiFont*  font_down = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font_down" ).c_str() ) );
	IOguiFont*  font_disa = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font_disa" ).c_str() ) );
	IOguiFont*  font_high = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font_high" ).c_str() ) );


	int w = getLocaleGuiInt( ( button_name + "_w" ).c_str(), 0 );
	int h = getLocaleGuiInt( ( button_name + "_h" ).c_str(), 0 );

	OguiButtonStyle* result = new OguiButtonStyle( norm, down, disa, high, font, w, h );
	result->textFontDisabled	= font_disa;
	result->textFontDown		= font_down;
	result->textFontHighlighted = font_high;

	styles.push_back( result );
	return result;

}

///////////////////////////////////////////////////////////////////////////////

void OptionsMenu::openCoopProfileMenu()
{

	menuCollection->setBackgroundImage( getLocaleGuiString( "gui_optionsmenu_coopbackground_image" ) );

	{
			
		std::string text =	getLocaleGuiString( "gui_optionsmenu_text_profiles" );
		int			x =		getLocaleGuiInt( "gui_optionsmenu_text_profiles_x", 0 );
		int			y =		getLocaleGuiInt( "gui_optionsmenu_text_profiles_y", 0 );
		int			w =		getLocaleGuiInt( "gui_optionsmenu_text_profiles_w", 0 );
		int			h =		getLocaleGuiInt( "gui_optionsmenu_text_profiles_h", 0 );

		cooperativeBigText = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
		cooperativeBigText->SetFont( fonts->big.normal );

		cooperativeBigText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}

	int scroll_button_w = 0;

	if( selectListStyle == NULL )
	{
		OguiButtonStyle* unselStyle =		loadStyle( "gui_optionsmenu_unselected_item" );
		OguiButtonStyle* selStyle =			loadStyle( "gui_optionsmenu_selected_item" );

#ifdef PROJECT_SURVIVOR
		OguiButtonStyle* newStyle =			loadStyle( "gui_optionsmenu_selected_item" );
		OguiButtonStyle* newUnStyle =		loadStyle( "gui_optionsmenu_selected_item" );
#else
		OguiButtonStyle* newStyle =			loadStyle( "gui_optionsmenu_new_selected_item" );
		OguiButtonStyle* newUnStyle =		loadStyle( "gui_optionsmenu_new_unselected_item" );
#endif

		OguiButtonStyle* scrollUpStyle =	loadStyle( "gui_optionsmenu_arrow_up" );
		OguiButtonStyle* scrollDownStyle =	loadStyle( "gui_optionsmenu_arrow_down" );

		scroll_button_w = scrollUpStyle->sizeX>scrollDownStyle->sizeX?scrollUpStyle->sizeX:scrollDownStyle->sizeX;

		int num_of_elements = MAX_PLAYERS_PER_CLIENT;
	
		selectListStyle = new OguiSelectListStyle( unselStyle, selStyle, newStyle, newUnStyle, scrollUpStyle, scrollDownStyle, unselStyle->sizeX, unselStyle->sizeY * num_of_elements, scrollUpStyle->sizeX, scrollUpStyle->sizeY );

	}
	
	{
		// buttonW	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
		// buttonH	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

		int x	= getLocaleGuiInt( "gui_optionsmenu_coopprofile_x", 0 );
		int y	= getLocaleGuiInt( "gui_optionsmenu_coopprofile_y", 0 );

		/*x += buttonAddX * convertToRunningNum( i );
		y += buttonAddY * convertToRunningNum( i );
		y += buttonH;*/

		cooperativeProfileList = ogui->CreateSelectList( win, x, y, selectListStyle, 0, NULL, NULL );
		cooperativeProfileList->setListener( this );
	}

	{
		int i;
		for ( i = 0; i < getNumberOfPlayers(); i++ )
		{
			cooperativeProfileList->addItem( game->getGameProfiles()->getCurrentProfile( i ), game->getGameProfiles()->getCurrentProfile( i ) );
			if( i == cooperativeSelection )
			{
				cooperativeProfileList->setSelected( i, true );
				setProfile( game->getGameProfiles()->getCurrentProfile( i ) );
			}
		}
	}

	// coopCurrentSelection = i;
}

//.............................................................................

void OptionsMenu::closeCoopProfileMenu()
{
}

///////////////////////////////////////////////////////////////////////////////

static OguiTextLabel* hackText1 = NULL;
static OguiTextLabel* hackText2 = NULL;

void OptionsMenu::createMouseButtons()
{
	
	// Lock y axis
	{
		cameraLockYAxis = new OguiCheckBox( win, ogui,
			getLocaleGuiInt( "gui_optionsmenu_cameralock_x", 0 ), 
			getLocaleGuiInt( "gui_optionsmenu_cameralock_y", 0 ), 
			getLocaleGuiInt( "gui_optionsmenu_cameralock_w", 0 ), 
			getLocaleGuiInt( "gui_optionsmenu_cameralock_h", 0 ), 	
			getLocaleGuiString( "gui_optionsmenu_camerabox_img_norm" ), "", "", 
			getLocaleGuiString( "gui_optionsmenu_camerabox_img_fill" ) );

		cameraLockYAxis->setText( ( (std::string)getLocaleGuiString( "camera_lock_y_axis" ) ).c_str(), OguiCheckBox::TEXT_ALIGN_RIGHT, 200, fonts->little.highlighted );
		cameraLockYAxis->setValue( camera_lock_y_axis );
	}

	{
		int x = getLocaleGuiInt( "gui_optionsmenu_camerarotate_x", 0 );
		int y = getLocaleGuiInt( "gui_optionsmenu_camerarotate_y", 0 );
		int w = getLocaleGuiInt( "gui_optionsmenu_camerarotate_w", 0 );
		int h = getLocaleGuiInt( "gui_optionsmenu_camerarotate_h", 0 );

		int clip_x = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_x", 0 );
		int clip_y = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_y", 0 );
		int clip_w = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_w", 0 );
		int clip_h = getLocaleGuiInt( "gui_optionsmenu_slider_button_clip_h", 0 );

		int text_x = getLocaleGuiInt( "gui_optionsmenu_camerarotate_text_x", 0 );
		int text_y = getLocaleGuiInt( "gui_optionsmenu_camerarotate_text_y", 0 );
		int text_w = getLocaleGuiInt( "gui_optionsmenu_camerarotate_text_w", 0 );
		int text_h = getLocaleGuiInt( "gui_optionsmenu_camerarotate_text_h", 0 );

		std::string sliderButtonBackgroundNormal =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_normal" );
		std::string sliderButtonBackgroundDown =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_down" );
		std::string sliderButtonBackgroundHigh =	getLocaleGuiString( "gui_optionsmenu_slider_button_background_high" );

		std::string sliderButtonForegroundNormal =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_normal" );
		std::string sliderButtonForegroundDown =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_down" );
		std::string sliderButtonForegroundHigh =	getLocaleGuiString( "gui_optionsmenu_slider_button_foreground_high" );

		float value = getCameraRotateRate();
		
		OguiSlider* slider;

		cameraRotateStrength = new OguiSlider( win, ogui, x, y, w, h, 
					sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
					sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
					COMMANDS_SLIDERROTATESPEED, value );

		slider = cameraRotateStrength;

		std::string textRotateSpeed = getLocaleGuiString( "gui_optionsmenu_text_rotatespeed" );
		{
			const std::string& text = textRotateSpeed;
			int x = text_x;
			int y = text_y;
			int w = text_w;
			int h = text_h;
			IOguiFont* font = fonts->little.highlighted;

			OguiTextLabel* foo;
			foo = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
			if ( font ) foo->SetFont( font );

			foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
			
			hackText1 = foo;
			// textLabels.push_back( foo );
		}

		slider->setBarPosition( x + clip_x, y + clip_y, clip_w, clip_h );
		slider->setValue( cameraRotateSpeed );
		slider->setListener( this );


		// sliderButtons.push_back( slider );

		//=====================================================================

		// Spring strength

		int sliderButtonAddX = getLocaleGuiInt( "gui_optionsmenu_slider_button_add_x", 0 );
		int sliderButtonAddY = getLocaleGuiInt( "gui_optionsmenu_slider_button_add_y", 0 );

		x += sliderButtonAddX;
		y += sliderButtonAddY;

		text_x += sliderButtonAddX;
		text_y += sliderButtonAddY;

		value = getMouseSpring();
		
		// OguiSlider* slider;

		cameraSpringStrength = new OguiSlider( win, ogui, x, y, w, h, 
					sliderButtonBackgroundNormal, sliderButtonBackgroundDown, sliderButtonBackgroundHigh,
					sliderButtonForegroundNormal, sliderButtonForegroundDown, sliderButtonForegroundHigh, 
					COMMANDS_SLIDERMOUSESPRING, value );

		slider = cameraSpringStrength;
		slider->setValue( mouseSpring );

		std::string textMouseSpring = getLocaleGuiString( "gui_optionsmenu_text_mousespring" );
		// addText( textMouseSpring, text_x, text_y, text_w, text_h, fonts->little.highlighted ); 
		{
			const std::string& text = textMouseSpring;
			int x = text_x;
			int y = text_y;
			int w = text_w;
			int h = text_h;
			IOguiFont* font = fonts->little.highlighted;

			OguiTextLabel* foo;
			foo = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
			if ( font ) foo->SetFont( font );

			foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
			
			hackText2 = foo;
			// textLabels.push_back( foo );
		}

		slider->setBarPosition( x + clip_x, y + clip_y, clip_w, clip_h );
		slider->setListener( this );

		// sliderButtons.push_back( slider );
	}
}

//.............................................................................

void OptionsMenu::freeMouseButtons()
{
	if( cameraLockYAxis ) 
		camera_lock_y_axis = cameraLockYAxis->getValue();

	delete cameraSpringStrength;
	cameraSpringStrength = NULL;

	delete cameraRotateStrength;
	cameraRotateStrength = NULL;

	delete cameraLockYAxis;
	cameraLockYAxis = NULL;

	delete hackText1;
	hackText1 = NULL;
	
	delete hackText2;
	hackText2 = NULL;
}


void OptionsMenu::updateControllerTypeText(void)
{
	GameController::CONTROLLER_TYPE type = (GameController::CONTROLLER_TYPE)currentController;

#ifndef PROJECT_SHADOWGROUNDS
	// mouse controller
	if(type >= GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 && type <= GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE4)
	{
		std::string mouse_desc = getLocaleGuiString("gui_optionsmenu_controllertype_mouse");
		int id = type - GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 + 1;
		controllerTypeButton->SetText((mouse_desc + " " + boost::lexical_cast<std::string>(id)).c_str());
	}
	// joystick controllers
	else if(type >= GameController::CONTROLLER_TYPE_JOYSTICK1 && type <= GameController::CONTROLLER_TYPE_JOYSTICK4)
	{
		std::string joystick_desc = getLocaleGuiString("gui_optionsmenu_controllertype_joystick");
		int id = type - GameController::CONTROLLER_TYPE_JOYSTICK1 + 1;
		controllerTypeButton->SetText((joystick_desc + " " + boost::lexical_cast<std::string>(id)).c_str());
	}
	else
	{
		std::string keyboard_desc = getLocaleGuiString("gui_optionsmenu_controllertype_keyboard");
		controllerTypeButton->SetText(keyboard_desc.c_str());
	}
#endif
}

void OptionsMenu::setControllerType(int type)
{
	if(type < GameController::CONTROLLER_TYPE_KEYBOARD_ONLY || type > GameController::CONTROLLER_TYPE_JOYSTICK4)
		return;

	currentController = (GameController::CONTROLLER_TYPE)type;

	gameController->setControllerType( (GameController::CONTROLLER_TYPE)currentController );
	updateControllerTypeText();

	for(unsigned int i = 0; i < controlNumArray.size(); i++)
	{
		int key = controlNumArray[ i ];
		// convert keys to new controller
		int keycode_0 = gameController->convertKeyToController(gameController->getBoundKey( key, 0 ), (GameController::CONTROLLER_TYPE)currentController);
		// int keycode_1 = gameController->convertKeyToController(gameController->getBoundKey( key, 1 ), (GameController::CONTROLLER_TYPE)currentController);
		gameController->unbindKey( key,  0 );
		gameController->unbindKey( key,  1 );
		gameController->bindKey( key, keycode_0, 0 );
	}
	readControls();
	updateControlDescriptions();

	if( currentController >= GameController::CONTROLLER_TYPE_JOYSTICK1 && currentController <= GameController::CONTROLLER_TYPE_JOYSTICK4 )
	{
		createJoystickButtons();
	}
	else
	{
		freeJoystickButtons();
	}
}

void OptionsMenu::openControllerTypeList()
{
	if(controllerTypeList)
		delete controllerTypeList;

	if(controllerTypeListCaptureEvents)
		delete controllerTypeListCaptureEvents;

	controllerTypeListCaptureEvents = ogui->CreateSimpleImageButton( win, getLocaleGuiInt( "gui_optionsmenu_window_x", 0 ), getLocaleGuiInt( "gui_optionsmenu_window_y", 0 ), getLocaleGuiInt( "gui_optionsmenu_window_w", 1024 ), getLocaleGuiInt( "gui_optionsmenu_window_h", 768 ), NULL, NULL, NULL, 0 );
	controllerTypeListCaptureEvents->SetListener( this );

	int num_mouses = Keyb3_GetNumberOfMouseDevices();
	if(num_mouses > 1) num_mouses--;

	if(controllerTypeListStyle == NULL)
	{
		OguiButtonStyle* unselStyle =		loadStyle( "gui_optionsmenu_controllertype_unselected" );
		OguiButtonStyle* selStyle =			loadStyle( "gui_optionsmenu_controllertype_selected" );
		OguiButtonStyle* newStyle =			loadStyle( "gui_optionsmenu_controllertype_selected" );
		OguiButtonStyle* newUnStyle =		loadStyle( "gui_optionsmenu_controllertype_selected" );
		OguiButtonStyle* scrollUpStyle =	loadStyle( "gui_optionsmenu_controllertype_arrow_up" );
		OguiButtonStyle* scrollDownStyle =	loadStyle( "gui_optionsmenu_controllertype_arrow_down" );

		int num_of_elements = 5;
	
		controllerTypeListStyle = new OguiSelectListStyle( unselStyle, selStyle, newStyle, newUnStyle, scrollUpStyle, scrollDownStyle, unselStyle->sizeX, unselStyle->sizeY * num_of_elements, scrollUpStyle->sizeX, scrollUpStyle->sizeY );
	}

	int x = controllerTypeButton->GetX();
	int y = controllerTypeButton->GetY();
	controllerTypeList = ogui->CreateSelectList( win, x, y, controllerTypeListStyle, 0, NULL, NULL );
	controllerTypeList->setListener( this );

	// keyboard controller
	std::string keyboard_desc = getLocaleGuiString("gui_optionsmenu_controllertype_keyboard");
	std::string value = boost::lexical_cast<std::string>(GameController::CONTROLLER_TYPE_KEYBOARD_ONLY);
	controllerTypeList->addItem(value.c_str(), keyboard_desc.c_str());

	// mouse controllers
	for(int i = 0; i < num_mouses; i++)
	{
		std::string mouse_desc = getLocaleGuiString("gui_optionsmenu_controllertype_mouse");
		std::string item = mouse_desc + " " + boost::lexical_cast<std::string>(i + 1);
		std::string value = boost::lexical_cast<std::string>(GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 + i);
		controllerTypeList->addItem(value.c_str(), item.c_str());
	}

	// joystick controllers
	for(int i = 0; i < JoyNum; i++)
	{
		std::string joystick_desc = getLocaleGuiString("gui_optionsmenu_controllertype_joystick");
		std::string item = joystick_desc + " " + boost::lexical_cast<std::string>(i + 1);
		std::string value = boost::lexical_cast<std::string>(GameController::CONTROLLER_TYPE_JOYSTICK1 + i);
		controllerTypeList->addItem(value.c_str(), item.c_str());
	}
}

void OptionsMenu::closeControllerTypeList()
{
	if(controllerTypeList)
		delete controllerTypeList;
	controllerTypeList = NULL;

	if(controllerTypeListCaptureEvents)
		delete controllerTypeListCaptureEvents;
	controllerTypeListCaptureEvents = NULL;
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace
