
#include "precompiled.h"

#include <boost/lexical_cast.hpp>

#include "cursordefs.h"
#include "cursordefs_files.h"
#include "../util/Debug_MemoryManager.h"

#include "../game/DHLocaleManager.h"

#define LOAD_CURSOR_CORNER(p_file, p_id) \
  ogui->LoadCursorImage(controller, p_file, p_id);

#define LOAD_CURSOR_CENTER(p_file, p_id) \
  ogui->LoadCursorImage(controller, p_file, p_id); \
  ogui->SetCursorImageOffset(controller, -16, -16, p_id);


void loadDHCursors(Ogui *ogui, int controller)
{
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_CLAW_PROTO)

  int i = controller;

  // DH_CURSOR_INVISIBLE - invisible = null image (unloaded)

  ogui->LoadCursorImage(i, DH_CURSOR_FILE_ARROW, DH_CURSOR_ARROW);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM, DH_CURSOR_AIM);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_REPAIR, DH_CURSOR_REPAIR);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_BUY, DH_CURSOR_BUY);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_SELL, DH_CURSOR_SELL);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_MOVE_TO, DH_CURSOR_MOVE_TO);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_INVALID_TARGET, DH_CURSOR_INVALID_TARGET);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_FORWARD, DH_CURSOR_FORWARD);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_BACKWARD, DH_CURSOR_BACKWARD);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_LEFT, DH_CURSOR_LEFT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_RIGHT, DH_CURSOR_RIGHT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_ROTATE_LEFT, DH_CURSOR_ROTATE_LEFT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_ROTATE_RIGHT, DH_CURSOR_ROTATE_RIGHT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_ORBIT_LEFT, DH_CURSOR_ORBIT_LEFT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_ORBIT_RIGHT, DH_CURSOR_ORBIT_RIGHT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_UP_LEFT, DH_CURSOR_UP_LEFT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_UP_RIGHT, DH_CURSOR_UP_RIGHT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_DOWN_LEFT, DH_CURSOR_DOWN_LEFT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_DOWN_RIGHT, DH_CURSOR_DOWN_RIGHT);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_HEAVY, DH_CURSOR_AIM_HEAVY);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_ALL, DH_CURSOR_AIM_ALL);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_SNEAK_TO, DH_CURSOR_SNEAK_TO);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_SPRINT_TO, DH_CURSOR_SPRINT_TO);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_SPREAD1, DH_CURSOR_AIM_SPREAD1);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_SPREAD2, DH_CURSOR_AIM_SPREAD2);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_SPREAD3, DH_CURSOR_AIM_SPREAD3);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_SPREAD4, DH_CURSOR_AIM_SPREAD4);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_SPREAD5, DH_CURSOR_AIM_SPREAD5);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_RELOADING, DH_CURSOR_AIM_RELOADING);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_PLAYER2, DH_CURSOR_AIM_PLAYER2);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_PLAYER3, DH_CURSOR_AIM_PLAYER3);
  ogui->LoadCursorImage(i, DH_CURSOR_FILE_AIM_PLAYER4, DH_CURSOR_AIM_PLAYER4);

  // these cursor images are centered (they point to image coords 16,16)
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_MOVE_TO);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_INVALID_TARGET);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_FORWARD);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_BACKWARD);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_LEFT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_RIGHT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_ROTATE_LEFT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_ROTATE_RIGHT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_ORBIT_LEFT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_ORBIT_RIGHT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_UP_LEFT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_UP_RIGHT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_DOWN_LEFT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_DOWN_RIGHT);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_HEAVY);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_ALL);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_SNEAK_TO);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_SPRINT_TO);

  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_SPREAD1);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_SPREAD2);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_SPREAD3);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_SPREAD4);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_SPREAD5);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_RELOADING);

  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_PLAYER2);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_PLAYER3);
  ogui->SetCursorImageOffset(i, -16, -16, DH_CURSOR_AIM_PLAYER4);

#elif defined(PROJECT_SURVIVOR)

	
  int i = controller;

	for(int j = 0; j < DH_CURSOR_AMOUNT; j++)
	{
		std::string locale = "gui_cursor" + boost::lexical_cast<std::string>(j);
		if(!::game::DHLocaleManager::getInstance()->hasString(::game::DHLocaleManager::BANK_GUI, locale.c_str()))
		{
			continue;
		}

		std::string str = game::getLocaleGuiString(locale.c_str());

		std::string::size_type pos = str.find_first_of('@');
		std::string filename = str.substr(0, pos);

		ogui->LoadCursorImage(i, filename.c_str(), j);


		if(pos != std::string::npos)
		{
			int x,y;
			std::string offset = str.substr(pos+1);
			if(sscanf(offset.c_str(), "%i,%i", &x, &y) == 2)
			{
				ogui->SetCursorImageOffset(i, x, y, j);
			}
			else
			{
				Logger::getInstance()->error(("Loading cursor " + boost::lexical_cast<std::string>(j) + " failed: invalid offset").c_str());
			}
		}
	}

#else

  // DH_CURSOR_INVISIBLE - invisible = null image (unloaded)

	LOAD_CURSOR_CORNER(DH_CURSOR_FILE_ARROW, DH_CURSOR_ARROW);

	LOAD_CURSOR_CENTER(DH_CURSOR_FILE_MOVE_TO, DH_CURSOR_MOVE_TO);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_SNEAK_TO, DH_CURSOR_SNEAK_TO);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_SPRINT_TO, DH_CURSOR_SPRINT_TO);

  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER1_AIM, DH_CURSOR_PLAYER1_AIM);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER2_AIM, DH_CURSOR_PLAYER2_AIM);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER3_AIM, DH_CURSOR_PLAYER3_AIM);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER4_AIM, DH_CURSOR_PLAYER4_AIM);

  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER1_RELOADING, DH_CURSOR_PLAYER1_RELOADING);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER2_RELOADING, DH_CURSOR_PLAYER2_RELOADING);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER3_RELOADING, DH_CURSOR_PLAYER3_RELOADING);
  LOAD_CURSOR_CENTER(DH_CURSOR_FILE_PLAYER4_RELOADING, DH_CURSOR_PLAYER4_RELOADING);

#endif

}

void unloadDHCursors(Ogui *ogui, int controller)
{
	/*
  int i = controller;
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_ARROW);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_REPAIR);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_BUY);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_SELL);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_MOVE_TO);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_INVALID_TARGET);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_FORWARD);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_BACKWARD);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_LEFT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_RIGHT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_ROTATE_LEFT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_ROTATE_RIGHT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_ORBIT_LEFT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_ORBIT_RIGHT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_UP_LEFT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_UP_RIGHT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_DOWN_LEFT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_DOWN_RIGHT);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_HEAVY);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_ALL);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_SNEAK_TO);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_SPRINT_TO);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_SPREAD1);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_SPREAD2);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_SPREAD3);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_SPREAD4);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_SPREAD5);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_RELOADING);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_PLAYER2);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_PLAYER3);
  ogui->LoadCursorImage(i, NULL, DH_CURSOR_AIM_PLAYER4);
	*/
	for (int i = 0; i < DH_CURSOR_AMOUNT; i++)
	{
	  ogui->LoadCursorImage(controller, NULL, i);
	}
}

