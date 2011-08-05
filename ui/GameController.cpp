#include "precompiled.h"

#include <fstream>
#include <boost/lexical_cast.hpp>

#include "GameController.h"

#include <keyb3.h>
#include <assert.h>
#include <string.h>
#include <string>

#include "../system/Logger.h"
#include "../system/Timer.h"
#include "../convert/str2int.h"
//#include "../util/Parser.h"
#include "../editor/parser.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include "../ogui/Ogui.h"

#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "../game/options/options_controllers.h"
#include "../game/GameConfigs.h"
#include "../game/GameOptionManager.h"

#include "../util/Debug_MemoryManager.h"
#include "igios.h"

#define KEYCODE_NAME_AMOUNT (534 + ADDITIONAL_KEYBOARD_KEYS_AMOUNT)
//#define KEYCODE_NAME_AMOUNT 534 
#define KEYREPEAT_FIRST_DELAY 800
#define KEYREPEAT_DELAY 80

using namespace frozenbyte;

namespace ui
{

const char *ctrlName[DH_CTRL_AMOUNT + 1] =
{
	"dummy",
	"camera_move_forward",
	"camera_move_backward",
	"camera_move_left",
	"camera_move_right",
	"camera_move_rotate_left",
	"camera_move_rotate_right",
	"camera_move_orbit_left",
	"camera_move_orbit_right",
	"camera_move_zoom_in",
	"camera_move_zoom_out",
	"camera_move_up",
	"camera_move_down",
	"camera_move_rotate_up",
	"camera_move_rotate_down",
	"camera_move_zoom_next",

	"camera_to_selected_unit",
	"camera_to_next_unit",
	"camera_unit_lock_toggle",
	"camera_behind_selected_unit",
	"camera_align_to_selected_unit",
	"camera_first_person_toggle",
	"camera_to_action",
	"camera_action_lock_toggle",
	"camera_terrain_lock_toggle",

	"gui_toggle",
	"gui_next_layout",

	"multiple_unit_select",
	"force_attack",

	"attack",
	"change_next_weapon",
	"unit_mode_toggle",

	"select_all_units",
	"select_unit_1",
	"select_unit_2",
	"select_unit_3",
	"select_unit_4",
	"select_unit_5",
	"select_unit_6",
	"select_team_1",
	"select_team_2",
	"select_team_3",

	"quit",
	"camera_move_zoom_mode",

	"gui_radar_toggle",
	"gui_units_toggle",
	
	"next_camera_mode",
	"camera_look_mode",
	"screenshot",
	"camera_unit_align_lock_toggle",

	"pause",
	"tactical_mode",

	"camera_move_fov_in",
	"camera_move_fov_out",
	
	"next_camera_angle_boundary",
	"special_move",
	"stop",
	"cease_fire",
	"stop_and_cease_fire",

	"console_toggle",

	"grenade",
	"change_prev_weapon",

	"reload",
	"flashlight",
	"execute",

	"attack_secondary",
	"use_medikit",
	"weapon_select_mode",

	"control_mode_switch",
	"other_unit_control",
	"select_next_unit",

	"_reserved_71",
	"_reserved_72",
	"_reserved_73",
	"_reserved_74",

	"run_script_1",
	"run_script_2",
	"run_script_3",
	"run_script_4",
	"run_script_5",
	"run_script_6",
	"run_script_7",
	"run_script_8",
	"run_script_9",
	"run_script_10",
	"run_script_11",
	"run_script_12",
	"run_script_13",
	"run_script_14",
	"run_script_15",
	"run_script_16",

	"weapon_1",
	"weapon_2",
	"weapon_3",
	"weapon_4",
	"weapon_5",
	"weapon_6",
	"weapon_7",
	"weapon_8",
	"weapon_9",
	"weapon_10",
	"weapon_11",
	"weapon_12",
	"weapon_13",
	"weapon_14",
	"weapon_15",
	"weapon_16",

	"open_anirecorder",

	"open_upgrade",

	"use_selected_item",

	"open_map",

	"open_log",

	"close_loading_window",

	"camera_target_offset_mode",
	"camera_position_offset_mode",

	"sprint",

	"open_menu",

	"***"
};

// NOTICE: the wheel up and wheel down are inverted here!
// this is because keyb3 seems to give them inverted, so we fix that here.
char *keycodeName[KEYCODE_NAME_AMOUNT] =
{
	"none", "esc", "1", "2", "3", "4", "5", "6", "7", "8", 
	"9", "0", "+", "", "backspace", "tab", "q", "w", "e", "r",
	"t", "y", "u", "i", "o", "p", "å", "", "enter", "ctrl-l",
	"a", "s", "d", "f", "g", "h", "j", "k", "l", "ä",
	"ö", "tilde", "shift-l", "", "z", "x", "c", "v", "b", "n",
	"m", "", "", "backslash", "shift-r", "pad-multiply", "alt", "space", "capslock", "f1",
	"f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "numlock",
	"scrolllock", "pad-7", "pad-8", "pad-9", "pad-minus", "pad-4", "pad-5", "pad-6", "pad-plus", "pad-1",
	"pad-2", "pad-3", "pad-0", "pad-dot", "", "", "", "f11", "f12", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "pad-enter", "ctrl-r", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "pad-divide", "", "printscreen", "alt-gr", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "home",
	"up", "pageup", "", "left", "", "right", "", "", "down", "pagedown",
	"insert", "delete", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "mouse-up", "mouse-down", "mouse-left", "mouse-right",
	"wheel-down", "wheel-up", "button-1", "button-2", "button-3", "button-4", "button-5", "button-6", "button-7", "button-8",
	"joy-up", "joy-down", "joy-left", "joy-right", "joy-but-1", "joy-but-2", "joy-but-3", "joy-but-4", "joy-but-5", "joy-but-6",
	"joy-but-7", "joy-but-8", "joy-but-9", "joy-but-10", "joy-but-11", "joy-but-12", "joy-but-13", "joy-but-14", "joy-but-15", "joy-but-16",
	"joy2-up", "joy2-down", "joy2-left", "joy2-right", "joy2-but-1", "joy2-but-2", "joy2-but-3", "joy2-but-4", "joy2-but-5", "joy2-but-6",
	"joy2-but-7", "joy2-but-8", "joy2-but-9", "joy2-but-10", "joy2-but-11", "joy2-but-12", "joy2-but-13", "joy2-but-14", "joy2-but-15", "joy2-but-16",
	"joy-rot-up", "joy-rot-down", "joy-rot-left", "joy-rot-right", "joy-throttle-up", "joy-throttle-down", "joy-rudder-left", "joy-rudder-right",
	"joy2-rot-up", "joy2-rot-down", "joy2-rot-left", "joy2-rot-right", "joy2-throttle-up", "joy2-throttle-down", "joy2-rudder-left", "joy2-rudder-right",

	"joy3-rot-up", "joy3-rot-down", "joy3-rot-left", "joy3-rot-right", "joy3-throttle-up", "joy3-throttle-down", "joy3-rudder-left", "joy3-rudder-right",
	"joy4-rot-up", "joy4-rot-down", "joy4-rot-left", "joy4-rot-right", "joy4-throttle-up", "joy4-throttle-down", "joy4-rudder-left", "joy4-rudder-right",

	"joy3-up",		"joy3-down",	"joy3-left", "joy3-right", "joy3-but-1", "joy3-but-2", "joy3-but-3", "joy3-but-4",			"joy3-but-5",	"joy3-but-6",
	"joy3-but-7",	"joy3-but-8",	"joy3-but-9", "joy3-but-10", "joy3-but-11", "joy3-but-12", "joy3-but-13", "joy3-but-14",	"joy3-but-15",	"joy3-but-16",
	"joy4-up",		"joy4-down",	"joy4-left", "joy4-right", "joy4-but-1", "joy4-but-2", "joy4-but-3", "joy4-but-4",			"joy4-but-5",	"joy4-but-6",
	"joy4-but-7",	"joy4-but-8",	"joy4-but-9", "joy4-but-10", "joy4-but-11", "joy4-but-12", "joy4-but-13", "joy4-but-14",	"joy4-but-15",	"joy4-but-16",
	
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",

	"mouse0-up", "mouse0-down", "mouse0-left", "mouse0-right", "wheel0-down", "wheel0-up", "button0-8", "button0-7", "button0-6", "button0-5",
	"button0-4", "button0-3", "button0-2", "button0-1", "", "", "", "", "", "",

	"mouse1-up", "mouse1-down", "mouse1-left", "mouse1-right", "wheel1-down", "wheel1-up", "button1-8", "button1-7", "button1-6", "button1-5",
	"button1-4", "button1-3", "button1-2", "button1-1", "", "", "", "", "", "",

	"mouse2-up", "mouse2-down", "mouse2-left", "mouse2-right", "wheel2-down", "wheel2-up", "button2-8", "button2-7", "button2-6", "button2-5",
	"button2-4", "button2-3", "button2-2", "button2-1", "", "", "", "", "", "",

	"mouse3-up", "mouse3-down", "mouse3-left", "mouse3-right", "wheel3-down", "wheel3-up", "button3-8", "button3-7", "button3-6", "button3-5",
	"button3-4", "button3-3", "button3-2", "button3-1", "", "", "", "", "", "",

	"mouse4-up", "mouse4-down", "mouse4-left", "mouse4-right", "wheel4-down", "wheel4-up", "button4-8", "button4-7", "button4-6", "button4-5",
	"button4-4", "button4-3", "button4-2", "button4-1", "", "", "", "", "", "",

	"mouse5-up", "mouse5-down", "mouse5-left", "mouse5-right", "wheel5-down", "wheel5-up", "button5-8", "button5-7", "button5-6", "button5-5",
	"button5-4", "button5-3", "button5-2", "button5-1", "", "", "", "", "", "",

	"mouse6-up", "mouse6-down", "mouse6-left", "mouse6-right", "wheel6-down", "wheel6-up", "button6-8", "button6-7", "button6-6", "button6-5",
	"button6-4", "button6-3", "button6-2", "button6-1"

};


int gamecontroller_next_keyreader_id = 1;

///////////////////////////////////////////////////////////////////////////////

const std::string& joystickMoveXAxisConfName = "joystick_move_x_axis";
const std::string& joystickMoveYAxisConfName = "joystick_move_y_axis";
const std::string& joystickDirXAxisConfName = "joystick_dir_x_axis";
const std::string& joystickDirYAxisConfName = "joystick_dir_y_axis";
const std::string& controllerTypeConfName = "controller_type";

GameController::JOYSTICK_AXIS convertToEnum( const std::string& value )
{
	if( value == "axis_x" )
		return GameController::JOYSTICK_AXIS_X;
	else if ( value == "axis_y" )
		return GameController::JOYSTICK_AXIS_Y;
	else if	( value == "axis_rx" )
		return GameController::JOYSTICK_AXIS_RX;
	else if ( value == "axis_ry" )
		return GameController::JOYSTICK_AXIS_RY;
	else if ( value == "axis_throttle" )
		return GameController::JOYSTICK_AXIS_THROTTLE;
	else if ( value == "axis_rudder" )
		return GameController::JOYSTICK_AXIS_RUDDER;

	Logger::getInstance()->warning(( "GameController::convertToEnum  and unidentified joystick axis: " + value ).c_str() );

	return GameController::JOYSTICK_AXIS_X;
}

std::string convertToString( GameController::JOYSTICK_AXIS axis )
{
	switch( axis )
	{
	case GameController::JOYSTICK_AXIS_X:
		return "axis_x";
	case GameController::JOYSTICK_AXIS_Y:
		return "axis_y";
	case GameController::JOYSTICK_AXIS_RX:
		return "axis_rx";
	case GameController::JOYSTICK_AXIS_RY:
		return "axis_ry";
	case GameController::JOYSTICK_AXIS_THROTTLE:
		return "axis_throttle";
	case GameController::JOYSTICK_AXIS_RUDDER:
		return "axis_rudder";
	case GameController::JOYSTICK_AXIS_UNKNOWN:
		return "";
	}

	return "";
}

struct JoystickValues
{
	int x;
	int y;
	int rx;
	int ry;
	int throttle;
	int rudder;
};

///////////////////////////////////////////////////////////////////////////////

int gameControllerInstancesInitialized = 0;

GameController::GameController(Ogui *ogui)
{
	if(gameControllerInstancesInitialized == 0)
	{
		// Name additional keyboards. (such like: "enter (kb2)")
		// TODO: Cleanup
		int kl = BASIC_KEYCODE_AMOUNT;
		for(int l = 1; l <= MAX_KEYBOARDS; l++)
		{
			for(int k = 0; k < MAX_KEYS; k++)
			{
				int len = strlen(keycodeName[k]);
				keycodeName[kl] = new char [ len + 7 ];
				strcpy(keycodeName[kl], keycodeName[k]);
				keycodeName[kl][len++] = ' ';
				keycodeName[kl][len++] = '(';
				keycodeName[kl][len++] = 'k';
				keycodeName[kl][len++] = 'b';
				keycodeName[kl][len++] = '1' + l - 1;
				keycodeName[kl][len++] = ')';
				keycodeName[kl][len++] = 0;
				kl++;
			}
		}
	}

	gameControllerInstancesInitialized++;

  this->ogui = ogui;

#ifdef _DEBUG
	if (ctrlName[DH_CTRL_AMOUNT] == NULL
		|| strcmp(ctrlName[DH_CTRL_AMOUNT], "***") != 0)
	{
		assert(!"GameController - Controller name array invalid.");
	}
#endif

	binds = new int *[DH_CTRL_AMOUNT];
	for (int i = 0; i < DH_CTRL_AMOUNT; i++)
	{
		binds[i] = new int[DH_CTRL_BINDS_PER_CONTROL];
		for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			binds[i][j] = 0;
		}
		ctrlOn[i] = false;
		prevCtrlOn[i] = false;
		ctrlForceEnabled[i] = false;
	}
	Timer::update();
	wheelUpStartTime = Timer::getTime();
	wheelDownStartTime = Timer::getTime();
	userControlsEnabled = true;
	controlsEnabled = true;
	lastKeyReadTime = Timer::getTime();
	lastNoKeysDownTime = Timer::getTime();
	noKeysDownAtLastRead = true;
	keyreader = NULL;
	keyreaderId = 0;
	{
		for (int j = 0; j < GAMECONTROLLER_MAX_KEYREADERS; j++)
		{
			keyreaderIdStack[j] = 0;
			keyreaderStack[j] = NULL;
		}
	}
	readKeyWritePos = 0;
	readKeyReadPos = 0;
	{
		for (int j = 0; j < GAMECONTROLLER_KEYREADER_BUFFER_SIZE; j++)
		{
			readKeyAsciiBuf[j] = 0;
			readKeyKeycodeBuf[j] = 0;
		}
	}

