
#include "precompiled.h"

#include "SoundScripting.h"

#include "scripting_macros_start.h"
#include "sound_script_commands.h"
#include "scripting_macros_end.h"

// NOTE: Some problems with the DatatypeDef.h including...
// unless this exists here, some math stuff will not be included.
// (even though this DatatypeDef is included by other headers)
#include <DatatypeDef.h>

#include "../Game.h"
#include "../GameUI.h"
#include "../Unit.h"
#include "../UnitType.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"

#include "../DHLocaleManager.h"

#include "../../sound/MusicPlaylist.h"
#include "../../sound/playlistdefs.h"
#include "../../sound/sounddefs.h"
#include "../../sound/AmbientAreaManager.h"
#include "../../sound/SoundMixer.h"
#include "../../sound/SoundLib.h"

#include "../SimpleOptions.h"
#include "../options/options_sounds.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../../game/physics/PhysicsContactSoundManager.h"

#include "../../ui/AmbientSoundManager.h"

#include "../../util/Debug_MemoryManager.h"
#include "../../util/StringUtil.h"


using namespace ui;

namespace game
{
  extern PhysicsContactSoundManager *gameui_physicsSoundsManager;

	int SoundScripting::soundScriptVolume = DEFAULT_SOUND_SCRIPT_VOLUME;


	void SoundScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_PLAYSOUNDEFFECT:
			if (stringData != NULL)
			{
				VC3 pos = gsd->position;
				float range = DEFAULT_SOUND_RANGE;
				if(gsd->originalUnit)
					range = gsd->originalUnit->getUnitType()->getSoundRange();

				game->gameUI->playSoundEffect(stringData, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, range, DEFAULT_SOUND_PRIORITY_NORMAL);
			} else {
				sp->warning("SoundScripting::process - playSoundEfffect parameter missing, filename expected.");
			}
			break;

		case GS_CMD_PLAYSTREAMEDSOUND:
			if (stringData != NULL)
			{
				game->gameUI->playStreamedSound(stringData);
			} else {
				sp->warning("SoundScripting::process - playStreamedSound parameter missing, filename expected.");
			}
			break;

		case GS_CMD_STOPALLSTREAMEDSOUNDS:
			game->gameUI->stopAllStreamedSounds();
			break;

		case GS_CMD_STOPALLSPEECHES:
			if(game->gameUI->getSoundMixer())
				game->gameUI->getSoundMixer()->stopAllSpeech();
			break;

		case GS_CMD_PLAYSPEECH:
			if (stringData != NULL)
			{
				VC3 pos = gsd->position;
				game->gameUI->playSpeech(convertLocaleSpeechString(stringData), pos.x, pos.y, pos.z, false, DEFAULT_SPEECH_VOLUME);
			} else {
				sp->warning("SoundScripting::process - playSpeech parameter missing, filename expected.");
			}
			break;

		case GS_CMD_SETMUSIC:
			if (stringData != NULL)
			{
				game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->playFile(stringData);
			} else {
				sp->warning("SoundScripting::process - setMusic parameter missing.");
			}
			break;

		case GS_CMD_LOADMUSICPLAYLIST:
			if (stringData != NULL)
			{
				bool paramok = false;
				for (int i = 0; i < PLAYLIST_AMOUNT; i++)
				{
					int pllen = strlen(playlistNames[i]);
					if (strncmp(stringData, playlistNames[i], pllen) == 0
						&& stringData[pllen] == ',')
					{
						game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->loadPlaylist(i, &stringData[pllen + 1]);
						paramok = true;
						break;
					}
				}
				if (!paramok)
				{
					sp->warning("SoundScripting::process - loadMusicPlaylist parameter invalid.");
				}
			} else {
				sp->warning("SoundScripting::process - loadMusicPlaylist parameter missing.");
			}
			break;

		case GS_CMD_STOPMUSIC:
			game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->stop();
			break;

		case GS_CMD_PLAYMUSIC:
			game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->play();
			break;

