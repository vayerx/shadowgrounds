#include "precompiled.h"

#include <boost/utility.hpp>
#include <SDL.h>

#include "../SoundLib.h"

// NOTE: unwanted dependency (due to sound_missing_warning option)
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"

#include <istorm3d_streambuffer.h>

#ifndef PROJECT_VIEWER
extern bool isThisDeveloper();
#endif

using namespace frozenbyte;


namespace sfx {


struct SoundStream::Data : public boost::noncopyable
{
	std::string filename;

	Data(const char *filename_)
	: filename(filename_)
	{
	}

};


void SoundStream::setVolume(float value) {}
void SoundStream::play() {}


void SoundStream::setType(int t)
{
}


int SoundStream::getType() const
{
	return 0;
}


bool SoundStream::hasEnded() const { return true; }
void SoundStream::stop() {}
void SoundLib::stopSound(int sound) {}
void SoundLib::setSound3D(int sound, const VC3 &position, const VC3 &velocity) {}
void SoundLib::setSoundVolume(int sound, float value) {}
bool SoundLib::isSoundPlaying(int sound) const { return false; }


SoundStream::SoundStream(Data *data_)
: data(data_)
{
}


SoundStream::~SoundStream() {}
void SoundStream::setBaseVolume(float value) {}


SoundStream *SoundLib::createStream(const char *file)
{
	return new SoundStream(new SoundStream::Data(file));
}


void SoundLib::setSoundLoop(int sound, bool loop) {}
void SoundLib::setSoundPaused(int sound, bool pause) {}
int SoundLib::createSound(Sound *sound, int priority) { return -1; }
void SoundLib::setSoundArea(const std::string &name) {}
void SoundLib::update() {}
void SoundStream::setLooping(bool loop) {}
Sound *SoundLib::loadSample(const char *file) { return NULL; }
void SoundLib::playSound(int sound) {}
void SoundLib::setSoundFrequency(int sound, int value) {}
void SoundLib::setFrequencyFactor(float scalar) {}
int SoundLib::getSoundFrequency(int sound) const { return 0; }
void SoundLib::setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection) {}
Sound::~Sound() {}
SoundLib::~SoundLib() {}
void SoundStream::setPanning(float value) {}
void SoundLib::setSoundAPI(const char *) {}
bool SoundLib::initialize() { return true; }
void SoundLib::setAcceleration(bool useHW, bool useEax_, int minHardwareChannels_, int maxHardwareChannels_) {}
void SoundLib::setProperties(int mixrate_, int softwareChannels_) {}
int Sound::getLength() const { return 1; }
SoundLib::SoundLib() {}


struct StormStream : public IStorm3D_Stream
{
	void activate()
	{
	}


	void deactivate()
	{
	}


	void addSample(const char*, int, Uint64, Uint64)
	{
	}


	Uint64 getCurrentTime() const
	{
		return 0;
	}

};


boost::shared_ptr<IStorm3D_Stream> SoundLib::createStormStream(bool stereo_, int frequency_, int bits_, float volume)
{
	return boost::shared_ptr<IStorm3D_Stream>(new StormStream());
}

}