	joystickMaxiumValues[0] = new JoystickValues;
	joystickMaxiumValues[1] = new JoystickValues;
	joystickMaxiumValues[2] = new JoystickValues;
	joystickMaxiumValues[3] = new JoystickValues;

	controllerType = CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;

	mouseJoystickAutodetection = false;
}

GameController::~GameController()
{
	gameControllerInstancesInitialized--;
	for (int i = 0; i < DH_CTRL_AMOUNT; i++)
	{
		delete [] binds[i];
	}
	delete [] binds;

	delete joystickMaxiumValues[0];
	delete joystickMaxiumValues[1];
	delete joystickMaxiumValues[2];
	delete joystickMaxiumValues[3];
}

void GameController::unloadConfiguration()
{
	for (int i = 0; i < DH_CTRL_AMOUNT; i++)
	{
		for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			binds[i][j] = 0;
		}
		ctrlOn[i] = false;
		prevCtrlOn[i] = false;
		ctrlForceEnabled[i] = false;
	}
	Timer::update();
	wheelUpStartTime = Timer::getTime();
	wheelDownStartTime = Timer::getTime();
	userControlsEnabled = true;
	controlsEnabled = true;
	lastKeyReadTime = Timer::getTime();
	lastNoKeysDownTime = Timer::getTime();
	noKeysDownAtLastRead = true;
	keyreader = NULL;
	keyreaderId = 0;
	{
		for (int j = 0; j < GAMECONTROLLER_MAX_KEYREADERS; j++)
		{
			keyreaderIdStack[j] = 0;
			keyreaderStack[j] = NULL;
		}
	}
	readKeyWritePos = 0;
	readKeyReadPos = 0;
	{
		for (int j = 0; j < GAMECONTROLLER_KEYREADER_BUFFER_SIZE; j++)
		{
			readKeyAsciiBuf[j] = 0;
			readKeyKeycodeBuf[j] = 0;
		}
	}
}

