
#include "precompiled.h"

#include "BuildingBlinker.h"
#include <string>
#include <vector>
#include <map>
#include <cassert>

using namespace std;

namespace util {
namespace {

	// How do we know which lights self illuminations to animate?
	// Some relation between animated lightmap and object/material names?

	// And .. Which lights are on when?

	struct MaterialData
	{
	};

	struct BlinkData
	{
		int time;

		string texture;
		vector<MaterialData> materials;

		BlinkData()
		:	time(0)
		{
		}

		BlinkData(int time_, const string &texture_)
		:	time(time_),
			texture(texture_)
		{
		}
	};

	struct Blinks
	{
		int currentTime;
		vector<BlinkData> blinkList;

		Blinks()
		:	currentTime(0)
		{
		}
	};
};

struct BuildingBlinker::Data
{
	frozenbyte::TextureCache &cache;
	map<string, Blinks> textureBlinks;

	Data(frozenbyte::TextureCache &cache_)
	:	cache(cache_)
	{
	}

	void addBlinkTexture(const char *original, const char *to, int delta)
	{
		Blinks &blinks = textureBlinks[original];
		vector<BlinkData> &blinkList = blinks.blinkList;

		BlinkData blink(delta, to);
		if(!blinkList.empty())
			blink.time += blinkList[blinkList.size() - 1].time;

		// ToDo: lights

		blinkList.push_back(blink);
	}

	void switchTo(const string &texture, BlinkData &blink)
	{
	}

	void update(int timeDelta)
	{
		map<string, Blinks>::iterator it = textureBlinks.begin();
		for(; it != textureBlinks.end(); ++it)
		{
			Blinks &blinks = it->second;
			vector<BlinkData> blinkList = blinks.blinkList;

		}

		//currentTime += timeDelta;
	}

	int getIndex(const Blinks &blinks, int time) const
	{
		vector<BlinkData> blinkList = blinks.blinkList;

		for(unsigned int i = 1; i < blinkList.size(); ++i)
		{
			BlinkData &blink = blinkList[i];
			if(time < blink.time)
				continue;

			//if(i < textureBlinks.size() - 1)
		}

		return 0;
	}
};

BuildingBlinker::BuildingBlinker(frozenbyte::TextureCache &cache)
:	data(new Data(cache))
{
}

BuildingBlinker::~BuildingBlinker()
{
}

void BuildingBlinker::addBlinkTexture(const char *original, const char *to, int delta)
{
	assert(original && to && delta > 0);
	data->addBlinkTexture(original, to, delta);
}

void BuildingBlinker::update(int timeDelta)
{
	assert(timeDelta >= 0);
	data->update(timeDelta);
}

} // utils
