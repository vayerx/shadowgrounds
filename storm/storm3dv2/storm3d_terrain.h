// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_H
#define INCLUDED_STORM3D_TERRAIN_H

#include <IStorm3D_Terrain.h>
#include <boost/scoped_ptr.hpp>

class Storm3D;
class Storm3D_Scene;
class Storm3D_TerrainModels;
struct Storm3D_TerrainData;

namespace util
{
	class AreaMap;
}

class Storm3D_Terrain: public IStorm3D_Terrain
{
	boost::scoped_ptr<Storm3D_TerrainData> data;

public:
	Storm3D_Terrain(Storm3D &storm);
	~Storm3D_Terrain();

	void setHeightMap(const unsigned short *buffer, const VC2I &resolution, const VC3 &size, int textureDetail, unsigned short *forceMap, int heightmapMultiplier, int obstaclemapMultiplier);
	void setClipMap(const unsigned char *buffer);
	void updateHeightMap(const unsigned short *buffer, const VC2I &start, const VC2I &end);
	void setObstacleHeightmap(const unsigned short *obstacleHeightmap, const util::AreaMap *areaMap);
	void recreateCollisionMap();
	void forcemapHeight(const VC2 &position, float radius, bool above = true, bool below = false);

	unsigned short *getCollisionHeightmap();

	// Texturing
	int addTerrainTexture(IStorm3D_Texture &texture);
	void removeTerrainTextures();
	void setBlendMap(int blockIndex, IStorm3D_Texture &blend, int textureA, int textureB);
	void resetBlends(int blockIndex);
	void setLightMap(int blockIndex, IStorm3D_Texture &map);

	// Terrain objects
	int addModel(boost::shared_ptr<IStorm3D_Model> model, boost::shared_ptr<IStorm3D_Model> fadeModel, const std::string &bones, const std::string &idleAnimation);
	void removeModels();
	int addInstance(int modeld, const VC3 &position, const QUAT &rotation, const COL &color);
	void setInstancePosition(int modelId, int instanceId, const VC3 &position);
	void setInstanceRotation(int modelId, int instanceId, const QUAT &rotation);
	void setInstanceLight(int modelId, int instanceId, int light, int lightId, const COL &color);
	void setInstanceSun(int modelId, int instanceId, const VC3 &direction, float strength);
	void setInstanceLightmapped(int modelId, int instanceId, bool lightmapped);
	void setInstanceFade(int modelId, int instanceId, float factor);
	void setInstanceInBuilding(int modelId, int instanceId, bool inBuilding);
	void setInstanceOccluded(int modelId, int instanceId, bool occluded);
	void removeInstance(int modelId, int instanceId);
	void removeInstances();
	void setInstanceColorsToMultiplier(const COL &color);
	IStorm3D_Model *getInstanceModel(int modelId, int instanceId);

	// Lights
	int addLight(const VC3 &position, float radius, const COL &color);
	void setLightPosition(int index, const VC3 &position);
	void setLightRadius(int index, float radius);
	void setLightColor(int index, const COL &color);
	void removeLight(int index);
	void clearLights();

	void setAmbient(const COL &color);
	void setClearColor(const COL &color);
	void render(Storm3D_Scene &scene, const COL &fogColor);

	Storm3D_TerrainModels &getModels();
	IStorm3D_TerrainRenderer &getRenderer();
	IStorm3D_TerrainDecalSystem &getDecalSystem();

	void releaseDynamicResources();
	void recreateDynamicResources();

	// Querys
	boost::shared_ptr<IStorm3D_TerrainModelIterator> getModelIterator(const VC3 &position, float radius);
	bool findObject(const VC3 &position, float radius, int &modelId, int &instanceId);
	VC3 getNormal(const VC2I &position) const;
	VC3 getFaceNormal(const VC2 &position) const;
	VC3 getInterpolatedNormal(const VC2 &position) const;
	float getHeight(const VC2 &position) const;
	float getPartiallyInterpolatedHeight(const VC2 &position) const;
	void rayTrace(const VC3 &position, const VC3 &directionNormalized, float rayLength, Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate, bool lineOfSight) const;
};

#endif