int GameController::getControlNumberForName(const char *forCtrlName)
{
	if (forCtrlName == NULL)
		return -1;

	for (int i = 0; i < DH_CTRL_AMOUNT; i++)
	{
		if (strcmp(forCtrlName, ctrlName[i]) == 0)
		{
			return i;
		}
	} 

	return -1;
}


int GameController::getKeycodeNumberForName(const char *forKeycodeName)
{
	if (forKeycodeName == NULL)
		return -1;

	for (int i = 0; i < KEYCODE_NAME_AMOUNT; i++)
	{
		if (strcmp(forKeycodeName, keycodeName[i]) == 0)
		{
			return i;
		}
	} 

	return -1;
}


void GameController::loadConfiguration( const char *filename)
{
	loadedConfFromHere = filename;
	// clear previous configuration
	for (int k = 0; k < DH_CTRL_AMOUNT; k++)
	{
		for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			binds[k][j] = 0;
		}
	}

	//Parser::Parser conf(filename);
	editor::EditorParser conf(true, false);
	filesystem::InputStream conf_file = filesystem::FilePackageManager::getInstance().getFile(filename);
	conf_file >> conf;

	// Get the joystick axes
	joystickMoveXAxis = convertToEnum( conf.getGlobals().getValue( joystickMoveXAxisConfName ) );
	joystickMoveYAxis = convertToEnum( conf.getGlobals().getValue( joystickMoveYAxisConfName ) );
	joystickDirXAxis = convertToEnum( conf.getGlobals().getValue( joystickDirXAxisConfName ) );
	joystickDirYAxis = convertToEnum( conf.getGlobals().getValue( joystickDirYAxisConfName ) );

	// Controller type
	try
	{
		int type = boost::lexical_cast< int >( conf.getGlobals().getValue( controllerTypeConfName ) );
		// legacy support
		if(type < 100)
		{
			if(type == -1)
				controllerType = CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;
			else
				controllerType = (CONTROLLER_TYPE)(CONTROLLER_TYPE_JOYSTICK1 + type);
		}
		else
		{
			controllerType = (CONTROLLER_TYPE)type;
		}
	}
	catch(...)
	{
		controllerType = CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;
	}


	for (int i = 0; i < DH_CTRL_AMOUNT; i++)
	{
		//std::string valueStr = Parser::GetString(conf.GetProperties(), ctrlName[i]);
		std::string valueStr = conf.getGlobals().getValue(ctrlName[i]);

		char *value = NULL;
		value = const_cast<char *> (valueStr.c_str());
		if (value != NULL && value[0] != '\0')
		{
			// for each comma seperated string...
			int valuelen = strlen(value);
			int lastpos = 0;
			for (int pos = 0; pos < valuelen + 1; pos++)
			{
				if (value[pos] == ',' || value[pos] == '\0')
				{
					// first solve keycode based on the string value given...
					int keycode = 0;
					if (value[0] == '#')
					{
						// was in #number format
						char *buf = new char[strlen(&value[lastpos + 1]) + 1];
						strcpy(buf, &value[lastpos + 1]);
						for (int j = 0; j < (int)strlen(buf); j++)
						{
							if (buf[j] == ',') buf[j] = '\0';
						}
						keycode = str2int(buf);
						delete [] buf;
					} else {
						bool nameok = false;
						// was in keycode name string format
						for (int j = 0; j < KEYCODE_NAME_AMOUNT; j++)
						{
							if (keycodeName[j] != NULL && keycodeName[j][0] != '\0')
							{
								int cmplen = strlen(keycodeName[j]);
								if (pos - lastpos > cmplen) cmplen = pos - lastpos;
								if (strncmp(keycodeName[j], &value[lastpos], cmplen) == 0)
								{
									keycode = j;
									nameok = true;
									break;
								}
							}
						}
						if (!nameok)
						{
							char *buf = new char[120 + strlen(ctrlName[i]) + 1];
							strcpy(buf, "GameController::loadConfiguration - Unknown key bound to \"");
							strcat(buf, ctrlName[i]);
							strcat(buf, "\" (value ");
							strcat(buf, value);
							strcat(buf, ").");
							Logger::getInstance()->warning(buf);
							delete [] buf;
						}
					}
					if (keycode != 0)
					{
						// just to check that we have not bound too many keycodes to 
						// the control
						bool isok = false;
						for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
						{
							if (binds[i][j] == 0)
							{
								isok = true;
								break;
							}
						}
						if (!isok)
						{
							char *buf = new char[120 + strlen(ctrlName[i]) + 1];
							strcpy(buf, "GameController::loadConfiguration - Too many keys bound to \"");
							strcat(buf, ctrlName[i]);
							strcat(buf, "\" (value ");
							strcat(buf, value);
							strcat(buf, ").");
							Logger::getInstance()->warning(buf);
							delete [] buf;
						} else {
							// then bind it...
							bindKey(i, keycode, -1, false);
						}
					}

					lastpos = pos + 1;
				}
			}
		}
	}

}

void GameController::saveConfiguration( const char *filename )
{
	// this assert is incorrect when options menu "reset to defaults" is used --jpk
	//assert( loadedConfFromHere == filename );

	std::fstream file( filename, std::ios::out );

	file << "// Extra padding for loading" << std::endl << std::endl;
	
	file << joystickMoveXAxisConfName << " = " << convertToString( joystickMoveXAxis ) << std::endl;
	file << joystickMoveYAxisConfName << " = " << convertToString( joystickMoveYAxis ) << std::endl;
	file << joystickDirXAxisConfName << " = " << convertToString( joystickDirXAxis ) << std::endl;
	file << joystickDirYAxisConfName << " = " << convertToString( joystickDirYAxis ) << std::endl;

	file << controllerTypeConfName << " = " << (int)controllerType << std::endl;


	for (int k = 0; k < DH_CTRL_AMOUNT; k++)
	{
		std::string keyname = ctrlName[ k ];
		std::string keys = "";

		for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			if( binds[k][j] != 0 )
			{
				if( !keys.empty() ) keys += ",";

				// TODO: should handle keys with no name using the #xxx notation...
				if (keycodeName[ binds[k][j] ][0] == '\0')
				{
					keys += "#" + binds[k][j];
				} else {
					keys += keycodeName[ binds[k][j] ];
				}
			}
		}

		if( !keys.empty() )
		{
			file << keyname << " = " << keys << std::endl;
		}
	}

	file << std::endl;

	file.close();
}

