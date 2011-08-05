
#include "precompiled.h"

#include "SoundLooper.h"
#include "sounddefs.h"

#include <assert.h>
#include "../system/Timer.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

#define PLAYPHASE_NONE 0
#define PLAYPHASE_INIT 1
#define PLAYPHASE_START 2
#define PLAYPHASE_LOOP 3
#define PLAYPHASE_END 4

namespace sfx {

SoundLooper::SoundLooper(SoundMixer *soundMixer)
{
	this->soundMixer = soundMixer;

	this->nextHandle = 0;
	this->nextKey = 0;

	for (int i = 0; i < SOUNDLOOPER_MAX_SOUNDS; i++)
	{
		loopSound[i] = NULL;
		startSound[i] = NULL;
		endSound[i] = NULL;
		loopHandle[i] = -1;
		startHandle[i] = -1;
		endHandle[i] = -1;
		startEndTime[i] = 0;
		loopEndTime[i] = 0;
		endEndTime[i] = 0;
		playPhase[i] = PLAYPHASE_NONE;
		positionX[i] = 0.0f;
		positionY[i] = 0.0f;
		positionZ[i] = 0.0f;
		handleKey[i] = -1;
		volume[i] = DEFAULT_SOUND_EFFECT_VOLUME;
		priority[i] = 0;
		range[i] = 0;
	}

	for (int j = 0; j < SOUNDLOOPER_MAX_USER_HANDLES; j++)
	{
		userStoredHandle[j] = -1;
	}
}


SoundLooper::~SoundLooper()
{
	reset();
}


int SoundLooper::playLoopedSound(SoundSample *start, 
	SoundSample *looped, SoundSample *end, 
	int startDuration, int loopDuration, int endDuration,
	float positionX, float positionY, float positionZ,
	int *key, bool muteVolume, float range_, int priority_, bool ambient_)
{
	int prevHandle = nextHandle;
	int ret = -1;
	while (true)
	{
		if (playPhase[nextHandle] == PLAYPHASE_NONE)
		{
			int curtime = Timer::getTime();
			loopSound[nextHandle] = looped;
			startSound[nextHandle] = start;
			endSound[nextHandle] = end;
			loopHandle[nextHandle] = -1;
			startHandle[nextHandle] = -1;
			endHandle[nextHandle] = -1;
			startEndTime[nextHandle] = curtime + startDuration;
			loopEndTime[nextHandle] = startEndTime[nextHandle] + loopDuration;
			endEndTime[nextHandle] = loopEndTime[nextHandle] + endDuration;
			
			volume[nextHandle] = DEFAULT_SOUND_EFFECT_VOLUME;
			range[nextHandle] = range_;
			priority[nextHandle] = priority_;

			playPhase[nextHandle] = PLAYPHASE_INIT;
			ambient[nextHandle] = ambient_;

			this->positionX[nextHandle] = positionX;
			this->positionY[nextHandle] = positionY;
			this->positionZ[nextHandle] = positionZ;

			handleKey[nextHandle] = nextKey;

			ret = nextHandle;
			*key = nextKey;
			nextKey++;
			break;
		}

		nextHandle = (nextHandle + 1) % SOUNDLOOPER_MAX_SOUNDS;
		if (nextHandle == prevHandle)
		{
			Logger::getInstance()->debug("SoundLooper::playLoopedSound - Too many sounds playing.");
			return -1;
		}
	}	
	nextHandle = (nextHandle + 1) % SOUNDLOOPER_MAX_SOUNDS;
	return ret;
}


bool SoundLooper::isSoundStillLooping(int loopedSoundHandle, int key)
{
	if (playPhase[loopedSoundHandle] != PLAYPHASE_NONE
		&& handleKey[loopedSoundHandle] == key)
		return true;
	else
		return false;	
}

bool SoundLooper::isSoundStillPlaying(int loopedSoundHandle, int key)
{
	if(playPhase[loopedSoundHandle] == PLAYPHASE_INIT)
		return true;

	return soundMixer->isSoundPlaying(loopedSoundHandle);
}

bool SoundLooper::continueLoopedSound(int loopedSoundHandle, 
	int key, int loopDuration)
{
	int i = loopedSoundHandle;

	if (handleKey[i] != key)
	{
		Logger::getInstance()->debug("SoundLooper::continueLoopedSound - Sound key mismatch.");
		return false;
	}

	if (playPhase[i] != PLAYPHASE_NONE)
	{
		if (playPhase[i] == PLAYPHASE_INIT
			|| playPhase[i] == PLAYPHASE_START
			|| playPhase[i] == PLAYPHASE_LOOP)
		{
			int currentTime = Timer::getTime();
			int diff = (currentTime + loopDuration) - loopEndTime[i];
			if (diff > 0)
			{
				loopEndTime[i] += diff;
				endEndTime[i] += diff;
			}
			return true;
		}
	}	
	return false;
}


void SoundLooper::stopLoopedSound(int loopedSoundHandle, 
	int key, bool immediately)
{
	int i = loopedSoundHandle;

	// TODO: shouldn't we check this???
	if (handleKey[i] != key)
	{
		Logger::getInstance()->warning("SoundLooper::stopLoopedSound - Sound key mismatch.");
		return;
	}

	if (playPhase[i] != PLAYPHASE_NONE)
	{
		if (playPhase[i] == PLAYPHASE_INIT)
		{
			playPhase[i] = PLAYPHASE_NONE;
		}	else {
			if (immediately)
			{
				if (playPhase[i] == PLAYPHASE_START)
				{
					soundMixer->stopSound(startHandle[i]);
				}
				if (playPhase[i] == PLAYPHASE_LOOP)
				{
					soundMixer->stopSound(loopHandle[i]);
				}
				if (playPhase[i] == PLAYPHASE_END)
				{
					soundMixer->stopSound(endHandle[i]);
				}
				playPhase[i] = PLAYPHASE_NONE;
			} else {
				if (playPhase[i] == PLAYPHASE_START)
				{
					int diff = loopEndTime[i] - startEndTime[i];
					if (diff > 0)
					{
						loopEndTime[i] -= diff;
						endEndTime[i] -= diff;
					}
				}
				if (playPhase[i] == PLAYPHASE_LOOP)
				{
					int currentTime = Timer::getTime();
					int diff = loopEndTime[i] - currentTime;
					if (diff > 0)
					{
						loopEndTime[i] -= diff;
						endEndTime[i] -= diff;
					}
				}
				//if (playPhase[i] == PLAYPHASE_END)
				//{
					// nop
				//}
			}
		}
	}
}


void SoundLooper::setSoundVolume(int loopedSoundHandle, int key, int volume)
{
	int i = loopedSoundHandle;

	// TODO: shouldn't we check this???
	if (handleKey[i] != key)
	{
 		Logger::getInstance()->warning("SoundLooper::setSoundVolume - Sound key mismatch.");
		return;
	}

	if (volume < 0) volume = 0;
	if (volume > 100) volume = 100;

  this->volume[i] = volume;

	//Logger::getInstance()->error(int2str(volume));

	if (playPhase[i] != PLAYPHASE_NONE)
	{
		if (playPhase[i] == PLAYPHASE_INIT)
		{
			// nop?
		}	else {
			if (playPhase[i] == PLAYPHASE_START)
			{
				soundMixer->setSoundVolume(startHandle[i], this->volume[i]);
			}
			if (playPhase[i] == PLAYPHASE_LOOP)
			{
				soundMixer->setSoundVolume(loopHandle[i], this->volume[i]);
			}
			if (playPhase[i] == PLAYPHASE_END)
			{
				soundMixer->setSoundVolume(endHandle[i], this->volume[i]);
			}
		}
	}
}



void SoundLooper::setSoundPosition(int loopedSoundHandle, int key, float x, float y, float z)
{
	int i = loopedSoundHandle;

	// TODO: shouldn't we check this???
	if (handleKey[i] != key)
	{
 		Logger::getInstance()->warning("SoundLooper::setSoundPosition - Sound key mismatch.");
		return;
	}

	//Logger::getInstance()->error(int2str(volume));

	if (playPhase[i] != PLAYPHASE_NONE)
	{
		if (playPhase[i] == PLAYPHASE_INIT)
		{
			// nop?
		}	else {
			if (playPhase[i] == PLAYPHASE_START)
			{
				soundMixer->setSoundPosition(startHandle[i], x, y, z, 0, 0, 0, this->volume[i], 0);
			}
			if (playPhase[i] == PLAYPHASE_LOOP)
			{
				soundMixer->setSoundPosition(loopHandle[i], x, y, z, 0, 0, 0, this->volume[i], 0);
			}
			if (playPhase[i] == PLAYPHASE_END)
			{
				soundMixer->setSoundPosition(endHandle[i], x, y, z, 0, 0, 0, this->volume[i], 0);
			}
		}
	}
}

void SoundLooper::run()
{
	for (int i = 0; i < SOUNDLOOPER_MAX_SOUNDS; i++)
	{
		if (playPhase[i] != PLAYPHASE_NONE)
		{
			int prevPhase = playPhase[i];
			int curTime = Timer::getTime();

			// need to switch phase?
			// if so, solve new phase
			if (playPhase[i] == PLAYPHASE_INIT)
			{
				if (startSound[i] != NULL && startEndTime[i] > curTime)
				{
					playPhase[i] = PLAYPHASE_START;
				}
				else if (loopSound[i] != NULL && loopEndTime[i] > curTime)
				{
					playPhase[i] = PLAYPHASE_LOOP;
				} 
				else if (endSound[i] != NULL && endEndTime[i] > curTime)
				{
					playPhase[i] = PLAYPHASE_END;
				} else {
					playPhase[i] = PLAYPHASE_NONE;
				}
			}
			else if (playPhase[i] == PLAYPHASE_START)
			{
				if (curTime > startEndTime[i])
				{
					if (loopSound[i] != NULL && loopEndTime[i] > curTime)
					{
						playPhase[i] = PLAYPHASE_LOOP;
					} 
					else if (endSound[i] != NULL && endEndTime[i] > curTime)
					{
						playPhase[i] = PLAYPHASE_END;
					} else {
						playPhase[i] = PLAYPHASE_NONE;
					}
				}
			}
			else if (playPhase[i] == PLAYPHASE_LOOP)
			{
				if (curTime > loopEndTime[i])
				{
					if (endSound[i] != NULL && endEndTime[i] > curTime)
					{
						playPhase[i] = PLAYPHASE_END;
					} else {
						playPhase[i] = PLAYPHASE_NONE;
					}
				}
			}
			else if (playPhase[i] == PLAYPHASE_END)
			{
				if (curTime > endEndTime[i])
				{
					playPhase[i] = PLAYPHASE_NONE;					
				}
			}

			if (playPhase[i] != prevPhase)
			{
				// stop old sound
				if (prevPhase == PLAYPHASE_START)			
				{
					//assert(startHandle[i] != -1);
					soundMixer->stopSound(startHandle[i]);
					startHandle[i] = -1;
				}
				else if (prevPhase == PLAYPHASE_LOOP)
				{
					//assert(loopHandle[i] != -1);
					soundMixer->stopSound(loopHandle[i]);
					loopHandle[i] = -1;
				}
				else if (prevPhase == PLAYPHASE_END)
				{
					//assert(endHandle[i] != -1);
					soundMixer->stopSound(endHandle[i]);
					endHandle[i] = -1;
				}

				// start new sound
				// TODO: sound volumes!!!
				if (playPhase[i] == PLAYPHASE_START)
				{
					if(ambient[i]) startHandle[i] = soundMixer->playAmbientSound(startSound[i], true, range[i], priority[i]);
					else startHandle[i] = soundMixer->playSoundEffect(startSound[i], true, range[i], priority[i]);
					if (startHandle[i] != -1)
					{
						soundMixer->setSoundPosition(startHandle[i], positionX[i], positionY[i], positionZ[i], 0, 0, 0, volume[i], 0);
					} else {
						//assert(!"SoundLooper::run - Looped sound start phase sound creation failed.");
					}
				}
				else if (playPhase[i] == PLAYPHASE_LOOP)
				{
					if(ambient[i]) loopHandle[i] = soundMixer->playAmbientSound(loopSound[i], true, range[i], priority[i]);
					else loopHandle[i] = soundMixer->playSoundEffect(loopSound[i], true, range[i], priority[i]);
					if (loopHandle[i] != -1)
					{
						soundMixer->setSoundPosition(loopHandle[i], positionX[i], positionY[i], positionZ[i], 0, 0, 0, volume[i], 0);
					} else {
						//assert(!"SoundLooper::run - Looped sound loop phase sound creation failed.");
					}
				}
				else if (playPhase[i] == PLAYPHASE_END)
				{
					if(ambient[i]) endHandle[i] = soundMixer->playAmbientSound(endSound[i], true, range[i], priority[i]);
					else endHandle[i] = soundMixer->playSoundEffect(endSound[i], true, range[i], priority[i]);
					if (endHandle[i] != -1)
					{
						soundMixer->setSoundPosition(endHandle[i], positionX[i], positionY[i], positionZ[i], 0, 0, 0, volume[i], 0);
					} else {
						//assert(!"SoundLooper::run - Looped sound end phase sound creation failed.");
					}
				}
			}

		}
	}
	
}


void SoundLooper::reset()
{
	for (int i = 0; i < SOUNDLOOPER_MAX_SOUNDS; i++)
	{
		if (playPhase[i] == PLAYPHASE_START)
		{
			soundMixer->stopSound(startHandle[i]);
			startHandle[i] = -1;
		}
		if (playPhase[i] == PLAYPHASE_LOOP)
		{
			soundMixer->stopSound(loopHandle[i]);
			loopHandle[i] = -1;
		}
		if (playPhase[i] == PLAYPHASE_END)
		{
			soundMixer->stopSound(endHandle[i]);
			endHandle[i] = -1;
		}
		loopSound[i] = NULL;
		startSound[i] = NULL;
		endSound[i] = NULL;
		loopHandle[i] = -1;
		startHandle[i] = -1;
		endHandle[i] = -1;
		startEndTime[i] = 0;
		loopEndTime[i] = 0;
		endEndTime[i] = 0;
		playPhase[i] = PLAYPHASE_NONE;
		positionX[i] = 0.0f;
		positionY[i] = 0.0f;
		positionZ[i] = 0.0f;
		volume[i] = DEFAULT_SOUND_EFFECT_VOLUME;
		range[i] = 0.0f;
		priority[i] = 0;
	}
	for (int j = 0; j < SOUNDLOOPER_MAX_USER_HANDLES; j++)
	{
		userStoredHandle[j] = -1;
	}
}


void SoundLooper::storeUserNumberedHandle(int number, int loopedSoundHandle)
{
	if (number < 0 || number >= SOUNDLOOPER_MAX_USER_HANDLES)
	{
		assert(0);
		return;
	}

	userStoredHandle[number] = loopedSoundHandle;
}


int SoundLooper::getStoredUserNumberedHandle(int number)
{
	if (number < 0 || number >= SOUNDLOOPER_MAX_USER_HANDLES)
	{
		assert(0);
		return -1;
	}

	return userStoredHandle[number];
}

void SoundLooper::setSoundsPaused(bool paused, bool affect_ambient_sounds)
{
	for (int i = 0; i < SOUNDLOOPER_MAX_SOUNDS; i++)
	{
		if(!affect_ambient_sounds && ambient[i])
			continue;

		if (playPhase[i] == PLAYPHASE_START)
		{
			soundMixer->setSoundPaused(startHandle[i], paused);
		}
		if (playPhase[i] == PLAYPHASE_LOOP)
		{
			soundMixer->setSoundPaused(loopHandle[i], paused);
		}
		if (playPhase[i] == PLAYPHASE_END)
		{
			soundMixer->setSoundPaused(endHandle[i], paused);
		}
	}
}

void SoundLooper::getHandles(int number, int *_startHandle, int *_loopHandle, int *_endHandle)
{
	if (number < 0 || number >= SOUNDLOOPER_MAX_USER_HANDLES)
	{
		assert(0);
		return;
	}
	if(_startHandle) *_startHandle = startHandle[number];
	if(_loopHandle) *_loopHandle = loopHandle[number];
	if(_endHandle) *_endHandle = loopHandle[number];
}

} // sfx
