// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_HEIGHTMAP_H
#define INCLUDED_STORM3D_TERRAIN_HEIGHTMAP_H

#include <boost/scoped_ptr.hpp>
#include "DatatypeDef.h"
#include <istorm3d_spotlight.h>

class Storm3D;
class Storm3D_Scene;
class Storm3D_Camera;
class Storm3D_Texture;
struct Storm3D_CollisionInfo;
class ObstacleCollisionInfo;
class Storm3D_Spotlight;

struct Storm3D_TerrainHeightmapData;

namespace util
{
	class AreaMap;
}

class Storm3D_TerrainHeightmap
{
	boost::scoped_ptr<Storm3D_TerrainHeightmapData> data;

public:
	Storm3D_TerrainHeightmap(Storm3D &storm, bool ps13);
	~Storm3D_TerrainHeightmap();

	void setHeightMap(const unsigned short *buffer, const VC2I &resolution, const VC3 &size, int textureDetail, unsigned short *forceMap, int heightmapMultiplier, int obstaclemapMultiplier);
	void setClipMap(const unsigned char *buffer);
	void updateHeightMap(const unsigned short *buffer, const VC2I &start, const VC2I &end);
	void setObstacleHeightmap(const unsigned short *obstacleHeightmap, const util::AreaMap *areaMap);
	void recreateCollisionMap();
	void forcemapHeight(const VC2 &position, float radius, bool above = true, bool below = false);

	unsigned short *getCollisionHeightmap();

	void calculateVisibility(Storm3D_Scene &scene);
	void renderTextures(Storm3D_Scene &scene, bool atiShader);

	enum RenderMode
	{
		Lighting,
		Depth,
		Projection,
	};
	enum RenderType
	{
		Ati,
		Nv
	};

	void renderDepth(Storm3D_Scene &scene, Storm3D_Camera *camera, RenderMode mode, RenderType type, IStorm3D_Spotlight::Type spot_type, Storm3D_Spotlight *spot);
	//void renderDepth(Storm3D_Scene &scene, Storm3D_Camera *camera, bool atiShader, bool atiLightingShader, int spot_type);

	// Texturing
	int addTerrainTexture(Storm3D_Texture &texture);
	void removeTerrainTextures();

	void setBlendMap(int blockIndex, Storm3D_Texture &blend, int textureA, int textureB);
	void setPartialBlendMap(int blockIndex, int subMask, Storm3D_Texture &blend, int textureA, int textureB);
	void resetBlends(int blockIndex);
	void setLightMap(int blockIndex, Storm3D_Texture &map);

	VC3 getNormal(const VC2I &position) const;
	VC3 getFaceNormal(const VC2 &position) const;
	VC3 getInterpolatedNormal(const VC2 &position) const;
	float getHeight(const VC2 &position) const;
	float getPartiallyInterpolatedHeight(const VC2 &position) const;
	VC3 solveObstacleNormal(const VC2I &obstaclePosition, const VC3& fromDirection) const;
	void rayTrace(const VC3 &position, const VC3 &directionNormalized, float rayLength, Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate, bool lineOfSight) const;
};

#endif