void GameController::reloadConfiguration()
{
	loadConfiguration( loadedConfFromHere.c_str() );
}

const char *GameController::getControlName(int controlNum)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::getControlName - Control number out of bounds.");
		return NULL;
	} 


	return ctrlName[controlNum];
}

const char* GameController::getKeycodeName( int keycode )
{
	if( keycode < 0 || keycode >= KEYCODE_NAME_AMOUNT )
	{
		Logger::getInstance()->error("GameController::getKeycodeName - Control number out of bounds.");
		return NULL;
	}

	return keycodeName[ keycode ];
}


void GameController::setForcedEnable(int controlNum, bool ctrlEnabled)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::setForcedEnable - Control number out of bounds.");
		return;
	} 

	ctrlForceEnabled[controlNum] = ctrlEnabled;
}


bool GameController::isKeyDown(int controlNum)
{
#ifdef _DEBUG
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::isKeyDown - Control number out of bounds.");
		return false;
	} 
#endif

	if (!controlsEnabled)
	{
		if (controlNum != DH_CTRL_QUIT
			&& controlNum != DH_CTRL_CONSOLE_TOGGLE
			&& controlNum != DH_CTRL_SCREENSHOT
			&& !ctrlForceEnabled[controlNum])
			return false;
	}

	if (ctrlOn[controlNum])
		return true;

	for (int i = 0; i < DH_CTRL_BINDS_PER_CONTROL; i++)
	{
		int keycode = binds[controlNum][i];
		if (keycode != 0)
		{
			if (keycode == KEYCODE_MOUSE_WHEEL_UP
				|| keycode == KEYCODE_MOUSE_WHEEL_DOWN)
			{
				int curTime = Timer::getTime();
				if (Keyb3_IsKeyDown(keycode))
				{
					if (keycode == KEYCODE_MOUSE_WHEEL_UP)
					{
						if (curTime < wheelUpStartTime + 100)
							wheelUpStartTime += 100;
						else
							wheelUpStartTime = curTime;
					}
					if (keycode == KEYCODE_MOUSE_WHEEL_DOWN)
					{
						if (curTime < wheelDownStartTime + 100)
							wheelDownStartTime += 100;
						else
							wheelDownStartTime = curTime;
					}
				}
				if (keycode == KEYCODE_MOUSE_WHEEL_UP)
				{
					if (curTime < wheelUpStartTime + 100)
					{
						return true;
					}
				}
				if (keycode == KEYCODE_MOUSE_WHEEL_DOWN)
				{
					if (curTime < wheelDownStartTime + 100)
					{
						return true;
					}
				}
			} else {
				if (Keyb3_IsKeyDown(keycode))
					return true;
			}
		}
	}
	return false;
}

bool GameController::isKeyDownByKeyCode( int keycode )
{
	return Keyb3_IsKeyDown( keycode )?true:false;
}

bool GameController::wasKeyClicked(int controlNum)
{
#ifdef _DEBUG
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::isKeyDown - Control number out of bounds.");
		return false;
	} 
#endif

	if (!controlsEnabled)
	{
		if (controlNum != DH_CTRL_QUIT
			&& controlNum != DH_CTRL_CONSOLE_TOGGLE
			&& controlNum != DH_CTRL_SCREENSHOT
			&& controlNum != DH_CTRL_CLOSE_LOADING_WINDOW
			&& !ctrlForceEnabled[controlNum])
			return false;
	}

	if (!ctrlOn[controlNum] && prevCtrlOn[controlNum])
	{
		return true;
	}

	for (int i = 0; i < DH_CTRL_BINDS_PER_CONTROL; i++)
	{
		int keycode = binds[controlNum][i];
		if (keycode != 0)
		{
			if (keycode == KEYCODE_MOUSE_WHEEL_UP
				|| keycode == KEYCODE_MOUSE_WHEEL_DOWN)
			{
				int curTime = Timer::getTime();
				if (Keyb3_IsKeyDown(keycode))
				{
					if (keycode == KEYCODE_MOUSE_WHEEL_UP)
					{
						if (curTime < wheelUpStartTime + 100)
						{
							wheelUpStartTime = curTime;
						} else {
							wheelUpStartTime = curTime;
							return true;
						}
					}
					if (keycode == KEYCODE_MOUSE_WHEEL_DOWN)
					{
						if (curTime < wheelDownStartTime + 100)
						{
							wheelDownStartTime = curTime;
						} else {
							wheelDownStartTime = curTime;
							return true;
						}
					}
				}
			} else {
				if (Keyb3_IsKeyPressed(binds[controlNum][i]))
					return true;
			}
		}
	}
	return false;
}

bool GameController::isAnyKeyBound(int controlNum)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::isAnyKeyBound - Control number out of bounds.");
		return false;
	} 
	for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
	{
		if (binds[controlNum][j] != 0)
			return true;
	}
	return false;
}

int GameController::getBoundKey(int controlNum, int alternativeNum)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::getBoundKey - Control number out of bounds.");
		return 0;
	} 
	if (alternativeNum < 0 || alternativeNum >= DH_CTRL_BINDS_PER_CONTROL)
	{
		Logger::getInstance()->error("GameController::getBoundKey - Alternative number out of bounds.");
		return 0;
	} 
	return binds[controlNum][alternativeNum];
}

void GameController::bindKey(int controlNum, int keycode, int alternativeNum, 
	bool clearFromOthers)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::bindKey - Control number out of bounds.");
		return;
	} 
	if (alternativeNum != -1)
	{
		if (alternativeNum < 0 || alternativeNum >= DH_CTRL_BINDS_PER_CONTROL)
		{
			Logger::getInstance()->error("GameController::bindKey - Alternative number out of bounds.");
			return;
		} 
	} else {
		// do magic!
		// already one of alternatives?
		int j;
		for (j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			if (binds[controlNum][j] == keycode)
				return;
		}
		// was not, then find an empty alternative
		for (j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			if (binds[controlNum][j] == 0)
			{
				alternativeNum = j;
				break;
			}
		}
		if (alternativeNum == -1) 
		{
			// still no luck? clear them all and use the first!
			for (j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
			{
				binds[controlNum][j] = 0;
			}
			alternativeNum = 0;
		}
	}
	if (clearFromOthers && keycode != 0)
	{
		for (int i = 0; i < DH_CTRL_AMOUNT; i++)
		{
			for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
			{
				if (binds[i][j] == keycode)
					binds[i][j] = 0;
			}
		}
	}
	binds[controlNum][alternativeNum] = keycode;
}


void GameController::unbindKey(int controlNum, int alternativeNum)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::unbindKey - Control number out of bounds.");
		return;
	} 
	if (alternativeNum != -1)
	{
		if (alternativeNum < 0 || alternativeNum >= DH_CTRL_BINDS_PER_CONTROL)
		{
			Logger::getInstance()->error("GameController::unbindKey - Alternative number out of bounds.");
			return;
		} 
		binds[controlNum][alternativeNum] = 0;
	} else {
		for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			 binds[controlNum][j] = 0;
		}
	}
}


void GameController::unbindKeyByKeycode(int keycode)
{
	for (int i = 0; i < DH_CTRL_AMOUNT; i++)
	{
		for (int j = 0; j < DH_CTRL_BINDS_PER_CONTROL; j++)
		{
			if (binds[i][j] == keycode)
			{
				binds[i][j] = 0;
			}
		}
	}
}


void HACKgetMouseDelta(int *deltaX, int *deltaY, int mouseID)
{
	Keyb3_ReadMouse(NULL, NULL, deltaX, deltaY, mouseID);
}

