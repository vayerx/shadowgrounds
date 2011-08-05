#include "precompiled.h"

#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "CombatSubWindowFactory.h"
#include "GenericBarWindow.h"
#include "GenericTextWindow.h"
#include "WeaponWindow.h"
#include "WeaponWindowCoop.h"

#include "../game/GameStats.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/Weapon.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/scripting/GameScripting.h"
#include "../game/PartType.h"
#include "../game/UnitList.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../game/DHLocaleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "../sound/sounddefs.h"
#include "../sound/SoundMixer.h"
#include "../ui/CombatWindow.h"
#include "../util/StringUtil.h"

namespace game {
	class Game;
}

namespace ui {
///////////////////////////////////////////////////////////////////////////////

class GenericBarTimerUpdator : public IGenericBarWindowUpdator
{
public:
	GenericBarTimerUpdator( game::Game* game, const std::string& global_value, const std::string& global_timer_length  ) :
		valueVarName( global_value ),
		lengthVarName( global_timer_length ),
		game( game )
	{
		lastValue = 0.0f;
	}

	float update()
	{
		if(game->isPaused())
			return lastValue;

		float value = (float)game->gameScripting->getGlobalIntVariableValue( valueVarName.c_str() );

		value *= GAME_TICKS_PER_SECOND;
		value += game->missionStartTime;

		int time_now = game->gameTimer;
		float time_gone = time_now - value;


		float length = (float)game->gameScripting->getGlobalIntVariableValue( lengthVarName.c_str() );
		length *= GAME_TICKS_PER_SECOND;

		if( length >= 0.0f && length < 0.0001f ) return 1.0f;

		lastValue = ( time_gone / length );
		return lastValue;
	}

	std::string valueVarName;
	std::string lengthVarName;
	game::Game* game;
	float lastValue;
};

///////////////////////////////////////////////////////////////////////////////

class GenericBarScriptsGlobalUpdator : public IGenericBarWindowUpdator
{
public:
	GenericBarScriptsGlobalUpdator( game::Game* game, GenericBarWindow *win, const std::string& global_value, const std::string& global_max, const std::string& global_min, bool hide_during_converse) :
		valueVarName( global_value ),
		maxVarName( global_max ),
		minVarName( global_min ),
		hideDuringConverse( hide_during_converse ),
		game( game ),
		win( win )
	{
	}

	float update()
	{
		if( game->gameUI
			&& game->gameUI->getCombatWindow(0)
			&& game->gameUI->getCombatWindow(0)->hasMessageWindow())
		{
			if(!win->isHidden())
				win->hide(200);
		}
		else if(win->isHidden()
			&& !game->gameUI->isVehicleGUIOpen()
			&& game->gameUI->getCombatWindow(0)
			&& !game->gameUI->getCombatWindow(0)->isGUIModeTempInvisible()
			&& game->gameUI->getCombatWindow(0)->isGUIVisible())
		{
			win->show(200);
		}

		int value = game->gameScripting->getGlobalIntVariableValue( valueVarName.c_str() );
		int max = game->gameScripting->getGlobalIntVariableValue( maxVarName.c_str() );
		int min = 0;
		if( minVarName.empty() == false )
			min = game->gameScripting->getGlobalIntVariableValue( minVarName.c_str() );

		value -= min;

		return( (float)value / (float)( max - min ) );
	}

	std::string valueVarName;
	std::string maxVarName;
	std::string minVarName;
	bool hideDuringConverse;
	game::Game* game;
	GenericBarWindow *win;
};

///////////////////////////////////////////////////////////////////////////////

class GenericBarTimerWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	GenericBarTimerWindowConstructor( const std::string& window_name, const std::string& locales_name, const std::string& value_var, const std::string& length_var ) :
		localesName( locales_name ),
		valueVarName( value_var ),
		lengthVarName( length_var )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		GenericBarWindow* result = new GenericBarWindow( ogui, game, player );
		result->loadDataFromLocales( localesName );
		result->setUpdator( new GenericBarTimerUpdator( game, valueVarName, lengthVarName ) );
		return result;
	}

	std::string localesName;
	std::string valueVarName;
	std::string lengthVarName;

};

///////////////////////////////////////////////////////////////////////////////

class GenericBarGlobalsFromScriptsWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	GenericBarGlobalsFromScriptsWindowConstructor( const std::string& window_name, const std::string& locales_name, const std::string& value_var, const std::string& max_var, const std::string& min_var, bool hide_during_converse ) :
		localesName( locales_name ),
		valueVarName( value_var ),
		maxVarName( max_var ),
		minVarName( min_var ),
		hideDuringConverse( hide_during_converse )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		GenericBarWindow* result = new GenericBarWindow( ogui, game, player );
		result->loadDataFromLocales( localesName );
		result->setUpdator( new GenericBarScriptsGlobalUpdator( game, result, valueVarName, maxVarName, minVarName, hideDuringConverse ) );
		return result;
	}

	std::string localesName;
	std::string valueVarName;
	std::string maxVarName;
	std::string minVarName;
	bool hideDuringConverse;

};

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class GenericBarWeaponReloadUpdator : public IGenericBarWindowUpdator
{
public:
	GenericBarWeaponReloadUpdator( game::Game* game, int player_id, int weapon_id ) :
		playerId( player_id ),
		weaponId( weapon_id ),
		weaponOnPlayer( -1 ),
		game( game ),
		weapon( NULL )
	{
		setWeapon( player_id, weapon_id );
	}

	void setWeapon( int player_id, int weapon_id )
	{
		playerId =  player_id;
		weaponId = weapon_id;

		game::Unit* unit = NULL;
		if( game && game->gameUI && game->gameUI->getFirstPerson( playerId ) )
			unit = game->gameUI->getFirstPerson( playerId );

		if( unit )
		{
			weaponOnPlayer = unit->getWeaponByWeaponType( weaponId );

			game::Weapon* w = NULL;
			if( weaponOnPlayer >= 0 && weaponOnPlayer < UNIT_MAX_WEAPONS )
				w = unit->getWeaponType( weaponOnPlayer );

			if( w )
				weapon = w;
		}
	}

	float update()
	{
		float result = 1.0f;
		if( weapon )
		{
			game::Unit* unit = game->gameUI->getFirstPerson( playerId );
			if( unit )
			{
				result = (float)unit->getFireReloadDelay( weaponOnPlayer ) / (float)weapon->getFireReloadTime();
				if( false )
				{
					int b = unit->getFireReloadDelay( weaponOnPlayer );
					int s = weapon->getFireReloadTime();

					if( b == 0 )
					{
						// unit->setFireWaitDelay( weapon->getFireRel
					}

					std::string crap = (std::string)(( "" ) + boost::lexical_cast< std::string >( b ) + " / " + boost::lexical_cast< std::string >( s ));
					// Logger::getInstance()->warning( crap.c_str() );
				}
				result = 1.0f - result;
				if( result < 0.0f )
					result = 0.0f;
				
				if( result > 1.0f ) 
					result = 1.0f;
			}
		}
		// int value = game->gameScripting->getGlobalIntVariableValue( valueVarName.c_str() );
		// int max = game->gameScripting->getGlobalIntVariableValue( maxVarName.c_str() );

		// return( (float)value / (float)max );

		return result;
	}

	int playerId;
	int weaponId;
	int weaponOnPlayer;
	game::Game* game;
	game::Weapon* weapon;
};

///////////////////////////////////////////////////////////////////////////////

class GenericBarWeaponReloadWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	GenericBarWeaponReloadWindowConstructor( const std::string& window_name, const std::string& locales_name, const std::string& weapon_id ) :
		localesName( locales_name ),
		weaponId( -1 )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
		const char* temp = weapon_id.c_str(); 
		weaponId = PARTTYPE_ID_STRING_TO_INT( temp );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		GenericBarWindow* result = new GenericBarWindow( ogui, game, player );
		result->loadDataFromLocales( localesName );
		result->setUpdator( new GenericBarWeaponReloadUpdator( game, player, weaponId ) );
		return result;
	}

	std::string localesName;
	int weaponId;

};

///////////////////////////////////////////////////////////////////////////////

class SurvivorWeaponWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	SurvivorWeaponWindowConstructor( const std::string& window_name, const std::string& profileName ) :
		profileName( profileName )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		if(profileName == "coop")
		{
			int num_players = 0;
			int c;
			for( c = 0; c < MAX_PLAYERS_PER_CLIENT; c++ )
			{
				if( ::game::SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + c ) )
				{
					num_players++;
				}
			}

			WeaponWindowCoop* result = new WeaponWindowCoop( ogui, game, num_players );
			return result;
		}
		else
		{
			WeaponWindow* result = new WeaponWindow( ogui, game, player, false, profileName );
			return result;
		}
	}

	std::string profileName;

};

///////////////////////////////////////////////////////////////////////////////

