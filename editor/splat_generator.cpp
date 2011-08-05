// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "splat_generator.h"
#include "storm.h"
#include "terrain_textures.h"

namespace frozenbyte {
namespace editor {
namespace {
	VC2I sceneToTexturing(const VC3 &source, const VC2I &mapSize, const VC3 &realSize)
	{
		VC2I result;
		result.x = mapSize.x / 2;
		result.y = mapSize.y / 2;

		result.x += int(source.x * mapSize.x / realSize.x);
		result.y += int(source.z * mapSize.y / realSize.z);

		return result;
	}

	VC2I unitToTexturing(const VC3 &source, const VC2I &mapSize, const VC3 &realSize)
	{
		VC2I result;

		result.x = int(source.x * mapSize.x / realSize.x);
		result.y = int(source.z * mapSize.y / realSize.z);

		return result;
	}

} // unnamed


struct SplatGeneratorData
{
	Storm &storm; 
	TerrainTextures &textures;

	SplatGeneratorData(Storm &storm_, TerrainTextures &textures_)
	:	storm(storm_),
		textures(textures_)
	{
	}

	void generate(int textureId, const VC2I &position, const VC2I &size, float strength)
	{
		TextureSplat splat(position, size);
		float radius = sqrtf(float(size.x/2 * size.x/2 + size.y/2 * size.y/2));

		for(int j = 0; j < size.y; ++j)
		for(int i = 0; i < size.x; ++i)
		{
			int x = abs(size.x / 2 - i);
			int y = abs(size.y / 2 - j);

			float distance = sqrtf(float(x*x + y*y)) / radius;
			float norm = strength * (1.f - distance);

			splat.weights[j * size.x + i] = unsigned char(norm * 255);
		}

		textures.addSplat(textureId, splat);
	}
};

SplatGenerator::SplatGenerator(Storm &storm, TerrainTextures &textures)
{
	boost::scoped_ptr<SplatGeneratorData> tempData(new SplatGeneratorData(storm, textures));
	data.swap(tempData);
}

SplatGenerator::~SplatGenerator()
{
}

void SplatGenerator::generate(int textureId, const VC3 &position, int size, float strength)
{
	VC2I splatPosition = sceneToTexturing(position, data->textures.getMapSize(), data->storm.heightmapSize);
	VC2I splatSize = unitToTexturing(VC3(float(size * 2), 0, float(size * 2)), data->textures.getMapSize(), data->storm.heightmapSize);
	//VC2I splatSize(5, 5); 

	splatPosition -= splatSize / 2;
	data->generate(textureId, splatPosition, splatSize, strength);
}

} // editor
} // frozenbyte
