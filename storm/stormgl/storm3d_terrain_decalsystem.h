// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_DECALSYSTEM_H
#define INCLUDED_STORM3D_TERRAIN_DECALSYSTEM_H

#include <boost/scoped_ptr.hpp>
#include <istorm3d_terrain_decalsystem.h>

class Storm3D;
class Storm3D_Scene;
class Storm3D_Spotlight;

class Storm3D_TerrainDecalSystem: public IStorm3D_TerrainDecalSystem
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	Storm3D_TerrainDecalSystem(Storm3D &storm);
	~Storm3D_TerrainDecalSystem();

	void setSceneSize(const VC3 &size);

	int addMaterial(IStorm3D_Material *material);
	int addDecal(int materialId, Type type, const VC3 &position, int &id, bool forceSpawn);
	void eraseDecal(int materialId, int decalId, int id);

	void setRotation(int materialId, int decalId, int id, const QUAT &rotation);
	void setSize(int materialId, int decalId, int id, const VC2 &size);
	void setAlpha(int materialId, int decalId, int id, float alpha);
	void setLighting(int materialId, int decalId, int id, const COL &color);

	void setLightmapFactor(const COL &factor);
	void setOutdoorLightmapFactor(const COL &factor);
	void setFog(float end, float range);
	void renderTextures(Storm3D_Scene &scene);
	void renderShadows(Storm3D_Scene &scene);
	void renderProjection(Storm3D_Scene &scene, Storm3D_Spotlight *spot);

	void setShadowMaterial(IStorm3D_Material *material);
	void setShadowDecal(const VC3 &position, const QUAT &rotation, const VC2 &size, float alpha);
	void clearShadowDecals();

	void releaseDynamicResources();
	void recreateDynamicResources();
};

#endif
