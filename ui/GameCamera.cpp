
#include "precompiled.h"

#include "GameCamera.h"

#include <cmath>

#ifndef M_PI
#define M_PI PI
#endif

#include <assert.h>
#include <Storm3D_UI.h>
#include <istorm3D_terrain_renderer.h>

#include "IGameCameraBoundary.h"
#include "../game/WaterManager.h"
#include "GameController.h"
#include "CameraAutotilter.h"
#include "../system/Logger.h"
#include "../system/SystemRandom.h"
#include "../game/GameMap.h"
#include "../util/AngleRotationCalculator.h"
#include "../util/Debug_MemoryManager.h"

#include "NoCameraBoundary.h"

#include "../game/SimpleOptions.h"
#include "../game/options/options_camera.h"
#include "../game/options/options_game.h"
#include "../game/options/options_graphics.h"
#include "../game/options/options_players.h"
#include "../game/Ani.h"

#include "igios.h"

#define GAMECAMERA_MIN_FOV 1
#define GAMECAMERA_MAX_FOV 359
#define GAMECAMERA_DEFAULT_FOV 120

//#define CAM_DEFAULT_MIN_ZOOM 4
#define CAM_DEFAULT_MIN_ZOOM 1

#define CAM_DEFAULT_MIDDLE_ZOOM 40
#define CAM_DEFAULT_MAX_ZOOM 80
#define CAM_DEFAULT_TARG_DIST 80

// half a second to destination (20ms * 25)
#define CAM_TIME_TO_DESTINATION 25

#define CAMERA_MOVES_AMOUNT 18
#define CAMERA_MODES_AMOUNT 5

// keep camera 0.7 meters above water.
#define CAMERA_ABOVE_WATER 0.7f


#define CAMERA_SHAKE_GLOBAL_FACTOR 0.01f
#define DEFAULT_CAMERA_SHAKE_TIME 2500
// TODO: range should be a parameter
#define CAMERA_SHAKE_RANGE 15.0f


using namespace game;


namespace ui
{

  const char *cameraMoves[CAMERA_MOVES_AMOUNT] =
  {
    "invalid",
    "forward",
    "backward",
    "left",
    "right",
    "rotate_left",
    "rotate_right",
    "orbit_left",
    "orbit_right",
    "zoom_in",
    "zoom_out",
    "up",
    "down",
    "rotate_up",
    "rotate_down",
    "zoom_next",
    "fov_in",
    "fov_out"
  };

  const char *cameraModes[CAMERA_MODES_AMOUNT] =
  {
    "invalid",
    "zoom_centric",
    "camera_centric",
    "target_centric",
    "flying"
  };


	// private data class for GameCamera
	class GameCameraImplData
	{
		private:
			VC3 cameraPosForScene;
			VC3 cameraTargForScene;
			static float lastFOV;
			int camerasAdded;
			CameraAutotilter *autotilter;
			VC3 upVector;

			int shakeTime;
			int shakeAmount;
			int totalShakeTime;

			GameCameraImplData(game::GameMap *gameMap)
			{
				cameraPosForScene = VC3(0,0,0);
				cameraTargForScene = VC3(0,0,0);
				upVector = VC3(0,1,0);
				camerasAdded = 0;
				autotilter = new CameraAutotilter(gameMap);

				shakeTime = 0;
				shakeAmount = 0;
				totalShakeTime = DEFAULT_CAMERA_SHAKE_TIME;

				// reset static lastfov variable (to make sure fov get's 
				// set after game restarts and stuff?)
				lastFOV = -1;
			}

			~GameCameraImplData()
			{
				if (autotilter != NULL)
					delete autotilter;
			}

			void setCameraForScene(const VC3 &pos, const VC3 &targ)
			{
				this->cameraPosForScene = pos;
				this->cameraTargForScene = targ;
				this->camerasAdded = 1;
			}

			void addCameraForScene(const VC3 &pos, const VC3 &targ)
			{
				this->cameraPosForScene += pos;
				this->cameraTargForScene += targ;
				this->camerasAdded++;
			}

		friend class GameCamera;
	};

	float GameCameraImplData::lastFOV = -1;


  GameCamera::GameCamera(IStorm3D_Scene *scene, game::GameMap *gameMap,
    GameController **gameControllers)
  {
    this->scene = scene;
    this->gameMap = gameMap;
    this->gameControllers = gameControllers;

    this->waterManager = NULL;

    alphaAngle = 0;
		minBetaAngle = (float)game::SimpleOptions::getInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE);
		maxBetaAngle = (float)game::SimpleOptions::getInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE);
#ifdef PROJECT_SHADOWGROUNDS
		betaAngle = (minBetaAngle + maxBetaAngle) / 2.0f;
#else
		betaAngle = (float)game::SimpleOptions::getInt(DH_OPT_I_CAMERA_DEFAULT_BETA_ANGLE);
#endif

    cameraMode = CAMERA_MODE_ZOOM_CENTRIC;
    //invertLook = false;

    cameraVelocity = 0;
    cameraX = -CAM_DEFAULT_TARG_DIST;
    cameraY = 0;
    cameraHeight = 80;

		interpolatedPosition = VC3(0,0,0);
		interpolatedPositionOffset = VC3(0,0,0);
		interpolatedTargetOffset = VC3(0,0,0);

    targetX = 0;
    targetY = 0;
    targetHeight = 0;

	 forceMapView = false;

    toDestination = false;
    destinationX = 0;
    destinationY = 0;
    sourceX = 0;
    sourceY = 0;

    zoom = CAM_DEFAULT_MIDDLE_ZOOM;
    zooming_to = CAM_DEFAULT_MIDDLE_ZOOM;

		fov = GAMECAMERA_DEFAULT_FOV;

    move_rotleft = false;
    move_rotright = false;
    move_rotup = false;
    move_rotdown = false;
    move_orbitleft = false;
    move_orbitright = false;
    move_goup = false;
    move_godown = false;
    move_fwd = false;
    move_back = false;
    move_left = false;
    move_right = false;
    move_zoom_in = false;
    move_zoom_out = false;
    move_fov_in = false;
    move_fov_out = false;

    key_move_rotleft = false;
    key_move_rotright = false;
    key_move_rotup = false;
    key_move_rotdown = false;
    key_move_orbitleft = false;
    key_move_orbitright = false;
    key_move_goup = false;
    key_move_godown = false;
    key_move_fwd = false;
    key_move_back = false;
    key_move_left = false;
    key_move_right = false;
    key_move_zoom_in = false;
    key_move_zoom_out = false;
    key_move_fov_in = false;
    key_move_fov_out = false;

    followingUnit = false;
    followBehind = false;

    disabledUserMovement = false;

    minZoom = CAM_DEFAULT_MIN_ZOOM;
    maxZoom = CAM_DEFAULT_MAX_ZOOM;
    middleZoom = CAM_DEFAULT_MIDDLE_ZOOM;
    targDist = CAM_DEFAULT_TARG_DIST;

    interpolateCamera = NULL;
    interpolateTime = 0;
    interpolateDuration = 0;

    firstPersonMode = false;
    thirdPerson = false;

		boundaries = NULL;

		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			this->scrollPosition[i] = VC3(0,0,0);
		}

		implData = new GameCameraImplData(gameMap);

		smoothInterpolation = true;

		positionOffset = VC3(0,0,0);
		targetOffset = VC3(0,0,0);
		positionOffset.x = SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_POSITION_OFFSET_X);
		positionOffset.z = SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_POSITION_OFFSET_Z);
		targetOffset.x = SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_TARGET_OFFSET_X);
		targetOffset.z = SimpleOptions::getFloat(DH_OPT_F_CAMERA_DEFAULT_TARGET_OFFSET_Z);

		directControlsEnabled = true;
  }



  GameCamera::~GameCamera()
  {
    delete implData;
  }


  GameCamera::CAMERA_MOVE GameCamera::getCameraMoveByName(char *name)
  {
    assert(name != NULL);
    for (int i = 0; i < CAMERA_MOVES_AMOUNT; i++)
    {
      if (strcmp(name, cameraMoves[i]) == 0)
        return (GameCamera::CAMERA_MOVE)i;
    }
    return CAMERA_MOVE_INVALID;
  }


  GameCamera::CAMERA_MODE GameCamera::getCameraModeByName(char *name)
  {
    assert(name != NULL);
    for (int i = 0; i < CAMERA_MODES_AMOUNT; i++)
    {
      if (strcmp(name, cameraModes[i]) == 0)
        return (GameCamera::CAMERA_MODE)i;
    }
    return CAMERA_MODE_INVALID;
  }



  void GameCamera::setWaterManager(game::WaterManager *waterManager)
	{	
		this->waterManager = waterManager;
	}


  void GameCamera::setMode(CAMERA_MODE cameraMode)
  {
    this->cameraMode = cameraMode;
  }


	GameCamera::CAMERA_MODE GameCamera::getMode()
  {
    return this->cameraMode;
  }


  void GameCamera::setBoundaries(IGameCameraBoundary *boundaries)
	{
		this->boundaries = boundaries;
	}


  float GameCamera::getFOV()
  {
		return fov;
	}


  void GameCamera::setFOV(float fov)
  {
		this->fov = fov;
		if (this->fov < GAMECAMERA_MIN_FOV) this->fov = GAMECAMERA_MIN_FOV;
		if (this->fov > GAMECAMERA_MAX_FOV) this->fov = GAMECAMERA_MAX_FOV;
	}


  void GameCamera::setFOVFactor(float factor)
  {
		fovFactor = factor;
	}


  float GameCamera::getAngleY()
  {
    float ret = 360 - alphaAngle;
		if (ret >= 360.0f)
			ret -= 360.0f;
    //if (ret < 0) ret += 360;
    return ret;
  }


  void GameCamera::setScrollPosition(const VC3 &position, int clientNumber)
  {
		assert(clientNumber >= 0 && clientNumber < MAX_PLAYERS_PER_CLIENT);
		this->scrollPosition[clientNumber] = position;
  }


  VC3 GameCamera::getScrollPosition(int clientNumber)
  {
		assert(clientNumber >= 0 && clientNumber < MAX_PLAYERS_PER_CLIENT);
		return this->scrollPosition[clientNumber];
  }


  void GameCamera::setAngleY(float angle)
  {
    alphaAngle = 360 - angle;
		if (alphaAngle >= 360.0f)
			alphaAngle -= 360.0f;

		if (SimpleOptions::getBool(DH_OPT_B_CAMERA_ALPHA_ANGLE_LOCKED))
		{
			alphaAngle = SimpleOptions::getFloat(DH_OPT_F_CAMERA_ALPHA_ANGLE_LOCKED_VALUE);
		}
  }


  void GameCamera::setMinBetaAngle(float angle)
  {
    minBetaAngle = angle;
  }


  void GameCamera::setMaxBetaAngle(float angle)
  {
    maxBetaAngle = angle;
  }


  void GameCamera::setBetaAngle(float angle)
  {
    betaAngle = angle;
  }


  float GameCamera::getBetaAngle()
  {
    return betaAngle;
  }


  VC3 GameCamera::getPosition()
  {
    VC3 ret = VC3(cameraX, cameraHeight, cameraY);
    return ret;
  }


  VC3 GameCamera::getActualInterpolatedPosition()
  {
#ifdef PROJECT_SHADOWGROUNDS   
		return interpolatedPosition;
#else
		return interpolatedPosition + interpolatedPositionOffset;
#endif
  }


  void GameCamera::setPosition(const VC3 &position)
  {
    cameraX = position.x;
    cameraHeight = position.y;
    cameraY = position.z;
  }

  void GameCamera::rotateAroundPosition(VC3 &position, int angle)
  {
	 float radAngle = -angle*3.141596f/180.0f;
     VC3 newPos = VC3(cameraX, cameraHeight, cameraY) - position;
     VC3 rotatedPos = VC3(cosf(radAngle)*newPos.x-sinf(radAngle)*newPos.z, cameraHeight,
							  sinf(radAngle)*newPos.x+cosf(radAngle)*newPos.z);
	 newPos = rotatedPos + position;
	 cameraX = newPos.x;
	 cameraY = newPos.z;
	 alphaAngle += angle;
  }
	/*
  void GameCamera::setInvertLook(bool inverted)
  {
    invertLook = inverted;
  }
	*/


  void GameCamera::setDisableUserMovement(bool disable)
  {
    this->disabledUserMovement = disable;
  }


  bool GameCamera::isUserMovementDisabled()
  {
    return disabledUserMovement;
  }


  void GameCamera::setFollowingUnit(bool following)
  {
    this->followingUnit = following;
  }


  void GameCamera::setFirstPersonPosition(float personX, float personY, float scrolledX, float scrolledY, float height, float alphaAngle, float betaAngle)
  {

    cameraX = scrolledX;
    cameraY = scrolledY;
    cameraHeight = height;

    VC3 pos = VC3(cameraX, cameraHeight, cameraY);
    VC3 targ;
    if (thirdPerson)
    {
      // third person view
			if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
		    targ = VC3(zoom,0,0);
			} else {
	      targ = VC3(8,0,0);
			}

      float rotSpeed = 0;
			if (alphaAngle < 0) alphaAngle += 360;
			if (alphaAngle >= 360) alphaAngle -= 360;
      if (this->alphaAngle < 0) this->alphaAngle += 360;
      if (this->alphaAngle >= 360) this->alphaAngle -= 360;
			rotSpeed += 2 * util::AngleRotationCalculator::getRotationForAngles(this->alphaAngle, alphaAngle, 2);
			rotSpeed += 2 * util::AngleRotationCalculator::getRotationForAngles(this->alphaAngle, alphaAngle, 25);

			VC3 tilt = VC3(0,0,0);
			if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
				if (game::SimpleOptions::getInt(DH_OPT_I_CAMERA_AUTOTILT_AMOUNT) > 0)
				{
					VC3 tiltPos = VC3(personX, cameraHeight, personY);
					tilt = implData->autotilter->getTilt(tiltPos, this->alphaAngle);
					int tiltInt = game::SimpleOptions::getInt(DH_OPT_I_CAMERA_AUTOTILT_AMOUNT);
					if (tiltInt < 0) tiltInt = 0;
					if (tiltInt > 100) tiltInt = 100;
					tilt *= 0.02f * float(tiltInt);

					int tiltIntA = game::SimpleOptions::getInt(DH_OPT_I_CAMERA_AUTOTILT_ALPHA);
					if (tiltIntA < 0) tiltIntA = 0;
					if (tiltIntA > 100) tiltIntA = 100;
					tilt.y *= 0.01f * float(tiltIntA);

					int tiltIntB = game::SimpleOptions::getInt(DH_OPT_I_CAMERA_AUTOTILT_BETA);
					if (tiltIntB < 0) tiltIntB = 0;
					if (tiltIntB > 100) tiltIntB = 100;
					tilt.x *= 0.01f * float(tiltIntB);
				}
			}

      this->alphaAngle += rotSpeed;
      if (this->alphaAngle < 0) this->alphaAngle += 360;
      if (this->alphaAngle >= 360) this->alphaAngle -= 360;

			// TEMP
			//if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			if (false)
			{
	      this->betaAngle = maxBetaAngle;
			} else {
				//this->betaAngle = 15;
				if (betaAngle > 180)
					this->betaAngle -= (360 - betaAngle); // / 10.0f;
				else
					this->betaAngle += betaAngle; // / 10.0f;
			}

			float tiltedAlphaAngle = this->alphaAngle - tilt.y;
			float tiltedBetaAngle = this->betaAngle - tilt.x;
      if (tiltedAlphaAngle < 0) tiltedAlphaAngle += 360;
      if (tiltedAlphaAngle >= 360) tiltedAlphaAngle -= 360;
			// TODO: should check that the angle is within boundaries.
			// (0 - 360 or -180 - 180... dunno which one ;)

			if (SimpleOptions::getBool(DH_OPT_B_CAMERA_ALPHA_ANGLE_LOCKED))
			{
				this->alphaAngle = SimpleOptions::getFloat(DH_OPT_F_CAMERA_ALPHA_ANGLE_LOCKED_VALUE);
				tiltedAlphaAngle = this->alphaAngle;
			}

      QUAT rot = QUAT();
      //rot.MakeFromAngles(0, this->alphaAngle*3.14f/180.0f, this->betaAngle*3.14f/180.0f);
      rot.MakeFromAngles(0, tiltedAlphaAngle*3.14f/180.0f, tiltedBetaAngle*3.14f/180.0f);
      targ = rot.GetRotated(targ);
