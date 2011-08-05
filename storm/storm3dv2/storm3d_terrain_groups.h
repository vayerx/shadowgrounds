// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_GROUP_H
#define INCLUDED_STORM3D_TERRAIN_GROUP_H

#include "DatatypeDef.h"
#include "IStorm3D_Terrain.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class Storm3D;
class Storm3D_Scene;
class Storm3D_Camera;
class Storm3D_Model;
class Storm3D_TerrainModels;
struct Storm3D_TerrainGroupData;

class Storm3D_TerrainGroup
{
	boost::scoped_ptr<Storm3D_TerrainGroupData> data;

public:
	Storm3D_TerrainGroup(Storm3D &storm, Storm3D_TerrainModels &models, bool ps14);
	~Storm3D_TerrainGroup();

	int addModel(boost::shared_ptr<Storm3D_Model> model, boost::shared_ptr<Storm3D_Model> fadeModel, const std::string &bones, const std::string &idleAnimation);
	void removeModels();
	int addInstance(int modeld, const VC3 &position, const QUAT &rotation, const COL &color);
	void removeInstance(int modelId, int instanceId);
	void setInstancePosition(int modelId, int instanceId, const VC3 &position);
	void setInstanceRotation(int modelId, int instanceId, const QUAT &rotation);
	void setInstanceLight(int modelId, int instanceId, int light, int lightId, const COL &color);
	void setInstanceSun(int modelId, int instanceId, const VC3 &direction, float strength);
	void setInstanceLightmapped(int modelId, int instanceId, bool lightmapped);
	void setInstanceFade(int modelId, int instanceId, float factor);
	void setInstanceInBuilding(int modelId, int instanceId, bool inBuilding);
	void setInstanceOccluded(int modelId, int instanceId, bool occluded);
	void removeInstances();
	void setInstanceColorsToMultiplier(const COL &color);
	IStorm3D_Model *getInstanceModel(int modelId, int instanceId);

	void enableCollision(bool enable);
	void enableBigCollision(bool enable);
	void enableLightmapCollision(bool enable);

	void setSceneSize(const VC3 &size);
	boost::shared_ptr<IStorm3D_TerrainModelIterator> getModelIterator(const VC3 &position, float radius);
	bool findObject(const VC3 &position, float radius, int &modelId, int &instanceId);
};

#endif
