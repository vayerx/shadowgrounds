#ifndef SOUNDMIXER_H
#define SOUNDMIXER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class IStorm3D_StreamBuilder;

// HACK: a quick hacky way to determine if play failed due to too many instances of sample playing
enum SOUNDMIXER_PLAY_ERRORCODE
{
	SOUNDMIXER_PLAY_ERRORCODE_NONE,
	SOUNDMIXER_PLAY_ERRORCODE_COUNT_EXCEEDED, // too many instances of sample already playing
	SOUNDMIXER_PLAY_ERRORCODE_OTHER
};
extern SOUNDMIXER_PLAY_ERRORCODE soundmixer_play_errorcode;


namespace sfx {

class Sound;
class SoundLib;
class SoundStream;

class SoundSample
{
public:
	Sound *data;
	int playCount;
	bool temporaryCache;
	char *filename;

	int getLength();
	SoundSample(const char *filename, Sound *data, bool temporaryCache);
	~SoundSample();
};

/**
 * A simple wrapper class for CSoundLib. 
 * Implementing music fade in/out feature and looping.
 *
 * Use this to set game's background music instead of using the
 * CSoundLib classes directly.
 *
 * (See also GameUI, it kinda wraps this one, normally should use it)
 *
 * @version v1.0.1, 2.7.2002 
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 * @see GameUI
 * @see CSoundLib
 * 
 */

class SoundMixer
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	enum SoundStreamType
	{
		SOUNDSTREAMTYPE_EFFECT = 0,
		SOUNDSTREAMTYPE_MUSIC = 1,
		SOUNDSTREAMTYPE_AMBIENT = 2
	};

public:
	explicit SoundMixer(SoundLib *soundLib);
	~SoundMixer();

	/**
	 * Clear all the currently cached sound samples.
	 * All sounds are stopped before cleaning the sample cache.
	 */
	void cleanSampleCache();

	/**
	 * Clear all the currently cached temporary sound samples.
	 * All sounds are stopped before cleaning the sample cache.
	 * Cleans only those samples that were originally loaded with
	 * temporaryCache flag set.
	 */
	void cleanTemporarySampleCache();

	/**
	 * Stops all sounds currently playing. 
	 * (Does not effect the music though.)
	 */
	void stopAllSounds();
	void stopAllSpeech();
	void setVolume(int masterVolume, int fxVolume, int speechVolume, int musicVolume); // backwards compatibility
	void setVolume(int masterVolume, int fxVolume, int speechVolume, int musicVolume, int ambientVolume);
	void setMute(bool fxMute, bool speechMute, bool musicMute);

	/**
	* Run the mixer.
	* Must be called pretty much all the time to make sure fading
	* gets done correctly - the more often you call this, the 
	* smoother the fade out and fade in is.
	* If not called often enough, music does not fade.
	* Basically, calling this every ~10 ms is more than good enough
	* This also handles the music looping.
	* @param currentTime  int, the current time in milliseconds.
	*/
	void runMixer(int currentTime);

	/**
	* Sets new music to be played, fades out current music if one
	* is playing and then fades in the new music.
	* @param filename  char*, music file to be set as background music.
	*/
	void setMusic(const char *filename, int fadeTime = 1000);

	/**
	* Check if music is fading in or out.
	* Returns true if music is currently fading in or out.
	* Can be used in while loop to wait for music to fade out, for 
	* example.
	* @return  bool, true if music is fading in or out.
	*/
	bool isMusicFading() const;
	bool hasMusicEnded() const;

	void setMusicLooping(bool looping);
	void playStreamedSound(const char *filename);
	void stopStreamedSounds();

	SoundSample *loadSample(const char *filename, bool temporaryCache);
	boost::shared_ptr<SoundStream> getStream(const char *filename, SoundStreamType type);
	IStorm3D_StreamBuilder *getStreamBuilder();

	int playSoundEffect(SoundSample *sample, bool loop, float range, int priority);
	int playAmbientSound(SoundSample *sample, bool loop, float range, int priority);
	int playSpeech(SoundSample *sample, bool loop);
	void stopSound(int soundHandle);
	void setSoundLoop(int soundHandle, bool loop);
	void setSoundPaused(int soundHandle, bool pause);

	void setSoundPosition(int soundHandle,
    float x,float y,float z,float vx,float vy,float vz, int volume, unsigned short position);

	void setSoundFrequency(int soundHandle, int frequency);
	int getSoundFrequency(int soundHandle);

	void setSoundVolume(int soundHandle, int volume);
	void setSoundFrequency(float frequency);
	bool isSoundPlaying(int soundHandle) const;

	void setSoundArea(const std::string &name);
	void setPosition(float x, float y, float z);
	void setListenerPosition(
		float x,float y,float z,
		float vx,float vy,float vz,
		float dx,float dy,float dz,
		float ux,float uy,float uz);

	enum SoundEventType
	{
		Event_Nop,
		Event_PlaySpeech,
		Event_Pause,
		Event_Resume,
		Event_Stop,
		Event_SpeechStart, // lowers volume
		Event_SpeechStop   // resets volume
	};

	struct SoundEvent
	{
		int time;
		SoundEventType type;
		// Event_Pause, Event_Resume, Event_Stop:
		int soundHandle;
		// Event_PlaySpeech:
		SoundSample *sample;
		float x,y,z;
		int volume;
		bool loop;
	};
	
	void queueSoundEvent(const SoundEvent &se);
	void clearAllSoundEvents(void);

	SoundEvent *getLastSoundEventOfType(SoundEventType type);

	// runs event instantly
	void runSoundEvent(const SoundEvent &se);

private:
	/*
	int playSound(SoundSample *sample, bool loop, int volume);

	SoundLib *soundLib;
	SoundStream *music;

	int masterVolume;

	int fxVolume;
	int speechVolume;
	int musicVolume;
	bool fxMute;
	bool speechMute;
	bool musicMute;

	bool musicEnded;

	int currentTime;
	int musicFadeStartTime;
	bool musicFadingOut;
	bool musicFadingIn;
	char *nextMusicFilename;

	int replayCheckTime;

	SoundSample **sampleRef;
	int *playVolume;

	SoundHashType *soundFileHash;

	bool musicLooping;
	*/
};

} // sfx

#endif
