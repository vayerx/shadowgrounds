// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_LOD_H
#define INCLUDED_STORM3D_TERRAIN_LOD_H

#include "DatatypeDef.h"
#include <boost/scoped_ptr.hpp>

class Storm3D;
class Storm3D_Scene;
struct Storm3D_TerrainLodData;

class Storm3D_TerrainLod
{
	boost::scoped_ptr<Storm3D_TerrainLodData> data;

public:
	Storm3D_TerrainLod(Storm3D &storm);
	~Storm3D_TerrainLod();

	void generate(int resolution, unsigned char *clipBuffer = 0);
	void setBlockRadius(float size);

	void render(Storm3D_Scene &scene, int subMask, float range, float rangeX1, float rangeY1, float rangeX2, float rangeY2);
};

#endif
