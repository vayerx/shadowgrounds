#include "precompiled.h"

#include <map>
#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "SurvivorUpgradeWindow.h"

#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UpgradeManager.h"
#include "../game/UpgradeType.h"
#include "../game/PlayerWeaponry.h"
#include "../game/Weapon.h"
#include "../game/scripting/GameScripting.h"
#include "../game/UnitInventory.h"
#include "../game/GameStats.h"

#include "../ogui/OguiStormDriver.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../ogui/OguiSlider.h"

#include "../util/TextureCache.h"
#include "../util/Parser.h"

#include "../util/fb_assert.h"
#include "../util/StringUtil.h"

#include "../game/DHLocaleManager.h"

using namespace game;

namespace ui {

	static const int FADE_IN_TIME = 500;
	static const int FADE_OUT_TIME = 500;
	
	static const int BUTTON_ID_UNDO = 5;
	static const int BUTTON_ID_CLOSE = 4;

	struct UpgradeButtonThingie
	{
		UpgradeButtonThingie() : 
			button( NULL ),
			selected_button( NULL ),
			text( NULL ),
			textFontNormal( NULL ),
			textFontDisabled( NULL ),
			hintMessage( "" ),
			upgradeId( -1 ),
			selected( false ),
			icon(0)
		{
		}

		~UpgradeButtonThingie()
		{
			delete selected_button;
			delete button;
			delete text;
			delete icon;
		}

		void setSelected( bool selected )
		{
			if( button )
			{
				this->selected = selected;

				// selection background button hack
				if(selected_button)
				{
					if(selected && !button->IsSelected())
					{
						button->SetSelected( selected );
						original_x = button->GetX();
						original_y = button->GetY();
						button->Move(original_x + selected_offset_x, original_y + selected_offset_y);
						selected_button->Move(original_x, original_y);
					}
					else
					{
						button->SetSelected( selected );
						button->Move(original_x, original_y);
						selected_button->Move(10000, 10000);
					}
				}
				else
				{
					button->SetSelected( selected );
				}
			}
		}

		void hideCostText(void)
		{
			if(text) 
			{
				text->SetText("");
				
				if(icon)
					icon->SetDisabled(true);
			}
		}

		void showCostText(game::Game* game, Unit *unit, int upgradesPendingCost)
		{
			if(text)
			{
				int cost = game->upgradeManager->getUpgradePartCost( upgradeId );
				std::string costText = boost::lexical_cast< std::string >(cost);
				if( game->upgradeManager->canUpgrade( unit, upgradeId, &upgradesPendingCost ) && !game->upgradeManager->isLocked(unit, upgradeId) )
					text->SetDisabledFont(textFontNormal);
				else 
					text->SetDisabledFont(textFontDisabled);

				text->SetText(costText.c_str());
				if(icon)
					icon->SetDisabled(false);
			}
		}

		OguiButton* button;
		OguiButton *selected_button;
		OguiButton* text;
		IOguiFont* textFontNormal;
		IOguiFont* textFontDisabled;
		std::string hintMessage;
		int upgradeId;
		bool selected;
		int original_x, original_y;
		int selected_offset_x, selected_offset_y;
		OguiButton* icon;
	};

	struct SurvivorItem
	{
		enum Type
		{
			TYPE_ITEM = 0,
			TYPE_VARIABLE = 1,
			TYPE_AMMO = 2,
			TYPE_TOGGLE_ITEM = 3
		};

		SurvivorItem() { }

		SurvivorItem( const std::string& variable, Type type, const std::string& locale, bool showCount ) :
			varName( variable ),
			varType( type ),
			localeName( locale ),
			createCountText( showCount ),
			button( NULL ),
			count_text( NULL )
		{ 
		}



		std::string varName;
		Type varType;

		std::string localeName;
		bool createCountText;
		UpgradeButtonThingie *button;
		OguiButton *count_text;

		std::pair<std::string, std::string> script_can_use;
		std::pair<std::string, std::string> script_use;
		std::string toggle_item_var;

		std::string use_text;
	};

///////////////////////////////////////////////////////////////////////////////

class SurvivorUpgradeWindow::SurvivorUpgradeWindowImpl : public IOguiButtonListener, private IOguiEffectListener
{
private:
	Ogui* ogui;
	game::Game* game;
	game::Unit* unit;
	std::string unit_name;
	int playerNum;

	OguiLocaleWrapper	oguiLoader;

	OguiWindow* window;
	std::list<OguiWindow *> decoWindows;

	OguiFormattedText* hintBox;
	OguiButton* undoButton;
	OguiButton* closeButton;
	OguiButton* insufficientPartsButton;

	OguiButton* upgradePartsAmount;
	OguiButton* characterPartsAmount;
	OguiFormattedText* characterStats;
	OguiFormattedText* characterStatsNumbers;

	OguiSlider*	expobarSlider;

	std::list< OguiButton* > decorationList;
	std::map< int, UpgradeButtonThingie* > buttonMap;

	std::list< OguiFormattedText* > decorationText;

	// these button are in decorationList, don't worry about deleting them
	OguiButton *maxHealthDeco;
	OguiButton *healthDeco;

	int max_hp;

	int visible;
	int idCount;
	int mouseOverHintButtonId;

	// upgrade crap
	std::list< int > upgradesPending;
	int upgradesPendingCost;
	int characterUpgradesPendingCost;

	int items_first_id;
	int items_last_id;

	// items for the players
	std::map< std::string, std::vector< SurvivorItem > > playerItemsMap;

	OguiButton *characterNameBackground;
	OguiButton *characterUpgradeBackground;
	OguiButton *weaponUpgradeBackground;

	IOguiImage *locked_img;

public:
	//-------------------------------------------------------------------------
	SurvivorUpgradeWindowImpl( Ogui *ogui, game::Game *game, game::Unit *unit ) :
		ogui( ogui ),
		game( game ),
		unit( unit ),
		oguiLoader( ogui ),

		window( NULL ),
		hintBox( NULL ),
		undoButton( NULL ),
		closeButton( NULL ),
		insufficientPartsButton( NULL ),

		upgradePartsAmount( NULL ),
		characterPartsAmount( NULL ),
		characterStats( NULL ),
		characterStatsNumbers( NULL ),
		expobarSlider( NULL ),

		decorationList(),
		buttonMap(),
		maxHealthDeco( NULL ),
		healthDeco( NULL ),
		visible( 1 ),
		idCount( 100 ),
		mouseOverHintButtonId( 0 ),

		upgradesPending(),
		upgradesPendingCost( 0 ),
		characterUpgradesPendingCost( 0 ),

		characterNameBackground(0),
		characterUpgradeBackground(0),
		weaponUpgradeBackground(0),
		locked_img(NULL)
	{
		// find player number
		for (playerNum = 0; playerNum < MAX_PLAYERS_PER_CLIENT; playerNum++)
		{
			if (unit == game->gameUI->getFirstPerson(playerNum)) break;
		}
		assert(playerNum < MAX_PLAYERS_PER_CLIENT);

		std::vector< SurvivorItem > defaultItems;
		defaultItems.resize(1);
		defaultItems[0] = SurvivorItem( "medikit", SurvivorItem::TYPE_ITEM, "item_medikit", true );
		defaultItems[0].script_can_use = std::pair<std::string, std::string>("item_medikit", "canuse");
		defaultItems[0].script_use = std::pair<std::string, std::string>("item_medikit", "useitem");
		defaultItems[0].use_text = getLocaleSubtitleString("hint_item_inventory_medikit_used");
		/*defaultItems[1] = SurvivorItem( "player_has_key_1", SurvivorItem::TYPE_VARIABLE, "item_keycard1", false );
		defaultItems[2] = SurvivorItem( "player_has_key_2", SurvivorItem::TYPE_VARIABLE, "item_keycard2", false );
		defaultItems[3] = SurvivorItem( "player_has_key_3", SurvivorItem::TYPE_VARIABLE, "item_keycard3", false );*/

		std::vector< SurvivorItem > napalmItems = defaultItems;
		std::vector< SurvivorItem > marineItems = defaultItems;
		std::vector< SurvivorItem > sniperItems = defaultItems;
		/*napalmItems[4] = SurvivorItem( "W_NGren", SurvivorItem::TYPE_AMMO, "ammo_", true );
		marineItems[4] = SurvivorItem( "W_MGren", SurvivorItem::TYPE_AMMO, "ammo_", true );
		sniperItems[4] = SurvivorItem( "W_SGren", SurvivorItem::TYPE_AMMO, "ammo_", true );*/

		sniperItems.push_back(SurvivorItem( "enemyintelligence", SurvivorItem::TYPE_TOGGLE_ITEM, "item_enemyintelligence", false ));
		sniperItems[sniperItems.size() - 1].script_use = std::pair<std::string, std::string>("item_enemyintelligence", "useitem");
		sniperItems[sniperItems.size() - 1].toggle_item_var = "enemyintelligence_in_use";

		playerItemsMap.insert( std::pair< std::string, std::vector< SurvivorItem > >( "surv_napalm", napalmItems ) );
		playerItemsMap.insert( std::pair< std::string, std::vector< SurvivorItem > >( "surv_marine", marineItems ) );
		playerItemsMap.insert( std::pair< std::string, std::vector< SurvivorItem > >( "surv_sniper", sniperItems ) );

		// debugging shiet
		//oguiLoader.SetLogging( true, "survivor_upgradewindow.txt" );

		loadDataForWindow( "upgradewindow" );

		unit_name = unit->getUnitType()->getName();
		unit_name = boost::to_lower_copy( unit_name );
		loadDataForUnit();
		
		loadWeaponUpgradesForUnit();

		// loadSelectionButton( "testi" );

		window->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, FADE_IN_TIME);
		window->SetEffectListener(this);

		updateTheButtons();
	}

	~SurvivorUpgradeWindowImpl()
	{
		delete locked_img;
		{
			std::map< int, UpgradeButtonThingie* >::iterator i;
			for( i = buttonMap.begin(); i != buttonMap.end(); ++i )
			{
				delete i->second;
			}
		}
		
		{
			std::list< OguiButton* >::iterator i;
			for( i = decorationList.begin(); i != decorationList.end(); ++i )
				delete (*i);
		}

		{
			std::list< OguiFormattedText* >::iterator i;
			for( i = decorationText.begin(); i != decorationText.end(); ++i )
				delete (*i);
		}
		
		delete expobarSlider;
		delete upgradePartsAmount;
		delete characterPartsAmount;
		delete characterStats;
		delete characterStatsNumbers;

		delete insufficientPartsButton;
		delete closeButton;
		delete undoButton;
		delete hintBox;
		{
			std::list<OguiWindow *>::iterator it;
			for( it = decoWindows.begin(); it != decoWindows.end(); it++)
			{
				delete (*it);
			}
		}
		delete window;
	}

	//............................................................................
	int getItemCount( SurvivorItem *item, std::string &localeName)
	{
		int itemCount = 0;
		localeName = item->localeName;

		if(item->varType == SurvivorItem::TYPE_ITEM || item->varType == SurvivorItem::TYPE_TOGGLE_ITEM)
		{
			itemCount = game::UnitInventory::getUnitItemCount(game, unit, item->varName.c_str());
		}
		else if(item->varType == SurvivorItem::TYPE_VARIABLE)
		{
			itemCount = game->gameScripting->getGlobalIntVariableValue(item->varName.c_str());
		}
		else if(item->varType == SurvivorItem::TYPE_AMMO)
		{
			if(PARTTYPE_ID_STRING_VALID(item->varName.c_str()))
			{
				int weapon_number = unit->getWeaponByWeaponType( PARTTYPE_ID_STRING_TO_INT(item->varName.c_str()) );
				if(weapon_number != -1)
				{
					itemCount = unit->getWeaponAmmoAmount(weapon_number);

					Weapon *weapon = unit->getWeaponType(weapon_number);
					if(weapon)
					{
						if(weapon->getBulletType())
						{
							localeName += weapon->getBulletType()->getPartTypeIdString();
						}
						else
						{
							Logger::getInstance()->error("SurvivorUpgradeWindow::getItemCount - Bullet type not found");
							Logger::getInstance()->debug(item->varName.c_str());
						}
					}
					else
					{
						Logger::getInstance()->error("SurvivorUpgradeWindow::getItemCount - Weapon type not found");
						Logger::getInstance()->debug(item->varName.c_str());
					}
				}
				else
				{
					Logger::getInstance()->error("SurvivorUpgradeWindow::getItemCount - Invalid weapon number");
					Logger::getInstance()->debug(item->varName.c_str());
				}
			}
			else
			{
				Logger::getInstance()->error("SurvivorUpgradeWindow::getItemCount - Invalid weapon type");
				Logger::getInstance()->debug(item->varName.c_str());
			}
		}
		return itemCount;
	}

	void addItem( SurvivorItem *item )
	{	
		std::string localeName;
		int itemCount = getItemCount(item, localeName);

		if(itemCount > 0 || item->createCountText)
		{
			UpgradeButtonThingie* temp = this->loadSelectionButton( localeName );
			item->button = temp;
		}

		if(item->createCountText)
		{
			OguiButton* temp_decoration = oguiLoader.LoadButton( localeName + "_number_text", window, 0 );
			temp_decoration->SetDisabled( true );
			std::string temp_string = boost::lexical_cast< std::string >( itemCount );
			temp_decoration->SetText( temp_string.c_str() );
			decorationList.push_back( temp_decoration );
			item->count_text = temp_decoration;
		}

		if(item->button && item->varType == SurvivorItem::TYPE_TOGGLE_ITEM)
		{
			int value = game->gameScripting->getGlobalIntVariableValue(item->toggle_item_var.c_str());
			item->button->setSelected( value != 0 );
		}
	}

	void removeItem( SurvivorItem *item )
	{
		if(item->button && item->button->button)
		{
			std::map< int, UpgradeButtonThingie* >::iterator i;
			i = buttonMap.find( item->button->button->GetId() );
			if(i != buttonMap.end())
			{
				delete item->button;
				item->button = NULL;
				buttonMap.erase(i);
			}
		}
		if(item->count_text)
		{
			delete item->count_text;
			decorationList.remove(item->count_text);
			item->count_text = NULL;
		}
	}

