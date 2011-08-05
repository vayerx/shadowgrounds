#include "precompiled.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <assert.h>
#include <boost/shared_ptr.hpp>

#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../system/Miscellaneous.h"
#include "SoundLib.h"
#include "SoundMixer.h"
#include "../util/Debug_MemoryManager.h"
#include <istorm3d_streambuffer.h>


#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

// msecs
#define MUSIC_FADE_OUT_TIME 3000
#define MUSIC_FADE_IN_TIME -1

#ifdef PROJECT_AOV
static const int MAX_SAMPLE_PLAY_COUNT = 5;
#else
static const int MAX_SAMPLE_PLAY_COUNT = 4;
#endif

SOUNDMIXER_PLAY_ERRORCODE soundmixer_play_errorcode = SOUNDMIXER_PLAY_ERRORCODE_NONE;

using namespace std;
using namespace boost;

namespace sfx {

	struct Song;
	struct Music;
	struct FadeMusic;

	struct SoundInstance
	{
		SoundSample *sample;
		float originalVolume;
		float volume;

		VC3 position;
		float range;

		int priority;
		bool loop;
		int time;

		enum Type
		{
			TYPE_EFFECT = 0,
			TYPE_SPEECH = 1,
			TYPE_AMBIENT = 2,
			TYPE_MUSIC = 3
		};
		Type type;

		SoundInstance()
		:	sample(0),
			originalVolume(1.f),
			volume(1.f),
			range(0),
			priority(0),
			loop(false),
			time(0),
			type(TYPE_EFFECT)
		{
		}
	};

	typedef map<string, SoundSample *> SampleList;
	typedef map<int, SoundInstance> SoundList;
	typedef vector<Song> SongList;
	typedef vector<FadeMusic> FadeMusicList;

	struct Song
	{
		string file;
		int fadeTime;

		Song()
		:	fadeTime(0)
		{
		}
	};

	struct Music
	{
		shared_ptr<SoundStream> stream;
	};

	struct FadeMusic
	{
		Music music;
		int time;
		int length;

		FadeMusic()
		:	time(0),
			length(0)
		{
		}

		float getFactor() const
		{
			float f = (float(time) / float(length - 1));
			if(f > 1.f)
				f = 1.f;
			if(f < 0)
				f = 0.f;

			return f;
		}
	};


SoundSample::SoundSample(const char *filename, Sound *data, bool temporaryCache) 
{ 
	if(filename != NULL)
	{
		this->filename = new char[strlen(filename) + 1];
		strcpy(this->filename, filename);
	} 
	else
		this->filename = NULL;
  
	this->data = data; 
	this->temporaryCache = temporaryCache;
	playCount = 0; 
}

SoundSample::~SoundSample() 
{ 
	if(playCount > 0) 
	{
		Logger::getInstance()->warning("~SoundSample - Sample still playing while destroyed.");
		Logger::getInstance()->debug(filename);
		assert(!"~SoundSample - Sample still playing while destroyed.");
	}

	delete [] filename;
	delete data; 
}

int SoundSample::getLength()
{
	if(!data)
		return 0;

	return data->getLength();
}

typedef std::list<boost::weak_ptr<SoundStream> > StreamList;

struct SoundMixer::Data : public IStorm3D_StreamBuilder
{
	SoundLib *soundLib;
	SoundStream *music;

	float masterVolume;
	float fxVolume;
	float speechVolume;
	float musicVolume;
	float ambientVolume;

	bool fxMute;
	bool speechMute;
	bool musicMute;

	SampleList samples;
	SoundList sounds;

	FadeMusic fadeInMusic;
	Music currentStream;
	FadeMusicList fadeOutMusic;
	Song nextSong;

	bool loopMusic;
	VC3 cameraPosition;
	VC3 listenerPosition;

	vector<shared_ptr<SoundStream> > streamedSounds;
	StreamList streamList;

	std::vector<SoundEvent> soundEvents;

	struct VolumeFade
	{
		VolumeFade() : lastFactor(-1.0f) {}
		float fxVolumeSource;
		float speechVolumeSource;
		float musicVolumeSource;
		float ambientVolumeSource;

		float fxVolumeTarget;
		float speechVolumeTarget;
		float musicVolumeTarget;
		float ambientVolumeTarget;

		int timeStart;
		int length;

		float lastFactor;
	};
	VolumeFade *volumeFade;

	// non-faded volumes as set by SetVolume
	float fxVolumeDefault;
	float speechVolumeDefault;
	float musicVolumeDefault;
	float ambientVolumeDefault;


	Data(SoundLib *soundLib_)
	:	soundLib(soundLib_),
		music(0),
		masterVolume(1.f),
		fxVolume(1.f),
		speechVolume(1.f),
		musicVolume(1.f),
		ambientVolume(1.f),
		fxMute(false),
		speechMute(false),
		musicMute(false),
		loopMusic(false),
		volumeFade( NULL ),
		fxVolumeDefault(1.f),
		speechVolumeDefault(1.f),
		musicVolumeDefault(1.f),
		ambientVolumeDefault(1.f)
	{

		stereo = true;
		frequency = 22050;
		bits = 16;
	}

	~Data()
	{
		delete volumeFade;
		stopAll();
		clearSamples();
	}

	void clearTemporaryCache()
	{
		// Yeah, stupid as hell but trying to delete while iterating crashes it
		for(;;)
		{
			bool found = false;

			SampleList::iterator it = samples.begin();
			for(; it != samples.end(); ++it)
			{
				SoundSample *s = it->second;
				if(s->temporaryCache && s->playCount == 0)
				{
					samples.erase(it);
					delete s;

					found = true;
					break;
				}
			}

			if(!found)
				break;
		}
	}