void GameController::setMouseJoystickAutodetection( bool value )
{
	mouseJoystickAutodetection = value;
}

void GameController::getMouseDelta( int *deltaX, int *deltaY, int mouseID )
{
	if( mouseJoystickAutodetection == false )
	{
		HACKgetMouseDelta( deltaX, deltaY, mouseID );
		return;
	}

	if( controllerTypeHasMouse() ) //game::SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR )
	{
		HACKgetMouseDelta( deltaX, deltaY, mouseID );
	}
	else if( controllerTypeHasJoystick() )
	{
		float joystick_sensitivy = game::SimpleOptions::getFloat( DH_OPT_F_JOYSTICK_SENSITIVY );

		// we don't care about direction axes, only movement axes
		getJoystickValues(deltaX, deltaY, NULL, NULL);

		(*deltaX) = (int)( (float)(*deltaX) * joystick_sensitivy * 2.0f );
		(*deltaY) = (int)( (float)(*deltaY) * joystick_sensitivy * 2.0f );
	}
	else
	{
		// keyboard-only

		if( isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT) )
			(*deltaX) = -10;
		else if( isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT) )
			(*deltaX) = 10;
		else
			(*deltaX) = 0;

		if( isKeyDown(DH_CTRL_CAMERA_MOVE_FORWARD) )
			(*deltaY) = -10;
		else if( isKeyDown(DH_CTRL_CAMERA_MOVE_BACKWARD) )
			(*deltaY) = 10;
		else
			(*deltaY) = 0;
	}
}

void GameController::updateEnabledStatus()
{
	if (userControlsEnabled && keyreader == NULL) 	
		controlsEnabled = true; 
	else
		controlsEnabled = false;	

	if (keyreader == NULL)
		ogui->setHotkeysEnabled(true);
	else
		ogui->setHotkeysEnabled(false);
}


void GameController::setControlsEnabled(bool enabled)
{
	// clear "forced enable controls" when state changes...
	if (userControlsEnabled != enabled)
	{
		for (int i = 0; i < DH_CTRL_AMOUNT; i++)
		{
			ctrlForceEnabled[i] = false;
		}
	}
	userControlsEnabled = enabled;
	updateEnabledStatus();
}

bool GameController::getUserControlsEnabled() const
{
	return userControlsEnabled;
}

/*
bool GameController::getControlsEnabled() const
{
	return controlsEnabled;
}
*/

int GameController::addKeyreader(IGameControllerKeyreader *keyreader)
{
	if (keyreaderStack[GAMECONTROLLER_MAX_KEYREADERS - 1] != NULL)
	{
		Logger::getInstance()->error("GameController::addKeyreader - Too many keyreaders (oldest keyreader information dropped).");
	}

	for (int i = GAMECONTROLLER_MAX_KEYREADERS-1; i >= 0; i--)
	{
		if (i == 0)
		{
			keyreaderStack[i] = this->keyreader;
			keyreaderIdStack[i] = keyreaderId;
		} else {
			keyreaderStack[i] = keyreaderStack[i-1];
			keyreaderIdStack[i] = keyreaderIdStack[i-1];
		}
	}

	this->keyreader = keyreader;
	this->keyreaderId = gamecontroller_next_keyreader_id;
	updateEnabledStatus();

	// WARNING: assuming that int value never overflows (we don't have billions of keyreaders, do we? ;)
	gamecontroller_next_keyreader_id++;

	return this->keyreaderId;
}

void GameController::removeKeyreader(int id)
{
	if (id == 0)
	{
		Logger::getInstance()->error("GameController::removeKeyreader - Invalid id parameter (zero) given.");
		return;
	}

	int delpos = GAMECONTROLLER_MAX_KEYREADERS;

	if (keyreaderId == id)
	{
		assert(keyreader != NULL);
		delpos = -1;
	} else {
		for (int i = 0; i < GAMECONTROLLER_MAX_KEYREADERS; i++)
		{
			if (keyreaderIdStack[i] == id)
			{
				assert(keyreaderStack[i] != NULL);
				delpos = i;
				break;
			}
		}
	}

	if (delpos == GAMECONTROLLER_MAX_KEYREADERS)
	{
		Logger::getInstance()->error("GameController::removeKeyreader - No keyreader found with given id.");
	} else {
		for (int i = delpos; i < GAMECONTROLLER_MAX_KEYREADERS-1; i++)
		{
			if (i == -1)
			{
				keyreader = keyreaderStack[i + 1];
				keyreaderId = keyreaderIdStack[i + 1];
			} else {
				keyreaderStack[i] = keyreaderStack[i + 1];
				keyreaderIdStack[i] = keyreaderIdStack[i + 1];
			}
		}
		keyreaderStack[GAMECONTROLLER_MAX_KEYREADERS-1] = NULL;
		keyreaderIdStack[GAMECONTROLLER_MAX_KEYREADERS-1] = 0;
	}

	// old keyreader impl.
	//noKeysDownAtLastRead = true;
	//lastNoKeysDownTime = Timer::getTime();
	updateEnabledStatus();
}

char GameController::convertKeycodeToAscii(int keycode, bool shiftDown, bool altDown)
{
	int ascchar = 0;

	if (keycodeName[keycode][0] != '\0')
	{
		if (keycodeName[keycode][1] == '\0')
		{
			if (keycodeName[keycode][0] >= 'a'
				&& keycodeName[keycode][0] <= 'z')
			{
				if (shiftDown)
					ascchar = (keycodeName[keycode][0] - 'a') + 'A';
				else
					ascchar = keycodeName[keycode][0];
			} 
			else if (keycodeName[keycode][0] >= '0'
				&& keycodeName[keycode][0] <= '9')
			{
				if (altDown)
				{
					if (keycodeName[keycode][0] == '2')
						ascchar = '@';
					if (keycodeName[keycode][0] == '4')
						ascchar = '$';
				} else {
					if (shiftDown)
					{
						if (keycodeName[keycode][0] == '1')
							ascchar = '!';
						if (keycodeName[keycode][0] == '2')
							ascchar = '"';
						if (keycodeName[keycode][0] == '3')
							ascchar = '#';
						if (keycodeName[keycode][0] == '5')
							ascchar = '%';
						if (keycodeName[keycode][0] == '6')
							ascchar = '&';
						if (keycodeName[keycode][0] == '7')
							ascchar = '/';
						if (keycodeName[keycode][0] == '8')
							ascchar = '(';
						if (keycodeName[keycode][0] == '9')
							ascchar = ')';
						if (keycodeName[keycode][0] == '0')
							ascchar = '=';
					} else {
						ascchar = keycodeName[keycode][0];
					}
				}
			} 
		} else {
			if (strcmp(keycodeName[keycode], "space") == 0)
			{
				ascchar = ' ';
			}
		}
	}

	// HACK!!!
	// TODO: proper implementation
	if (keycode == 51)
	{
		if (shiftDown)
			ascchar = ';';
		else
			ascchar = ',';
	}
	if (keycode == 52)
	{
		if (shiftDown)
			ascchar = ':';
		else
			ascchar = '.';
	}
	if (keycode == 53)
	{
		if (shiftDown)
			ascchar = '_';
		else
			ascchar = '-';
	}

	return ascchar;
}