// this is a weird hack to enable moving windows with minimal effort..
//
class StupidWindowAligner : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	StupidWindowAligner( const std::string& name, const std::string& window ) :
		window( window ),
		name( name )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		if(game->gameUI->getCombatWindow(0) == NULL) return NULL;

		ICombatSubWindow* win = game->gameUI->getCombatWindow(0)->getSubWindow(window);
		if(win == NULL) return NULL;

		// find windows
		GenericBarWindow* windows[3];
		windows[0] = (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorNapalmFlameBar");
		windows[1] = (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorShieldBar");
		windows[2] = (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorKillingSpreeReloadBar");

		GenericBarWindow* win1 = (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow(window.c_str());
		if(win1 == NULL)
			return NULL;

		int win1_x, win1_y = 0, win1_w, win1_h = 0;
		win1->getWindowRect(win1_x, win1_y, win1_w, win1_h);

		// test from bottom to top
		int free_pos;
		for(free_pos = win1_y; free_pos >= win1_h;)
		{
			bool free = true;
			for(int i = 0; i < 3; i++)
			{
				GenericBarWindow* win2 = windows[i];
				if(!win2 || win2 == win1)
				{
					continue;
				}

				int win2_x, win2_y = 0, win2_w, win2_h = 0;
				win2->getWindowRect(win2_x,win2_y,win2_w,win2_h);

				// test if overlap
				if(   (win2_y <= free_pos && win2_y + win2_h <= free_pos) // win2 is above win1
					 || (win2_y >= free_pos + win1_h && win2_y + win2_h >= free_pos + win1_h)) // win2 is below win1
				{
					// no overlap
				}
				else
				{
					free_pos = win2_y - win1_h - 8;
					free = false;
				}
			}
			if(free)
			{
				break;
			}
		}

		// bottom of win1 is at highest position
		win1->moveBy(0, free_pos - win1_y);

		return NULL;
	}

	std::string window;
	std::string name;

};

///////////////////////////////////////////////////////////////////////////////


class KillCountUpdater : public IGenericTextWindowUpdator
{
public:
	KillCountUpdater(int client, game::Game *game, const std::string &vocal_locale, Ogui *ogui) : client(client), game(game), ogui(ogui)
	{
		desc_text = NULL;
		description = game::getLocaleGuiString("survivor_killcounter_text");
		offset_x = game::getLocaleGuiInt("gui_survivor_killcounter_x",0);
		offset_y = game::getLocaleGuiInt("gui_survivor_killcounter_y",0);
		description += " ";

		killcount_start = game::GameStats::instances[client]->getTotalKills();

		game->gameScripting->newGlobalIntVariable("sniper_kill_counter_kills", true);
		killcount_start -= game->gameScripting->getGlobalIntVariableValue("sniper_kill_counter_kills");

		aligned = false;

		playingVocal = -1;
		nextVocalTrigger = 0;
		const char *vocals_const = game::getLocaleGuiString(vocal_locale.c_str());
		unsigned int length = strlen(vocals_const);
		char *vocals = (char *)alloca(length + 1);
		memcpy(vocals, vocals_const, length + 1);
		while(true)
		{
			// find '/'
			unsigned int i = 0;
			while(vocals[i] != '/' && vocals[i] != 0)
				i++;
			// convert spaces in filename back to _
			while(vocals[i] != '.' && vocals[i] != 0)
			{
				if(vocals[i] == ' ')
					vocals[i] = '_';
				i++;
			}

			int number = 0;
			char vocal[1024];
			if(sscanf(vocals, "%i , %s", &number, vocal) == 2)
			{
				vocalTriggers.push_back( std::pair<int, std::string> (number, game::convertLocaleSpeechString( vocal )) );
			}
			// jump to end of line
			while(vocals[0] != 0 && vocals[0] != '\n')
			{
				vocals++;
			}
			// jump to beginning of next line
			if(vocals[0] == '\n')
				vocals++;
			// end of string
			if(vocals[0] == 0)
				break;
		}

		// jump triggers
		int kills = game->gameScripting->getGlobalIntVariableValue("sniper_kill_counter_kills");
		while(nextVocalTrigger < vocalTriggers.size())
		{
			const std::pair<int, std::string> &trigger = vocalTriggers[nextVocalTrigger];
			if(kills >= trigger.first)
			{
				nextVocalTrigger++;
			}
			else
			{
				break;
			}
		}
	}

	~KillCountUpdater()
	{
		delete desc_text;
	}

	void update(GenericTextWindow *win)
	{
		// align position to expo bar
		if(!aligned)
		{
			GenericBarWindow* bar_windows[4] =
			{
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_0"),
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_1"),
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_2"),
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_3")
			};

			// get last bar
			GenericBarWindow* bar_win = NULL;
			for(int i = 0; i < 4; i++)
			{
				if(bar_windows[i] != NULL) bar_win = bar_windows[i];
			}
			if(bar_win != NULL)
			{
				int x = 0, y = 0, w, h = 0;
				bar_win->getWindowRect(x,y,w,h);
				win->getWindow()->MoveTo(x + offset_x, y + h + offset_y);
			}

			// get timer
			{
				GenericTextWindow* win2 = (GenericTextWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("CountdownTimer");
				if(win2 && win2->getWindow())
				{
					int bottom = win2->getWindow()->GetPositionY() + win2->getWindow()->GetSizeY();
					win->getWindow()->MoveTo(win->getX(), bottom + offset_y);
				}
			}

			
			desc_text = ogui->CreateSimpleTextButton(win->getWindow(),
				win->getText()->getX(), win->getText()->getY()-2,
				win->getText()->getW(), win->getText()->getH(), NULL, NULL, NULL, description.c_str(), 0, 0, false);
			desc_text->SetFont(win->getText()->getFont());
			desc_text->SetDisabled(true);
			desc_text->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
			desc_text->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
			win->getText()->setTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);
			aligned = true;
		}

		// hide during converse
		if( game->gameUI
			&& game->gameUI->getCombatWindow(0)
			&& game->gameUI->getCombatWindow(0)->hasMessageWindow())
		{
			if(!win->isHidden())
				win->hide(200);
		}
		else if(win->isHidden()
			&& !game->gameUI->isVehicleGUIOpen()
			&& game->gameUI->getCombatWindow(0)
			&& !game->gameUI->getCombatWindow(0)->isGUIModeTempInvisible()
			&& game->gameUI->getCombatWindow(0)->isGUIVisible())
		{
			win->show(200);
		}

		int kills = game::GameStats::instances[client]->getTotalKills() - killcount_start;
		game->gameScripting->setGlobalIntVariableValue("sniper_kill_counter_kills", kills);
		char killstext[256];
		sprintf(killstext, "%i", kills);
		win->setText(killstext);

		if(nextVocalTrigger < vocalTriggers.size())
		{
			const std::pair<int, std::string> &trigger = vocalTriggers[nextVocalTrigger];
			if(kills >= trigger.first)
			{
				VC3 pos = game->gameUI->getListenerPosition();
				if(playingVocal == -1 || !game->gameUI->getSoundMixer()->isSoundPlaying(playingVocal))
					playingVocal = game->gameUI->playSpeech(trigger.second.c_str(), pos.x, pos.y, pos.z, false, DEFAULT_SPEECH_VOLUME);
				nextVocalTrigger++;
			}
		}
	}
	std::string description;
	int client;
	int offset_x;
	int offset_y;
	int killcount_start;
	game::Game *game;
	OguiButton *desc_text;
	Ogui *ogui;

	bool aligned;
	std::vector< std::pair<int, std::string> > vocalTriggers;
	unsigned int nextVocalTrigger;
	int playingVocal;
};
class KillCountConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	KillCountConstructor(const std::string &name, const std::string &unit_type, const std::string &vocal_locale)
		: unit_type(unit_type), vocal_locale(vocal_locale)
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow(name, this);
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		// find the first player with correct unit
		int client;
		for(client = 0; client < MAX_PLAYERS_PER_CLIENT; client++)
		{
			game::Unit *unit = game->gameUI->getFirstPerson(client);
			if(unit)
			{
				// this is the unit
				if(unit->getUnitType()->getName() == unit_type)
				{
					break;
				}
				else
				{
					// or transporting the unit
					int id = unit->variables.getVariable("cargounitid");
					unit = game->units->getUnitById(id);
					if(unit && unit->getUnitType()->getName() == unit_type)
					{
						break;
					}
				}
			}
		}
		if(client == MAX_PLAYERS_PER_CLIENT)
		{
			Logger::getInstance()->warning(("KillCountConstructor - no player with type " + unit_type).c_str());
			return NULL;
		}

		GenericTextWindow *win = new GenericTextWindow(ogui, game, player);
		win->loadDataFromLocales("survivor_killcounter");
		win->setUpdator(new KillCountUpdater(client, game, vocal_locale, ogui));
		return win;
	}

	std::string unit_type;
	std::string vocal_locale;
};