//#ifdef CRIMSON_MODE
      //VC3 tmp2 = targ / 3;
			//tmp2.y = 0;
//#endif
      targ = pos - targ;
      VC3 tmp = pos;
      pos = targ;
      targ = tmp;
//#ifdef CRIMSON_MODE
//			pos += tmp2;
//			targ += tmp2;
//#endif
      float minHeight = 0;
      if (gameMap->isInScaledBoundaries(pos.x, pos.z))
      {
        minHeight = gameMap->getScaledHeightAt(pos.x, pos.z) + 1;
				// should we keep this above water (third person view)?
      }
      if (pos.y < minHeight) pos.y = minHeight;
    } else {
      // first person view
      targ = VC3(20,0,0);
      this->alphaAngle = alphaAngle;
      this->betaAngle = betaAngle;
      QUAT rot = QUAT();
      rot.MakeFromAngles(0, this->alphaAngle*3.14f/180.0f, this->betaAngle*3.14f/180.0f);
      targ = rot.GetRotated(targ);
      targ = pos + targ;
    }

    targetX = targ.x;
    targetY = targ.z;
    targetHeight = targ.y;

		// NEW: fixed(?)
    cameraX = pos.x;
    cameraY = pos.z;
    cameraHeight = pos.y;

		implData->addCameraForScene(pos, targ);
  }


  void GameCamera::setFirstPersonMode(bool firstPersonMode)
  {
    this->firstPersonMode = firstPersonMode;
  }


  bool GameCamera::isFirstPersonMode()
  {
    return firstPersonMode;
  }


  void GameCamera::setFollowPosition(float x, float y)
  {
    followX = x;
    followY = y;
  }


  bool GameCamera::isFollowingUnit()
  {
    return followingUnit;
  }


  void GameCamera::setFollowingUnitBehind(bool followBehind)
  {
    this->followBehind = followBehind;
  }


  void GameCamera::interpolateFrom(GameCamera *camera, float seconds)
  {
		if (seconds >= 0.0001f)
		{
			interpolateCamera = camera;
			interpolateTime = 0;
			interpolateDuration = seconds * 50;  // (20ms * 50 = 1 second)
		} else {
			interpolateCamera = NULL;
			interpolateTime = 0;
			interpolateDuration = 0;
		}
  }


  bool GameCamera::isFollowingUnitBehind()
  {
    return followBehind;
  }


  void GameCamera::setMovement(CAMERA_MOVE moveType, bool moving)
  {
    switch(moveType)
    {
    case CAMERA_MOVE_FORWARD:
      move_fwd = moving;
      break;
    case CAMERA_MOVE_BACKWARD:
      move_back = moving;
      break;
    case CAMERA_MOVE_LEFT:
      move_left = moving;
      break;
    case CAMERA_MOVE_RIGHT:
      move_right = moving;
      break;
    case CAMERA_MOVE_UP:
      move_goup = moving;
      break;
    case CAMERA_MOVE_DOWN:
      move_godown = moving;
      break;
    case CAMERA_MOVE_ROTATE_LEFT:
      move_rotleft = moving;
      break;
    case CAMERA_MOVE_ROTATE_RIGHT:
      move_rotright = moving;
      break;
    case CAMERA_MOVE_ORBIT_LEFT:
      move_orbitleft = moving;
      break;
    case CAMERA_MOVE_ORBIT_RIGHT:
      move_orbitright = moving;
      break;
    case CAMERA_MOVE_ROTATE_UP:
      move_rotup = moving;
      break;
    case CAMERA_MOVE_ROTATE_DOWN:
      move_rotdown = moving;
      break;
    case CAMERA_MOVE_ZOOM_IN:
      move_zoom_in = moving;
      break;
    case CAMERA_MOVE_ZOOM_OUT:
      move_zoom_out = moving;
      break;
    case CAMERA_MOVE_FOV_IN:
      move_fov_in = moving;
      break;
    case CAMERA_MOVE_FOV_OUT:
      move_fov_out = moving;
      break;
    default:
      Logger::getInstance()->error("GameCamera::setMovement - Unknown movement type.");
      assert(0);
    };
  }

  /*
  void GameCamera::followUnit(game::Unit *unit)
  {
    followingUnit = unit;
  }
  */


  void GameCamera::setDestination(float x, float y)
  {
    toDestination = true;
    destinationX = x + (cameraX - targetX);
    destinationY = y + (cameraY - targetY);
    destinationTime = 0;
    sourceX = cameraX;
    sourceY = cameraY;
  }


  void GameCamera::setPositionNear(float x, float y)
  {
		// make sure the target-position difference is correct...
    // (that is, it is based on current angles, zoom, etc.)
    doMovement(10);

    float offsetX = x - targetX;
    float offsetY = y - targetY;
    targetX += offsetX;
    targetY += offsetY;
    cameraX += offsetX;
    cameraY += offsetY;
    cameraVelocity = 0;

    // make sure we are not underground or anything...
    // this call should fix all that kinda things.
    doMovement(10);
  }


  bool GameCamera::isThirdPersonView()
  {
		return thirdPerson;
  }


  void GameCamera::setThirdPersonView(bool thirdPersonView)
  {
    this->thirdPerson = thirdPersonView;
  }


	// TODO: time/range params too?
  void GameCamera::setShakeEffect(int amount, int totalTime, const VC3 &position)
  {
		VC2 pos2d(position.x, position.z);
		VC2 cam2d(cameraX, cameraY);

		VC2 diff = pos2d - cam2d;
		float dist = diff.GetLength();

		if (dist >= CAMERA_SHAKE_RANGE)
			return;
		
		float fact = (1.0f - dist / CAMERA_SHAKE_RANGE);

		implData->shakeAmount = int((float)amount * fact);
		implData->shakeTime = 0;
		implData->totalShakeTime = totalTime;
  }


  void GameCamera::doMovement(float delta)
  {
		if (implData->shakeAmount > 0)
		{
			int deltaInt = (int)delta;
			if (deltaInt <= 0)
				deltaInt = 1;
			implData->shakeTime += deltaInt;
			if (implData->shakeTime >= implData->totalShakeTime)
			{
				implData->shakeAmount = 0;
			}
		}

		int numPlayers;
		for(numPlayers = 0; numPlayers < MAX_PLAYERS_PER_CLIENT; numPlayers++)
		{
			if(!SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED + numPlayers))
			{
				break;
			}
		}

		// NOTE: this intentionally disables only the movements that may "conflict" with player unit controls
		// (as unit is controlled by some of these same camera controls)
		// the others, such as zoom in/out are left untouched - as those are considered development binds only.
		if (directControlsEnabled)
		{
			for(int c = 0; c < numPlayers; c++)
			{
				GameController *gameController = gameControllers[c];

				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_FORWARD))
					key_move_fwd = true; else key_move_fwd = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_BACKWARD))
					key_move_back = true; else key_move_back = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_LEFT))
					key_move_left = true; else key_move_left = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_RIGHT))
					key_move_right = true; else key_move_right = false;

				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ROTATE_LEFT)) 
					key_move_rotleft = true; else key_move_rotleft = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ROTATE_RIGHT)) 
					key_move_rotright = true; else key_move_rotright = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ROTATE_UP)) 
					key_move_rotup = true; else key_move_rotup = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ROTATE_DOWN)) 
					key_move_rotdown = true; else key_move_rotdown = false;

				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ORBIT_LEFT)) 
					key_move_orbitleft = true; else key_move_orbitleft = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ORBIT_RIGHT)) 
					key_move_orbitright = true; else key_move_orbitright = false;

				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_UP)) 
					key_move_goup = true; else key_move_goup = false;
				if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_DOWN)) 
					key_move_godown = true; else key_move_godown = false;
			}
		}

		for(int c = 0; c < numPlayers; c++)
		{
			GameController *gameController = gameControllers[c];

			if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_FOV_IN)) 
				key_move_fov_in = true; else key_move_fov_in = false;
			if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_FOV_OUT)) 
				key_move_fov_out = true; else key_move_fov_out = false;

			if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ZOOM_IN)) 
				key_move_zoom_in = true; else key_move_zoom_in = false;
			if (gameController->isKeyDown(DH_CTRL_CAMERA_MOVE_ZOOM_OUT)) 
				key_move_zoom_out = true; else key_move_zoom_out = false;

			// next zoom of 3 choices...
			if (gameController->wasKeyClicked(DH_CTRL_CAMERA_MOVE_ZOOM_NEXT)) 
			{
				float fromzoom;
				if (zooming_to < 0)
				{
					fromzoom = zoom;
				} else {
					fromzoom = zooming_to;
				}
				if (fromzoom < (minZoom + middleZoom) / 2)
				{
					zooming_to = maxZoom;
				} else {
					if (fromzoom < (middleZoom + maxZoom) / 2)
					{
						zooming_to = minZoom;
					} else {
						zooming_to = middleZoom;
					}
				}
			}
		}

    // if we're zooming to one of the 3 choices...
		/*
    if (zooming_to >= 0)
    {
      if (zoom < zooming_to) 
      {
        zoom += (2.0f * delta) / 20.0f;
        if (zoom > zooming_to)
        {
          zoom = zooming_to;
          zooming_to = -1;
        }
      } else {
        zoom -= (2.0f * delta) / 20.0f;
        if (zoom < zooming_to)
        {
          zoom = zooming_to;
          zooming_to = -1;
        }
      }
    }
		*/
		// used for inside/outside building zooming...
    if (zooming_to >= 0)
    {
      if (zoom < zooming_to) 
      {
        zoom += (2.0f * delta) / 2000.0f;
        if (zoom > zooming_to)
        {
          zoom = zooming_to;
          zooming_to = -1;
        }
      } else {
        zoom -= (2.0f * delta) / 2000.0f;
        if (zoom < zooming_to)
        {
          zoom = zooming_to;
          zooming_to = -1;
        }
      }
    }

    // movement scaled to 20 ms

    if (move_rotleft || key_move_rotleft)
    {
      alphaAngle -= (2.0f * delta) / 20.0f;
      if (alphaAngle < 0) alphaAngle += 360;
    }
    if (move_rotright || key_move_rotright)
    {
      alphaAngle += (2.0f * delta) / 20.0f;
      if (alphaAngle > 360) alphaAngle -= 360;
    }
    if (move_rotup || key_move_rotup)
    {
      if (betaAngle > minBetaAngle) betaAngle -= (1.0f * delta) / 20.0f;
    }
    if (move_rotdown || key_move_rotdown)
    {
      if (betaAngle < maxBetaAngle) betaAngle += (1.0f * delta) / 20.0f;
    }
    if (move_goup || key_move_goup)
    {
      if (cameraHeight < 80.0) cameraHeight += (0.2f * delta) / 20.0f;
    }
    if (move_godown || key_move_godown)
    {
      if (cameraHeight > 0.0) cameraHeight -= (0.2f * delta) / 20.0f;
    }
    if (!disabledUserMovement)
    {
      if (move_fwd || key_move_fwd)
      {
        if (cameraVelocity < 0.6)
        {
          cameraVelocity += (0.04f * delta) / 20.0f;
        }
      }
      if (move_back || key_move_back)
      {
        if (cameraVelocity > -0.6)
        {
          cameraVelocity -= (0.04f * delta) / 20.0f;
        }
      }
    }
    if (move_zoom_out || key_move_zoom_out)
    {
      float factor = 1;
      if (maxZoom != minZoom)
        factor += 2 * (zoom - minZoom) / (maxZoom - minZoom);
      if (zoom < maxZoom) 
			{
				zoom += (1.0f * factor * delta) / 20.0f;
				if (zoom > maxZoom) 
				{
					zoom = maxZoom;
				}
			}
    }
    if (move_zoom_in || key_move_zoom_in)
    {
      float factor = 1;
      if (maxZoom != minZoom)
        factor += 2 * (zoom - minZoom) / (maxZoom - minZoom);
      if (zoom > minZoom) 
			{
				zoom -= (1.0f * factor * delta) / 20.0f;
				if (zoom < minZoom) 
				{
					zoom = minZoom;
				}
			}
    }
    if (move_fov_in || key_move_fov_in)
    {
      if (fov > GAMECAMERA_MIN_FOV) fov -= (1.0f * delta) / 20.0f;
			setFOV(fov);
    }
    if (move_fov_out || key_move_fov_out)
    {
      if (fov < GAMECAMERA_MAX_FOV) fov += (1.0f * delta) / 20.0f;
			setFOV(fov);
    }

    // slow down camera velocity
    if (!move_fwd && !move_back && !key_move_fwd && !key_move_back)
    {
      if (cameraVelocity > 0) 
      {
        cameraVelocity -= (0.04f * delta) / 20.0f;
        if (cameraVelocity < 0) cameraVelocity = 0;
      }
      if (cameraVelocity < 0) cameraVelocity += (0.04f * delta) / 20.0f;
    }

		for(int c = 0; c < numPlayers; c++)
		{
			GameController *gameController = gameControllers[c];

			if (gameController->wasKeyClicked(DH_CTRL_NEXT_CAMERA_ANGLE_BOUNDARY)) 
			{
				if (minBetaAngle == -85 && maxBetaAngle == 85)
				{
					minBetaAngle = (float)game::SimpleOptions::getInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE);
					maxBetaAngle = (float)game::SimpleOptions::getInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE);
					if (maxBetaAngle < minBetaAngle)
						maxBetaAngle = minBetaAngle;
				} else {
					minBetaAngle = -85;
					maxBetaAngle = 85;
				}
				if (betaAngle < minBetaAngle)
					betaAngle = minBetaAngle;
				if (betaAngle > maxBetaAngle)
					betaAngle = maxBetaAngle;
			}
		}

// TEMP
    //if (firstPersonMode)
    //{
    //  return;
    //}

		for(int c = 0; c < numPlayers; c++)
		{
			GameController *gameController = gameControllers[c];

			if (gameController->wasKeyClicked(DH_CTRL_NEXT_CAMERA_MODE)) 
			{
				if (cameraMode == CAMERA_MODE_FLYING)
				{
					cameraMode = CAMERA_MODE_ZOOM_CENTRIC;
				} else {
					if (cameraMode == CAMERA_MODE_TARGET_CENTRIC)
						cameraMode = CAMERA_MODE_FLYING;
					if (cameraMode == CAMERA_MODE_CAMERA_CENTRIC)
						cameraMode = CAMERA_MODE_TARGET_CENTRIC;
					if (cameraMode == CAMERA_MODE_ZOOM_CENTRIC)
						cameraMode = CAMERA_MODE_CAMERA_CENTRIC;
				}
			}

			int mdx = 0;
			int mdy = 0;

			if (gameController->controllerTypeHasJoystick())
			{
				if (gameController->isKeyDown(DH_CTRL_CAMERA_LOOK_MODE))
				{
					// joystick code
					gameController->getJoystickValues(c, NULL, NULL, &mdx, &mdy);

					if (mdx != 0 || mdy != 0)
					{
						VC2 dir(mdx, mdy);
						dir.Normalize();
						if (mdx > 0)
						{
							//  0 < angle < 180
							alphaAngle = ((M_PI * 0.5) + asinf(dir.y)) * 180.0 / M_PI;
						} else {
							// 180 < angle < 360
							alphaAngle = (2*M_PI + -(M_PI * 0.5 + asinf(dir.y))) * 180.0 / M_PI;
						}
					}
				}
			} else {
				// mouse code

				gameController->getMouseDelta(&mdx, &mdy);

				if (game::SimpleOptions::getBool(DH_OPT_B_CAMERA_INVERT_LOOK))
					mdy = -mdy;

				if (gameController->isKeyDown(DH_CTRL_CAMERA_POSITION_OFFSET_MODE))
				{
					positionOffset.x += (0.5f * mdx) / 20.0f;
					positionOffset.z -= (0.5f * mdy) / 20.0f;
					// TODO: clamp to limits?
				}
				if (gameController->isKeyDown(DH_CTRL_CAMERA_TARGET_OFFSET_MODE))
				{
					targetOffset.x += (0.5f * mdx) / 20.0f;
					targetOffset.z -= (0.5f * mdy) / 20.0f;
					// TODO: clamp to limits?
				}

				if (gameController->isKeyDown(DH_CTRL_CAMERA_LOOK_MODE)) 
				{
					if (!SimpleOptions::getBool(DH_OPT_B_CAMERA_ALPHA_ANGLE_LOCKED))
					{
						alphaAngle += (4.0f * mdx) / 20.0f;
						if (alphaAngle < 0) alphaAngle += 360;
						if (alphaAngle > 360) alphaAngle -= 360;
					}

					if (cameraMode == CAMERA_MODE_ZOOM_CENTRIC)
					{
						zoom += (2.0f * mdy) / 20.0f;
						if (zoom < minZoom) zoom = minZoom;
						if (zoom > maxZoom) zoom = maxZoom;
					} else {
						betaAngle += (4.0f * mdy) / 20.0f;
						if (cameraMode == CAMERA_MODE_TARGET_CENTRIC)
						{
							// check for both -10 and min angle here...
							// must be above both.
							if (betaAngle < -10) betaAngle = -10;
							if (betaAngle < minBetaAngle) betaAngle = minBetaAngle;
						} else {
							if (betaAngle < minBetaAngle) betaAngle = minBetaAngle;
						}
						if (betaAngle > maxBetaAngle) betaAngle = maxBetaAngle;
					}
				}
			}
		}

    if (firstPersonMode)
    {
      return;
    }

    if (!disabledUserMovement)
    {
      float terrHeight = 0;
      if (cameraX > -gameMap->getScaledSizeX() / 2 
        && cameraY > -gameMap->getScaledSizeY() / 2
        && cameraX < gameMap->getScaledSizeX() / 2
        && cameraY < gameMap->getScaledSizeY() / 2)
      {
        terrHeight = gameMap->getScaledHeightAt(cameraX, cameraY);
      }

      float heightfactor = 1.0f + 4.0f * ((cameraHeight - terrHeight) / gameMap->getScaledMaxHeight());

      float tmpX;
      float tmpY;
      if (move_left || key_move_left)
      {
        tmpX = (0.4f * delta) * cosf((-alphaAngle+90)*3.14f/180.0f) / 20.0f;
        tmpY = (0.4f * delta) * sinf((-alphaAngle+90)*3.14f/180.0f) / 20.0f;
        cameraX += tmpX * heightfactor;
        cameraY += tmpY * heightfactor;
        targetX += tmpX * heightfactor;
        targetY += tmpY * heightfactor;
      }

      if (move_right || key_move_right)
      {
        tmpX = (0.4f * delta) * cosf((-alphaAngle-90)*3.14f/180.0f) / 20.0f;
        tmpY = (0.4f * delta) * sinf((-alphaAngle-90)*3.14f/180.0f) / 20.0f;
        cameraX += tmpX * heightfactor;
        cameraY += tmpY * heightfactor;
        targetX += tmpX * heightfactor;
        targetY += tmpY * heightfactor;
      }

      tmpX = (cameraVelocity * delta) * cosf(-alphaAngle*3.14f/180.0f) / 20.0f;
      tmpY = (cameraVelocity * delta) * sinf(-alphaAngle*3.14f/180.0f) / 20.0f;
      cameraX += tmpX * heightfactor;
      cameraY += tmpY * heightfactor;
      targetX += tmpX * heightfactor;
      targetY += tmpY * heightfactor;
    }

    bool do_orbit = false;
    if (move_orbitleft || key_move_orbitleft)
    {
      alphaAngle += (1.0f * delta) / 20.0f;
      if (alphaAngle > 360) alphaAngle -= 360;
      do_orbit = true;
    }
    if (move_orbitright || key_move_orbitright)
    {
      alphaAngle -= (1.0f * delta) / 20.0f;
      if (alphaAngle < 0) alphaAngle += 360;
      do_orbit = true;
    }
    if (do_orbit && cameraMode != CAMERA_MODE_TARGET_CENTRIC)
    {
      VC3 orbpos = VC3(targetX, targetHeight, targetY);
      VC3 orbtarg = VC3(-targDist,0,0);
      QUAT orbrot = QUAT();
      if (cameraMode == CAMERA_MODE_ZOOM_CENTRIC)
      {
        orbrot.MakeFromAngles(0, alphaAngle*3.14f/180.0f, 0);
      } else {
        orbrot.MakeFromAngles(0, alphaAngle*3.14f/180.0f, betaAngle*3.14f/180.0f);
      }
      orbtarg = orbrot.GetRotated(orbtarg);
      orbpos = orbpos + orbtarg;
      cameraX = orbpos.x;
      cameraY = orbpos.z;
    }

    VC3 pos;
    VC3 targ;
    QUAT rot;
    float minHeight;
    float minTargHeight;
    switch (cameraMode)
    {
    case CAMERA_MODE_ZOOM_CENTRIC:
      minHeight = 0;
      if (cameraX > -gameMap->getScaledSizeX() / 2 
        && cameraY > -gameMap->getScaledSizeY() / 2
        && cameraX < gameMap->getScaledSizeX() / 2
        && cameraY < gameMap->getScaledSizeY() / 2)
      {
        minHeight = gameMap->getScaledHeightAt(cameraX, cameraY) + 2;

				// keep it above water....
				if (waterManager != NULL)
				{
					VC3 tmp(cameraX, minHeight - CAMERA_ABOVE_WATER, cameraY);
					float waterDepth = waterManager->getWaterDepthAt(tmp);
					if (waterDepth > 0) minHeight += waterDepth;
				}
      }
  //    if (cameraHeight < minHeight) cameraHeight = minHeight;
      cameraHeight = minHeight + zoom;

      pos = VC3(cameraX, cameraHeight, cameraY);
      targ = VC3(targDist,0,0);
      rot = QUAT();
      //rot.MakeFromAngles(0, alphaAngle*3.14f/180.0f, betaAngle*3.14f/180.0f);
      rot.MakeFromAngles(0, alphaAngle*3.14f/180.0f, 0);
      targ = rot.GetRotated(targ);
      targ = pos + targ;

      targ.y = cameraHeight - zoom;
  //    targ.y = 0;

      targetX = targ.x;
      targetY = targ.z;
      targetHeight = targ.y;
      break;

    case CAMERA_MODE_CAMERA_CENTRIC:
    case CAMERA_MODE_FLYING:
      minHeight = 0;
      if (cameraX > -gameMap->getScaledSizeX() / 2 
        && cameraY > -gameMap->getScaledSizeY() / 2
        && cameraX < gameMap->getScaledSizeX() / 2
        && cameraY < gameMap->getScaledSizeY() / 2)
      {
        //minHeight = gameMap->getScaledHeightAt(cameraX, cameraY) + 2;
#ifdef PROJECT_CLAW_PROTO
      minHeight = -999999.0f;
#else
        minHeight = gameMap->getScaledHeightAt(cameraX, cameraY) + 0.5f;
#endif

				// keep it above water....
				if (waterManager != NULL)
				{
					VC3 tmp(cameraX, minHeight - CAMERA_ABOVE_WATER, cameraY);
					float waterDepth = waterManager->getWaterDepthAt(tmp);
					if (waterDepth > 0) minHeight += waterDepth;
				}
      }
      if (cameraMode == CAMERA_MODE_FLYING)
      {
        if (cameraHeight < minHeight) cameraHeight = minHeight;
      } else {
        cameraHeight = minHeight + zoom;
      }

      pos = VC3(cameraX, cameraHeight, cameraY);
      targ = VC3(targDist,0,0);
      rot = QUAT();
      rot.MakeFromAngles(0, alphaAngle*3.14f/180.0f, betaAngle*3.14f/180.0f);
      targ = rot.GetRotated(targ);
      targ = pos + targ;

      targetX = targ.x;
      targetY = targ.z;
      targetHeight = targ.y;
      break;

    case CAMERA_MODE_TARGET_CENTRIC:

      minTargHeight = 0;
      if (gameMap->isInScaledBoundaries(targetX, targetY))
      {
        minTargHeight = gameMap->getScaledHeightAt(targetX, targetY) + 2;

				// keep it above water....
				if (waterManager != NULL)
				{
					VC3 tmp(targetX, minTargHeight - CAMERA_ABOVE_WATER, targetY);
					float waterDepth = waterManager->getWaterDepthAt(tmp);
					if (waterDepth > 0) minTargHeight += waterDepth;
				}
      }
      //if (targetHeight < minTargHeight) 
      targetHeight = minTargHeight;


      targ = VC3(targetX, targetHeight, targetY);
      pos = VC3(-zoom, 0, 0);
      rot = QUAT();
      rot.MakeFromAngles(0, alphaAngle*3.14f/180.0f, betaAngle*3.14f/180.0f);
      pos = rot.GetRotated(pos);
      pos = targ + pos;

      cameraX = pos.x;
      cameraY = pos.z;
      cameraHeight = pos.y;

      minHeight = 0;
      if (gameMap->isInScaledBoundaries(cameraX, cameraY))
      {
        minHeight = gameMap->getScaledHeightAt(cameraX, cameraY) + 2;
				// keep it above water....
				if (waterManager != NULL)
				{
					VC3 tmp(cameraX, minHeight - CAMERA_ABOVE_WATER, cameraY);
					float waterDepth = waterManager->getWaterDepthAt(tmp);
					if (waterDepth > 0) minHeight += waterDepth;
				}
      }
      if (cameraHeight < minHeight) 
      {
        cameraHeight = minHeight;
        pos.y = cameraHeight;
      }

      break;

	case CAMERA_MODE_INVALID:
		igiosErrorMessage("impossible cameraMode: CAMERA_MODE_INVALID");
		break;
    }

    // if we are jumping to another location
    if (toDestination)
    {
      destinationTime += (1.0f * delta) / 20.0f;
      if (destinationTime >= CAM_TIME_TO_DESTINATION)
      {
        destinationTime = CAM_TIME_TO_DESTINATION;
        toDestination = false;
      }
      float newCameraX = (sourceX * (CAM_TIME_TO_DESTINATION - destinationTime)
        + destinationX * destinationTime) / CAM_TIME_TO_DESTINATION;
      float newCameraY = (sourceY * (CAM_TIME_TO_DESTINATION - destinationTime)
        + destinationY * destinationTime) / CAM_TIME_TO_DESTINATION;
      cameraX = newCameraX;
      cameraY = newCameraY;

      float minHeight2 = 0;
      if (gameMap->isInScaledBoundaries(cameraX, cameraY))
      {
        minHeight2 = gameMap->getScaledHeightAt(cameraX, cameraY) + 2;

				// keep it above water....
				if (waterManager != NULL)
				{
					VC3 tmp(cameraX, minHeight2 - CAMERA_ABOVE_WATER, cameraY);
					float waterDepth = waterManager->getWaterDepthAt(tmp);
					if (waterDepth > 0) minHeight2 += waterDepth;
				}
      }
      cameraHeight = minHeight2 + zoom;

      VC3 pos2 = VC3(cameraX, cameraHeight, cameraY);
      VC3 targ2;
      if (cameraMode == CAMERA_MODE_TARGET_CENTRIC)
        targ2 = VC3(zoom,0,0);
      else
        targ2 = VC3(targDist,0,0);
      QUAT rot2 = QUAT();
      if (cameraMode == CAMERA_MODE_ZOOM_CENTRIC)
        rot2.MakeFromAngles(0, alphaAngle*3.14f/180.0f, 0);
      else
        rot2.MakeFromAngles(0, alphaAngle*3.14f/180.0f, betaAngle*3.14f/180.0f);
      targ2 = rot2.GetRotated(targ2);
      targ2 = pos2 + targ2;

      targ2.y = cameraHeight - zoom;

      targetX = targ2.x;
      targetY = targ2.z;    
      targetHeight = targ2.y;
    }

    if (followingUnit && disabledUserMovement)
    {
      float offsetX = followX - targetX;
      float offsetY = followY - targetY;

      float diffX = (float)fabs(offsetX);
      float diffY = (float)fabs(offsetY);
      if (diffX > 2 || diffY > 2 || toDestination)
      {
        if (!toDestination)
          setDestination(followX, followY);
      } else {
        targetX += offsetX;
        targetY += offsetY;
        cameraX += offsetX;
        cameraY += offsetY;
        targ.x = targetX;
        targ.z = targetY;
        pos.x = cameraX;
        pos.z = cameraY;
      }
    }

    gameMap->keepWellInScaledBoundaries(&cameraX, &cameraY);
    pos.x = cameraX;
    pos.z = cameraY;

		if (boundaries != NULL)
		{
			if (!boundaries->isPositionInsideBoundaries(pos))
			{
				VC3 vec = boundaries->getVectorToInsideBoundaries(pos);
				pos += vec;
				cameraX = pos.x;
				cameraY = pos.z;
				targetX += vec.x;
				targetY += vec.z;
			}
		}

    if (interpolateCamera != NULL)
    {
      bool stopInterpolate = false;
      interpolateTime += (1.0f * delta) / 20.0f;
      if (interpolateTime >= interpolateDuration)
      {
        interpolateTime = interpolateDuration;
        stopInterpolate = true;
      }
      VC3 otherPos = VC3(interpolateCamera->cameraX, 
        interpolateCamera->cameraHeight, 
        interpolateCamera->cameraY);
      VC3 otherTarg = VC3(interpolateCamera->targetX, 
        interpolateCamera->targetHeight, 
        interpolateCamera->targetY);
      VC3 otherPosOffset = interpolateCamera->positionOffset; 
			VC3 otherTargOffset = interpolateCamera->targetOffset;

			VC3 interPos;
			VC3 interTarg;
			VC3 interPosOffset;
			VC3 interTargOffset;
			bool smooth = false;
			if ((interpolateTime / interpolateDuration) < 0.5f)
			{
				if (interpolateCamera->smoothInterpolation)
					smooth = true;
				else
					smooth = false;				
			} else {
				if (this->smoothInterpolation)
					smooth = true;
				else
					smooth = false;				
			}
			if (smooth)
			{
        interPos = otherPos + (pos - otherPos) * 
					((float)sinf(3.1415926f * (interpolateTime / interpolateDuration - 0.5f)) * 0.5f + 0.5f);
        interTarg = otherTarg + (targ - otherTarg) * 
					((float)sinf(3.1415926f * (interpolateTime / interpolateDuration - 0.5f)) * 0.5f + 0.5f);
        interPosOffset = otherPosOffset + (positionOffset - otherPosOffset) * 
					((float)sinf(3.1415926f * (interpolateTime / interpolateDuration - 0.5f)) * 0.5f + 0.5f);
        interTargOffset = otherTargOffset + (targetOffset - otherTargOffset) * 
	        ((float)sinf(3.1415926f * (interpolateTime / interpolateDuration - 0.5f)) * 0.5f + 0.5f);
			} else {
        interPos = otherPos + (pos - otherPos) * (interpolateTime / interpolateDuration);
        interTarg = otherTarg + (targ - otherTarg) * (interpolateTime / interpolateDuration);
        interPosOffset = otherPosOffset + (positionOffset - otherPosOffset) * (interpolateTime / interpolateDuration);
        interTargOffset = otherTargOffset + (targetOffset - otherTargOffset) * (interpolateTime / interpolateDuration);
			}

			this->interpolatedPosition = interPos;
			this->interpolatedPositionOffset = interPosOffset;
			this->interpolatedTargetOffset = interTargOffset;

			implData->setCameraForScene(interPos, interTarg);

      if (stopInterpolate)
        interpolateCamera = NULL;
    } else {
			this->interpolatedPosition = VC3(cameraX, cameraHeight, cameraY);
			this->interpolatedPositionOffset = positionOffset;
			this->interpolatedTargetOffset = targetOffset;
			implData->setCameraForScene(pos, targ);
    }
  }

	void GameCamera::copyFrom(GameCamera *camera)
	{
		this->alphaAngle = camera->alphaAngle;
		this->betaAngle = camera->betaAngle;
		this->cameraHeight = camera->cameraHeight;
		this->cameraMode = camera->cameraMode;
		this->cameraVelocity = camera->cameraVelocity;
		this->cameraX = camera->cameraX;
		this->cameraY = camera->cameraY;
		this->destinationTime = camera->destinationTime;
		this->destinationX = camera->destinationX;
		this->destinationY = camera->destinationY;
		this->sourceX = camera->sourceX;
		this->sourceY = camera->sourceY;
		this->targDist = camera->targDist;
		this->targetHeight = camera->targetHeight;
		this->targetX = camera->targetX;
		this->targetY = camera->targetY;
		this->toDestination = camera->toDestination;
		this->zoom = camera->zoom;
		this->zooming_to = camera->zooming_to;
		this->positionOffset = camera->positionOffset;
		this->targetOffset = camera->targetOffset;

		// don't copy these kinds things?
		//this->disabledDirectControls = camera->disabledDirectControls;
	}

	void GameCamera::setHeight(float height)
	{
		this->cameraHeight = height;
	}

	void GameCamera::applyToSceneAbsolute(VC3 position, VC3 target, VC3 up, float FOV)
	{
        VC3 dir = target-position;
		VC3 flatDir(dir.x, 0.0f, dir.z);
        VC3 zeroAngle(0.0f, 0.0f, 1.0f);
		VC3 upDir(0.0f, 1.0f, 0.0f);

		/*
		alphaAngle = 180*flatDir.GetAngleTo(zeroAngle)/3.1415f;
		if (flatDir.GetCrossWith(zeroAngle).y < 0) {
			alphaAngle = 360-alphaAngle;
		}
		*/
		float tmp = 180*flatDir.GetAngleTo(zeroAngle)/3.1415f;
		if (flatDir.GetCrossWith(zeroAngle).y < 0) {
			tmp = 360-alphaAngle;
		}
		tmp += 90.0f;
		if (tmp >= 360.0f) tmp -= 360.0f;
		setAngleY(tmp);

		betaAngle = -(90-180*dir.GetAngleTo(upDir)/3.1415f);

		cameraX = position.x;
		cameraY = position.z;
		cameraHeight = position.y;

		this->interpolatedPosition = getPosition();
		this->interpolatedPositionOffset = VC3(0,0,0);

		targetX = target.x;
		targetY = target.z;
		targetHeight = target.y;

		zoom = dir.GetLength();
		fov = FOV;

		scene->GetCamera()->SetFieldOfViewFactor(fovFactor);
		scene->GetCamera()->SetFieldOfView(fov * 3.1415f / 180.0f / 2.0f);
		scene->GetCamera()->SetPosition(position);
		scene->GetCamera()->SetTarget(target);
		scene->GetCamera()->SetUpVec(up);
	}

	void GameCamera::applyToScene()
	{
		static bool old_forceMapView = forceMapView;
		static VC3 old_upVec;
		if(!forceMapView)
		{
			if(old_forceMapView) // Just switched to normal view from map view
			{
				scene->GetCamera()->SetUpVec(old_upVec);				
				scene->GetCamera()->ForceOrthogonalProjection(false);
				gameMap->getTerrain()->getRenderer ().setFloatValue( IStorm3D_TerrainRenderer::ForceAmbient, 0.0f );
				
			}

			if (implData->lastFOV != fov * fovFactor)
			{
				implData->lastFOV = fov * fovFactor;
				scene->GetCamera()->SetFieldOfViewFactor(fovFactor);
				scene->GetCamera()->SetFieldOfView(fov * 3.1415f / 180.0f / 2.0f);
			}

			assert(implData->camerasAdded >= 1);
			// FIXME!
			/*
			if (implData->camerasAdded == 0)
			{
				assert(fov >= 20 && fov <= 170);
				scene->GetCamera()->SetPosition(VC3(0,20,0));
				scene->GetCamera()->SetTarget(VC3(1,1,1));
				return;
			}
			*/

			VC3 finalPos;
			VC3 finalTarg;
			if (implData->camerasAdded == 1)
			{
				finalPos = implData->cameraPosForScene;
				finalTarg = implData->cameraTargForScene;
			} else {
				finalPos = implData->cameraPosForScene / (float)implData->camerasAdded;
				finalTarg = implData->cameraTargForScene / (float)implData->camerasAdded;
			}

			if (implData->shakeAmount > 0)
			{
				//float shakeFact = float(shakeAmount) * 1.0f - (fabs(float((CAMERA_SHAKE_TIME / 2) - shakeTime)) / (CAMERA_SHAKE_TIME / 2.0f));
				float shakeFact = float(implData->shakeAmount) * (1.0f - (implData->shakeTime / implData->totalShakeTime));
				VC3 shakeVec = VC3(
					float((SystemRandom::getInstance()->nextInt() % 1001) - 500),
					float((SystemRandom::getInstance()->nextInt() % 1001) - 500),
					float((SystemRandom::getInstance()->nextInt() % 1001) - 500));
				shakeVec *= (CAMERA_SHAKE_GLOBAL_FACTOR / 1000.0f);
				shakeVec *= shakeFact;

				finalPos += shakeVec;
				finalTarg += shakeVec;
			}

			this->interpolatedPosition = finalPos;

			/*
			finalPos.x += SimpleOptions::getFloat(DH_OPT_F_CAMERA_POSITION_OFFSET_X);
			finalPos.z += SimpleOptions::getFloat(DH_OPT_F_CAMERA_POSITION_OFFSET_Z);
			finalTarg.x += SimpleOptions::getFloat(DH_OPT_F_CAMERA_TARGET_OFFSET_X);
			finalTarg.z += SimpleOptions::getFloat(DH_OPT_F_CAMERA_TARGET_OFFSET_Z);
			*/
			finalPos += this->interpolatedPositionOffset;
			finalTarg += this->interpolatedTargetOffset;

			// test hack

			scene->GetCamera()->SetPosition(finalPos);
			scene->GetCamera()->SetTarget(finalTarg);
		} else 
		{
			// warning: lots of scary hacks ahead...

			if(!old_forceMapView) // Just switched to map view.
			{
				old_upVec = scene->GetCamera()->GetUpVec();
				/*int dummy1=0, dummy2=0;
				game->gameScripting->runSingleSimpleStringCommand( "hideAllUnits", "", &dummy1, &dummy2 );*/
			}

			gameMap->getTerrain()->getRenderer ().setFloatValue( IStorm3D_TerrainRenderer::ForceAmbient, 
				game::SimpleOptions::getFloat( DH_OPT_F_MAPVIEW_AMBIENT_LIGHTING )
				);

			this->setPosition( VC3(0,500,0) );
			this->setUpVector( VC3(1,0,0) );
			targetX = 0;
			targetY = 0;
			targetHeight = 0;

			static IGameCameraBoundary * mbn = new NoCameraBoundary();
			this->setBoundaries ( mbn );
			this->setHeight ( 500.0f );

			scene->GetCamera()->SetPosition(VC3(0,500,0));
			scene->GetCamera()->SetTarget(VC3(0,0,0));
			scene->GetCamera()->SetUpVec (VC3(1,0,0));

			float mapwidth  = gameMap->getScaledSizeX();
			float mapheight = gameMap->getScaledSizeY();
			int scrwidth = 0, scrheight = 0; 
			
			float scrAspect = this->screenAspect;
			if(scene->getStorm()->getRenderTarget(0))
			{
				scrwidth = scene->getStorm()->getRenderTarget(0)->getWidth();
				scrheight = scene->getStorm()->getRenderTarget(0)->getHeight();
			}
			else
			{
				scrwidth = scene->getStorm()->GetScreenSize().width;
				scrheight = scene->getStorm()->GetScreenSize().height;
			}

			scrAspect = (float)scrwidth / scrheight;

			float orthoOffsetX = 0.0f, orthoOffsetY = 0.0f, orthoWidth = mapwidth, orthoHeight = mapheight;
			if( mapviewAreaX >= 0 )  
			{
				orthoOffsetX = 512.f / mapviewWidth  * mapwidth * mapviewAreaX;
				orthoOffsetY = 512.f / mapviewHeight * mapheight * mapviewAreaY;
				orthoWidth  = 512.f / mapviewWidth * mapwidth;
				orthoHeight = 512.f / mapviewHeight * mapheight;
			}
			

	
			if( SimpleOptions::getBool( DH_OPT_B_MAPRENDER_USE_USER_RESOLUTION ) || !scene->getStorm()->getRenderTarget(0) )
			{
				if(mapwidth <= mapheight)
					scene->GetCamera()->ForceOrthogonalProjection (true, 
					-mapheight/2, mapheight/2,
					-mapheight/(2 * scrAspect), mapheight/(2 * scrAspect) );
				else
					scene->GetCamera()->ForceOrthogonalProjection (true, 
					-mapwidth/2 * scrAspect, mapwidth/2 * scrAspect,
					-mapwidth/2, mapwidth/2 );
			}
			else
			{
				scene->GetCamera()->ForceOrthogonalProjection (true, 
					-mapheight/2 + orthoOffsetY, -mapheight/2 + orthoOffsetY + orthoHeight,
					-mapwidth/2 + orthoOffsetX, -mapwidth/2 + orthoOffsetX + orthoWidth );
			}
		}
		old_forceMapView = forceMapView;
	}

	void GameCamera::resetForScene()
	{
		implData->cameraPosForScene = VC3(0,0,0);
		implData->cameraTargForScene = VC3(0,0,0);
		implData->camerasAdded = 0;
	}

	void GameCamera::setSmoothCameraInterpolation(bool smoothInterpolation)
	{
		this->smoothInterpolation = smoothInterpolation;
	}

  void GameCamera::restoreDefaultTargetDistance()
	{
    targDist = CAM_DEFAULT_TARG_DIST;
	}

	void GameCamera::zoomTo(float zoom)
	{
		this->zooming_to = zoom;
	}


	void GameCamera::setGameCameraMode(bool aimUpward)
	{
		// HACK HACK HACK!!!
		if (aimUpward != getGameCameraMode())
		{
			if (aimUpward)
			{
				game::SimpleOptions::setBool(DH_OPT_B_GAME_MODE_AIM_UPWARD, true);
				//game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE, 60);
				//game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE, 60);
				//minBetaAngle = 60;
				//maxBetaAngle = 60;
			} else {
				game::SimpleOptions::setBool(DH_OPT_B_GAME_MODE_AIM_UPWARD, false);
				//game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE, 55);
				//game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE, 85);
				//minBetaAngle = 55;
				//maxBetaAngle = 85;
			}
		}
	}


	bool GameCamera::getGameCameraMode()
	{
		return game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD);
	}

	void GameCamera::setGameCameraYAxisLock(bool locked)
	{
		// HACK HACK HACK!!!
		if (locked != getGameCameraYAxisLock())
		{
			if (locked)
			{
				if (betaAngle < 55)
					betaAngle = 55;
				if (betaAngle > 85)
					betaAngle = 85;
				game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE, (int)betaAngle);
				game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE, (int)betaAngle);
				minBetaAngle = betaAngle;
				maxBetaAngle = betaAngle;
			} else {
				game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE, 55);
				game::SimpleOptions::setInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE, 85);
				minBetaAngle = 55;
				maxBetaAngle = 85;
			}
		}
	}

	bool GameCamera::getGameCameraYAxisLock()
	{
		if (game::SimpleOptions::getInt(DH_OPT_I_CAMERA_MIN_BETA_ANGLE) == 
			game::SimpleOptions::getInt(DH_OPT_I_CAMERA_MAX_BETA_ANGLE))
		{
			return true;
		} else {
			return false;
		}
	}

	void GameCamera::setUpVector(const VC3 &upVector)
	{
		this->implData->upVector = upVector;
		if (scene != NULL && scene->GetCamera() != NULL )
		{
			/*
			scene->GetCamera()->SetUpVec(this->implData->upVector);
			*/

			QUAT tmp_quat( 0, DEG2RAD( game::Ani::getGlobalRotation() ), 0 );
			tmp_quat.RotateVector( this->implData->upVector );

			scene->GetCamera()->SetUpVec(this->implData->upVector);
		}
		else
		{
			Logger::getInstance()->error( "GameCamera::setUpVector() failed - no storm scene or camera\n" );
		}
	}

	/*
	VC3 GameCamera::getUpVector()
	{
		return this->implData->upVector;
	}
	*/


	/*
	void GameCamera::applyToViewport(PlayerViewport *viewport)
	{
		if (implData->lastFOV != fov)
		{
			implData->lastFOV = fov;
			scene->GetCamera()->SetFieldOfView(fov * 3.1415f / 180.0f / 2.0f);
		}
    scene->GetCamera()->SetPosition(pos);
    scene->GetCamera()->SetTarget(targ);
	}
	*/

	float GameCamera::fovFactor = 1.0f;
}