void GameController::run()
{
	for (int ctrl = 0; ctrl < DH_CTRL_AMOUNT; ctrl++)
	{
		prevCtrlOn[ctrl] = ctrlOn[ctrl];
	}

	// FIXME: if no keyreader, should clear the keyread buffers
	// (or clear the buffers every time a keyreader is added/removed)


	if (keyreader != NULL)
	{
		int failcount = 0;
		while (readKeyReadPos != readKeyWritePos)
		{
			int ascchar = 0;
			int keycode = 0;
			ascchar = readKeyAsciiBuf[readKeyReadPos];
			keycode = readKeyKeycodeBuf[readKeyReadPos];
			readKeyReadPos = (readKeyReadPos + 1) % GAMECONTROLLER_KEYREADER_BUFFER_SIZE;

			keyreader->readKey(ascchar, keycode, keycodeName[keycode]);

			if (keyreader == NULL) break;

			failcount++;
			if (failcount > GAMECONTROLLER_KEYREADER_BUFFER_SIZE)
			{
				// this should _never_ happen.
				Logger::getInstance()->error("GameController::run - Internal error while iterating keyreader buffer.");
				break;
			}
		}

		// OLD HACK KEYREADER IMPLEMENTATION
		/*
		bool repeatNow = false;
		int now = Timer::getTime();
		if (now > lastNoKeysDownTime + KEYREPEAT_FIRST_DELAY
			&& now > lastKeyReadTime + KEYREPEAT_DELAY)
		{
			repeatNow = true;
		}
		bool noKeysDown = true;
		bool shiftDown = false;
		bool altDown = false;
		if (Keyb3_IsKeyDown(KEYCODE_SHIFT_LEFT) 
			|| Keyb3_IsKeyDown(KEYCODE_SHIFT_RIGHT))
		{
			shiftDown = true;
		}
		if (Keyb3_IsKeyDown(KEYCODE_ALT) 
			|| 'IsKeyDown(KEYCODE_ALT_GR))
		{
			altDown = true;
		}

		for (int i = 0; i < KEYCODE_NAME_AMOUNT; i++)
		{
			int ascchar = 0;
			bool isDown = false;
			if (Keyb3_IsKeyDown(i))
				isDown = true;

			if (isDown)
			{
				ascchar = convertKeycodeToAscii(i, shiftDown, altDown);
			}

			if (isDown 
				&& (ascchar != 0 || i == KEYCODE_BACKSPACE || i == KEYCODE_DELETE 
				|| i == KEYCODE_UP_ARROW || i == KEYCODE_DOWN_ARROW || i == KEYCODE_LEFT_ARROW || i == KEYCODE_RIGHT_ARROW))
			{
				noKeysDown = false;
			}
			if (Keyb3_IsKeyPressed(i) 
				|| (isDown && repeatNow))
			{
				lastKeyReadTime = now;
				keyreader->readKey(ascchar, i, keycodeName[i]);
			}
		}

		noKeysDownAtLastRead = noKeysDown;
		if (noKeysDown)
		{
			lastNoKeysDownTime = now;
		}
		*/
	}
}


void GameController::startJoystickDetection()
{
	for(int joynum = 0; joynum < 4; joynum++)
	{
		JoystickValues* max = joystickMaxiumValues[joynum];
		getJoystickValues( joynum, &max->x, &max->y, &max->rx, &max->ry, &max->throttle, &max->rudder );
	}

	/*
	if( abs( max->x ) > max->x )				max->x = abs( max->x );
	if( abs( max->y ) > max->y )				max->y = abs( max->y );
	if( abs( max->rx ) > max->rx )				max->rx = abs( max->rx );
	if( abs( max->ry ) > max->ry )				max->ry = abs( max->ry );
	if( abs( max->throttle ) > max->throttle )	max->throttle = abs( max->throttle );
	if( abs( max->rudder ) > max->rudder )		max->rudder = abs( max->rudder );
	
	max->x = 0;
	max->y = 0;
	max->rx = 0;
	max->ry = 0;
	max->throttle = 0;
	max->rudder = 0;
	*/
}

GameController::JOYSTICK_AXIS	GameController::getDetectedAxis()
{
	static int last_time = 0;

	if( ( Timer::getTime() - last_time ) > 200 )
	{
		startJoystickDetection();
		last_time = Timer::getTime();
	}
	else
	{
		last_time = Timer::getTime();
	}

	GameController::JOYSTICK_AXIS result = JOYSTICK_AXIS_UNKNOWN;

	int jvalues[] = {0, 0, 0, 0, 0, 0};
	int maxval = 0;
	int imax = 0;

	// HACK: wait until Keyb3 returns something else than 0 from some axis
	while (maxval == 0)
	{
		if (Timer::getTime() - last_time > 4000)
			break;

		for(int joynum = 0; joynum < 4; joynum++)
		{
			JoystickValues* max = joystickMaxiumValues[joynum];

			getJoystickValues( joynum, &jvalues[0], &jvalues[1], &jvalues[2], &jvalues[3], &jvalues[4], &jvalues[5] );

			JoystickValues temp;
			temp.x = jvalues[0];
			temp.y = jvalues[1];
			temp.rx = jvalues[2];
			temp.ry = jvalues[3];
			temp.throttle = jvalues[4];
			temp.rudder = jvalues[5];

			// Find largest absolute value
			for (int i = 0; i < 6; i++)
			{
				if (abs(jvalues[i]) > maxval)
				{
					maxval = abs(jvalues[i]);
					imax = i;
				}
			}

			if( imax == 0 )					
			{ 
				//Logger::getInstance()->warning( ( std::string( "joy-axis-x         " )  + boost::lexical_cast< std::string >( max->x ) + " != " + boost::lexical_cast< std::string >( temp.x ) ).c_str() );
				max->x = temp.x;					
				result = GameController::JOYSTICK_AXIS_X;			
			}
			
			if( imax == 1 )					
			{ 
				//Logger::getInstance()->warning( ( std::string( "joy-axis-y         " )  + boost::lexical_cast< std::string >( max->y ) + " != " + boost::lexical_cast< std::string >( temp.y ) ).c_str() );
				max->y = temp.y;					
				result = GameController::JOYSTICK_AXIS_Y;			
			} 

			if( imax == 2 )				
			{ 
				//Logger::getInstance()->warning( ( std::string( "joy-axis-rx        " )  + boost::lexical_cast< std::string >( max->rx ) + " != " + boost::lexical_cast< std::string >( temp.rx ) ).c_str() );
				max->rx = temp.rx;				
				result = GameController::JOYSTICK_AXIS_RX;			
			}
			
			if( imax == 3 )				
			{ 
				//Logger::getInstance()->warning( ( std::string( "joy-axis-ry        " )  + boost::lexical_cast< std::string >( max->ry ) + " != " + boost::lexical_cast< std::string >( temp.ry ) ).c_str() );
				max->ry = temp.ry;				
				result = GameController::JOYSTICK_AXIS_RY;			
			}
			
			if( imax == 4 )	
			{ 
				//Logger::getInstance()->warning( ( std::string( "joy-axis-throttle  " )  + boost::lexical_cast< std::string >( max->throttle ) + " != " + boost::lexical_cast< std::string >( temp.throttle ) ).c_str() );
				max->throttle = temp.throttle;	
				result = GameController::JOYSTICK_AXIS_THROTTLE;	
			}
			
			if( imax == 5 )		
			{ 
				//Logger::getInstance()->warning( ( std::string( "joy-axis-rudder    " )  + boost::lexical_cast< std::string >( max->rudder ) + " != " + boost::lexical_cast< std::string >( temp.rudder ) ).c_str() );
				max->rudder = temp.rudder;		
				result = GameController::JOYSTICK_AXIS_RUDDER;		
			}
		}

		if (maxval == 0)
			Keyb3_UpdateDevices();

		Timer::update();
	}

	return result;
}