///////////////////////////////////////////////////////////////////////////////

class KillingSpreeCountUpdater : public IGenericTextWindowUpdator
{
public:
	KillingSpreeCountUpdater(int client, game::Game *game) : client(client), game(game)
	{
		originalText = game::getLocaleGuiString("gui_survivor_killingspree_counter_text");
		killcount_start = game::GameStats::instances[client]->getTotalKills();
	}
	void update(GenericTextWindow *win)
	{
		int kills = game::GameStats::instances[client]->getTotalKills() - killcount_start;
		char killstext[256];
		sprintf(killstext, "%i", kills);
		std::string text = util::StringReplace("($kills)", killstext, originalText);
		win->setText(text.c_str());
	}
	int client;
	int killcount_start;
	game::Game *game;
	std::string originalText;
};

class KillingSpreeCountConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	KillingSpreeCountConstructor(const std::string &name)
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow(name, this);
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		// find the player with slow motion
		int client = game->gameScripting->getGlobalIntVariableValue("survivor_player_with_slomo");

		GenericTextWindow *win = new GenericTextWindow(ogui, game, player);
		win->loadDataFromLocales("survivor_killingspree_counter");
		win->setUpdator(new KillingSpreeCountUpdater(client, game));
		return win;
	}
};

///////////////////////////////////////////////////////////////////////////////


class SurvivorTimerUpdator : public IGenericTextWindowUpdator,  public IGenericBarWindowUpdator
{
public:
	SurvivorTimerUpdator( game::Game* game, bool countdown, GenericBarWindow *barwindow ) :
		game( game ),
		countdown( countdown ),
		barwindow( barwindow )
	{
		pausedTime = game->missionPausedTime;
		startTime = game->gameTimer;
		aligned = false;
		target_value = game->gameScripting->getGlobalIntVariableValue("timerwindow_value");
		if(countdown)
		{
			lastValue = time2str(target_value);
			bar_value = 1.0f;
		}
		else
		{
			game->gameScripting->setGlobalIntVariableValue("timerwindow_value", 0);
			lastValue = time2str(0);
			bar_value = 0.0f;
		}
	}

