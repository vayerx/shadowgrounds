
#include "precompiled.h"

#include "CameraScripting.h"

#include "scripting_macros_start.h"
#include "camera_script_commands.h"
#include "scripting_macros_end.h"

// NOTE: Some problems with the DatatypeDef.h including...
// unless this exists here, some math stuff will not be included.
// (even though this DatatypeDef is included by other headers)
#include <DatatypeDef.h>

#include "../Game.h"
#include "../GameUI.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../../ui/GameCamera.h"
#include "../../ui/CameraAutozoomer.h"
#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../../util/ObjectStretchingCalculator.h"
#include "../../util/AngleRotationCalculator.h"
#include "../EnvironmentalEffectManager.h"
#include "../Ani.h"

#include "../SimpleOptions.h"
#include "GameScripting.h"
#include "../GameScene.h"
#include <IStorm3D_Scene.h>
#include <IStorm3D.h>

#include "../options/options_graphics.h"
#include "../options/options_camera.h"

#include "../../util/Debug_MemoryManager.h"

#include "../../ui/camera_system/CameraAreaStreet.h"
#include "../../ui/camera_system/ICameraSystem.h"

using namespace ui;

namespace game
{
	CameraAreaParameters CameraScripting::cameraAreaParameters = CameraAreaParameters();	

	void CameraScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_CAMERAPOSITION:
			// TODO: proper handling of players in netgame
			gsd->position = game->gameUI->getGameCamera()->getPosition();

#ifdef LEGACY_FILES
			// ok
#else
			sp->warning("CameraScripting::process - cameraPosition deprecated, use actualCameraPosition instead.");
#endif
			break;

		case GS_CMD_LISTENERPOSITION:
			// TODO: proper handling of players in netgame
			
			// TODO: real listener position, this is just an approximation...
			// (that is, using camera position as the camera position for now)
			//gsd->position = game->gameUI->getGameCamera()->getPosition();
			gsd->position = game->gameUI->getListenerPosition();
			break;

		case GS_CMD_SETCAMERAPOSITIONNEAR:
			// TODO: proper handling of players in netgame
			game->gameUI->getGameCamera()->setPositionNear(gsd->position.x, gsd->position.z);
			break;

		case GS_CMD_SETCAMERAPOSITION:
			{
				// new: global offsetting via 2 pivots and rotation around the nwe "pivot".
				VC3 pos = gsd->position;

				pos -= Ani::getGlobalOffsetSource();
				float angle = Ani::getGlobalRotation();

				float tmpX = pos.x;
				pos.x = pos.x * cosf(UNIT_ANGLE_TO_RAD(angle)) + pos.z * sinf(UNIT_ANGLE_TO_RAD(angle));
				pos.z = pos.z * cosf(UNIT_ANGLE_TO_RAD(angle)) - tmpX * sinf(UNIT_ANGLE_TO_RAD(angle));

				pos += Ani::getGlobalOffsetTarget();

				game->gameUI->getGameCamera()->setPosition(pos);
				game->gameUI->getGameCamera()->doMovement(1);
			}
			break;

		case GS_CMD_rotateCameraAroundPosition:
			game->gameUI->getGameCamera()->rotateAroundPosition(gsd->position, *lastValue);
			game->gameUI->getGameCamera()->doMovement(1);
			break;

		case GS_CMD_SETCAMERAANGLE:
			// TODO: proper handling of players in netgame
			if (intData >= 0 || intData < 360)
			{
				float angle = (float)intData;
				angle += Ani::getGlobalRotation();
				if (angle < 0) angle += 360.0f;
				if (angle > 360.0f) angle -= 360.0f;

				game->gameUI->getGameCamera()->setAngleY(angle);
			} else {
				sp->error("GameScripting::process - setCameraAngle parameter bad.");
			}
			game->gameUI->getGameCamera()->doMovement(1);
			break;
		case GS_CMD_SETCAMERAANGLEFLOAT:
			{
				float floatData = intFloat.f;
				if (floatData >= 0.0f || floatData < 360.0f)
				{
					floatData -= Ani::getGlobalRotation();
					if (floatData < 0) floatData += 360.0f;
					if (floatData > 360.0f) floatData -= 360.0f;

					game->gameUI->getGameCamera()->setAngleY(floatData);
				} else {
					sp->error("GameScripting::process - setCameraAngleFloat parameter bad.");
				}
				game->gameUI->getGameCamera()->doMovement(1);
			}
			break;

		case GS_CMD_SETCAMERAANGLETOVALUE:
			game->gameUI->getGameCamera()->setAngleY((float)(90 - *lastValue));
			game->gameUI->getGameCamera()->doMovement(1);
			break;

		case GS_CMD_GETCAMERAANGLE:
			{
				*lastValue = (int)game->gameUI->getGameCamera()->getAngleY();
				*lastValue = 270 - *lastValue;
				if (*lastValue < 0) *lastValue += 360;
			}
			break;

		case GS_CMD_GETCAMERABETAANGLE:
			{
				*lastValue = (int)game->gameUI->getGameCamera()->getBetaAngle();
			}
			break;

		case GS_CMD_SETCAMERABETAANGLE:
			// TODO: proper handling of players in netgame
			if (intData >= -180 || intData < 180)
			{
				game->gameUI->getGameCamera()->setBetaAngle((float)intData);
			} else {
				sp->error("GameScripting::process - setCameraBetaAngle parameter bad.");
			}
			game->gameUI->getGameCamera()->doMovement(1);
			break;

		case GS_CMD_SETCAMERABETAANGLEFLOAT:
			{
				float floatData = intFloat.f;
				if (floatData >= -180 || floatData < 180)
				{
					game->gameUI->getGameCamera()->setBetaAngle(floatData);
				} else {
					sp->error("GameScripting::process - setCameraBetaAngleFloat parameter bad.");
				}
				game->gameUI->getGameCamera()->doMovement(1);
			}
			break;

		case GS_CMD_SETCAMERAZOOM:
			// TODO: proper handling of players in netgame
			game->gameUI->getGameCamera()->setZoom((float)intData);
			game->gameUI->getGameCamera()->doMovement(1);
			break;

		case GS_CMD_SETCAMERAFLOATZOOM:
			if (stringData != NULL)
			{
				game->gameUI->getGameCamera()->setZoom((float)atof(stringData));
				game->gameUI->getGameCamera()->doMovement(1);
			} else {
				sp->error("GameScripting::process - setCameraFloatZoom parameter missing.");
			}
			break;

		case GS_CMD_SELECTCAMERA:
			if (stringData != NULL)
			{
				// oh yeah! copy & paste programming ahead...
				if (strncmp(stringData, "cinematic1,", 11) == 0)
				{
					GameCamera *curcam = game->gameUI->getGameCamera();
					game->gameUI->selectCamera(GAMEUI_CAMERA_CINEMATIC1);
					int itime = str2int(&stringData[11]);
					if (itime != 0)
						game->gameUI->getGameCamera()->interpolateFrom(curcam, (float)itime);
					else
						game->gameUI->getGameCamera()->interpolateFrom(game->gameUI->getGameCamera(), 0);
					game->gameUI->getGameCamera()->doMovement(1);
					break;
				}
				if (strncmp(stringData, "cinematic2,", 11) == 0)
				{
					GameCamera *curcam = game->gameUI->getGameCamera();
					game->gameUI->selectCamera(GAMEUI_CAMERA_CINEMATIC2);
					int itime = str2int(&stringData[11]);
					if (itime != 0)
						game->gameUI->getGameCamera()->interpolateFrom(curcam, (float)itime);
					else
						game->gameUI->getGameCamera()->interpolateFrom(game->gameUI->getGameCamera(), 0);
					game->gameUI->getGameCamera()->doMovement(1);
					break;
				}
				if (strncmp(stringData, "normal,", 7) == 0)
				{
					GameCamera *curcam = game->gameUI->getGameCamera();
					game->gameUI->selectCamera(GAMEUI_CAMERA_NORMAL);
					int itime = str2int(&stringData[7]);
					if (itime != 0)
						game->gameUI->getGameCamera()->interpolateFrom(curcam, (float)itime);
					else
						game->gameUI->getGameCamera()->interpolateFrom(game->gameUI->getGameCamera(), 0);
					game->gameUI->getGameCamera()->doMovement(1);
					break;
				}
				if (strncmp(stringData, "tactical,", 9) == 0)
				{
					GameCamera *curcam = game->gameUI->getGameCamera();
					game->gameUI->selectCamera(GAMEUI_CAMERA_TACTICAL);
					int itime = str2int(&stringData[9]);
					if (itime != 0)
						game->gameUI->getGameCamera()->interpolateFrom(curcam, (float)itime);
					else
						game->gameUI->getGameCamera()->interpolateFrom(game->gameUI->getGameCamera(), 0);
					game->gameUI->getGameCamera()->doMovement(1);
					break;
				}
				sp->error("GameScripting::process - selectCamera parameter bad.");
			} else {
				sp->error("GameScripting::process - selectCamera parameter expected.");
			}
			break;
			
		case GS_CMD_SETCAMERAMOVEMENTON:
			// TODO: proper handling of players in netgame
			if (stringData != NULL)
			{
				GameCamera::CAMERA_MOVE moveNum = GameCamera::getCameraMoveByName(stringData);
				if (moveNum != GameCamera::CAMERA_MOVE_INVALID)
				{
					game->gameUI->getGameCamera()->setMovement(moveNum, true);
				} else {
					sp->error("GameScripting::process - setCameraMovementOn parameter invalid.");
				}
			} else {
				sp->error("GameScripting::process - setCameraMovementOn parameter missing.");
			}
			break;

		case GS_CMD_SETCAMERAMOVEMENTOFF:
			// TODO: proper handling of players in netgame
			if (stringData != NULL)
			{
				GameCamera::CAMERA_MOVE moveNum = GameCamera::getCameraMoveByName(stringData);
				if (moveNum != GameCamera::CAMERA_MOVE_INVALID)
				{
					game->gameUI->getGameCamera()->setMovement(moveNum, false);
				} else {
					sp->error("GameScripting::process - setCameraMovementOff parameter invalid.");
				}
			} else {
				sp->error("GameScripting::process - setCameraMovementOff parameter missing.");
			}
			break;

		case GS_CMD_SETCAMERAMODE:
			// TODO: proper handling of players in netgame
			if (stringData != NULL)
			{
				GameCamera::CAMERA_MODE modeNum = GameCamera::getCameraModeByName(stringData);
				if (modeNum != GameCamera::CAMERA_MODE_INVALID)
				{
					game->gameUI->getGameCamera()->setMode(modeNum);
					game->gameUI->getGameCamera()->doMovement(1);
				} else {
					sp->error("GameScripting::process - setCameraMode parameter invalid.");
				}
			} else {
				sp->error("GameScripting::process - setCameraMode parameter missing.");
			}
			break;

		case GS_CMD_SETCAMERATIMEFACTOR:
			if (stringData != NULL)
			{
				float timeFactor = (float)atof(stringData);
				if (timeFactor < 0.1f) timeFactor = 0.1f;
				if (timeFactor > 10.0f) timeFactor = 10.0f;
				game->gameUI->setCameraTimeFactor(timeFactor);
			} else {
				sp->error("GameScripting::process - setCameraTimeFactor parameter missing.");
			}
			break;

		case GS_CMD_ISCAMERAZOOMLESSTHAN:
			if (game->gameUI->getGameCamera()->getZoom() < (float)intData)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_ISCAMERAZOOMGREATERTHAN:
			if (game->gameUI->getGameCamera()->getZoom() > (float)intData)
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETCAMERAFOV:
			game->gameUI->getGameCamera()->setFOV((float)intData);
			break;

		case GS_CMD_setCameraFOVFloat:
			{
				float floatData = intFloat.f;
				game->gameUI->getGameCamera()->setFOV(floatData);
			}
			break;

		case GS_CMD_COPYCAMERATO:
			if (stringData != NULL)
			{
				int curnum = game->gameUI->getCameraNumber();
				GameCamera *curcam = game->gameUI->getGameCamera();
				bool paramok = false;
				if (strcmp(stringData, "cinematic1") == 0)
				{
					game->gameUI->selectCamera(GAMEUI_CAMERA_CINEMATIC1);
					paramok = true;
				}
				if (strcmp(stringData, "cinematic2") == 0)
				{
					game->gameUI->selectCamera(GAMEUI_CAMERA_CINEMATIC2);
					paramok = true;
				}
				if (strcmp(stringData, "normal") == 0)
				{
					game->gameUI->selectCamera(GAMEUI_CAMERA_NORMAL);
					paramok = true;
				}
				if (strcmp(stringData, "tactical") == 0)
				{
					game->gameUI->selectCamera(GAMEUI_CAMERA_TACTICAL);
					paramok = true;
				}
				game->gameUI->getGameCamera()->copyFrom(curcam);
				game->gameUI->selectCamera(curnum);
				game->gameUI->getGameCamera()->doMovement(1);
				if (!paramok)
				{
					sp->error("GameScripting::process - selectCamera parameter bad.");
				}
			} else {
				sp->error("GameScripting::process - selectCamera parameter expected.");
			}
			break;

		case GS_CMD_setMapView:
			{

				bool showMap = stringData != NULL ? true : false;

				if(strcmp("stringData", "0") == 0)
					showMap = false;

				IStorm3D *storm = game->getGameUI()->getStorm3D();

				int dummy1 = 0, dummy2 = 0;

				if(showMap)
				{
					game->gameScripting->runSingleSimpleStringCommand( "hideGUI", "", &dummy1, &dummy2 );
					game->gameScripting->runSingleSimpleStringCommand( "hideAllUnits", "", &dummy1, &dummy2 );

					game->getEnvironmentalEffectManager()->enableSunlight();
					game->getEnvironmentalEffectManager()->setSunlightDirection ( VC3( 1,-1, 1) );
					game->getEnvironmentalEffectManager()->setSunlightColor ( COL(1,1,1) );

				} else
				{
					game->gameScripting->runSingleSimpleStringCommand( "showAllUnits", "", &dummy1, &dummy2 );
					game->gameScripting->runSingleSimpleStringCommand( "showGUI", "", &dummy1, &dummy2 );
					game->getEnvironmentalEffectManager()->disableSunlight();
				}

				if(showMap)
				{

					float scrAspect;
					if(storm->getRenderTarget(0))
						scrAspect = (float)storm->getRenderTarget(0)->getWidth() / storm->getRenderTarget(0)->getHeight();
					else
						scrAspect = (float)storm->GetScreenSize().width / storm->GetScreenSize().height;

					int areaX = -1, areaY = -1;
					// if stringData is in form NxN, where N is a number...
					if(  strlen(stringData) == 3 &&
						( stringData[0] >='1' && stringData[0] <='9' ) &&
						( stringData[1] == 'x' ) &&
						( stringData[2] >='1' && stringData[2] <='9' ) )
					{
						areaX = stringData[0] - '1';
						areaY = stringData[2] - '1';
					}

					game->gameUI->getGameCamera()->setForceMapView(true, areaX, areaY, 1024, 1024, scrAspect );

				}
				else
					game->gameUI->getGameCamera()->setForceMapView(false);

			}
			break;

		case GS_CMD_ROTATECAMERATOWARDUNIT:
			if (gsd->unit != NULL)
			{
				VC3 upos = gsd->unit->getPosition();
				VC3 campos = game->gameUI->getGameCamera()->getPosition();
				VC3 midpos(0,0,0);
				VC3 rotation(0,0,0);
				float scale;
				
				util::ObjectStretchingCalculator::calculateStretchValues(
					campos, upos, &midpos, &rotation, &scale);

				rotation.y = 270 - rotation.y;
				if (rotation.y < 0) rotation.y += 360;
				game->gameUI->getGameCamera()->setAngleY(rotation.y);
				game->gameUI->getGameCamera()->setBetaAngle(-rotation.x);
				game->gameUI->getGameCamera()->doMovement(1);

			} else {
				sp->warning("GameScripting::process - Attempt to rotateCameraTowardUnit for null unit.");
			} 					
			break;

		case GS_CMD_SETCAMERAHEIGHT:
			// TODO: proper handling of players in netgame
			game->gameUI->getGameCamera()->setHeight((float)intData);
			game->gameUI->getGameCamera()->doMovement(1);
			break;

		case GS_CMD_SHAKECAMERANEARPOSITIONSHORT:
			game->gameUI->getGameCamera()->setShakeEffect(intData, 500, gsd->position);
			break;

		case GS_CMD_SHAKECAMERANEARPOSITIONMEDIUM:
			game->gameUI->getGameCamera()->setShakeEffect(intData, 2500, gsd->position);
			break;

		case GS_CMD_SHAKECAMERANEARPOSITIONLONG:
			game->gameUI->getGameCamera()->setShakeEffect(intData, 8000, gsd->position);
			break;

		case GS_CMD_SETCAMERAINTERPOLATIONTYPE:
			if (stringData != NULL)
			{
				if (strcmp(stringData, "sinwave") == 0)
				{
					game->gameUI->getGameCamera()->setSmoothCameraInterpolation(true);
				}
				else if (strcmp(stringData, "linear") == 0)
				{
					game->gameUI->getGameCamera()->setSmoothCameraInterpolation(false);
				} 
				else 
				{
					sp->error("GameScripting::process - setCameraInterpolationType parameter bad.");
				}
			} else {
				// TODO: error
			}
			break;

		case GS_CMD_SETCAMERARANGE:
			if (intData > 0)
			{
				game->gameUI->setCameraRange((float)intData);
			} else {
				sp->error("GameScripting::process - setCameraRange parameter out of range (positive range value expected).");
			}
			break;

		case GS_CMD_GETCAMERARANGE:
			*lastValue = (int)game->gameUI->getCameraRange();
			break;

		case GS_CMD_RESTORECAMERARANGE:
			game->gameUI->restoreCameraRange();
			break;

		case GS_CMD_SETCAMERATARGETDISTANCE:
			if (game->inCombat)
			{
				game->gameUI->getGameCamera()->setTargetDistance((float)intData);
			}
			break;

		case GS_CMD_RESTORECAMERATARGETDISTANCE:
			if (game->inCombat)
			{
				game->gameUI->getGameCamera()->restoreDefaultTargetDistance();
			}
			break;

		case GS_CMD_setCameraAutozoomIndoor:
			if (game->inCombat)
			{
				float floatData = intFloat.f;
				ui::CameraAutozoomer::setAreaZoom(ui::CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_INDOOR, floatData);
			}
			break;

		case GS_CMD_setCameraAutozoomOutdoor:
			if (game->inCombat)
			{
				float floatData = intFloat.f;
				ui::CameraAutozoomer::setAreaZoom(ui::CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_OUTDOOR, floatData);
			}
			break;

		case GS_CMD_saveCameraAutozoom:
			if (game->inCombat)
			{
				ui::CameraAutozoomer::saveCheckpointState();
			}
			break;

		case GS_CMD_loadCameraAutozoom:
			if (game->inCombat)
			{
				ui::CameraAutozoomer::loadCheckpointState();
			}
			break;

		case GS_CMD_setCameraUpVector:
			if (stringData != NULL)
			{
				VC3 upvec = VC3(0,0,0);
				if (gs_tricoord_param(stringData, &upvec))
				{
					if (!(upvec.GetSquareLength() > 0.99f * 0.99f
						&& upvec.GetSquareLength() < 1.01f * 1.01f))
					{
						sp->warning("CameraScripting::process - setCameraUpVector parameter is not a normalized vector (will be normalized).");
					}
					if (!(upvec.GetSquareLength() > 0.001f * 0.001f))
					{
						sp->warning("CameraScripting::process - setCameraUpVector parameter is a zero length or invalid vector (falling back to default).");
						upvec = VC3(0,1,0);
					}
					upvec.Normalize();
					game->gameUI->getGameCamera()->setUpVector(upvec);
				} else {
					sp->error("CameraScripting::process - setCameraUpVector parameter invalid (expected vector in format x,y,z).");
				}
			} else {
				sp->error("CameraScripting::process - setCameraUpVector parameter missing (expected vector in format x,y,z).");
			}
			break;

		case GS_CMD_setCameraNearClipValue:
			{
				float floatData = intFloat.f;
				if( game && game->gameScene && game->gameScene->getStormScene() && game->gameScene->getStormScene()->GetCamera() )
					game->gameScene->getStormScene()->GetCamera()->SetZNear( floatData );
			}
			break;

		case GS_CMD_setCameraNearClipDefault:
			if( game && game->gameScene && game->gameScene->getStormScene() && game->gameScene->getStormScene()->GetCamera() )
				game->gameScene->getStormScene()->GetCamera()->SetZNearDefault();
			break;

		case GS_CMD_setCameraPositionOffsetXToFloatValue:
			if (game->inCombat)
			{
				VC3 tmp = game->gameUI->getGameCamera()->getPositionOffset();
				tmp.x = gsd->floatValue;
				game->gameUI->getGameCamera()->setPositionOffset(tmp);
			}
			break;

		case GS_CMD_setCameraPositionOffsetYToFloatValue:
			if (game->inCombat)
			{
				VC3 tmp = game->gameUI->getGameCamera()->getPositionOffset();
				tmp.y = gsd->floatValue;
				game->gameUI->getGameCamera()->setPositionOffset(tmp);
			}			
			break;

		case GS_CMD_setCameraPositionOffsetZToFloatValue:
			if (game->inCombat)
			{
				VC3 tmp = game->gameUI->getGameCamera()->getPositionOffset();
				tmp.z = gsd->floatValue;
				game->gameUI->getGameCamera()->setPositionOffset(tmp);
			}						
			break;

		case GS_CMD_setCameraTargetOffsetXToFloatValue:
			if (game->inCombat)
			{
				VC3 tmp = game->gameUI->getGameCamera()->getTargetOffset();
				tmp.x = gsd->floatValue;
				game->gameUI->getGameCamera()->setTargetOffset(tmp);
			}
			break;

		case GS_CMD_setCameraTargetOffsetYToFloatValue:
			if (game->inCombat)
			{
				VC3 tmp = game->gameUI->getGameCamera()->getTargetOffset();
				tmp.y = gsd->floatValue;
				game->gameUI->getGameCamera()->setTargetOffset(tmp);
			}			
			break;

		case GS_CMD_setCameraTargetOffsetZToFloatValue:
			if (game->inCombat)
			{
				VC3 tmp = game->gameUI->getGameCamera()->getTargetOffset();
				tmp.z = gsd->floatValue;
				game->gameUI->getGameCamera()->setTargetOffset(tmp);
			}						
			break;

		case GS_CMD_setCameraTargetOffsetAtPosition:
			if (game->inCombat)
			{
				VC3 targpos = game->gameUI->getGameCamera()->getTargetPosition();
				VC3 diff = gsd->position - targpos;
				game->gameUI->getGameCamera()->setTargetOffset(diff);
			}						
			break;

		case GS_CMD_setCameraPositionOffsetAtPosition:
			if (game->inCombat)
			{
				VC3 campos = game->gameUI->getGameCamera()->getPosition();
				VC3 diff = gsd->position - campos;
				game->gameUI->getGameCamera()->setPositionOffset(diff);
			}						
			break;

		case GS_CMD_disableDirectCameraControls:
			if (game->inCombat)
			{
				game->gameUI->getGameCamera()->setDirectControlsEnabled(false);
			}
			break;

		case GS_CMD_enableDirectCameraControls:
			if (game->inCombat)
			{
				game->gameUI->getGameCamera()->setDirectControlsEnabled(true);
			}						
			break;

		case GS_CMD_updateCamera:
			{
				game->gameUI->getGameCamera()->doMovement(0);
			}
			break;

		case GS_CMD_setCameraAreaType:
			{
				cameraAreaParameters.type = intData;
			}
			break;

		case GS_CMD_setCameraAreaCorner1ToPosition:
			{
				VC3 pos = gsd->position;
				cameraAreaParameters.corner[0] = pos;				
			}
			break;

		case GS_CMD_setCameraAreaCorner2ToPosition:
			{
				VC3 pos = gsd->position;
				cameraAreaParameters.corner[1] = pos;				
			}
			break;

		case GS_CMD_setCameraAreaCorner3ToPosition:
			{
				VC3 pos = gsd->position;
				cameraAreaParameters.corner[2] = pos;				
			}
			break;

		case GS_CMD_setCameraAreaCorner4ToPosition:
			{
				VC3 pos = gsd->position;
				cameraAreaParameters.corner[3] = pos;				
			}
			break;

