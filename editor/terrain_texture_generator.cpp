// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_texture_generator.h"
#include "terrain_textures.h"
#include "storm.h"
#include <vector>
#include <istorm3d_terrain.h>

namespace frozenbyte {
namespace editor {

struct TerrainTextureGeneratorData
{
	Storm &storm;
	TerrainTextures &terrainTextures;

	int terrainTexture[2];
	int bottomTexture;
	float waterHeight;

	TerrainTextureGeneratorData(Storm &storm_, TerrainTextures &terrainTextures_)
	:	storm(storm_),
		terrainTextures(terrainTextures_)
	{
		terrainTexture[0] = terrainTexture[1] = -1;
		bottomTexture = -1;
		waterHeight = 0;
	}

	void generate()
	{
		VC2I blendSize = storm.heightmapResolution / IStorm3D_Terrain::BLOCK_SIZE;
		blendSize *= IStorm3D_Terrain::BLOCK_SIZE - 1;
		blendSize += VC2I(1, 1);

		terrainTextures.resetBlendMap(blendSize);

		VC2I splatSize = blendSize;
		TextureSplat baseSplat(VC2I(0, 0), splatSize);
		TextureSplat splat(VC2I(0, 0), splatSize);

		for(int y = 0; y < splatSize.y; ++y)
		for(int x = 0; x < splatSize.x; ++x)
		{
			int position = y * splatSize.x + x;
			int xb = 5 - x % 10;
			int yb = 5 - y % 10;
			
			xb = int(sqrtf(float(xb * xb)));
			yb = int(sqrtf(float(yb * yb)));

			int weight = 255 * (yb + xb) / 10;

			baseSplat.weights[position] = 255;
			splat.weights[position] = weight;
		}

		terrainTextures.addSplat(terrainTexture[0], baseSplat);
		if(terrainTexture[0] != terrainTexture[1])
			terrainTextures.addSplat(terrainTexture[1], splat);
	}
};

TerrainTextureGenerator::TerrainTextureGenerator(Storm &storm, TerrainTextures &terrainTextures)
{
	boost::scoped_ptr<TerrainTextureGeneratorData> tempData(new TerrainTextureGeneratorData(storm, terrainTextures));
	data.swap(tempData);
}

TerrainTextureGenerator::~TerrainTextureGenerator()
{
}

void TerrainTextureGenerator::setTerrainTexture(int index, int textureIndex)
{
	assert(index >= 0 && index < 2);
	assert(textureIndex >= 0 && textureIndex < 128);

	data->terrainTexture[index] = textureIndex;
}

void TerrainTextureGenerator::setWater(int bottomTextureIndex, float waterHeight)
{
	assert(bottomTextureIndex >= -1 && bottomTextureIndex < 128);
	assert(waterHeight >= 0);

	data->bottomTexture = bottomTextureIndex;
	data->waterHeight = waterHeight;
}

void TerrainTextureGenerator::generate()
{
	if(data->terrainTexture[0] == -1 || data->terrainTexture[1] == -1)
		return;

	data->generate();
}

} // end of namespace editor
} // end of namespace frozenbyte
