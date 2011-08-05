
#include "precompiled.h"

#include "JoystickAimer.h"

#include <Storm3D_UI.h>

#include "../ogui/Ogui.h"
#include "../game/Unit.h"
#include "../game/Game.h"
#include "../ui/GameController.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "../game/GameScene.h"
#include "../util/PositionsDirectionCalculator.h"
#include "../util/AngleRotationCalculator.h"

#include "../util/Debug_MemoryManager.h"

namespace ui
{

	JoystickAimer::JoystickAimer(game::Unit *unit, Ogui *ogui, 
		game::Game *game, GameController *gameController,
		int clientNumber)
	{
		assert(unit != NULL);
		this->clientNumber = clientNumber;
		this->game = game;
		this->unit = unit;
		this->ogui = ogui;
		this->gameController = gameController;
		currentAimingAngle = 0;
	}


	void JoystickAimer::aimWithJoystick(int timeElapsed)
	{
		if (unit == NULL)
			return;

		IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
		IStorm3D_Camera *cam = scene->GetCamera();

		VC3 pos = unit->getPosition();
		VC3 result = VC3(0,0,0);
		float rhw = 0;
		float real_z = 0;
		bool infront = cam->GetTransformedToScreen(pos, result, rhw, real_z);
		if (!infront)
		{
			return;
		}
		
		int scrmidx = (int)(result.x * 1024.0f);
		int scrmidy = (int)(result.y * 768.0f);

		int scheme = 0;
		if (clientNumber == 0)
			scheme = game::SimpleOptions::getInt(DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME);
		if (clientNumber == 1)
			scheme = game::SimpleOptions::getInt(DH_OPT_I_2ND_PLAYER_CONTROL_SCHEME);
		if (clientNumber == 2)
			scheme = game::SimpleOptions::getInt(DH_OPT_I_3RD_PLAYER_CONTROL_SCHEME);
		if (clientNumber == 3)
			scheme = game::SimpleOptions::getInt(DH_OPT_I_4TH_PLAYER_CONTROL_SCHEME);
		/*if (scheme < 0) scheme = 0;
		if (scheme >= 4 * 3) scheme = 4 * 3 - 1;*/
	
		int forcedRotation = 0;
		int x = 0, y = 0; // , rx, ry, throttle, rudder;

		// what is this?
		bool legacy_support = scheme == 4 * 3 || scheme == (4 * 3) + 1;

		if (legacy_support || scheme == GameController::CONTROLLER_TYPE_KEYBOARD_ONLY)
		{
			if (scheme == (4 * 3) + 1)
			{
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ORBIT_RIGHT))
				{
					forcedRotation = -1;
					x = -1000;
				}
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ORBIT_LEFT))
				{
					forcedRotation = 1;
					x = 1000;
				}
			} else {
				if (!gameController->isKeyDown(DH_CTRL_CAMERA_UNIT_ALIGN_LOCK_TOGGLE))
				{
					if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_FORWARD))
						y = -1000;
					if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_BACKWARD))
						y = 1000;
					if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT))
						x = -1000;
					if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT))
						x = 1000;
				}
			}
		} else {
			// TODO: use direction axes for something!

			gameController->getJoystickValues( scheme - GameController::CONTROLLER_TYPE_JOYSTICK1, NULL, NULL, &x, &y);
		}
		VC2 vec = VC2( (float)x, (float)y );

		/*if ((scheme % 3) == 0)
			vec = VC2((float)x, (float)y);
		else if ((scheme % 3) == 1)
			vec = VC2((float)rx, (float)ry);
		else if ((scheme % 3) == 2)
			vec = VC2( (float)rudder, (float)throttle );
		*/
		if (vec.GetLength() > 5)
		{
			float timeFactor = 0.05f * (float)timeElapsed;
			if (timeFactor < 0.1f) timeFactor = 0.1f;
			if (timeFactor > 5.0f) timeFactor = 5.0f;

			vec.Normalize();
			float joyangle = util::PositionDirectionCalculator::calculateDirection(VC3(0,0,0), VC3(vec.x, 0, vec.y));
			float newangle = joyangle;

			while (newangle >= 360.0f) newangle -= 360.0f;
			while (newangle < 0.0f) newangle += 360.0f;

			float newanglerad = UNIT_ANGLE_TO_RAD(newangle);
			vec = VC2(-(float)sinf(newanglerad), -(float)cosf(newanglerad));

			currentAimingAngle = newangle;
		} else {
			float newanglerad = UNIT_ANGLE_TO_RAD(currentAimingAngle);
			vec = VC2(-(float)sinf(newanglerad), -(float)cosf(newanglerad));
		}

		int scrx = scrmidx + (int)(vec.x * (1024/4));
		int scry = scrmidy + (int)(vec.y * (768/4));
		ogui->setCursorScreenX(clientNumber, scrx);
		ogui->setCursorScreenY(clientNumber, scry);
	}

}
