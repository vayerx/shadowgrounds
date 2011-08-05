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

	bool ps13;

	Storm3D_TerrainData(Storm3D &storm_, bool ps13_, bool ps14_, bool ps20_)
	:	storm(storm_),
		heightMap(storm, ps13_),
		models(storm),
		modelGroups(storm, models, ps14_),
		decalSystem(storm),
		renderer(storm, heightMap, modelGroups, models, decalSystem, ps14_, ps20_),
		ps13(ps13_)
	{
	}

	~Storm3D_TerrainData()
	{
		modelGroups.removeInstances();
	}
};

Storm3D_Terrain::Storm3D_Terrain(Storm3D &storm, bool ps13, bool ps14, bool ps20)
{
	boost::scoped_ptr<Storm3D_TerrainData> tempData(new Storm3D_TerrainData(storm, ps13, ps14, ps20));
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

bool Storm3D_Terrain::legacyTexturing() const
{
	if(data->ps13)
		return false;

	return true;
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
	//data->modelGroups.setAmbient(color);
	data->renderer.setAmbient(color);
}

void Storm3D_Terrain::setClearColor(const COL &color)
{
	data->renderer.setClearColor(color);
}

void Storm3D_Terrain::render(Storm3D_Scene &scene, const COL &fogColor)
{
	/*
	IDirect3DDevice8 &device = *data->storm.GetD3DDevice();

	device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device.SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	device.SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	device.SetRenderState(D3DRS_LIGHTING, FALSE);

	D3DMATERIAL8 material = { 0 };
	device.SetMaterial(&material);

	if(GetKeyState('R') & 0x80)
		device.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	D3DMATRIX dm = { 0 };
	dm._11 = 1.f;
	dm._22 = 1.f;
	dm._33 = 1.f;
	dm._44 = 1.f;

	data->modelGroups.render(scene);

	device.SetTransform(D3DTS_WORLD, &dm);
	data->heightMap.render(scene, fogColor);
	*/
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

/*
  Class: Storm3D_Terrain

  Terrain
    - 16 bit heightmap
	- Texturing map: 4 textures can affect one vertex
    - Basic terrain (precalculated blocks)
	- Trees, plants, weeds, grass etc (precalculated blocks)

  TODO:
	- Iteration with Group ID


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_terrain.h"
#include "VertexFormats.h"
#include <fstream>
#include <vector>

#include "Storm3D_ShaderManager.h"
#include "..\..\util\Debug_MemoryManager.h"



//------------------------------------------------------------------
//------------------------------------------------------------------
// Storm3D_Terrain
//------------------------------------------------------------------
//------------------------------------------------------------------



//------------------------------------------------------------------
// Creation/deletion
//------------------------------------------------------------------
Storm3D_Terrain::Storm3D_Terrain(Storm3D *s2,int bsize) :
	Storm3D2(s2),hmap(NULL),tmap(NULL),blocksize(bsize*2), // Interpolated hmap version 
	bmapsize(0,0),blocks(NULL),sun_dir(0,-1,0),sun_col(1,1,1),
	slope_divider(70.0f),slope_start(0.15f),precgrid(NULL),
	lastcampos(0,0,0),lastcamrange(0),detail_texture(NULL),detail_repeat(1),
	detail_texture2(NULL),detail_repeat2(4),shad_xlookup(NULL),shad_ylookup(NULL),
	obstacleHeightmap(NULL),collisionMap(NULL),forcemap(NULL),
	shadow_darkness(5),shadow_minvalue(1),shadow_col_r(255),
	shadow_col_g(255),shadow_col_b(255),shadow_obstacle_height(16),
	shadow_smooth(true),visBlocks(0),memoryReserve(0),precacheAmount(0),
	newGenBlocks(0), totalGenBlocks(0), recountCounter(0)
{
	// Clear texturelayers
	for (int i=0;i<TMLAYER_ESIZE+1;i++) texlayers[i]=255;

	// Clear indexbuffers
	for (i=0;i<LOD_LEVELS;i++) dx8_ibufs[i]=NULL;

	// Create texbank pointer array
	texbanks=new PTRTEXBANK[TERRAIN_TEX_AMOUNT];

	// Create texture banks
	for (i=0;i<TERRAIN_TEX_AMOUNT;i++) texbanks[i]=new TRTexBank(this);

	// Set default (infinite) object group visibility ranges
	for (i=0;i<256;i++) obj_group_vis_range[i]=HUGE;

	// Start optimization: Create 30 vertexbuffers for each LOD
	for (int lod=0;lod<LOD_LEVELS;lod++)
	for (i=0;i<30;i++)
	{
		// Create buffer
		LPDIRECT3DVERTEXBUFFER8 buffer;
		int xsz=(blocksize>>lod)+1;
		int ysz=(blocksize>>lod)+1;
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(xsz*ysz*sizeof(VXFORMAT_TERRAIN),
			D3DUSAGE_WRITEONLY,FVF_VXFORMAT_TERRAIN,D3DPOOL_MANAGED,&buffer);

		// Add to buffer
		vbuf_recycler_lists[lod].Add(buffer);	
	}

	// Do not create blocks or heightmap yet
}


Storm3D_Terrain::~Storm3D_Terrain()
{
	// Remove from Storm3D's list
	Storm3D2->Remove(this);

	// Delete maps
	// These pointer adjustments are because the arrays have safe guardband area
	// this is because some optimized routines would otherwise cause buffer overflow
	if (hmap) hmap-=(hmap_size.x*16)+16;
	if (tmap) tmap-=(hmap_size.x*16)+16;
	SAFE_DELETE_ARRAY(hmap);		// Delete hmap (unlike in previous versions), because now it's a part of terrain (copied from source)
	SAFE_DELETE_ARRAY(tmap);

	// Delete blocks
	if (blocks)
	{
		// Delete all blocks
		for (int i=0;i<bmapsize.y*bmapsize.x;i++) SAFE_DELETE(blocks[i]);

		// Delete block pointer array
		SAFE_DELETE_ARRAY(blocks);
	}

	// Delete texture banks
	for (int i=0;i<TERRAIN_TEX_AMOUNT;i++) SAFE_DELETE(texbanks[i]);

  // Delete collision map (terrain raytrace optimization map)
	SAFE_DELETE_ARRAY(collisionMap);

	// Delete texbank pointer array
	SAFE_DELETE_ARRAY(texbanks);

	// Delete old grid
	SAFE_DELETE_ARRAY(precgrid);

	// Delete indexbuffers
	for (i=0;i<LOD_LEVELS;i++) SAFE_RELEASE(dx8_ibufs[i]);

	// Release vertexbuffers (recycler stuff)
	for (int lod=0;lod<LOD_LEVELS;lod++)
	for (PtrListIterator<IDirect3DVertexBuffer8> pli=vbuf_recycler_lists[lod].Begin();(*pli)!=NULL;pli++)
	{
		(*pli)->Release();
	}

	if(detail_texture)
		detail_texture->Release();
	if(detail_texture2)
		detail_texture2->Release();

	// Delete shadow lookups
	SAFE_DELETE_ARRAY(shad_xlookup);
	SAFE_DELETE_ARRAY(shad_ylookup);

}


//------------------------------------------------------------------
// Precalculation (optimization)
//------------------------------------------------------------------
void Storm3D_Terrain::Precalculate()
{
	// Test
	if (hmap_size.x<1) return;
	if (hmap_size.y<1) return;

	// Precalc block stuff
	if (blocksize>0)
	{
		int x,y;
		int xlen=blocksize+1;
		int ylen=blocksize+1;
		float xadd=(size.x/(float)hmap_size.x)*1.00001f;
		float yadd=(size.z/(float)hmap_size.y)*1.00001f;
		//float xadd=(size.x/(float)hmap_size.x)*1.000001f;
		//float yadd=(size.z/(float)hmap_size.y)*1.000001f;

		// Delete old grid
		if (precgrid) delete[] precgrid;

		// Create new
		precgrid=new PMAPPIX[xlen*ylen];

		// Fill the grid
		int yp=0;
		for (y=0;y<ylen;y++)
		{
			for (x=0;x<xlen;x++)
			{
				// Do some distortion (not on the edges): Mesh does not look so monotonious ("griddy")
				float dx=0.0f,dy=0.0f;
				if ((x>0)&&(x<(xlen-1))&&(y>0)&&(y<(ylen-1)))
				{
					dx=SRND()*0.3f;
					dy=SRND()*0.3f;
				}

				// Terrain position
				precgrid[yp+x].terrain_pos=VC3(((float)x+dx)*xadd,0,((float)y+dy)*yadd);

				// Terrain texturecoordinates
				precgrid[yp+x].terrain_dtc=VC2((((float)x+dx)*(float)detail_repeat)/(float)blocksize,(((float)y+dy)*(float)detail_repeat)/(float)blocksize);
				precgrid[yp+x].terrain_dtc2=VC2((((float)x+dx)*(float)detail_repeat2)/(float)blocksize,(((float)y+dy)*(float)detail_repeat2)/(float)blocksize);
			}
			yp+=xlen;
		}	
	}

	// Precalculate shadow lookups
	// No shadows if light is directly from above (0,1,0)
	if ((fabsf(sun_dir.x)>=0.01f)||(fabsf(sun_dir.z)>=0.01f))
	{
		// Calculate sun direction projection in plane (0,1,0)
		VC2 sd_proj(sun_dir.x,sun_dir.z);

		// Calculate X gradients
		float ydelta=sd_proj.y/sd_proj.x;

		// Calculate X lookup
		SAFE_DELETE_ARRAY(shad_xlookup);
		shad_xlookup=new short[hmap_size.x];
		float yp=0;
		for (int x=0;x<hmap_size.x;x++)
		{
			shad_xlookup[x]=yp;
			yp+=ydelta;
		}

		// Calculate Y gradients
		float xdelta=sd_proj.x/sd_proj.y;

		// Calculate Y lookup
		SAFE_DELETE_ARRAY(shad_ylookup);
		shad_ylookup=new short[hmap_size.y];
		float xp=0;
		for (int y=0;y<hmap_size.y;y++)
		{
			shad_ylookup[y]=xp;
			xp+=xdelta;
		}
	}
}				


//------------------------------------------------------------------
// Vertexbuffer recycler
//------------------------------------------------------------------
void Storm3D_Terrain::CreateNewVertexbuffer(LPDIRECT3DVERTEXBUFFER8 *buffer,int lod)
{
	// Is there buffers available (with current lod)?
	if (vbuf_recycler_lists[lod].IsEmpty())
	{
		// No (create new)
		int xsz=(blocksize>>lod)+1;
		int ysz=(blocksize>>lod)+1;
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(xsz*ysz*sizeof(VXFORMAT_TERRAIN),
			D3DUSAGE_WRITEONLY,FVF_VXFORMAT_TERRAIN,D3DPOOL_MANAGED,buffer);
	}
	else
	{
		// Yes (get one)
		PtrListIterator<IDirect3DVertexBuffer8> vbi=vbuf_recycler_lists[lod].Begin();
		(*buffer)=*vbi;
		vbuf_recycler_lists[lod].Remove(vbi);
	}
}


void Storm3D_Terrain::DeleteVertexbuffer(LPDIRECT3DVERTEXBUFFER8 buffer,int lod)
{
	// No illegal buffers
	if (!buffer) return;

	// Add the buffer to list (do not delete at all)
	vbuf_recycler_lists[lod].Add(buffer);
}


//------------------------------------------------------------------
// Settings for generated texturing
//------------------------------------------------------------------
void Storm3D_Terrain::SetTopHeight(float height)
{
	height_top=height;
}


void Storm3D_Terrain::SetBottomHeight(float height)
{
	height_bottom=height;
}


void Storm3D_Terrain::SetWaterHeight(float height)
{
	height_water=height;
}


void Storm3D_Terrain::SetSlopeDivider(float divider)
{
	slope_divider=divider;
}


void Storm3D_Terrain::SetSlopeStart(float start)
{
	slope_start=start;
}


void Storm3D_Terrain::SetTexturelayer(TMLAYER layer,int texnum)
{
	texlayers[layer]=texnum;
}


int Storm3D_Terrain::getVisibleBlockCount() const
{
	return visBlocks;
}

int Storm3D_Terrain::getPrecachedBlockCount() const
{
	return newGenBlocks;
}

int Storm3D_Terrain::getTotalGeneratedBlockCount() const
{
	return totalGenBlocks;
}


void Storm3D_Terrain::RegenerateTexturing()
{
	// Calculate shadows
	CalculateShadows();
	
	// Calculate light, shadow and blend texturing
	BlendLightShadowAndTexturing(Rect<int>(Vec2<int>(0,0),hmap_size));

	// Recreate
	for(int i = 0; i < bmapsize.x; ++i)
	for(int j = 0; j < bmapsize.y; ++j)
		blocks[(j*bmapsize.x)+i]->DeGenerateHeightMesh();
	//	blocks[(j*bmapsize.x)+i]->DeGenerateAll();
}


void Storm3D_Terrain::SetShadow(int darkness, int obstacleShadowHeight, COL color, bool smooth)
{
	if (darkness < 0) darkness = 0;
	if (darkness > 9) darkness = 9;
	this->shadow_col_r = (int)(255 * color.r);
	this->shadow_col_g = (int)(255 * color.g);
	this->shadow_col_b = (int)(255 * color.b);
	this->shadow_darkness = darkness;
	if (shadow_darkness <= 4)
	{
		shadow_minvalue = 5 - shadow_darkness;
		shadow_darkness = 5 - shadow_minvalue;
	} else {
		shadow_minvalue = 1;
	}
	shadow_smooth = smooth;	

	this->shadow_obstacle_height = (16 * obstacleShadowHeight) / 100;
	// ...no longer percentages, now 0 - 16
}


void Storm3D_Terrain::GenerateTexturing()
{
	// Allocate memory for terrain texturing map
	// 16x16 extra size must be allocated because of some optimizations	
	if (tmap) tmap-=(hmap_size.x*16)+16;
	SAFE_DELETE_ARRAY(tmap);
	tmap=new TMPix[(hmap_size.y+16)*(hmap_size.x+16)];
	tmap+=(hmap_size.x*16)+16;

	// Calculate shadows
	CalculateShadows();

	// Precalc
	int ih_water=height_water;
	int ih_top=height_top;
	int ih_bottom=height_bottom;
	int tb_ran=ih_top-ih_bottom;
	int tb_ran_d4=tb_ran/4;
	if (tb_ran_d4<1) tb_ran_d4=1;
	int ih_l2=ih_bottom+tb_ran_d4;
	int ih_l3=ih_top-tb_ran_d4;
	float tbf_ran_d4=(float)tb_ran_d4/256.0f;

	// Test if slope divider is illegal and fix it
	if (slope_divider<0.001f) slope_divider=0.001f;

	// Generate (each "pixel")
	for (int y=0;y<hmap_size.y;y++)
	for (int x=0;x<hmap_size.x;x++)
	{
		// Get pixel
		int mpos=(y<<hmapsh)+x;
		int pixh=hmap[mpos];

		// Calculate slope
		int mpos_sx=hmap[(y<<hmapsh)+x+1];
		int mpos_sy=hmap[((y+1)<<hmapsh)+x];
		int slope=abs(mpos_sx-pixh)+abs(mpos_sy-pixh);
		float slopmul=((float)slope)/slope_divider;
		if (slopmul<slope_start) slopmul=0.0f;
		if (slopmul>1.0f) slopmul=1.0f;

		// Clear textures from pixel
		for (int i=0;i<TPIX_MAX_TEXTURES;i++) tmap[mpos].textures[i].texture=255;

		// Set texturing (according to height)
		if (pixh<=ih_water) // 100% water!!
		{
			tmap[mpos].textures[0].texture=texlayers[TMLAYER_WATER];
			tmap[mpos].textures[0].amount=255;
		}
		else
		if (pixh<=ih_bottom) // 100% level 1
		{
			if (slopmul<0.01f)
			{
				tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL1];
				tmap[mpos].textures[0].amount=255;
			}
			else
			{
				tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL1];
				tmap[mpos].textures[0].amount=255.0f*(1.0f-slopmul);
				tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL1_SLOPE];
				tmap[mpos].textures[1].amount=255.0f*slopmul;
			}
		}
		else
		if (pixh>=ih_top) // 100% level 4
		{
			if (slopmul<0.01f)
			{
				tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL4];
				tmap[mpos].textures[0].amount=255;
			}
			else
			{
				tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL4];
				tmap[mpos].textures[0].amount=255.0f*(1.0f-slopmul);
				tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL4_SLOPE];
				tmap[mpos].textures[1].amount=255.0f*slopmul;
			}
		}
		else	// Between level 1 and 4
		{
			if (slopmul<0.01f)
			{
				// No slope (2 textures)
				if (pixh>=ih_l3)	// 3-4
				{
					int amount=(float)(pixh-ih_l3)/tbf_ran_d4;
					tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL3];
					tmap[mpos].textures[0].amount=255-amount;
					tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL4];
					tmap[mpos].textures[1].amount=amount;
				}
				else if (pixh>=ih_l2)	// 2-3
				{
					int amount=(float)(pixh-ih_l2)/(2*tbf_ran_d4);
					tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL2];
					tmap[mpos].textures[0].amount=255-amount;
					tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL3];
					tmap[mpos].textures[1].amount=amount;
				}
				else	// 1-2
				{
					int amount=(float)(pixh-ih_bottom)/tbf_ran_d4;
					tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL1];
					tmap[mpos].textures[0].amount=255-amount;
					tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL2];
					tmap[mpos].textures[1].amount=amount;
				}
			}
			else
			{
				// Slope (2-4 textures)
				if (pixh>=ih_l3)	// 3-4
				{
					int amount=(float)(pixh-ih_l3)/tbf_ran_d4;
					tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL3];
					tmap[mpos].textures[0].amount=((float)(255-amount))*(1.0f-slopmul);
					tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL4];
					tmap[mpos].textures[1].amount=((float)amount)*(1.0f-slopmul);

					tmap[mpos].textures[2].texture=texlayers[TMLAYER_LEVEL3_SLOPE];
					tmap[mpos].textures[2].amount=((float)(255-amount))*slopmul;
					tmap[mpos].textures[3].texture=texlayers[TMLAYER_LEVEL4_SLOPE];
					tmap[mpos].textures[3].amount=((float)amount)*slopmul;
				}
				else if (pixh>=ih_l2)	// 2-3
				{
					int amount=(float)(pixh-ih_l2)/(2*tbf_ran_d4);
					tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL2];
					tmap[mpos].textures[0].amount=((float)(255-amount))*(1.0f-slopmul);
					tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL3];
					tmap[mpos].textures[1].amount=((float)amount)*(1.0f-slopmul);

					tmap[mpos].textures[2].texture=texlayers[TMLAYER_LEVEL2_SLOPE];
					tmap[mpos].textures[2].amount=((float)(255-amount))*slopmul;
					tmap[mpos].textures[3].texture=texlayers[TMLAYER_LEVEL3_SLOPE];
					tmap[mpos].textures[3].amount=((float)amount)*slopmul;
				}
				else	// 1-2
				{
					int amount=(float)(pixh-ih_bottom)/tbf_ran_d4;
					tmap[mpos].textures[0].texture=texlayers[TMLAYER_LEVEL1];
					tmap[mpos].textures[0].amount=((float)(255-amount))*(1.0f-slopmul);
					tmap[mpos].textures[1].texture=texlayers[TMLAYER_LEVEL2];
					tmap[mpos].textures[1].amount=((float)amount)*(1.0f-slopmul);
					
					tmap[mpos].textures[2].texture=texlayers[TMLAYER_LEVEL1_SLOPE];
					tmap[mpos].textures[2].amount=((float)(255-amount))*slopmul;
					tmap[mpos].textures[3].texture=texlayers[TMLAYER_LEVEL2_SLOPE];
					tmap[mpos].textures[3].amount=((float)amount)*slopmul;
				}
			}

			// Remove unused texture layers
			if (tmap[mpos].textures[0].amount<1) tmap[mpos].textures[0].texture=255;
			if (tmap[mpos].textures[1].amount<1) tmap[mpos].textures[1].texture=255;
			if (tmap[mpos].textures[2].amount<1) tmap[mpos].textures[2].texture=255;
			if (tmap[mpos].textures[3].amount<1) tmap[mpos].textures[3].texture=255;
		}

		// Compact texturing... There cannot be 2 same textures (combine them to one)
		for (int a=0;a<TPIX_MAX_TEXTURES;a++) if (tmap[mpos].textures[a].texture!=255)
		for (int b=a+1;b<TPIX_MAX_TEXTURES;b++)
		if (tmap[mpos].textures[a].texture==tmap[mpos].textures[b].texture)
		{
			// Add B to A, and delete B
			tmap[mpos].textures[a].amount+=tmap[mpos].textures[b].amount;
			tmap[mpos].textures[b].texture=255;
		}
	}

	// Fix water (smoother edges)
	for (y=1;y<(hmap_size.y-1);y++)
	for (int x=1;x<(hmap_size.x-1);x++)
	{
		// Get pixel
		int mpos=(y<<hmapsh)+x;
		int pixh=hmap[mpos];

		// If it's water
		if (pixh<=ih_water)
		{
			// Get neightbour pixels
			int tx1=tmap[mpos-1].textures[0].texture;
			int tx2=tmap[mpos+1].textures[0].texture;
			int ty1=tmap[mpos-hmap_size.x].textures[0].texture;
			int ty2=tmap[mpos+hmap_size.x].textures[0].texture;
			int txy1=tmap[mpos-1-hmap_size.x].textures[0].texture;
			int txy2=tmap[mpos+1-hmap_size.x].textures[0].texture;
			int txy3=tmap[mpos-1+hmap_size.x].textures[0].texture;
			int txy4=tmap[mpos+1+hmap_size.x].textures[0].texture;

			// Test if neightbour pixel is not water
			int other=255;
			int amo=127;
			if (tx1!=texlayers[TMLAYER_WATER]) {other=tx1;amo=230;}
			if (tx2!=texlayers[TMLAYER_WATER]) {other=tx2;amo=230;}
			if (ty1!=texlayers[TMLAYER_WATER]) {other=ty1;amo=230;}
			if (ty2!=texlayers[TMLAYER_WATER]) {other=ty2;amo=230;}
			if (txy1!=texlayers[TMLAYER_WATER]) other=txy1;
			if (txy2!=texlayers[TMLAYER_WATER]) other=txy2;
			if (txy3!=texlayers[TMLAYER_WATER]) other=txy3;
			if (txy4!=texlayers[TMLAYER_WATER]) other=txy4;

			if (other!=255)
			{
				// Blend water and other
				tmap[mpos].textures[0].amount=(255-amo);
				tmap[mpos].textures[1].texture=other;
				tmap[mpos].textures[1].amount=amo;
			}
		}
	}

	// Calculate light, shadow and blend texturing
	BlendLightShadowAndTexturing(Rect<int>(Vec2<int>(0,0),hmap_size));

	// Recreate
	RecreateBlocks();
}



//------------------------------------------------------------------
// Terrain precalculation
//------------------------------------------------------------------
void Storm3D_Terrain::BlendLightShadowAndTexturingPixel(int &x,int &y,int &xpp,int &ypp,int &nyp,int &nyp2,const Vec3<int> &sun_dir256,int &mul_r,int &mul_g,int &mul_b)
{
	// Get pixel
	int mpos=(y<<hmapsh)+x;

	// Blend textures
	TColor<int> col(0,0,0);
	for (int i=0;i<TPIX_MAX_TEXTURES;i++)
	if (tmap[mpos].textures[i].texture!=255)
	{
		TColor<BYTE> &cb=texbanks[tmap[mpos].textures[i].texture]->GetColorAt(x,y);
		col.r+=cb.r*tmap[mpos].textures[i].amount;
		col.g+=cb.g*tmap[mpos].textures[i].amount;
		col.b+=cb.b*tmap[mpos].textures[i].amount;
	}

	// Calc lighting
	int pos0=hmap[mpos];
	int posx=hmap[mpos+1];
	int posy=hmap[mpos+hmap_size.x];

	// This is the unoptimal row: VC3 normal=VC3(xpp,((float)(posx-pos0)/65535.0f)*size.y,0).GetCrossWith(VC3(0,((float)(posy-pos0)/65535.0f)*size.y,ypp));
	// Optimized version ->
	Vec3<int> normal(((posx-pos0)*xpp)/65536,nyp,((posy-pos0)*ypp)/65536);	// Optimized cross product
	int ilen=255/(int)(sqrtf((int)(normal.x*normal.x)+nyp2+(normal.z*normal.z))+1);
	normal.x*=ilen;
	normal.y*=ilen;
	normal.z*=ilen;

	int cmu=normal.GetDotWith(sun_dir256);
	if (cmu<0) cmu=0;
	cmu+=23040;
	if (cmu>65535) cmu=65535;
	int r=cmu*mul_r;
	int g=cmu*mul_g;
	int b=cmu*mul_b;
	r>>=16;
	g>>=16;
	b>>=16;

	// Shadows!!!

	
	//if (tmap[mpos].shadow>hmap[mpos])
	//{
	//	//cmu>>=1;	// Vertex color divided by 2 if inside shadow
	//	r>>=1;	// Vertex color divided by 2 if inside shadow
	//	g>>=1;	// Vertex color divided by 2 if inside shadow
	//	b>>=1;	// Vertex color divided by 2 if inside shadow
	//}
	
	// haxor to "despecle" and smooth shadows
	// --jpk
	int mpos2=(y<<hmapsh)+(x+1);
	int mpos3=(y<<hmapsh)+(x-1);
	int mpos4=((y+1)<<hmapsh)+x;
	int mpos5=((y-1)<<hmapsh)+x;
	int shadow_amount = 0;
	if (shadow_smooth)
	{
		if (tmap[mpos].shadow>hmap[mpos]) shadow_amount++;
		if (tmap[mpos2].shadow>hmap[mpos2]) shadow_amount++;
		if (tmap[mpos3].shadow>hmap[mpos3]) shadow_amount++;
		if (tmap[mpos4].shadow>hmap[mpos4]) shadow_amount++;
		if (tmap[mpos5].shadow>hmap[mpos5]) shadow_amount++;
	} else {
		if (tmap[mpos].shadow>hmap[mpos]) shadow_amount = 5;
	}
	if (shadow_amount >= shadow_minvalue)
	{
	  int seff = 20 - shadow_darkness - shadow_amount;
		r = (((r * seff) >> 4) * shadow_col_r + r * (255-shadow_col_r)) >> 8;
		g = (((g * seff) >> 4) * shadow_col_g + g * (255-shadow_col_g)) >> 8;
		b = (((b * seff) >> 4) * shadow_col_b + b * (255-shadow_col_b)) >> 8;
	}

	// Set color
	tmap[mpos].color=(((col.r*r)>>16)<<16)+(((col.g*g)>>16)<<8)+((col.b*b)>>16);
}

	
void Storm3D_Terrain::BlendLightShadowAndTexturing(const Rect<int> &area)
{
	// Blend colors (optimization)
	int xpp=((65535.0f*size.x)/((float)hmap_size.x*size.y));
	int ypp=((65535.0f*size.z)/((float)hmap_size.y*size.y));
	int nyp=(-xpp*ypp)/65536;
	int nyp2=nyp*nyp;
	Vec3<int> sun_dir256(sun_dir.x*255.0f,sun_dir.y*255.0f,sun_dir.z*255.0f);
	int mul_r=sun_col.r*255.0f;
	int mul_g=sun_col.g*255.0f;
	int mul_b=sun_col.b*255.0f;

	for (int y=area.upper_left.y;y<area.lower_right.y;y++)
	for (int x=area.upper_left.x;x<area.lower_right.x;x++)
	{
		// Get pixel
		int mpos=(y<<hmapsh)+x;

		// Blend textures
		TColor<int> col(0,0,0);
		for (int i=0;i<TPIX_MAX_TEXTURES;i++)
		if (tmap[mpos].textures[i].texture!=255)
		{
			TColor<BYTE> &cb=texbanks[tmap[mpos].textures[i].texture]->GetColorAt(x,y);
			col.r+=cb.r*tmap[mpos].textures[i].amount;
			col.g+=cb.g*tmap[mpos].textures[i].amount;
			col.b+=cb.b*tmap[mpos].textures[i].amount;
		}

		// Calc lighting
		int pos0=hmap[mpos];
		int posx=hmap[mpos+1];
		int posy=hmap[mpos+hmap_size.x];

		// This is the unoptimal row: VC3 normal=VC3(xpp,((float)(posx-pos0)/65535.0f)*size.y,0).GetCrossWith(VC3(0,((float)(posy-pos0)/65535.0f)*size.y,ypp));
		// Optimized version ->
		Vec3<int> normal(((posx-pos0)*xpp)/65536,nyp,((posy-pos0)*ypp)/65536);	// Optimized cross product
		int ilen=255/(int)(sqrtf((int)(normal.x*normal.x)+nyp2+(normal.z*normal.z))+1);
		normal.x*=ilen;
		normal.y*=ilen;
		normal.z*=ilen;

		int cmu=normal.GetDotWith(sun_dir256);
		if (cmu<0) cmu=0;
		cmu+=23040;
		if (cmu>65535) cmu=65535;
		int r=cmu*mul_r;
		int g=cmu*mul_g;
		int b=cmu*mul_b;
		r>>=16;
		g>>=16;
		b>>=16;

		// Shadows!!!
		//if (tmap[mpos].shadow>hmap[mpos])
		//{
		//	//cmu>>=1;	// Vertex color divided by 2 if inside shadow
		//	r>>=1;	// Vertex color divided by 2 if inside shadow
		//	g>>=1;	// Vertex color divided by 2 if inside shadow
		//	b>>=1;	// Vertex color divided by 2 if inside shadow
		//}
		
		// haxor to "despecle" and smooth shadows
		// --jpk
		int mpos2=(y<<hmapsh)+(x+1);
		int mpos3=(y<<hmapsh)+(x-1);
		int mpos4=((y+1)<<hmapsh)+x;
		int mpos5=((y-1)<<hmapsh)+x;
		int shadow_amount = 0;
		if (shadow_smooth)
		{
			if (tmap[mpos].shadow>hmap[mpos]) shadow_amount++;
			if (tmap[mpos2].shadow>hmap[mpos2]) shadow_amount++;
			if (tmap[mpos3].shadow>hmap[mpos3]) shadow_amount++;
			if (tmap[mpos4].shadow>hmap[mpos4]) shadow_amount++;
			if (tmap[mpos5].shadow>hmap[mpos5]) shadow_amount++;
		} else {
			if (tmap[mpos].shadow>hmap[mpos]) shadow_amount = 5;
		}
		if (shadow_amount >= shadow_minvalue)
		{
			int seff = 20 - shadow_darkness - shadow_amount;
			r = (((r * seff) >> 4) * shadow_col_r + r * (255-shadow_col_r)) >> 8;
			g = (((g * seff) >> 4) * shadow_col_g + g * (255-shadow_col_g)) >> 8;
			b = (((b * seff) >> 4) * shadow_col_b + b * (255-shadow_col_b)) >> 8;
		}

		// Set color
		tmap[mpos].color=(((col.r*r)>>16)<<16)+(((col.g*g)>>16)<<8)+((col.b*b)>>16);
	}
}

	
//------------------------------------------------------------------
// Shadows
//------------------------------------------------------------------
void Storm3D_Terrain::CalculateShadows()
{
	// No shadows if light is directly from above (0,1,0)
	if ((fabsf(sun_dir.x)<0.01f)&&(fabsf(sun_dir.z)<0.01f)) return;

	// Calculate sun direction projection in plane (0,1,0)
	VC2 sd_proj(sun_dir.x,sun_dir.z);

  int obstacleShift = hmapsh + OBSTACLE_MAP_MULT_SHIFT;

	// 2 different calculation situations depending if abs(sd_proj.x)>abs(sd_proj.y) 
	// This way the calculation is much more optimal in some cases
	if (fabs(sd_proj.x)>fabs(sd_proj.y))
	{
		// Calculate gradients
		float ydelta=sd_proj.y/sd_proj.x;
		int hdelta=((sun_dir.y*65535.0f)/(float)hmap_size.x)*(size.x/size.y);

		// Precalculate
		int start=0;
		int end=hmap_size.y-(ydelta*(float)hmap_size.x);
		if (ydelta>0)
		{
			start=-ydelta*(float)hmap_size.x;
			end=hmap_size.y;
		}

		if (sd_proj.x>0)
		{
			// Loop in y direction
			for (int y=start;y<end;y++)
			{
				int silh=0;
				for (int x=0;x<hmap_size.x;x++)
				{
					// Calculate Y position
					int ypp=y+shad_xlookup[x];
					if ((ypp<0)||(ypp>=hmap_size.y)) continue;

					// Get pixel
					int ppos=(ypp<<hmapsh)+x;
					int hgt=hmap[ppos];
					if (obstacleHeightmap != NULL)
						if ((obstacleHeightmap[((ypp<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(x<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_SEETHROUGH) == 0)
							hgt += (((obstacleHeightmap[((ypp<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(x<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_HEIGHT) 
								* shadow_obstacle_height) >> 4);
					
					// Modify silhouette (if needed)
					if (hgt>=silh)
					{
						silh=hgt;
					}
					else 
					{
						silh-=hdelta;
						if (silh<0) silh=0;
					}

					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;
				}
			}
		}
		else
		{
			// Loop in y direction
			for (int y=start;y<end;y++)
			{
				int silh=0;
				for (int x=hmap_size.x-1;x>=0;x--)
				{
					// Calculate Y position
					int ypp=y+shad_xlookup[x];
					if ((ypp<0)||(ypp>=hmap_size.y)) continue;

					// Get pixel
					int ppos=(ypp<<hmapsh)+x;
					int hgt=hmap[ppos];
					if (obstacleHeightmap != NULL)
						if ((obstacleHeightmap[((ypp<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(x<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_SEETHROUGH) == 0)
							hgt += (((obstacleHeightmap[((ypp<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(x<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_HEIGHT)
								* shadow_obstacle_height) >> 4);
					
					// Modify silhouette (if needed)
					if (hgt>=silh)
					{
						silh=hgt;
					}
					else 
					{
						silh-=hdelta;
						if (silh<0) silh=0;
					}

					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;
				}
			}
		}
	}
	else
	{
		// Calculate gradients
		float xdelta=sd_proj.x/sd_proj.y;
		int hdelta=((sun_dir.x*65535.0f)/(float)hmap_size.y)*(size.z/size.y);

		// Precalculate
		int start=0;
		int end=hmap_size.x-(xdelta*(float)hmap_size.y);
		if (xdelta>0)
		{
			start=-xdelta*(float)hmap_size.y;
			end=hmap_size.x;
		}

		if (sd_proj.y>0)
		{
			// Loop in y direction
			for (int x=start;x<end;x++)
			{
				int silh=0;
				for (int y=0;y<hmap_size.y;y++)
				{
					// Calculate Y position
					int xpp=x+shad_ylookup[y];
					if ((xpp<0)||(xpp>=hmap_size.x)) continue;

					// Get pixel
					int ppos=(y<<hmapsh)+xpp;
					int hgt=hmap[ppos];
					if (obstacleHeightmap != NULL)
						if ((obstacleHeightmap[((y<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(xpp<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_SEETHROUGH) == 0)
							hgt += (((obstacleHeightmap[((y<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(xpp<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_HEIGHT)
								* shadow_obstacle_height) >> 4);
					
					// Modify silhouette (if needed)
					if (hgt>=silh)
					{
						silh=hgt;
					}
					else 
					{
						silh-=hdelta;
						if (silh<0) silh=0;
					}

					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;
				}
			}
		}
		else
		{
			// Loop in y direction
			for (int x=start;x<end;x++)
			{
				int silh=0;
				for (int y=hmap_size.y-1;y>=0;y--)
				{
					// Calculate Y position
					int xpp=x+shad_ylookup[y];
					if ((xpp<0)||(xpp>=hmap_size.x)) continue;

					// Get pixel
					int ppos=(y<<hmapsh)+xpp;
					int hgt=hmap[ppos];
					if (obstacleHeightmap != NULL)
						if ((obstacleHeightmap[((y<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(xpp<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_SEETHROUGH) == 0)
							hgt += (((obstacleHeightmap[((y<<OBSTACLE_MAP_MULT_SHIFT)<<obstacleShift)+(xpp<<OBSTACLE_MAP_MULT_SHIFT)] & OBSTACLE_MAP_MASK_HEIGHT)
								* shadow_obstacle_height) >> 4);

					// Modify silhouette (if needed)
					if (hgt>=silh)
					{
						silh=hgt;
					}
					else 
					{
						silh-=hdelta;
						if (silh<0) silh=0;
					}

					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;
				}
			}
		}
	}
}


void Storm3D_Terrain::UpdateShadows(DWORD counter)
{
	// No shadows if light is directly from above (0,1,0)
	if ((fabsf(sun_dir.x)<0.01f)&&(fabsf(sun_dir.z)<0.01f)) return;

	// Blend colors (optimization)
	int bxpp=((65535.0f*size.x)/((float)hmap_size.x*size.y));
	int bypp=((65535.0f*size.z)/((float)hmap_size.y*size.y));
	int nyp=(-bxpp*bypp)/65536;
	int nyp2=nyp*nyp;
	Vec3<int> sun_dir256(sun_dir.x*255.0f,sun_dir.y*255.0f,sun_dir.z*255.0f);
	int mul_r=sun_col.r*255.0f;
	int mul_g=sun_col.g*255.0f;
	int mul_b=sun_col.b*255.0f;

	// Calculate sun direction projection in plane (0,1,0)
	VC2 sd_proj(sun_dir.x,sun_dir.z);

	// 2 different calculation situations depending if abs(sd_proj.x)>abs(sd_proj.y) 
	// This way the calculation is much more optimal in some cases
	if (fabs(sd_proj.x)>fabs(sd_proj.y))
	{
		// Calculate gradients
		float ydelta=sd_proj.y/sd_proj.x;
		int hdelta=((sun_dir.z*65535.0f)/(float)hmap_size.x)*(size.x/size.y);

		// Precalculate
		int start=0;
		int end=hmap_size.y-(ydelta*(float)hmap_size.x);
		if (ydelta>0)
		{
			start=-ydelta*(float)hmap_size.x;
			end=hmap_size.y;
		}

		// Set the counter on right range
		counter%=end-start;
		counter+=start;
		int y=counter;

		if (sd_proj.x>0)
		{
			// Loop in y direction
			int silh=0;
			for (int x=0;x<hmap_size.x;x++)
			{
				// Calculate Y position
				int ypp=y+shad_xlookup[x];
				if ((ypp<0)||(ypp>=hmap_size.y)) continue;

				// Get pixel
				int ppos=(ypp<<hmapsh)+x;
				int hgt=hmap[ppos];
					
				// Modify silhouette (if needed)
				if (hgt>=silh)
				{
					silh=hgt;
				}
				else 
				{
					silh-=hdelta;
					if (silh<0) silh=0;
				}

				// Test if there is a change
				if (tmap[ppos].shadow!=silh)
				{
					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;

					// Blend texturing
					BlendLightShadowAndTexturingPixel(x,ypp,bxpp,bypp,nyp,nyp2,sun_dir256,mul_r,mul_g,mul_b);

					// Degenerate affected block
					//blocks[((ypp/blocksize)*bmapsize.x)+x]->DeGenerateHeightMesh();
				}
			}
		}
		else
		{
			// Loop in y direction
			int silh=0;
			for (int x=hmap_size.x;x>=0;x--)
			{
				// Calculate Y position
				int ypp=y+shad_xlookup[x];
				if ((ypp<0)||(ypp>=hmap_size.y)) continue;

				// Get pixel
				int ppos=(ypp<<hmapsh)+x;
				int hgt=hmap[ppos];
					
				// Modify silhouette (if needed)
				if (hgt>=silh)
				{
					silh=hgt;
				}
				else 
				{
					silh-=hdelta;
					if (silh<0) silh=0;
				}

				// Test if there is a change
				if (tmap[ppos].shadow!=silh)
				{
					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;

					// Blend texturing
					BlendLightShadowAndTexturingPixel(x,ypp,bxpp,bypp,nyp,nyp2,sun_dir256,mul_r,mul_g,mul_b);

					// Degenerate affected block
					//blocks[((ypp/blocksize)*bmapsize.x)+x]->DeGenerateHeightMesh();
				}
			}
		}
	}
	else
	{
		// Calculate gradients
		float xdelta=sd_proj.x/sd_proj.y;
		int hdelta=((sun_dir.y*65535.0f)/(float)hmap_size.y)*(size.z/size.y);

		// Precalculate
		int start=0;
		int end=hmap_size.x-(xdelta*(float)hmap_size.y);
		if (xdelta>0)
		{
			start=-xdelta*(float)hmap_size.y;
			end=hmap_size.x;
		}

		// Set the counter on right range
		counter%=end-start;
		counter+=start;
		int x=counter;

		if (sd_proj.y>0)
		{
			// Loop in y direction
			int silh=0;
			for (int y=0;y<hmap_size.y;y++)
			{
				// Calculate Y position
				int xpp=x+shad_ylookup[y];
				if ((xpp<0)||(xpp>=hmap_size.x)) continue;

				// Get pixel
				int ppos=(y<<hmapsh)+xpp;
				int hgt=hmap[ppos];
					
				// Modify silhouette (if needed)
				if (hgt>=silh)
				{
					silh=hgt;
				}
				else 
				{
					silh-=hdelta;
					if (silh<0) silh=0;
				}

				// Test if there is a change
				if (tmap[ppos].shadow!=silh)
				{
					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;

					// Blend texturing
					BlendLightShadowAndTexturingPixel(xpp,y,bxpp,bypp,nyp,nyp2,sun_dir256,mul_r,mul_g,mul_b);

					// Degenerate affected block
					//blocks[((y/blocksize)*bmapsize.x)+xpp]->DeGenerateHeightMesh();
				}
			}			
		}
		else
		{
			// Loop in y direction
			int silh=0;
			for (int y=hmap_size.y-1;y>=0;y--)
			{
				// Calculate Y position
				int xpp=x+shad_ylookup[y];
				if ((xpp<0)||(xpp>=hmap_size.x)) continue;

				// Get pixel
				int ppos=(y<<hmapsh)+xpp;
				int hgt=hmap[ppos];

				// Modify silhouette (if needed)
				if (hgt>=silh)
				{
					silh=hgt;
				}
				else 
				{
					silh-=hdelta;
					if (silh<0) silh=0;
				}

				// Test if there is a change
				if (tmap[ppos].shadow!=silh)
				{
					// Save silhouette pixel to tmap
					tmap[ppos].shadow=silh;

					// Blend texturing
					BlendLightShadowAndTexturingPixel(xpp,y,bxpp,bypp,nyp,nyp2,sun_dir256,mul_r,mul_g,mul_b);
					
					// Degenerate affected block
					//blocks[((y/blocksize)*bmapsize.x)+xpp]->DeGenerateHeightMesh();
				}
			}
		}
	}
}


//------------------------------------------------------------------
// Texture functions
//------------------------------------------------------------------
void Storm3D_Terrain::LoadTextureAtBank(IStorm3D_Texture *texture,int texnum,bool tile_mirror)
{
	texbanks[texnum]->ChangeTexture((Storm3D_Texture*)texture,tile_mirror);
}


//------------------------------------------------------------------
// Detail texturing
//------------------------------------------------------------------
void Storm3D_Terrain::SetDetailTexture(IStorm3D_Texture *texture,int _detail_repeat)
{
	if(detail_texture)
		detail_texture->Release();
	if(texture)
		((Storm3D_Texture*)texture)->AddRef();

	detail_texture=(Storm3D_Texture*)texture;
	detail_repeat=_detail_repeat;
}


//------------------------------------------------------------------
// Detail texturing
//------------------------------------------------------------------
void Storm3D_Terrain::SetDetailTexture2(IStorm3D_Texture *texture,int _detail_repeat)
{
	if(detail_texture2)
		detail_texture2->Release();
	if(texture)
		((Storm3D_Texture*)texture)->AddRef();

	detail_texture2=(Storm3D_Texture*)texture;
	detail_repeat2=_detail_repeat;
}


//------------------------------------------------------------------
// Recreate blocks
//------------------------------------------------------------------
void Storm3D_Terrain::RecreateBlocks()
{
	// Test 1
	if (hmap==NULL) return;

	// Precalculate stuff
	Precalculate();

	// Test 2
	if (blocksize<1) return;

	// Generate indexbuffer for blockblend texture rendering and terrain rendering...

	// Create new indexbuffers (and release old if exists)
	for (int l=0;l<LOD_LEVELS;l++)
	{
		int curbsize=blocksize>>l;
		SAFE_RELEASE(dx8_ibufs[l]);
		Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*(2*(curbsize+4)*curbsize),
			D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibufs[l]);

		// Generate faces...

		// Lock indexbuffer
		WORD *fs=NULL;
		
		// BUG: Can specify D3DLOCK_DISCARD or D3DLOCK_NOOVERWRITE for only Index Buffers created with D3DUSAGE_DYNAMIC
		// FixMe: This fix stalls pipeline
		//	-- psd
			//dx8_ibufs[l]->Lock(0,sizeof(WORD)*(2*(curbsize+4)*curbsize),(BYTE**)&fs,D3DLOCK_DISCARD);
		dx8_ibufs[l]->Lock(0,sizeof(WORD)*(2*(curbsize+4)*curbsize),(BYTE**)&fs,0);

		// Do facestrip (optimized for vertexcache)
		int x,y;
		int dir=0;
		face_amounts[l]=0;
		for (y=0;y<curbsize;y++)
		{	
			for (x=0;x<(curbsize+2);x++)
			{
				if (dir==0)
				{
					if (x==(curbsize+1))	// Do stripjoin
					{
						// No join at the last row
						if (y<(curbsize-1))
						{
							// Add extra index to the array
							// Otherwise culling does not work correctly
							fs[face_amounts[l]++]=(y+1)*(curbsize+1)+x-1;
						}
					}
					else	// Basic stuff
					{
						// Add 2 indexes in to the array
						fs[face_amounts[l]++]=y*(curbsize+1)+x;
						fs[face_amounts[l]++]=(y+1)*(curbsize+1)+x;
					}
				}
				else
				{
					if (x==(curbsize+1))	// Do stripjoin
					{
						// No join at the last row
						if (y<(curbsize-1))
						{
							// Add extra index to the array
							// Otherwise culling does not work correctly
							fs[face_amounts[l]++]=(y+1)*(curbsize+1);
						}
					}
					else	// Basic stuff
					{
						// Add 2 indexes in to the array
						fs[face_amounts[l]++]=y*(curbsize+1)+(curbsize-x);
						fs[face_amounts[l]++]=(y+1)*(curbsize+1)+(curbsize-x);
					}
				}
			}

			// Change the direction (in each row)
			if (dir==0) dir=1; else dir=0;
		}		
		
		//for (y=0;y<curbsize;y++)
		//{	
		//	for (x=0;x<(curbsize+1);x++)
		//	{
		//		if (dir==0)
		//		{
		//			// Add 2 indexes in to the array
		//			fs[face_amounts[l]++]=y*(curbsize+1)+x;
		//			fs[face_amounts[l]++]=(y+1)*(curbsize+1)+x;
		//		}
		//		else
		//		{
		//			// Add 2 indexes in to the array
		//			fs[face_amounts[l]++]=y*(curbsize+1)+(curbsize-x);
		//			fs[face_amounts[l]++]=(y+1)*(curbsize+1)+(curbsize-x);
		//		}
		//	}

			// Change the direction (in each row)
			if (dir==0) dir=1; else dir=0;
		}

		face_amounts[l]-=2;	// First 2 indices in strip are not faces

		// Unlock indexbuffer
		dx8_ibufs[l]->Unlock();
	}

	// Delete old blocks (if exists)
	if (blocks)
	{
		// Delete all blocks
		for (int i=0;i<bmapsize.y*bmapsize.x;i++) delete blocks[i];

		// Delete block pointer array
		delete[] blocks;
	}

	// Create block pointer array
	bmapsize=hmap_size/blocksize;
	blocks=new PTRBLOCK[bmapsize.x*bmapsize.y];

	// Create all blocks
	for (int y=0;y<bmapsize.y;y++)
	for (int x=0;x<bmapsize.x;x++)
	{
		blocks[(y*bmapsize.x)+x]=new TRBlock(this,VC2I(x*blocksize,y*blocksize));
	}
}

	
//------------------------------------------------------------------
// Block size (Set only at beginning)
//------------------------------------------------------------------
void Storm3D_Terrain::SetBlockSize(int bsize)
{
	// Set blocksize
	//blocksize=bsize;
	blocksize=bsize*2;	// Interpolated hmap version

	// Recreate blocks
	RecreateBlocks();
}


//------------------------------------------------------------------
// Heightmap stuff
//------------------------------------------------------------------
void Storm3D_Terrain::SetHeightMap(WORD *mapdata,const VC2I &map_size,const VC3 &real_size, WORD *forcemap)
{
	// NOTICE: not copied to storm! reference outside storm!
	this->forcemap = forcemap;

	// Set parameters
	//hmap_size=map_size;
	hmap_size=map_size*2;		// 2x2 Interpolated version

	// psd
//	hmap_size=map_size;

	size=real_size;
	mmult_map_world=size/VC3(hmap_size.x,65535,hmap_size.y);	
	mmult_world_map=VC3(hmap_size.x,65535,hmap_size.y)/size;

	// Calculate hmap shift (optimization)
	hmapsh=0;
	switch (hmap_size.x)
	{
		case 1: hmapsh=0; break;
		case 2: hmapsh=1; break;
		case 4: hmapsh=2; break;
		case 8: hmapsh=3; break;
		case 16: hmapsh=4; break;
		case 32: hmapsh=5; break;
		case 64: hmapsh=6; break;
		case 128: hmapsh=7; break;
		case 256: hmapsh=8; break;
		case 512: hmapsh=9; break;
		case 1024: hmapsh=10; break;
		case 2048: hmapsh=11; break;
		case 4096: hmapsh=12; break;
		case 8192: hmapsh=13; break;
		case 16384: hmapsh=14; break;
		case 32768: hmapsh=15; break;
		case 65536: hmapsh=16; break;
	}

//	hmap_size = map_size * 2;

	// Allocate memory for terrain heightmap
	// 16x16 extra size must be allocated because of some optimizations
	hmap=new WORD[(hmap_size.x+16)*(hmap_size.y+16)];
	ZeroMemory(hmap, (hmap_size.x+16)*(hmap_size.y+16)*sizeof(WORD));
	hmap=&hmap[(hmap_size.x*16)+16];

//	hmap_size = map_size;
	// Copy heightmap
	//for (int y=0;y<hmap_size.y;y++)
	//for (int x=0;x<hmap_size.x;x++)
	//{
	//	hmap[(y<<hmapsh)+x]=map[(y<<hmapsh)+x];
	//}

	// 2x2 Interpolated version...

	// Copy heightmap (fills each other pixel)
	// The heightmap size is 2x2 bigger than source, because of the detail addition in near blocks
	for (int y=0;y<hmap_size.y;y+=2)
	for (int x=0;x<hmap_size.x;x+=2)
	{
		hmap[(y<<hmapsh)+x]=mapdata[((y>>1)<<(hmapsh-1))+(x>>1)];
	}

	// leave a little zero values at edges (cos they wrap around to 
	// the other side of the map.)
	// -jpk
	for (int cy=0;cy<hmap_size.y;cy+=2)
	{
		hmap[(cy<<hmapsh)+0]=0;
		hmap[(cy<<hmapsh)+(hmap_size.x-2)]=0;
		hmap[(cy<<hmapsh)+2] /= 2;
		hmap[(cy<<hmapsh)+(hmap_size.x-4)] /= 2;
	}
	for (int cx=0;cx<hmap_size.x;cx+=2)
	{
		hmap[(0<<hmapsh)+cx]=0;
		hmap[((hmap_size.y-2)<<hmapsh)+cx]=0;
		hmap[(2<<hmapsh)+cx] /= 2;
		hmap[((hmap_size.y-4)<<hmapsh)+cx] /= 2;
	}

	// Interpolate missing pixels (even rows)
	for (y=0;y<hmap_size.y;y+=2)
	{
		int yp=(y<<hmapsh);
		for (int x=3;x<hmap_size.x-2;x+=2)	// starts at 3 (should be 1), ends at -2
		{
			// Linear interpolation first
			hmap[yp+x]=((hmap[yp+x-1]+hmap[yp+x+1])>>1);

			// back to linear, it's better in our case... -jpk

			// Modify depending on tangents
			//int xn2=hmap[yp+x-3];
			//int xn1=hmap[yp+x-1];
			//int xp1=hmap[yp+x+1];
			//int	xp2=hmap[yp+x+3];
			//int cur=(xn1+xp1)>>1;
			//int tann2=xn1-xn2;
			//int tanp2=xp2-xp1;
			//cur+=((tann2-tanp2)>>3);
			//if (cur<0) cur=0;
			//	else if (cur>65535) cur=65535;
			//hmap[yp+x]=cur;
		}
	}

	// Interpolate missing pixels (odd rows)
	for (y=3;y<hmap_size.y-2;y+=2)	// starts at 3 (should be 1), ends at -2
	{
		int yp=(y<<hmapsh);
		int ypm=((y-1)<<hmapsh);
		int ypp=((y+1)<<hmapsh);
		int ypm3=((y-3)<<hmapsh);
		int ypp3=((y+3)<<hmapsh);
		for (int x=0;x<hmap_size.x;x++)
		{
			// Linear interpolation first
			hmap[yp+x]=((hmap[ypm+x]+hmap[ypp+x])>>1);

			// back to linear, it's better in our case... -jpk

			// Modify depending on tangents
			//int xn2=hmap[ypm3+x];
			//int xn1=hmap[ypm+x];
			//int xp1=hmap[ypp+x];
			//int	xp2=hmap[ypp3+x];
			//int cur=(xn1+xp1)>>1;
			//int tann2=xn1-xn2;
			//int tanp2=xp2-xp1;
			//cur+=((tann2-tanp2)>>3);
			//if (cur<0) cur=0;
			//	else if (cur>65535) cur=65535;
			//hmap[yp+x]=cur;
		}
	}

  // forced heightmap map
  if (forcemap != NULL)
  {
	  for (int fy=0;fy<hmap_size.y;fy+=2)
	  for (int fx=0;fx<hmap_size.x;fx+=2)
	  {
      WORD val = forcemap[((fy>>1)<<(hmapsh-1))+(fx>>1)];
      if (val > hmap[(fy<<hmapsh)+fx])
      {
				WORD newforce = hmap[(fy<<hmapsh)+fx];
  		  hmap[(fy<<hmapsh)+fx]=val;
  		  hmap[(fy<<hmapsh)+fx+1]=val;
  		  hmap[((fy+1)<<hmapsh)+fx]=val;
  		  hmap[((fy+1)<<hmapsh)+fx+1]=val;
				// then set the forcemap to previous hmap value.
				forcemap[((fy>>1)<<(hmapsh-1))+(fx>>1)] = newforce;
      }
	  }
  }

	// Recreate blocks
	RecreateBlocks();
}

//------------------------------------------------------------------
// The obstacle haxoring
//------------------------------------------------------------------

void Storm3D_Terrain::SetObstacleHeightmap(WORD *obstacleHeightmap)
{
  this->obstacleHeightmap = obstacleHeightmap;
  RecreateCollisionMap();
}

//------------------------------------------------------------------
// Editing
//------------------------------------------------------------------
void Storm3D_Terrain::Paint(const VC2 &position,float brush_radius,int texnum, int max_change, int max_value)
{
	if (brush_radius<0.01f) return;
	VC2I pint=ConvertWorldToMap(position);
	int bint=brush_radius*mmult_world_map.x;

	// Precalc
	int hx=hmap_size.x/2;
	int hy=hmap_size.y/2;
	float mx=size.x/(float)hmap_size.x;
	float my=size.z/(float)hmap_size.y;
	float hmx=mx/2;
	float hmy=my/2;

	int xmin=(pint.x-bint-1);
	int xmax=(pint.x+bint+1);
	int ymin=(pint.y-bint-1);
	int ymax=(pint.y+bint+1);

	// psd .. sigh
	if((xmin <= 0) || (ymin <= 0))
		return;
	if((xmax >= hmap_size.x-1) || (ymax >= hmap_size.y-1))
		return;

	for (int y=ymin;y<=ymax;y++)
	for (int x=xmin;x<=xmax;x++)
	{
		// Calculate upper-corner real-world coordinates
		float upx=((float)(x-hx))*mx;
		float upy=((float)(y-hy))*my;

		// Calculate range to brush center
		VC2 bcenter(upx+hmx,upy+hmy);
		float ran=position.GetRangeTo(bcenter);

		int mpos=(y<<hmapsh)+x;

		// Is it in the area of influence?
		if (ran<brush_radius)
		{
			// Calculate influence
			float influence=(1.0f-(ran/brush_radius));
			//int amountplus=255*influence;
			// psd
			int amountplus = max_change*influence;

			// small amountplus values causing problems... ?
			// i dunno why.
			// --jpk
			if (amountplus < 33) amountplus = 33;

			int added_to=-1;

			// Check if the texture already exists in pixel
			int i;
			for (i=0;i<TPIX_MAX_TEXTURES;i++)
				if (tmap[mpos].textures[i].texture==texnum) break;

			// Does it exist?
			if (i<TPIX_MAX_TEXTURES) // It exists
			{
				// psd
				//if(amountplus > max_change)
					//amountplus = max_change;

				// psd: Amount already too high
				if(tmap[mpos].textures[i].amount > max_value)
				{
					continue;
				} 

				// Add amount (i has correct texture number)
				int new_amount=((int)tmap[mpos].textures[i].amount)+amountplus;
				if (new_amount>255) new_amount=255; else if (new_amount<0) new_amount=0;
				tmap[mpos].textures[i].amount=new_amount;

				// psd
				if(tmap[mpos].textures[i].amount > max_value)
					tmap[mpos].textures[i].amount = max_value;				

				// Set "added_to"
				added_to=i;
			}
			else // Does not exist
			{
				// Add it...

				// Search for free slot
				for (int i=0;i<TPIX_MAX_TEXTURES;i++)
					if (tmap[mpos].textures[i].texture==255) break;

				// Make a free slot if not available
				if (i>=TPIX_MAX_TEXTURES)
				{
					// Search texture with the lowest amount (and set it into i)
					// So the lowest texture is overwritten with this new one
					i=0;
					for (int a=1;a<TPIX_MAX_TEXTURES;a++) 
						if (tmap[mpos].textures[a].amount<
							tmap[mpos].textures[i].amount) i=a;
				}

				// Add texture to the free slot (i has correct texture number)
				tmap[mpos].textures[i].texture=texnum;

				// psd
				//if(amountplus > max_change)
					//amountplus = max_change;

				// Set amount
				tmap[mpos].textures[i].amount=amountplus;
		
				// psd
				if(tmap[mpos].textures[i].amount > max_value)
					tmap[mpos].textures[i].amount=max_value;

				// Set "added_to"
				added_to=i;
			}


			// Decrease other's textures amount in this pixel (sum must be 255 at all times)
			if (added_to>=0)
			{
				// Calculate sum
				int sum=0;
				for (i=0;i<TPIX_MAX_TEXTURES;i++)
				if (tmap[mpos].textures[i].texture!=255)
				{
					sum+=tmap[mpos].textures[i].amount;
				}

				// Modify (other than "added_to")
				float multiplier=((float)(512-sum))/255.0f;
				for (i=0;i<TPIX_MAX_TEXTURES;i++) if (i!=added_to)
				{
					int newamo=(((float)tmap[mpos].textures[i].amount)*multiplier);
					if (newamo>0)
					{
						tmap[mpos].textures[i].amount=newamo;
					}
					else
					{
						tmap[mpos].textures[i].texture=255;
						tmap[mpos].textures[i].amount=0;
					}
				}
			}
		}
	}

	// Reblend affected map portion
	BlendLightShadowAndTexturing(Rect<int>(VC2I(xmin,ymin),VC2I(xmax,ymax)));

	// Transform to blocks
	xmin/=blocksize;
	xmax/=blocksize;
	ymin/=blocksize;
	ymax/=blocksize;

	// Degenerate affected blocks
	blocks[(ymin*bmapsize.x)+xmin]->DeGenerateHeightMesh();
	blocks[(ymin*bmapsize.x)+xmax]->DeGenerateHeightMesh();
	blocks[(ymax*bmapsize.x)+xmin]->DeGenerateHeightMesh();
	blocks[(ymax*bmapsize.x)+xmax]->DeGenerateHeightMesh();
}



void Storm3D_Terrain::ModifyHeight(const VC2 &position,float brush_radius,float amount)
{
	if (brush_radius<0.01f) return;
	VC2I pint=ConvertWorldToMap(position);
	int bint=brush_radius*mmult_world_map.x;

	// Precalc
	int hx=hmap_size.x/2;
	int hy=hmap_size.y/2;
	float mx=size.x/(float)hmap_size.x;
	float my=size.z/(float)hmap_size.y;
	float hmx=mx/2;
	float hmy=my/2;

	int xmin=(pint.x-bint-1);
	int xmax=(pint.x+bint+1);
	int ymin=(pint.y-bint-1);
	int ymax=(pint.y+bint+1);

  if (xmin < 0) xmin = 0;
  if (ymin < 0) ymin = 0;
  if (xmax >= hmap_size.x) xmax = hmap_size.x - 1;
  if (ymax >= hmap_size.y) ymax = hmap_size.y - 1;

	for (int y=ymin;y<=ymax;y++)
	for (int x=xmin;x<=xmax;x++)
	{
		// Calculate upper-corner real-world coordinates
		float upx=((float)(x-hx))*mx;
		float upy=((float)(y-hy))*my;

		// Calculate range to brush center
		VC2 bcenter(upx+hmx,upy+hmy);
		float ran=position.GetRangeTo(bcenter);

		// Is it in the area of influence?
		if (ran<brush_radius)
		{
			// Calculate influence
			float influence=(1.0f-(ran/brush_radius));

			// Raise the block
			int newval=hmap[(y<<hmapsh)+x]+amount*influence;
			if (newval<0) newval=0; else if (newval>65535) newval=65535;
			if (forcemap != NULL)
			{
				int forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];

				// now, interpolate forcemap a bit...
				// WARNING: does not check for map boundaries!
				// presumes "safety area" around map.
				if ((x & 1) == 1)
				{
					if ((y & 1) == 1)
					{
						forceval += forcemap[(((y>>1)+1)<<(hmapsh-1))+(x>>1)];
						forceval += forcemap[((y>>1)<<(hmapsh-1))+(x>>1)+1];
						forceval /= 3;
					} else {
						forceval += forcemap[((y>>1)<<(hmapsh-1))+(x>>1)+1];
						forceval /= 2;
					}
				} else {
					if ((y & 1) == 1)
					{
						forceval += forcemap[(((y>>1)+1)<<(hmapsh-1))+(x>>1)];
						forceval /= 2;
					}
				}

				if (forceval != 0 && newval < forceval) newval = (WORD)forceval;
			}
			hmap[(y<<hmapsh)+x]=newval;
		}
	}

	// Reblend affected map portion
	BlendLightShadowAndTexturing(Rect<int>(VC2I(xmin,ymin),VC2I(xmax,ymax)));

  // Redo the collision map (raytrace optimization) for that area
	RecreateCollisionMapArea(xmin, ymin, xmax + 1, ymax + 1);
	// TEMP...
	//RecreateCollisionMap();

	// Transform to blocks
	xmin/=blocksize;
	xmax/=blocksize;
	ymin/=blocksize;
	ymax/=blocksize;

	for(int i = xmin; i <= xmax; ++i)
	for(int j = ymin; j <= ymax; ++j)
		blocks[(j*bmapsize.x)+i]->DeGenerateHeightMesh();
}



void Storm3D_Terrain::forcemapHeight(const VC2 &position, float radius, 
	bool above, bool below)
{
	if (forcemap == NULL) return;

	VC2I pint=ConvertWorldToMap(position);
	int bint=radius*mmult_world_map.x;

	// Precalc
	int hx=hmap_size.x/2;
	int hy=hmap_size.y/2;
	float mx=size.x/(float)hmap_size.x;
	float my=size.z/(float)hmap_size.y;
	float hmx=mx/2;
	float hmy=my/2;

	int xmin=(pint.x-bint-1);
	int xmax=(pint.x+bint+1);
	int ymin=(pint.y-bint-1);
	int ymax=(pint.y+bint+1);

  if (xmin < 0) xmin = 0;
  if (ymin < 0) ymin = 0;
  if (xmax >= hmap_size.x) xmax = hmap_size.x - 1;
  if (ymax >= hmap_size.y) ymax = hmap_size.y - 1;

	bool affected = false;

	for (int y=ymin;y<=ymax;y++)
	for (int x=xmin;x<=xmax;x++)
	{
		// Calculate upper-corner real-world coordinates
		float upx=((float)(x-hx))*mx;
		float upy=((float)(y-hy))*my;

		// Calculate range to brush center
		VC2 bcenter(upx+hmx,upy+hmy);
		float ran=position.GetRangeTo(bcenter);

		// Is it in the area of influence?
		if (ran<radius)
		{
			// Raise the block
			int newval=hmap[(y<<hmapsh)+x];
			if (newval<0) newval=0; else if (newval>65535) newval=65535;

			//WORD forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];
			int forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];

			// now, interpolate forcemap a bit...
			// WARNING: does not check for map boundaries!
			// presumes "safety area" around map.
			if ((x & 1) == 1)
			{
				if ((y & 1) == 1)
				{
					int v1 = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)+1];
					if (v1 == 0) v1 = forceval;
					int v2 = forcemap[(((y>>1)+1)<<(hmapsh-1))+(x>>1)];
					if (v2 == 0) v2 = forceval;
					if (forceval == 0)
					{
						if (v1 != 0 && v2 != 0)
							forceval = (v1 + v2) / 2;
						else
							forceval = 0;
					} else {
						forceval += v1 + v2;
						forceval /= 3;
					}
				} else {
					int v1 = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)+1];
					if (v1 == 0) v1 = forceval;
					if (forceval == 0 && v1 != 0)
						forceval = v1;
					forceval += v1;
					forceval /= 2;
				}
			} else {
				if ((y & 1) == 1)
				{
					int v1 = forcemap[(((y>>1)+1)<<(hmapsh-1))+(x>>1)];
					if (v1 == 0) v1 = forceval;
					if (forceval == 0 && v1 != 0)
						forceval = v1;
					forceval += v1;
					forceval /= 2;
				}
			}

			if (forceval != 0)
			{
				if ((newval < forceval && above)
					|| (newval > forceval && below)) 
				{
					affected = true;
					newval = forceval;
					hmap[(y<<hmapsh)+x]=newval;
				}
			}
		}
	}

  if (!affected) return;

	// Reblend affected map portion
	BlendLightShadowAndTexturing(Rect<int>(VC2I(xmin,ymin),VC2I(xmax,ymax)));

  // Redo the collision map (raytrace optimization) for that area
	RecreateCollisionMapArea(xmin, ymin, xmax + 1, ymax + 1);
	// TEMP...
	//RecreateCollisionMap();

	// Transform to blocks
	xmin/=blocksize;
	xmax/=blocksize;
	ymin/=blocksize;
	ymax/=blocksize;

	for(int i = xmin; i <= xmax; ++i)
	for(int j = ymin; j <= ymax; ++j)
		blocks[(j*bmapsize.x)+i]->DeGenerateHeightMesh();
}



void Storm3D_Terrain::SphereCutoff(const VC3 &position,float radius,float depth)
{
	if (radius<0.01f) return;

  VC2 pos=VC2(position.x, position.z);
	VC2I pint=ConvertWorldToMap(pos);
	int bint=radius*mmult_world_map.x;
	
  int hint=position.y*mmult_world_map.y;

	// Precalc
	int hx=hmap_size.x/2;
	int hy=hmap_size.y/2;
	float mx=size.x/(float)hmap_size.x;
	float my=size.z/(float)hmap_size.y;
	float hmx=mx/2;
	float hmy=my/2;

	int xmin=(pint.x-bint-1);
	int xmax=(pint.x+bint+1);
	int ymin=(pint.y-bint-1);
	int ymax=(pint.y+bint+1);

  if (xmin < 0) xmin = 0;
  if (ymin < 0) ymin = 0;
  if (xmax >= hmap_size.x) xmax = hmap_size.x - 1;
  if (ymax >= hmap_size.y) ymax = hmap_size.y - 1;

	for (int y=ymin;y<=ymax;y++)
	for (int x=xmin;x<=xmax;x++)
	{
		// Calculate upper-corner real-world coordinates
		float upx=((float)(x-hx))*mx;
		float upy=((float)(y-hy))*my;

		// Calculate range to brush center
		VC2 bcenter(upx+hmx,upy+hmy);
		float ran=pos.GetRangeTo(bcenter);

		// Is it in the area of influence?
		if (ran<radius)
		{
			// Calculate influence
			float influence=(1.0f-(ran/radius));

			// Cut off the block
  	  int newval=((float)hint-depth)*influence + ((float)hmap[(y<<hmapsh)+x])*(1.0f-influence);
		  if (newval<0) newval=0; else if (newval>65535) newval=65535;
			if (forcemap != NULL)
			{
				//WORD forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];
				//if (forceval != 0 && newval < forceval) newval = forceval;

				int forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];

				// now, interpolate forcemap a bit...
				// WARNING: does not check for map boundaries!
				// presumes "safety area" around map.
				if ((x & 1) == 1)
				{
					if ((y & 1) == 1)
					{
						forceval += forcemap[(((y>>1)+1)<<(hmapsh-1))+(x>>1)];
						forceval += forcemap[((y>>1)<<(hmapsh-1))+(x>>1)+1];
						forceval /= 3;
					} else {
						forceval += forcemap[((y>>1)<<(hmapsh-1))+(x>>1)+1];
						forceval /= 2;
					}
				} else {
					if ((y & 1) == 1)
					{
						forceval += forcemap[(((y>>1)+1)<<(hmapsh-1))+(x>>1)];
						forceval /= 2;
					}
				}

				if (forceval != 0 && newval < forceval) newval = (WORD)forceval;
			}
      if (depth > 0)
      {
        if ((int)hmap[(y<<hmapsh)+x] > newval)
        {
	  		  hmap[(y<<hmapsh)+x]=newval;
        }
      } else {
        if ((int)hmap[(y<<hmapsh)+x] < newval)
        {
	  		  hmap[(y<<hmapsh)+x]=newval;
        }
      }
		}
	}

	// Reblend affected map portion
	BlendLightShadowAndTexturing(Rect<int>(VC2I(xmin,ymin),VC2I(xmax,ymax)));

  // Redo the collision map (raytrace optimization) for that area
	RecreateCollisionMapArea(xmin, ymin, xmax + 1, ymax + 1);
	// TEMP...
  //RecreateCollisionMap();

	// Transform to blocks
	xmin/=blocksize;
	xmax/=blocksize;
	ymin/=blocksize;
	ymax/=blocksize;

	for(int i = xmin; i <= xmax; ++i)
	for(int j = ymin; j <= ymax; ++j)
		blocks[(j*bmapsize.x)+i]->DeGenerateHeightMesh();
}


//------------------------------------------------------------------
// Object group visibility (new)
//------------------------------------------------------------------
void Storm3D_Terrain::SetObjectGroupVisibilityRange(BYTE group_id,float range)
{
	obj_group_vis_range[group_id]=range;
}


//------------------------------------------------------------------
// Models in terrain
// Copies of models in terrain (grass, plants, trees etc)
//------------------------------------------------------------------
void Storm3D_Terrain::AddModelCopy(IStorm3D_Model *model,const VC3 &position,const QUAT &rotation,BYTE group_id)
{
	// Calculate world->bmap
	float mx=size.x/(float)bmapsize.x;
	float my=size.z/(float)bmapsize.y;
	int hx=bmapsize.x/2;
	int hy=bmapsize.y/2;
	int bx=(position.x/mx)+hx;
	int by=(position.z/my)+hy;

	// Calculate inblock coordinates
	VC3 bp;
	bp.x=fmod(position.x,mx);
	bp.z=fmod(position.z,my);
	if (bp.x<0) bp.x=mx+bp.x;
	if (bp.z<0) bp.z=my+bp.z;
	bp.y=position.y;

	// Add model copy to block
	blocks[(by*bmapsize.x)+bx]->AddModelCopy((Storm3D_Model*)model,bp,rotation,position,group_id);
}

VC3 Storm3D_Terrain::RemoveModelCopy(int model_id, const VC3 &position)
{
	// Calculate world->bmap
	float mx=size.x/(float)bmapsize.x;
	float my=size.z/(float)bmapsize.y;
	int hx=bmapsize.x/2;
	int hy=bmapsize.y/2;
	int bx=(position.x/mx)+hx;
	int by=(position.z/my)+hy;

	// Add model copy to block
	return blocks[(by*bmapsize.x)+bx]->RemoveModelCopy(model_id);
}


//------------------------------------------------------------------
// Sunlight
//------------------------------------------------------------------
void Storm3D_Terrain::SetSunlight(VC3 direction,COL color)
{
	// psd .. hangs on 0 components
	if(fabsf(direction.x) < 0.1f)
		direction.x = .1f;
	if(fabsf(direction.y) < 0.1f)
		direction.y = .1f;
	if(fabsf(direction.z) < 0.1f)
		direction.z = .1f;

	sun_dir=direction.GetNormalized();
	sun_col=color.GetClamped();

	RecreateBlocks();

	Vector foo = direction;
	foo.Normalize();
	foo.x = -foo.x;
	foo.y = -foo.y;
	foo.z = -foo.z;

	Storm3D_ShaderManager::GetSingleton()->SetLight(foo, color);
}


//------------------------------------------------------------------
// Test collision etc
//------------------------------------------------------------------
float Storm3D_Terrain::GetHeightAt(const VC2 &position) const
{
	// OLD VERSION (NO INTERPOLATION)
	//VC2I pint=ConvertWorldToMap(position);
	//return (float)hmap[(pint.y<<hmapsh)+pint.x]*mmult_map_world.y;

	// psd: have to fix this
	VC2I pint = ConvertWorldToMap(position);
	if((pint.x < 0) || (pint.y < 0))
		return 0;
	if((pint.x > hmap_size.x) || (pint.y > hmap_size.y))
		return 0;


	// New version with interpolation...

	// Calculate position (ix,iy) and interpolation position (ip_x,ip_y)
	VC2 pmap=ConvertWorldToMapFloat(position);
	int ix=(int)pmap.x;
	int iy=(int)pmap.y;
	float ip_x=pmap.x-(float)ix;
	float ip_y=pmap.y-(float)iy;

	// Select quads face at coordinates
	if ((ip_x+ip_y)<=1)	// Upper left face
	{
		// Calculate edges
		float p0=(float)hmap[(iy<<hmapsh)+ix]*mmult_map_world.y;
		float px=(float)hmap[(iy<<hmapsh)+ix+1]*mmult_map_world.y;
		float py=(float)hmap[((iy+1)<<hmapsh)+ix]*mmult_map_world.y;

		// Interpolate
		return p0+(px-p0)*ip_x+(py-p0)*ip_y;
	}
	else	// Lower right face
	{
		// Calculate edges
		float pxy=(float)hmap[((iy+1)<<hmapsh)+ix+1]*mmult_map_world.y;
		float px=(float)hmap[(iy<<hmapsh)+ix+1]*mmult_map_world.y;
		float py=(float)hmap[((iy+1)<<hmapsh)+ix]*mmult_map_world.y;

		// Interpolate
		return pxy+(py-pxy)*(1.0f-ip_x)+(px-pxy)*(1.0f-ip_y);
	}
}


BYTE Storm3D_Terrain::GetTextureAmountAt(int texture,const VC2 &position) const
{
	// NO INTERPOLATION (It's slow, and it's not really needed)
	VC2I pint=ConvertWorldToMap(position);
	int amo=0;
	for (int i=0;i<TPIX_MAX_TEXTURES;i++)
	if (tmap[(pint.y<<hmapsh)+pint.x].textures[i].texture==texture)
	{
		amo+=tmap[(pint.y<<hmapsh)+pint.x].textures[i].amount;
	}
	if (amo>255) amo=255;
	return amo;
}


void Storm3D_Terrain::RecreateCollisionMapArea(int x1, int y1, int x2, int y2)
{
  // create a low resolution map of height + obstacle map.
  int collShift = hmapsh - COLLISION_MAP_DIV_SHIFT;
  int startX = (x1 >> COLLISION_MAP_DIV_SHIFT);
  int startY = (y1 >> COLLISION_MAP_DIV_SHIFT);
  int endX = ((x2-1) >> COLLISION_MAP_DIV_SHIFT);
  int endY = ((y2-1) >> COLLISION_MAP_DIV_SHIFT);
  int hx;
  int hy;
  int ox;
  int oy;
  int maxh = 0;

  hy = startY << COLLISION_MAP_DIV_SHIFT;
  oy = hy << OBSTACLE_MAP_MULT_SHIFT;
  for (int y = startY; y <= endY; y++)
  {
    hx = startX << COLLISION_MAP_DIV_SHIFT;
    ox = hx << OBSTACLE_MAP_MULT_SHIFT;
    for (int x = startX; x <= endX; x++)
    {
      maxh = 0;
      // choose highest height+obstacle block for the collision map.
      for (int offy = 0; offy < COLLISION_MAP_DIVIDER * OBSTACLE_MAP_MULTIPLIER; offy++)
      {
        for (int offx = 0; offx < COLLISION_MAP_DIVIDER * OBSTACLE_MAP_MULTIPLIER; offx++)
        {
          int hmapIndex = (hx + (offx >> OBSTACLE_MAP_MULT_SHIFT)) + 
            ((hy + (offy >> OBSTACLE_MAP_MULT_SHIFT)) << hmapsh);
          assert(hmapIndex >= 0 && hmapIndex < hmap_size.x * hmap_size.y);
          int tmp = hmap[hmapIndex];
          int omapIndex = (ox + offx) + ((oy + offy)<<(hmapsh+OBSTACLE_MAP_MULT_SHIFT));
          assert(omapIndex >= 0 && omapIndex < hmap_size.x * hmap_size.y * OBSTACLE_MAP_MULTIPLIER * OBSTACLE_MAP_MULTIPLIER);
					if (obstacleHeightmap != NULL)
					{
						WORD otmp = obstacleHeightmap[omapIndex];
						// ignore such obstacles that are seethrough and unhittable
						if ((otmp & (OBSTACLE_MAP_MASK_SEETHROUGH | OBSTACLE_MAP_MASK_UNHITTABLE)) 
							!= (OBSTACLE_MAP_MASK_SEETHROUGH | OBSTACLE_MAP_MASK_UNHITTABLE))
						{
							tmp += otmp;
						}
						if (tmp > maxh)
						{
							maxh = tmp;
						}
					}
        }
      }
      collisionMap[x + (y << collShift)] = maxh;
      hx += COLLISION_MAP_DIVIDER;
      ox += COLLISION_MAP_DIVIDER * OBSTACLE_MAP_MULTIPLIER;
    }
    hy += COLLISION_MAP_DIVIDER;
    oy += COLLISION_MAP_DIVIDER * OBSTACLE_MAP_MULTIPLIER;
  }
}


void Storm3D_Terrain::RecreateCollisionMap()
{
  // collision map not yet initialized?
  if (collisionMap == NULL)
  {
    int size = (hmap_size.x>>COLLISION_MAP_DIV_SHIFT) 
      * (hmap_size.y>>COLLISION_MAP_DIV_SHIFT);
    collisionMap = new WORD[size];
    for (int i = 0; i < size; i++) collisionMap[i] = 0;
  }

  RecreateCollisionMapArea(0, 0, hmap_size.x, hmap_size.y);
}


void Storm3D_Terrain::RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate, bool lineOfSight) const
{
  if (accurate)
  {
		// damn, can't have this here if the raytrace is const... =/
		//if (collisionMap == NULL)
		//  RecreateCollisionMap();

    // alternative method for terrain raytrace, more accurate, but slower
    // not very clear code, quickly transformed from a 2d line drawing
    // routine. -jpk

    VC3 epos(position+direction_normalized*ray_length);

	  //VC2I ispos=ConvertWorldToMap(VC2(position.x,position.z));
	  //VC2I iepos=ConvertWorldToMap(VC2(epos.x,epos.z));
    VC2I ispos=ConvertWorldToObstacleMap(VC2(position.x,position.z));
	  VC2I iepos=ConvertWorldToObstacleMap(VC2(epos.x,epos.z));

    int i;
    int steep = 0;
    int sx, sy;
    int dx, dy;
    int e;

		WORD skipObstacleMask;
		if (lineOfSight)
		{
			skipObstacleMask = OBSTACLE_MAP_MASK_SEETHROUGH;
		} else {
			skipObstacleMask = OBSTACLE_MAP_MASK_UNHITTABLE;
		}

    int x, x2;
    int y, y2;
    //int maxIndex = hmap_size.x * hmap_size.y;
    int h;
	  //int hs = ((int)((1<<22)+position.y * mmult_world_map.y)) << 8;
	  //int he = ((int)((1<<22)+epos.y * mmult_world_map.y)) << 8;
	  int hs = position.y * mmult_world_map.y;
	  int he = epos.y * mmult_world_map.y;
    int hdiff = he - hs;

    int obstacleShift = hmapsh + OBSTACLE_MAP_MULT_SHIFT;
    int collShift = hmapsh - COLLISION_MAP_DIV_SHIFT;

    // obstacle to hmap shift
    // can't do it like this, would result into "1/2 value" multiplications
    //int o2hblockShift = hmapsh - OBSTACLE_MAP_MULT_SHIFT;
    // obstacle to collision shift
    //int o2cblockShift = hmapsh - OBSTACLE_MAP_MULT_SHIFT 
    //  - COLLISION_MAP_DIV_SHIFT;

    x = ispos.x;
    y = ispos.y;
    x2 = iepos.x;
    y2 = iepos.y;

    dx = abs(x2 - x);
    sx = ((x2 - x) > 0) ? 1 : -1;
    dy = abs(y2 - y);
    sy = ((y2 - y) > 0) ? 1 : -1;

    if (dy > dx) 
    {
	    steep = 1;
	    x ^= y;
	    y ^= x;
	    x ^= y;
	    dx ^= dy;
	    dy ^= dx;
	    dx ^= dy;
	    sx ^= sy;
	    sy ^= sx;
	    sx ^= sy;
    }

    //int maxx = hmap_size.x<<OBSTACLE_MAP_MULT_SHIFT;
    //int maxy = hmap_size.y<<OBSTACLE_MAP_MULT_SHIFT;
    int maxcx = hmap_size.x>>COLLISION_MAP_DIV_SHIFT;
    int maxcy = hmap_size.y>>COLLISION_MAP_DIV_SHIFT;
    int prevcx = -1;
    int prevcy = -1;
    //WORD collHeight = 0;
    int collHeight = 0;

    e = 2 * dy - dx;
    for (i = 0; i < dx; i++) 
    {
      //h = hs + hdiff * i / dx;
      // for better accuracy...
      h = 3 * hs + (3 * hdiff * i) / dx;

      int cy;
      int cx;
      if (steep) 
      {
        // notice: x and y swapped here!
        cx = y>>(COLLISION_MAP_DIV_SHIFT + OBSTACLE_MAP_MULT_SHIFT);
        cy = x>>(COLLISION_MAP_DIV_SHIFT + OBSTACLE_MAP_MULT_SHIFT);
      } else {
        cx = x>>(COLLISION_MAP_DIV_SHIFT + OBSTACLE_MAP_MULT_SHIFT);
        cy = y>>(COLLISION_MAP_DIV_SHIFT + OBSTACLE_MAP_MULT_SHIFT);
      }
      if (cx != prevcx || cy != prevcy)
      {
        prevcx = cx;
        prevcy = cy;

        // interpolation (below) will f*ck up if we don't keep a little
        // safety distance to map edges...
        
        //if (cx < 0 || cx >= maxcx
        //  || cy < 0 || cy >= maxcy)
        //  return;
        
        // ...thus this...
        if (cx <= 0 || cx >= maxcx-1
          || cy <= 0 || cy >= maxcy-1)
          return;

        //collHeight = collisionMap[(cy<<collShift) + cx];
        collHeight = 3 * (int)collisionMap[(cy<<collShift) + cx];
      }
      if (collHeight > h)
      {
        int blockIndex;
        int obstacleBlockIndex;

        // improve raytrace accuracy with hmap interpolation... slow :(
        // designed for obstacle/height map multiplier 2 (should work for 
        // other multiplier values too, but result may be less desirable)
        //int blockIndexU;
        //int blockIndexL;
        int blockIndexR;
        int blockIndexD;

        if (steep) 
        {
          blockIndex = ((x>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (y>>OBSTACLE_MAP_MULT_SHIFT);
          //blockIndexU = ((x>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + ((y-1)>>OBSTACLE_MAP_MULT_SHIFT);
          blockIndexD = ((x>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + ((y+1)>>OBSTACLE_MAP_MULT_SHIFT);
          //blockIndexL = (((x-1)>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (y>>OBSTACLE_MAP_MULT_SHIFT);
          blockIndexR = (((x+1)>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (y>>OBSTACLE_MAP_MULT_SHIFT);
          obstacleBlockIndex = (x<<obstacleShift) + y;
        } else {
          blockIndex = ((y>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (x>>OBSTACLE_MAP_MULT_SHIFT);
          //blockIndexU = ((y>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + ((x-1)>>OBSTACLE_MAP_MULT_SHIFT);
          blockIndexD = ((y>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + ((x+1)>>OBSTACLE_MAP_MULT_SHIFT);
          //blockIndexL = (((y-1)>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (x>>OBSTACLE_MAP_MULT_SHIFT);
          blockIndexR = (((y+1)>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (x>>OBSTACLE_MAP_MULT_SHIFT);
          obstacleBlockIndex = (y<<obstacleShift) + x;
        }

		    // Test hit
        //WORD mapHeight = hmap[blockIndex];
        // average... gives us interpolation...
        //WORD mapHeight = ((int)hmap[blockIndex] + (int)hmap[blockIndexU] + (int)hmap[blockIndexD]
        //  + (int)hmap[blockIndexL] + (int)hmap[blockIndexR]) / 3;
        //int mapHeight = ((int)hmap[blockIndex] + (int)hmap[blockIndexU] + (int)hmap[blockIndexD]
        //  + (int)hmap[blockIndexL] + (int)hmap[blockIndexR]);
        int mapHeight = ((int)hmap[blockIndex] + (int)hmap[blockIndexR] 
          + (int)hmap[blockIndexD]);

		    if (mapHeight > h)
		    {
			    rti.hit = true;
			    rti.model = NULL;
			    rti.object = NULL;
          VC2 hitpos;
          if (steep) 
          {
            hitpos = ConvertObstacleMapToWorld(VC2I(y,x));
	          //hitpos = ConvertMapToWorld(VC2I(y>>OBSTACLE_MAP_MULT_SHIFT,x>>OBSTACLE_MAP_MULT_SHIFT));
          } else {
            hitpos = ConvertObstacleMapToWorld(VC2I(x,y));
	          //hitpos = ConvertMapToWorld(VC2I(x>>OBSTACLE_MAP_MULT_SHIFT,y>>OBSTACLE_MAP_MULT_SHIFT));
          }
          //VC3 hitvec = VC3(hitpos.x, hmap[blockIndex]*mmult_map_world.y, hitpos.y) - position;
          VC3 hitvec = VC3(hitpos.x, (mapHeight / 3)*mmult_map_world.y, hitpos.y) - position;
			    rti.range = hitvec.GetLength();
			    rti.position = position + direction_normalized*rti.range;
          assert(!(rti.position.x <= -(hmap_size.x * mmult_map_world.x)/2 
            || rti.position.x >= (hmap_size.x * mmult_map_world.x)/2
            || rti.position.z <= -(hmap_size.y * mmult_map_world.z)/2
            || rti.position.z >= (hmap_size.y * mmult_map_world.z)/2)
          );
			    return;
        } else {
          // hit obstacle?
          //assert(obstacleHeightmap != NULL);
  		    if (obstacleHeightmap != NULL
            && mapHeight + 3 * ((int)(obstacleHeightmap[obstacleBlockIndex] & OBSTACLE_MAP_MASK_HEIGHT)) > h)
//            && obstacleHeightmap[obstacleBlockIndex] != 0)
          {
						// make sure that the obstacle is something we want to 
						// hit... (not seethrough or unhittable)
            if ((obstacleHeightmap[obstacleBlockIndex] & skipObstacleMask) == 0)
            {
              oci.hit = true;
              VC2 hitpos;
              if (steep) 
              {
	              hitpos = ConvertObstacleMapToWorld(VC2I(y,x));
              } else {
	              hitpos = ConvertObstacleMapToWorld(VC2I(x,y));
              }
              //VC3 hitvec = VC3(hitpos.x, hmap[blockIndex]*mmult_map_world.y, hitpos.y) - position;
              VC3 hitvec = VC3(hitpos.x, (h / 3) *mmult_map_world.y, hitpos.y) - position;
              float hitveclen = hitvec.GetLength();
              if (oci.hitAmount == 0)
              {
  	  		      oci.range = hitveclen;
    			      oci.position = position + direction_normalized*oci.range;
              }
              if (oci.hitAmount < MAX_OBSTACLE_COLLISIONS)
              {
                oci.ranges[oci.hitAmount] = hitveclen;
              }
              oci.hitAmount++;
            }
          }
        }
      }
      while (e >= 0) 
      {
	      y += sy;
	      e -= 2 * dx;
	    }
	    x += sx;
	    e += 2 * dy;
    }
  } else {
    // The old, not so accurate, but pretty fast(?) implementation

    // Calculate ray end position
	  VC3 epos(position+direction_normalized*ray_length);

	  // Convert to hmap coordinates
	  VC2I ispos=ConvertWorldToMap(VC2(position.x,position.z));
	  VC2I iepos=ConvertWorldToMap(VC2(epos.x,epos.z));
	  int dx=iepos.x-ispos.x;
	  int dy=iepos.y-ispos.y;
	  int len=sqrt(dx*dx+dy*dy)+1;
	  
	  // Precalculate
	  int xadd=(dx<<2)/len;
	  int yadd=(dy<<2)/len;

	  // Precalc height
	  int hs=position.y*mmult_world_map.y;
	  int he=epos.y*mmult_world_map.y;
	  int hp=(hs<<2);
	  int hadd=((he-hs)<<2)/len;

	  // Test pixels
	  int xp=(ispos.x<<2);
	  int yp=(ispos.y<<2);
    int blockIndex;
    int maxIndex = hmap_size.x * hmap_size.y;
	  for (int i=0;i<len;i++)
	  if (hp<(16777215<<2))
	  {
      // out of boundaries?
      blockIndex = ((yp>>2)<<hmapsh)+(xp>>2);
      if (blockIndex < 0 || blockIndex >= maxIndex) return;
		  // Test hit
		  if (hmap[blockIndex]>(hp>>2))
		  {
			  rti.hit=true;
			  rti.model=NULL;
			  rti.object=NULL;
			  rti.range=((float)i/(float)len)*ray_length;
			  //rti.range=0.5f*ray_length;
			  rti.position=position+direction_normalized*rti.range;
			  return;
		  }

		  // Add
		  xp+=xadd;
		  yp+=yadd;
		  hp+=hadd;
	  }
  }


  
	// Calculate ray end position
	//VC3 epos(position+direction_normalized*ray_length);

	// Convert to hmap coordinates
	//VC2I ispos=ConvertWorldToMap(VC2(position.x,position.z));
	//VC2I iepos=ConvertWorldToMap(VC2(epos.x,epos.z));
	//int dx=iepos.x-ispos.x;
	//int dy=iepos.y-ispos.y;
	//int len=sqrt(dx*dx+dy*dy)+1;
	
	// Precalculate
	//int xadd=(dx<<8)/len;
	//int yadd=(dy<<8)/len;

	// Precalc height
	//int hs=position.y*mmult_world_map.y;
	//int he=epos.y*mmult_world_map.y;
	//int hp=(hs<<8);
	//int hadd=((he-hs)<<8)/len;

	// Test pixels
	//int xp=(ispos.x<<8);
	//int yp=(ispos.y<<8);
	//for (int i=0;i<len;i++)
	//if (hp<16777215)
	//{
	//	// Test hit
	//	if (hmap[((yp>>8)<<hmapsh)+(xp>>8)]>(hp>>8))
	//	{
	//		rti.hit=true;
	//		rti.model=NULL;
	//		rti.object=NULL;
	//		rti.range=((float)i/(float)len)*ray_length;
	//		//rti.range=0.5f*ray_length;
	//		rti.position=position+direction_normalized*rti.range;
	//		return;
	//	}

	//	// Add
	//	xp+=xadd;
	//	yp+=yadd;
	//	hp+=hadd;
	}
}


WORD *Storm3D_Terrain::GetHeightmap() const {
	return hmap;
}

void Storm3D_Terrain::SaveColorMap(const char *fname) const
{
	std::ofstream stream(fname, std::ios::binary);

	// psd
	for(int y = 0; y < hmap_size.x; ++y)
	for(int x = 0; x < hmap_size.y; ++x)
	{
		DWORD color = tmap[y * hmap_size.x + x].color;
		unsigned char r = color >> 16;
		unsigned char g = color >> 8;
		unsigned char b = color & 0x000000FF;
		
		stream << r;
		stream << g;
		stream << b;
	}
}

void Storm3D_Terrain::SaveTerrain(const char *fname) const
{
	// ToDo
	std::ofstream stream(fname, std::ios::binary);
}

void Storm3D_Terrain::LoadTerrain(const char *fname)
{
	// ToDo
	std::ifstream stream(fname, std::ios::binary);
}

void Storm3D_Terrain::SetAmbientColor(const COL &new_ambient)
{
	ambient_color = new_ambient;
}

//------------------------------------------------------------------
// Object copy handling
//------------------------------------------------------------------
ICreate<IStorm3D_Terrain_ObjectCopyHandle*> *Storm3D_Terrain::GetObjectListFromBlockAtPosition(const VC2 &position,BYTE group_id)
{
	// Get block at position
	VC2I pint=ConvertWorldToMap(position);
	pint/=blocksize;
	return blocks[(pint.y*bmapsize.x)+pint.x]->GetIterator();
	// TODO: Group ID must be used!!!!!
}


void Storm3D_Terrain::RemoveObjectCopy(Iterator<IStorm3D_Terrain_ObjectCopyHandle*> *objectcopy,const VC2 &position)
{
	// Get block at position (for object mesh degeneration)
	VC2I pint=ConvertWorldToMap(position);
	pint/=blocksize;
	blocks[(pint.y*bmapsize.x)+pint.x]->DeGenerateObjectMeshes();

	// Remove object (could be optimized more, but then PrtList must be modified)
	IteratorIM_TRBMOList<IStorm3D_Terrain_ObjectCopyHandle*> *itr=(IteratorIM_TRBMOList<IStorm3D_Terrain_ObjectCopyHandle*>*)objectcopy;
	//(*(itr->itm))->objects.Remove(*itr->it);

	// For debug
	PtrList<TRBlock_MatHandle_Obj> &obj = (*(itr->itm))->objects;
	PtrListIterator<TRBlock_MatHandle_Obj> &obj_it = itr->it;
	obj.Remove(obj_it);
}


//------------------------------------------------------------------
// Memory reserves / cache
//------------------------------------------------------------------

void Storm3D_Terrain::SetMemoryReserve(int memoryReserveAmount)
{
	this->memoryReserve = memoryReserveAmount;
}

void Storm3D_Terrain::SetPreCache(int precacheAmount)
{
	this->precacheAmount = precacheAmount;
}


//------------------------------------------------------------------
// Rendering
//------------------------------------------------------------------
void Storm3D_Terrain::Render(Storm3D_Scene *scene)
{
	// Update shadows
	static DWORD scount1=0;
	//UpdateShadows(scount1++);

	// Set renderstates
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_LIGHTING,FALSE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_CURRENT);
	Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_TEXTURE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	//Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHAREF,(DWORD)0x00000001);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHAREF,(DWORD)0x00000070);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	
	// psd
	//Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);

	// Set sunlight 
	//D3DLIGHT8 lgt;
	//ZeroMemory(&lgt,sizeof(D3DLIGHT8));
	//lgt.Diffuse.r=sun_col.r;
	//lgt.Diffuse.g=sun_col.g;
	//lgt.Diffuse.b=sun_col.b;
	//lgt.Specular.r=sun_col.r;
	//lgt.Specular.g=sun_col.g;
	//lgt.Specular.b=sun_col.b;
	//lgt.Ambient.r=0;
	//lgt.Ambient.g=0;
	//lgt.Ambient.b=0;
	//lgt.Type=D3DLIGHT_DIRECTIONAL;
	//lgt.Direction=D3DXVECTOR3(sun_dir.x,sun_dir.y,sun_dir.z);

	// Set light (to 3d-card)
	//Storm3D2->GetD3DDevice()->SetLight(0,&lgt);

	// World matrix
	float mt[16]=
	{
		1,	0,	0,	0,
		0,	1,	0,	0,
		0,	0,	1,	0,
		0,	0,	0,	1
	};

	// Calculate realworld block size
	float mx=size.x/(float)bmapsize.x;
	float my=size.z/(float)bmapsize.y;
	int hx=bmapsize.x/2;
	int hy=bmapsize.y/2;

	// Calculate camera area
	IStorm3D_Camera *cam=scene->GetCamera();
	VC3 campos=cam->GetPosition();
	float camran=cam->GetVisibilityRange();
	if (camran<0.01f) camran=0.01f;
	lastcampos=campos;
	lastcamrange=camran;
	int xmin=(int)((campos.x-camran)/mx-1.0f)+hx;
	int xmax=(int)((campos.x+camran)/mx)+hx;
	int ymin=(int)((campos.z-camran)/my-1.0f)+hy;
	int ymax=(int)((campos.z+camran)/my)+hy;
	if (xmin<0) xmin=0; else if (xmin>=bmapsize.x) xmin=bmapsize.x-1;
	if (xmax<0) xmax=0; else if (xmax>=bmapsize.x) xmax=bmapsize.x-1;
	if (ymin<0) ymin=0; else if (ymin>=bmapsize.y) ymin=bmapsize.y-1;
	if (ymax<0) ymax=0; else if (ymax>=bmapsize.y) ymax=bmapsize.y-1;

	// Precalculate (optimization)
	float hmx=mx/2;
	float hmy=my/2;
	float brad_xz=hmx*hmx+hmy*hmy;

	visBlocks = 0;
	newGenBlocks = 0;

	// Render all blocks in sight
	for (int y=ymin;y<=ymax;y++)
	for (int x=xmin;x<=xmax;x++)
	{
		// Calculate upper-corner real-world coordinates
		float upx=((float)(x-hx))*mx;
		float upy=((float)(y-hy))*my;

		// Test if block is visible (inside camera cone)
		float hsy=size.y*0.5f;

		float brad=sqrtf(hsy*hsy+brad_xz);
		// the above one treats the terrain block as a box...
		// this would tread it as a flat quad instead...
		// although, might bug with high altitudes or height changes.
		//float brad = sqrtf(brad_xz);

		VC3 bcenter(upx+hmx,hsy,upy+hmy);
		//VC2 bc2(bcenter.x,bcenter.z);
		//VC2 cp2(campos.x,campos.z);
		if (cam->TestSphereVisibility(bcenter,brad))	// Render if visible!
		{
			visBlocks++;
			// Set world matrix
			mt[12]=upx;
			mt[14]=upy;
			Storm3D2->GetD3DDevice()->SetTransform(D3DTS_WORLD,(D3DMATRIX*)&mt);

			// Calculate LOD level depending on range from camera
			float bran=bcenter.GetRangeTo(campos);
			//float bran2=bc2.GetRangeTo(cp2);

			// a bit less LOD, thank you :) -jpk
			int lod=(((float)LOD_LEVELS+1.0f)*(bran-(camran/5)))/camran;
			if (lod < 0) lod = 0;
			// original.
			//int lod=(((float)LOD_LEVELS+1.0f)*bran)/camran;

			if (lod>=LOD_LEVELS) lod=LOD_LEVELS-1;

			// Render the block
			blocks[(y*bmapsize.x)+x]->Render(scene,(D3DMATRIX*)mt,lod,bran);
		}
	}

	// Precache
	if (precacheAmount > 0)
	{
		int prexmin=xmin-1 - precacheAmount;
		int preymin=ymin-1 - precacheAmount;
		int prexmax=xmax+1 + precacheAmount;
		int preymax=ymax+1 + precacheAmount;
		if (prexmin<0) prexmin=0; else if (prexmin>=bmapsize.x) prexmin=bmapsize.x-1;
		if (prexmax<0) prexmax=0; else if (prexmax>=bmapsize.x) prexmax=bmapsize.x-1;
		if (preymin<0) preymin=0; else if (preymin>=bmapsize.y) preymin=bmapsize.y-1;
		if (preymax<0) preymax=0; else if (preymax>=bmapsize.y) preymax=bmapsize.y-1;
		// TODO: won't give a correct total block amount.
		// TODO: optimize maybe?
		// now it goes thru all the rendered blocks too...
		bool doneOnePre = false;
		for (int prey=preymin;prey<=preymax;prey++)
		for (int prex=prexmin;prex<=prexmax;prex++)
		{
			if (!blocks[(prey*bmapsize.x)+prex]->isGenerated())
			{
				if (!doneOnePre)
				{
					newGenBlocks++;
					for (int prelod = 0; prelod < LOD_LEVELS; prelod++)
					{
						blocks[(prey*bmapsize.x)+prex]->Generate(prelod,0);
					}
					doneOnePre = true;
				}
			}
		}
	}

	// Degenerate borders
	int mxmin=xmin-1 - memoryReserve;
	int mymin=ymin-1 - memoryReserve;
	int mxmax=xmax+1 + memoryReserve;
	int mymax=ymax+1 + memoryReserve;

	// UP / BOTTOM
	if (mxmin<0) mxmin=0; else if (mxmin>=bmapsize.x) mxmin=bmapsize.x-1;
	if (mxmax<0) mxmax=0; else if (mxmax>=bmapsize.x) mxmax=bmapsize.x-1;

	if (mymin>=0)
	{
		for (int x=mxmin;x<=mxmax;x++)
		{
			blocks[(mymin*bmapsize.x)+x]->DeGenerateAll();
		}
	}

	if (mymax<bmapsize.y)
	{
		for (int x=mxmin;x<=mxmax;x++)
		{
			blocks[(mymax*bmapsize.x)+x]->DeGenerateAll();
		}
	}

	// LEFT / RIGHT
	mxmin=xmin-1 - memoryReserve;
	mxmax=xmax+1 + memoryReserve;
	if (mymin<0) mymin=0; else if (mymin>=bmapsize.y) mymin=bmapsize.y-1;
	if (mymax<0) mymax=0; else if (mymax>=bmapsize.y) mymax=bmapsize.y-1;

	if (mxmin>=0)
	{
		for (int y=mymin;y<=mymax;y++)
		{
			blocks[(y*bmapsize.x)+mxmin]->DeGenerateAll();
		}
	}

	if (mxmax<bmapsize.x)
	{
		for (int y=mymin;y<=mymax;y++)
		{
			blocks[(y*bmapsize.x)+mxmax]->DeGenerateAll();		
		}
	}

	// Every once a while count generated blocks
	recountCounter++;
	if (recountCounter >= 100)
	{
		recountCounter = 0;
		totalGenBlocks = 0;
		for (int crey=0;crey<bmapsize.y;crey++)
		for (int crex=0;crex<bmapsize.x;crex++)
		{
			if (blocks[(crey*bmapsize.x)+crex]->isGenerated())
			{
				totalGenBlocks++;
			}
		}
	}
}


//------------------------------------------------------------------
// Conversion
//------------------------------------------------------------------
VC2I Storm3D_Terrain::ConvertWorldToMap(const VC2 &position) const
{
	return VC2I((position.x*mmult_world_map.x)+(hmap_size.x/2),(position.y*mmult_world_map.z)+(hmap_size.y/2));
}


VC2 Storm3D_Terrain::ConvertWorldToMapFloat(const VC2 &position) const
{
	return VC2((position.x*mmult_world_map.x)+(float)(hmap_size.x/2),(position.y*mmult_world_map.z)+(float)(hmap_size.y/2));
}


VC2 Storm3D_Terrain::ConvertMapToWorld(const VC2I &position) const
{
	return VC2((float)(position.x-(hmap_size.x/2))*mmult_map_world.x,(float)(position.y-(hmap_size.y/2))*mmult_map_world.z);
}


VC2I Storm3D_Terrain::ConvertWorldToObstacleMap(const VC2 &position) const
{
	return VC2I((position.x*mmult_world_map.x*OBSTACLE_MAP_MULTIPLIER)+
    (hmap_size.x/(2/OBSTACLE_MAP_MULTIPLIER)),(position.y*mmult_world_map.z*OBSTACLE_MAP_MULTIPLIER)+(hmap_size.y/(2/OBSTACLE_MAP_MULTIPLIER)));
}


VC2 Storm3D_Terrain::ConvertObstacleMapToWorld(const VC2I &position) const
{
	return VC2((float)(position.x-(hmap_size.x/(2/OBSTACLE_MAP_MULTIPLIER)))*mmult_map_world.x/OBSTACLE_MAP_MULTIPLIER,
    (float)(position.y-(hmap_size.y/(2/OBSTACLE_MAP_MULTIPLIER)))*mmult_map_world.z/OBSTACLE_MAP_MULTIPLIER);
}


//------------------------------------------------------------------
//------------------------------------------------------------------
// TRTexBank
//------------------------------------------------------------------
//------------------------------------------------------------------


//------------------------------------------------------------------
// Change texture
//------------------------------------------------------------------
void TRTexBank::ChangeTexture(Storm3D_Texture *_texture,bool _tile_mirror)
{
	// Add texture reference count
	if (_texture) _texture->AddRef();

	// Delete old texture (actually decreases ref.count)
	if (texture) texture->Release();;

	// Set new texture
	texture=_texture;

	// Set tiling
	tile_mirror=_tile_mirror;

	// Copy texture data to lookup
	Storm3D_SurfaceInfo ssi=texture->GetSurfaceInfo();
	DWORD *sbuf=new DWORD[ssi.height*ssi.width];
	texture->CopyTextureTo32BitSysMembuffer(sbuf);

	//for (int i=0;i<ssi.height*ssi.width;i++)
	//{
	//	texcol_lookup[i].r=((sbuf[i]&0x00FF0000)>>16);
	//	texcol_lookup[i].g=((sbuf[i]&0x0000FF00)>>8);
	//	texcol_lookup[i].b=sbuf[i]&0x000000FF;
	//}

	// psd
	for(int x = 0; x < ssi.width; ++x)
	{
		int sbuf_x = terrain->blocksize * x / ssi.width;

		for(int y = 0; y < ssi.height; ++y)
		{
			int texcol_position = y * ssi.width + x;
			int sbuf_y = terrain->blocksize * y / ssi.height;
			int sbuf_position = sbuf_y * terrain->blocksize + sbuf_x;
			
			texcol_lookup[sbuf_position].r=((sbuf[texcol_position]&0x00FF0000)>>16);
			texcol_lookup[sbuf_position].g=((sbuf[texcol_position]&0x0000FF00)>>8);
			texcol_lookup[sbuf_position].b=sbuf[texcol_position]&0x000000FF;
		}
	}

	delete[] sbuf;
}


//------------------------------------------------------------------
// Texture color look-up stuff
//------------------------------------------------------------------
TColor<BYTE> &TRTexBank::GetColorAt(int x,int y)
{
	x%=terrain->blocksize;
	y%=terrain->blocksize;
	return texcol_lookup[y*terrain->blocksize+x];
}


//------------------------------------------------------------------
// Construct & Destruct
//------------------------------------------------------------------
TRTexBank::TRTexBank(Storm3D_Terrain *_terrain) : terrain(_terrain), texture(NULL),
	texcol_lookup(NULL),tile_mirror(false)
{
	texcol_lookup=new TColor<BYTE>[terrain->blocksize*terrain->blocksize];
}


TRTexBank::~TRTexBank()
{
	// Delete texture (actually decreases ref.count)
	if (texture) texture->Release();;

	// Delete lookup
	if (texcol_lookup) delete[] texcol_lookup;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
// TRBlock
//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
// Generation
//------------------------------------------------------------------
void TRBlock::Generate(int detail_level,float range)
{
	// Temp variables
	int x,y;
	int blocksize=terrain->blocksize;
	int hsh=terrain->hmapsh;
	int xlen=blocksize+1;
	int ylen=blocksize+1;
	int xsz=(blocksize>>detail_level)+1;
	int ysz=(blocksize>>detail_level)+1;
	//float yfix=detail_level*200.0f*terrain->mmult_map_world.y;
	float yfix=detail_level*1.0f*terrain->mmult_map_world.y;
	int dlm=1<<detail_level;

	// Generate height mesh (visible terrain)
	if (!dx8_vbufs[detail_level])
	{
		// Create new vertexbuffer
		terrain->CreateNewVertexbuffer(&dx8_vbufs[detail_level],detail_level);

		// Generate vertexes...

		// Lock the buffer
		VXFORMAT_TERRAIN *vert;
		
		// BUG: Can specify D3DLOCK_DISCARD or D3DLOCK_NOOVERWRITE for only Vertex Buffers created with D3DUSAGE_DYNAMIC
		// FixMe: stalls pipeline
			// dx8_vbufs[detail_level]->Lock(0,0,(BYTE**)&vert,D3DLOCK_DISCARD);
		dx8_vbufs[detail_level]->Lock(0,0,(BYTE**)&vert,0);
		
		
		if (vert==NULL) return;

		// Generate vertex grid
		int yp=0;
		vertex_amounts[detail_level]=0;
		DWORD yfix=detail_level*100;

		// Do first row
		for (x=0;x<xlen;x+=dlm)
		{
			// Coordinates
			int yps=terrain->hmap[(blockpos.y<<hsh)+blockpos.x+x];
			
			// Take the lowest coordinate (on sampling area)
			for (int i=1-dlm;i<dlm;i++)
			{
				int ypn=terrain->hmap[(blockpos.y<<hsh)+blockpos.x+x+i];
				if (ypn<yps) yps=ypn;
			}
			yps-=yfix;
			if (yps<0) yps=0;
			VC3 pos=terrain->precgrid[x].terrain_pos;
			pos.y=((float)(yps))*terrain->mmult_map_world.y; // NOTE: Tän kertolaskun saa pois, kun pistää 3d-kortin skaalaamaan

			// Add the vertex in to the array
			vert[vertex_amounts[detail_level]++]=VXFORMAT_TERRAIN(pos,terrain->tmap[(blockpos.y<<hsh)+blockpos.x+x].color,terrain->precgrid[x].terrain_dtc,terrain->precgrid[x].terrain_dtc2);
		}
		yp+=xlen*dlm;

		// Do [1,n-1] rows
		for (y=dlm;y<ylen-dlm;y+=dlm)
		{
			// First column
			int yps=terrain->hmap[((blockpos.y+y)<<hsh)+blockpos.x];
			for (int i=1-dlm;i<dlm;i++)
			{
				int ypn=terrain->hmap[((blockpos.y+y+i)<<hsh)+blockpos.x];
				if (ypn<yps) yps=ypn;
			}
			yps-=yfix;
			if (yps<0) yps=0;
			VC3 pos=terrain->precgrid[yp].terrain_pos;
			pos.y=((float)(yps))*terrain->mmult_map_world.y;

			// Add the vertex in to the array
			vert[vertex_amounts[detail_level]++]=VXFORMAT_TERRAIN(pos,terrain->tmap[((blockpos.y+y)<<hsh)+blockpos.x].color,terrain->precgrid[yp].terrain_dtc,terrain->precgrid[yp].terrain_dtc2);

			// Middle columns
			for (x=dlm;x<xlen-dlm;x+=dlm)
			{
				// Coordinates
				pos=terrain->precgrid[yp+x].terrain_pos;
				pos.y=((float)terrain->hmap[((blockpos.y+y)<<hsh)+blockpos.x+x])*terrain->mmult_map_world.y;

				// Add the vertex in to the array
				vert[vertex_amounts[detail_level]++]=VXFORMAT_TERRAIN(pos,terrain->tmap[((blockpos.y+y)<<hsh)+blockpos.x+x].color,terrain->precgrid[yp+x].terrain_dtc,terrain->precgrid[yp+x].terrain_dtc2);
			}

			// Last column
			yps=terrain->hmap[((blockpos.y+y)<<hsh)+blockpos.x+x];
			for (i=1-dlm;i<dlm;i++)
			{
				int ypn=terrain->hmap[((blockpos.y+y+i)<<hsh)+blockpos.x+x];
				if (ypn<yps) yps=ypn;
			}
			yps-=yfix;
			if (yps<0) yps=0;
			pos=terrain->precgrid[yp+x].terrain_pos;
			pos.y=((float)(yps))*terrain->mmult_map_world.y;

			// Add the vertex in to the array
			vert[vertex_amounts[detail_level]++]=VXFORMAT_TERRAIN(pos,terrain->tmap[((blockpos.y+y)<<hsh)+blockpos.x+x].color,terrain->precgrid[yp+x].terrain_dtc,terrain->precgrid[yp+x].terrain_dtc2);

			// Next row
			yp+=xlen*dlm;
		}

		// Do last row
		for (x=0;x<xlen;x+=dlm)
		{
			// Coordinates
			int yps=terrain->hmap[((blockpos.y+y)<<hsh)+blockpos.x+x];
			
			// Take the lowest coordinate (on sampling area)
			for (int i=1-dlm;i<dlm;i++)
			{
				int ypn=terrain->hmap[((blockpos.y+y)<<hsh)+blockpos.x+x+i];
				if (ypn<yps) yps=ypn;
			}
			yps-=yfix;
			if (yps<0) yps=0;
			VC3 pos=terrain->precgrid[yp+x].terrain_pos;
			pos.y=((float)(yps))*terrain->mmult_map_world.y;

			// Add the vertex in to the array
			vert[vertex_amounts[detail_level]++]=VXFORMAT_TERRAIN(pos,terrain->tmap[((blockpos.y+y)<<hsh)+blockpos.x+x].color,terrain->precgrid[yp+x].terrain_dtc,terrain->precgrid[yp+x].terrain_dtc2);
		}

		// Unlock buffer
		dx8_vbufs[detail_level]->Unlock();
	}

	// Generate object meshes (group mesh)
	// NOTE: always some object may need to be rebuilt (different visibility ranges!)
	//if (!generated_buf_objects)
	{
		// Generate object groups
		for (PtrListIterator<TRBlock_MatHandle> mhi=mathandles.Begin();(*mhi)!=NULL;mhi++)
		{
			TRBlock_MatHandle *mh=*mhi;
			mh->Generate(range);
		}

		// It's now ready to use
		generated_buf_objects=true;
	}
}


void TRBlock::DeGenerateHeightMesh()
{
	// Release buffers
	for (int l=0;l<LOD_LEVELS;l++)
	{
		if (dx8_vbufs[l])
		{
			terrain->DeleteVertexbuffer(dx8_vbufs[l],l);
			dx8_vbufs[l]=NULL;
		}
	}
}


void TRBlock::DeGenerateObjectMeshes()
{
	// DeGenerate object groups
	for (PtrListIterator<TRBlock_MatHandle> mhi=mathandles.Begin();(*mhi)!=NULL;mhi++)
	{
		TRBlock_MatHandle *mh=*mhi;
		mh->DeGenerate();
	}

	// Is not generated anymore
	generated_buf_objects=false;
}


void TRBlock::DeGenerateAll()
{
	// Degenerate all stuff
	DeGenerateHeightMesh();
	DeGenerateObjectMeshes();
}


//------------------------------------------------------------------
// Model copy add
//------------------------------------------------------------------
void TRBlock::AddModelCopy(Storm3D_Model *model,const VC3 &position,const QUAT &rotation,const VC3 &orig_position, BYTE group_id)
{
	// pds: get model id
	int model_id = models.size();
	models.resize(model_id + 1);

	// store model position
	model_positions.push_back(orig_position);

	for (set<IStorm3D_Model_Object*>::iterator oit=model->objects.begin();oit!=model->objects.end();++oit)
	{
		IStorm3D_Model_Object *obj=*oit;
		IStorm3D_Mesh *mesh=obj->GetMesh();
		Storm3D_Material *mat=(Storm3D_Material*)mesh->GetMaterial();

		TRBlock_MatHandle_Obj *object_material_handle = 0;
		TRBlock_MatHandle *nmat = 0;

		// Search if material already exists
		bool mat_found=false;
		for (PtrListIterator<TRBlock_MatHandle> mit=mathandles.Begin();(*mit)!=NULL;mit++)
		{
			nmat=*mit;
			if (nmat->group_id==group_id)
			//if (nmat->material==mat)
			if(nmat->material->IsIdenticalWith(mat)) // psd
			{
				// Add object to that material handle
				object_material_handle = nmat->AddObject((Storm3D_Model_Object*)obj,position+obj->GetPosition(), rotation,orig_position);
				mat_found=true;
				break;
			}
		}

		// If material didn't exist
		if (!mat_found)
		{
			// Create new material handle
			nmat=new TRBlock_MatHandle(terrain,this,mat,group_id);
			mathandles.Add(nmat);

			// Add object to it
			object_material_handle = nmat->AddObject((Storm3D_Model_Object*)obj,position+obj->GetPosition(), rotation,orig_position);
		}

		// psd: store some settings to handle
		object_material_handle->model = model;
		object_material_handle->model_id = model_id;
		object_material_handle->material = nmat;

		// store to model id
		models[model_id].push_back(object_material_handle);
	}
}

VC3 TRBlock::RemoveModelCopy(int model_id)
{
	assert(model_id < models.size());

	std::vector<TRBlock_MatHandle_Obj *> &handles = models[model_id];
	if(handles.empty())
	{
		assert(!"Model already removed!");
		return VC3();
	}

	DeGenerateObjectMeshes();

	for(int i = 0; i < handles.size(); ++i)
	{
		TRBlock_MatHandle_Obj *obj_handle = handles[i];

		PtrList<TRBlock_MatHandle_Obj> &obj = obj_handle->material->objects;
		obj.Remove(obj_handle);

		//assert(obj_handle->material->objects.IsEmpty() == false);
		if(obj_handle->material->objects.IsEmpty() == true)
		{
			mathandles.Remove(obj_handle->material);
		}
	}

	handles.clear();
	return model_positions[model_id];
}

//------------------------------------------------------------------
// Rendering
//------------------------------------------------------------------
void TRBlock::Render(Storm3D_Scene *scene,D3DMATRIX *mxx,int detail_level,float range)
{
		// Generate if not generated
		Generate(detail_level,range);

		// Render ground...

		// DO NOT USE BASETEXTURE ANYMORE!
		// Only detail texturing
		if (terrain->detail_texture) terrain->detail_texture->Apply(0);
		if (terrain->detail_texture2) terrain->detail_texture2->Apply(1);

		// Set buffers
		terrain->Storm3D2->GetD3DDevice()->SetVertexShader(FVF_VXFORMAT_TERRAIN);
		terrain->Storm3D2->GetD3DDevice()->SetStreamSource(0,dx8_vbufs[detail_level],sizeof(VXFORMAT_TERRAIN));
		terrain->Storm3D2->GetD3DDevice()->SetIndices(terrain->dx8_ibufs[detail_level],0);

		// Modulate 2x blending op for detail textures
		terrain->Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE2X);
		if (terrain->detail_texture2) terrain->Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_MODULATE2X);

		// Render it!
		terrain->Storm3D2->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
			0,vertex_amounts[detail_level],0,terrain->face_amounts[detail_level]);
		scene->AddPolyCounter(terrain->face_amounts[detail_level]);

		// Return blending ops
		terrain->Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
		terrain->Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);

		// Mesh buffer optimization stuff (v2.6)
		terrain->Storm3D2->SetActiveMesh(NULL);

		// Calculate block position
		//VC2 bpo=terrain->ConvertMapToWorld(blockpos);

		// Render objects
		for (PtrListIterator<TRBlock_MatHandle> mit=mathandles.Begin();(*mit)!=NULL;mit++)
		{
			TRBlock_MatHandle *mh=*mit;

			// (Lighting on if not shader)
			// NO lighting in any way anymore (precalculated)
			if (mh->material->HasShader())
			{
				//terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_LIGHTING,FALSE);
				terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FOGVERTEXMODE,D3DFOG_LINEAR);
				terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FOGTABLEMODE,D3DFOG_NONE);
			}
			else
			{
				//terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_LIGHTING,TRUE);
				terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FOGVERTEXMODE,D3DFOG_NONE);
				terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FOGTABLEMODE,D3DFOG_LINEAR);
			}

			// Apply material
			bool isalpha=false;
			mh->material->ApplyBaseTextureExtOnly_NoAlphaSort(scene,FVF_VXFORMAT_TOBJ,mxx);

			// Render objects with this material
			//for (PtrListIterator<TRBlock_MatHandle_Obj> oit=mh->objects.Begin();(*oit)!=NULL;oit++)
			//{
			//	TRBlock_MatHandle_Obj *oh=*oit;
			//	Storm3D_Model_Object *obj=oh->object;
			//	obj->SetPosition(oh->position+VC3(bpo.x,0,bpo.y));
			//	((Storm3D_Mesh*)(obj->GetMesh()))->RenderWithoutMaterial(scene,false,obj);
			//}

			// Render the grouped mesh
			mh->RenderGroupMesh(scene,range);

			// Return states back
			terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
			terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
		}

		// Return states back
		//terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_LIGHTING,FALSE);
		//terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FOGENABLE,scene->IsFogEnabled());
		terrain->Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_FOGTABLEMODE,D3DFOG_LINEAR);
		terrain->Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_LINEAR);
}


//------------------------------------------------------------------
// Construct & Destruct
//------------------------------------------------------------------
TRBlock::TRBlock(Storm3D_Terrain *_terrain,const VC2I &_blockpos) : 
	blockpos(_blockpos),
	terrain(_terrain),
	generated_buf_objects(false)
{
	// Create iterator
	iterator=new ICreateIM_TRBMOList<IStorm3D_Terrain_ObjectCopyHandle*>(&(mathandles));

	for (int l=0;l<LOD_LEVELS;l++)
	{
		dx8_vbufs[l]=NULL;
		vertex_amounts[l]=0;
	}
}


TRBlock::~TRBlock()
{
	// Delete iterator
	delete iterator;

	DeGenerateAll();
	mathandles.DeleteObjects();
}



//------------------------------------------------------------------
//------------------------------------------------------------------
// TRBlock_MatHandle
//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
// Generation
//------------------------------------------------------------------
void TRBlock_MatHandle::Generate(float range)
{
	// Do not generate again
	if (generated) return;

	// Is group visible?
	if (terrain->obj_group_vis_range[group_id]<range) return;

	// Create group mesh...

	// Calculate vertex and face amounts
	int vx_count=0,fc_count=0;
	int biggest_vx_amt=0;
	int biggest_fc_amt=0;
	PtrListIterator<TRBlock_MatHandle_Obj> oit = objects.Begin();
	for (;(*oit)!=NULL;oit++)
	{
		TRBlock_MatHandle_Obj *oh=*oit;
		Storm3D_Model_Object *obj=oh->object;
		Storm3D_Mesh *msh=(Storm3D_Mesh*)obj->GetMesh();
		int fcc=msh->GetFaceCount();
		int vxxc=msh->GetVertexCount();
		fc_count+=fcc;
		vx_count+=vxxc;
		if (fcc>biggest_fc_amt) biggest_fc_amt=fcc;
		if (vxxc>biggest_vx_amt) biggest_vx_amt=vxxc;
	}

	assert(vx_count <= fc_count * 3);

	// Test
	if (fc_count<1) return;
	if (vx_count<1) return;
	if (biggest_vx_amt<1) return;
	if (biggest_fc_amt<1) return;
	if (!material) return;

	// Test if stuff fits
	//if ((fc_count>=(65536/3))||(vx_count>=65536)) 
	//{
		// DOES NOT FIT!!! (=problems)
		// However used should never have this much faces in one block
		// because FPS would be very low.
	//}
	//else

	// Delete old buffers
	assert(dx8_ibufs.size() == dx8_vbufs.size());
	for (int dpart = 0; dpart < dx8_ibufs.size(); dpart++)
	{
		SAFE_RELEASE(*(dx8_ibufs[dpart]));
		SAFE_RELEASE(*(dx8_vbufs[dpart]));
	}
	dx8_ibufs.clear();
	dx8_vbufs.clear();
	dx8_ibuf_amounts.clear();
	dx8_vbuf_amounts.clear();

	vertex_amount=vx_count;
	face_amount=fc_count;

	bool alldone = false;

	// which buffer are we filling
	int part = 0;
	// and how much of the data have we copied to previous buffers
	int total_f_amount = 0;
	int total_v_amount = 0;

	VXFORMAT_TOBJ *tempvbuf=new VXFORMAT_TOBJ[biggest_vx_amt];
	WORD *tempfbuf=new WORD[biggest_fc_amt*3];

  oit=objects.Begin();
	int break_looped_f = 0;
	int break_looped_v = 0;
	int break_vx_count = 0;
	int break_fc_count = 0;

	while (!alldone)
	{
		LPDIRECT3DINDEXBUFFER8 *newibuf = new LPDIRECT3DINDEXBUFFER8();
		LPDIRECT3DVERTEXBUFFER8 *newvbuf = new LPDIRECT3DVERTEXBUFFER8();
		int newibuf_amount = 0;
		int newvbuf_amount = 0;
		dx8_ibufs.push_back(newibuf);
		dx8_vbufs.push_back(newvbuf);
		dx8_ibuf_amounts.push_back(newibuf_amount);
		dx8_vbuf_amounts.push_back(newvbuf_amount);

		int vbufsize = vertex_amount;
		int ibufsize = face_amount;
		if (vbufsize > TRBLOCK_IBUF_MAX_SIZE*3)
			vbufsize = TRBLOCK_IBUF_MAX_SIZE*3;
		if (ibufsize > TRBLOCK_IBUF_MAX_SIZE)
			ibufsize = TRBLOCK_IBUF_MAX_SIZE;
		if (total_f_amount + TRBLOCK_IBUF_MAX_SIZE > face_amount)
		{
			ibufsize = face_amount - total_f_amount;
		}
		if (total_v_amount + TRBLOCK_IBUF_MAX_SIZE*3 > vertex_amount)
		{
			vbufsize = vertex_amount - total_v_amount;
		}

		// Create new indexbuffer
		terrain->Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*ibufsize*3,
			D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,dx8_ibufs[part]);
		assert(dx8_ibufs[part] != NULL);
		assert(*(dx8_ibufs[part]) != NULL);

		// Create new vertexbuffer
		DWORD fvf=FVF_VXFORMAT_TOBJ;
		if ((material)&&(material->HasShader())) fvf=0;	// Shader's FVF is always 0
		terrain->Storm3D2->GetD3DDevice()->CreateVertexBuffer(vbufsize*sizeof(VXFORMAT_TOBJ),
			D3DUSAGE_WRITEONLY,fvf,D3DPOOL_MANAGED,dx8_vbufs[part]);
		assert(dx8_vbufs[part] != NULL);
		assert(*(dx8_vbufs[part]) != NULL);

		// Copy object list's vertexes to group_mesh
		WORD *ip;
		VXFORMAT_TOBJ *vp;
		
		// Bug: Can't use discard here
			//dx8_vbuf->Lock(0,0,(BYTE**)&vp,D3DLOCK_DISCARD);
			//dx8_ibuf->Lock(0,sizeof(WORD)*fc_count*3,(BYTE**)&ip,D3DLOCK_DISCARD);
		(*(dx8_vbufs[part]))->Lock(0,0,(BYTE**)&vp,0);
		(*(dx8_ibufs[part]))->Lock(0,sizeof(WORD)*ibufsize*3,(BYTE**)&ip,0);

		int added_f_amount = 0;
		int added_v_amount = 0;
		int looped_f = break_looped_f;
		int looped_v = break_looped_v;
		
		vx_count=break_vx_count;
		fc_count=break_fc_count;

		int vam=0,fam=0;
		//Storm3D_Mesh *ex_mesh=NULL;
		//for (oit=objects.Begin();(*oit)!=NULL;oit++)
		for (;(*oit)!=NULL;oit++)
		{	
			// Get buffers etc
			TRBlock_MatHandle_Obj *oh=*oit;
			Storm3D_Model_Object *obj=oh->object;
			Storm3D_Mesh *msh=(Storm3D_Mesh*)obj->GetMesh();

			fam=msh->GetFaceCount();
			vam=msh->GetVertexCount();

			bool copy_faces = false;
			if (looped_f >= total_f_amount 
				&& (looped_f - total_f_amount) + fam < TRBLOCK_IBUF_MAX_SIZE)
			{
				copy_faces = true;
			} else {
				if ((looped_f - total_f_amount) + fam >= TRBLOCK_IBUF_MAX_SIZE)
				{
					// no need to loop the rest of the objects
					// continue from here during next part...
					break_looped_f = looped_f;
					break_looped_v = looped_v;
					break_vx_count = vx_count;
					break_fc_count = fc_count;
					break;
				}
			}

			if (copy_faces)
			{			
				// Copy stuff to buffer
				const Storm3D_Face *m_faces=msh->GetFaceBufferReadOnly();
				const Storm3D_Vertex *m_vxs=msh->GetVertexBufferReadOnly();
				
				// Faces
				int p=0;
				for(int i=0;i<fam;i++)
				{
					tempfbuf[p]=m_faces[i].vertex_index[0]+(vx_count-total_v_amount);
					tempfbuf[p+1]=m_faces[i].vertex_index[1]+(vx_count-total_v_amount);
					tempfbuf[p+2]=m_faces[i].vertex_index[2]+(vx_count-total_v_amount);
					//tempfbuf[p]=m_faces[i].vertex_index[0]+vx_count;
					//tempfbuf[p+1]=m_faces[i].vertex_index[1]+vx_count;
					//tempfbuf[p+2]=m_faces[i].vertex_index[2]+vx_count;
					p+=3;
				}

				// Precalculate
				TColor<float> fcol=(material->GetColor()*terrain->sun_col)*255.0f;
				//TColor<int> colmul=TColor<int>(fcol.r,fcol.g,fcol.b);
				
				//TColor<float> fsi=material->GetSelfIllumination()*terrain->sun_col*65535.0f;
				// psd: change self illumination to ambient
				//TColor<float> fsi=terrain->ambient_color*terrain->sun_col*65535.0f;
				// psd: both self illum and ambient ;-)
				
				//TColor<float> fsi=(terrain->ambient_color+material->GetSelfIllumination())*terrain->sun_col*65535.0f;
				//TColor<float> fsi=(terrain->ambient_color*terrain->sun_col+material->GetSelfIllumination()*terrain->sun_col)*65535.0f;
				TColor<float> fsi=(terrain->ambient_color+material->GetSelfIllumination())*65535.0f * terrain->sun_col;

				TColor<int> coladd=TColor<int>(fsi.r,fsi.g,fsi.b);
				//int coladd=(fsi.r+fsi.g+fsi.b)/3;
				VC3 sundir=terrain->sun_dir;

				QUAT &model_rotation = oh->rotation;
				MAT model_transform;
				model_transform.CreateRotationMatrix(model_rotation);

				MAT object_transform = model_transform * obj->GetMXG();
				MAT object_transform_inverse;

				object_transform_inverse.CreateRotationMatrix(model_rotation.GetInverse());
				object_transform_inverse.RotateVector(sundir);

				// Vertices
				for(i=0;i<vam;i++)
				{
					// Calculate color
					int intens=-(int)(m_vxs[i].normal.GetDotWith(sundir)*255.0f);
					if (intens<0) intens=0;
					//TColor<int> col=colmul*intens+coladd;

					TColor<float> light_color=terrain->sun_col*intens;//coladd;
					TColor<int> col(light_color.r * 255.f, light_color.g * 255.f, light_color.b * 255.f);

					col += coladd;

					if (col.r>65535) col.r=65535;
					if (col.g>65535) col.g=65535;
					if (col.b>65535) col.b=65535;
					col.r>>=8;
					col.g>>=8;
					col.b>>=8;

					VC3 pos = m_vxs[i].position;
					object_transform.TransformVector(pos);

					pos += oh->position;

					// Set vertex
					tempvbuf[i]=VXFORMAT_TOBJ(pos,0xFF000000+(col.r<<16)+(col.g<<8)+col.b,
						m_vxs[i].texturecoordinates);

				}
			}

			// Copy faces
			if (copy_faces)
			{
				memcpy(ip,tempfbuf,sizeof(WORD)*fam*3);
				ip+=fam*3;
				added_f_amount += fam;
			}
			looped_f += fam;

			// Copy vertices
			// (note, copy_faces on purpose, no such thing as copy_vertices)
			// (as we want to copy such vertices that have faces in the buffer)
			if (copy_faces)
			{
				VC3 pos=oh->position;
				float mul_y=65535.0f/terrain->size.y;
				float mul_x=(float)terrain->hmap_size.x/terrain->size.x;
				float mul_z=(float)terrain->hmap_size.y/terrain->size.z;
				int hsh=terrain->hmapsh;

				// Calculate object center shadow height (optimization)
				int ix=block->blockpos.x+(pos.x*mul_x);
				int iy=block->blockpos.y+(pos.z*mul_z);
				WORD wsypos=terrain->tmap[(iy<<hsh)+ix].shadow;
				if (wsypos<=terrain->hmap[(iy<<hsh)+ix]) wsypos=0;	// Prevent shadowing if vertex in underground
				float sypos=(float)wsypos/mul_y;

				for(int i=0;i<vam;i++)
				{
					VC3 vwp(tempvbuf[i].position);
					DWORD colo=tempvbuf[i].color;

					//int ix=block->blockpos.x+(vwp.x*mul_x);
					//int iy=block->blockpos.y+(vwp.z*mul_z);
					//WORD sypos=terrain->tmap[(iy<<hsh)+ix].shadow;
					//if (sypos>terrain->hmap[(iy<<hsh)+ix])	// Prevent shadowing if vertex in underground
					

					if (vwp.y<sypos)
					{
						//colo>>=1;							// divide by 2
						//colo=0xFF000000+(colo&0x007F7F7F);	// mask that removes highest bits of r,g,b (but not alpha)

						int r = GetRValue(colo);
						int g = GetGValue(colo);
						int b = GetBValue(colo);

						// 12/16 (3/4) of color when shadowed 
						//r = 12 * r >> 4;
						//g = 12 * g >> 4;
						//b = 12 * b >> 4;
						// a more proper shadowing based on shadow settings:
						int shadow_amount = 4;
						if (shadow_amount >= terrain->shadow_minvalue)
						{
							int seff = 20 - terrain->shadow_darkness - shadow_amount;
							r = (((r * seff) >> 4) * terrain->shadow_col_b + r * (255-terrain->shadow_col_b)) >> 8;
							g = (((g * seff) >> 4) * terrain->shadow_col_g + g * (255-terrain->shadow_col_g)) >> 8;
							b = (((b * seff) >> 4) * terrain->shadow_col_r + b * (255-terrain->shadow_col_r)) >> 8;
							// NOTE: for some reason the red/blue are inverted
							// when compared to terrain heightmesh vertices...
							// thus, shadow_r and shadow_b inverted.
							// (reason likely that mixed RGBA and BGRA)
						}

						colo = 0xFF000000 + RGB(r,g,b);
					}

					*vp++=VXFORMAT_TOBJ(vwp,colo,tempvbuf[i].texcoords);
				}
				added_v_amount += vam;
			}
			looped_v += vam;
			
			// Add buffer pointers
			vx_count+=vam;

			// Set ex mesh
			//ex_mesh=msh;
		}
		(*(dx8_vbufs[part]))->Unlock();
		(*(dx8_ibufs[part]))->Unlock();

		dx8_ibuf_amounts[part] = added_f_amount;
		dx8_vbuf_amounts[part] = added_v_amount;
		total_f_amount += added_f_amount;
		total_v_amount += added_v_amount;

		part++;

		if (added_f_amount == 0 || total_f_amount >= face_amount)
			alldone = true;
	}

	delete[] tempvbuf;
	delete[] tempfbuf;

	assert(total_f_amount == face_amount);
	assert(total_v_amount == vertex_amount);

	generated=true;
}


void TRBlock_MatHandle::DeGenerate()
{
	// Delete mesh
	assert(dx8_ibufs.size() == dx8_vbufs.size());
	assert(dx8_ibuf_amounts.size() == dx8_vbuf_amounts.size());
	assert(dx8_ibuf_amounts.size() == dx8_ibufs.size());
	for (int dpart = 0; dpart < dx8_ibufs.size(); dpart++)
	{
		SAFE_RELEASE(*(dx8_ibufs[dpart]));
		SAFE_RELEASE(*(dx8_vbufs[dpart]));
	}
	dx8_ibufs.clear();
	dx8_vbufs.clear();
	dx8_ibuf_amounts.clear();
	dx8_vbuf_amounts.clear();

	generated=false;
}


//------------------------------------------------------------------
// Object add
//------------------------------------------------------------------
TRBlock_MatHandle_Obj *TRBlock_MatHandle::AddObject(Storm3D_Model_Object *object,const VC3 &position, const QUAT &rotation,const VC3 &orig_position)
{
	TRBlock_MatHandle_Obj *foo = new TRBlock_MatHandle_Obj(object,position, rotation,orig_position);
	objects.Add(foo);

	DeGenerate();
	return foo;
}	
	

//------------------------------------------------------------------
// Render group mesh
//------------------------------------------------------------------
void TRBlock_MatHandle::RenderGroupMesh(Storm3D_Scene *scene,float range)
{
	// Test
	//if (dx8_vbufs==NULL) return;
	//if (dx8_ibufs==NULL) return;

	if (dx8_vbufs.size() == 0) return;
	if (dx8_ibufs.size() == 0) return;

	// Is group visible?
	if (terrain->obj_group_vis_range[group_id]<range) return;

	// psd
	// added support for split buffers
	// -jpk
	terrain->Storm3D2->GetD3DDevice()->SetVertexShader(FVF_VXFORMAT_TOBJ);
	for (int i = 0; i < dx8_vbufs.size(); i++)
	{
		terrain->Storm3D2->GetD3DDevice()->SetStreamSource(0,*(dx8_vbufs[i]),sizeof(VXFORMAT_TOBJ));
		terrain->Storm3D2->GetD3DDevice()->SetIndices(*(dx8_ibufs[i]),0);

		int faces = dx8_ibuf_amounts[i];
		int vertices = dx8_vbuf_amounts[i];
			
		// Render it!
		terrain->Storm3D2->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0,vertices,0,faces);
		scene->AddPolyCounter(faces);
	}
}


//------------------------------------------------------------------
// Construct & Destruct
//------------------------------------------------------------------
TRBlock_MatHandle::TRBlock_MatHandle(Storm3D_Terrain *_terrain,TRBlock *_block,Storm3D_Material *mat,BYTE _group_id) :
	terrain(_terrain),block(_block),material(mat),generated(false),
	group_id(_group_id), face_amount(0), vertex_amount(0)
	// dx8_ibufs(NULL),dx8_vbufs(NULL),
{
}


TRBlock_MatHandle::~TRBlock_MatHandle()
{
	DeGenerate();
	objects.DeleteObjects();
}


//------------------------------------------------------------------
//------------------------------------------------------------------
// TRBlock_MatHandle_Obj
//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
// Get object
//------------------------------------------------------------------
IStorm3D_Model_Object *TRBlock_MatHandle_Obj::GetObject() const
{
	return object;
}


//------------------------------------------------------------------
// Get position
//------------------------------------------------------------------
VC3 TRBlock_MatHandle_Obj::GetPosition() const
{
	//return position;
	return original_position;
}

QUAT TRBlock_MatHandle_Obj::GetRotation() const
{
	return rotation;
}

int TRBlock_MatHandle_Obj::GetModelId() const
{
	return model_id;;
}

IStorm3D_Model *TRBlock_MatHandle_Obj::GetModel() const
{
	return model;
}

//------------------------------------------------------------------
// Construct & Destruct
//------------------------------------------------------------------
TRBlock_MatHandle_Obj::TRBlock_MatHandle_Obj(Storm3D_Model_Object *_object,const VC3 &_position, const QUAT &rotation_,const VC3 &orig_position) :
	object(_object), position(_position), rotation(rotation_),original_position(orig_position)
{
}


TRBlock_MatHandle_Obj::~TRBlock_MatHandle_Obj()
{
}


*/