void GameController::getJoystickValues(int joynum, int *x, int *y, int *rx, int *ry, int *throttle, int *rudder)
{
	igiosWarning("GameController::getJoystickValues(1): joynum = %d\n", joynum);
	assert(joynum >= 0 && joynum < 4);
	Keyb3_ReadJoystick(joynum, x, y, rx, ry, throttle, rudder);

	// dead zones
	int deadzoneSize = game::SimpleOptions::getInt(DH_OPT_I_JOYSTICK1_DEADZONE + joynum);

	//LOG_DEBUG(strPrintf("GameController::getJoystickValues(1): joystick %d before deadzone(%d): %d %d %d %d", joynum, deadzoneSize, *x, *y, *rx, *ry).c_str());

	if (abs(*x) < deadzoneSize && abs(*y) < deadzoneSize)
	{
		*x = 0;
		*y = 0;
	}

	if (abs(*rx) < deadzoneSize && abs(*ry) < deadzoneSize)
	{
		*rx = 0;
		*ry = 0;
	}

	if (abs(*throttle) < deadzoneSize)
	{
		*throttle = 0;
	}

	if (abs(*rudder) < deadzoneSize)
	{
		*rudder = 0;
	}

	//LOG_DEBUG(strPrintf("GameController::getJoystickValues(1): joystick %d after deadzone(%d): %d %d %d %d", joynum, deadzoneSize, *x, *y, *rx, *ry).c_str());
}


void GameController::getJoystickValues(int *out_move_x, int *out_move_y, int *out_dir_x, int *out_dir_y)
{
	// use our own joystick
	// FIXME: WhyTF does the explicit-joynum-one even exist?
	assert(getControllerType() >= CONTROLLER_TYPE_JOYSTICK1 && getControllerType() <= CONTROLLER_TYPE_JOYSTICK4);
	int joystick = getControllerType() - CONTROLLER_TYPE_JOYSTICK1;

	getJoystickValues(joystick, out_move_x, out_move_y, out_dir_x, out_dir_y);
}


void GameController::getJoystickValues( int joynum, int *out_move_x, int *out_move_y, int *out_dir_x, int *out_dir_y)
{
	igiosWarning("GameController::getJoystickValues(2): joynum = %d\n", joynum);
	assert( joynum >= 0 && joynum < 4 );
	
	int x = 0, y = 0;
	int rx = 0, ry = 0;
	int throttle = 0;
	int rudder = 0;

	Keyb3_ReadJoystick(joynum, &x, &y, &rx, &ry, &throttle, &rudder);

	// dead zones
	int deadzoneSize = game::SimpleOptions::getInt(DH_OPT_I_JOYSTICK1_DEADZONE + joynum);

	//LOG_DEBUG(strPrintf("GameController::getJoystickValues(2): joystick %d before deadzone(%d): %d %d %d %d", joynum, deadzoneSize, x, y, rx, ry).c_str());

	//LOG_DEBUG(strPrintf("GameController::getJoystickValues(2): joystick %d after deadzone(%d): %d %d %d %d", joynum, deadzoneSize, x, y, rx, ry).c_str());

	// FIXME: copypasta follows. figure out some way to refactor it

	if (out_move_x != NULL)
	{
		int result;
		switch( joystickMoveXAxis )
		{
		case JOYSTICK_AXIS_X:
			result = x;
			break;
		case JOYSTICK_AXIS_Y:
			result = y;
			break;
		case JOYSTICK_AXIS_RX:
			result = rx;
			break;
		case JOYSTICK_AXIS_RY:
			result = ry;
			break;
		case JOYSTICK_AXIS_THROTTLE:
			result = throttle;
			break;
		case JOYSTICK_AXIS_RUDDER:
			result = rudder;
			break;
		default:
			result = 0;
			break;
		
		}

		(*out_move_x) = result;
	}

	if (out_move_y != NULL)
	{
		int result;
		switch( joystickMoveYAxis )
		{
		case JOYSTICK_AXIS_X:
			result = x;
			break;
		case JOYSTICK_AXIS_Y:
			result = y;
			break;
		case JOYSTICK_AXIS_RX:
			result = rx;
			break;
		case JOYSTICK_AXIS_RY:
			result = ry;
			break;
		case JOYSTICK_AXIS_THROTTLE:
			result = throttle;
			break;
		case JOYSTICK_AXIS_RUDDER:
			result = rudder;
			break;
		default:
			result = 0;
			break;
		}

		(*out_move_y) = result;
	}

	if (out_move_x != NULL
		&& out_move_y != NULL
		&& abs(*out_move_y) < deadzoneSize
		&& abs(*out_move_x) < deadzoneSize)
	{
		*out_move_y = 0;
		*out_move_x = 0;
	}

	if (out_dir_x != NULL)
	{
		int result;
		switch( joystickDirXAxis )
		{
		case JOYSTICK_AXIS_X:
			result = x;
			break;
		case JOYSTICK_AXIS_Y:
			result = y;
			break;
		case JOYSTICK_AXIS_RX:
			result = rx;
			break;
		case JOYSTICK_AXIS_RY:
			result = ry;
			break;
		case JOYSTICK_AXIS_THROTTLE:
			result = throttle;
			break;
		case JOYSTICK_AXIS_RUDDER:
			result = rudder;
			break;
		default:
			result = 0;
			break;
		
		}

		(*out_dir_x) = result;
	}

	if (out_dir_y)
	{
		int result;
		switch( joystickDirYAxis )
		{
		case JOYSTICK_AXIS_X:
			result = x;
			break;
		case JOYSTICK_AXIS_Y:
			result = y;
			break;
		case JOYSTICK_AXIS_RX:
			result = rx;
			break;
		case JOYSTICK_AXIS_RY:
			result = ry;
			break;
		case JOYSTICK_AXIS_THROTTLE:
			result = throttle;
			break;
		case JOYSTICK_AXIS_RUDDER:
			result = rudder;
			break;
		default:
			result = 0;
			break;
		}

		(*out_dir_y) = result;
	}

	if (out_dir_x != NULL
		&& out_dir_y != NULL
		&& abs(*out_dir_y) < deadzoneSize
		&& abs(*out_dir_x) < deadzoneSize)
	{
		*out_dir_y = 0;
		*out_dir_x = 0;
	}

}

void GameController::setJoystickMoveXAxis( JOYSTICK_AXIS axis )
{
	joystickMoveXAxis = axis;
}

void GameController::setJoystickMoveYAxis( JOYSTICK_AXIS axis )
{
	joystickMoveYAxis = axis;
}

void GameController::setJoystickDirXAxis( JOYSTICK_AXIS axis )
{
	joystickDirXAxis = axis;
}

void GameController::setJoystickDirYAxis( JOYSTICK_AXIS axis )
{
	joystickDirYAxis = axis;
}

GameController::JOYSTICK_AXIS GameController::getJoystickMoveXAxis() const
{
	return joystickMoveXAxis;
}

GameController::JOYSTICK_AXIS GameController::getJoystickMoveYAxis() const
{
	return joystickMoveYAxis;
}

GameController::JOYSTICK_AXIS GameController::getJoystickDirXAxis() const
{
	return joystickDirXAxis;
}

GameController::JOYSTICK_AXIS GameController::getJoystickDirYAxis() const
{
	return joystickDirYAxis;
}

std::string GameController::getJoystickAxisName( GameController::JOYSTICK_AXIS axis ) const
{
	return convertToString( axis );
}


GameController::CONTROLLER_TYPE GameController::getControllerTypeByKey( int key ) const
{
	if( key >= 0 && key < 270 )			return CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;
	else if ( key >= 270 && key < 290 ) return CONTROLLER_TYPE_JOYSTICK1;
	else if ( key >= 290 && key < 310 ) return CONTROLLER_TYPE_JOYSTICK2;
	else if ( key >= 342 && key < 362 ) return CONTROLLER_TYPE_JOYSTICK3;
	else if ( key >= 362 && key < 382 ) return CONTROLLER_TYPE_JOYSTICK4;
	return CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1;	// Default is keyboard.
}


