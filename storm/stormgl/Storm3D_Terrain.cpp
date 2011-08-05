// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <queue>
#include <vector>

#include "storm3d_terrain.h"
#include "storm3d_terrain_heightmap.h"
#include "storm3d_terrain_groups.h"
#include "storm3d_terrain_models.h"
#include "storm3d_terrain_renderer.h"
#include "storm3d_terrain_decalsystem.h"
#include "storm3d_texture.h"
#include "storm3d_model.h"
#include "storm3d_scene.h"
#include "Storm3D_ShaderManager.h"

#include "storm3d.h"
#include <boost/scoped_array.hpp>

#include "../../util/Debug_MemoryManager.h"

namespace {
} // unnamed

struct Storm3D_TerrainData
{
	Storm3D &storm;
	Storm3D_TerrainHeightmap heightMap;
	Storm3D_TerrainModels models;
	Storm3D_TerrainGroup modelGroups;
	Storm3D_TerrainDecalSystem decalSystem;
	Storm3D_TerrainRenderer renderer;

	Storm3D_TerrainData(Storm3D &storm_)
	:	storm(storm_),
		heightMap(storm),
		models(storm),
		modelGroups(storm, models),
		decalSystem(storm),
		renderer(storm, heightMap, modelGroups, models, decalSystem)
	{
	}

	~Storm3D_TerrainData()
	{
		modelGroups.removeInstances();
	}
};

Storm3D_Terrain::Storm3D_Terrain(Storm3D &storm)
{
	boost::scoped_ptr<Storm3D_TerrainData> tempData(new Storm3D_TerrainData(storm));
	data.swap(tempData);
}

Storm3D_Terrain::~Storm3D_Terrain()
{
	data->storm.Remove(this);
}

void Storm3D_Terrain::setHeightMap(const unsigned short *buffer, const VC2I &resolution, const VC3 &size, int textureDetail, unsigned short *forceMap, int heightmapMultiplier, int obstaclemapMultiplier)
{
	data->heightMap.setHeightMap(buffer, resolution, size, textureDetail, forceMap, heightmapMultiplier, obstaclemapMultiplier);
	data->modelGroups.setSceneSize(size);
	data->models.buildTree(size);
	data->decalSystem.setSceneSize(size);
}

IStorm3D_Model *Storm3D_Terrain::getInstanceModel(int modelId, int instanceId)
{
	return data->modelGroups.getInstanceModel ( modelId, instanceId );
}


void Storm3D_Terrain::setClipMap(const unsigned char *buffer)
{
	data->heightMap.setClipMap(buffer);
}

void Storm3D_Terrain::updateHeightMap(const unsigned short *buffer, const VC2I &start, const VC2I &end)
{
	data->heightMap.updateHeightMap(buffer, start, end);
}

void Storm3D_Terrain::setObstacleHeightmap(const unsigned short *obstacleHeightmap, const util::AreaMap *areaMap)
{
	data->heightMap.setObstacleHeightmap(obstacleHeightmap, areaMap);
}

void Storm3D_Terrain::recreateCollisionMap()
{
	data->heightMap.recreateCollisionMap();
}

void Storm3D_Terrain::forcemapHeight(const VC2 &position, float radius, bool above, bool below)
{
	data->heightMap.forcemapHeight(position, radius, above, below);
}

int Storm3D_Terrain::addTerrainTexture(IStorm3D_Texture &texture)
{
	return data->heightMap.addTerrainTexture(static_cast<Storm3D_Texture &> (texture));
}

void Storm3D_Terrain::removeInstance(int modelId, int instanceId)
{
	data->modelGroups.removeInstance(modelId, instanceId);
}

void Storm3D_Terrain::removeTerrainTextures()
{
	data->heightMap.removeTerrainTextures();
}

void Storm3D_Terrain::setBlendMap(int blockIndex, IStorm3D_Texture &blend, int textureA, int textureB)
{
	data->heightMap.setBlendMap(blockIndex, static_cast<Storm3D_Texture &> (blend), textureA, textureB);
}

void Storm3D_Terrain::resetBlends(int blockIndex)
{
	data->heightMap.resetBlends(blockIndex);
}

void Storm3D_Terrain::setLightMap(int blockIndex, IStorm3D_Texture &map_)
{
	data->heightMap.setLightMap(blockIndex, static_cast<Storm3D_Texture &> (map_));
}

int Storm3D_Terrain::addModel(boost::shared_ptr<IStorm3D_Model> model, boost::shared_ptr<IStorm3D_Model> fadeModel, const std::string &bones, const std::string &idleAnimation)
{
	boost::shared_ptr<Storm3D_Model> m = boost::static_pointer_cast<Storm3D_Model, IStorm3D_Model> (model);
	boost::shared_ptr<Storm3D_Model> mf = boost::static_pointer_cast<Storm3D_Model, IStorm3D_Model> (fadeModel);
	return data->modelGroups.addModel(m, mf, bones, idleAnimation);
}

void Storm3D_Terrain::removeModels()
{
	data->modelGroups.removeModels();
}

int Storm3D_Terrain::addInstance(int modelId, const VC3 &position, const QUAT &rotation, const COL &color)
{
	return data->modelGroups.addInstance(modelId, position, rotation, color);
}

void Storm3D_Terrain::setInstancePosition(int modelId, int instanceId, const VC3 &position)
{
	data->modelGroups.setInstancePosition(modelId, instanceId, position);
}

void Storm3D_Terrain::setInstanceRotation(int modelId, int instanceId, const QUAT &rotation)
{
	data->modelGroups.setInstanceRotation(modelId, instanceId, rotation);
}

void Storm3D_Terrain::setInstanceLight(int modelId, int instanceId, int light, int lightId, const COL &color)
{
	data->modelGroups.setInstanceLight(modelId, instanceId, light, lightId, color);
}

void Storm3D_Terrain::setInstanceSun(int modelId, int instanceId, const VC3 &direction, float strength)
{
	data->modelGroups.setInstanceSun(modelId, instanceId, direction, strength);
}

void Storm3D_Terrain::setInstanceLightmapped(int modelId, int instanceId, bool lightmapped)
{
	data->modelGroups.setInstanceLightmapped(modelId, instanceId, lightmapped);
}

void Storm3D_Terrain::setInstanceFade(int modelId, int instanceId, float factor)
{
	data->modelGroups.setInstanceFade(modelId, instanceId, factor);
}

void Storm3D_Terrain::setInstanceInBuilding(int modelId, int instanceId, bool inBuilding)
{
	data->modelGroups.setInstanceInBuilding(modelId, instanceId, inBuilding);
}

void Storm3D_Terrain::setInstanceOccluded(int modelId, int instanceId, bool occluded)
{
	data->modelGroups.setInstanceOccluded(modelId, instanceId, occluded);
}

void Storm3D_Terrain::removeInstances()
{
	data->modelGroups.removeInstances();
}

void Storm3D_Terrain::setInstanceColorsToMultiplier(const COL &color)
{
	data->modelGroups.setInstanceColorsToMultiplier(color);
}

int Storm3D_Terrain::addLight(const VC3 &position, float radius, const COL &color)
{
	return data->models.addLight(position, radius, color);
}

void Storm3D_Terrain::setLightPosition(int index, const VC3 &position)
{
	data->models.setLightPosition(index, position);
}

void Storm3D_Terrain::setLightRadius(int index, float radius)
{
	data->models.setLightRadius(index, radius);
}

void Storm3D_Terrain::setLightColor(int index, const COL &color)
{
	data->models.setLightColor(index, color);
}

void Storm3D_Terrain::removeLight(int index)
{
	data->models.removeLight(index);
}

void Storm3D_Terrain::clearLights()
{
	data->models.clearLights();
}

void Storm3D_Terrain::setAmbient(const COL &color)
{
	data->renderer.setAmbient(color);
}

void Storm3D_Terrain::setClearColor(const COL &color)
{
	data->renderer.setClearColor(color);
}

void Storm3D_Terrain::render(Storm3D_Scene &scene, const COL &fogColor)
{
	data->renderer.renderBase(scene);
}

Storm3D_TerrainModels &Storm3D_Terrain::getModels()
{
	return data->models;
}

IStorm3D_TerrainRenderer &Storm3D_Terrain::getRenderer()
{
	return data->renderer;
}

IStorm3D_TerrainDecalSystem &Storm3D_Terrain::getDecalSystem()
{
	return data->decalSystem;
}

void Storm3D_Terrain::releaseDynamicResources()
{
	data->renderer.releaseDynamicResources();
	data->decalSystem.releaseDynamicResources();
}

void Storm3D_Terrain::recreateDynamicResources()
{
	data->renderer.recreateDynamicResources();
	data->decalSystem.recreateDynamicResources();
}

boost::shared_ptr<IStorm3D_TerrainModelIterator> Storm3D_Terrain::getModelIterator(const VC3 &position, float radius)
{
	return data->modelGroups.getModelIterator(position, radius);
}

bool Storm3D_Terrain::findObject(const VC3 &position, float radius, int &modelId, int &instanceId)
{
	return data->modelGroups.findObject(position, radius, modelId, instanceId);
}

VC3 Storm3D_Terrain::getNormal(const VC2I &position) const
{
	return data->heightMap.getNormal(position);
}

VC3 Storm3D_Terrain::getFaceNormal(const VC2 &position) const
{
	return data->heightMap.getFaceNormal(position);
}

VC3 Storm3D_Terrain::getInterpolatedNormal(const VC2 &position) const
{
	return data->heightMap.getInterpolatedNormal(position);
}

float Storm3D_Terrain::getHeight(const VC2 &position) const
{
	return data->heightMap.getHeight(position);
}

float Storm3D_Terrain::getPartiallyInterpolatedHeight(const VC2 &position) const
{
	return data->heightMap.getPartiallyInterpolatedHeight(position);
}

unsigned short *Storm3D_Terrain::getCollisionHeightmap()
{
	return data->heightMap.getCollisionHeightmap();
}

void Storm3D_Terrain::rayTrace(const VC3 &position, const VC3 &directionNormalized, float rayLength, Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate, bool lineOfSight) const
{
	data->heightMap.rayTrace(position, directionNormalized, rayLength, rti, oci, accurate, lineOfSight);
}