	bool get_aligned_pos(int &xpos, int &ypos)
	{
		// COUNTDOWN ONLY: align position to expo bar
		if(!aligned && countdown)
		{
			GenericBarWindow* bar_windows[4] =
			{
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_0"),
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_1"),
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_2"),
			 (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SurvivorExpoBar_3")
			};

			int offset_y = ypos;
			int offset_x = xpos;

			// get last bar
			GenericBarWindow* bar_win = NULL;
			for(int i = 0; i < 4; i++)
			{
				if(bar_windows[i] != NULL) bar_win = bar_windows[i];
			}
			if(bar_win != NULL)
			{
				int x = 0, y = 0, w, h = 0;
				bar_win->getWindowRect(x,y,w,h);
				xpos = x + offset_x;
				ypos = y + h + offset_y;
			}

			// get killcounter
			GenericTextWindow* win2 = (GenericTextWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("SniperKillCounter");
			if(win2 && win2->getWindow())
			{
				int bottom = win2->getWindow()->GetPositionY() + win2->getWindow()->GetSizeY();
				ypos = bottom + offset_y;
			}

			aligned = true;
			return true;
		}
		return false;
	}

	// get updated values
	//
	void updateValues()
	{
		int time_now = game->gameTimer;
		if(game->missionPausedTime > pausedTime)
		{
			time_now -= game->missionPausedTime - pausedTime;
		}

		int time_gone = (time_now - startTime)  / GAME_TICKS_PER_SECOND;

		if(countdown)
		{
			time_gone = target_value - time_gone;
			if(time_gone < 0)
			{
				time_gone = 0;
			}
		}
		else
		{
			if(time_gone > target_value)
			{
				time_gone = target_value;
			}
		}

		if(barwindow)
		{
			// time in previous tick
			float last_bar_value = (time_now - 1 - startTime) / (float)(target_value * GAME_TICKS_PER_SECOND);
			if(last_bar_value > 1.0f)
				last_bar_value = 1.0f;
			else if(last_bar_value < 0.0f)
				last_bar_value = 0.0f;
			if(countdown)
				last_bar_value = 1.0f - last_bar_value;

			// time in current tick
			bar_value = (time_now - startTime) / (float)(target_value * GAME_TICKS_PER_SECOND);
			if(bar_value > 1.0f)
				bar_value = 1.0f;
			if(countdown)
				bar_value = 1.0f - bar_value;

			// interpolate for smoother movement
			float interpolate_factor = (Timer::getTime() - game->lastTickTime) / (1000.0f / (float) GAME_TICKS_PER_SECOND);
			bar_value = bar_value * interpolate_factor + last_bar_value * (1.0f - interpolate_factor);
		}

		game->gameScripting->setGlobalIntVariableValue("timerwindow_value", time_gone);
		lastValue = time2str(time_gone);
	}

	// update GenericTextWindow
	//
	void update(GenericTextWindow *win)
	{
		// align window
		int x = win->getWindow()->GetPositionX();
		int y = win->getWindow()->GetPositionY();
		if(get_aligned_pos(x, y))
		{
			win->getWindow()->MoveTo(x, y);
		}

		if(game->isPaused())
		{
			win->setText(lastValue);
			return;
		}

		updateValues();

		win->setText(lastValue);
	}

	// update GenericBarWindow
	//
	float update()
	{
		// align window
		/*int x,y,w,h;
		barwindow->getWindowRect(x,y,w,h);
		if(get_aligned_pos(x, y))
		{
			barwindow->move(x, y);
		}*/

		if(game->isPaused())
		{
			return bar_value;
		}

		updateValues();

		return bar_value;
	}

	game::Game* game;
	int pausedTime;
	int startTime;
	std::string lastValue;
	bool aligned;
	bool countdown;
	int target_value;
	float bar_value;
	GenericBarWindow *barwindow;
};


class SurvivorTimerWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	SurvivorTimerWindowConstructor( const std::string& window_name, const std::string& locales_name, bool countdown, bool bar) :
		localesName( locales_name ),
		countdown(countdown),
		bar(bar)
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		if(bar)
		{
			GenericBarWindow* result = new GenericBarWindow( ogui, game, player );
			result->loadDataFromLocales( localesName );
			result->setUpdator( new SurvivorTimerUpdator( game, countdown, result ) );
			return result;
		}
		else
		{
			GenericTextWindow* result = new GenericTextWindow( ogui, game, player );
			result->loadDataFromLocales( localesName );
			result->setUpdator( new SurvivorTimerUpdator( game, countdown, NULL ) );
			return result;
		}
	}