		case GS_CMD_NEXTMUSIC:
			game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->nextTrack();
			break;

		case GS_CMD_PREVIOUSMUSIC:
			game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->previousTrack();
			break;

		case GS_CMD_MUSICPLAYLIST:
			if (stringData != NULL)
			{
				bool paramok = false;
				for (int i = 0; i < PLAYLIST_AMOUNT; i++)
				{
					if (strcmp(stringData, playlistNames[i]) == 0)
					{
						game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->setBank(i);
						paramok = true;
						break;
					}
				}
				if (!paramok)
				{
					sp->warning("SoundScripting::process - musicPlaylist parameter invalid.");
				}
			} else {
				sp->warning("SoundScripting::process - musicPlaylist parameter missing.");
			}
			break;

		case GS_CMD_CLEARALLAMBIENTSOUNDS:
			game->gameUI->getAmbientSoundManager()->clearAllAmbientSounds();
			break;

		case GS_CMD_SETAMBIENTSOUNDNUMBER:
			//sp->debug("setAmbientSoundNumber");
			game->gameUI->getAmbientSoundManager()->setSelectedAmbientSound(intData);
			break;

		case GS_CMD_setAmbientSoundNumberByValue:
			game->gameUI->getAmbientSoundManager()->setSelectedAmbientSound(*lastValue);
			break;

		case GS_CMD_SETNEXTFREEAMBIENTSOUNDNUMBER:
				//sp->debug("setAmbientSoundNumber");
				game->gameUI->getAmbientSoundManager()->setNextFreeAmbientSound();
			break;

		case GS_CMD_SETAMBIENTSOUNDNUMBERBYNAME:
			if (stringData != NULL)
			{
				game->gameUI->getAmbientSoundManager()->selectAmbientSoundByName(stringData);
			} else {
				sp->error("SoundScripting::process - setAmbientSoundNumberByName parameter missing.");
			}
			break;

		case GS_CMD_SETAMBIENTSOUNDRANGE:
			{
				//sp->debug("setAmbientSoundRange");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->setAmbientSoundRange(asman->getSelectedAmbientSound(), (float)intData);
			}
			break;

		case GS_CMD_SETAMBIENTSOUNDCLIPTOVALUE:
			{
				if (intData <= 0 || intData > 15)
				{
					sp->warning("SoundScripting::process - setAmbientSoundClipToValue parameter out of expected range (probably an erronous value).");
				}
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->setAmbientSoundClip(asman->getSelectedAmbientSound(), intData);
			}
			break;

