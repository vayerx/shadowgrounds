#ifndef SOUNDLIB_H
#define SOUNDLIB_H


#include <boost/shared_ptr.hpp>


class IStorm3D_Stream;


namespace sfx {

class SoundLib;
class StormStream;

class Sound
{
	struct Data;
	boost::shared_ptr<Data> data;
	friend class SoundLib;

public:
	Sound(Data *data_);
	~Sound();

	void setDefaultVolume(float value);
	void setPriority(int value);
	int getLength() const;

	const std::string &getFileName() const;
};


class SoundStream
{
	struct Data;
	boost::shared_ptr<Data> data;
	friend class SoundLib;
	friend struct Data;

public:

	explicit SoundStream(Data *data_);
	~SoundStream();

	void setBaseVolume(float value);
	void setVolume(float value);
	void setPanning(float value);
	void setLooping(bool loop);
	void play();
	void pause();
	void stop();

	void setType(int t);
	int getType() const;

	bool hasEnded() const;
	const std::string &getFileName() const;
};


class SoundLib
{
	struct Data;
	boost::shared_ptr<Data> data;

	friend class SoundStream;
	friend class StormStream;

public:
	SoundLib();
	~SoundLib();

	enum SpeakerType
	{
		DolbyDigital,
		HeadPhones,
		MonoSpeakers,
		StereoSpeakers,
		QuadSpeakers,
		SurroundSpeakers
	};

	void setProperties(int mixrate, int softwareChannels);
	void setAcceleration(bool useHW, bool useEax, int minHardwareChannels, int maxHardwareChannels);
	void setSpeakers(SpeakerType speakerType);
	void setSoundAPI(const char *api);
	bool initialize();
	void setFrequencyFactor(float scalar);

	Sound *loadSample(const char *file);
	SoundStream *createStream(const char *file);

	// Sounds
	int createSound(Sound *sound, int priority);
	void playSound(int sound);
	void setSoundLoop(int sound, bool loop);
	void setSoundPaused(int sound, bool pause);
	void setSoundVolume(int sound, float value);
	void setSoundFrequency(int sound, int value);
	void setSound3D(int sound, const VC3 &position, const VC3 &velocity);
	void stopSound(int sound);

	float getSoundVolume(int sound) const;
	int getSoundFrequency(int sound) const;
	int getSoundTime(int sound) const;
	bool isSoundPlaying(int sound) const;

	// Music

	// General
	void setGlobalVolume(float volume);
	void setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection);
	void setSoundArea(const std::string &name);

	void update();

	boost::shared_ptr<IStorm3D_Stream> createStormStream(bool stereo_, int frequency_, int bits_, float volume);
};


} // sfx


#endif