inline int getFirstControllerButton(GameController::CONTROLLER_TYPE type)
{
	if ( type == GameController::CONTROLLER_TYPE_JOYSTICK1 ) return 270;
	else if ( type == GameController::CONTROLLER_TYPE_JOYSTICK2 ) return 290;
	else if ( type == GameController::CONTROLLER_TYPE_JOYSTICK3 ) return 342;
	else if ( type == GameController::CONTROLLER_TYPE_JOYSTICK4 ) return 362;
	return 0;
}

inline int getMouseNumberByKey(int key)
{
	if( key >= KEYCODE_MOUSE_WHEEL_DOWN && key <= KEYCODE_MOUSE_BUTTON8 ) return 0;
	else if( key >= KEYCODE_MOUSE0_WHEEL_DOWN && key <= KEYCODE_MOUSE0_BUTTON1 ) return 1;
	else if( key >= KEYCODE_MOUSE1_WHEEL_DOWN && key <= KEYCODE_MOUSE1_BUTTON1 ) return 2;
	else if( key >= KEYCODE_MOUSE2_WHEEL_DOWN && key <= KEYCODE_MOUSE2_BUTTON1 ) return 3;
	else if( key >= KEYCODE_MOUSE3_WHEEL_DOWN && key <= KEYCODE_MOUSE3_BUTTON1 ) return 4;
	return -1;
}

inline int getFirstMouseButton(int mouse)
{
	if ( mouse == 0 ) return KEYCODE_MOUSE_BUTTON1;
	else if ( mouse == 1 ) return KEYCODE_MOUSE0_BUTTON8;
	else if ( mouse == 2 ) return KEYCODE_MOUSE1_BUTTON8;
	else if ( mouse == 3 ) return KEYCODE_MOUSE2_BUTTON8;
	else if ( mouse == 4 ) return KEYCODE_MOUSE3_BUTTON8;
	return -1;
}
inline bool convertJoystickKey( int &key, GameController::CONTROLLER_TYPE type_new, GameController::CONTROLLER_TYPE type_old )
{
	// no conversion
	if(type_old < GameController::CONTROLLER_TYPE_JOYSTICK1 || type_old > GameController::CONTROLLER_TYPE_JOYSTICK4)
		return false;

	// no conversion
	if(type_new < GameController::CONTROLLER_TYPE_JOYSTICK1 || type_new > GameController::CONTROLLER_TYPE_JOYSTICK4)
		return false;

	int button_num = key - getFirstControllerButton(type_old);
	key = button_num + getFirstControllerButton(type_new);
	return true;
}

inline bool convertMouseKey( int &key, GameController::CONTROLLER_TYPE type_new, GameController::CONTROLLER_TYPE type_old )
{
	int mouse_old = getMouseNumberByKey(key);

	// no conversion
	if(mouse_old < 0)
		return false;

	// no conversion
	if(type_new < GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 || type_new > GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE4)
		return false;

	int mouse_new = type_new - GameController::CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 + 1;

	// map to mouse 0 (all mouses)
	if(!game::SimpleOptions::getBool( DH_OPT_B_CONTROLLER_MULTIPLE_INPUT_DEVICES_ENABLED ))
	{
		mouse_new = 0;
	}

	int button_num = key - getFirstMouseButton(mouse_old);

	// mouse wheel
	if(button_num == -1 || button_num == -2)
	{
		key = button_num + getFirstMouseButton(mouse_new);
		return true;
	}

	// stupid reverse order
	if(mouse_old != 0)
		button_num = 7 - button_num;

	// stupid reverse order
	if(mouse_new != 0)
		button_num = 7 - button_num;

	key = button_num + getFirstMouseButton(mouse_new);
	return true;
}

int GameController::convertKeyToController( int key, GameController::CONTROLLER_TYPE type_new ) const
{
	// current controller for the key
	CONTROLLER_TYPE type_old = getControllerTypeByKey(key);

	if(convertJoystickKey(key, type_new, type_old))
		return key;

	if(convertMouseKey(key, type_new, type_old))
		return key;

	return key;
}

// returns the type of the controller
GameController::CONTROLLER_TYPE GameController::getControllerType() const
{
	return controllerType;
}

bool GameController::controllerTypeHasMouse() const
{
	return controllerType >= CONTROLLER_TYPE_KEYBOARD_AND_MOUSE1 && controllerType <= CONTROLLER_TYPE_KEYBOARD_AND_MOUSE4;
}

bool GameController::controllerTypeHasJoystick() const
{
	return controllerType >= CONTROLLER_TYPE_JOYSTICK1 && controllerType <= CONTROLLER_TYPE_JOYSTICK4;
}

void GameController::setControllerType( GameController::CONTROLLER_TYPE type )
{
	controllerType = type;
}



void GameController::setControlOn(int controlNum)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::setControlOn - Control number out of bounds.");
		return;
	} 
	ctrlOn[controlNum] = true;
}

void GameController::setControlOff(int controlNum)
{
	if (controlNum < 0 || controlNum >= DH_CTRL_AMOUNT)
	{
		Logger::getInstance()->error("GameController::setControlOff - Control number out of bounds.");
		return;
	} 
	ctrlOn[controlNum] = false;
}

void GameController::addReadKey(char ascii, int keycode)
{
	if (this->keyreader != NULL)
	{
		int nextPos = (readKeyWritePos + 1) % GAMECONTROLLER_KEYREADER_BUFFER_SIZE;
		if (nextPos != readKeyReadPos)
		{

			bool shiftDown = false;
			bool altDown = false;
			if (Keyb3_IsKeyDown(KEYCODE_SHIFT_LEFT) 
				|| Keyb3_IsKeyDown(KEYCODE_SHIFT_RIGHT))
			{
				shiftDown = true;
			}
			if (Keyb3_IsKeyDown(KEYCODE_ALT) 
				|| Keyb3_IsKeyDown(KEYCODE_ALT_GR))
			{
				altDown = true;
			}

			// attempt to solve ascii based on keycode, if no ascii supplied
			if (ascii == 0)
			{
				if (keycode != 0)
					ascii = this->convertKeycodeToAscii(keycode, shiftDown, altDown);
			}
			// attempt to solve keycode based on ascii, if no keycode supplied
			if (keycode == 0)
			{
				if (ascii != 0)
				{
					// TODO: this is very very inefficient - optimize!
					for (int i = 0; i < KEYCODE_NAME_AMOUNT; i++)
					{
						if (convertKeycodeToAscii(i, shiftDown, altDown) == ascii)
						{
							keycode = i;
							break;
						}
					}
					// TODO: uh...  horrible hack...
					if (ascii == 27)
					{
						keycode = KEYCODE_ESC;
					}
					if (ascii == '\r')
					{
						keycode = KEYCODE_ENTER;
					}
					if (ascii == '\b')
					{
						keycode = KEYCODE_BACKSPACE;
					}
					if (ascii == '\t')
					{
						keycode = KEYCODE_TAB;
					}
				}
			}
			// now we should have at least keycode or ascii...
			if (ascii != 0 || keycode != 0)
			{
				readKeyAsciiBuf[readKeyWritePos] = ascii;
				readKeyKeycodeBuf[readKeyWritePos] = keycode;
				readKeyWritePos = nextPos;
			} else {
				Logger::getInstance()->error("GameController::addReadKey - Attempt to add zero ascii and keycode.");
			}
		} else {
			// buffer full. do nothing. (just ignore the read key)
		}
	}
}

// user signalled quit
void GameController::suicide() {
	game::GameOptionManager::cleanInstance();
	game::GameConfigs::cleanInstance();
	exit(0);
}

}