	void clearSamples()
	{
		SampleList::iterator it = samples.begin();
		for(; it != samples.end(); ++it)
			delete it->second;

		samples.clear();
	}

	void stopAll()
	{
		SoundList::iterator it = sounds.begin();
		for(; it != sounds.end(); ++it)
		{
			assert(it->second.sample);

			--it->second.sample->playCount;
			soundLib->stopSound(it->first);
		}

		sounds.clear();
	}

	void stopAllSpeech()
	{
		/*
		SoundList::iterator it = sounds.begin();
		for(; it != sounds.end(); ++it)
		{
			assert(it->second.sample);
			if(it->second.type != SoundInstance::TYPE_SPEECH)
				continue;

			--it->second.sample->playCount;
			soundLib->stopSound(it->first);


		}

		sounds.clear();
		*/

		for(;;)
		{
			bool found = false;

			SoundList::iterator it = sounds.begin();
			for(; it != sounds.end(); ++it)
			{
				assert(it->second.sample);
				if(it->second.type != SoundInstance::TYPE_SPEECH)
					continue;

				--it->second.sample->playCount;
				soundLib->stopSound(it->first);

				sounds.erase(it);

				found = true;
				break;
			}

			if(!found)
				break;
		}

	}

	SoundSample *loadSample(string filename, bool temporaryCache)
	{
		assert(soundLib != NULL);

		for(unsigned int i = 0; i < filename.size(); ++i)
		{
			if(filename[i] == '\\')
				filename[i] = '/';

			filename[i] = tolower(filename[i]);
		}

		SampleList::iterator it = samples.find(filename);
		if(it != samples.end())
		{
			return it->second;
		}

		LOG_DEBUG(strPrintf("SoundMixer::loadSample - Loading sound sample %s", filename.c_str()).c_str());

		Sound *sound = soundLib->loadSample(filename.c_str());
		if(!sound)
			return 0;

		SoundSample *sample = new SoundSample(filename.c_str(), sound, temporaryCache);
		samples[filename] = sample;
		return sample;
	}

	int play(SoundSample *sample, bool loop, float volume, float range, int priority)
	{
		soundmixer_play_errorcode = SOUNDMIXER_PLAY_ERRORCODE_NONE;

		if(!sample)
		{
			Logger::getInstance()->warning("Tried to play NULL sample.");
			assert(sample);
			soundmixer_play_errorcode = SOUNDMIXER_PLAY_ERRORCODE_OTHER;
			return -1;
		}

		if(sample->playCount > MAX_SAMPLE_PLAY_COUNT)
		{
			soundmixer_play_errorcode = SOUNDMIXER_PLAY_ERRORCODE_COUNT_EXCEEDED;
			return -1;
		}

		int handle = soundLib->createSound(sample->data, priority);
		if(handle < 0)
		{
			string message = "play -- Failed to create sound handle ";
			message += sample->filename;

			Logger::getInstance()->warning(message.c_str());
			soundmixer_play_errorcode = SOUNDMIXER_PLAY_ERRORCODE_OTHER;
			return -1;
		}

		SoundInstance &instance = sounds[handle];
		if(instance.sample)
		{
			Logger::getInstance()->warning("play -- Tried to override playing sample.");
			soundmixer_play_errorcode = SOUNDMIXER_PLAY_ERRORCODE_OTHER;
			return -1;
		}

		soundLib->setSoundLoop(handle, loop);
		soundLib->setSoundVolume(handle, volume);

		instance.sample = sample;
		instance.originalVolume = volume;
		instance.range = range / 10.f; // scale !
		instance.position = cameraPosition;
		instance.priority = priority;
		instance.loop = loop;

		++sample->playCount;
		return handle;
	}

	void setInstanceProperties(int soundHandle, const VC3 &position, const VC3 &velocity)
	{
		if(!soundLib || soundHandle < 0)
			return;

		SoundList::iterator it = sounds.find(soundHandle);
		if(it == sounds.end())
		{
			LOG_DEBUG(strPrintf("setInstanceProperties: Invalid soundHandle %d", soundHandle).c_str());
			return;
		}

		if(it->second.type != SoundInstance::TYPE_SPEECH)
		{
			soundLib->setSound3D(soundHandle, position, velocity);

			SoundInstance &instance = it->second;
			instance.position = position;
		}
	}

	void playStreamedSound(shared_ptr<SoundStream> &stream)
	{
		streamedSounds.push_back(stream);
	}

	struct SoundEventCompare
	{
		bool operator()(const SoundEvent &a, const SoundEvent &b)
		{
			return a.time > b.time;
		}
	};

	void queueSoundEvent(const SoundEvent &se)
	{
		soundEvents.push_back(se);
		std::push_heap(soundEvents.begin(), soundEvents.end(), SoundEventCompare());
	}

