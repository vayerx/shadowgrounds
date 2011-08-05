// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_TEXTURE_GENERATOR_H
#define INCLUDED_EDITOR_TERRAIN_TEXTURE_GENERATOR_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace editor {

class TerrainTextures;
struct Storm;
struct TerrainTextureGeneratorData;

class TerrainTextureGenerator
{
	boost::scoped_ptr<TerrainTextureGeneratorData> data;

public:
	TerrainTextureGenerator(Storm &storm, TerrainTextures &terrainTextures);
	~TerrainTextureGenerator();

	void setTerrainTexture(int index, int textureIndex);
	void setWater(int bottomTextureIndex, float waterHeight);

	void generate();
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
