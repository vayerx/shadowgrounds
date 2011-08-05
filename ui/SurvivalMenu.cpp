
#include "precompiled.h"

#include <sstream>
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <algorithm>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "SurvivalMenu.h"
#include "CoopMenu.h"

#include "../system/Timer.h"
#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/savegamevars.h"
#include "../game/GameUI.h"
#include "MenuCollection.h"
#include "../game/DHLocaleManager.h"
#include "../ui/GameController.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_game.h"
#include "../game/options/options_gui.h"
#include "../game/GameStats.h"
#include "../game/scripting/GameScripting.h"
#include "../game/BonusManager.h"

#include "../editor/file_wrapper.h"
#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"
#include "../util/StringUtil.h"
#include "../util/Parser.h"
#include "../game/options/options_locale.h"
#include "../filesystem/input_stream_wrapper.h"

#include "../ogui/OguiLocaleWrapper.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiCheckBox.h"
#include "../filesystem/file_package_manager.h"
#include "../ui/LoadingMessage.h"

#include "CharacterSelectionWindow.h"

#include "../survivor/SurvivorConfig.h"

#include "../game/userdata.h"

using namespace game;

namespace ui
{
	bool SurvivalMenu::startAsCoop = false;
	SurvivalMenu::MissionInfo SurvivalMenu::lastLoadedMission;

SurvivalMenu::SurvivalMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
	MenuBaseImpl( NULL ),
	scrollUpArrow( NULL ),
	scrollDownArrow( NULL ),
	doubleClickHack( -1 ),
	doubleClickTimeHack( 0 ),
	scrollCount( 0 ),
	scrollPosition( 0 ),
	buttonYMinLimit( 64 ),
	buttonYMaxLimit( 450 ),
	menuCollection( menu ),
	fonts( fonts ),
	locked_font( NULL )
{
	assert( o_gui );
	assert( menu );
	assert( fonts );
	assert( g );

	game = g;
	ogui = o_gui;
	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_survivalmenu_window_x", 0 ), getLocaleGuiInt( "gui_survivalmenu_window_y", 0 ), getLocaleGuiInt( "gui_survivalmenu_window_w", 1024 ), getLocaleGuiInt( "gui_survivalmenu_window_h", 768 ), NULL );
	win->Hide();
	win->SetUnmovable();

	menu->setBackgroundImage( getLocaleGuiString( "gui_survivalmenu_background_image" ) );

	if(startAsCoop) addHeaderText( getLocaleGuiString( "gui_survivalmenu_header_coop" ), fonts->big.normal );
	else addHeaderText( getLocaleGuiString( "gui_survivalmenu_header" ), fonts->big.normal );
	

	buttonX	= getLocaleGuiInt( "gui_survivalmenu_button_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_survivalmenu_button_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_survivalmenu_button_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_survivalmenu_button_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

	buttonXStart = buttonX;
	buttonXLimit= getLocaleGuiInt( "gui_survivalmenu_buttons_max_width", 0 ); 

	buttonAddX = getLocaleGuiInt( "gui_survivalmenu_button_add_x", getLocaleGuiInt( "gui_menu_common_button_add_x", 0 ) );
	buttonAddY = getLocaleGuiInt( "gui_survivalmenu_button_add_y", getLocaleGuiInt( "gui_menu_common_button_add_y", 28 ) );

	buttonYMinLimit = getLocaleGuiInt( "gui_survivalmenu_button_ymin_limit", 0 );
	buttonYMaxLimit = getLocaleGuiInt( "gui_survivalmenu_button_ymax_limit", 768 );

	if( game->inCombat )
	{
		closeMenuByEsc = false;
	}
	else
	{
		closeMenuByEsc = true;
		editHandle = game->gameUI->getController(0)->addKeyreader( this );
		debugKeyreader( editHandle, false, "SurvivalMenu::SurvivalMenu()" );
	}

	createMissionButtons();

	// load button
	//
	{
		const int x	= getLocaleGuiInt( "gui_survivalmenu_loadbutton_x", 0 );
		const int y	= getLocaleGuiInt( "gui_survivalmenu_loadbutton_y", 0 );
		const int w	= getLocaleGuiInt( "gui_survivalmenu_loadbutton_w", 0 );
		const int h	= getLocaleGuiInt( "gui_survivalmenu_loadbutton_h", 0 );
		std::string buttonText = getLocaleGuiString( "gui_survivalmenu_load" );
		OguiButton* b;
		std::string imageCrap;
		b = ogui->CreateSimpleTextButton( win, x, y, w, h, 
			imageCrap.c_str(), imageCrap.c_str(), imageCrap.c_str(), buttonText.c_str(), COMMANDS_LOAD );
		b->SetListener( this );

		if( fonts->medium.normal ) b->SetFont( fonts->medium.normal );
		if( fonts->medium.highlighted ) b->SetHighlightedFont( fonts->medium.highlighted );
		if( fonts->medium.down ) b->SetDownFont( fonts->medium.down );
		if( fonts->medium.disabled ) b->SetDisabledFont( fonts->medium.disabled );

		b->SetEventMask( OGUI_EMASK_CLICK |  OGUI_EMASK_OVER );
		b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		buttons.push_back(b);
	}


	showCustomMissions = new OguiCheckBox( win, ogui, 
			getLocaleGuiInt( "gui_survivalmenu_showcustom_x", 0 ), 
			getLocaleGuiInt( "gui_survivalmenu_showcustom_y", 0 ), 
			getLocaleGuiInt( "gui_survivalmenu_showcustom_w", 0 ), 
			getLocaleGuiInt( "gui_survivalmenu_showcustom_h", 0 ), 	
			getLocaleGuiString( "gui_survivalmenu_showcustom_img_norm" ), "", "", 
			getLocaleGuiString( "gui_survivalmenu_showcustom_img_fill" ));

	showCustomMissions->setText( getLocaleGuiString( "gui_survivalmenu_showcustom_text" ), OguiCheckBox::TEXT_ALIGN_LEFT, getLocaleGuiInt( "gui_survivalmenu_showcustom_textwidth", 200 ), fonts->little.highlighted );
	showCustomMissions->setValue(SimpleOptions::getBool(DH_OPT_B_GUI_SHOW_CUSTOM_SURVIVAL_MISSIONS));
	showCustomMissions->setListener( this );

	createTexts();
	updateArrows();
}

SurvivalMenu::~SurvivalMenu()
{
	if( closeMenuByEsc )
	{
		game->gameUI->getController(0)->removeKeyreader( editHandle );
		debugKeyreader( editHandle, true, "SurvivalMenu::~SurvivalMenu()" );
	}

	delete showCustomMissions;

	std::map< int, OguiButton* >::iterator i;
	for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
	{
		delete i->second;
	}

	if(infoText)
	{
		infoText->deleteRegisteredFonts();
		delete infoText->getFont();
		delete infoText;
	}
	if(scoreText)
	{
		scoreText->deleteRegisteredFonts();
		delete scoreText->getFont();
		delete scoreText;
	}

	while( !buttons.empty() )
	{
		delete *(buttons.begin());
		buttons.pop_front();
	}

	for(unsigned int i = 0; i < starButtons.size(); i++)
		delete starButtons[i];
	starButtons.clear();

	delete headerText;

	delete locked_font;

	delete win;
}

//.............................................................................

int SurvivalMenu::getType() const
{
	return MenuCollection::MENU_TYPE_SURVIVALMENU;
}

//.............................................................................

void SurvivalMenu::closeMenu()
{
	assert( menuCollection );

	menuCollection->closeMenu();
}

void SurvivalMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

void SurvivalMenu::applyChanges()
{

}

//.............................................................................
void SurvivalMenu::selectButton( int command )
{
	int ex_selection = activeSelection;
	MenuBaseImpl::selectButton( command );

	// unselect old button
	std::map< int, OguiButton* >::iterator it;
	it = selectButtons.find( ex_selection );
	if( it != selectButtons.end() )
	{
		it->second->SetSelected( false );
	}

	// select new button
	it = selectButtons.find( activeSelection );
	if( it != selectButtons.end() )
	{
		it->second->SetSelected( true );
	}

	// delete existing stars
	for(unsigned int i = 0; i < starButtons.size(); i++)
		delete starButtons[i];
	starButtons.clear();

	if(activeSelection > 0 && (unsigned)activeSelection < missionInfos.size())
	{
		std::string desc = missionInfos[activeSelection].description;
		if(missionInfos[activeSelection].locked)
			desc += std::string(getLocaleGuiString("gui_survivalmenu_unlock_instruction"));
		infoText->setText(desc);

		std::string title = getLocaleGuiString( "gui_survivalmenu_scores" );
		scoreText->setText("<h1>" + title + "<br></h1><br>"+ missionInfos[activeSelection].scores);
		// 0 title<br>
		// 1 <br>
		// 2 name<br>
    // 3 time + score<br>
		// 4 <br>
		// 5 name<br>
    // 6 time + score<br>
		// 7 <br>
		// 8 name<br>
    // 9 time + score<br>
		// 10 <br>

		// score limits
		int developer, ultimate;
		MissionInfo &mi = missionInfos[activeSelection];
		std::string dirname = boost::to_lower_copy( mi.dir );
		if(GameStats::getScoreLimits(dirname.c_str(), developer, ultimate))
		{
			for(unsigned int i = 0; i < mi.scoreNumbers.size(); i++)
			{
				if(mi.scoreNumbers[i] > ultimate)
				{
					int x = getLocaleGuiInt("gui_survivalmenu_ultimatestar_x", 0) + scoreText->getX();
					int y = getLocaleGuiInt("gui_survivalmenu_ultimatestar_y", 0) + scoreText->getLinePositionY(2 + i * 3);
					int w = getLocaleGuiInt("gui_survivalmenu_ultimatestar_w", 0);
					int h = getLocaleGuiInt("gui_survivalmenu_ultimatestar_h", 0);
					const char *img = getLocaleGuiString("gui_survivalmenu_ultimatestar_img");
					OguiButton *but = ogui->CreateSimpleImageButton(win, x, y, w, h, NULL, NULL, NULL, img, 0, 0, false);
					but->SetDisabled(true);
					starButtons.push_back(but);
				}
				else if(mi.scoreNumbers[i] > developer)
				{
					int x = getLocaleGuiInt("gui_survivalmenu_developerstar_x", 0) + scoreText->getX();
					int y = getLocaleGuiInt("gui_survivalmenu_developerstar_y", 0) + scoreText->getLinePositionY(2 + i * 3);
					int w = getLocaleGuiInt("gui_survivalmenu_developerstar_w", 0);
					int h = getLocaleGuiInt("gui_survivalmenu_developerstar_h", 0);
					const char *img = getLocaleGuiString("gui_survivalmenu_developerstar_img");
					OguiButton *but = ogui->CreateSimpleImageButton(win, x, y, w, h, NULL, NULL, NULL, img, 0, 0, false);
					but->SetDisabled(true);
					starButtons.push_back(but);
				}
			}
		}
	}
}

void SurvivalMenu::checkBoxEvent( OguiCheckBoxEvent* eve )
{
	if(eve->checkBox == showCustomMissions)
	{
		SimpleOptions::setBool(DH_OPT_B_GUI_SHOW_CUSTOM_SURVIVAL_MISSIONS, eve->value);

		std::map< int, OguiButton* >::iterator i;
		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{
			delete i->second;
		}
		selectButtons.clear();
		
		scrollCount = 0;
		scrollPosition = 0;

		buttonX	= getLocaleGuiInt( "gui_survivalmenu_button_x", 0 );
		buttonY	= getLocaleGuiInt( "gui_survivalmenu_button_y", 0 );
		buttonW	= getLocaleGuiInt( "gui_survivalmenu_button_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
		buttonH	= getLocaleGuiInt( "gui_survivalmenu_button_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

		createMissionButtons();
		updateArrows();
		infoText->setText("");
		scoreText->setText("");
		for(unsigned int i = 0; i < starButtons.size(); i++)
			delete starButtons[i];
		starButtons.clear();
	}
}

void SurvivalMenu::CursorEvent( OguiButtonEvent* eve )
{
	MenuBaseImpl::CursorEvent( eve );

	if( eve->eventType == OGUI_EMASK_CLICK )
	{
		if(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK)
		{
			scrollMissionsUp();
			updateArrows();
		}
		else if(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK)
		{
			scrollMissionsDown();
			updateArrows();
		}
		else switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_CLOSEME:
			menuClose();
			break;

		case COMMANDS_LOAD:
			menuLoad();
			break;

		case COMMANDS_ARROWDOWN:
			scrollMissionsDown();
			updateArrows();
			break;
		
		case COMMANDS_ARROWUP:
			scrollMissionsUp();
			updateArrows();
			break;
		default:
			{
				if( doubleClickHack == eve->triggerButton->GetId() && ( Timer::getTime() - doubleClickTimeHack ) < 500  )
				{
					menuLoad();
				} else {
					// select new button
					MenuBaseImpl::CursorEvent( eve );
					doubleClickHack = activeSelection;
					doubleClickTimeHack = Timer::getTime();
				}
				return;
			}
		}
	}
	
}

//.............................................................................

void SurvivalMenu::menuClose()
{
	closeMenu(); 
}

void SurvivalMenu::reloadLastMission(game::Game *game)
{
	loadMission(lastLoadedMission, game);
}

void SurvivalMenu::menuLoad()
{
	if(activeSelection > 0 && (unsigned)activeSelection < missionInfos.size())
	{
		loadMission(missionInfos[activeSelection], game);
	}
}

void SurvivalMenu::loadMission(MissionInfo &mi, game::Game *game)
{
	if(mi.locked)
		return;

	if(mi.dir.empty())
		return;

	std::string file = mi.dir + "/save.dhs";
	// bleh hax
	for(unsigned int i = 0; i < file.size(); i++)
	{
		if(file[i] == '\\')
		{
			file[i] = '/';
		}
	}

	LoadingWindow::showCharacterSelection = false;
	LoadingWindow::autoCloseEnabled = false;
	LoadingWindow::showUpgradeWindowOnClose = false;

	// character selection
	if(!mi.characters.empty())
	{
		CharacterSelectionWindow::parseChoices(mi.characters.c_str());
		LoadingWindow::showCharacterSelection = true;
		LoadingWindow::autoCloseEnabled = true;
	}

	if(mi.show_upgradewindow)
	{
		LoadingWindow::showUpgradeWindowOnClose = true;
		LoadingWindow::autoCloseEnabled = true;
	}


	if(startAsCoop)
	{
		CoopMenu::enableCoopGameSettings(game);
		GameStats::setCurrentScoreFile((mi.dir + "/score_coop.dat").c_str());
	}
	else
	{
		CoopMenu::disableCoopGameSettings(game);
		GameStats::setCurrentScoreFile((mi.dir + "/score.dat").c_str());
	}

	// no bonuses in survival
	game->bonusManager->deactivateAll();

	if(game->loadGame(file.c_str()))
	{
		lastLoadedMission = mi;
		startAsCoop = false;
	}
	else
	{
		// in case we can't load, reset some stuff
		LoadingWindow::showCharacterSelection = false;
		LoadingWindow::autoCloseEnabled = false;
		LoadingWindow::showUpgradeWindowOnClose = false;
	}

	// must do these after loading (in savegame), because unloading
	// the last level would otherwise cause these to be lost
	//
	//game->gameScripting->newGlobalIntVariable("survival_mode_enabled", false);
	//game->gameScripting->setGlobalIntVariableValue("survival_mode_enabled", 1);
}


void SurvivalMenu::createTexts()
{
	// info text
	{
		const int x	= getLocaleGuiInt( "gui_survivalmenu_infotext_x", 0 );
		const int y	= getLocaleGuiInt( "gui_survivalmenu_infotext_y", 0 );
		const int w	= getLocaleGuiInt( "gui_survivalmenu_infotext_w", 0 );
		const int h	= getLocaleGuiInt( "gui_survivalmenu_infotext_h", 0 );

		const std::string font_normal = getLocaleGuiString( "gui_survivalmenu_infotext_font_normal" );
		const std::string font_b = getLocaleGuiString( "gui_survivalmenu_infotext_font_bold" );
		const std::string font_i = getLocaleGuiString( "gui_survivalmenu_infotext_font_italic" );
		const std::string font_h1 = getLocaleGuiString( "gui_survivalmenu_infotext_font_h1" );
		
		infoText = new OguiFormattedText( win, ogui, x, y, w, h, 0 );
		
		if( font_normal.empty() == false )
			infoText->setFont( ogui->LoadFont( font_normal.c_str() ) );
		
		if( font_b.empty() == false )
			infoText->registerFont( "b", ogui->LoadFont( font_b.c_str() ) );
		
		if( font_i.empty() == false )
			infoText->registerFont( "i", ogui->LoadFont( font_i.c_str() ) );
		
		if( font_h1.empty() == false )
			infoText->registerFont( "h1", ogui->LoadFont( font_h1.c_str() ) );
	}

	// high score text
	{
		const int x	= getLocaleGuiInt( "gui_survivalmenu_scoretext_x", 0 );
		const int y	= getLocaleGuiInt( "gui_survivalmenu_scoretext_y", 0 );
		const int w	= getLocaleGuiInt( "gui_survivalmenu_scoretext_w", 0 );
		const int h	= getLocaleGuiInt( "gui_survivalmenu_scoretext_h", 0 );

		const std::string font_normal = getLocaleGuiString( "gui_survivalmenu_scoretext_font_normal" );
		const std::string font_b = getLocaleGuiString( "gui_survivalmenu_scoretext_font_bold" );
		const std::string font_i = getLocaleGuiString( "gui_survivalmenu_scoretext_font_italic" );
		const std::string font_h1 = getLocaleGuiString( "gui_survivalmenu_scoretext_font_h1" );
		
		scoreText = new OguiFormattedText( win, ogui, x, y, w, h, 0 );
		
		if( font_normal.empty() == false )
			scoreText->setFont( ogui->LoadFont( font_normal.c_str() ) );
		
		if( font_b.empty() == false )
			scoreText->registerFont( "b", ogui->LoadFont( font_b.c_str() ) );
		
		if( font_i.empty() == false )
			scoreText->registerFont( "i", ogui->LoadFont( font_i.c_str() ) );
		
		if( font_h1.empty() == false )
			scoreText->registerFont( "h1", ogui->LoadFont( font_h1.c_str() ) );
	}
}

void SurvivalMenu::addImageSelectionButton( const std::string& image_norm, const std::string& image_high, const std::string& image_down, const std::string& image_disabled, bool disabled, int command, IOguiFont* font, void* param )
{
	assert( ogui );
	assert( win );
	assert( command >= 0 );

	if( command > numberOfWorkingSelectButtons ) numberOfWorkingSelectButtons = command;

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, buttonX, buttonY, buttonW, buttonH, image_norm.c_str(), image_down.c_str(), image_high.c_str(), "", command, param, true );
	b->SetListener( this );
	b->SetDisabledImage( ogui->LoadOguiImage(image_disabled.c_str()) );
	b->SetImageAutoDelete(true, true, true, true);
	b->SetDisabled( disabled );
	b->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );
	b->SetClipToWindow( false );
	b->Move( buttonX, buttonY );
	
	if( buttonX == buttonXStart && buttonY > buttonYMaxLimit )
	{
		scrollCount++;
	}

	if( buttonY > buttonYMaxLimit )
	{
		b->MoveBy( 0, 1000 );
	}

	buttonX += buttonAddX;

	if( buttonX > buttonXLimit )
	{
		buttonY += buttonAddY;

		buttonX = buttonXStart;
	}

	selectButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

bool SurvivalMenu::loadMissionInfo(const std::string &directory, MissionInfo &mi)
{
	// ignore template
	if(directory == "Template" || directory == "template")
	{
		return false;
	}

	const char *locale_id = DHLocaleManager::getInstance()->getLocaleIdString(SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE));
	std::string description_key = "description_" + std::string(locale_id);

	std::string root = "Survival";
	std::string dir = root + "/" + directory;
	std::string optionsfile = dir + "/mission_options.txt";
	std::string scorefile = dir + "/score.dat";
	if(startAsCoop)
	{
		scorefile = dir + "/score_coop.dat";
	}
	util::SimpleParser parser;
	if(!parser.loadFile(optionsfile.c_str()))
	{
		return false;
	}

#ifdef PROJECT_SURVIVOR_DEMO
	unsigned int crc = frozenbyte::filesystem::FilePackageManager::getInstance().getCrc(optionsfile);
	if(crc == 0xFFFFFFFF)
		return false;
#endif

	std::string description;
	std::string characters;
	std::string scores;
	bool show_upgradewindow = true;

	while (parser.next())
	{
		char *k = parser.getKey();
		if(k == NULL)
			continue;

		if(strcmp(k, "thumbnail_norm") == 0)
			mi.thumbnail_norm = parser.getValue();
		else if(strcmp(k, "thumbnail_high") == 0)
			mi.thumbnail_high = parser.getValue();
		else if(strcmp(k, "thumbnail_down") == 0)
			mi.thumbnail_down = parser.getValue();
		else if(strcmp(k, "thumbnail_selected_norm") == 0)
			mi.thumbnail_selected_norm = parser.getValue();
		else if(strcmp(k, "thumbnail_selected_high") == 0)
			mi.thumbnail_selected_high = parser.getValue();
		else if(strcmp(k, description_key.c_str()) == 0)
			description = parser.getValue();
		else if(strcmp(k, "characters") == 0)
			characters = parser.getValue();
		else if(strcmp(k, "show_upgradewindow") == 0)
			show_upgradewindow = strcmp(parser.getValue(), "0") == 0 ? false : true;
	}

	std::vector<GameStats::ScoreData> scoreArray;
	GameStats::loadScores(scorefile.c_str(), scoreArray);
	unsigned int numScores = scoreArray.size();
	// max 3 scores
	if(numScores > 3) numScores = 3;
	mi.scoreNumbers.resize(numScores);
	for(unsigned int j = 0; j < numScores; j++)
	{
		scores += "<b>" + scoreArray[j].name + "</b><br>" +
							scoreArray[j].time + " (" +
							scoreArray[j].score + " p)<br><br>";

		mi.scoreNumbers[j] = atoi(scoreArray[j].score.c_str());
	}
	

	mi.show_upgradewindow = show_upgradewindow;
	mi.dir = dir;
	mi.characters = characters;
	mi.description = description;
	mi.scores = scores;
	mi.locked = false;
	return true;
}

void SurvivalMenu::createMissionButtons()
{
	std::string root = "Survival";
	frozenbyte::editor::FileWrapper fileWrapper(root, "*.dhs");
	std::vector<std::string> dirs;
	dirs = fileWrapper.getAllDirs();


	int time_loading_start = Timer::getCurrentTime();
	bool show_loading_bar = false;

	std::vector<std::string> lockedMissions;
	readLockedMissions(lockedMissions);

	const char *locked_img_norm = getLocaleGuiString("gui_survivalmenu_locked_button_image_norm");
	const char *locked_img_down = getLocaleGuiString("gui_survivalmenu_locked_button_image_down");
	const char *locked_img_high = getLocaleGuiString("gui_survivalmenu_locked_button_image_high");
	const char *locked_img_selected_norm = getLocaleGuiString("gui_survivalmenu_locked_button_image_selected_norm");
	const char *locked_img_selected_high = getLocaleGuiString("gui_survivalmenu_locked_button_image_selected_high");
	locked_font = ogui->LoadFont(getLocaleGuiString("gui_survivalmenu_locked_button_font"));
	const char *locked_text = getLocaleGuiString("gui_survivalmenu_locked_button_text");

	bool show_custom = SimpleOptions::getBool(DH_OPT_B_GUI_SHOW_CUSTOM_SURVIVAL_MISSIONS);

	const char *standard_missions[] =
	{
		"surv_spider",
		"surv_shriek",
		"surv_overtime",
		"surv_comecloser",
		"surv_clawsoff",
		"surv_allover",
		"surv_forest",
		"surv_techfacil",
		"surv_snowstora",
		NULL,
	};

	// reorder missions
	{
		// collect custom dirs
		std::vector<std::string> custom_dirs;
		if(show_custom)
		{
			//unsigned int insert_str = 0;
			for(unsigned int i = 0; i < dirs.size(); i++)
			{
				std::string dirname = boost::to_lower_copy( dirs[i] );
				bool is_standard = false;
				for(unsigned int j = 0; standard_missions[j] != NULL; j++)
				{
					if(dirname == standard_missions[j])
					{
						is_standard = true;
						break;
					}
				}
				if(!is_standard)
				{
					custom_dirs.push_back(dirname);
				}			
			}
		}
		dirs.clear();

		// insert standard dirs
		for(unsigned int j = 0; standard_missions[j] != NULL; j++)
		{
			dirs.push_back(standard_missions[j]);
		}

		// insert custom dirs
		dirs.insert(dirs.end(), custom_dirs.begin(), custom_dirs.end());
	}

	missionInfos.resize(dirs.size() + 1);
	unsigned int numDirs = dirs.size();
	for(unsigned int i = 0; i < numDirs; i++)
	{
		if(!loadMissionInfo(dirs[i], missionInfos[i+1]))
		{
      // printf("WARNING: Loading of mission info for %s failed!\n",dirs[i].c_str());
			continue;
		}

		if(std::find(lockedMissions.begin(), lockedMissions.end(), dirs[i]) != lockedMissions.end())
		{
			// locked
			missionInfos[i+1].locked = true;
			this->addImageSelectionButton(locked_img_norm, locked_img_high, locked_img_down, locked_img_down, false, i+1, NULL);

			if( selectButtons[ i+1 ] )
			{
				selectButtons[ i+1 ]->SetSelectedImages( ogui->LoadOguiImage( locked_img_selected_norm ), ogui->LoadOguiImage( locked_img_selected_high ) );
				selectButtons[ i+1 ]->SetFont( locked_font );
				selectButtons[ i+1 ]->SetDownFont( locked_font );
				selectButtons[ i+1 ]->SetDisabledFont( locked_font );
				selectButtons[ i+1 ]->SetHighlightedFont( locked_font );
				selectButtons[ i+1 ]->SetText(locked_text);
			}
		}
		else
		{
			missionInfos[i+1].locked = false;
			this->addImageSelectionButton( missionInfos[i+1].thumbnail_norm, missionInfos[i+1].thumbnail_high, missionInfos[i+1].thumbnail_down, missionInfos[i+1].thumbnail_down, false, i+1, NULL );

			if( selectButtons[ i+1 ] )
				selectButtons[ i+1 ]->SetSelectedImages( ogui->LoadOguiImage( missionInfos[i+1].thumbnail_selected_norm.c_str() ), ogui->LoadOguiImage( missionInfos[i+1].thumbnail_selected_high.c_str() ) );
		}

		if(i == 4)
		{
			// loading first 5 entries took longer than 100 ms
			if(Timer::getCurrentTime() - time_loading_start > 100)
			{
				// show loading bar
				show_loading_bar = true;
			}
		}

		if(show_loading_bar && (i+1)%5 == 0)
		{
			SET_LOADING_BAR_TEXT(getLocaleGuiString("gui_loadingbar_loading"));
			SHOW_LOADING_BAR((i+1)*100/numDirs);
		}
	}
}

//.............................................................................

void SurvivalMenu::scrollMissionsDown()
{
	// scroll++;
	if( scrollPosition < scrollCount )
	{
		scrollPosition++;
		std::map< int, OguiButton* >::iterator i;
		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{	
			i->second->MoveBy( 0, -buttonAddY );
			if( i->second->GetY() > buttonYMinLimit - 900 && i->second->GetY() < buttonYMinLimit )
			{
				i->second->MoveBy( 0, -1000 );
			}

			if( i->second->GetY() > buttonYMaxLimit && i->second->GetY() < buttonYMaxLimit + 1000 )
			{
				i->second->MoveBy( 0, -1000 );
			}
		}
	}
}

void SurvivalMenu::scrollMissionsUp()
{
	if( scrollPosition > 0 )
	{
		scrollPosition--;
		std::map< int, OguiButton* >::iterator i;
		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{
			i->second->MoveBy( 0, buttonAddY );
			if( i->second->GetY() < buttonYMaxLimit + 900 && i->second->GetY() > buttonYMaxLimit )
			{
				i->second->MoveBy( 0, +1000 ); 
			}

			if( i->second->GetY() < buttonYMinLimit && i->second->GetY() >= buttonYMinLimit - 1000 )
			{
				i->second->MoveBy( 0, +1000 );
			}
		}
	}
}

void SurvivalMenu::updateArrows()
{
	// scroll buttons
	//
	if( scrollCount > 0 && scrollUpArrow == NULL)
	{
		const int x = getLocaleGuiInt( "gui_survivalmenu_arrow_up_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivalmenu_arrow_up_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivalmenu_arrow_up_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivalmenu_arrow_up_h", 0 );

		const std::string image_norm = getLocaleGuiString( "gui_survivalmenu_arrow_up_image_norm" );
		const std::string image_down = getLocaleGuiString( "gui_survivalmenu_arrow_up_image_down" );
		const std::string image_high = getLocaleGuiString( "gui_survivalmenu_arrow_up_image_high" );
		const std::string image_disa = getLocaleGuiString( "gui_survivalmenu_arrow_up_image_disa" );

		scrollUpArrow = this->addImageButtton( image_norm, image_down, image_high, image_disa, COMMANDS_ARROWUP, x, y, w, h );
	}
	if( scrollCount > 0 && scrollDownArrow == NULL)
	{
		const int x = getLocaleGuiInt( "gui_survivalmenu_arrow_down_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivalmenu_arrow_down_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivalmenu_arrow_down_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivalmenu_arrow_down_h", 0 );

		const std::string image_norm = getLocaleGuiString( "gui_survivalmenu_arrow_down_image_norm" );
		const std::string image_down = getLocaleGuiString( "gui_survivalmenu_arrow_down_image_down" );
		const std::string image_high = getLocaleGuiString( "gui_survivalmenu_arrow_down_image_high" );
		const std::string image_disa = getLocaleGuiString( "gui_survivalmenu_arrow_down_image_disa" );

		scrollDownArrow = this->addImageButtton( image_norm, image_down, image_high, image_disa, COMMANDS_ARROWDOWN, x, y, w, h );
	}

	if( scrollUpArrow )
	{
		if( scrollPosition > 0 )
		{
			scrollUpArrow->SetDisabled( false );
		}
		else
		{
			scrollUpArrow->SetDisabled( true );
		}
	}
	
	if( scrollDownArrow )
	{
		if( scrollPosition < scrollCount )
		{
			scrollDownArrow->SetDisabled( false );
		}
		else
		{
			scrollDownArrow->SetDisabled( true );
		}
	}
}

void SurvivalMenu::unlockMission(const std::string &mission)
{
	FILE *file = fopen(igios_mapUserDataPrefix("Config/unlocked_missions.dat").c_str(), "ab");
	if(file)
	{
		char buf[256];
		sprintf(buf, "%s$", mission.c_str());

		// encrypt
		for(unsigned int i = 0; i < 256; i++)
		{
			if(buf[i] == 0) break;
			buf[i] = ~buf[i];
		}

		fprintf(file, "%s", buf);
		fclose(file);
	}
}

bool SurvivalMenu::readLockedMissions(std::vector<std::string> &missions)
{
	std::list<std::string> locked;
	locked.push_back("surv_clawsoff");
	locked.push_back("surv_comecloser");
	locked.push_back("surv_forest");
	locked.push_back("surv_overtime");
	locked.push_back("surv_shriek");
	locked.push_back("surv_snowstora");
	locked.push_back("surv_allover");
	locked.push_back("surv_techfacil");

	// open file
	frozenbyte::filesystem::FB_FILE *f = frozenbyte::filesystem::fb_fopen(igios_mapUserDataPrefix("Config/unlocked_missions.dat").c_str(), "rb");
	if(f != NULL)
	{
		int flen = frozenbyte::filesystem::fb_fsize(f);
		char *buf = (char *)alloca(flen + 1);
		int got = frozenbyte::filesystem::fb_fread(buf, sizeof(char), flen, f);
		frozenbyte::filesystem::fb_fclose(f);

		// parse
		int last = 0;
		for(int i = 0; i < got; i++)
		{
			// decrypt
			buf[i] = ~buf[i];
			if(buf[i] == '$')
			{
				buf[i] = 0;
				std::string k = buf + last;
				std::list<std::string>::iterator it = std::find(locked.begin(), locked.end(), k);
				if(it != locked.end())
				{
					locked.erase(it);
				}
				last = i+1;
			}
		}
	}

	missions.resize(locked.size());
	unsigned int i = 0;
	std::list<std::string>::iterator it;
	for(it = locked.begin(); it != locked.end(); it++)
	{
		missions[i] = *it;
		i++;
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui



