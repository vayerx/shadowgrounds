
#include "precompiled.h"

#include "CinematicScripting.h"

#include "scripting_macros_start.h"
#include "cinematic_script_commands.h"
#include "scripting_macros_end.h"

// NOTE: Some problems with the DatatypeDef.h including...
// unless this exists here, some math stuff will not be included.
// (even though this DatatypeDef is included by other headers)
#include <DatatypeDef.h>

#include "../../ui/UIEffects.h"

#include "../Game.h"
#include "../GameUI.h"
#include "GameScriptingUtils.h"
#include "../../ui/GameVideoPlayer.h"
#include "../GameScene.h"
#include "../Character.h"
#include "GameScriptData.h"
#include "../DHLocaleManager.h"
#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../../util/LipsyncManager.h"
#include <istorm3D_terrain_renderer.h>
#include "../../sound/SoundMixer.h"
#include "../game/options/options_camera.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	void CinematicScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		int intData = intFloat.i;
		switch(command)
		{

		case GS_CMD_HIDEGUI:
			// TODO: proper handling of players in netgame
			game->gameUI->setGUIVisibility(game->singlePlayerNumber, false);
			break;

		case GS_CMD_SHOWGUI:
			// TODO: proper handling of players in netgame
			game->gameUI->setGUIVisibility(game->singlePlayerNumber, true);
			break;

		case GS_CMD_FADED:
			game->gameUI->getEffects()->setDefaultFadeImageIfHitImage();
			game->gameUI->getEffects()->setFaded();
			break;

		case GS_CMD_NOFADE:
			game->gameUI->getEffects()->setDefaultFadeImageIfHitImage();
			game->gameUI->getEffects()->setNoFade();
			break;

		case GS_CMD_FADEIN:
			if (intData >= GAME_TICK_MSEC)
			{
				game->gameUI->getEffects()->setDefaultFadeImageIfHitImage();
				game->gameUI->getEffects()->startFadeIn(intData);
			} else {
				sp->error("CinematicScripting::process - fadeIn parameter invalid, fade-in duration in msec expected.");
			}
			break;

		case GS_CMD_FADEOUT:
			if (intData >= GAME_TICK_MSEC)
			{
				game->gameUI->getEffects()->setDefaultFadeImageIfHitImage();
				game->gameUI->getEffects()->startFadeOut(intData);
			} else {
				sp->error("CinematicScripting::process - fadeOut parameter invalid, fade-out duration in msec expected.");
			}
			break;

		case GS_CMD_SETFADEIMAGE:
			if (stringData != NULL)
			{
				game->gameUI->getEffects()->setFadeImageFilename(stringData);
			} else {
				sp->error("CinematicScripting::process - setFadeImage parameter missing, fade image filename expected.");
			}
			break;

		case GS_CMD_SETFADEIMAGETODEFAULT:
			game->gameUI->getEffects()->setDefaultFadeImage();
			break;

		case GS_CMD_STARTCINEMATIC:
			if(!game->gameUI->pushUIState())
			{
				sp->error("GameScripting::process - startCinematic failed");
			}
			break;

		case GS_CMD_ENDCINEMATIC:
			if(!game->gameUI->popUIState())
			{
				sp->error("GameScripting::process - endCinematic failed");
			}
			break;

		case GS_CMD_CINEMATICSCRIPT:
			if (stringData != NULL)
			{
				game->setCinematicScriptProcess(stringData);
				game->gameUI->setPointersChangedFlag(game->singlePlayerNumber);
			} else {
				sp->error("GameScripting::process - cinematicScript parameter missing.");
			}
			break;

		case GS_CMD_MOVIEASPECTRATIO:
			{				
				Terrain *t = game->gameUI->getTerrain();
				if(t && t->GetTerrain())
				{
#ifdef PROJECT_SURVIVOR
					// fade
					game->gameUI->setMovieAspectRatio(true);
#else
					t->GetTerrain()->getRenderer().setMovieAspectRatio(true, false);
#endif
				}
				//game->gameUI->getEffects()->setMovieAspectRatio(true);
				float fovFactor = SimpleOptions::getFloat(DH_OPT_F_CAMERA_MOVIE_FOV_FACTOR);
				GameCamera::setFOVFactor(fovFactor);
			}
			break;

		case GS_CMD_NORMALASPECTRATIO:
			{
				Terrain *t = game->gameUI->getTerrain();
				if(t && t->GetTerrain())
				{
#ifdef PROJECT_SURVIVOR
					// fade
					game->gameUI->setMovieAspectRatio(false);
#else
					t->GetTerrain()->getRenderer().setMovieAspectRatio(false, false);
#endif
				}
				//game->gameUI->getEffects()->setMovieAspectRatio(false);
				float fovFactor = SimpleOptions::getFloat(DH_OPT_F_CAMERA_NORMAL_FOV_FACTOR);
				GameCamera::setFOVFactor(fovFactor);
			}
			break;

		case GS_CMD_SETMASKPICTURE:
			if (stringData != NULL)
			{
				game->gameUI->getEffects()->setMaskPicture(stringData);
			} else {
				sp->error("GameScripting::process - setMaskPicture parameter missing.");
			}
			break;

		case GS_CMD_STARTMASKPICTUREFADEIN:
			if (intData > 0)
			{
				game->gameUI->getEffects()->startMaskPictureFadeIn(intData);
			} else {
				sp->error("GameScripting::process - startMaskPictureFadeIn parameter out of range, fade-in duration expected.");
			}
			break;

		case GS_CMD_STARTMASKPICTUREFADEOUT:
			if (intData > 0)
			{
				game->gameUI->getEffects()->startMaskPictureFadeOut(intData);
			} else {
				sp->error("GameScripting::process - startMaskPictureFadeOut parameter out of range, fade-out duration expected.");
			}
			break;

		case GS_CMD_STARTMASKPICTUREMOVEIN:
			if (intData > 0)
			{
				game->gameUI->getEffects()->startMaskPictureMoveIn(intData);
			} else {
				sp->error("GameScripting::process - startMaskPictureMoveIn parameter out of range, move-in duration (msec) expected.");
			}
			break;

		case GS_CMD_STARTMASKPICTUREMOVEOUT:
			if (intData > 0)
			{
				game->gameUI->getEffects()->startMaskPictureMoveOut(intData);
			} else {
				sp->error("GameScripting::process - startMaskPictureMoveOut parameter out of range, move-out duration (msec) expected.");
			}
			break;

		case GS_CMD_SETMASKPICTUREPOSITION:
			game->gameUI->getEffects()->setMaskPicturePosition(intData);
			break;

		case GS_CMD_SETMASKPICTUREPOSITIONX:
			game->gameUI->getEffects()->setMaskPicturePositionX(intData);
			break;

		case GS_CMD_SETMASKPICTUREPOSITIONY:
			game->gameUI->getEffects()->setMaskPicturePositionY(intData);
			break;

		case GS_CMD_SETMASKPICTUREPOSITIONXTOVALUE:
			game->gameUI->getEffects()->setMaskPicturePositionX(*lastValue);
			break;

		case GS_CMD_SETMASKPICTUREPOSITIONYTOVALUE:
			game->gameUI->getEffects()->setMaskPicturePositionY(*lastValue);
			break;

		case GS_CMD_CLEARMASKPICTURE:
			game->gameUI->getEffects()->clearMaskPicture();
			break;

		case GS_CMD_SETMASKPICTURESIZEX:
			game->gameUI->getEffects()->setMaskPictureSizeX(intData);
			break;

		case GS_CMD_SETMASKPICTURESIZEY:
			game->gameUI->getEffects()->setMaskPictureSizeY(intData);
			break;

		case GS_CMD_SETMASKPICTURESIZEXTOVALUE:
			game->gameUI->getEffects()->setMaskPictureSizeX(*lastValue);
			break;

		case GS_CMD_SETMASKPICTURESIZEYTOVALUE:
			game->gameUI->getEffects()->setMaskPictureSizeY(*lastValue);
			break;

		case GS_CMD_SETACTIVEMASKPICTURELAYER:
			game->gameUI->getEffects()->setActiveMaskPictureLayer(intData);
			break;

		case GS_CMD_ISSKIPPINGCINEMATIC:
			if (game->isSkippingCinematic())
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_ISCINEMATICSCRIPTRUNNING:
			if (game->isCinematicScriptRunning())
			{
				*lastValue = 1;
			} else {
				*lastValue = 0;
			}
			break;

		case GS_CMD_SETMASKPICTUREFONT:
			if (stringData != NULL)
			{
				game->gameUI->getEffects()->setMaskPictureFont(convertLocaleSubtitleString(stringData));
			} else {
				sp->warning("CinematicScripting::process - setMaskPictureFont parameter missing.");
			}
			break;

		case GS_CMD_SETMASKPICTURETEXT:
			if (stringData != NULL)
			{
				game->gameUI->getEffects()->setMaskPictureText(convertLocaleSubtitleString(stringData));
			} else {
				sp->warning("CinematicScripting::process - setMaskPictureText parameter missing.");
			}
			break;

		case GS_CMD_ADDMASKPICTURETEXTLINE:
			if (stringData != NULL)
			{
				game->gameUI->getEffects()->addMaskPictureTextLine(convertLocaleSubtitleString(stringData));
			} else {
				sp->warning("CinematicScripting::process - addMaskPictureText parameter missing.");
			}
			break;

		case GS_CMD_CLEARMASKPICTURETEXT:
			game->gameUI->getEffects()->clearMaskPictureText();
			break;

		case GS_CMD_SETMASKPICTURETEXTPOSITIONX:
			game->gameUI->getEffects()->setMaskPictureTextPositionX(intData);
			break;

		case GS_CMD_SETMASKPICTURETEXTPOSITIONY:
			game->gameUI->getEffects()->setMaskPictureTextPositionY(intData);
			break;

		case GS_CMD_SETMASKPICTURETEXTAREASIZEX:
			game->gameUI->getEffects()->setMaskPictureTextAreaSizeX(intData);
			break;

		case GS_CMD_SETMASKPICTURETEXTAREASIZEY:
			game->gameUI->getEffects()->setMaskPictureTextAreaSizeY(intData);
			break;

		case GS_CMD_SETMASKPICTURETEXTTOSTRING:
			if (gsd->stringValue != NULL)
			{
				game->gameUI->getEffects()->setMaskPictureText(convertLocaleSubtitleString(gsd->stringValue));
			} else {
				sp->warning("CinematicScripting::process - Attempt to setMaskPictureTextToString for null string value.");
			}
			break;

		case GS_CMD_ENABLESCROLLY:
			game->gameUI->setScrollyEnabled(true);
			break;

		case GS_CMD_DISABLESCROLLY:
			game->gameUI->setScrollyEnabled(false);
			break;

		case GS_CMD_PLAYVIDEO:
			if (stringData != NULL)
			{
				//float volume = game->gameUI->getVideoVolume();
				
				IStorm3D_StreamBuilder *builder = 0;
				sfx::SoundMixer *mixer = game->gameUI->getSoundMixer();
				if(mixer)
					builder = mixer->getStreamBuilder();

				ui::GameVideoPlayer::playVideo(game->getGameScene()->getStormScene(), stringData, builder);
			} else {
				sp->warning("CinematicScripting::process - playVideo parameter missing.");
			}
			break;

		case GS_CMD_DISABLEPLAYERSELFILLUMINATION:
			game->gameUI->setPlayerSelfIlluminationEnabled(false);
			break;

		case GS_CMD_ENABLEPLAYERSELFILLUMINATION:
			game->gameUI->setPlayerSelfIlluminationEnabled(true);
			break;

		case GS_CMD_ENDCONVERSATION:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				std::string str = "";
				game->gameUI->getLipsyncManager()->setCharacter(util::LipsyncManager::Left, str);
				game->gameUI->getLipsyncManager()->setCharacter(util::LipsyncManager::Right, str);
				game->gameUI->getLipsyncManager()->setActive(false);
			} else {
				sp->warning("CinematicScripting::process - Attempt to call endConversation when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_STARTCONVERSATION:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				game->gameUI->getLipsyncManager()->setActive(true);
			} else {
				sp->warning("CinematicScripting::process - Attempt to call startConversation when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_SETCONVERSATIONUNITLEFT:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (gsd->unit->getCharacter() != NULL)
					{
						if (gsd->unit->getCharacter()->getLipsyncId() != NULL)
						{
							std::string str = std::string(gsd->unit->getCharacter()->getLipsyncId());
							game->gameUI->getLipsyncManager()->setCharacter(util::LipsyncManager::Left, str);
						} else {
							sp->warning("CinematicScripting::process - Attempt to call setConversationUnitLeft for character with no lipsync id.");
						}
					} else {
						sp->warning("CinematicScripting::process - Attempt to call setConversationUnitLeft for unit with no character.");
					}
				} else {
					sp->warning("CinematicScripting::process - Attempt to call setConversationUnitLeft for null unit.");
				}
			} else {
				sp->warning("CinematicScripting::process - Attempt to call setConversationUnitLeft when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_SETCONVERSATIONUNITRIGHT:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (gsd->unit->getCharacter() != NULL)
					{
						if (gsd->unit->getCharacter()->getLipsyncId() != NULL)
						{
							std::string str = std::string(gsd->unit->getCharacter()->getLipsyncId());
							game->gameUI->getLipsyncManager()->setCharacter(util::LipsyncManager::Right, str);
						} else {
							sp->warning("CinematicScripting::process - Attempt to call setConversationUnitRight for character with no lipsync id.");
						}
					} else {
						sp->warning("CinematicScripting::process - Attempt to call setConversationUnitRight for unit with no character.");
					}
				} else {
					sp->warning("CinematicScripting::process - Attempt to call setConversationUnitRight for null unit.");
				}
			} else {
				sp->warning("CinematicScripting::process - Attempt to call setConversationUnitRight when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_SETCONVERSATIONIDLEANIMATIONFORUNIT:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (gsd->unit->getCharacter() != NULL)
					{
						if (stringData != NULL)
						{
							if (gsd->unit->getCharacter()->getLipsyncId() != NULL)
							{
								std::string str = std::string(gsd->unit->getCharacter()->getLipsyncId());
								std::string strval = std::string(stringData);
								game->gameUI->getLipsyncManager()->setIdle(str, strval);
							} else {
								sp->warning("CinematicScripting::process - Attempt to setConversationIdleAnimationForUnit for character with no lipsync id.");
							}
						} else {
							sp->warning("CinematicScripting::process - setConversationIdleAnimationForUnit parameter missing.");
						}
					} else {
						sp->warning("CinematicScripting::process - Attempt to call setConversationIdleAnimationForUnit for unit with no character.");
					}
				} else {
					sp->warning("CinematicScripting::process - Attempt to call setConversationIdleAnimationForUnit for null unit.");
				}
			} else {
				sp->warning("CinematicScripting::process - Attempt to call setConversationIdleAnimationForUnit when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_SETCONVERSATIONEXPRESSIONANIMATIONFORUNIT:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (gsd->unit->getCharacter() != NULL)
					{
						if (stringData != NULL)
						{
							if (gsd->unit->getCharacter()->getLipsyncId() != NULL)
							{
								std::string str = std::string(gsd->unit->getCharacter()->getLipsyncId());
								std::string strval = std::string(stringData);
								game->gameUI->getLipsyncManager()->setExpression(str, strval);
							} else {
								sp->warning("CinematicScripting::process - Attempt to setConversationExpressionAnimationForUnit for character with no lipsync id.");
							}
						} else {
							sp->warning("CinematicScripting::process - setConversationExpressionAnimationForUnit parameter missing.");
						}
					} else {
						sp->warning("CinematicScripting::process - Attempt to call setConversationExpressionAnimationForUnit for unit with no character.");
					}
				} else {
					sp->warning("CinematicScripting::process - Attempt to call setConversationExpressionAnimationForUnit for null unit.");
				}
			} else {
				sp->warning("CinematicScripting::process - Attempt to call setConversationExpressionAnimationForUnit when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_setConversationNoiseLeft:
			{
				game->gameUI->setConversationNoise(0, intData);
			}
			break;

		case GS_CMD_setConversationNoiseRight:
			{
				game->gameUI->setConversationNoise(1, intData);
			}
			break;

		case GS_CMD_setConversationNoiseForUnit:
			if (game->gameUI->getLipsyncManager() != NULL)
			{
				if (gsd->unit != NULL)
				{
					if (gsd->unit->getCharacter() != NULL)
					{
						if (gsd->unit->getCharacter()->getLipsyncId() != NULL)
						{
							std::string str = std::string(gsd->unit->getCharacter()->getLipsyncId());
							int side = -1;
							if(game->gameUI->getLipsyncManager()->getCharacter(util::LipsyncManager::Left) == str)
								side = 0;
							else if(game->gameUI->getLipsyncManager()->getCharacter(util::LipsyncManager::Right) == str)
								side = 1;

							if(side >= 0)
								game->gameUI->setConversationNoise(side, intData);
							else
								sp->warning("CinematicScripting::process - Attempt to setConversationNoiseForUnit for unit which is not active.");
						} else {
							sp->warning("CinematicScripting::process - Attempt to setConversationNoiseForUnit for character with no lipsync id.");
						}
					} else {
						sp->warning("CinematicScripting::process - Attempt to call setConversationNoiseForUnit for unit with no character.");
					}
				} else {
					sp->warning("CinematicScripting::process - Attempt to call setConversationNoiseForUnit for null unit.");
				}
			} else {
				sp->warning("CinematicScripting::process - Attempt to call setConversationNoiseForUnit when no lipsync manager available (not in combat).");
			}
			break;

		case GS_CMD_openCinematicScreen:
			{
				if( game && game->gameUI && ( stringData != NULL ) )
				{
					game->gameUI->openCinematicScreen( stringData );
				}
			}
			break;

		case GS_CMD_closeCinematicScreen:
			{
				if( game && game->gameUI )
				{
					game->gameUI->closeCinematicScreen();
				}
			}
			break;

		case GS_CMD_isCinematicScreenOpen:
			{
				*lastValue = game->gameUI->isCinematicScreenOpen()?1:0;
			}
			break;

		case GS_CMD_setVehicleGUI:
			{
				if(stringData != NULL)
				{
					if(strcmp(stringData, "disabled") == 0)
					{
						game->gameUI->closeVehicleGUI();
					}
					else
					{
						game->gameUI->openVehicleGUI(stringData);
					}					
				}
				else
				{
					sp->warning("CinematicScripting::process - setVehicleGUI parameter missing.");
				}
			}
			break;

		case GS_CMD_setNVGoggleEnabled:
			{
				game->gameUI->getEffects()->setNVGoggleEffect(intData == 0 ? false : true);
			}
			break;

		case GS_CMD_openCharacterSelectionWindow:
			if (stringData != NULL)
			{
				game->gameUI->openCharacterSelectionWindow(stringData);
			} else {
				sp->warning("CinematicScripting::process - openCharacterSelectionWindow parameter missing.");
			}
			break;

		case GS_CMD_setSlomoEffectEnabled:
			{
				game->gameUI->getEffects()->setSlomoEffect(intData == 0 ? false : true);
			}
			break;

		case GS_CMD_setNapalmEffectEnabled:
			{
				game->gameUI->getEffects()->setNapalmEffect(intData == 0 ? false : true);
			}
			break;

		case GS_CMD_setHealEffectEnabled:
			{
				game->gameUI->getEffects()->setHealEffect(intData == 0 ? false : true);
			}
			break;

		case GS_CMD_setCustomFilterEffect:
			if (stringData != NULL)
			{
				const char *parse_error = "CinematicScripting::process - setCustomFilterEffect expects format \"brightness,contrast,red,green,blue\" or \"disabled\"";
				if(strcmp(stringData, "disabled") == 0)
				{
					game->gameUI->getEffects()->setCustomFilterEffect(false,0,0,0,0,0);
					break;
				}
				int br, co, r, g, b;
				if(sscanf(stringData, "%i,%i,%i,%i,%i", &br, &co, &r, &g, &b) != 5)
				{
					sp->error(parse_error);
					break;
				}
				game->gameUI->getEffects()->setCustomFilterEffect(true, br, co, r, g, b);
			}
			else
			{
				sp->error("CinematicScripting::process - setCustomFilterEffect parameter missing.");
			}
			break;

		case GS_CMD_waitUntilCinematicScreenClosed:
			{
				gsd->waitCinematicScreen = true;
				gsd->waitCounter = 1;
				*pause = true;
			}
			break;

		case GS_CMD_openMissionFailureWindow:
			{
				game->gameUI->openMissionFailureWindow();
			}
			break;

		case GS_CMD_isCharacterSelectionOpen:
			{
				*lastValue = game->gameUI->isCharacterSelectionWindowOpen() ? 1 : 0;
			}
			break;

		default:
			sp->error("CinematicScripting::process - Unknown command.");
			assert(0);
		}
	}
}

