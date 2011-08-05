#ifndef SOUNDLOOPER_H
#define SOUNDLOOPER_H

#include "SoundMixer.h"

#define SOUNDLOOPER_MAX_SOUNDS 128
#define SOUNDLOOPER_MAX_USER_HANDLES 64

namespace sfx {

class SoundLooper
{
public:
  SoundLooper(SoundMixer *soundMixer);
  ~SoundLooper();

	int playLoopedSound(SoundSample *start, SoundSample *looped,
		SoundSample *end,
		int startDuration, int loopDuration, int endDuration,
		float positionX, float positionY, float positionZ,
		int *key, bool muteVolume, float range, int priority, bool ambient);

	bool isSoundStillLooping(int loopedSoundHandle, int key);

	bool isSoundStillPlaying(int loopedSoundHandle, int key);

	bool continueLoopedSound(int loopedSoundHandle, int key, int loopDuration);

	void stopLoopedSound(int loopedSoundHandle, int key, bool immediately);

	void setSoundVolume(int loopedSoundHandle, int key, int volume);

	void setSoundPosition(int loopedSoundHandle, int key, float x, float y, float z);

	void run();

	void reset();

	// so that one does not have to make own arrays for ambient
	// sounds, these are here for that.
	void storeUserNumberedHandle(int number, int loopedSoundHandle);
	int getStoredUserNumberedHandle(int number);

	void setSoundsPaused(bool paused, bool affect_ambient_sounds);

	void getHandles(int number, int *startHandle, int *loopHandle, int *endHandle);

private:
	SoundMixer *soundMixer;

	int nextHandle;
	int nextKey;

	SoundSample *loopSound[SOUNDLOOPER_MAX_SOUNDS];
	SoundSample *startSound[SOUNDLOOPER_MAX_SOUNDS];
	SoundSample *endSound[SOUNDLOOPER_MAX_SOUNDS];
	int loopHandle[SOUNDLOOPER_MAX_SOUNDS];
	int startHandle[SOUNDLOOPER_MAX_SOUNDS];
	int endHandle[SOUNDLOOPER_MAX_SOUNDS];
	int startEndTime[SOUNDLOOPER_MAX_SOUNDS];
	int loopEndTime[SOUNDLOOPER_MAX_SOUNDS];
	int endEndTime[SOUNDLOOPER_MAX_SOUNDS];
	int playPhase[SOUNDLOOPER_MAX_SOUNDS];
	float positionX[SOUNDLOOPER_MAX_SOUNDS];
	float positionY[SOUNDLOOPER_MAX_SOUNDS];
	float positionZ[SOUNDLOOPER_MAX_SOUNDS];
	int handleKey[SOUNDLOOPER_MAX_SOUNDS];
	int volume[SOUNDLOOPER_MAX_SOUNDS];
	int priority[SOUNDLOOPER_MAX_SOUNDS];
	float range[SOUNDLOOPER_MAX_SOUNDS];
	bool ambient[SOUNDLOOPER_MAX_SOUNDS];

	int userStoredHandle[SOUNDLOOPER_MAX_USER_HANDLES];

};

} // sfx

#endif