		case GS_CMD_setCameraAreaFOV:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.FOV = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaAngle:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.angle = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaBetaAngle:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.betaAngle = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaBank:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.bank = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaOffsetX:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.offset.x = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaOffsetY:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.offset.y = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaOffsetZ:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.offset.z = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaFollowX:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.follow.x = floatData*0.01f;			
			}
			break;

		case GS_CMD_setCameraAreaFollowY:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.follow.y = floatData*0.01f;			
			}
			break;

		case GS_CMD_setCameraAreaFollowZ:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.follow.z = floatData*0.01f;			
			}
			break;

		case GS_CMD_setCameraAreaTargetToPosition:
			{
				VC3 pos = gsd->position;
				cameraAreaParameters.target = pos;				
			}
			break;

		case GS_CMD_setCameraAreaAnimation:
			{
				cameraAreaParameters.animation = stringData;				
			}
			break;

		case GS_CMD_setCameraAreaGroup:
			{
				cameraAreaParameters.group = intData;
			}
			break;

		case GS_CMD_addCameraArea:
			{
				game->getGameUI()->getCameraSystem()->addCameraArea(new CameraAreaStreet(cameraAreaParameters.name, cameraAreaParameters.group, cameraAreaParameters.collision, cameraAreaParameters.corner[0], cameraAreaParameters.corner[1], cameraAreaParameters.corner[2], cameraAreaParameters.corner[3], cameraAreaParameters.angle, cameraAreaParameters.betaAngle, cameraAreaParameters.bank, cameraAreaParameters.distance, cameraAreaParameters.FOV, cameraAreaParameters.target, cameraAreaParameters.offset, cameraAreaParameters.follow, cameraAreaParameters.animation));
//				initializeCameraAreaParameters();
			}
			break;

		case GS_CMD_setCameraAreaAngleToValue:
			{
				float floatData = (float)(*lastValue);
				cameraAreaParameters.angle = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaName:
			{
				cameraAreaParameters.name = stringData;				
			}
			break;

		case GS_CMD_setCameraAreaDistance:
			{
				float floatData = intFloat.f;
				cameraAreaParameters.distance = floatData;			
			}
			break;

		case GS_CMD_setCameraAreaCollision:
			{
				cameraAreaParameters.type = intData;
			}
			break;

		case GS_CMD_actualCameraPosition:
			gsd->position = game->gameUI->getGameCamera()->getActualInterpolatedPosition();
			break;

		case GS_CMD_moveCameraAngle:
			{
				float floatData = intFloat.f;
				game->getGameUI()->getCameraSystem()->moveCameraAngle(floatData);			
			}
			break;

		case GS_CMD_moveCameraBetaAngle:
			{
				float floatData = intFloat.f;
				game->getGameUI()->getCameraSystem()->moveCameraBetaAngle(floatData);			
			}
			break;

		case GS_CMD_moveCameraBank:
			{
				float floatData = intFloat.f;
				game->getGameUI()->getCameraSystem()->moveCameraBank(floatData);			
			}
			break;

		case GS_CMD_moveCameraDistance:
			{
				float floatData = intFloat.f;
				game->getGameUI()->getCameraSystem()->moveCameraDistance(floatData);			
			}
			break;

		case GS_CMD_moveCameraFOV:
			{
				float floatData = intFloat.f;
				game->getGameUI()->getCameraSystem()->moveCameraFOV(floatData);			
			}
			break;

		case GS_CMD_clearCameraAreas:
			{
				game->getGameUI()->getCameraSystem()->clearCameraAreas();			
			}
			break;

			
		case GS_CMD_isCameraSelected:
			if (stringData != NULL)
			{
				// oh yeah! copy & paste programming ahead...
				if (strncmp(stringData, "cinematic1", 10) == 0)
				{
					*lastValue = game->gameUI->getCameraNumber() == GAMEUI_CAMERA_CINEMATIC1 ? 1 : 0;
					break;
				}
				if (strncmp(stringData, "cinematic2", 10) == 0)
				{
					*lastValue = game->gameUI->getCameraNumber() == GAMEUI_CAMERA_CINEMATIC2 ? 1 : 0;
					break;
				}
				if (strncmp(stringData, "normal", 6) == 0)
				{
					*lastValue = game->gameUI->getCameraNumber() == GAMEUI_CAMERA_NORMAL ? 1 : 0;
					break;
				}
				if (strncmp(stringData, "tactical", 8) == 0)
				{
					*lastValue = game->gameUI->getCameraNumber() == GAMEUI_CAMERA_TACTICAL ? 1 : 0;
					break;
				}
				*lastValue = 0;
				sp->error("GameScripting::process - isCameraSelected parameter bad.");
			} else {
				*lastValue = 0;
				sp->error("GameScripting::process - isCameraSelected parameter expected.");
			}
			break;

		default:
			sp->error("CameraScripting::process - Unknown command.");
			assert(0);
		}
	}
}
