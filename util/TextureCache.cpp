
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "TextureCache.h"
#include "../system/Logger.h"
#include <IStorm3D.h>
#include "../filesystem/input_stream_wrapper.h"

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>

using namespace std;
using namespace boost;

namespace frozenbyte {

	static const int TEMPORARY_TIME = 20 * 1000;

	struct TextureDeleter
	{
		void operator() (IStorm3D_Texture *t)
		{
			if(t)
				t->Release();
		}
	};

	static void makeLower(std::string &string)
	{
		for(unsigned int i = 0; i < string.size(); ++i)
			string[i] = tolower(string[i]);
	}

	struct TemporaryTexture
	{
		shared_ptr<IStorm3D_Texture> texture;
		int timeLeft;

		explicit TemporaryTexture(shared_ptr<IStorm3D_Texture> &texture_)
		:	texture(texture_),
			timeLeft(TEMPORARY_TIME)
		{
			if(texture)
				texture->AddHighPriority();
		}

		TemporaryTexture(const TemporaryTexture &other)
		:	texture(other.texture),
			timeLeft(other.timeLeft)
		{
			if(texture)
				texture->AddHighPriority();
		}

		~TemporaryTexture()
		{
			if(texture)
				texture->RemoveHighPriority();
		}

		const TemporaryTexture &operator = (const TemporaryTexture &other)
		{
			if(texture)
				texture->RemoveHighPriority();

			texture = other.texture;
			timeLeft = other.timeLeft;

			if(texture)
				texture->AddHighPriority();

			return *this;
		}
	};


	struct TextureData
	{
		char *data;
		size_t data_size;
	};

typedef list<TemporaryTexture> TimedTemporaryList;

struct TextureCache::Data
{
	IStorm3D &storm;

	map<string, shared_ptr<IStorm3D_Texture> > textures;
	map<string, shared_ptr<IStorm3D_Texture> > temporaryTextures;

	// texture data loaded to memory
	map<string, TextureData > textureDatas;

	TimedTemporaryList timedTemporaries;

	int loadflags;

	Data(IStorm3D &storm_)
	:	storm(storm_),
		loadflags(0)
	{
	}

	~Data()
	{
		size_t total = 0;
		map<string, TextureData>::iterator it;
		for(it = textureDatas.begin(); it != textureDatas.end(); it++)
		{
			total += it->second.data_size;
			delete[] it->second.data;
		}
		Logger::getInstance()->debug(("TextureCache preloaded a total of " + boost::lexical_cast<std::string>(total) + " bytes").c_str());
	}

	void loadTexture(string fileName, bool temporaryCache)
	{
		makeLower(fileName);

		// try to find texture data from CPU side memory
		const void *data = NULL;
		size_t data_size = 0;
		map<string, TextureData >::iterator it = textureDatas.find(fileName.c_str());
		if(it != textureDatas.end())
		{
			data = it->second.data;
			data_size = it->second.data_size;
		}

		IStorm3D_Texture *t = storm.CreateNewTexture(fileName.c_str(), loadflags, 0, data, data_size);
		if(!t)
		{
			std::string foo = "TextureCache::loadTexture - Can't find texture: " + std::string(fileName);
			Logger::getInstance()->warning(foo.c_str());

			return;
		}

		shared_ptr<IStorm3D_Texture> texture(t, TextureDeleter());

		if(temporaryCache)
		{
			TemporaryTexture t(texture);
			timedTemporaries.push_back(t);

			if(temporaryTextures.find(fileName) == temporaryTextures.end())
				temporaryTextures[fileName] = texture;
		}
		else
		{
			if(textures.find(fileName) == textures.end())
				textures[fileName] = texture;
		}
	}

	IStorm3D_Texture *getTexture(string fileName)
	{
		makeLower(fileName);

		map<string, shared_ptr<IStorm3D_Texture> >::iterator it = textures.find(fileName);
		if(it != textures.end())
			return it->second.get();

		it = temporaryTextures.find(fileName);
		if(it != temporaryTextures.end())
			return it->second.get();

		return 0;
	}

	void loadTextureData(string fileName)
	{
		makeLower(fileName);

		// already loaded
		if(textureDatas.find(fileName) != textureDatas.end()) return;

		filesystem::FB_FILE *file = filesystem::fb_fopen(fileName.c_str(), "rb");
		if(file)
		{
			TextureData td;
			td.data_size = filesystem::fb_fsize(file);

			if(td.data_size > 0)
			{
				td.data = new char[td.data_size];
				if(filesystem::fb_fread(td.data, 1, td.data_size, file) == td.data_size)
				{
					textureDatas[fileName] = td;
				}
				else
				{
					delete[] td.data;
				}
			}

			filesystem::fb_fclose(file);
		}
	}
};

TextureCache::TextureCache(IStorm3D &storm)
:	data(new Data(storm))
{
}

TextureCache::~TextureCache()
{
}

void TextureCache::loadTexture(const char *fileName, bool temporaryCache)
{
	data->loadTexture(fileName, temporaryCache);
}

void TextureCache::clearTemporary()
{
	data->temporaryTextures.clear();
	data->timedTemporaries.clear();
}

void TextureCache::update(int ms)
{
	TimedTemporaryList::iterator it = data->timedTemporaries.begin();
	for(; it != data->timedTemporaries.end(); )
	{
		TemporaryTexture &t = *it;
		t.timeLeft -= ms;

		if(t.timeLeft <= 0)
		{
			it = data->timedTemporaries.erase(it);
			continue;
		}

		++it;
	}
}

IStorm3D_Texture *TextureCache::getTexture(const char *fileName, bool temporaryCache)
{
	IStorm3D_Texture *result = data->getTexture(fileName);

	if(!result)
	{
		loadTexture(fileName, temporaryCache);
		result = data->getTexture(fileName);
	}

	return result;
}

// loads texture data to cpu side memory
void TextureCache::loadTextureDataToMemory(const char *fileName)
{
	data->loadTextureData(fileName);
}

void TextureCache::setLoadFlags(int fag)
{
	data->loadflags = fag;
}

int TextureCache::getLoadFlags()
{
	return data->loadflags;
}

} // frozenbyte
