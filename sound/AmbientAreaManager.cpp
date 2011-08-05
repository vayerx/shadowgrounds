#include "precompiled.h"

#include <boost/lexical_cast.hpp>
#include <fstream>

// Copyright 2002-2004 Frozenbyte Ltd.

#include "AmbientAreaManager.h"
#include "SoundMixer.h"
#include "SoundLib.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../system/Logger.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <string>

using namespace std;
using namespace boost;
using namespace frozenbyte;
using namespace frozenbyte::editor;

#ifdef LEGACY_FILES
#define DEFAULT_AMBIENT_AREA "Default"
#define DEFAULT_ENVIRONMENT_AREA "Default"
#define INDOOR_GROUP_NAME "Indoor"
#define OUTDOOR_GROUP_NAME "Outdoor"
#define AMBIENT_GROUP_NAME "Ambient"
#define RANDOM_GROUP_NAME "Random"
#else
#define DEFAULT_AMBIENT_AREA "default"
#define DEFAULT_ENVIRONMENT_AREA "default"
#define INDOOR_GROUP_NAME "indoor"
#define OUTDOOR_GROUP_NAME "outdoor"
#define AMBIENT_GROUP_NAME "ambient"
#define RANDOM_GROUP_NAME "random"
#endif

namespace sfx {

	struct Stream
	{
		enum Mode
		{
			FadeIn,
			Normal,
			FadeOut
		};

		string filename;
		shared_ptr<SoundStream> stream;

		Mode mode;
		int time;
		int fadeTime;

		int volumeTime;
		int volumeFadeTime;
		bool looped;

		float oldMaxVolume;
		float maxVolume;

		Stream(boost::shared_ptr<SoundStream> stream_, float volume, float panning)
		:	stream(stream_),
			mode(Normal),
			time(0),
			fadeTime(1),
			volumeTime(1),
			volumeFadeTime(1),
			looped(false),
			oldMaxVolume(volume),
			maxVolume(volume)
		{
			if(!stream)
				return;

			stream->setVolume(volume);
			stream->setLooping(false);
			stream->setPanning(panning);
			stream->play();
		}

		Stream(boost::shared_ptr<SoundStream> stream_, int fadeTime_, float maxVolume_, const std::string &file)
		:	filename(file),
			stream(stream_),
			mode(FadeIn),
			time(0),
			fadeTime(fadeTime_),
			volumeTime(fadeTime_),
			volumeFadeTime(fadeTime_),
			looped(true),
			oldMaxVolume(maxVolume_),
			maxVolume(maxVolume_)
		{
			if(!stream)
				return;

			/*
			string foo = "Creating new stream ";
			foo += filename;
			Logger::getInstance()->warning(foo.c_str());
			*/

			stream->setVolume(0.f);
			stream->setLooping(true);
			stream->play();
		}

		~Stream()
		{
		}

		float getMaxFactor() const
		{
			float position = float(volumeTime) / float(volumeFadeTime);
			if(position > 1.f)
				position = 1.f;

			float maxVolumeDiff = maxVolume - oldMaxVolume;
			float currentMaxVolume = oldMaxVolume + position * maxVolumeDiff;
			if(currentMaxVolume > 1.f)
				currentMaxVolume = 1.f;
			if(currentMaxVolume < 0.f)
				currentMaxVolume = 0.f;

			return currentMaxVolume;
		}

		float getVolumeFactor() const
		{
			float volume = 1.f;
			if(mode != Normal)
			{
				volume = float(time) / float(fadeTime);
				if(volume > 1.f)
					volume = 1.f;
			}

			return volume;
		}

		float getFactor() const
		{
			/*
			float volume = 1.f;
			if(mode != Normal)
			{
				volume = float(time) / float(fadeTime);
				if(volume > 1.f)
					volume = 1.f;

				if(mode == FadeOut)
					volume = 1.f - volume;
			}
			*/

			float volume = getVolumeFactor();
			if(mode == FadeOut)
				volume = 1.f - volume;

			volume *= getMaxFactor();
			return volume;
		}

		bool hasEnded() const
		{
			if(!stream)
				return true;

			if(mode == FadeOut && time >= fadeTime)
				return true;

			if(stream->hasEnded())
					return true;

			return false;
		}

		void update(int ms)
		{
			if(!stream)
				return;

			//if(mode != FadeOut && volumeTime < volumeFadeTime)
			//	volumeTime += ms;
			if(volumeTime < volumeFadeTime)
				volumeTime += ms;

			float volume = getFactor();
			if(mode != Normal)
			{
				time += ms;
				if(time >= fadeTime)
				{
					if(mode == FadeIn)
					{
						//time = 0;
						//stream->setVolume(maxVolume);
						//volume = getMaxFactor();
						mode = Normal;
					}
					else
					{
						//stream->setVolume(0.f);
						volume = 0;
						stream->stop();
						stream.reset();
					}
				}
			}

			if(stream)
				stream->setVolume(volume);
		}

		void fadeIn(int ms, float newMaxVolume)
		{
			if(mode != Normal)
			{
				if(mode == FadeIn)
				{
					if(ms < fadeTime)
					{
						time = int(getVolumeFactor() * ms);
						fadeTime = ms;
					}
				}
				else if(mode == FadeOut)
				{
					time = int((1.f - getVolumeFactor()) * ms);
					fadeTime = ms;
				}

				mode = FadeIn;
			}

			{
				/*
				string foo = "Fading in old stream ";
				foo += filename;
				Logger::getInstance()->warning(foo.c_str());
				*/

				float maxFactor = getMaxFactor();
				oldMaxVolume = maxFactor;
				maxVolume = newMaxVolume;

				volumeTime = 0;
				volumeFadeTime = ms;
			}

		}

		void fadeOut(int ms)
		{
			if(mode == Normal)
			{
				time = 0;
				fadeTime = ms;
			}
			else if(mode == FadeIn)
			{
				time = int((1.f - getVolumeFactor()) * ms);
				fadeTime = ms;
			}
			else if(mode == FadeOut)
			{
				if(ms < fadeTime)
				{
					time = int(getVolumeFactor() * ms);
					fadeTime = ms;
				}
			}

			/*
			string foo = "Fading out old stream ";
			foo += filename;
			foo += ", ";
			foo += lexical_cast<string> (ms);
			Logger::getInstance()->warning(foo.c_str());
			*/

			mode = FadeOut;
		}
	};

	typedef vector<shared_ptr<Stream> > AmbientStreamList;

	// .......

	typedef vector<string> StringList;

	struct Ambient
	{
		string file;
		int fadeTime;
		float volume;

		Ambient()
		:	fadeTime(1000),
			volume(1.f)
		{
		}

		void load(const ParserGroup &group)
		{
			file = group.getValue("file");
			fadeTime = convertFromString<int> (group.getValue("fade_time"), 100);
			volume = convertFromString<int> (group.getValue("volume"), 100) / 100.f;
		}
	};

	struct Random
	{
		int minTime;
		int maxTime;
		int volumeVar;
		int volume;

		StringList files;
		int randomPanning;

		Random()
		:	minTime(0),
			maxTime(0),
			volumeVar(0),
			randomPanning(0)
		{
		}

		void load(const ParserGroup &group)
		{
			minTime = convertFromString<int> (group.getValue("min_time"), 10000);
			maxTime = convertFromString<int> (group.getValue("max_time"), 60000);
			volume = convertFromString<int> (group.getValue("volume"), 100);
			volumeVar = convertFromString<int> (group.getValue("volume_var"), 20);
			randomPanning = convertFromString<int> (group.getValue("random_panning"), 20);

			for(int i = 0; i < group.getLineCount(); ++i)
			{
				const std::string &file = group.getLine(i);
				if(ifstream(file.c_str()).good())
					files.push_back(file);
				else
				{
					string message = "Random ambient file not found: ";
					message += file;

					Logger::getInstance()->error(message.c_str());
				}
			}
		}
	};

	struct AreaInfo
	{
		Ambient ambient;
		Random random;
		string soundArea;

		int time;
		int randomTime;
		mutable int randomIndex;

		AreaInfo()
		:	time(0),
			randomTime(50),
			randomIndex(0)
		{
		}

		void load(const ParserGroup &group)
		{
			ambient.load(group.getSubGroup(AMBIENT_GROUP_NAME));
			random.load(group.getSubGroup(RANDOM_GROUP_NAME));

			randomTime = random.minTime;
			soundArea = group.getValue("type", DEFAULT_ENVIRONMENT_AREA);
		}

		void makeRandomTime()
		{
			int delta = random.maxTime - random.minTime;
			if(delta > 0)
				randomTime = (rand() % delta) + random.minTime;
		}

		const std::string &getRandomSound() const
		{
			static string empty;
			if(random.files.empty())
				return empty;

			if(randomIndex >= int(random.files.size()))
				randomIndex = 0;

			return random.files[randomIndex++];
		}

		float getRandomVolume() const
		{
			float factor = float(random.volume) * .01f;
			if(random.volumeVar > 0)
			{
				int value = -random.volumeVar/2 + rand() % random.volumeVar;
				factor += (float(value) * .01f);
				if(factor < 0.0f) factor = 0.0f;
				if(factor > 1.0f) factor = 1.0f;
			}

			return factor;
		}

		float getRandomPanning() const
		{
			if(random.randomPanning)
			{
				/*
				float factor = float(rand() % random.randomPanning) * .01f * .5f;
				if(rand() % 2)
					return .5f + factor;
				else
					return .5f - factor;
				*/
				int a = rand() % 4;
				if(a == 0)
					return 0.25f;
				else if(a == 1)
					return 0.f;
				else if(a == 2)
					return 1.f;
				else if(a == 2)
					return .8f;
			}

			return .5f;
		}
	};

	typedef map<string, AreaInfo> AmbientAreaMap;

	struct ListInfo
	{
		AmbientAreaMap areas;
		string active;

		ListInfo()
		:	active(DEFAULT_AMBIENT_AREA)
		{
		}

		void load(const string &groupName, const ParserGroup &group)
		{
			areas.clear();
			active = DEFAULT_AMBIENT_AREA;

			int areaAmount = group.getSubGroupAmount();
			for(int i = 0; i < areaAmount; ++i)
				areas[group.getSubGroupName(i)].load(group.getSubGroup(i));

			AreaInfo silent;
			silent.ambient.fadeTime = 333;
			areas["VerySpecialSilentArea"] = silent;

			if(areas.find(DEFAULT_AMBIENT_AREA) == areas.end())
				Logger::getInstance()->warning("Unable to find default ambient group");
		}
	};


struct AmbientAreaManager::Data
{
	SoundMixer *mixer;
	ListInfo list[2];
	string oldActives[2];
	string eaxArea[2];

	AreaType activeArea;
	AmbientStreamList ambients;
	AmbientStreamList randoms;

	bool active;
	bool ambientEnabled;

	Data(SoundMixer *mixer_)
	:	mixer(mixer_),
		activeArea(Outdoor),
		active(false),
		ambientEnabled(true)
	{
	}

	void load(const std::string &file)
	{
		activeArea = Outdoor;

		EditorParser parser;
		filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(file);
		fileStream >> parser;

		const ParserGroup &group = parser.getGlobals();
		list[Outdoor].load(OUTDOOR_GROUP_NAME , group.getSubGroup(OUTDOOR_GROUP_NAME));
		list[Indoor].load(INDOOR_GROUP_NAME , group.getSubGroup(INDOOR_GROUP_NAME));
	}

	void applyArea()
	{
		if(!mixer || !active)
			return;
		if(!ambientEnabled)
			return;

		ListInfo &info = list[activeArea];
		AreaInfo &area = info.areas[info.active];

		{
			int fadeTime = area.ambient.fadeTime;
			float maxVolume = area.ambient.volume;
			const std::string &file = area.ambient.file;

			// Fade out old ambients
			AmbientStreamList::iterator it = ambients.begin();
			AmbientStreamList::iterator existing = ambients.end();

			for(; it != ambients.end(); ++it)
			{
				if(!file.empty() && (*it)->stream && (*it)->filename == file)
				{
					//if((*it)->mode != Stream::FadeOut && fabsf(maxVolume - (*it)->maxVolume) < 0.01f)
					//	return;

					if(existing == ambients.end())
						existing = it;
				}

				if(it != existing)
					(*it)->fadeOut(fadeTime);
			}

			// Fade in new ambient
			if(!file.empty())
			{
				if(existing != ambients.end())
				{
					(*existing)->fadeIn(fadeTime, maxVolume);
				}
				else
				{
					shared_ptr<Stream> ptr(new Stream(mixer->getStream(area.ambient.file.c_str(), SoundMixer::SOUNDSTREAMTYPE_AMBIENT), fadeTime, maxVolume, file));

					if(ptr)
					{
						ambients.push_back(ptr);
					}
					else
					{
						Logger::getInstance()->warning("Unable to load ambient stream");
						Logger::getInstance()->warning(area.ambient.file.c_str());
					}
				}
			}
		}
	}