	std::string localesName;
	bool countdown;
	bool bar;
};

///////////////////////////////////////////////////////////////////////////////

class SurvivorTimerTextWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	SurvivorTimerTextWindowConstructor( const std::string& window_name, const std::string& locales_name, bool countdown) :
		localesName( locales_name ),
		countdown( countdown )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		GenericTextWindow* result = new GenericTextWindow( ogui, game, player );
		result->loadDataFromLocales( localesName );

		GenericBarWindow *barwin = NULL;
		GenericTextWindow *textwin = NULL;

		if(countdown)
		{
			barwin = (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("CountdownBar");
			textwin = (GenericTextWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("CountdownTimer");
		}
		else
		{
			barwin = (GenericBarWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("CountupBar");
			textwin = (GenericTextWindow*)game->gameUI->getCombatWindow(0)->getSubWindow("CountupTimer");
		}

		int x = game::getLocaleGuiInt(("gui_"  + localesName + "_x").c_str(), 0);
		int y = game::getLocaleGuiInt(("gui_"  + localesName + "_y").c_str(), 0);
		int w = game::getLocaleGuiInt(("gui_"  + localesName + "_w").c_str(), 0);
		//int h = game::getLocaleGuiInt(("gui_"  + localesName + "_h").c_str(), 0);

		int x2 = 0, y2 = 0, w2 = 0, h2;

		if(barwin)
		{
			barwin->getWindowRect(x2,y2,w2,h2);

			// move
			result->getWindow()->MoveTo(x2 + (w2 - w) / 2 + x, y2 + y);
		}
		else if(textwin && textwin->getWindow())
		{
			x2 = textwin->getWindow()->GetPositionX();
			y2 = textwin->getWindow()->GetPositionY();
			w2 = textwin->getWindow()->GetSizeX();
			h2 = textwin->getWindow()->GetSizeY();

			// move
			result->getWindow()->MoveTo(x2 + (w2 - w) / 2 + x, y2 + y);
		}

		return result;
	}

	std::string localesName;
	bool countdown;
};

///////////////////////////////////////////////////////////////////////////////

class SurvivorGrenadeWindowUpdator : public IGenericTextWindowUpdator
{
public:
	SurvivorGrenadeWindowUpdator(Ogui *ogui, game::Game* game) : ogui(ogui), game(game)
	{
		unit = game->gameUI->getFirstPerson(0);
		first_run = true;

		desc = NULL;
		desc_font = NULL;
		img1 = NULL;
		img2 = NULL;
	}

	~SurvivorGrenadeWindowUpdator()
	{
		delete img1;
		delete img2;
		delete desc;
		delete desc_font;
	}

	void update(GenericTextWindow *win)
	{
		if(first_run)
		{

			{
				int x = game::getLocaleGuiInt("gui_grenadewindow_img1_x", 0);
				int y = game::getLocaleGuiInt("gui_grenadewindow_img1_y", 0);
				int w = game::getLocaleGuiInt("gui_grenadewindow_img1_w", 0);
				int h = game::getLocaleGuiInt("gui_grenadewindow_img1_h", 0);
				const char *txt = game::getLocaleGuiString("gui_grenadewindow_img1_img");
				img1 = ogui->CreateSimpleImageButton(win->getWindow(), x, y, w, h, NULL, NULL, NULL, txt, 0, 0, false);
				img1->SetDisabled(true);
			}

			{
				int x = game::getLocaleGuiInt("gui_grenadewindow_img2_x", 0);
				int y = game::getLocaleGuiInt("gui_grenadewindow_img2_y", 0);
				int w = game::getLocaleGuiInt("gui_grenadewindow_img2_w", 0);
				int h = game::getLocaleGuiInt("gui_grenadewindow_img2_h", 0);
				const char *txt = game::getLocaleGuiString("gui_grenadewindow_img2_img");
				img2 = ogui->CreateSimpleImageButton(win->getWindow(), x, y, w, h, NULL, NULL, NULL, txt, 0, 0, false);
				img2->SetDisabled(true);
			}

			{
				int x = game::getLocaleGuiInt("gui_grenadewindow_desc_x", 0);
				int y = game::getLocaleGuiInt("gui_grenadewindow_desc_y", 0);
				int w = game::getLocaleGuiInt("gui_grenadewindow_desc_w", 0);
				int h = game::getLocaleGuiInt("gui_grenadewindow_desc_h", 0);
				const char *txt = game::getLocaleGuiString("gui_grenadewindow_desc_text");
				const char *fnt = game::getLocaleGuiString("gui_grenadewindow_desc_font");
				desc_font = ogui->LoadFont(fnt);
				desc = ogui->CreateSimpleTextButton(win->getWindow(), x, y, w, h, NULL, NULL, NULL, txt, 0, 0, false);
				desc->SetDisabled(true);
				desc->SetFont(desc_font);
			}
			first_run = false;
		}
		if(unit && unit->getSelectedSecondaryWeapon() != -1)
		{
			int count = unit->getWeaponAmmoAmount( unit->getSelectedSecondaryWeapon() );
			win->setText(boost::lexical_cast<std::string>(count));
		}
	}
	bool first_run;
	Ogui *ogui;
	game::Game* game;
	game::Unit *unit;


	IOguiFont *desc_font;
	OguiButton *desc;
	OguiButton *img1;
	OguiButton *img2;
};

class SurvivorGrenadeWindowConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	SurvivorGrenadeWindowConstructor( const std::string& window_name)
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( window_name, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		GenericTextWindow* result = new GenericTextWindow( ogui, game, player );
		result->loadDataFromLocales( "grenadewindow" );

		result->setUpdator(new SurvivorGrenadeWindowUpdator(ogui, game));
		return result;
	}
};