		case GS_CMD_SETAMBIENTSOUNDNAME:
			if (stringData != NULL)
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->setAmbientSoundName(asman->getSelectedAmbientSound(), stringData);
			} else {
				sp->error("SoundScripting::process - setAmbientSoundName parameter missing.");
			}
			break;

		case GS_CMD_SETAMBIENTSOUNDROLLOFF:
			{
				//sp->debug("setAmbientSoundRollOff");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();			 
				asman->setAmbientSoundRollOff(asman->getSelectedAmbientSound(), intData);
			}
			break;

		case GS_CMD_MAKEAMBIENTSOUND:
			if (stringData != NULL)
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->makeAmbientSoundFromDefString(asman->getSelectedAmbientSound(), stringData);
				asman->setAmbientSoundPosition(asman->getSelectedAmbientSound(), gsd->position);
			} else {
				sp->error("SoundScripting::process - makeAmbientSound parameter missing, sound definition string expected.");
			}
			break;

		case GS_CMD_STARTAMBIENTSOUNDBYNUMBER:
			{
				//sp->debug("startAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->startAmbientSound(intData);
			}
			break;

		case GS_CMD_STOPAMBIENTSOUNDBYNUMBER:
			{
				//sp->debug("stopAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->stopAmbientSound(intData, false);
			}
			break;

		case GS_CMD_startAmbientSoundByNumberValue:
			{
				//sp->debug("startAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->startAmbientSound(*lastValue);
			}
			break;

		case GS_CMD_stopAmbientSoundByNumberValue:
			{
				//sp->debug("stopAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->stopAmbientSound(*lastValue, false);
			}
			break;

		case GS_CMD_getAmbientSoundNumber:
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				*lastValue = asman->getSelectedAmbientSound();
			}
			break;

		case GS_CMD_STOPAMBIENTSOUNDIMMEDIATELYBYNUMBER:
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->stopAmbientSound(intData, true);
			}
			break;

		case GS_CMD_STARTSELECTEDAMBIENTSOUND:
			{
				//sp->debug("startAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->startAmbientSound(asman->getSelectedAmbientSound());
			}
			break;

		case GS_CMD_STOPSELECTEDAMBIENTSOUND:
			{
				//sp->debug("stopAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->stopAmbientSound(asman->getSelectedAmbientSound(), false);
			}
			break;

		case GS_CMD_STOPSELECTEDAMBIENTSOUNDIMMEDIATELY:
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				asman->stopAmbientSound(asman->getSelectedAmbientSound(), true);
			}
			break;

		case GS_CMD_STARTAMBIENTSOUNDBYNAME:
			if (stringData != NULL)
			{
				//sp->debug("startAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				int num = asman->getAmbientSoundNumberByName(stringData);
				if (num != -1)
				{
					asman->startAmbientSound(num);
				}
			} else {
				sp->error("SoundScripting::process - startAmbientSoundByName parameter missing, ambient sound name expected.");
			}
			break;

		case GS_CMD_STOPAMBIENTSOUNDBYNAME:
			if (stringData != NULL)
			{
				//sp->debug("stopAmbientSound");
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				int num = asman->getAmbientSoundNumberByName(stringData);
				if (num != -1)
				{
					asman->stopAmbientSound(num, false);
				}
			} else {
				sp->error("SoundScripting::process - stopAmbientSoundByName parameter missing, ambient sound name expected.");
			}
			break;

		case GS_CMD_STOPAMBIENTSOUNDIMMEDIATELYBYNAME:
			if (stringData != NULL)
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();
				int num = asman->getAmbientSoundNumberByName(stringData);
				if (num != -1)
				{
					asman->stopAmbientSound(num, true);
				}
			} else {
				sp->error("SoundScripting::process - stopAmbientSoundImmediatelyByName parameter missing, ambient sound name expected.");
			}
			break;

		case GS_CMD_SETSOUNDVOLUME:
			SoundScripting::soundScriptVolume = intData;
			if (SoundScripting::soundScriptVolume < 0)
				SoundScripting::soundScriptVolume = 0;
			if (SoundScripting::soundScriptVolume > 100)
				SoundScripting::soundScriptVolume = 100;
			break;

		case GS_CMD_PLAYSOUNDEFFECTWITHVOLUME:
			if (stringData != NULL)
			{
				VC3 pos = gsd->position;
				float range = DEFAULT_SOUND_RANGE;
				if(gsd->originalUnit)
					range = gsd->originalUnit->getUnitType()->getSoundRange();

				game->gameUI->playSoundEffect(stringData, pos.x, pos.y, pos.z, false, SoundScripting::soundScriptVolume, range, DEFAULT_SOUND_PRIORITY_NORMAL);
			} else {
				sp->error("SoundScripting::process - playSoundEffectWithVolume parameter missing, sound filename expected.");
			}
			break;

		case GS_CMD_PLAYSPEECHWITHVOLUME:
			if (stringData != NULL)
			{
				VC3 pos = gsd->position;
				game->gameUI->playSpeech(stringData, pos.x, pos.y, pos.z, false, SoundScripting::soundScriptVolume);
			} else {
				sp->error("SoundScripting::process - playSpeechWithVolume parameter missing, sound filename expected.");
			}
			break;

		case GS_CMD_PRELOADSOUND:
			if (stringData != NULL)
			{
				if (SimpleOptions::getBool(DH_OPT_B_SOUND_PRELOAD))
				{
					// NOTE: this actually assumes all preloaded sounds use the speech locale (in other words, the preloaded
					// sounds that are localized, are all speeches)
					// (that should not be a problem though, sound effects are not localized or they would use the speech locale)
					game->gameUI->preloadSound(convertLocaleSpeechString(stringData), false);
				}
			} else {
				sp->error("SoundScripting::process - preloadSound parameter missing, sound filename expected.");
			}
			break;

		case GS_CMD_PRELOADTEMPORARYSOUND:
			if (stringData != NULL)
			{
				if (SimpleOptions::getBool(DH_OPT_B_SOUND_PRELOAD))
				{
					// NOTE: this actually assumes all preloaded sounds use the speech locale (in other words, the preloaded
					// sounds that are localized, are all speeches)
					// (that should not be a problem though, sound effects are not localized or they would use the speech locale)
					game->gameUI->preloadSound(convertLocaleSpeechString(stringData), true);
				}
			} else {
				sp->error("SoundScripting::process - preloadTemporarySound parameter missing, sound filename expected.");
			}
			break;

		case GS_CMD_CLEANSOUNDCACHE:
			game->gameUI->cleanSoundCache(false);
			break;

		case GS_CMD_CLEANTEMPORARYSOUNDCACHE:
			game->gameUI->cleanSoundCache(true);
			break;

		case GS_CMD_SETMUSICFADETIME:
			game->gameUI->getMusicPlaylist(game->singlePlayerNumber)->setMusicFadeTime(intData);
			break;

		case GS_CMD_LOADAMBIENTPLAYLIST:
			if (stringData != NULL)
			{
				game->gameUI->getAmbientAreaManager()->loadList(std::string(stringData));
			} else {
				sp->error("SoundScripting::process - loadAmbientPlaylist parameter missing, playlist filename expected.");
			}
			break;

		case GS_CMD_PLAYAMBIENTAREASOUNDS:
			game->gameUI->getAmbientAreaManager()->fadeIn();
			break;

		case GS_CMD_FADEOUTAMBIENTAREASOUNDS:
			game->gameUI->getAmbientAreaManager()->fadeOut(intData);
			break;

		case GS_CMD_SETAMBIENTOUTDOORAREA:
			if (stringData != NULL)
			{
				game->gameUI->getAmbientAreaManager()->setArea(sfx::AmbientAreaManager::Outdoor, std::string(stringData));
			} else {
				sp->error("SoundScripting::process - setAmbientOutdoorArea parameter missing.");
			}
			break;

		case GS_CMD_SETAMBIENTINDOORAREA:
			if (stringData != NULL)
			{
				game->gameUI->getAmbientAreaManager()->setArea(sfx::AmbientAreaManager::Indoor, std::string(stringData));
			} else {
				sp->error("SoundScripting::process - setAmbientIndoorArea parameter missing.");
			}
			break;

		case GS_CMD_SETAMBIENTOUTDOORAREABYSTRINGVALUE:
			if (gsd->stringValue != NULL)
			{
				game->gameUI->getAmbientAreaManager()->setArea(sfx::AmbientAreaManager::Outdoor, std::string(gsd->stringValue));
			} else {
				sp->error("SoundScripting::process - setAmbientOutdoorAreaByValue parameter missing.");
			}
			break;

		case GS_CMD_SETAMBIENTINDOORAREABYSTRINGVALUE:
			if (gsd->stringValue != NULL)
			{
				game->gameUI->getAmbientAreaManager()->setArea(sfx::AmbientAreaManager::Indoor, std::string(gsd->stringValue));
			} else {
				sp->error("SoundScripting::process - setAmbientIndoorAreaByValue parameter missing.");
			}
			break;

		case GS_CMD_ENABLEAMBIENTAREASOUNDS:
			game->gameUI->getAmbientAreaManager()->enableAmbient();
			break;

		case GS_CMD_DISABLEAMBIENTAREASOUNDS:
			game->gameUI->getAmbientAreaManager()->disableAmbient();
			break;

		case GS_CMD_setEaxOutdoorArea:
			if(stringData)
				game->gameUI->getAmbientAreaManager()->setEaxArea(sfx::AmbientAreaManager::Outdoor, std::string(stringData));
			else
				sp->error("SoundScripting::process - setEaxOutdoorArea parameter missing.");
			break;

		case GS_CMD_setEaxIndoorArea:
			if(stringData)
				game->gameUI->getAmbientAreaManager()->setEaxArea(sfx::AmbientAreaManager::Indoor, std::string(stringData));
			else
				sp->error("SoundScripting::process - setEaxIndoorArea parameter missing.");
			break;

		case GS_CMD_setEaxOutdoorAreaByStringValue:
			if(gsd->stringValue)
				game->gameUI->getAmbientAreaManager()->setEaxArea(sfx::AmbientAreaManager::Outdoor, std::string(gsd->stringValue));
			else
				sp->error("SoundScripting::process - setEaxOutdoorAreaByStringValue parameter missing.");
			break;

		case GS_CMD_setEaxIndoorAreaByStringValue:
			if(gsd->stringValue)
				game->gameUI->getAmbientAreaManager()->setEaxArea(sfx::AmbientAreaManager::Indoor, std::string(gsd->stringValue));
			else
				sp->error("SoundScripting::process - setEaxIndoorAreaByStringValue parameter missing.");
			break;

		case GS_CMD_getWaveLength:
			if( stringData )
			{
				if( game && game->gameUI && game->gameUI->getSoundMixer() )
				{
					sfx::SoundMixer* mixer = game->gameUI->getSoundMixer();
					sfx::SoundSample* sample = mixer->loadSample( convertLocaleSpeechString(stringData), true );
					if( sample )
					{
						int length = sample->data->getLength();
						(*lastValue) = length;
					}
					else
					{
						sp->error( "SoundScripting::process - getWaveLength could not load soundsample." );
						*lastValue = 0;
					}
				}
				else
				{
					sp->error("SoundScripting::process - getWaveLength SoundMixer is NULL." );
					*lastValue = 0;
				}
			}
			else
			{
				sp->error("SoundScripting::process - getWaveLength parameter missing." );
			}
			break;

		case GS_CMD_reloadSoundMaterials:
			if (gameui_physicsSoundsManager != NULL)
			{
				gameui_physicsSoundsManager->reloadConfiguration();
			}
			break;

		case GS_CMD_setAmbientSoundVolume:
			{
				ui::AmbientSoundManager *asman = game->gameUI->getAmbientSoundManager();			 
				asman->setAmbientSoundVolume(asman->getSelectedAmbientSound(), intData);
			}
			break;
		case GS_CMD_playSoundEffectFromStringValue:
			{
				const char* parse_error = "SoundScripting::process - Parse error: string value must be in format \"file,loop,volume,range\".";

				if(gsd->stringValue == NULL)
				{
					sp->error(parse_error);
					sp->debug(gsd->stringValue);
					break;
				}

				std::vector < std::string > splitParams = util::StringSplit( "," , gsd->stringValue);
				if(splitParams.size() != 4)
				{
					sp->error(parse_error);
					sp->debug(gsd->stringValue);
					break;
				}

				bool loop = str2int(splitParams[1].c_str()) == 0 ? false : true;
				int volume = str2int(splitParams[2].c_str());
				int range = str2int(splitParams[3].c_str());

				VC3 pos = gsd->position;
				game->gameUI->playSoundEffect(splitParams[0].c_str(), pos.x, pos.y, pos.z, loop, volume, (float)range, DEFAULT_SOUND_PRIORITY_NORMAL);
			}
			break;

		default:
			sp->error("SoundScripting::process - Unknown command.");
			assert(0);
		}
	}
}