	void applyEax()
	{
		if(mixer)
			mixer->setSoundArea(eaxArea[activeArea]);
	}

	void enterArea(AreaType type)
	{
		if(activeArea != type)
		{
			activeArea = type;
			applyArea();
			applyEax();
		}
	}

	void setArea(AreaType type, const std::string &name)
	{
		if(list[type].active != name)
		{
			if(list[type].areas.find(name) == list[type].areas.end())
			{
				string foo("Ambient area not defined: ");
				foo += name;
				Logger::getInstance()->error(foo.c_str());
			}

			list[type].active = name;

			if(activeArea == type)
				applyArea();
		}
	}

	void setEaxArea(AreaType type, const std::string &name)
	{
		if(eaxArea[type] != name)
		{
			eaxArea[type] = name;
			if(activeArea == type)
				applyEax();
		}
	}

	void fadeIn()
	{
		active = true;
		applyArea();
	}

	void update(int ms)
	{
		// Fade & remove already finished streams
		{
			AmbientStreamList::iterator it = ambients.begin();
			for(; it != ambients.end(); )
			{
				Stream *ptr = (*it).get();
				ptr->update(ms);

				if(ptr->hasEnded())
					it = ambients.erase(it);
				else
					++it;
			}

			it = randoms.begin();
			for(; it != randoms.end(); )
			{
				Stream *ptr = (*it).get();
				ptr->update(ms);

				if(ptr->hasEnded())
				{
					it = randoms.erase(it);
				}
				else
					++it;
			}
		}

		ListInfo &info = list[activeArea];
		AreaInfo &area = info.areas[info.active];

		if(randoms.empty() && active && mixer)
		{
			area.time += ms;
			if(area.time > area.randomTime)
			{
				// Create random ambient 
				const std::string &file = area.getRandomSound();
				float volume = area.getRandomVolume();
				float panning = area.getRandomPanning();

				if(!file.empty())
				{
					shared_ptr<Stream> ptr(new Stream (mixer->getStream(file.c_str(), SoundMixer::SOUNDSTREAMTYPE_AMBIENT), volume, panning));
					if(ptr)
						randoms.push_back(ptr);
					else
					{
						Logger::getInstance()->warning("Unable to load random ambient stream");
						Logger::getInstance()->warning(file.c_str());
					}
				}

				area.time = 0;
				area.makeRandomTime();
			}
		}
	}

	void fadeOut(int ms)
	{
		AmbientStreamList::iterator it = ambients.begin();
		for(; it != ambients.end(); ++it)
			(*it)->fadeOut(ms);

		it = randoms.begin();
		for(; it != randoms.end(); ++it)
			(*it)->fadeOut(ms);

		active = false;
	}
};

AmbientAreaManager::AmbientAreaManager(SoundMixer *mixer)
{
	scoped_ptr<Data> tempData(new Data(mixer));
	data.swap(tempData);
}

AmbientAreaManager::~AmbientAreaManager()
{
}

void AmbientAreaManager::loadList(const std::string &file)
{
	data->load(file);
}

void AmbientAreaManager::enterArea(AreaType type)
{
	data->enterArea(type);
}

void AmbientAreaManager::setArea(AreaType type, const std::string &name)
{
	data->setArea(type, name);
}

void AmbientAreaManager::setEaxArea(AreaType type, const std::string &name)
{
	data->setEaxArea(type, name);
}

void AmbientAreaManager::fadeIn()
{
	data->fadeIn();
}

void AmbientAreaManager::update(int ms)
{
	if(ms > 100)
		ms = 100;

	data->update(ms);
}

void AmbientAreaManager::fadeOut(int ms)
{
	data->fadeOut(ms);
}

void AmbientAreaManager::enableAmbient()
{
	if(data->ambientEnabled)
		return;

	data->setArea(Outdoor, data->oldActives[0]);
	data->setArea(Indoor, data->oldActives[1]);

	data->ambientEnabled = false;
}

void AmbientAreaManager::disableAmbient()
{
	if(!data->ambientEnabled)
		return;

	data->ambientEnabled = false;

	data->oldActives[0] = data->list[0].active;
	data->oldActives[1] = data->list[1].active;

	data->setArea(Outdoor, "VerySpecialSilentArea");
	data->setArea(Indoor, "VerySpecialSilentArea");
}

} // sfx
