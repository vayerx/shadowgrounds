// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_TERRAIN_H
#define INCLUDED_ISTORM3D_TERRAIN_H

#include "DatatypeDef.h"
#include <boost/shared_ptr.hpp>
#include <string>

class IStorm3D_TerrainRenderer;
class IStorm3D_TerrainDecalSystem;
class IStorm3D_Model;
class IStorm3D_Texture;
struct Storm3D_CollisionInfo;
class ObstacleCollisionInfo;

namespace util
{
	class AreaMap;
}

// Standard stl-iterator limitations apply
class IStorm3D_TerrainModelIterator
{
public:
	virtual ~IStorm3D_TerrainModelIterator() {}

	virtual void next() = 0;
	virtual void erase() = 0;
	virtual bool end() const = 0;

	virtual VC3 getPosition() const = 0;
	virtual QUAT getRotation() const = 0;
	virtual COL getColor() const = 0;
	virtual int getModelId() const = 0;
	virtual int getInstanceId() const = 0;
};

class IStorm3D_Terrain
{
public:
	enum { BLOCK_SIZE = 8 };
	enum { CENTER_BLOCK_SIZE = 2 };

	virtual ~IStorm3D_Terrain() {}

	// Maps
	virtual void setHeightMap(const unsigned short *buffer, const VC2I &resolution, const VC3 &size, int textureDetail, unsigned short *forceMap, int heightmapMultiplier, int obstaclemapMultiplier) = 0;
	virtual void setClipMap(const unsigned char *buffer) = 0;
	virtual void updateHeightMap(const unsigned short *buffer, const VC2I &start, const VC2I &end) = 0;
	virtual void setObstacleHeightmap(const unsigned short *obstacleHeightmap, const util::AreaMap *areaMap) = 0;
	virtual void recreateCollisionMap() = 0;
	virtual void forcemapHeight(const VC2 &position, float radius, bool above = true, bool below = false) = 0;

	// Get shared collision heightmap (2x2 original heightmap size)
	virtual unsigned short *getCollisionHeightmap() = 0;

	// Texturing
	virtual int addTerrainTexture(IStorm3D_Texture &texture) = 0;
	virtual void removeTerrainTextures() = 0;
	virtual void setBlendMap(int blockIndex, IStorm3D_Texture &blend, int textureA, int textureB) = 0;
	virtual void resetBlends(int blockIndex) = 0;
	virtual void setLightMap(int blockIndex, IStorm3D_Texture &map) = 0;

	// Terrain objects
	virtual int addModel(boost::shared_ptr<IStorm3D_Model> model, boost::shared_ptr<IStorm3D_Model> fadeModel, const std::string &bones, const std::string &idleAnimation) = 0;
	virtual void removeModels() = 0;
	virtual int addInstance(int modelId, const VC3 &position, const QUAT &rotation, const COL &color) = 0;
	virtual void setInstancePosition(int modelId, int instanceId, const VC3 &position) = 0;
	virtual void setInstanceRotation(int modelId, int instanceId, const QUAT &rotation) = 0;
	virtual void setInstanceLight(int modelId, int instanceId, int light, int lightId, const COL &color) = 0;
	virtual void setInstanceSun(int modelId, int instanceId, const VC3 &direction, float strength) = 0;
	virtual void setInstanceLightmapped(int modelId, int instanceId, bool lightmapped) = 0;
	virtual void setInstanceFade(int modelId, int instanceId, float factor) = 0;
	virtual void setInstanceInBuilding(int modelId, int instanceId, bool inBuilding) = 0;
	virtual void setInstanceOccluded(int modelId, int instanceId, bool occluded) = 0;
	virtual void removeInstance(int modelId, int instanceId) = 0;
	virtual void removeInstances() = 0;
	virtual void setInstanceColorsToMultiplier(const COL &color) = 0;
	virtual IStorm3D_Model *getInstanceModel(int modelId, int instanceId) = 0;

	// Lights
	virtual int addLight(const VC3 &position, float radius, const COL &color) = 0;
	virtual void setLightPosition(int index, const VC3 &position) = 0;
	virtual void setLightRadius(int index, float radius) = 0;
	virtual void setLightColor(int index, const COL &color) = 0;
	virtual void removeLight(int index) = 0;
	virtual void clearLights() = 0;

	// Rendering
	virtual IStorm3D_TerrainRenderer &getRenderer() = 0;
	virtual IStorm3D_TerrainDecalSystem &getDecalSystem() = 0;

	// Querys
	virtual boost::shared_ptr<IStorm3D_TerrainModelIterator> getModelIterator(const VC3 &position, float radius) = 0;
	virtual bool findObject(const VC3 &position, float radius, int &modelId, int &instanceId) = 0;
	virtual VC3 getNormal(const VC2I &position) const = 0;
	virtual VC3 getFaceNormal(const VC2 &position) const = 0;
	virtual VC3 getInterpolatedNormal(const VC2 &position) const = 0;
	virtual float getHeight(const VC2 &position) const = 0;
	virtual float getPartiallyInterpolatedHeight(const VC2 &position) const = 0;
	virtual void rayTrace(const VC3 &position, const VC3 &directionNormalized, float rayLength, Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate = false, bool lineOfSight = false) const = 0;
};

#endif

/*

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <windows.h>

// Common datatypes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Texture.h"

// obstacle map bit defs. -jpk
#include "Storm3D_ObstacleMapDefs.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Terrain;
class IStorm3D_Terrain_ObjectCopyHandle;
class IStorm3D_Texture;


//------------------------------------------------------------------
// IStorm3D_Terrain (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Terrain
{

public:

	// Terrain texture generation layers
	enum TMLAYER
	{
		TMLAYER_WATER=0,
		TMLAYER_LEVEL1,
		TMLAYER_LEVEL2,
		TMLAYER_LEVEL3,
		TMLAYER_LEVEL4,
		TMLAYER_LEVEL1_SLOPE,
		TMLAYER_LEVEL2_SLOPE,
		TMLAYER_LEVEL3_SLOPE,
		TMLAYER_LEVEL4_SLOPE,
		TMLAYER_ESIZE  // DO NOT USE!
	};

	// Settings for generated texturing
	virtual void SetTopHeight(float height)=0;						// Above this height all texturing is 100% level4
	virtual void SetBottomHeight(float height)=0;					// Under this height all texturing is 100% level1
	virtual void SetWaterHeight(float height)=0;						// Water level (water plane is here)
	virtual void SetSlopeDivider(float divider=70.0f)=0;			// Slope divider (less=slope texture gets visible easier)
	virtual void SetSlopeStart(float start=0.15f)=0;				// Slope start (cut). If slope is lower than this, then it's not a slope (no slope texture is applied)
	virtual void SetTexturelayer(TMLAYER layer,int texnum)=0;	// Sets texturelayer to use some texture bank (use LoadTextureAtBank to load terraintextures)
	virtual void GenerateTexturing()=0;									// Generates texturing (overwrites old)

	virtual void RegenerateTexturing()=0;

	// Sets the memory reserving amount, affects block degeneration
	// The more memory reserve, the more extra blocks are kept in 
	// memory. (valid values 0 - xxx).
	virtual void SetMemoryReserve(int memoryReserveAmount)=0;

	// Sets the precache amount, how much the terrain will
	// attempt to generate blocks in advance.
	// (notice: memory reserve should be equal or greater than precache!)
	virtual void SetPreCache(int precacheAmount)=0;

	virtual int getVisibleBlockCount() const =0;
	virtual int getPrecachedBlockCount() const =0;
	virtual int getTotalGeneratedBlockCount() const =0;

	// Set shadow darkness and color
	// darkness 0 - 9, 9 being darkest
	// obstacleShadowHeight 0 - 100, percent of actual obstacle height
	// color is the masking color...
	virtual void SetShadow(int darkness, int obstacleShadowHeight, COL color, bool smooth)=0;
	
	// Texture functions
	virtual void LoadTextureAtBank(IStorm3D_Texture *texture,int texnum,bool tile_mirror=false)=0;

	// Detail texturing
	virtual void SetDetailTexture(IStorm3D_Texture *texture,int _detail_repeat)=0;	// NULL to remove detail texture
	virtual void SetDetailTexture2(IStorm3D_Texture *texture,int _detail_repeat)=0;	// NULL to remove second detail texture
	
	// Block size (Set only at beginning)
	virtual void SetBlockSize(int bsize=32)=0;

	// Heightmap stuff (Heightmap size must be 2^x)
	// NOTE: unlike in previous version this copies heightmap to terrainengine,
	// so you can delete your own copy right after calling this method.
	virtual void SetHeightMap(WORD *map,const VC2I &map_size,const VC3 &real_size,WORD *forcemap = 0)=0;

	// Editing
	virtual void Paint(const VC2 &position,float brush_radius,int texnum, int max_change = 255, int max_value = 255)=0;
	virtual void ModifyHeight(const VC2 &position,float brush_radius,float amount)=0;

	virtual void SphereCutoff(const VC3 &position,float radius,float depth)=0;
	// modify the height with 3D sphere rather than 2D circle as the above
	// cuts off a spheric area from the terrain

	// Forces the heightmap in given area to below or above the forcemap 
	virtual void forcemapHeight(const VC2 &position, float radius, bool above = true,
		bool below = false) = 0;

	// Object group visibility (new)
	virtual void SetObjectGroupVisibilityRange(BYTE group_id,float range)=0;

	// Models in terrain
	// Copies of models in terrain (grass, plants, trees etc)
	// New feature: group. You can create different object groups with this
	// for example trees (big objects) would be one group and grass/plants/rocks (small objects) other.
	virtual void AddModelCopy(IStorm3D_Model *model,const VC3 &position,const QUAT &rotation,BYTE group_id=0)=0;
	virtual VC3 RemoveModelCopy(int model_id, const VC3 &position)=0;

	// Sunlight
	virtual void SetSunlight(VC3 direction,COL color)=0;

	// Test collision etc
	virtual float GetHeightAt(const VC2 &position) const=0;
	virtual BYTE GetTextureAmountAt(int texture,const VC2 &position) const=0;
	virtual void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate = false, bool lineOfSight = false) const=0;

	virtual WORD *GetHeightmap() const=0;
	virtual void SaveColorMap(const char *fname) const = 0;

	virtual void SaveTerrain(const char *fname) const = 0;
	virtual void LoadTerrain(const char *fname) = 0;

	virtual void SetObstacleHeightmap(WORD *obstacleHeightmap)=0;

	// Recreate raytrace optimization map.
	virtual void RecreateCollisionMap()=0;

	// Object copy handling
	virtual ICreate<IStorm3D_Terrain_ObjectCopyHandle*> *GetObjectListFromBlockAtPosition(const VC2 &position,BYTE group_id=0)=0;	// Gets block at position, and returns it's objectlist
	virtual void RemoveObjectCopy(Iterator<IStorm3D_Terrain_ObjectCopyHandle*> *objectcopy,const VC2 &position)=0;					// Use the same position as with GetObjectListFromBlockAtPosition

	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_Terrain() {};
};


//------------------------------------------------------------------
// IStorm3D_Terrain_ObjectCopyHandle (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Terrain_ObjectCopyHandle
{
public:
	virtual ~IStorm3D_Terrain_ObjectCopyHandle() {}

	// Get object
	virtual IStorm3D_Model_Object *GetObject() const=0;

	// Get position
	virtual VC3 GetPosition() const=0;
	virtual QUAT GetRotation() const=0;

	// psd: for decent model removing
	virtual int GetModelId() const = 0;
	virtual IStorm3D_Model *GetModel() const = 0;
};

*/

