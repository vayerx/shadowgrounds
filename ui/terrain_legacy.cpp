
#include "precompiled.h"

#if 0

#include "terrain_legacy.h"
#include "../util/parser.h"
#include "../filesystem/input_file_stream.h"
#include "istorm3d_texture.h"
#include "istorm3d.h"
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

#ifdef _MSC_EXTENSIONS
#define for if(false) {} else for
#endif

namespace frozenbyte {
namespace ui {
namespace {
	class StormTexture
	{
		IStorm3D_Texture *texture;

		StormTexture &operator = (const StormTexture &);
		StormTexture(const StormTexture &);

	public:
		StormTexture()
		:	texture(0)
		{
		}

		~StormTexture()
		{
			if(texture)
				texture->Release();
		}

		void attach(IStorm3D_Texture *newTexture)
		{
			if(texture)
				texture->Release();

			texture = newTexture;
		}

		IStorm3D_Texture *getTexture()
		{
			return texture;
		}
	};
}

struct TerrainLegacyData
{
	typedef std::map<std::string, boost::shared_ptr<StormTexture> > TextureMap;
	TextureMap paintTextures;

	int baseIndex;
	int indexArray[5];

	VC2I mapSize;
	float terrainScale;

	IStorm3D *storm;
	IStorm3D_Terrain *terrain;

	TerrainLegacyData()
	{
		terrainScale = 1;
		for(int i = 0; i < 5; ++i)
			indexArray[i] = 0;

		storm = 0;
		terrain = 0;
	}
};

TerrainLegacy::TerrainLegacy()
{
	boost::scoped_ptr<TerrainLegacyData> tempData(new TerrainLegacyData());
	data.swap(tempData);
}

TerrainLegacy::~TerrainLegacy()
{
}

void TerrainLegacy::setTexturing(int baseIndex, int *indexArray)
{
	data->baseIndex = baseIndex;
	for(int i = 0; i < 5; ++i)
		data->indexArray[i] = indexArray[i];
}

void TerrainLegacy::setStorm(IStorm3D *storm, IStorm3D_Terrain *terrain)
{
	data->storm = storm;
	data->terrain = terrain;
}

void TerrainLegacy::setProperties(const VC2I &mapSize, float terrainScale)
{
	data->mapSize = mapSize;
	data->terrainScale = terrainScale;
}

void TerrainLegacy::apply(const Parser::ParserGroup &group)
{
	clear();

	const Parser::string_map properties = group.GetProperties();  
	if (!Parser::HasProperty(properties, "map"))
	{
		return;
	}

	std::string paintTexture[5][3];
	int paintIndex[5][3] = { 0 };

	paintTexture[0][0] = Parser::GetString(properties, "level0_paint1");
	paintTexture[1][0] = Parser::GetString(properties, "level1_paint1");
	paintTexture[2][0] = Parser::GetString(properties, "level2_paint1");
	paintTexture[3][0] = Parser::GetString(properties, "level3_paint1");
	paintTexture[4][0] = Parser::GetString(properties, "level4_paint1");
	paintTexture[0][1] = Parser::GetString(properties, "level0_paint2");
	paintTexture[1][1] = Parser::GetString(properties, "level1_paint2");
	paintTexture[2][1] = Parser::GetString(properties, "level2_paint2");
	paintTexture[3][1] = Parser::GetString(properties, "level3_paint2");
	paintTexture[4][1] = Parser::GetString(properties, "level4_paint2");
	paintTexture[0][2] = Parser::GetString(properties, "level0_paint3");
	paintTexture[1][2] = Parser::GetString(properties, "level1_paint3");
	paintTexture[2][2] = Parser::GetString(properties, "level2_paint3");
	paintTexture[3][2] = Parser::GetString(properties, "level3_paint3");
	paintTexture[4][2] = Parser::GetString(properties, "level4_paint3");

	for(int i = 0; i < 5; ++i)
	for(int j = 0; j < 3; ++j)
	{
		std::string &texture = paintTexture[i][j];
		if(texture.empty())
			continue;

		TerrainLegacyData::TextureMap::iterator it = data->paintTextures.find(texture);
		if(it == data->paintTextures.end())
		{
			boost::shared_ptr<StormTexture> stormTexture(new StormTexture());
			stormTexture->attach(data->storm->CreateNewTexture(texture.c_str(), TEXLOADFLAGS_NOCOMPRESS));

			data->paintTextures[texture] = stormTexture;
		}
	}

	int index = data->baseIndex;

//	for(TerrainLegacyData::TextureMap::iterator it = data->paintTextures.begin(); it != data->paintTextures.end(); ++it)
//		data->terrain->LoadTextureAtBank((*it).second.get()->getTexture(), index++);

	for(int i = 0; i < 5; ++i)
	for(int j = 0; j < 3; ++j)
	{
		std::string &texture = paintTexture[i][j];
		int &index = paintIndex[i][j] = -1;

		int loopIndex = -1;
		std::map<std::string, boost::shared_ptr<StormTexture> >::iterator it = data->paintTextures.begin();

		for(; it != data->paintTextures.end(); ++it)
		{
			++loopIndex;
			if(it->first == texture)
			{
				index = loopIndex + data->baseIndex;
				break;
			}
		}
	}

	filesystem::InputStream stream = filesystem::createInputFileStream(Parser::GetString(properties, "map").c_str());

	for(int y = data->mapSize.y*2 - 1; y >= 0; --y)
	for(int x = 0; x < data->mapSize.x*2; ++x)
	{
		VC2 position = VC2((float)(float(x) / 2.0f - (data->mapSize.x / 2)) * data->terrainScale, (float)(float(y) / 2.0f - (data->mapSize.y / 2)) * data->terrainScale);

		unsigned char c = 0;
		for (int p = 0; p < 3; p++)
		{
			stream >> c;

			if (c > 0)
			{
  				// Find right texture
				int texture_index = -1;
				int highest = 0;
				int tmp = 0;

				for (int i = 1; i < 5; ++i)
				{
					/*
					if((tmp = data->terrain->GetTextureAmountAt(data->indexArray[i], position)) > highest)
					{
						texture_index = paintIndex[i][p];
						highest = tmp;
						//break;
					}
					*/

					/*
					std::string &texture = data->textureName[i][0];
					
					int loopIndex = -1;
					int current_index = -1;

					std::map<std::string, boost::shared_ptr<StormTexture> >::iterator it = data->textureMap.begin();
					for(; it != data->textureMap.end(); ++it)
					{
						++loopIndex;
						if(it->first == texture)
						{
							current_index = loopIndex;
							break;
						}
					}

					if(current_index == -1)
						continue;

					if((tmp = data->terrain->GetTextureAmountAt(current_index, position)) > highest)
					{
						texture_index = paintIndex[i][p];
						highest = tmp;
						break;
					}
					*/
				}

				if(texture_index != -1)
				{
					// really need to optimize this stuff.
					// maybe add a method to storm so the texture arrays can be
					// modified more directly...
  				//	data->terrain->Paint(position, 0.5f*data->terrainScale, texture_index, c, c);
				}
			}
		}
	}
}

void TerrainLegacy::clear()
{
	data->paintTextures.clear();
}

} // ui
} // frozenbyte

#endif