	void runSoundEvent(const SoundEvent &se, SoundMixer *mixer)
	{
		switch(se.type)
		{
		case Event_PlaySpeech:
			{
				int h = mixer->playSpeech(se.sample, se.loop);
				if(h != -1)
				{
					mixer->setSoundPosition(h, se.x, se.y, se.z, 0, 0, 0, se.volume, 0);
				}
			}
			break;
		case Event_Pause:
			mixer->setSoundPaused(se.soundHandle, true);
			break;
		case Event_Resume:
			mixer->setSoundPaused(se.soundHandle, false);
			break;
		case Event_Stop:
			mixer->stopSound(se.soundHandle);
			break;
		case Event_SpeechStart:
			{
				delete volumeFade;
				volumeFade = new VolumeFade();

				volumeFade->fxVolumeSource = fxVolume;
				volumeFade->fxVolumeTarget = fxVolumeDefault * 0.5f;

				volumeFade->musicVolumeSource = musicVolume;
				volumeFade->musicVolumeTarget = musicVolumeDefault * 0.5f;

				volumeFade->ambientVolumeSource = ambientVolume;
				volumeFade->ambientVolumeTarget = ambientVolumeDefault * 0.5f;

				volumeFade->speechVolumeSource = speechVolume;
				volumeFade->speechVolumeTarget = speechVolumeDefault;

				volumeFade->timeStart = Timer::getTime();
				volumeFade->length = 1000;
			}
			break;
		case Event_SpeechStop:
			{
				delete volumeFade;
				volumeFade = new VolumeFade();

				volumeFade->fxVolumeSource = fxVolume;
				volumeFade->fxVolumeTarget = fxVolumeDefault;

				volumeFade->musicVolumeSource = musicVolume;
				volumeFade->musicVolumeTarget = musicVolumeDefault;

				volumeFade->ambientVolumeSource = ambientVolume;
				volumeFade->ambientVolumeTarget = ambientVolumeDefault;

				volumeFade->speechVolumeSource = speechVolume;
				volumeFade->speechVolumeTarget = speechVolumeDefault;

				volumeFade->timeStart = Timer::getTime();
				volumeFade->length = 1000;
			}
			break;
		default:
			break;
		};
	}

	void updateSoundEvents(int currenttime, SoundMixer *mixer)
	{
		while(!soundEvents.empty())
		{
			const SoundEvent &se = soundEvents[0];

			if(se.time > currenttime)
				break;

			runSoundEvent(se, mixer);

			std::pop_heap(soundEvents.begin(), soundEvents.end(), SoundEventCompare());
			soundEvents.pop_back();
		}

		if(volumeFade != NULL)
		{
			float factor = (Timer::getTime() - volumeFade->timeStart) / (float)volumeFade->length;
			if(factor > 1.0f)
				factor = 1.0f;
			else if(factor < 0.0f)
				factor = 0.0f;

			float factor2 = 1.0f - factor;

			if(factor != volumeFade->lastFactor)
			{
				volumeFade->lastFactor = factor;

				fxVolume =      volumeFade->fxVolumeSource * factor2      + volumeFade->fxVolumeTarget * factor;
				ambientVolume = volumeFade->ambientVolumeSource * factor2 + volumeFade->ambientVolumeTarget * factor;
				musicVolume =   volumeFade->musicVolumeSource * factor2   + volumeFade->musicVolumeTarget * factor;
				speechVolume =  volumeFade->speechVolumeSource * factor2  + volumeFade->speechVolumeTarget * factor;
				applyVolumes();

				if(factor == 1.0f)
				{
					delete volumeFade;
					volumeFade = NULL;
				}
			}
		}
	}

	void updateSounds(int delta)
	{
		// ToDo: 
		//  - Disable sounds if volume drops below 0
		//  - Enable sounds if they get dropped 'cause too many sounds are playing

		SoundList::iterator it = sounds.begin();
		for(; it != sounds.end(); )
		{
			SoundInstance &instance = it->second;
			instance.time += delta;

			int handle = it->first;
			if(!soundLib->isSoundPlaying(handle))
			{
				/*
				// TEST -- try to maintain sounds
				if(it->second.loop || instance.time < instance.sample->getLength() - 1)
				{
				}
				else
				*/
				{
					if(it->second.sample)
					{
						--it->second.sample->playCount;
						assert(it->second.sample->playCount >= 0);
						soundLib->stopSound(it->first);
					}
					else
					{
						assert(it->second.sample);
						Logger::getInstance()->warning("updateSounds -- Sound has no sample");
					}

					sounds.erase(it++);
				}
			}
			else
			{
				float volume = 0;
				const VC3 &a = listenerPosition; //cameraPosition;
				const VC3 &b = instance.position;
				float xd = a.x - b.x;
				float zd = a.z - b.z;

				if(instance.type != SoundInstance::TYPE_SPEECH)
				{
					float distance = sqrtf(xd*xd + zd*zd);
					if(distance < instance.range)
					{
						volume = 1.f - (distance / instance.range);
						volume *= instance.volume * instance.originalVolume;
					}

					soundLib->setSoundVolume(handle, volume);
				} else 
				{
					SoundInstance &instance = it->second;
					instance.position = listenerPosition;
					soundLib->setSound3D(handle, listenerPosition, VC3());
					soundLib->setSoundVolume(handle, instance.volume * instance.originalVolume);
				}
				++it;
			}
		}

		{
			vector<shared_ptr<SoundStream> >::iterator it = streamedSounds.begin();
			for(; it != streamedSounds.end(); )
			{
				if((*it)->hasEnded())
					it = streamedSounds.erase(it);
				else
					++it;
			}
		}
	}

