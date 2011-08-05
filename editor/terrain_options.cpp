// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_options.h"
#include "storm_texture.h"
#include "storm.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"

#include <istorm3d_terrain.h>
#include <boost/shared_ptr.hpp>
#include <cassert>

namespace frozenbyte {
namespace editor {

struct TerrainOptionsData
{
	Storm &storm;

	boost::shared_ptr<IStorm3D_Texture> detailTexture[2];
	std::string detailTextureName[2];
	int detailRepeat[2];

	int height[3];
	int slopeDivider;
	int slopeStart;

	TerrainOptionsData(Storm &storm_)
	:	storm(storm_)
	{
		clear();
	}

	void apply()
	{
	}

	void clear()
	{
		detailTextureName[0] = "";
		detailTextureName[1] = "";

		detailRepeat[0] = 3;
		detailRepeat[1] = 5;

		height[0] = 12800;
		height[1] = 22600;
		height[2] = 38400;
		slopeDivider = 2000;
		slopeStart = 15;
	}
};

TerrainOptions::TerrainOptions(Storm &storm)
{
	boost::scoped_ptr<TerrainOptionsData> tempData(new TerrainOptionsData(storm));
	data.swap(tempData);
}

TerrainOptions::~TerrainOptions()
{
}

void TerrainOptions::applyToTerrain()
{
	data->apply();
}

void TerrainOptions::clear()
{
	data->clear();
	data->detailTexture[0].reset();
	data->detailTexture[1].reset();
}

void TerrainOptions::setDetailTexture(int index, const std::string &textureName)
{
	assert((index >= 0) && (index <= 1));

	data->detailTexture[index] = loadTexture(textureName, data->storm);
	data->detailTextureName[index] = textureName;
}

bool TerrainOptions::setDetailRepeat(int index, int value)
{
	assert((index >= 0) && (index <= 1));
	if(data->detailRepeat[index] == value)
		return false;

	data->detailRepeat[index] = value;
	return true;
}

bool TerrainOptions::setHeight(int index, int value)
{
	assert((index >= 0) && (index <= 2));
	if(data->height[index] == value)
		return false;
	
	data->height[index] = value;
	return true;
}

bool TerrainOptions::setSlopeDivider(int value)
{
	if(data->slopeDivider == value)
		return false;
	
	data->slopeDivider = value;
	return true;
}

bool TerrainOptions::setSlopeStart(int value)
{
	if(data->slopeStart == value)
		return false;

	data->slopeStart = value;
	return true;
}

std::string TerrainOptions::getDetailTexture(int index) const
{
	assert((index >= 0) && (index <= 1));
	return data->detailTextureName[index];
}

int TerrainOptions::getDetailRepeat(int index) const
{
	assert((index >= 0) && (index <= 1));
	return data->detailRepeat[index];
}

int TerrainOptions::getHeight(int index) const
{
	assert((index >= 0) && (index <= 2));
	return data->height[index];
}

int TerrainOptions::getSlopeDivider() const
{
	return data->slopeDivider;
}

int TerrainOptions::getSlopeStart() const
{
	return data->slopeStart;
}

void TerrainOptions::doExport(Exporter &exporter) const
{
	ExporterScene &scene = exporter.getScene();

	/*
	scene.setHeight(0, getHeight(0));
	scene.setHeight(1, getHeight(1));
	scene.setHeight(2, getHeight(2));
	scene.setSlopeDivider(getSlopeDivider());
	scene.setSlopeStart(getSlopeStart());
	
	scene.setDetailTexture(0, getDetailTexture(0));
	scene.setDetailTexture(1, getDetailTexture(1));
	scene.setDetailRepeat(0, getDetailRepeat(0));
	scene.setDetailRepeat(1, getDetailRepeat(1));
	*/
}

filesystem::OutputStream &TerrainOptions::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(0);

	for(int i = 0; i < 2; ++i)
		stream << data->detailTextureName[i] << data->detailRepeat[i];

	stream << data->height[0] << data->height[1] << data->height[2];
	stream << data->slopeDivider << data->slopeStart;

	return stream;
}

filesystem::InputStream &TerrainOptions::readStream(filesystem::InputStream &stream)
{
	data->clear();

	int version = 0;
	stream >> version;

	for(int i = 0; i < 2; ++i)
	{
		stream >> data->detailTextureName[i] >> data->detailRepeat[i];
		if(!data->detailTextureName[i].empty())
			data->detailTexture[i] = loadTexture(data->detailTextureName[i], data->storm);
	}

	stream >> data->height[0] >> data->height[1] >> data->height[2];
	stream >> data->slopeDivider >> data->slopeStart;
	
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