///////////////////////////////////////////////////////////////////////////////

using namespace game;

class LaunchSpeedBarUpdater : public IGenericBarWindowUpdator
{
public:
	LaunchSpeedBarUpdater( game::Game* game, GenericBarWindow *win, int client ) :
		game( game ), client(client), win(win)
	{
		offset_x = game::getLocaleGuiInt("gui_grenadelaunchbar_x",0);
		offset_y = game::getLocaleGuiInt("gui_grenadelaunchbar_y",0);
	}

	float update()
	{
		if(!game || !game->gameUI)
			return 0;

		Unit *unit = game->gameUI->getFirstPerson(client);
		if(!unit)
			return 0;


		// get weapon that uses launch speed
		int wnum = -1;
		Weapon *w = NULL;
		if(unit->getSelectedWeapon() != -1)
		{
			wnum = unit->getSelectedWeapon();
			w = unit->getWeaponType(wnum);
			if(w && !w->usesLaunchSpeed())
			{
				// try attached weapon
				if(unit->getAttachedWeapon(wnum) != -1)
				{
					wnum = unit->getAttachedWeapon(wnum);
					w = unit->getWeaponType(wnum);
					if(w && !w->usesLaunchSpeed())
					{
						w = NULL;
					}
				}
				else
				{
					w = NULL;
				}
			}
		}

		// try secondary weapon
		if(w == NULL)
		{
			if(unit->getSelectedSecondaryWeapon() != -1 && unit->getWeaponType(unit->getSelectedSecondaryWeapon()))
			{
				wnum = unit->getSelectedSecondaryWeapon();
				w = unit->getWeaponType(wnum);
			}
		}
		
		// no weapon uses launchspeed
		if(!w || !w->usesLaunchSpeed())
		{
			if(!win->isHidden())
			 win->hide(0);

			return 0;
		}

		float value = unit->getLaunchSpeed() / w->getLaunchSpeedMax();

		if(!game->isPaused() && value > 0)
		{
			bool exact = game->gameUI->getGameCamera()->getGameCameraMode();
			int x = 0, y = 0, w, h;
			win->getWindowRect(x,y,w,h);
			win->moveBy(game->gameUI->getCursorScreenX(client, exact) - x + offset_x, game->gameUI->getCursorScreenY(client, exact) - y + offset_y);
			if(win->isHidden())
				win->show(0);
		}
		else
		{
			if(!win->isHidden())
			 win->hide(0);
		}

		return value;
	}

	game::Game* game;
	int client;
	int offset_x;
	int offset_y;
	GenericBarWindow *win;
};

class LaunchSpeedBarConstructor : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	LaunchSpeedBarConstructor(const std::string &name, int client) : name(name), client(client)
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow(name, this);
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		GenericBarWindow *win = new GenericBarWindow(ogui, game, client);
		win->loadDataFromLocales("gui_grenadelaunchbar");
		win->setUpdator(new LaunchSpeedBarUpdater(game, win, client));
		return win;
	}

	std::string name;
	int client;
};

///////////////////////////////////////////////////////////////////////////////

GenericBarGlobalsFromScriptsWindowConstructor*	surv_shield_bar = new GenericBarGlobalsFromScriptsWindowConstructor( "SurvivorShieldBar", "survivor_shield_bar", "player_shield_counter", "player_shield_max", "", false);
GenericBarTimerWindowConstructor*				surv_napalm_reload = new GenericBarTimerWindowConstructor( "SurvivorNapalmFlameBar", "survivor_napalmflame_bar", "survivor_napalm_flame_last_time", "survivor_napalm_flame_reload_length" );
GenericBarTimerWindowConstructor*				surv_killingspree_reload = new GenericBarTimerWindowConstructor( "SurvivorKillingSpreeReloadBar", "survivor_killingspree_reloadbar", "survivor_slomo_last_time", "survivor_slomo_reload_length" );
GenericBarTimerWindowConstructor*				surv_killingspree_usebar = new GenericBarTimerWindowConstructor( "SurvivorKillingSpreeUseBar", "survivor_killingspree_usebar", "survivor_slomo_current_time", "survivor_slomo_duration" );
KillingSpreeCountConstructor* surv_killingspree_counter = new KillingSpreeCountConstructor("SurvivorKillingSpreeCounter");
StupidWindowAligner *surv_aligner1 = new StupidWindowAligner("SurvivorShieldBarAligner", "SurvivorShieldBar");
StupidWindowAligner *surv_aligner2 = new StupidWindowAligner("SurvivorNapalmFlameBarAligner", "SurvivorNapalmFlameBar");
StupidWindowAligner *surv_aligner3 = new StupidWindowAligner("SurvivorKillingSpreeReloadBarAligner", "SurvivorKillingSpreeReloadBar");

