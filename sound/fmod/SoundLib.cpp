#include "precompiled.h"

#ifdef _WIN32
#include <windows.h>

#else  // _WIN32
#include <wincompat.h>

#endif  // _WIN32

#include <SDL.h>

#include <assert.h>

#include <string>
#include <list>

#include <fmod.h>

#include <boost/lexical_cast.hpp>
#include <boost/shared_array.hpp>
#include <boost/utility.hpp>

#include "../SoundLib.h"
#include <istorm3d_streambuffer.h>

// NOTE: unwanted dependency (due to sound_missing_warning option)
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"

#include "../system/Logger.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"

#include "../util/Debug_MemoryManager.h"

#ifdef _MSC_VER
#pragma comment(lib, "fmodvc.lib")
#endif

#ifndef PROJECT_VIEWER
extern bool isThisDeveloper();
#endif

using namespace std;
using namespace boost;
using namespace frozenbyte::editor;
using namespace frozenbyte;

namespace sfx {

/*
  Sound
*/


struct Sound::Data : public boost::noncopyable {
	Data(const char *file, int flags);
	~Data();

	FSOUND_SAMPLE *sample;
	int volume;
	int priority;
	int length;
	bool fileNotFound;

	std::string filename;

	void updateProperties();

};


struct SoundStream::Data : public boost::noncopyable {
	Data(const char *file);
	~Data();

	FSOUND_STREAM *stream;
	int channel;

	boost::shared_array<char> buffer;
	std::string filename;

	float baseVolume;
	float volume;
	int type; // SoundMixer::SoundStreamType
};


struct SoundLib::Data : public boost::noncopyable {
	Data();
	~Data();

	bool initialized;
	bool useHardware;
	bool useEax;
	int mixrate;
	int softwareChannels;
	int minHardwareChannels;
	int maxHardwareChannels;
	int speakerType;
	std::string soundAPI;

	float frequencyFactor;

	std::map<std::string, FSOUND_REVERB_PROPERTIES> soundAreas;
	std::map<std::string, std::string> areaAlias;
};


void Sound::Data::updateProperties()
{
	assert(sample);
	//FSOUND_Sample_SetDefaults(sample, -1, volume, -1, priority);
}


Sound::Sound(Data *data_)
: data(data_)
{

	if(data->sample)
	{
		setDefaultVolume(255);
		setPriority(1);

		int frequency = 0;
		FSOUND_Sample_GetDefaults(data->sample, &frequency, 0, 0, 0);
		//length = frequency * FSOUND_Sample_GetLength(sample);
		data->length = 1000 * FSOUND_Sample_GetLength(data->sample) / frequency;
	}
}


Sound::Data::Data(const char *file, int flags)
:	sample(0),
	volume(255),
	priority(0),
	length(0),
	fileNotFound(false)
, filename(file)
{
	if(!file)
		return;

	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(file);
	if(stream.isEof())
	{
		fileNotFound = true;
		return;
	}

	//sample = FSOUND_Sample_Load(FSOUND_UNMANAGED, file, flags, 0, 0);
	vector<char> buffer(stream.getSize());
	stream.read(&buffer[0], buffer.size());
	sample = FSOUND_Sample_Load(FSOUND_UNMANAGED, &buffer[0], flags | FSOUND_LOADMEMORY, 0, buffer.size());
}


Sound::~Sound()
{
}


Sound::Data::~Data()
{
	if(sample)
	{
		FSOUND_Sample_Free(sample);
		sample = NULL;
	}
}


void Sound::setDefaultVolume(float value)
{
	assert(data->sample);

	data->volume = int(value * 255.f);
	if(data->volume < 0)
		data->volume = 0;
	if(data->volume > 255)
		data->volume = 255;

	data->updateProperties();
}

void Sound::setPriority(int value)
{
	assert(data->sample);
	data->priority = value;

	data->updateProperties();
}

int Sound::getLength() const
{
	return data->length;
}

const std::string &Sound::getFileName() const
{
	return data->filename;
}

/*
  SoundStream
*/


SoundStream::SoundStream(Data *data_)
: data(data_)
{
}


SoundStream::Data::Data(const char *file)
: stream(0)
, channel(-1)
, filename(file)
, baseVolume(1.f)
, volume(1.f)
, type(0)
{
	filename = file;

	filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(file);
	if(fileStream.isEof())
		return;

	//stream = FSOUND_Stream_Open(file, 0, 0, 0);

	buffer.reset(new char[fileStream.getSize()]);
	fileStream.read(buffer.get(), fileStream.getSize());
	stream = FSOUND_Stream_Open(buffer.get(), FSOUND_LOADMEMORY, 0, fileStream.getSize());

	if(stream)
		channel = FSOUND_Stream_PlayEx(FSOUND_FREE, stream, 0, TRUE);

	type = 0;
}


SoundStream::~SoundStream()
{
}


SoundStream::Data::~Data()
{
	if(stream)
	{
		FSOUND_Stream_Close(stream);
		stream = NULL;
	}
}

void SoundStream::setBaseVolume(float value)
{
	assert(data);

	data->baseVolume = value;

	int vol = int(data->baseVolume * data->volume * 255.f);
	if(vol < 0)
		vol = 0;
	if(vol > 255)
		vol = 255;

	if(data->channel >= 0)
		FSOUND_SetVolume(data->channel, vol);
}

void SoundStream::setVolume(float value)
{
	assert(data);

	data->volume = value;

	int vol = int(data->baseVolume * data->volume * 255.f);
	if(vol < 0)
		vol = 0;
	if(vol > 255)
		vol = 255;

	if(data->channel >= 0)
		FSOUND_SetVolume(data->channel, vol);
}

void SoundStream::setPanning(float value)
{
	int panning = int(value * 255.f);
	if(panning < 0)
		panning = 0;
	if(panning > 255)
		panning = 255;

	if(data->channel >= 0)
		FSOUND_SetPan(data->channel, panning);
}

void SoundStream::setLooping(bool loop)
{
	if(data->stream)
	{
		if(loop)
			FSOUND_Stream_SetMode(data->stream, FSOUND_LOOP_NORMAL);
		else
			FSOUND_Stream_SetMode(data->stream, FSOUND_LOOP_OFF);
	}
}

void SoundStream::play()
{
	if(data->channel >= 0)
		FSOUND_SetPaused(data->channel, false);
}

void SoundStream::pause()
{
	if(data->channel >= 0)
		FSOUND_SetPaused(data->channel, TRUE);
}

void SoundStream::stop()
{
	//FSOUND_Stream_Stop
	if(data->channel >= 0)
	{
		FSOUND_SetPaused(data->channel, TRUE);
		FSOUND_Stream_SetTime(data->stream, 0);
	}
}


void SoundStream::setType(int t)
{
	data->type = t;
}


int SoundStream::getType() const
{
	return data->type;
}


bool SoundStream::hasEnded() const
{
	if(!data->stream || data->channel < 0)
		return true;

	//if(int(FSOUND_Stream_GetPosition(stream)) >= FSOUND_Stream_GetLength(stream))
	//	return true;
	//return false;

	if(FSOUND_IsPlaying(data->channel))
		return false;

	return true;
}

const std::string &SoundStream::getFileName() const
{
	return data->filename;
}

/*
  SoundLib
*/


SoundLib::SoundLib()
: data(new Data())
{
}


SoundLib::Data::Data()
:	initialized(false),
	useHardware(false),
	useEax(false),
	mixrate(44100),
	softwareChannels(32),
	minHardwareChannels(16),
	maxHardwareChannels(32),
	speakerType(StereoSpeakers),
	frequencyFactor(1.f)
{
}


SoundLib::~SoundLib()
{
}


SoundLib::Data::~Data()
{
	if(initialized)
		FSOUND_Close();
}

void SoundLib::setProperties(int mixrate_, int softwareChannels_)
{
	data->mixrate = mixrate_;
	data->softwareChannels = softwareChannels_;
}

void SoundLib::setAcceleration(bool useHW, bool useEax_, int minHardwareChannels_, int maxHardwareChannels_)
{
	data->useHardware = useHW;
	data->useEax = useEax_;
	data->minHardwareChannels = minHardwareChannels_;
	data->maxHardwareChannels = maxHardwareChannels_;
}

void SoundLib::setSpeakers(SpeakerType speakerType_)
{
	data->speakerType = speakerType_;
}

void SoundLib::setSoundAPI(const char *api)
{
	if (!api) data->soundAPI = "";
	else data->soundAPI = api;
	
#ifdef _WIN32
	if (data->soundAPI == "winmm") FSOUND_SetOutput(FSOUND_OUTPUT_WINMM);
	else if (data->soundAPI == "dsound") FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
#else
	if (data->soundAPI == "alsa")	FSOUND_SetOutput(FSOUND_OUTPUT_ALSA);
	else if (data->soundAPI == "oss") FSOUND_SetOutput(FSOUND_OUTPUT_OSS);
#endif
	else FSOUND_SetOutput(-1);
}

bool SoundLib::initialize()
{
	assert(!data->initialized);

	//if(useEax && useHardware)
	{

		int driverAmount = FSOUND_GetNumDrivers();
		for(int i = 0; i < driverAmount; ++i)
		{
			unsigned int caps = 0;
			bool eax = false;
			FSOUND_GetDriverCaps(i, &caps);
			{
				if(caps & FSOUND_CAPS_EAX2)
					eax = true;
				if(caps & FSOUND_CAPS_EAX3)
					eax = true;
			}

			string foo = "Driver ";
			foo += lexical_cast<string> (i);
			foo += "(";

			string name = "Null";
			const char *ptr = FSOUND_GetDriverName(i);
			if(ptr)
				name = ptr;

			foo += name;
			foo += ")";
			foo += " -- ";

			if(eax)
				foo += "supports EAX";
			else
				foo += "does NOT support EAX";

			Logger::getInstance()->debug(foo.c_str());
		}
	}

	if(!data->useHardware)
		data->maxHardwareChannels = 0;

	FSOUND_SetMinHardwareChannels(data->minHardwareChannels);
	FSOUND_SetMaxHardwareChannels(data->maxHardwareChannels);
	FSOUND_SetSpeakerMode(data->speakerType);

	char result = FSOUND_Init(data->mixrate, data->softwareChannels, 0);
	if(result)
	{
		data->initialized = true;
		FSOUND_3D_SetRolloffFactor(1);
		FSOUND_3D_SetRolloffFactor(0.f);

		int hwchans = 0;
		FSOUND_GetNumHWChannels(0, &hwchans, 0);
		string foo = "Hardware sound channels initialized: ";
		foo += lexical_cast<string> (hwchans);
		Logger::getInstance()->debug(foo.c_str());

		if(hwchans && data->useEax)
		{
			int output = FSOUND_GetOutput();
			if(output != FSOUND_OUTPUT_DSOUND)
				Logger::getInstance()->debug("Does not use DSOUND!");

			int driver = FSOUND_GetDriver();
			string foo = "Selected sound driver ";
			foo += lexical_cast<string> (driver);
			Logger::getInstance()->debug(foo.c_str());

			{
				EditorParser parser;
#ifdef LEGACY_FILES
				filesystem::InputStream env_file = filesystem::FilePackageManager::getInstance().getFile("Data/Ambient/environment.txt");
#else
				filesystem::InputStream env_file = filesystem::FilePackageManager::getInstance().getFile("data/audio/environment/environment.txt");
#endif
				env_file >> parser;

				ParserGroup &root = parser.getGlobals();
				for(int i = 0; i < root.getSubGroupAmount(); ++i)
				{
					const std::string &name = root.getSubGroupName(i);
					const ParserGroup &group = root.getSubGroup(name);

					FSOUND_REVERB_PROPERTIES p = FSOUND_PRESET_OFF;
					p.Environment = convertFromString<int> (group.getValue("environment"), 0);
					p.EnvSize = convertFromString<float> (group.getValue("env_size"), 7.5f);
					p.EnvDiffusion = convertFromString<float> (group.getValue("env_diffusion"), 1.f);
					p.Room = convertFromString<int> (group.getValue("room"), -1000);
					p.RoomHF = convertFromString<int> (group.getValue("room_hf"), -100);
					p.RoomLF = convertFromString<int> (group.getValue("room_lf"), 0);
					p.DecayTime = convertFromString<float> (group.getValue("decay_time"), 1.49f);
					p.Reflections = convertFromString<int> (group.getValue("reflections"), -2602);
					p.ReflectionsDelay = convertFromString<float> (group.getValue("reflections_delay"), 0.007f);
					p.Reverb = convertFromString<int> (group.getValue("reverb"), 200);
					p.ReverbDelay = convertFromString<float> (group.getValue("reverb_delay"), 0.011f);
					p.EchoTime = convertFromString<float> (group.getValue("echo_time"), 0.25f);
					p.EchoDepth = convertFromString<float> (group.getValue("echo_depth"), 0.f);
					p.ModulationTime = convertFromString<float> (group.getValue("modulation_time"), 0.25f);
					p.ModulationDepth = convertFromString<float> (group.getValue("modulation_depth"), 0.f);
					p.AirAbsorptionHF = convertFromString<float> (group.getValue("air_absorption_hf"), -5.0f);
					p.HFReference = convertFromString<float> (group.getValue("hf_reference"), 5000.f);
					p.LFReference = convertFromString<float> (group.getValue("hf_reference"), 250.f);
					p.RoomRolloffFactor = convertFromString<float> (group.getValue("room_rolloff_factor"), 0.f);
					p.Diffusion = convertFromString<float> (group.getValue("diffusion"), 100.f);
					p.Density = convertFromString<float> (group.getValue("density"), 100.f);

					data->soundAreas[name] = p;
					data->areaAlias[name] = name;
				}
			}

			{
				EditorParser parser;
#ifdef LEGACY_FILES
				filesystem::InputStream alias_file = filesystem::FilePackageManager::getInstance().getFile("Data/Ambient/environment_aliases.txt");
#else
				filesystem::InputStream alias_file = filesystem::FilePackageManager::getInstance().getFile("data/audio/environment/environment_aliases.txt");
#endif
				alias_file >> parser;

				ParserGroup &root = parser.getGlobals();
				for(int i = 0; i < root.getValueAmount(); ++i)
				{
					const std::string &key = root.getValueKey(i);
					const std::string &value = root.getValue(key);

					data->areaAlias[key] = value;
				}
			}
		}
	}

	return data->initialized;
}

void SoundLib::setFrequencyFactor(float scalar)
{
	data->frequencyFactor = scalar;
}

Sound *SoundLib::loadSample(const char *file)
{
	if(!data->initialized)
		return 0;

	if (file == NULL)
	{
		string message = "SoundLib::loadSample - Null filename parameter.";
		Logger::getInstance()->warning(message.c_str());
		return 0;
	}

	// failsafe check for fmod memory leak on failure.
	// (without this, the game will choke on some extensive leaks with bad (stereo?) ambient sounds)
	static std::string lastFailedSampleLoad;
	if (!lastFailedSampleLoad.empty())
	{
		if (lastFailedSampleLoad == file)
		{
			string message = "SoundLib::loadSample - Failsafe, this sample needs to be fixed: ";
			message += file;
			Logger::getInstance()->error(message.c_str());

#ifndef PROJECT_VIEWER
			if (isThisDeveloper())
			{
				// you ain't gonna get past this just ignoring it! --jpk :)

				Logger::getInstance()->error("SoundLib::loadSample - DON'T IGNORE THIS - FIX IT!!!");

				// this should get their attention...
				abort();
			}
#endif

			return 0;
		}
	}

	int flags = FSOUND_LOOP_OFF;
	if(data->useHardware)
		flags |= FSOUND_HW3D;

	// test
	flags |= FSOUND_MONO;

	Sound *result = new Sound(new Sound::Data(file, flags));
	if(result->data->sample)
	{
		lastFailedSampleLoad = "";
		return result;
	}

	if(result->data->fileNotFound)
	{
#ifndef PROJECT_VIEWER
		if (game::SimpleOptions::getBool(DH_OPT_B_SOUND_MISSING_WARNING))
		{
			string message = "SoundLib::loadSample - Sound sample not found: ";
			message += file;
			Logger::getInstance()->warning(message.c_str());
		}
#endif
	} else {
		lastFailedSampleLoad = file;

		string message = "SoundLib::loadSample - Sound sample loading failure: ";
		message += file;
		Logger::getInstance()->warning(message.c_str());
	}

	delete result;
	return 0;
}

SoundStream *SoundLib::createStream(const char *file)
{
	if(!data->initialized)
		return 0;

	SoundStream *result = new SoundStream(new SoundStream::Data(file));
	if(result->data->stream && result->data->channel >= 0)
		return result;

//	string message = "Music stream not found: ";
//	message += file;
//	Logger::getInstance()->warning(message.c_str());

	delete result;
	return 0;
}

int SoundLib::createSound(Sound *sound, int priority)
{
	if(!sound)
		return -1;

	int id = FSOUND_PlaySoundEx(FSOUND_FREE, sound->data->sample, 0, TRUE);
	if(id >= 0)
		FSOUND_SetPriority(id, priority);

	return id;
}

void SoundLib::playSound(int sound)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	if(FSOUND_GetPaused(sound) == TRUE)
	{
		float frequency = getSoundFrequency(sound) * data->frequencyFactor;
		setSoundFrequency(sound, int(frequency + .5f));

		FSOUND_SetPaused(sound, false);
	}
}

void SoundLib::setSoundLoop(int sound, bool loop)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	int flag = loop ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF;
	FSOUND_SetLoopMode(sound, flag);
}

void SoundLib::setSoundPaused(int sound, bool pause)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	FSOUND_SetPaused(sound, pause ? TRUE : FALSE);
}

void SoundLib::setSoundVolume(int sound, float value)
{
	if(sound < 0)
	{
		//assert(!"Invalid sound handle");
		return;
	}

	int volume = int(value * 255.f);
	if(volume < 0)
		volume = 0;
	if(volume > 255)
		volume = 255;

	FSOUND_SetVolume(sound, volume);
}

void SoundLib::setSoundFrequency(int sound, int value)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	FSOUND_SetFrequency(sound, value);
}

void SoundLib::setSound3D(int sound, const VC3 &position, const VC3 &velocity)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	float p[3] = { position.x, position.y, position.z };
	float v[3] = { 0 }; //{ velocity.x, velocity.y, velocity.z };

	FSOUND_3D_SetAttributes(sound, p, v);
}

void SoundLib::stopSound(int sound)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	FSOUND_StopSound(sound);
}

float SoundLib::getSoundVolume(int sound) const
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return 0;
	}

	int volume = FSOUND_GetVolume(sound);
	return volume / 255.f;
}

int SoundLib::getSoundFrequency(int sound) const
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return 0;
	}

	return FSOUND_GetFrequency(sound);
}

int SoundLib::getSoundTime(int sound) const
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return 0;
	}

	int freq = FSOUND_GetFrequency(sound);
	int pos = FSOUND_GetCurrentPosition(sound);

	Uint64 temp = pos * 1000;
	temp /= freq;

	return int(temp);
}

bool SoundLib::isSoundPlaying(int sound) const
{
	if(sound < 0)
	{
		//assert(!"Invalid sound handle");
		return false;
	}

	if(FSOUND_IsPlaying(sound))
		return true;

	return false;
}

void SoundLib::setGlobalVolume(float value)
{
	int volume = int(value * 255.f);
	if(volume < 0)
		volume = 0;
	if(volume > 255)
		volume = 255;

	FSOUND_SetSFXMasterVolume(volume);
}

void SoundLib::setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection)
{
	float p[3] = { position.x, position.y, position.z };
	float v[3] = { 0 }; //{ velocity.x, velocity.y, velocity.z };

	FSOUND_3D_Listener_SetAttributes(p, v, 
		forwardDirection.x, forwardDirection.y, forwardDirection.z,
		upDirection.x, upDirection.y, upDirection.z);
}

void SoundLib::setSoundArea(const std::string &name)
{
	if(!data->useEax)
		return;

	const std::string &alias = data->areaAlias[name];
	map<string, FSOUND_REVERB_PROPERTIES>::iterator it = data->soundAreas.find(alias);
	if(it == data->soundAreas.end())
	{
		string foo = "Sound reverb area not found: ";
		foo += name;

		Logger::getInstance()->warning(foo.c_str());
		return;
	}

	FSOUND_Reverb_SetProperties(&it->second);
}

void SoundLib::update()
{
	FSOUND_Update();
}


signed char __stdcall streamcallback(FSOUND_STREAM *stream, void *buff, int len, void *param);

struct SampleBuffer
{
	Uint64 start;
	Uint64 duration;
	std::vector<char> buffer;

	SampleBuffer()
	:	start(0),
		duration(0)
	{
	}
};

typedef std::list<SampleBuffer> BufferList;
static const int BUFFER_SAMPLES = 2000;
//static const int BUFFER_SAMPLES = 2500;
static const Uint64 BUFFER_TIME_ADD = BUFFER_SAMPLES * 10000 / 2;
//static const Uint64 BUFFER_TIME_ADD = BUFFER_SAMPLES * 10000;

class StormStream: public IStorm3D_Stream
{
public:
	bool stereo;
	int frequency;
	int bits;

	FSOUND_STREAM *stream;
	int channel;

	BufferList buffers;
	int position;

	mutable SDL_mutex *lock;
	int updateTime;
	int updates;
	int sampleSize;
	bool active;

	int time;

	StormStream(bool stereo_, int frequency_, int bits_, float volume)
	:	stereo(stereo_),
		frequency(frequency_),
		bits(bits_),
		stream(0),
		position(0),
		updateTime(0),
		sampleSize(0),
		active(false),
		time(0)
	{
		lock = SDL_CreateMutex();
		if(stereo)
			sampleSize = 2 * (bits / 8);
		else
			sampleSize = (bits / 8);

		updateTime = BUFFER_SAMPLES * 1000 / frequency;

		int type = FSOUND_2D;
		if(stereo)
			type |= FSOUND_STEREO;
		else
			type |= FSOUND_MONO;

		if(bits == 16)
			type |= FSOUND_16BITS;
		else
			type |= FSOUND_8BITS;

		//int data = (int) (void *) (this);
		stream = FSOUND_Stream_Create(&streamcallback, sampleSize * BUFFER_SAMPLES, type, frequency, this);
		if(stream)
		{
			channel = FSOUND_Stream_PlayEx(FSOUND_FREE, stream, 0, TRUE);

			if(channel >= 0)
			{
				int vol = int(volume * 255.f);
				if(vol > 255)
					vol = 255;
				else if(vol < 0)
					vol = 0;

				FSOUND_SetVolume(channel, vol);
				FSOUND_SetPaused(channel, FALSE);
			}
		}

		FSOUND_Update();
	}

	~StormStream()
	{
		if(stream)
			FSOUND_Stream_Close(stream);

		SDL_DestroyMutex(lock);
	}

	void activate()
	{
		if(!active)
		{
			updates = 0;
			time = SDL_GetTicks();
			active = true;
		}
	}

	void deactivate()
	{
		SDL_LockMutex(lock);

		active = false;
		buffers.clear();
		position = 0;

		SDL_UnlockMutex(lock);
	}

	void addSample(const char *buffer_, int length, Uint64 start, Uint64 duration)
	{
		SDL_LockMutex(lock);
		buffers.push_back(SampleBuffer());

		SampleBuffer &sample = buffers.back();
		sample.start = start;
		sample.duration = duration;
		sample.buffer.resize(length);
		
		memcpy(&sample.buffer[0], buffer_, length);
		SDL_UnlockMutex(lock);
	}

	Uint64 getCurrentTime() const
	{
		static const Sint64 delta = (200 * 10000);

		if(!active)
			return 0;

		//int ms = (timeGetTime() - time);
		//__int64 result = __int64(ms) * 10000;
		//return result;

		Sint64 time = ((Sint64) (updates * BUFFER_SAMPLES)) * 1000 * 10000 / frequency;

		if(time < delta)
			return 0;
		return time - delta;
	}
};

signed char __stdcall streamcallback(FSOUND_STREAM *stream, void *buff, int len, void *param) 
{
	StormStream *ptr = (StormStream *) (param);
	SDL_LockMutex(ptr->lock);

	if(ptr->active)
	{
		++ptr->updates;
		int written = 0;
		while(!ptr->buffers.empty() && written < len)
		{
			BufferList::iterator first = ptr->buffers.begin();
			const std::vector<char> &buffer = first->buffer;

			int size = buffer.size();
			int start = ptr->position;
			int end = ptr->position + (len - written);
			if(end > size)
				end = size;

			if(start < size - 1 && end <= size)
			{
				int amount = end - start;
				memcpy((char *)buff + written, &buffer[start], amount);
				written += amount;
				ptr->position += amount;
			}

			if(written < len)
			{
				ptr->position = 0;
				ptr->buffers.erase(first);
			}
		}
	}

	/*
	{
		static float    t1 = 0, t2 = 0;        // time
		static float    v1 = 0, v2 = 0;        // velocity
		signed short   *stereo16bitbuffer = (signed short *) buff;

		for(int count = 0; count < len >> 2; ++count)        // >>2 = 16bit stereo (4 bytes per sample)
		{
			*stereo16bitbuffer++ = (signed short)(sin(t1) * 32767.0f);    // left channel
			*stereo16bitbuffer++ = (signed short)(sin(t2) * 32767.0f);    // right channel

			t1 += 0.01f   + v1;
			t2 += 0.0142f + v2;
			v1 += (float)(sin(t1) * 0.002f);
			v2 += (float)(sin(t2) * 0.002f);
		}
	}
	*/

	SDL_UnlockMutex(ptr->lock);
	return TRUE;
}


boost::shared_ptr<IStorm3D_Stream> SoundLib::createStormStream(bool stereo_, int frequency_, int bits_, float volume)
{
	return boost::shared_ptr<IStorm3D_Stream>(new StormStream(stereo_, frequency_, bits_, volume));
}


} // sfx