	void updateMusic(int delta)
	{
		// Fade out music
		{
			FadeMusicList::iterator it = fadeOutMusic.begin();
			for(; it != fadeOutMusic.end(); )
			{
				FadeMusic &fade = *it;
				Music &music = fade.music;

				fade.time += delta;
				if(fade.time > fade.length)
				{
					music.stream->stop();
					it = fadeOutMusic.erase(it);
					continue;
				}

				music.stream->setVolume((1.f - fade.getFactor()));
				++it;
			}
		}

		// Fade in music
		if(fadeInMusic.music.stream)
		{
			fadeInMusic.time += delta;
			Music &music = fadeInMusic.music;

			if(fadeInMusic.time > fadeInMusic.length)
			{
				currentStream = music;
				currentStream.stream->setVolume(1.f);
				fadeInMusic = FadeMusic();
			}
			else
				music.stream->setVolume(fadeInMusic.getFactor());
		}

		if(currentStream.stream && currentStream.stream->hasEnded())
		{
			if(loopMusic)
			{
				// ToDo: Proper looping!
				//currentStream.stream->stop();
				//currentStream.stream->play();
			}
			else
				currentStream = Music();
		}

		// New songs
		if(!nextSong.file.empty())
		{
			boost::shared_ptr<SoundStream> stream = getStream(nextSong.file.c_str(), SOUNDSTREAMTYPE_MUSIC);

			Music music;
			if(stream)
			{
				music.stream = stream;
				music.stream->setVolume(1.f);

				// Set directly if only song
				if(!currentStream.stream && !fadeInMusic.music.stream)
					currentStream = music;
				else
				{
					// Current fade in to fade out
					if(fadeInMusic.music.stream)
					{
						fadeInMusic.time = fadeInMusic.length - fadeInMusic.time;
						fadeOutMusic.push_back(fadeInMusic);
						fadeInMusic = FadeMusic();
					}
					// Current song to fade out
					if(currentStream.stream)
					{
						FadeMusic fade;
						fade.music = currentStream;
						fade.length = nextSong.fadeTime;
						fadeOutMusic.push_back(fade);
						currentStream = Music();
					}

					FadeMusic fade;
					fade.music = music;
					fade.length = nextSong.fadeTime;

					fadeInMusic = fade;
					music.stream->setVolume(0);
				}

				music.stream->play();
			}

			nextSong.file.clear();
			nextSong.fadeTime = 0;
		}
	}

	boost::shared_ptr<SoundStream> getStream(const char *filename, SoundStreamType type)
	{
		SoundStream *nullStream = 0;
		if(!soundLib)
		{
			return boost::shared_ptr<SoundStream>(nullStream);
		}

		if(!filename)
		{
			Logger::getInstance()->error("SoundMixer::loadStream - Null filename parameter given.");
			return boost::shared_ptr<SoundStream>(nullStream);
		}

		boost::shared_ptr<SoundStream> stream(soundLib->createStream(filename));
		if(!stream)
			return stream;

		stream->setType(type);

		if(type == SOUNDSTREAMTYPE_MUSIC)
		{
			stream->setBaseVolume(musicVolume);
		}
		else if(type == SOUNDSTREAMTYPE_AMBIENT)
		{
			stream->setBaseVolume(ambientVolume);
		}
		else
		{
			stream->setBaseVolume(fxVolume);
		}
		
		boost::weak_ptr<SoundStream> weakStream(stream);
		streamList.push_back(weakStream);
		return stream;
	}
	
	void applyVolumes()
	{
		float volume_music = musicVolume;
		if(musicMute)
			volume_music = 0.f;
		
		float volume_effect = fxVolume;
		if(fxMute)
			volume_effect = 0.0f;

		float volume_ambient = ambientVolume;
		if(false)
			volume_ambient = 0.0f;

		float volume_speech = speechVolume;
		if(speechMute)
			volume_speech = 0.0f;

		for(StreamList::iterator it = streamList.begin(); it != streamList.end(); )
		{
			boost::shared_ptr<SoundStream> stream = it->lock();
			if(!stream)
			{
				it = streamList.erase(it);
				continue;
			}

			if(stream->getType() == SOUNDSTREAMTYPE_MUSIC)
			{
				stream->setBaseVolume( volume_music );
			}
			else if(stream->getType() == SOUNDSTREAMTYPE_AMBIENT)
			{
				stream->setBaseVolume( volume_ambient );
			}
			else
			{
				stream->setBaseVolume( volume_effect );
			}
			++it;
		}

		SoundList::iterator it = sounds.begin();
		for(; it != sounds.end(); ++it)
		{
			SoundInstance &instance = it->second;
			if(instance.type == SoundInstance::TYPE_EFFECT)
			{
				instance.originalVolume = volume_effect;
			}
			else if(instance.type == SoundInstance::TYPE_SPEECH)
			{
				instance.originalVolume = volume_speech;
			}
			else if(instance.type == SoundInstance::TYPE_AMBIENT)
			{
				instance.originalVolume = volume_ambient;
			}
			else if(instance.type == SoundInstance::TYPE_MUSIC)
			{
				instance.originalVolume = volume_music;
			}
		}
	}

	bool stereo;
	int frequency;
	int bits;

	void setStereo(bool data)
	{
		stereo = data;
	}

	void setFrequency(int data)
	{
		frequency = data;
	}

	void setBits(int data)
	{
		bits = data;
	}

	void update()
	{
		soundLib->update();
	}

	boost::shared_ptr<IStorm3D_Stream> getStream()
	{
		return soundLib->createStormStream(stereo, frequency, bits, fxVolume);
	}
};

SoundMixer::SoundMixer(SoundLib *soundLib)
{
	scoped_ptr<Data> tempData(new Data(soundLib));
	data.swap(tempData);
}

SoundMixer::~SoundMixer()
{
}

void SoundMixer::stopAllSounds()
{
	data->stopAll();
}

void SoundMixer::stopAllSpeech()
{
	data->stopAllSpeech();
}

void SoundMixer::cleanSampleCache()
{
	data->stopAll();
	data->clearSamples();
}

void SoundMixer::cleanTemporarySampleCache()
{
	data->clearTemporaryCache();
}

void SoundMixer::setVolume(int masterVolume, int fxVolume, int speechVolume, int musicVolume)
{
	setVolume(masterVolume, fxVolume, speechVolume, musicVolume, fxVolume);
}
void SoundMixer::setVolume(int masterVolume, int fxVolume, int speechVolume, int musicVolume, int ambientVolume)
{
	if (masterVolume < 0) 
		masterVolume = 0;
	if (masterVolume > 100) 
		masterVolume = 100;
	if (fxVolume < 0) 
		fxVolume = 0;
	if (fxVolume > 100) 
		fxVolume = 100;
	if (speechVolume < 0) 
		speechVolume = 0;
	if (speechVolume > 100) 
		speechVolume = 100;
	if (musicVolume < 0) 
		musicVolume = 0;
	if (musicVolume > 100) 
		musicVolume = 100;
	if (ambientVolume > 100)
		ambientVolume = 100;

	data->masterVolume = masterVolume / 100.f;
	data->fxVolume = (fxVolume / 100.f) * data->masterVolume;
	data->speechVolume = (speechVolume / 100.f) * data->masterVolume;
	data->musicVolume = (musicVolume / 100.f) * data->masterVolume;  
	data->ambientVolume = (ambientVolume / 100.0f) * data->masterVolume;

	data->fxVolumeDefault = data->fxVolume;
	data->speechVolumeDefault = data->speechVolume;
	data->musicVolumeDefault = data->musicVolume;  
	data->ambientVolumeDefault = data->ambientVolume;

	data->applyVolumes();
}

void SoundMixer::setMute(bool fxMute, bool speechMute, bool musicMute)
{
	data->fxMute = fxMute;
	data->speechMute = speechMute;
	data->musicMute = musicMute;
}

void SoundMixer::runMixer(int currentTime)
{
	if(!data->soundLib)
		return;

	static int lastTime = currentTime;
	int delta = currentTime - lastTime;
	if(delta > 100)
		delta = 100;

	lastTime = currentTime;

	data->updateSoundEvents(currentTime, this);
	data->updateSounds(delta);
	data->updateMusic(delta);
	
	data->soundLib->update();
}

void SoundMixer::setMusic(const char *filename, int fadeTime)
{
	if(!data->soundLib || data->musicMute)
		return;

	if(filename)
	{
		data->nextSong.file = filename;
		data->nextSong.fadeTime = fadeTime;
	}
	else
	{
		// Current fade in to fade out
		if(data->fadeInMusic.music.stream)
		{
			data->fadeInMusic.time = data->fadeInMusic.length - data->fadeInMusic.time;
			data->fadeOutMusic.push_back(data->fadeInMusic);
			data->fadeInMusic = FadeMusic();
		}
		// Current song to fade out
		if(data->currentStream.stream)
		{
			FadeMusic fade;
			fade.music = data->currentStream;
			fade.length = fadeTime;
			data->fadeOutMusic.push_back(fade);
			data->currentStream = Music();
		}
	}
}

bool SoundMixer::isMusicFading() const
{
	if(data->fadeInMusic.music.stream || !data->fadeOutMusic.empty())
		return true;

	return false;
}

bool SoundMixer::hasMusicEnded() const
{
	if(isMusicFading() || data->currentStream.stream)
		return false;

	if(!data->nextSong.file.empty())
		return false;

	return true;
}

void SoundMixer::setMusicLooping(bool looping)
{
	data->loopMusic = looping;
}

void SoundMixer::playStreamedSound(const char *filename)
{
	if(data->fxMute)
		return;

	if(!filename)
	{
		Logger::getInstance()->error("SoundMixer::playStreamedSound - Null filename parameter given.");
		return;
	}

	boost::shared_ptr<SoundStream> stream(data->getStream(filename, SOUNDSTREAMTYPE_EFFECT));
	if(stream)
	{
		stream->setVolume(data->fxVolume);
		stream->setLooping(false);
		stream->play();

		data->playStreamedSound(stream);
	}
}

void SoundMixer::stopStreamedSounds()
{
	for(unsigned int i = 0; i < data->streamedSounds.size(); ++i)
		data->streamedSounds[i]->stop();

	data->streamedSounds.clear();
}

SoundSample *SoundMixer::loadSample(const char *filename, bool temporaryCache)
{
	if (filename == NULL)
	{
		assert(!"SoundMixer::loadSample - Null filename parameter given.");
		Logger::getInstance()->error("SoundMixer::loadSample - Null filename parameter given.");
		return NULL;
	}

	return data->loadSample(filename, temporaryCache);
}

boost::shared_ptr<SoundStream> SoundMixer::getStream(const char *filename, SoundStreamType type)
{
	return data->getStream(filename, type);
}

IStorm3D_StreamBuilder *SoundMixer::getStreamBuilder()
{
	return data.get();
}

int SoundMixer::playSoundEffect(SoundSample *sample, bool loop, float range, int priority)
{
	if(!data->fxMute)
	{
		int handle = data->play(sample, loop, 0, range, priority);
		if(handle > 0)
		{
			SoundInstance &instance = data->sounds[handle];
			instance.originalVolume = data->fxVolume;
			instance.type = SoundInstance::TYPE_EFFECT;
		}

		return handle;
	}

	return -1;
}

int SoundMixer::playAmbientSound(SoundSample *sample, bool loop, float range, int priority)
{
	if(true)
	{	
		int handle = data->play(sample, loop, 0, range, priority);
		if(handle > 0)
		{
			SoundInstance &instance = data->sounds[handle];
			instance.originalVolume = data->ambientVolume;
			instance.type = SoundInstance::TYPE_AMBIENT;
		}

		return handle;
	}

	return -1;
}

int SoundMixer::playSpeech(SoundSample *sample, bool loop)
{
	if(!data->speechMute)
	{
		int handle = data->play(sample, loop, 0, 10000.f, 255);
		if(handle > 0)
		{
			SoundInstance &instance = data->sounds[handle];
			instance.originalVolume = data->speechVolume;
			instance.type = SoundInstance::TYPE_SPEECH;
		}

		return handle;
		
	}

	return -1;
}

void SoundMixer::stopSound(int soundHandle)
{
	if(soundHandle < 0 || !data->soundLib)
		return;

	data->soundLib->stopSound(soundHandle);
	return;

	/*
	SoundList::iterator it = data->sounds.find(soundHandle);
	if(it != data->sounds.end())
	{
		if(it->second.sample)
			--it->second.sample->playCount;
		else
		{
			assert(it->second.sample);
		}

		data->soundLib->stopSound(soundHandle);
		data->sounds.erase(it);
	}
	*/
}

void SoundMixer::setSoundLoop(int soundHandle, bool loop)
{
	if(soundHandle < 0 || !data->soundLib)
		return;

	data->soundLib->setSoundLoop(soundHandle, loop);
	return;
}

void SoundMixer::setSoundPaused(int soundHandle, bool pause)
{
	if(soundHandle < 0 || !data->soundLib)
		return;

	data->soundLib->setSoundPaused(soundHandle, pause);
	return;
}

void SoundMixer::setSoundPosition(int soundHandle,
  float x,float y,float z,float vx,float vy,float vz, int volume, unsigned short position)
{
	if(soundHandle < 0 || !data->soundLib)
		return;

	x /= 10.f;
	y /= 10.f;
	z /= 10.f;

	VC3 p(x, y, z);
	VC3 v(vx, vy, vz);

	setSoundVolume(soundHandle, volume);

	data->setInstanceProperties(soundHandle, p, v);
	data->soundLib->playSound(soundHandle);
}

void SoundMixer::setSoundVolume(int soundHandle, int volume)
{
	if(!data->soundLib || soundHandle < 0)
		return;

	if (volume < 0) 
		volume = 0;
	if (volume > 100) 
		volume = 100;

	SoundList::iterator it = data->sounds.find(soundHandle);
	if(it == data->sounds.end())
	{
		//assert(!"Invalid soundhandle");
		return;
	}

	SoundInstance &instance = it->second;
	instance.volume = volume / 100.f;
}

void SoundMixer::setSoundFrequency(float frequency)
{
	if(!data->soundLib)
		return;

	data->soundLib->setFrequencyFactor(frequency);

	SoundList::iterator it = data->sounds.begin();
	for(; it != data->sounds.end(); ++it)
	{
		int id = it->first;
		
		float freq = data->soundLib->getSoundFrequency(id) * frequency;
		data->soundLib->setSoundFrequency(id, int(freq + .5f));
	}
}

void SoundMixer::setSoundFrequency(int soundHandle, int freq)
{
	if(!data->soundLib || soundHandle < 0)
		return;

	data->soundLib->setSoundFrequency(soundHandle, freq);
}

int SoundMixer::getSoundFrequency(int soundHandle)
{
	if(!data->soundLib || soundHandle < 0)
		return 0;

	return data->soundLib->getSoundFrequency(soundHandle);
}


bool SoundMixer::isSoundPlaying(int soundHandle) const
{
	if(!data->soundLib || soundHandle < 0)
		return false;

	return data->soundLib->isSoundPlaying(soundHandle);
}

void SoundMixer::setListenerPosition(
  float x,float y,float z,
  float vx,float vy,float vz,
  float dx,float dy,float dz,
  float ux,float uy,float uz)
{
	if(!data->soundLib)
		return;

	x /= 10.f;
	y /= 10.f;
	z /= 10.f;

	data->cameraPosition = VC3(x,y,z);
	data->soundLib->setListener(data->cameraPosition, VC3(vx,vy,vz), VC3(dx,dy,dz), VC3(ux,uy,uz));
}

void SoundMixer::setPosition(float x, float y, float z)
{
	data->listenerPosition.x = x / 10.f;
	data->listenerPosition.y = y / 10.f;
	data->listenerPosition.z = z / 10.f;
}

void SoundMixer::setSoundArea(const std::string &name)
{
	if(data->soundLib)
		data->soundLib->setSoundArea(name);
}

void SoundMixer::runSoundEvent(const SoundEvent &se)
{
	data->runSoundEvent(se, this);
}

void SoundMixer::queueSoundEvent(const SoundEvent &se)
{
	data->queueSoundEvent(se);
}

void SoundMixer::clearAllSoundEvents(void)
{
	data->soundEvents.clear();
	delete data->volumeFade;
	data->volumeFade = NULL;
}

SoundMixer::SoundEvent *SoundMixer::getLastSoundEventOfType(SoundEventType type)
{
	SoundMixer::SoundEvent *se = NULL;
	for(unsigned int i = 0; i < data->soundEvents.size(); i++)
	{
		if(data->soundEvents[i].type == type)
		{
			se = &(data->soundEvents[i]);
		}
	}
	return se;
}

/*
SoundMixer::SoundMixer(SoundLib *soundLib)
{
  this->soundLib = soundLib;
  music = NULL;
  nextMusicFilename = NULL;

  currentTime = 0;
  musicFadeStartTime = 0;
  musicFadingOut = false;
  musicFadingIn = false;
  replayCheckTime = 0;

  musicVolume = 100;
  fxVolume = 100;
  speechVolume = 100;
  musicMute = false;
  fxMute = false;
  speechMute = false;

  musicEnded = false;

  sampleRef = new SoundSample *[SOUNDMIXER_MAX_SOUNDS];
  playVolume = new int[SOUNDMIXER_MAX_SOUNDS];
  for (int i = 0; i < SOUNDMIXER_MAX_SOUNDS; i++)
  {
    sampleRef[i] = NULL;
    playVolume[i] = 100;
  }

  soundFileHash = new SoundHashType();

  musicLooping = false;
}


SoundMixer::~SoundMixer()
{
  if (music != NULL)
  {
    // music->Stop(); // unnecessary
    delete music;
    music = NULL;
  }

	cleanSampleCache();
	delete soundFileHash;

  delete [] playVolume;
  delete [] sampleRef;
}


// WARNING: if someone is relying on some sound being played 
// (looping sounds), this may result into an error when the sound 
// is being stopped again.

void SoundMixer::stopAllSounds()
{
	for (int h = 0; h < SOUNDMIXER_MAX_SOUNDS; h++)
	{
		if (sampleRef[h] != NULL)
		{
			this->stopSound(h);
		}
	}
}


void SoundMixer::cleanSampleCache()
{
	this->stopAllSounds();

	SoundHashType::iterator it = soundFileHash->begin();
	for (; it != soundFileHash->end(); ++it)
	{
		delete it->second;
		//soundFileHash->erase(it);
	}
	soundFileHash->clear();	
}


void SoundMixer::cleanTemporarySampleCache()
{
	assert(!"TODO");
}


void SoundMixer::setVolume(int masterVolume, int fxVolume, int speechVolume, int musicVolume)
{
  if (masterVolume < 0) masterVolume = 0;
  if (masterVolume > 100) masterVolume = 100;
  if (fxVolume < 0) fxVolume = 0;
  if (fxVolume > 100) fxVolume = 100;
  if (speechVolume < 0) speechVolume = 0;
  if (speechVolume > 100) speechVolume = 100;
  if (musicVolume < 0) musicVolume = 0;
  if (musicVolume > 100) musicVolume = 100;
  this->masterVolume = masterVolume;
  this->fxVolume = (fxVolume * masterVolume) / 100;
  this->speechVolume = (speechVolume * masterVolume) / 100;
  this->musicVolume = (musicVolume * masterVolume) / 100;  
}

void SoundMixer::setMute(bool fxMute, bool speechMute, bool musicMute)
{
  this->fxMute = fxMute;
  this->speechMute = speechMute;
  this->musicMute = musicMute;
}

// psd .. music is just too loud. should make this scriptable
//const int musicVolume = -1500;

void SoundMixer::runMixer(int currentTime)
{
  this->currentTime = currentTime;
  // fade in/out music if music is changing
  if (musicFadingOut) 
  {
    if (musicFadeStartTime == -1) 
      musicFadeStartTime = currentTime;
    //Logger::getInstance()->debug(int2str(currentTime - musicFadeStartTime));
    if (currentTime - musicFadeStartTime > MUSIC_FADE_OUT_TIME)
    {
      music->setVolume(-10000);
      music->stop();
      delete music;
      music = NULL;
      musicFadingOut = false;
      if (nextMusicFilename != NULL)
      {
        musicFadingIn = true;
        musicFadeStartTime = currentTime;
      }
    } else {
      // real silence limit would ne -10000, but this
      // is logarithm value i think, so it goes out really
      // fast and the rest of the values cannot be heard
      int vol = (-5000 * (currentTime - musicFadeStartTime)) / MUSIC_FADE_OUT_TIME;
      if (vol > -2500 + musicVolume * 25) vol = -2500 + musicVolume * 25;
      if (vol < -5000) vol = -5000;
      music->setVolume(vol);
    }
  } else {
    if (musicFadingIn) 
    {
      if (musicFadeStartTime == -1) 
        musicFadeStartTime = currentTime;
      if (nextMusicFilename != NULL)
      {
        // music not yet started? start it now.
        if (music == NULL)
        {
          //musicFadeStartTime = currentTime;
          music = soundLib->createStream(nextMusicFilename);
          if (music != NULL)
          {
            music->setVolume(-10000);
            music->play();
          } else {
            musicFadingIn = false;
            Logger::getInstance()->error("SoundMixer::setMusic - Failed to load music.");
          }
        }
      }
      if (music != NULL)
      {
        if (currentTime - musicFadeStartTime > MUSIC_FADE_IN_TIME) 
        {
          // done fading in, just keep on playing...
          music->setVolume(-2500 + musicVolume * 25);
          musicFadingIn = false;
        } else {
          int vol = (-5000 * (MUSIC_FADE_IN_TIME - (currentTime - musicFadeStartTime))) / MUSIC_FADE_IN_TIME;
          if (vol > -2500 + musicVolume * 25) vol = -2500 + musicVolume * 25;
          if (vol < -5000) vol = -5000;
          music->setVolume(vol);
        }
      }
    } else {
      if (music != NULL)
      {
        // check every once a while if music has completed, replay if so.
        if (currentTime >= replayCheckTime)
        {
          replayCheckTime = currentTime + 1000;
          if (music->hasEnded())
          {
            if (musicLooping)
            {
              music->stop();
              music->play();
              musicEnded = false;
            } else {
              musicEnded = true;
            }
          } else {
            musicEnded = false;
          }
        }
      }
    }
  }

  soundLib->update();

	// mark all (non-looping) sounds that have completed as being stopped.
	for (int h = 0; h < SOUNDMIXER_MAX_SOUNDS; h++)
	{
		if (sampleRef[h] != NULL)
		{
			if (!soundLib->isSoundPlaying(h))
			{
				this->stopSound(h);
			}
		}
	}
}


void SoundMixer::setMusic(const char *filename)
{
  musicEnded = false;

  if (musicMute && filename != NULL)
    return;

  if (soundLib != NULL)
  {
    // set new music filename
    if (nextMusicFilename != NULL)
    {
      delete [] nextMusicFilename;
    }
    if (filename != NULL)
    {
      nextMusicFilename = new char[strlen(filename) + 1];
      strcpy(nextMusicFilename, filename);
    } else {
      nextMusicFilename = NULL;
    }
    // start fading out old music
    // notice: this won't give a nice result if we are
    // already fading in some music.
    if (music != NULL)
    {
      // unless already fading out some other music...
      if (!musicFadingOut)
      {
        musicFadingOut = true;
        musicFadeStartTime = -1; //currentTime;
      }
    } else {
      // if no old music, just start fading in the new one
      if (nextMusicFilename != NULL)
      {
        musicFadingIn = true;
        musicFadeStartTime = -1; //currentTime;
      }
    }
  }
}


bool SoundMixer::isMusicFading() const
{
  if (musicFadingIn || musicFadingOut) 
    return true;
  else 
    return false;
}


bool SoundMixer::hasMusicEnded() const
{
  return musicEnded;
}


void SoundMixer::setMusicLooping(bool looping)
{
  musicLooping = looping;
}


SoundSample *SoundMixer::loadSample(const char *filename, bool temporaryCache)
{
  // ooohh yeah... ;)
  // maybe, just maybe, should use some other method to do this
  int hashCode = 0;
  int i = 0;
  int mult = 0;
  while(filename[i] != '\0')
  {
    hashCode ^= (filename[i] << mult);
    mult++;
    if (mult > 21) mult = 0;
    i++;
  }

  // see if the sample is already loaded...
  SoundHashType::iterator iter = soundFileHash->find(hashCode);
  if (iter != soundFileHash->end())
  {
    SoundSample *loaded = (*iter).second; 
    return loaded;
  }

  Logger::getInstance()->debug("SoundMixer::loadSample - Loading sound sample.");
  Logger::getInstance()->debug(filename);

  Sound *snd = soundLib->loadSample(filename);
  if (snd == NULL)
  {
    Logger::getInstance()->error("SoundMixer::loadSample - Failed to load sample.");
    return NULL;
  }
  SoundSample *ret = new SoundSample(filename, snd, temporaryCache);

  soundFileHash->insert(std::pair<int, SoundSample *> (hashCode, ret));

  return ret;
}


int SoundMixer::playSoundEffect(SoundSample *sample, bool loop)
{
  if (!fxMute)
    return playSound(sample, loop, fxVolume);
  else
    return -1;
}


int SoundMixer::playSpeech(SoundSample *sample, bool loop)
{
  if (!speechMute)
    return playSound(sample, loop, speechVolume);
  else
    return -1;
}


int SoundMixer::playSound(SoundSample *sample, bool loop, int volume)
{
  if (sample == NULL)
  {
    Logger::getInstance()->debug("SoundMixer::playSound - Attempt to play null sample.");
    return -1;
  }
  int handle = soundLib->createSound(sample->data);
  if (handle >= 0)
  {
	  if(loop)
		  soundLib->setSoundLoop(handle, true);
	  soundLib->playSound(handle);

// TEMP
//Logger::getInstance()->error("play");
//Logger::getInstance()->error(int2str(handle));
//Logger::getInstance()->error(sample->filename);
    sample->playCount++;
    sampleRef[handle] = sample;
    playVolume[handle] = volume;
  } else {
    Logger::getInstance()->error("SoundMixer::playSound - Could not play sound.");
  }
  return handle;
}


void SoundMixer::stopSound(int soundHandle)
{
  if (soundHandle < 0) 
  {
    Logger::getInstance()->debug("SoundMixer::stopSound - Negative sound handle parameter given.");
    return;
  }

  SoundSample *sample = sampleRef[soundHandle];

  if (sample == NULL)
  {
    Logger::getInstance()->debug("SoundMixer::stopSound - No sound playing for given handle.");
//Logger::getInstance()->error("stop, bad handle");
//Logger::getInstance()->error(int2str(soundHandle));
    return;
  }
// TEMP
//Logger::getInstance()->error("stop");
//Logger::getInstance()->error(int2str(soundHandle));
//Logger::getInstance()->error(sample->filename);

  sample->playCount--;

  //Logger::getInstance()->error(int2str(soundHandle));

	soundLib->stopSound(soundHandle);

  //if (sample->playCount == 0)
  //{
    sampleRef[soundHandle] = NULL;
  //}
}


void SoundMixer::setSoundPosition(int soundHandle,
  float x,float y,float z,float vx,float vy,float vz, int volume, unsigned short position)
{
  if (soundHandle < 0) 
  {
    Logger::getInstance()->debug("SoundMixer::setSoundPosition - Negative sound handle parameter given.");
    return;
  }
  if (sampleRef[soundHandle] == NULL)
  {
    Logger::getInstance()->warning("SoundMixer::setSoundPosition - No sound playing for given handle.");
    return;
  }
  int mixedVolume = -5000 + ((playVolume[soundHandle] * 50) * volume) / 100;
  // presuming DWORD = unsigned short
  // making a little position scaling...
  x /= 20;
  y /= 20;
  z /= 20;
  //soundLib->SetSound(soundHandle, x, y, z, vx, vy, vz, mixedVolume, position);
  soundLib->setSound3D(soundHandle, VC3(x,y,z), VC3(vx,vy,vz));
}


void SoundMixer::setSoundVolume(int soundHandle, int volume)
{
  if (soundHandle < 0) 
  {
    Logger::getInstance()->debug("SoundMixer::setSoundVolume - Negative sound handle parameter given.");
    return;
  }
  if (sampleRef[soundHandle] == NULL)
  {
    Logger::getInstance()->debug("SoundMixer::setSoundVolume - No sound playing for given handle.");
//Logger::getInstance()->error("volume, bad handle");
//Logger::getInstance()->error(int2str(soundHandle));
    return;
  }
  int mixedVolume = -5000 + ((playVolume[soundHandle] * 50) * volume) / 100;
  soundLib->setSoundVolume(soundHandle, mixedVolume);
}


void SoundMixer::setListenerPosition(
  float x,float y,float z,
  float vx,float vy,float vz,
  float dx,float dy,float dz,
  float ux,float uy,float uz)
{
  x /= 20;
  y /= 20;
  z /= 20;
  soundLib->setListener(VC3(x,y,z), VC3(vx,vy,vz), VC3(dx,dy,dz), VC3(ux,uy,uz));
}
*/

} // sfx