// one for each player..
GenericBarGlobalsFromScriptsWindowConstructor*	surv_expo_bar0 = new GenericBarGlobalsFromScriptsWindowConstructor( "SurvivorExpoBar_0", "survivor_expo_bar_0", "survivor_current_expo[0]", "survivor_expo_max[0]", "survivor_expo_min[0]", true );
GenericBarGlobalsFromScriptsWindowConstructor*	surv_expo_bar1 = new GenericBarGlobalsFromScriptsWindowConstructor( "SurvivorExpoBar_1", "survivor_expo_bar_1", "survivor_current_expo[1]", "survivor_expo_max[1]", "survivor_expo_min[1]", true );
GenericBarGlobalsFromScriptsWindowConstructor*	surv_expo_bar2 = new GenericBarGlobalsFromScriptsWindowConstructor( "SurvivorExpoBar_2", "survivor_expo_bar_2", "survivor_current_expo[2]", "survivor_expo_max[2]", "survivor_expo_min[2]", true );
GenericBarGlobalsFromScriptsWindowConstructor*	surv_expo_bar3 = new GenericBarGlobalsFromScriptsWindowConstructor( "SurvivorExpoBar_3", "survivor_expo_bar_3", "survivor_current_expo[3]", "survivor_expo_max[3]", "survivor_expo_min[3]", true );

KillCountConstructor*	surv_kill_counter = new KillCountConstructor("SniperKillCounter", "Surv_Sniper", "sniper_kill_counter_vocals");

SurvivorTimerWindowConstructor*	survivor_timerwindow_down = new SurvivorTimerWindowConstructor("CountdownTimer", "survivor_countdowntimer", true, false);
SurvivorTimerWindowConstructor*	survivor_timerwindow_up = new SurvivorTimerWindowConstructor("CountupTimer", "survivor_countuptimer", false, false);
SurvivorTimerWindowConstructor*	survivor_barwindow_down = new SurvivorTimerWindowConstructor("CountdownBar", "gui_survivor_countdownbar", true, true);
SurvivorTimerWindowConstructor*	survivor_barwindow_up = new SurvivorTimerWindowConstructor("CountupBar", "gui_survivor_countupbar", false, true);

SurvivorTimerTextWindowConstructor*	survivor_barwindow_text_down = new SurvivorTimerTextWindowConstructor("CountdownText", "survivor_countertext", true);
SurvivorTimerTextWindowConstructor*	survivor_barwindow_text_up = new SurvivorTimerTextWindowConstructor("CountupText", "survivor_countertext", false);
SurvivorTimerTextWindowConstructor*	survivor_barwindow_text_down2 = new SurvivorTimerTextWindowConstructor("CountdownText2", "survivor_countertext2", true);
SurvivorTimerTextWindowConstructor*	survivor_barwindow_text_up2 = new SurvivorTimerTextWindowConstructor("CountupText2", "survivor_countertext2", false);


SurvivorWeaponWindowConstructor*	surv_marine_weapons = new SurvivorWeaponWindowConstructor( "SurvivorMarineWeaponWindow", "marine_" );
SurvivorWeaponWindowConstructor*	surv_napalm_weapons = new SurvivorWeaponWindowConstructor( "SurvivorNapalmWeaponWindow", "napalm_" );
SurvivorWeaponWindowConstructor*	surv_spiner_weapons = new SurvivorWeaponWindowConstructor( "SurvivorSniperWeaponWindow", "sniper_" );
SurvivorWeaponWindowConstructor*	surv_coop_weapons = new SurvivorWeaponWindowConstructor( "WeaponWindowCoop", "coop" );
LaunchSpeedBarConstructor* launchspeedbar1 = new LaunchSpeedBarConstructor("GrenadeLaunchBar_0", 0);
LaunchSpeedBarConstructor* launchspeedbar2 = new LaunchSpeedBarConstructor("GrenadeLaunchBar_1", 1);
LaunchSpeedBarConstructor* launchspeedbar3 = new LaunchSpeedBarConstructor("GrenadeLaunchBar_2", 2);
LaunchSpeedBarConstructor* launchspeedbar4 = new LaunchSpeedBarConstructor("GrenadeLaunchBar_3", 3);
SurvivorGrenadeWindowConstructor*	grenadewin = new SurvivorGrenadeWindowConstructor( "GrenadeWindow" );

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui
