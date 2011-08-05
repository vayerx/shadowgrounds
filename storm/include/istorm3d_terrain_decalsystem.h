// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_TERRAIN_DECALSYSTEM_H
#define INCLUDED_ISTORM3D_TERRAIN_DECALSYSTEM_H

#include <DatatypeDef.h>

class IStorm3D_Material;

class IStorm3D_TerrainDecalSystem
{
public:
	virtual ~IStorm3D_TerrainDecalSystem() {}

	enum Type
	{
		Inside,
		Outside
	};

	virtual int addMaterial(IStorm3D_Material *material) = 0;
	virtual int addDecal(int materialId, Type type, const VC3 &position, int &id, bool forceSpawn) = 0;
	virtual void eraseDecal(int materialId, int decalId, int id) = 0;

	virtual void setRotation(int materialId, int decalId, int id, const QUAT &rotation) = 0;
	virtual void setSize(int materialId, int decalId, int id, const VC2 &size) = 0;
	virtual void setAlpha(int materialId, int decalId, int id, float alpha) = 0;
	virtual void setLighting(int materialId, int decalId, int id, const COL &color) = 0;

	virtual void setShadowMaterial(IStorm3D_Material *material) = 0;
	virtual void setShadowDecal(const VC3 &position, const QUAT &rotation, const VC2 &size, float alpha) = 0;
};

#endif