	//-------------------------------------------------------------------------

	void loadDataForWindow( const std::string& window_name )
	{

		int deco_id = 1;
		while(true)
		{
			std::string str = window_name + "_decoration" + std::string(int2str(deco_id));
			std::string imgstr = "gui_" + str + "_background_image";
			if(!DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, imgstr.c_str() ))
			{
				break;
			}
			OguiWindow *decowin = oguiLoader.LoadWindow( str );
			decowin->setBackgroundRepeatAuto();
			decoWindows.push_back(decowin);
			deco_id++;
		}

		window = oguiLoader.LoadWindow( window_name );

		// Some additional background pictures
		{
			{
				int x = getLocaleGuiInt( "gui_upgradewindow_charactername_background_x", 0 );
				int y = getLocaleGuiInt( "gui_upgradewindow_charactername_background_y", 0 );
				int w = getLocaleGuiInt( "gui_upgradewindow_charactername_background_w", 0 );
				int h = getLocaleGuiInt( "gui_upgradewindow_charactername_background_h", 0 );
				const char *img = getLocaleGuiString("gui_upgradewindow_charactername_background_image");

				characterNameBackground = ogui->CreateSimpleImageButton(window, x, y, w, h, img, img, img);
			}
			{
				int x = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_background2_x", 0 );
				int y = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_background2_y", 0 );
				int w = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_background2_w", 0 );
				int h = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_background2_h", 0 );
				const char *img = getLocaleGuiString("gui_upgradewindow_characterupgrade_background2_image");

				characterUpgradeBackground = ogui->CreateSimpleImageButton(window, x, y, w, h, img, img, img);
			}
			{
				int x = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_background_x", 0 );
				int y = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_background_y", 0 );
				int w = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_background_w", 0 );
				int h = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_background_h", 0 );
				const char *img = getLocaleGuiString("gui_upgradewindow_weaponupgrade_background_image");

				weaponUpgradeBackground = ogui->CreateSimpleImageButton(window, x, y, w, h, img, img, img);
			}
		}

		hintBox = oguiLoader.LoadFormattedText( "hintbox", window, 0 );
		undoButton = oguiLoader.LoadButton( "undobutton", window, BUTTON_ID_UNDO );
		undoButton->SetListener( this );
		
		closeButton = oguiLoader.LoadButton( "closebutton", window,  BUTTON_ID_CLOSE );
		closeButton->SetListener( this );

		upgradePartsAmount = oguiLoader.LoadButton( "upgradepartsamount", window, 0 );
		upgradePartsAmount->SetDisabled( true );
		upgradePartsAmount->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

		loadSelectionButton( "upgradepartspicture" );

		characterPartsAmount = oguiLoader.LoadButton( "characterpartsamount", window, 0 );
		characterPartsAmount->SetDisabled( true );
		characterPartsAmount->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

		// stats
		{
			characterStats = oguiLoader.LoadFormattedText( "characterstats", window, 0 );


			int timeValue = 0;

			int value = 0;
			if(util::Script::getGlobalIntVariableValue("survival_mode_enabled", &value) && value == 1)
			{
				// survival mode
				//
				// get survival time from stats
				timeValue = game::GameStats::instances[playerNum]->getSurvivalTime();
			}
			else
			{
				// normal mode
				//
				timeValue = game->gameScripting->getGlobalIntVariableValue("player_survival_time");
				timeValue += game->getGameplayTime() / GAME_TICKS_PER_SECOND;
			}

			int killsValue = 0;
			util::Script::getGlobalArrayVariableValue( "player_total_kills", playerNum, &killsValue );
			killsValue += game::GameStats::instances[playerNum]->getTotalKills();

			std::string kills = boost::lexical_cast<std::string>(killsValue);
			std::string secrets = boost::lexical_cast<std::string>(game->gameScripting->getGlobalIntVariableValue("secretpart_amount"));


			int expo_value = 0;
			int expo_min = 0;
			int expo_max = 0;
			util::Script::getGlobalArrayVariableValue( "survivor_current_expo", playerNum, &expo_value );
			util::Script::getGlobalArrayVariableValue( "survivor_expo_min", playerNum, &expo_min );
			util::Script::getGlobalArrayVariableValue( "survivor_expo_max", playerNum, &expo_max );

			std::string statsText = getLocaleGuiString("gui_upgradewindow_characterstatsnumbers_text");
			statsText = util::StringReplace("($exp)", int2str((int)(0.5f + expo_value/10.0f)), statsText);
			statsText = util::StringReplace("($exptotal)", int2str((int)(0.5f + expo_max/10.0f)), statsText);
			statsText = util::StringReplace("($kills)", kills, statsText);
			statsText = util::StringReplace("($time)", time2str(timeValue), statsText);

			characterStatsNumbers = oguiLoader.LoadFormattedText( "characterstatsnumbers", window, 0 );
			characterStatsNumbers->setText(statsText);
		}

		loadSelectionButton( "characterpartspicture" );

		OguiButton* temp_decoration = oguiLoader.LoadButton( "weaponupgradetext", window, 0 );
		temp_decoration->SetDisabled( true );
		temp_decoration->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		decorationList.push_back( temp_decoration );

		/*temp_decoration = oguiLoader.LoadButton( "itemstext", window, 0 );
		temp_decoration->SetDisabled( true );
		decorationList.push_back( temp_decoration );*/

	}

	//-------------------------------------------------------------------------

	void loadDataForUnit()
	{
		// decoration, levels and crap
		{
			// picture rank + name;
			std::string temp_string = unit_name + "_deco_mugshot";
			OguiButton* temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
			decorationList.push_back( temp_decoration );

			temp_string = unit_name + "_deco_name";
			OguiFormattedText *temp_ftext = oguiLoader.LoadFormattedText( temp_string, window, 0 );
			decorationText.push_back( temp_ftext );

			temp_string = unit_name + "_deco_rank";
			temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
			temp_decoration->SetDisabled( true );
			temp_decoration->SetTextHAlign( OguiButton::TEXT_H_ALIGN_RIGHT );
			decorationList.push_back( temp_decoration );

			// Health
			temp_string = "deco_healthtext";
			temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
			temp_decoration->SetDisabled( true );
			temp_decoration->SetTextHAlign( OguiButton::TEXT_H_ALIGN_RIGHT );
			decorationList.push_back( temp_decoration );

			float health_multiplier = unit->getUnitType()->getHealthTextMultiplier();

			float min_health_rounded = (float)unit->getHP() * health_multiplier;
			if(min_health_rounded > 0)
				min_health_rounded = ceilf(min_health_rounded);
			else
				min_health_rounded = 0;

			int min_health = (int)( min_health_rounded );

			temp_string = "deco_health_now";
			temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
			temp_decoration->SetDisabled( true );
			temp_decoration->SetTextHAlign( OguiButton::TEXT_H_ALIGN_RIGHT );
			healthDeco = temp_decoration;
			decorationList.push_back( temp_decoration );
			
			std::string temp_text = boost::lexical_cast< std::string >( min_health );
			temp_decoration->SetText( temp_text.c_str() );
			

			max_hp = unit->getMaxHP();
			float max_health_rounded = (float)max_hp * health_multiplier;
			if(max_health_rounded > 0)
				max_health_rounded = ceilf(max_health_rounded);
			else
				max_health_rounded = 0;

			int max_health = (int)( max_health_rounded );

			temp_string = "deco_health_max";
			temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
			temp_decoration->SetDisabled( true );
			temp_decoration->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
			maxHealthDeco = temp_decoration;
			decorationList.push_back( temp_decoration );
			
			temp_text = boost::lexical_cast< std::string >( max_health );
			temp_decoration->SetText( temp_text.c_str() );
			
			temp_string = "deco_health_slash";
			temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
			temp_decoration->SetDisabled( true );			
			decorationList.push_back( temp_decoration );
			// unit->getMaxHP();

			// expo slider
			{
				expobarSlider = oguiLoader.LoadSlider( "expo_bar", window, 0 );
				expobarSlider->setDisabled( true );

				int expo_value = 0;
				int expo_min = 0;
				int expo_max = 0;
				util::Script::getGlobalArrayVariableValue( "survivor_current_expo", playerNum, &expo_value );
				util::Script::getGlobalArrayVariableValue( "survivor_expo_min", playerNum, &expo_min );
				util::Script::getGlobalArrayVariableValue( "survivor_expo_max", playerNum, &expo_max );

				float slider_value = ( (float)(expo_value - expo_min) / (float)( expo_max - expo_min ) );

				expobarSlider->setValue( slider_value );
	
				int level = 0;
				util::Script::getGlobalArrayVariableValue( "survivor_current_level", playerNum, &level);
				level++;
				// Logger::getInstance()->error( ((std::string)("Cur-level: " ) + boost::lexical_cast< std::string >( level ) ).c_str() );
				
				std::stringstream ss;
				ss << getLocaleGuiString("gui_upgradewindow_level") << " " << level;
				temp_string = "expo_bar_level_text";
				temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
				temp_decoration->SetDisabled( true );
				temp_decoration->SetText( ss.str().c_str() );
				decorationList.push_back( temp_decoration );

				/*temp_string = "expo_bar_current_text";
				temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
				temp_decoration->SetDisabled( true );
				temp_decoration->SetText( int2str((int)(expo_value / 10.0f + 0.5f)) );
				decorationList.push_back( temp_decoration );

				temp_string = "expo_bar_max_text";
				temp_decoration = oguiLoader.LoadButton( temp_string, window, 0 );
				temp_decoration->SetDisabled( true );
				temp_decoration->SetText( int2str((int)(expo_max / 10.0f + 0.5f)) );
				decorationList.push_back( temp_decoration );*/
			}
		}


		// create the text thing and decoration crap
		for(int i = 0; i < 2; i++)
		{
			OguiButton* deco_temp = oguiLoader.LoadButton( std::string( "charupgdecorationbar_" ) + boost::lexical_cast< std::string >( i ), window,  0 );
			decorationList.push_back( deco_temp );

			if(i == 0)
			{
				deco_temp = oguiLoader.LoadButton( std::string( "charupgdecorationtext_" ) + boost::lexical_cast< std::string >( i ), window,  0 );
				deco_temp->SetDisabled( true );
				decorationList.push_back( deco_temp );				
			}
		}

		// load units upgrade parts
		{
			std::vector< int > temp_vector;
			game->upgradeManager->getUpgradesForPart( unit_name.c_str(), temp_vector );

			for( int i = 0; i < (signed)temp_vector.size(); ++i )
			{
				game::UpgradeType *upgType = game->upgradeManager->getUpgradeTypeById( temp_vector[ i ] );
				std::string upgScript = upgType->getScript();
		
				std::string upgButton;
				// general upgrades starting from 4, no need to separate locales
				if(i >= 4) upgButton = "upgrade_" + boost::lexical_cast< std::string >( i );
				else upgButton = unit_name + "_upgrade_" + boost::lexical_cast< std::string >( i );


				// selected button
				OguiButton *selected_button = NULL;
				{
					int w = getLocaleGuiInt("gui_upgradewindow_characterupgrade_background_w", 0);
					int h = getLocaleGuiInt("gui_upgradewindow_characterupgrade_background_h", 0);
					IOguiImage *img = ogui->LoadOguiImage(getLocaleGuiString("gui_upgradewindow_characterupgrade_background"));
					selected_button = ogui->CreateSimpleImageButton(window, 0, 0, w, h, NULL, NULL, NULL, NULL, 0, NULL, false);
					selected_button->SetDisabledImage(img);
					selected_button->SetImageAutoDelete(true, false, false, false);
					selected_button->SetDisabled(true);
					selected_button->Move(10000, 10000);
				}

				UpgradeButtonThingie* temp = loadSelectionButton( upgButton );
				IOguiImage *imgs = ogui->LoadOguiImage(getLocaleGuiString(("gui_upgradewindow_" + upgButton + "_img_normal").c_str()));
				IOguiImage *imgsh = ogui->LoadOguiImage(getLocaleGuiString(("gui_upgradewindow_" + upgButton + "_img_normal").c_str()));
				temp->button->SetDownImage(imgs);
				temp->button->DeleteSelectedImages();
				temp->button->SetSelectedImages(imgs, imgsh);
				temp->upgradeId = temp_vector[i];
				temp->text = oguiLoader.LoadButton( upgButton + "_cost_text", window, 0 );
				temp->text->SetDisabled(true);
				temp->textFontNormal = oguiLoader.LoadFont( getLocaleGuiString(  (std::string( "gui_" ) + oguiLoader.GetWindowName() + "_" + upgButton + "_cost_text_font_normal" ).c_str() ) );
				temp->textFontDisabled = oguiLoader.LoadFont( getLocaleGuiString(  (std::string( "gui_" ) + oguiLoader.GetWindowName() + "_" + upgButton + "_cost_text_font_disabled" ).c_str() ) );
				temp->selected_button = selected_button;
				temp->selected_offset_x = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_background_button_offset_x", 0 );
				temp->selected_offset_y = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_background_button_offset_y", 0 );
				temp->original_x = temp->button->GetX();
				temp->original_y = temp->button->GetY();

				// Cost icon
				{
					int x = temp->button->GetX() + getLocaleGuiInt( "gui_upgradewindow_characterupgrade_costicon_offset_x", 0 );
					int y = temp->button->GetY() + getLocaleGuiInt( "gui_upgradewindow_characterupgrade_costicon_offset_y", 0 );
					int w = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_costicon_w", 0 );
					int h = getLocaleGuiInt( "gui_upgradewindow_characterupgrade_costicon_h", 0 );
					const char *img = getLocaleGuiString("gui_upgradewindow_characterupgrade_costicon_image");
					temp->icon = ogui->CreateSimpleImageButton(window, x, y, w, h, img, img, img);
					if(temp->icon)
					{
						temp->icon->SetDisabledImage(0);
						temp->icon->SetDisabled(true);
					}
				}

				// special case for sniper's "general" upgrade
				if(i == 6 && unit_name == "surv_sniper")
				{
					temp->hintMessage = game::getLocaleGuiString( "gui_upgradewindow_upgrade_6_hintmessage_sniper" );
				}

				// upgrade is locked
				if(game->upgradeManager->isLocked(unit, temp_vector[ i ]))
				{
					if(locked_img == NULL)
					{
						locked_img = ogui->LoadOguiImage(game::getLocaleGuiString("gui_upgradewindow_upgrade_locked_img"));
					}

					temp->button->SetImage(locked_img);
					temp->button->SetDownImage(locked_img);
					temp->button->SetDisabledImage(locked_img);
					temp->button->SetHighlightedImage(locked_img);

					temp->button->Resize(game::getLocaleGuiInt("gui_upgradewindow_upgrade_locked_w", 0),
															 game::getLocaleGuiInt("gui_upgradewindow_upgrade_locked_h", 0));
					delete temp->text;
					temp->text = NULL;
					temp->hintMessage = game::getLocaleGuiString( "gui_upgradewindow_characterupgrade_locked" );
				}

			}
		}

		// load items for unit
		{
			createItems();
		}
	}

	void createItems()
	{
		items_first_id = idCount;
		if( playerItemsMap.find( unit_name ) != playerItemsMap.end() )
		{
			for( int i = 0; i < (signed)playerItemsMap[ unit_name ].size(); ++i )
			{
				addItem(&playerItemsMap[ unit_name ][i]);
			}
		}
		items_last_id = idCount;
	}

	void destroyItems()
	{
		items_first_id = -1;
		items_last_id = -1;
		if( playerItemsMap.find( unit_name ) != playerItemsMap.end() )
		{
			for( int i = 0; i < (signed)playerItemsMap[ unit_name ].size(); ++i )
			{
				removeItem(&playerItemsMap[ unit_name ][i]);
			}
		}
	}
	//-------------------------------------------------------------------------

	void loadWeaponUpgradesForUnit()
	{
		const int SURVIVOR_UPGRADEWINDOW_MAX_WEAPONS = 3;
		const size_t SURVIVOR_UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON = 3;

		int weapon_count = 0;

		for (int i = 0; i < SURVIVOR_UPGRADEWINDOW_MAX_WEAPONS; i++ )
		{
			// NOTE: this is actually not an ok part type id!
			// (thus, should not use this value like this)
			int partTypeId = -1;

			partTypeId = game::PlayerWeaponry::getWeaponIdByUINumber( unit, i );

			int wnum = -1;
			if( partTypeId != -1 )
				wnum = unit->getWeaponByWeaponType( partTypeId );

			if( wnum != -1 )
			{
				std::string weapName;

				game::Weapon *w = unit->getWeaponType( wnum );
				if (w != NULL)
					weapName = w->getPartTypeIdString();
				
				// create the text thing and decoration crap
				{
					OguiButton* deco_temp = oguiLoader.LoadButton( std::string( "weapondecorationbar_" ) + boost::lexical_cast< std::string >( weapon_count ), window,  0 );
					// deco_temp->SetDisabled( true );
					decorationList.push_back( deco_temp );

					std::string locale = "gui_upgradewindow_" + boost::to_lower_copy( weapName ) + "_image_text";
				
					deco_temp = oguiLoader.LoadButton( std::string( "weapondecorationtext_" ) + boost::lexical_cast< std::string >( weapon_count ), window,  0 );
					deco_temp->SetDisabled( true );
					deco_temp->SetText( getLocaleGuiString(locale.c_str()) );
					deco_temp->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
					decorationList.push_back( deco_temp );				
				}


				// create icon
				UpgradeButtonThingie* but = loadSelectionButton( boost::to_lower_copy( weapName ) + "_image" );
				but->button->SetText("");

				if(!unit->isWeaponOperable( wnum ))
				{
					// disabled image switcheroo
					IOguiImage *img_disabled = NULL;
					but->button->GetImages(NULL, NULL, &img_disabled, NULL);
					but->button->SetImage(img_disabled);
					but->button->SetDownImage(img_disabled);
					but->button->SetDisabledImage(img_disabled);
					but->button->SetHighlightedImage(img_disabled);

					std::string temp = std::string( "gui_upgradewindow_" ) + boost::to_lower_copy( weapName ) + "_image_hintmessage_disabled";
					if(::game::DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, temp.c_str() ))
					{
						but->hintMessage = game::getLocaleGuiString( temp.c_str() );
					}
				}

				// upgrades for that weapon
				{
					std::vector< int > upgIds;
					game->upgradeManager->getUpgradesForPart( weapName.c_str(), upgIds );

					if( upgIds.size() > SURVIVOR_UPGRADEWINDOW_MAX_UPGRADES_PER_WEAPON )
					{
						Logger::getInstance()->error( "SurvivorUpgradeWindow - Weapon has too many upgrades." );
						Logger::getInstance()->debug( weapName.c_str() );
					} 

					for( int j = 0; j < (int)upgIds.size(); j++ )
					{
						game::UpgradeType *upgType = game->upgradeManager->getUpgradeTypeById( upgIds[ j ] );
						std::string upgScript = upgType->getScript();
				

						// selected button
						OguiButton *selected_button = NULL;
						{
							int w = getLocaleGuiInt("gui_upgradewindow_weaponupgrade_background_w", 0);
							int h = getLocaleGuiInt("gui_upgradewindow_weaponupgrade_background_h", 0);
							IOguiImage *img = ogui->LoadOguiImage(getLocaleGuiString("gui_upgradewindow_weaponupgrade_background"));
							selected_button = ogui->CreateSimpleImageButton(window, 0, 0, w, h, NULL, NULL, NULL, NULL, 0, NULL, false);
							selected_button->SetDisabledImage(img);
							selected_button->SetImageAutoDelete(true, false, false, false);
							selected_button->SetDisabled(true);
							selected_button->Move(10000, 10000);
						}

						UpgradeButtonThingie* temp = loadSelectionButton( upgScript );
						IOguiImage *imgs = ogui->LoadOguiImage(getLocaleGuiString(("gui_upgradewindow_" + upgScript + "_img_normal").c_str()));
						IOguiImage *imgsh = ogui->LoadOguiImage(getLocaleGuiString(("gui_upgradewindow_" + upgScript + "_img_normal").c_str()));
						temp->button->SetDownImage(imgs);
						temp->button->DeleteSelectedImages();
						temp->button->SetSelectedImages(imgs, imgsh);
						temp->upgradeId = upgIds[ j ];
						temp->text = oguiLoader.LoadButton( upgScript + "_cost_text", window, 0 );
						temp->text->SetDisabled(true);
						temp->textFontNormal = oguiLoader.LoadFont( getLocaleGuiString(  (std::string( "gui_" ) + oguiLoader.GetWindowName() + "_" + upgScript + "_cost_text_font_normal" ).c_str() ) );
						temp->textFontDisabled = oguiLoader.LoadFont( getLocaleGuiString(  (std::string( "gui_" ) + oguiLoader.GetWindowName() + "_" + upgScript + "_cost_text_font_disabled" ).c_str() ) );
						temp->selected_button = selected_button;
						temp->selected_offset_x = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_background_button_offset_x", 0 );
						temp->selected_offset_y = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_background_button_offset_y", 0 );
						temp->original_x = temp->button->GetX();
						temp->original_y = temp->button->GetY();

						// Cost icon
						{
							int x = temp->button->GetX() + getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_costicon_offset_x", 0 );
							int y = temp->button->GetY() + getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_costicon_offset_y", 0 );
							int w = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_costicon_w", 0 );
							int h = getLocaleGuiInt( "gui_upgradewindow_weaponupgrade_costicon_h", 0 );
							const char *img = getLocaleGuiString("gui_upgradewindow_weaponupgrade_costicon_image");
							temp->icon = ogui->CreateSimpleImageButton(window, x, y, w, h, img, img, img);
							if(temp->icon)
							{
								temp->icon->SetDisabledImage(0);
								temp->icon->SetDisabled(true);
							}
						}

						// upgrade is locked
						if(game->upgradeManager->isLocked(unit, upgIds[ j ]))
						{
							if(locked_img == NULL)
							{
								locked_img = ogui->LoadOguiImage(game::getLocaleGuiString("gui_upgradewindow_upgrade_locked_img"));
							}

							temp->button->SetImage(locked_img);
							temp->button->SetDownImage(locked_img);
							temp->button->SetDisabledImage(locked_img);
							temp->button->SetHighlightedImage(locked_img);

							temp->button->Resize(game::getLocaleGuiInt("gui_upgradewindow_upgrade_locked_w", 0),
																	 game::getLocaleGuiInt("gui_upgradewindow_upgrade_locked_h", 0));
							delete temp->text;
							temp->text = NULL;
							temp->hintMessage = game::getLocaleGuiString( "gui_upgradewindow_weaponupgrade_locked" );
						}
					}
				}
			}
			weapon_count++;
		}
	}

	//-------------------------------------------------------------------------

	UpgradeButtonThingie* loadSelectionButton( const std::string& button_name)
	{
		OguiButton* button = oguiLoader.LoadButton( button_name, window, idCount );

		if( button != NULL )
		{
			button->SetListener( this );
			button->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_PRESS | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );

			UpgradeButtonThingie* ub = new UpgradeButtonThingie;
			ub->button = button;
			std::string temp =  std::string( "gui_upgradewindow_" ) + button_name + "_hintmessage";
			ub->hintMessage = game::getLocaleGuiString( temp.c_str() );

			buttonMap.insert( std::pair< int, UpgradeButtonThingie* >( idCount, ub ) );

			idCount++;
			return ub;
		}

		return NULL;
	}

	//*************************************************************************

	void showHintMessage( int id )
	{
		if( id == mouseOverHintButtonId )
			return;

		std::map< int, UpgradeButtonThingie* >::iterator i;
		i = buttonMap.find( id );
		if( i != buttonMap.end() )
		{
			if( i->second->hintMessage.empty() == false )
			{
				hintBox->setText( i->second->hintMessage );
				mouseOverHintButtonId = id;
			}
		}
	}

	//.........................................................................

	void clearHintMessage()
	{
		hintBox->setText( "" );
		mouseOverHintButtonId = 0;

		clearInsufficientPartsText();
	}

	//.........................................................................

	void showInsufficientPartsText(OguiButton *button)
	{
		// delete old message box
		if(insufficientPartsButton != NULL)
			delete insufficientPartsButton;

		// create new one
		insufficientPartsButton = oguiLoader.LoadButton( "insufficientparts", window, 0 );
		insufficientPartsButton->SetDisabled(true);
		insufficientPartsButton->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);

		int x = button->GetX() - insufficientPartsButton->GetSizeX() / 2 + button->GetSizeX() / 2;
		int y = button->GetY() + button->GetSizeY();

		// make sure text does not go outside the screen
		if(x + insufficientPartsButton->GetSizeX() > 1024)
			x = 1024 - insufficientPartsButton->GetSizeX()/2;
		if(y + insufficientPartsButton->GetSizeY() > 768)
			y = 768 - insufficientPartsButton->GetSizeY();

		insufficientPartsButton->Move(x, y);
	}

	//.........................................................................

	void clearInsufficientPartsText(void)
	{
		delete insufficientPartsButton;
		insufficientPartsButton = NULL;
	}

	//*************************************************************************

	void pressOnUpgrade( int button_id )
	{
		std::map< int, UpgradeButtonThingie*  >::iterator i;
		i = buttonMap.find( button_id );
		if( i != buttonMap.end() )
		{
			int upgid = i->second->upgradeId;

			if( upgid != -1 )
			{
				bool alreadyPending = false;

				std::list< int >::iterator iter;
				for( iter = upgradesPending.begin(); iter != upgradesPending.end(); ++iter )
				{
					if( *iter == upgid )
					{
						alreadyPending = true;
						break;
					}
				}

				bool isCharacterUpgrade = game->upgradeManager->isCharacterUpgrade(upgid);
				int &pendingCost = isCharacterUpgrade ? characterUpgradesPendingCost : upgradesPendingCost;
				int tmp = pendingCost;

				// upgrade is not pending and can't afford it
				//
				if(!alreadyPending && !game->upgradeManager->canAfford( unit, upgid, &tmp)
					&& !game->upgradeManager->isUpgraded( unit, upgid )
					&& !game->upgradeManager->isLocked( unit, upgid ))
				{
					// don't allow pressing the button down
					//
					// NOTE: we cannot simply disable the button because
					//       showInsufficientPartsText must be called...
					i->second->button->PressButton( false );
				}
			}
		}
	}

	void clickOnUpgrade( int button_id )
	{
		// should be an item
		if(button_id >= items_first_id && button_id < items_last_id)
		{
			int originalMaxHP = unit->getMaxHP();

			std::string unit_name = unit->getUnitType()->getName();
			unit_name = boost::to_lower_copy( unit_name );
			std::vector< SurvivorItem > &items = playerItemsMap[unit_name];
			for(unsigned int i = 0; i < items.size(); i++)
			{
				if(items[i].button == NULL
					|| items[i].button->button == NULL
					|| items[i].button->button->GetId() != button_id
					|| (items[i].varType != SurvivorItem::TYPE_ITEM && items[i].varType != SurvivorItem::TYPE_TOGGLE_ITEM)) continue;

				int count = game::UnitInventory::getUnitItemCount(game, unit, items[i].varName.c_str());
				if(count <= 0)
				{
					game->gameUI->playGUISound( game::getLocaleGuiString( "gui_upgrades_sound_upgrade_failed" ) );
					continue;
				}

				if(!items[i].script_can_use.first.empty())
				{
					// hack: use upgraded max hp
					unit->setMaxHP(max_hp);

					// test if item can be used
					int return_value = game->gameScripting->runOtherScript(items[i].script_can_use.first.c_str(), items[i].script_can_use.second.c_str(), unit, VC3(0,0,0));
					if(return_value == 0)
					{
						game->gameUI->playGUISound( game::getLocaleGuiString( "gui_upgrades_sound_upgrade_failed" ) );
						continue;
					}
				}


				if(!items[i].script_use.first.empty())
				{
					// hack: make max health large enough so that
					// medikits give health after upgrading the
					// max hp (changed to use max_hp initialized above)
					unit->setMaxHP(max_hp);
					// unit->setMaxHP(1000);

					game->gameScripting->runOtherScript(items[i].script_use.first.c_str(), items[i].script_use.second.c_str(), unit, VC3(0,0,0));

					// decrease item count
					if(items[i].varType == SurvivorItem::TYPE_ITEM)
					{
						count--;
						game::UnitInventory::setUnitItemCount(game, unit, items[i].varName.c_str(), count);

						// update text
						if(items[i].count_text)
						{
							std::string temp_string = boost::lexical_cast< std::string >( count );
							items[i].count_text->SetText( temp_string.c_str() );
						}
					}
					else if(items[i].varType == SurvivorItem::TYPE_TOGGLE_ITEM)
					{
						int value = game->gameScripting->getGlobalIntVariableValue(items[i].toggle_item_var.c_str());
						items[i].button->setSelected( value != 0 );
					}
				}

				if(!items[i].use_text.empty())
				{
					// delete old message box
					if(insufficientPartsButton != NULL)
						delete insufficientPartsButton;

					// create new one
					insufficientPartsButton = oguiLoader.LoadButton( "itemused", window, 0 );
					insufficientPartsButton->SetDisabled(true);
					insufficientPartsButton->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);

					int x = items[i].button->button->GetX() - insufficientPartsButton->GetSizeX() / 2 + items[i].button->button->GetSizeX() / 2;
					int y = items[i].button->button->GetY() - insufficientPartsButton->GetSizeY();

					// make sure text does not go outside the screen
					if(x + insufficientPartsButton->GetSizeX() > 1024)
						x = 1024 - insufficientPartsButton->GetSizeX()/2;
					if(y + insufficientPartsButton->GetSizeY() > 768)
						y = 768 - insufficientPartsButton->GetSizeY();

					insufficientPartsButton->Move(x, y);
					insufficientPartsButton->SetText(items[i].use_text.c_str());
				}
				break;
			}

			// reset original max hp
			unit->setMaxHP(originalMaxHP);
		}
		else
		{
			// not an item, but an upgrade:

			std::map< int, UpgradeButtonThingie*  >::iterator i;
			i = buttonMap.find( button_id );
			if( i != buttonMap.end() )
			{
				int upgid = i->second->upgradeId;

				if( upgid != -1 )
				{

					bool playDoneSound = false;
					bool alreadyPending = false;

					std::list< int >::iterator iter;
					for( iter = upgradesPending.begin(); iter != upgradesPending.end(); ++iter )
					{
						if( *iter == upgid )
						{
							alreadyPending = true;
							break;
						}
					}

					bool isCharacterUpgrade = game->upgradeManager->isCharacterUpgrade(upgid);
					int &pendingCost = isCharacterUpgrade ? characterUpgradesPendingCost : upgradesPendingCost;

					if( alreadyPending == false )
					{
						int tmp = pendingCost;
						
						if( game->upgradeManager->canUpgrade( unit, upgid, &tmp ) && !game->upgradeManager->isLocked(unit, upgid) )
						{
							fb_assert( tmp > pendingCost );

							if(game->upgradeManager->runStartPendingScript( unit, upgid ))
							{
								destroyItems();
								createItems();
							}

							upgradesPending.push_back( upgid );
							pendingCost = tmp;

							playDoneSound = true;
						}
						else
						{
							if(!game->upgradeManager->canAfford( unit, upgid, &tmp)
								&& !game->upgradeManager->isUpgraded( unit, upgid )
								&& !game->upgradeManager->isLocked( unit, upgid ))
							{
								showInsufficientPartsText(i->second->button);
							}
						}

					}
					else	// undo by clicking
					{
						pendingCost -= game->upgradeManager->getUpgradePartCost( upgid );

						if(game->upgradeManager->runStopPendingScript( unit, upgid ))
						{
							destroyItems();
							createItems();
						}
						upgradesPending.erase( iter );
						playDoneSound = true;
					}
					
					if( playDoneSound )
					{
						game->gameUI->playGUISound( game::getLocaleGuiString( "gui_upgrades_sound_upgrade_done" ) );
					} else {
						game->gameUI->playGUISound( game::getLocaleGuiString( "gui_upgrades_sound_upgrade_failed" ) );
					}
				}
			}

		}
		// redo the button selections
		updateTheButtons();
	}

	//*************************************************************************

	void updateTheButtons()
	{
		std::map< int, UpgradeButtonThingie* >::iterator i;
		for( i = buttonMap.begin(); i != buttonMap.end(); ++i )
		{
			int upgid = i->second->upgradeId;
			if( upgid != -1 )
			{
				bool isCharacterUpgrade = game->upgradeManager->isCharacterUpgrade(upgid);
				int &pendingCost = isCharacterUpgrade ? characterUpgradesPendingCost : upgradesPendingCost;

				i->second->setSelected( false );

				bool alreadyUpgraded = false;
				if( game->upgradeManager->isUpgraded( unit, upgid ) )
				{
					alreadyUpgraded = true;
				}

				bool pending = false;

				std::list< int >::iterator iter;
				for( iter = upgradesPending.begin(); iter != upgradesPending.end(); ++iter )
				{
					if( *iter == upgid )
					{
						pending = true;
						break;
					}
				}

				if( pending || alreadyUpgraded )
				{
					i->second->setSelected( true );
					// hide cost
					i->second->hideCostText();
				} else {
					i->second->setSelected( false );
					// show cost
					i->second->showCostText(game, unit, pendingCost);
				}
			}
		}

		// undo button
		if( undoButton )
		{
			if( upgradesPending.size() > 0 )
			{
				undoButton->SetDisabled( false );
				const char *label = game::getLocaleGuiString( "gui_upgrades_undo" );
				undoButton->SetText( label );
			} else {
				undoButton->SetDisabled( true );
				undoButton->SetText( "" );
			}
		}

		// upgrade amounts
		{
			int tmp = game->upgradeManager->getUpgradePartsAmount( this->unit ) - this->upgradesPendingCost;
			upgradePartsAmount->SetText( boost::lexical_cast< std::string >( tmp ).c_str() );

			int tmp2 = game->upgradeManager->getCharacterPartsAmount( this->unit ) - this->characterUpgradesPendingCost;
			characterPartsAmount->SetText( boost::lexical_cast< std::string >( tmp2 ).c_str() );
		}

		// stupid hack to make health upgrade visible instantly
		if(maxHealthDeco)
		{
			max_hp = unit->getMaxHP();
			int max_health = (int)( (float)max_hp * unit->getUnitType()->getHealthTextMultiplier() );

			// if we have a health upgrade pending
			std::list< int >::iterator i;
			for( i = upgradesPending.begin(); i != upgradesPending.end(); ++i )
			{
				UpgradeType *upgt = game->upgradeManager->getUpgradeTypeById(*i);
				if(upgt && strstr(upgt->getName(), "health"))
				{
					// use the new max health
					max_hp = game->gameScripting->getGlobalIntVariableValue("health_upgrade_max_hp");
					max_health = (int)( (float)max_hp * unit->getUnitType()->getHealthTextMultiplier() );
					break;
				}
			}

			// set max health text
			maxHealthDeco->SetText( boost::lexical_cast< std::string >( max_health ).c_str() );

			// clamp health
			float min_health_rounded = (float)unit->getHP() * unit->getUnitType()->getHealthTextMultiplier();
			if(min_health_rounded > 0)
				min_health_rounded = ceilf(min_health_rounded);
			else
				min_health_rounded = 0;

			int health = (int)( min_health_rounded );

			// set health text
			healthDeco->SetText( boost::lexical_cast< std::string >( health ).c_str() );
		}
	}

	//*************************************************************************

	void applyUpgrades()
	{
		std::list< int >::iterator i;
		for( i = upgradesPending.begin(); i != upgradesPending.end(); ++i )
		{
			// WARNING: void * -> int cast!
			int upgid = *i;
			game->upgradeManager->runStopPendingScript( unit, upgid );

			if( game->upgradeManager->canUpgrade( unit, upgid ) && !game->upgradeManager->isLocked(unit, upgid) )
			{
				game->upgradeManager->upgrade( unit, upgid );
			} else {
				Logger::getInstance()->warning("SurvivorUpgradeWindow::applyUpgrades - Pending upgrade cannot be applied, internal error.");
			}
		}
		upgradesPending.clear();
		this->upgradesPendingCost = 0;
		this->characterUpgradesPendingCost = 0;

		// clamp hp
		unit->setHP(unit->getHP());
	}

	//-------------------------------------------------------------------------

	void undoUpgrades()
	{
		upgradesPending.clear();
		upgradesPendingCost = 0;
		characterUpgradesPendingCost = 0;
	}

	//-------------------------------------------------------------------------

	void effectUpdate(int msecTimeDelta)
	{
		// effectWindow->update(msecTimeDelta);
	}

	void raise()
	{
		// effectWindow->raise();
		window->Raise();
	}

	void fadeOut()
	{
		visible = 2;
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEOUT, FADE_OUT_TIME );
		{
			std::list<OguiWindow *>::iterator it;
			for( it = decoWindows.begin(); it != decoWindows.end(); it++)
			{
				(*it)->StartEffect( OGUI_WINDOW_EFFECT_FADEOUT, FADE_OUT_TIME );
			}
		}
		// effectWindow->fadeOut(FADE_OUT_TIME);
		game->gameUI->prepareCloseUpgradeWindow( unit );
	}

	int getFadeInTime() const
	{
		return FADE_IN_TIME;
	}

	int getFadeOutTime() const
	{
		return FADE_OUT_TIME;
	}

	int isVisible() const
	{
		return visible;
	}

	//-------------------------------------------------------------------------

	void EffectEvent( OguiEffectEvent *e )
	{
		if( e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT )
		{
			game->gameUI->closeUpgradeWindow( this->unit );
			return;
		}
	}

	//.........................................................................

	void CursorEvent( OguiButtonEvent *eve )
	{
		int id = eve->triggerButton->GetId();

		switch( eve->eventType )
		{
		case OguiButtonEvent::EVENT_TYPE_OVER:
			{
				std::map< int, UpgradeButtonThingie* >::iterator i;
				i = buttonMap.find( id );
				if ( i != buttonMap.end() )
				{
					showHintMessage( id );
				}
			}
			break;

		case OguiButtonEvent::EVENT_TYPE_LEAVE:
			{
				if( id == mouseOverHintButtonId )
					clearHintMessage();
			}
			break;

		case OguiButtonEvent::EVENT_TYPE_PRESS:
			switch( id )
			{
			case BUTTON_ID_UNDO:
			case BUTTON_ID_CLOSE:
				break;
			default:
				pressOnUpgrade( id );
			}
			break;


		case OguiButtonEvent::EVENT_TYPE_CLICK:

			switch( id )
			{
			case BUTTON_ID_UNDO:
				{
					game->gameUI->playGUISound( game::getLocaleGuiString( "gui_upgrades_sound_undo" ) );
					undoUpgrades();
					updateTheButtons();
				}
				break;
			
			case BUTTON_ID_CLOSE:
				{
					game->gameUI->playGUISound( game::getLocaleGuiString( "gui_upgrades_sound_close" ) );

					this->applyUpgrades();
					game->setPaused( false );

					// NOTE: must return here as this UpgradeWindow object HAS BEEN DELETED!
					//game->gameUI->closeUpgradeWindow(this->unit);
					fadeOut();
					return;
				}
				break;

			default:
				clickOnUpgrade( id );
				break;
			}

			break;
		default:
			break;
		}
	}
	
	//-------------------------------------------------------------------------
};

///////////////////////////////////////////////////////////////////////////////

SurvivorUpgradeWindow::SurvivorUpgradeWindow( Ogui *ogui, game::Game *game, game::Unit *unit ) :
	impl( NULL )
{
	impl = new SurvivorUpgradeWindowImpl( ogui, game, unit );
}

//.............................................................................

SurvivorUpgradeWindow::~SurvivorUpgradeWindow()
{
	delete impl;
	impl = NULL;
}

//=============================================================================

void SurvivorUpgradeWindow::applyUpgrades()
{
	impl->applyUpgrades();
}

void SurvivorUpgradeWindow::undoUpgrades()
{
	impl->undoUpgrades();
}

void SurvivorUpgradeWindow::effectUpdate(int msecTimeDelta)
{
	impl->effectUpdate( msecTimeDelta );
}

void SurvivorUpgradeWindow::raise()
{
	impl->raise();
}

void SurvivorUpgradeWindow::fadeOut()
{
	impl->fadeOut();
}

int SurvivorUpgradeWindow::getFadeInTime() const
{
	return impl->getFadeInTime();
}

int SurvivorUpgradeWindow::getFadeOutTime() const
{
	return impl->getFadeOutTime();
}

int SurvivorUpgradeWindow::isVisible() const
{
	return impl->isVisible();
}

void SurvivorUpgradeWindow::preloadTextures(game::GameUI *gameUI)
{
	// this is a bit hacky, but just preload all textures from the locale file..
	//
	util::SimpleParser parser;
	if(!parser.loadFile("Data/Locales/Common/gui_survivorupgradewindow.txt"))
	{
		Logger::getInstance()->warning("SurvivorUpgradeWindow::preloadTextures - locale file not found.");
		return;
	}

	while (parser.next())
	{
		char *k = parser.getKey();
		char *v = parser.getValue();
		if(k && v && (strstr(v, ".tga") || strstr(v, ".dds")))
		{
			gameUI->getOguiStormDriver()->getTextureCache()->loadTextureDataToMemory(v);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
