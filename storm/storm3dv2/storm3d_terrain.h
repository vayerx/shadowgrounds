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
	Storm3D_Terrain(Storm3D &storm, bool ps13, bool ps14, bool ps20);
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
	bool legacyTexturing() const;

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

/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001



#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "Storm3D.h"
#include "IStorm3D_Terrain.h"
#include "Storm3D_Texture.h"
#include "Storm3D_Model.h"
#include "Storm3D_Model_Object.h"
#include "Storm3D_Mesh.h"
#include "Storm3D_Material.h"
#include "Storm3D_Scene.h"
#include "Iterator_TRBMOList.h"
#include <deque>
#include <vector>

//------------------------------------------------------------------
// Defines etc
//------------------------------------------------------------------
#define LOD_LEVELS				3
#define TPIX_MAX_TEXTURES		4
#define TERRAIN_TEX_AMOUNT		255

// size of the collision map (2 would be halved height map)
#define COLLISION_MAP_DIVIDER 4
#define COLLISION_MAP_DIV_SHIFT 2

// max face buf size 
#define TRBLOCK_IBUF_MAX_SIZE (65535/3)
// (65536 not divisible by 3, 65535 is, that's why that one)


//------------------------------------------------------------------
// Protos
//------------------------------------------------------------------
struct TMPix;
struct PMAPPIX;
class TRBlock;
class TRTexBank;
class TRBlock_MatHandle;
class TRBlock_MatHandle_Obj;


//------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------
typedef TRBlock *PTRBLOCK;
typedef TRTexBank *PTRTEXBANK;


//------------------------------------------------------------------
// TRTexBank
//------------------------------------------------------------------
class TRTexBank
{
	// Pointer to terrain that owns this block
	Storm3D_Terrain *terrain;

	// Texture pointer
	Storm3D_Texture *texture;

	// Texture color look-ups
	TColor<BYTE> *texcol_lookup;  // [blocksize][blocksize]

	// Tiling (standard tiling or each other mirrored)
	bool tile_mirror;

public:

	// Change texture
	void ChangeTexture(Storm3D_Texture *texture,bool _tile_mirror);

	// Texture color look-up stuff
	inline TColor<BYTE> &GetColorAt(int x,int y);

	// Construct & Destruct
	TRTexBank(Storm3D_Terrain *_terrain);
	~TRTexBank();

	friend class Storm3D_Terrain;
	friend class TRBlock;
};


//------------------------------------------------------------------
// Storm3D_Terrain
//------------------------------------------------------------------
class Storm3D_Terrain : public IStorm3D_Terrain
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// Last camera position and range
	VC3 lastcampos;
	float lastcamrange;

	// Sizes
	VC3 size;				// Real (world) size
	VC3 mmult_map_world;	// Map coordinate multiplier (converts map coodinates to word coords)
	VC3 mmult_world_map;	// Map coordinate multiplier (converts world coodinates to map coords)

	// Heightmap
	WORD *hmap;
	VC2I hmap_size;
	int hmapsh;				// Heightmap x shift (heightmap size is 2^x)

	WORD *forcemap;

	// Texturing map
	TMPix *tmap;

	// Texture banks
	PTRTEXBANK *texbanks;

	// Detail texture
	Storm3D_Texture *detail_texture;
	Storm3D_Texture *detail_texture2;
	int detail_repeat;
	int detail_repeat2;

	// Generated texturing stuff
	float height_top;
	float height_bottom;
	float height_water;
	float slope_divider;
	float slope_start;
	BYTE texlayers[TMLAYER_ESIZE+1];		// 255 = not assigned

	// Blocks
	PTRBLOCK *blocks;
	int blocksize;
	VC2I bmapsize; //=hmap_size/blocksize;

	// Index buffer for ground mesh rendering
	LPDIRECT3DINDEXBUFFER8 dx8_ibufs[LOD_LEVELS];
	int face_amounts[LOD_LEVELS];

	// Sunlight
	VC3 sun_dir;
	COL sun_col;
	TColor<float> ambient_color;

	// Shadows
	int shadow_col_r;
	int shadow_col_g;
	int shadow_col_b;
	int shadow_darkness;
	int shadow_minvalue;
	int shadow_obstacle_height;
	bool shadow_smooth;

	// Stats, visible blocks at render, precached and total generated
	// blocks...
	int visBlocks;
	int newGenBlocks;
	int totalGenBlocks;
	int recountCounter;

	// Object group visibility ranges
	float obj_group_vis_range[256];	// BYTE = [0,255]

	// Precalculation (optimization)
	PMAPPIX *precgrid;
	void Precalculate();

	// Conversion
	inline VC2I ConvertWorldToMap(const VC2 &position) const;
	inline VC2 ConvertWorldToMapFloat(const VC2 &position) const;
	inline VC2 ConvertMapToWorld(const VC2I &position) const;

	inline VC2I ConvertWorldToObstacleMap(const VC2 &position) const;
	inline VC2 ConvertObstacleMapToWorld(const VC2I &position) const;

	// Recreate blocks
	void RecreateBlocks();

	// Vertexbuffer recycler
	PtrList<IDirect3DVertexBuffer8> vbuf_recycler_lists[LOD_LEVELS];

	// Shadow lookup tables
	short *shad_xlookup,*shad_ylookup;

	WORD *obstacleHeightmap;
	WORD *collisionMap;

	int precacheAmount;
	int memoryReserve;

	// x1,y1 inclusive, x2,y2 exclusive
	void RecreateCollisionMapArea(int x1, int y1, int x2, int y2);
	
public:

	// Recreate raytrace optimization map.
	void RecreateCollisionMap();

	// Vertexbuffer recycler
	void CreateNewVertexbuffer(LPDIRECT3DVERTEXBUFFER8 *buffer,int lod);
	void DeleteVertexbuffer(LPDIRECT3DVERTEXBUFFER8 buffer,int lod);

	// Settings for generated texturing
	void SetTopHeight(float height);					// Above this height all texturing is 100% level4
	void SetBottomHeight(float height);					// Under this height all texturing is 100% level1
	void SetWaterHeight(float height);					// Water level (water plane is here)
	void SetSlopeDivider(float divider=70.0f);			// Slope divider (less=slope texture gets visible easier)
	void SetSlopeStart(float start=0.15f);				// Slope start (cut). If slope is lower than this, then it's not a slope (no slope texture is applied)
	void SetTexturelayer(TMLAYER layer,int texnum);		// Sets texturelayer to use some texture bank (use LoadTextureAtBank to load terraintextures)
	void GenerateTexturing();							// Generates texturing (overwrites old)
	
	// just to get terrain obstacle shadows --jpk
	void RegenerateTexturing();

	// Sets the memory reserving amount, affects block degeneration
	// The more memory reserve, the more extra blocks are kept in 
	// memory. (valid values 0 - xxx).
	void SetMemoryReserve(int memoryReserveAmount);

	// Sets the precache amount, how much the terrain will
	// attempt to generate blocks in advance.
	// (notice: memory reserve should be equal or greater than precache!)
	void SetPreCache(int precacheAmount);

	// Set shadow darkness and color
	// darkness 0 - 9, 9 being darkest
	// obstacleShadowHeight 0 - 100, percent of actual obstacle height
	// color is the masking color...
	void SetShadow(int darkness, int obstacleShadowHeight, COL color, bool smooth);

	// Terrain precalculation
	inline void BlendLightShadowAndTexturingPixel(int &x,int &y,int &xpp,int &ypp,int &nyp,int &nyp2,const Vec3<int> &sun_dir256,int &mul_r,int &mul_g,int &mul_b);
	void BlendLightShadowAndTexturing(const Rect<int> &area);

	// Shadows
	void CalculateShadows();							// Called after heightmap is set
	void UpdateShadows(DWORD counter);					// Called every frame (add counter every frame)

	// Texture functions
	void LoadTextureAtBank(IStorm3D_Texture *texture,int texnum,bool tile_mirror=false);

	// Detail texturing
	void SetDetailTexture(IStorm3D_Texture *texture,int _detail_repeat);	// NULL to remove detail texture
	void SetDetailTexture2(IStorm3D_Texture *texture,int _detail_repeat);	// NULL to remove second detail texture
	
	// Block size (Set only at beginning)
	void SetBlockSize(int bsize=32);

	// Heightmap stuff (Heightmap size must be 2^x)
	// NOTE: unlike in previous version this copies heightmap to terrainengine,
	// so you can delete your own copy right after calling this method.
	void SetHeightMap(WORD *map,const VC2I &map_size,const VC3 &real_size, WORD *forcemap = 0);

	void SetObstacleHeightmap(WORD *obstacleHeightmap);

	// Editing
	void Paint(const VC2 &position,float brush_radius,int texnum, int max_change = 255, int max_value = 255);
	void ModifyHeight(const VC2 &position,float brush_radius,float amount);
	void SphereCutoff(const VC3 &position,float radius,float depth);

	// Forces the heightmap in given area to below or above the forcemap 
	void forcemapHeight(const VC2 &position, float radius, bool above = true,
		bool below = false);

	// Object group visibility (new)
	void SetObjectGroupVisibilityRange(BYTE group_id,float range);

	// Models in terrain
	// Copies of models in terrain (grass, plants, trees etc)
	// New feature: group. You can create different object groups with this
	// for example trees (big objects) would be one group and grass/plants/rocks (small objects) other.
	void AddModelCopy(IStorm3D_Model *model,const VC3 &position,const QUAT &rotation,BYTE group_id=0);
	VC3 RemoveModelCopy(int model_id, const VC3 &position);

	// Sunlight
	void SetSunlight(VC3 direction,COL color);

	// Test collision etc
	float GetHeightAt(const VC2 &position) const;
	BYTE GetTextureAmountAt(int texture,const VC2 &position) const;
	void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate = false, bool lineOfSight = false) const;

	// This just returns the pointer for the heightmap data.
	// This is a kludge to get the "editor" working more or less.
	WORD *GetHeightmap() const;

	void SaveColorMap(const char *fname) const;

	void SaveTerrain(const char *fname) const;
	void LoadTerrain(const char *fname);

	// Statistics
	int getVisibleBlockCount() const;
	int getPrecachedBlockCount() const;
	int getTotalGeneratedBlockCount() const;

	// Set scenes ambient color
	void SetAmbientColor(const COL &new_ambient);

	// Object copy handling
	ICreate<IStorm3D_Terrain_ObjectCopyHandle*> *GetObjectListFromBlockAtPosition(const VC2 &position,BYTE group_id=0);	// Gets block at position, and returns it's objectlist
	void RemoveObjectCopy(Iterator<IStorm3D_Terrain_ObjectCopyHandle*> *objectcopy,const VC2 &position);				// Use the same position as with GetObjectListFromBlockAtPosition

	// Rendering
	void Render(Storm3D_Scene *scene);

	// Creation/deletion
	Storm3D_Terrain(Storm3D *s2,int bsize=32);
	~Storm3D_Terrain();

	friend class TRBlock;
	friend class TRTexBank;
	friend class TRBlock_MatHandle;
};


//------------------------------------------------------------------
// TMPair
//------------------------------------------------------------------
struct TMPair
{
	BYTE texture;		// 0-254 (255 = not used)
	BYTE amount;		// 0-255

	TMPair() : texture(255) {}
};


//------------------------------------------------------------------
// TMPix (texturing map "pixel")
//------------------------------------------------------------------
struct TMPix
{
	TMPair textures[TPIX_MAX_TEXTURES];
	DWORD color;		// Result color (optimization)
	WORD shadow;		// Shadow (silhouette height level)
	//BYTE light;		// Light intensity (both light and shadow)

	TMPix() : shadow(0) {}
};


//------------------------------------------------------------------
// TRBlock (terrain block)
//------------------------------------------------------------------
class TRBlock
{
	// Pointer to terrain that owns this block
	Storm3D_Terrain *terrain;

	// Ground vertexbuffer (indexed strip)
	LPDIRECT3DVERTEXBUFFER8 dx8_vbufs[LOD_LEVELS];
	int vertex_amounts[LOD_LEVELS];

	// Generated or not
	bool generated_buf_objects;

	// Terrain stuff
	VC2I blockpos;		// Upper left corner position in heightmap

	// Material handles (includes objects)
	PtrList<TRBlock_MatHandle> mathandles;

	// Iterator stuff for user object iterating
	ICreate<IStorm3D_Terrain_ObjectCopyHandle*> *iterator;

	// psd: map object handles to model id (index == id, might be null if removed)
	std::deque<std::vector<TRBlock_MatHandle_Obj *> > models;
	std::deque<VC3> model_positions;

public:

	// Get iterator
	ICreate<IStorm3D_Terrain_ObjectCopyHandle*> *GetIterator() {return iterator;}

	// Model copy add
	void AddModelCopy(Storm3D_Model *model,const VC3 &position, const QUAT &rotation, const VC3 &orig_position, BYTE group_id);
	VC3 RemoveModelCopy(int model_id);

	// Generation
	void Generate(int detail_level,float range);

	inline bool isGenerated() { return generated_buf_objects; }

	// Degeneration
	void DeGenerateAll();
	void DeGenerateHeightMesh();
	void DeGenerateObjectMeshes();
	
	// Rendering
	void Render(Storm3D_Scene *scene,D3DMATRIX *mxx,int detail_level,float range);

	// Construct & Destruct
	TRBlock(Storm3D_Terrain *_terrain,const VC2I &_blockpos);
	~TRBlock();

	friend class TRBlock_MatHandle; // for blockpos
};


//------------------------------------------------------------------
// TRBlock_MatHandle (Terrain block material handle for objects)
//------------------------------------------------------------------
class TRBlock_MatHandle
{
	// Pointer to terrain that owns this handle
	Storm3D_Terrain *terrain;

	// Pointer to block that owns this handle
	TRBlock *block;

	// Generated or not
	bool generated;

	// Group id
	BYTE group_id;

	// Material
	Storm3D_Material *material;

	// Objects (pointers) with this material
	PtrList<TRBlock_MatHandle_Obj> objects;

	// DirectX Buffers for group mesh (optimized)
	std::vector<LPDIRECT3DVERTEXBUFFER8*> dx8_vbufs;
	std::vector<LPDIRECT3DINDEXBUFFER8*> dx8_ibufs;
	std::vector<int> dx8_vbuf_amounts;
	std::vector<int> dx8_ibuf_amounts;
	int vertex_amount;
	int face_amount;

public:

	// Generation
	void Generate(float range);
	void DeGenerate();
	
	// Object add
	TRBlock_MatHandle_Obj *AddObject(Storm3D_Model_Object *object,const VC3 &position, const QUAT &rotation, const VC3 &orig_position);

	// Render group mesh
	void RenderGroupMesh(Storm3D_Scene *scene,float range);

	// Construct & Destruct
	TRBlock_MatHandle(Storm3D_Terrain *_terrain,TRBlock *_block,Storm3D_Material *mat,BYTE _group_id);
	~TRBlock_MatHandle();

	friend class Storm3D_Terrain;
	friend class TRBlock;
	friend class IteratorIM_TRBMOList<IStorm3D_Terrain_ObjectCopyHandle*>;
};


//------------------------------------------------------------------
// TRBlock_MatHandle_Obj (Materialhandle's object)
//------------------------------------------------------------------
class TRBlock_MatHandle_Obj : public IStorm3D_Terrain_ObjectCopyHandle
{
	// Pointer to object
	Storm3D_Model_Object *object;

	// Object position inside block
	VC3 position;
	QUAT rotation;
	VC3 original_position;

	// Stuff
	IStorm3D_Model *model;
	int model_id;
	TRBlock_MatHandle *material;

public:

	// Get object
	IStorm3D_Model_Object *GetObject() const;

	// Get position
	VC3 GetPosition() const;
	QUAT GetRotation() const;

	// psd: for decent model removing
	int GetModelId() const;
	IStorm3D_Model *GetModel() const;

	// Construct & Destruct
	TRBlock_MatHandle_Obj(Storm3D_Model_Object *_object,const VC3 &_position, const QUAT &rotation, const VC3 &orig_position);
	~TRBlock_MatHandle_Obj();

	friend class TRBlock;
	friend class TRBlock_MatHandle;
};


//------------------------------------------------------------------
// Precalculation map pixel (PMAPPIX)
//------------------------------------------------------------------
struct PMAPPIX
{
	// Terrain mesh
	VC3 terrain_pos;
	VC2 terrain_dtc;	// For detail texture
	VC2 terrain_dtc2;	// For 2nd detail texture
};

*/

