// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <string>
#include <vector>
#include <fstream>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "storm3d_terrain_heightmap.h"
#include "storm3d_terrain_lod.h"
#include "storm3d_terrain_utils.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_spotlight.h"
#include <c2_oobb.h>
#include <c2_frustum.h>

#include <cassert>
#include "storm3d.h"
#include "storm3d_scene.h"
#include "storm3d_texture.h"

#include "Storm3D_ObstacleMapDefs.h"
#include "../../util/AreaMap.h"
#include "../../util/Debug_MemoryManager.h"

#include <d3dx9core.h>
#include <d3dx9math.h>
#include <atlbase.h>

	static const int BLOCK_SIZE = IStorm3D_Terrain::BLOCK_SIZE;
	static const int VERTEX_COUNT = BLOCK_SIZE + 1;

	static const int STREAM_0_SIZE = 6 * sizeof(float);
	static const int STREAM_1_SIZE = 6 * sizeof(float);

	static const bool DYNAMIC_POSITION = false;

	struct TexturePass
	{
		boost::shared_ptr<Storm3D_Texture> weights;

		int textureA;
		int textureB;

		int subMask;

		TexturePass(boost::shared_ptr<Storm3D_Texture> weights_, int textureA_, int textureB_, int subMask_ = -1)
		:	weights(weights_),
			textureA(textureA_),
			textureB(textureB_),

			subMask(subMask_)
		{
		}
	};

	// Sort passes for better coherency
	bool operator < (const TexturePass &a, const TexturePass &b)
	{
		if(a.textureA < b.textureA)
			return true;
		if(a.textureB < b.textureB)
			return true;
		if(a.subMask < b.subMask)
			return true;

		return false;
	}

	void getPosition(VC3 &result, int xIndex, int yIndex, const unsigned short *buffer, const VC3 &size, const VC2I &resolution, const VC3 &scale)
	{
		int index = yIndex * resolution.x + xIndex;
		result.x = xIndex * scale.x - size.x/2;
		result.y = buffer[index] * scale.y;
		result.z = yIndex * scale.z - size.z/2;
	}

	void getFaceNormalImp(VC3 &result, const VC3 &v1, const VC3 &v2, const VC3 &v3)
	{
		Vector a = v2 - v1;
		Vector b = v3 - v1;
		result = a.GetCrossWith(b);
	}

	struct TerrainBlock
	{
		frozenbyte::storm::VertexBuffer heightBuffer;
		int positionX;
		int positionY;

		std::vector<TexturePass> passes;
		float height;
		float heightRadius;
		float heightMin;
		float heightMax;

		boost::shared_ptr<Storm3D_TerrainLod> indexBuffer;
		boost::shared_ptr<Storm3D_Texture> lightMap;

		TerrainBlock()
		:	positionX(0),
			positionY(0),
			height(0),
			heightRadius(0),
			heightMin(0),
			heightMax(0)
		{
		}

		void createBuffer(IDirect3DDevice9 &device)
		{
			heightBuffer.create(device, VERTEX_COUNT * VERTEX_COUNT, STREAM_0_SIZE, DYNAMIC_POSITION);
		}

		void fillBuffer(const VC2I &start, const VC2I &end, boost::scoped_array<unsigned short> &buffer, int blockX, int blockY, const VC2I &resolution, const VC3 &scale, const VC3 &size)
		{
			float *pointer = static_cast<float *> (heightBuffer.lock());
			if(!pointer)
				return;

			float minHeight =  9999999.f;
			float maxHeight = -9999999.f;

			VC3 position;

			VC3 faceNormal1;
			VC3 faceNormal2;
			VC3 faceNormal3;
			VC3 faceNormal4;
			VC3 faceNormal5;
			VC3 faceNormal6;
			VC3 faceNormal7;
			VC3 faceNormal8;
			VC3 positionU;
			VC3 positionD;
			VC3 positionL;
			VC3 positionR;
			VC3 positionUL;
			VC3 positionUR;
			VC3 positionDL;
			VC3 positionDR;

			for(int j = start.y; j < end.y; ++j)
			for(int i = start.x; i < end.x; ++i)
			{
				int yPosition = j + blockY * BLOCK_SIZE;
				int xPosition = i + blockX * BLOCK_SIZE;
				// int index = yPosition * resolution.x + xPosition;
				
				getPosition(position, xPosition, yPosition, buffer.get(), size, resolution, scale);
				if(position.y < minHeight)
					minHeight = position.y;
				if(position.y > maxHeight)
					maxHeight = position.y;

				*pointer++ = position.x;
				*pointer++ = position.y;
				*pointer++ = position.z;

				// Should really optimize this stuff ..

				if(xPosition > 0 && xPosition < resolution.x - 1 && yPosition > 0 && yPosition < resolution.y - 1)
				{
					getPosition(positionU, xPosition, yPosition - 1, buffer.get(), size, resolution, scale);
					getPosition(positionD, xPosition, yPosition + 1, buffer.get(), size, resolution, scale);
					getPosition(positionL, xPosition - 1, yPosition, buffer.get(), size, resolution, scale);
					getPosition(positionR, xPosition + 1, yPosition, buffer.get(), size, resolution, scale);
					getPosition(positionUL, xPosition - 1, yPosition - 1, buffer.get(), size, resolution, scale);
					getPosition(positionUR, xPosition + 1, yPosition - 1, buffer.get(), size, resolution, scale);
					getPosition(positionDL, xPosition - 1, yPosition + 1, buffer.get(), size, resolution, scale);
					getPosition(positionDR, xPosition + 1, yPosition + 1, buffer.get(), size, resolution, scale);

					getFaceNormalImp(faceNormal1, positionUL, position, positionU);
					getFaceNormalImp(faceNormal2, positionL, position, positionUL);
					getFaceNormalImp(faceNormal3, positionU, position, positionUR);
					getFaceNormalImp(faceNormal4, positionR, position, positionUR);
					getFaceNormalImp(faceNormal5, positionDL, position, positionL);
					getFaceNormalImp(faceNormal6, positionD, position, positionDL);
					getFaceNormalImp(faceNormal7, positionDR, position, positionD);
					getFaceNormalImp(faceNormal8, positionR, position, positionDR);

					faceNormal1 += faceNormal2;
					faceNormal1 += faceNormal3;
					faceNormal1 += faceNormal4;
					faceNormal1 += faceNormal5;
					faceNormal1 += faceNormal6;
					faceNormal1 += faceNormal7;
					faceNormal1 += faceNormal8;
					if(faceNormal1.GetSquareLength() > 0.0001f)
						faceNormal1.Normalize();

					*pointer++ = faceNormal1.x;
					*pointer++ = faceNormal1.y;
					*pointer++ = faceNormal1.z;
				}
				else
				{
					*pointer++ = 0.f;
					*pointer++ = 1.f;
					*pointer++ = 0.f;
				}
			}

			heightBuffer.unlock();
			height = (minHeight + maxHeight) * .5f;
			heightRadius = (maxHeight - minHeight);

			heightMin = minHeight;
			heightMax = maxHeight;
		}
	};

	struct TerrainTexture
	{
		boost::shared_ptr<Storm3D_Texture> texture;

		explicit TerrainTexture(boost::shared_ptr<Storm3D_Texture> texture_)
		:	texture(texture_)
		{
		}
	};

	struct RenderBlock
	{
		float range;
		int indexX;
		int indexY;

		OOBB oobb;

		RenderBlock(float range_, int indexX_, int indexY_, const OOBB &oobb_)
		:	range(range_),
			indexX(indexX_),
			indexY(indexY_),
			oobb(oobb_)
		{
		}
	};

	bool operator < (const RenderBlock &a, const RenderBlock &b)
	{
		return a.range < b.range;
	}

struct Storm3D_TerrainHeightmapData
{
	Storm3D &storm;
	IDirect3DDevice9 &device;

	boost::scoped_array<unsigned short> heightMap;
	boost::scoped_array<unsigned short> collisionHeightMap;
	VC2I resolution;
	VC3 size;
	VC2I collResolution;

	VC2I blockAmount;
	VC2 positionDelta;
	float textureDetail;

	frozenbyte::storm::PixelShader pixelShader;
	frozenbyte::storm::PixelShader lightPixelShader;

	frozenbyte::storm::VertexShader atiDefaultShader;
	frozenbyte::storm::VertexShader atiLightingShader;
	frozenbyte::storm::VertexShader atiShadowShaderDirectional;
	frozenbyte::storm::VertexShader atiShadowShaderPoint;
	frozenbyte::storm::VertexShader atiShadowShaderFlat;
	frozenbyte::storm::VertexShader atiShadowTerrainShaderDirectional;
	frozenbyte::storm::VertexShader atiShadowTerrainShaderPoint;
	frozenbyte::storm::VertexShader atiShadowTerrainShaderFlat;
	frozenbyte::storm::VertexShader atiDepthShader;
	frozenbyte::storm::VertexShader nvDefaultShader;
	frozenbyte::storm::VertexShader nvLightingShader;
	frozenbyte::storm::VertexShader nvShadowShaderDirectional;
	frozenbyte::storm::VertexShader nvShadowShaderPoint;
	frozenbyte::storm::VertexShader nvShadowShaderFlat;
	
	frozenbyte::storm::VertexBuffer vertexBuffer;

	std::vector<TerrainBlock> blocks;
	boost::shared_ptr<Storm3D_TerrainLod> indexBuffer;

	std::vector<TerrainTexture> textures;
	boost::shared_ptr<Storm3D_Texture> lightmap;

	const unsigned short *obstacleHeightmap;
	const util::AreaMap *areaMap;
	unsigned short *forcemap;
	std::vector<RenderBlock> visibleBlocks;

	int obstaclemapMultiplier;
	int obstaclemapShiftMult;
	int heightmapMultiplier;
	int heightmapShiftMult;

	float radius;
	bool ps13;

	Storm3D_TerrainHeightmapData(Storm3D &storm_, bool ps13_)
	:	storm(storm_),
		device(*storm.GetD3DDevice()),
		textureDetail(0),

		pixelShader(device),
		lightPixelShader(device),
		atiDefaultShader(device),
		atiLightingShader(device),
		atiShadowShaderDirectional(device),
		atiShadowShaderPoint(device),
		atiShadowShaderFlat(device),
		atiShadowTerrainShaderDirectional(device),
		atiShadowTerrainShaderPoint(device),
		atiShadowTerrainShaderFlat(device),
		atiDepthShader(device),
		nvDefaultShader(device),
		nvLightingShader(device),
		nvShadowShaderDirectional(device),
		nvShadowShaderPoint(device),
		nvShadowShaderFlat(device),

		indexBuffer(new Storm3D_TerrainLod(storm)),
		obstacleHeightmap(0),
		areaMap(0),
		obstaclemapMultiplier(1),
		obstaclemapShiftMult(0),
		heightmapMultiplier(1),
		heightmapShiftMult(0),
		radius(0),
		ps13(ps13_)
	{
		lightPixelShader.createTerrainLightShader();

		if(ps13)
		{
			pixelShader.createTerrainShader();
			
			atiDefaultShader.createAtiTerrainShader();
			atiLightingShader.createAtiLightingShader();
			atiShadowShaderDirectional.createAtiTerrainShadowShaderDirectional();
			atiShadowShaderPoint.createAtiTerrainShadowShaderPoint();
			atiShadowShaderFlat.createAtiTerrainShadowShaderFlat();
			atiShadowTerrainShaderDirectional.createAtiTerrainShadowShaderDirectional();
			atiShadowTerrainShaderPoint.createAtiTerrainShadowShaderPoint();
			atiShadowTerrainShaderFlat.createAtiTerrainShadowShaderFlat();
			atiDepthShader.createAtiDepthTerrainShader();

			nvDefaultShader.createNvTerrainShader();
			nvLightingShader.createNvLightingShader();
			nvShadowShaderDirectional.createNvTerrainShadowShaderDirectional();
			nvShadowShaderPoint.createNvTerrainShadowShaderPoint();
			nvShadowShaderFlat.createNvTerrainShadowShaderFlat();
		}
	}

	void createVertexBuffer(float textureDetail)
	{
		vertexBuffer.create(device, VERTEX_COUNT * VERTEX_COUNT, STREAM_1_SIZE, false);
		float *pointer = static_cast<float *> (vertexBuffer.lock());

		if(!pointer)
			return;

		for(int j = 0; j < VERTEX_COUNT; ++j)
		for(int i = 0; i < VERTEX_COUNT; ++i)
		{
			float u = float(i) / (VERTEX_COUNT - 1);
			float v = float(j) / (VERTEX_COUNT - 1);

			*pointer++ = u;// - 1.f/128.f;
			*pointer++ = v;// - 1.f/128.f;

			*pointer++ = u * textureDetail;
			*pointer++ = v * textureDetail;

			*pointer++ = u * textureDetail;
			*pointer++ = v * textureDetail;
		}

		vertexBuffer.unlock();
	}

	void createHeightBuffers()
	{
		std::vector<TerrainBlock> tempBlocks(blockAmount.x * blockAmount.y);
		VC3 scale(size.x / (resolution.x - 1), size.y / 65535.f, size.z / (resolution.y - 1));
		//VC3 scale(size.x / (resolution.x), size.y / 65535.f, size.z / (resolution.y));

		VC2I start(0, 0);
		VC2I end(VERTEX_COUNT, VERTEX_COUNT);

		for(int j = 0; j < blockAmount.y; ++j)
		for(int i = 0; i < blockAmount.x; ++i)
		{
			tempBlocks[j * blockAmount.x + i].createBuffer(*storm.GetD3DDevice());
			tempBlocks[j * blockAmount.x + i].fillBuffer(start, end, heightMap, i, j, resolution, scale, size);
		}

		blocks.swap(tempBlocks);
	}

	void updateHeightBuffers(const VC2I &start, const VC2I &end)
	{
		VC3 scale(size.x / (resolution.x - 1), size.y / 65535.f, size.z / (resolution.y - 1));

		VC2I startBlock = start / (BLOCK_SIZE);
		VC2I endBlock = end / (BLOCK_SIZE);

		for(int j = startBlock.y; j < endBlock.y; ++j)
		for(int i = startBlock.x; i < endBlock.x; ++i)
		{
			TerrainBlock &block = blocks[j * blockAmount.x + i];

			block.createBuffer(*storm.GetD3DDevice());
			block.fillBuffer(VC2I(), VC2I(VERTEX_COUNT, VERTEX_COUNT), heightMap, i, j, resolution, scale, size);
		}
	}

	void createBuffers()
	{
		blockAmount.x = resolution.x / BLOCK_SIZE;
		blockAmount.y = resolution.y / BLOCK_SIZE;

		positionDelta.x = size.x / blockAmount.x;
		positionDelta.y = size.z / blockAmount.y;
		
		float radiusX = positionDelta.x * 1.5f / 2.f;
		float radiusY = positionDelta.y * 1.5f / 2.f;
		if(radiusX > radiusY)
			radius = radiusX;
		else
			radius = radiusY;

		indexBuffer.reset(new Storm3D_TerrainLod(storm));
		indexBuffer->generate(VERTEX_COUNT);
		indexBuffer->setBlockRadius(max(positionDelta.x, positionDelta.y));

		createVertexBuffer(textureDetail);
		createHeightBuffers();

		for(int j = 0; j < blockAmount.y; ++j)
		for(int i = 0; i < blockAmount.x; ++i)
		{
			TerrainBlock &block = blocks[j * blockAmount.x + i];
			block.indexBuffer = indexBuffer;
		}
	}

	void setClipMap(const unsigned char *buffer)
	{
		if(!heightMap)
			return;

		unsigned char clipArray[VERTEX_COUNT * VERTEX_COUNT] = { 0 };

		for(int j = 0; j < blockAmount.y; ++j)
		for(int i = 0; i < blockAmount.x; ++i)
		{
			TerrainBlock &block = blocks[j * blockAmount.x + i];

			bool hasClippedVertex = false;
			bool hasVisibleVertex = false;

			VC2I start = VC2I(i, j) * (VERTEX_COUNT - 1);
			VC2I end = start + VC2I(VERTEX_COUNT, VERTEX_COUNT);

			for(int y = start.y; y < end.y; ++y)
			for(int x = start.x; x < end.x; ++x)
			{
				int xx = x - (i * (VERTEX_COUNT - 1));
				int yy = y - (j * (VERTEX_COUNT - 1));

				assert(yy * VERTEX_COUNT + xx < VERTEX_COUNT * VERTEX_COUNT);
				clipArray[yy * VERTEX_COUNT + xx] = 0;

				if(i == blockAmount.x - 1 && x == end.x - 1)
					continue;
				if(j == blockAmount.y - 1 && y == end.y - 1)
					continue;

				int index = (y * (resolution.x - 1)) + x;
				int value = buffer[index];

				if(value)
				{
					hasClippedVertex = true;
					clipArray[yy * VERTEX_COUNT + xx] = 1;
				}
				else
					hasVisibleVertex = true;
			}

			if(hasClippedVertex && !hasVisibleVertex)
				block.indexBuffer.reset();
			else if(hasClippedVertex && hasVisibleVertex)
			{
				// ToDo: Create clipped buffer!!!
				//block.indexBuffer.reset();
				block.indexBuffer.reset(new Storm3D_TerrainLod(storm));
				block.indexBuffer->generate(VERTEX_COUNT, clipArray);
			}
		}
	}

	float getRange(Storm3D_Scene &scene, int blockX, int blockY)
	{
		if(blockX < 0 || blockX >= blockAmount.x)
			return 10000.f;
		if(blockY < 0 || blockY >= blockAmount.y)
			return 10000.f;

		VC3 position(-size.x/2 + positionDelta.x/2, 0, -size.z/2 + positionDelta.y/2);
		position.x += positionDelta.x * blockX;
		position.z += positionDelta.y * blockY;

		float range = scene.GetCamera()->GetPosition().GetRangeTo(position);

		if(!scene.GetCamera()->TestSphereVisibility(position, radius) && radius > 200)
			return 10000.f;
		
		return range;
	}

	void collectVisibleBlocks(Storm3D_Scene &scene, std::vector<RenderBlock> &visibleBlocks)
	{
		// ToDo: Do rough mapping to avoid looping all(?)

		OOBB oobb;
		oobb.axes[0].x = 1.f;
		oobb.axes[0].y = 0.f;
		oobb.axes[0].z = 0.f;
		oobb.axes[1].x = 0.f;
		oobb.axes[1].y = 1.f;
		oobb.axes[1].z = 0.f;
		oobb.axes[2].x = 0.f;
		oobb.axes[2].y = 0.f;
		oobb.axes[2].z = 1.f;
		oobb.extents.x = positionDelta.x * 0.5f;
		oobb.extents.y = 0.f;
		oobb.extents.z = positionDelta.y * 0.5f;

		Frustum frustum = static_cast<Storm3D_Camera *> (scene.GetCamera())->getFrustum();
		Sphere sphere;
			
		for(int j = 0; j < blockAmount.y; ++j)
		for(int i = 0; i < blockAmount.x; ++i)
		{
			VC3 position(-size.x/2 + positionDelta.x/2, 0, -size.z/2 + positionDelta.y/2);
			position.x += positionDelta.x * i;
			position.z += positionDelta.y * j;

			const TerrainBlock &block = blocks[j * blockAmount.x + i];
			position.y = block.height;

			float sphereRadius = radius;
			if(block.heightRadius > sphereRadius)
				sphereRadius = block.heightRadius;
			//if(!scene.GetCamera()->TestSphereVisibility(position, sphereRadius))
			//	continue;
			sphere.position = position;
			sphere.radius = sphereRadius;
			if(!frustum.visibility(sphere, true))
				continue;

			{
				oobb.center = position;
				oobb.extents.y = block.heightRadius * 0.5f;
				if(oobb.extents.y < 0.5f)
					oobb.extents.y = 0.5f;

				if(!frustum.visibility(oobb))
					continue;
			}

			float range = getRange(scene, i, j);
			visibleBlocks.push_back(RenderBlock(range, i, j, oobb));
		}

		/*
		for(int j = 0; j < blockAmount.y; ++j)
		for(int i = 0; i < blockAmount.x; ++i)
		{
			VC3 position(-size.x/2 + positionDelta.x/2, 0, -size.z/2 + positionDelta.y/2);
			position.x += positionDelta.x * i;
			position.z += positionDelta.y * j;

			const TerrainBlock &block = blocks[j * blockAmount.x + i];
			position.y = blocks[j * blockAmount.x + i].height;

			float sphereRadius = radius;
			if(block.heightRadius > sphereRadius)
				sphereRadius = block.heightRadius;

			if(!scene.GetCamera()->TestSphereVisibility(position, sphereRadius))
				continue;

			float range = getRange(scene, i, j);
			visibleBlocks.push_back(RenderBlock(range, i, j));
		}
		*/

		//std::sort(visibleBlocks.begin(), visibleBlocks.end());
	}

	VC3 getNormal(const VC2I &position) const
	{
		int xPosition = position.x;
		int yPosition = position.y;
		if(xPosition <= 0 || xPosition >= resolution.x - 1 || yPosition <= 0 || yPosition >= resolution.y - 1)
			return VC3(0, 1.f, 0);

		VC3 faceNormal1;
		VC3 faceNormal2;
		VC3 faceNormal3;
		VC3 faceNormal4;
		VC3 faceNormal5;
		VC3 faceNormal6;
		VC3 faceNormal7;
		VC3 faceNormal8;
		VC3 positionU;
		VC3 positionD;
		VC3 positionL;
		VC3 positionR;
		VC3 positionUL;
		VC3 positionUR;
		VC3 positionDL;
		VC3 positionDR;

		VC3 scale(size.x / (resolution.x - 1), size.y / 65535.f, size.z / (resolution.y - 1));
		getPosition(positionU, xPosition, yPosition - 1, heightMap.get(), size, resolution, scale);
		getPosition(positionD, xPosition, yPosition + 1, heightMap.get(), size, resolution, scale);
		getPosition(positionL, xPosition - 1, yPosition, heightMap.get(), size, resolution, scale);
		getPosition(positionR, xPosition + 1, yPosition, heightMap.get(), size, resolution, scale);
		getPosition(positionUL, xPosition - 1, yPosition - 1, heightMap.get(), size, resolution, scale);
		getPosition(positionUR, xPosition + 1, yPosition - 1, heightMap.get(), size, resolution, scale);
		getPosition(positionDL, xPosition - 1, yPosition + 1, heightMap.get(), size, resolution, scale);
		getPosition(positionDR, xPosition + 1, yPosition + 1, heightMap.get(), size, resolution, scale);

		VC3 pos;
		getPosition(pos, xPosition, yPosition, heightMap.get(), size, resolution, scale);
		getFaceNormalImp(faceNormal1, positionUL, pos, positionU);
		getFaceNormalImp(faceNormal2, positionL, pos, positionUL);
		getFaceNormalImp(faceNormal3, positionU, pos, positionUR);
		getFaceNormalImp(faceNormal4, positionR, pos, positionUR);
		getFaceNormalImp(faceNormal5, positionDL, pos, positionL);
		getFaceNormalImp(faceNormal6, positionD, pos, positionDL);
		getFaceNormalImp(faceNormal7, positionDR, pos, positionD);
		getFaceNormalImp(faceNormal8, positionR, pos, positionDR);

		faceNormal1 += faceNormal2;
		faceNormal1 += faceNormal3;
		faceNormal1 += faceNormal4;
		faceNormal1 += faceNormal5;
		faceNormal1 += faceNormal6;
		faceNormal1 += faceNormal7;
		faceNormal1 += faceNormal8;
		if(faceNormal1.GetSquareLength() > 0.0001f)
			faceNormal1.Normalize();

		return faceNormal1;
	}

	VC2I convertWorldToMap(const VC2 &position) const
	{
		int x = int((position.x + size.x / 2) / size.x * (resolution.x-1));
		int y = int((position.y + size.z / 2) / size.z * (resolution.y-1));

		return VC2I(x, y);
	}

	VC2I convertWorldToCollisionMap(const VC2 &position) const
	{
		//int x = int((position.x + size.x / 2) / size.x * (collResolution.x-1));
		//int y = int((position.y + size.z / 2) / size.z * (collResolution.y-1));
		int x = int((position.x + size.x / 2) / size.x * (collResolution.x));
		int y = int((position.y + size.z / 2) / size.z * (collResolution.y));

		return VC2I(x, y);
	}

	VC2I convertWorldToObstacleMap(const VC2 &position) const
	{
		VC3 mmult_world_map=VC3((float)(resolution.x-1),65535.0f,(float)(resolution.y-1))/size;
		
		float obstacle_map_multiplier = (float)this->obstaclemapMultiplier;

		return VC2I(
			(int)((position.x*mmult_world_map.x*obstacle_map_multiplier)+((float)(resolution.x-1)/(2.0f/obstacle_map_multiplier))),
			(int)((position.y*mmult_world_map.z*obstacle_map_multiplier)+((float)(resolution.y-1)/(2.0f/obstacle_map_multiplier))));
	}


	VC2 convertObstacleMapToWorld(const VC2I &position) const
	{
		VC3 mmult_map_world=size/VC3((float)(resolution.x-1),65535.0f,(float)(resolution.y-1));	

		float obstacle_map_multiplier = (float)this->obstaclemapMultiplier;

		return VC2((float)(float(position.x)+0.5f-((resolution.x-1)/(2.0f/obstacle_map_multiplier)))*mmult_map_world.x/obstacle_map_multiplier,
			(float)(float(position.y)+0.5f-((resolution.y-1)/(2.0f/obstacle_map_multiplier)))*mmult_map_world.z/obstacle_map_multiplier);
	}

};

Storm3D_TerrainHeightmap::Storm3D_TerrainHeightmap(Storm3D &storm, bool ps13)
{
	boost::scoped_ptr<Storm3D_TerrainHeightmapData> tempData(new Storm3D_TerrainHeightmapData(storm, ps13));
	data.swap(tempData);
}

Storm3D_TerrainHeightmap::~Storm3D_TerrainHeightmap()
{
}

void Storm3D_TerrainHeightmap::setHeightMap(const unsigned short *buffer, const VC2I &resolution, const VC3 &size, int textureDetail, unsigned short *forceMap, int heightmapMultiplier, int obstaclemapMultiplier)
{
	assert(resolution.x % BLOCK_SIZE == 0);
	assert(resolution.y % BLOCK_SIZE == 0);

	assert(obstaclemapMultiplier >= heightmapMultiplier);
	assert(obstaclemapMultiplier >= 1 && obstaclemapMultiplier < 256);
	assert(heightmapMultiplier >= 1 && heightmapMultiplier < 256);

	int shiftsum;

	data->heightmapMultiplier = heightmapMultiplier;
	data->heightmapShiftMult = -1;
	shiftsum = 1;
	for (int s = 0; s < 8; s++)
	{
		if (heightmapMultiplier == shiftsum)
		{
			data->heightmapShiftMult = s;
			break;
		}
		shiftsum = (shiftsum << 1);
	}
	assert(data->heightmapShiftMult != -1);

	data->obstaclemapMultiplier = obstaclemapMultiplier;
	data->obstaclemapShiftMult = -1;
	shiftsum = 1;
	for (int s = 0; s < 8; s++)
	{
		if (obstaclemapMultiplier == shiftsum)
		{
			data->obstaclemapShiftMult = s;
			break;
		}
		shiftsum = (shiftsum << 1);
	}
	assert(data->obstaclemapShiftMult != -1);

	int bufferSize = (resolution.x + 1) * (resolution.y + 1);
	boost::scoped_array<unsigned short> tempBuffer(new unsigned short [bufferSize]);

	int j = 0;
	int i = 0;

	for(j = 0 ; j < resolution.y; ++j)
	{
		for(i = 0 ; i < resolution.x; ++i)
			tempBuffer[j * (resolution.x + 1) + i] = buffer[j * resolution.x + i];
	}	

	// Clear safe borders
	for(i = 0; i <= resolution.x; ++i)
		tempBuffer[resolution.y * (resolution.x + 1) + i] = 0;
	for(j = 0; j <= resolution.y; ++j)
		tempBuffer[j * (resolution.x + 1) + resolution.x] = 0;

	data->collResolution = resolution * heightmapMultiplier;
	
	// Create correctly interpolated collision map
	{
		int bufferSize = data->collResolution.x * (data->collResolution.y + 1);
		boost::scoped_array<unsigned short> tempBuffer(new unsigned short [bufferSize]);

		const VC2I &sourceRes = resolution;
		const unsigned short *sourceBuffer = buffer;
		const VC2I &destRes = data->collResolution;
		unsigned short *destBuffer = tempBuffer.get();

		float factorDelta(1.f / heightmapMultiplier);
		for(int j = 0; j < sourceRes.y - 1; ++j)
		for(int i = 0; i < sourceRes.x - 1; ++i)
		{
			int sourceIndex = j * sourceRes.x + i;

			// We could try to get rid of all these float conversions
			// .. sometime >:)

			float ipY = 0.f;
			for(int y = 0; y < heightmapMultiplier; ++y)
			{
				float ipX = 0.f;
				for(int x = 0; x < heightmapMultiplier; ++x)
				{
					assert(ipX >= 0 && ipX < 1.f);
					assert(ipY >= 0 && ipY < 1.f);

					float sourcevals[4];
					sourcevals[0] = float(sourceBuffer[sourceIndex + 0]);
					sourcevals[1] = float(sourceBuffer[sourceIndex + 1]);
					sourcevals[2] = float(sourceBuffer[sourceIndex + sourceRes.x]);
					sourcevals[3] = float(sourceBuffer[sourceIndex + sourceRes.x + 1]);

					float lerped_x1 = sourcevals[0] + (sourcevals[1] - sourcevals[0]) * ipX;
					float lerped_x2 = sourcevals[2] + (sourcevals[3] - sourcevals[2]) * ipX;
					float value = lerped_x1 + (lerped_x2 - lerped_x1) * ipY;

					// this is a weird and buggy alternative
					/*
					float px = float(sourceBuffer[sourceIndex + 1]);
					float py = float(sourceBuffer[sourceIndex + sourceRes.x]);
					float value = 0;

					int comp = (x + y) - (heightmapMultiplier - 1);
					if(comp < 0) // upper part
					{
						float p0 = float(sourceBuffer[sourceIndex]);
						value = p0 + (px - p0) * ipX + (py - p0) * ipY;
					}
					//else if(comp == 0) // diagonal
					//{
					//	value = py + ((x - py) * (factorDelta * (COLLISION_HEIGHTMAP_MULT - 1 - x)));
					//}
					else // lower part
					{
						float pxy = float(sourceBuffer[sourceIndex + sourceRes.x + 1]);
						value = pxy + (py - pxy) * (1.0f - ipX) + (px - pxy) * (1.0f - ipY);
					}*/

					int destIndex = ((j * heightmapMultiplier) + y) * destRes.x + ((i * heightmapMultiplier) + x);
					destBuffer[destIndex] = (unsigned short)(value);

					ipX += factorDelta;
				}

				ipY += factorDelta;
			}
		}

		data->collisionHeightMap.swap(tempBuffer);
	}

/*
	int bufferSize2 = data->collResolution.x * (data->collResolution.y + 1);
	boost::scoped_array<unsigned short> tempBuffer2(new unsigned short [bufferSize2]);

	// HACK: iterated scale up (by always creating a doubled map..)
	int bufferSize3 = data->collResolution.x * (data->collResolution.y + 1);
	boost::scoped_array<unsigned short> tempBuffer3(new unsigned short [bufferSize3]);

	//for(j = 0 ; j < resolution.y; ++j)
	//{
	//	for(i = 0 ; i < resolution.x; ++i)
	//	{
	//		tempBuffer2[j * 2 * (resolution.x * 2) + i * 2] = buffer[j * resolution.x + i];
	//		tempBuffer2[j * 2 * (resolution.x * 2) + (i * 2 + 1)] = buffer[j * resolution.x + i];
	//		tempBuffer2[(j * 2 + 1) * (resolution.x * 2) + i * 2] = buffer[j * resolution.x + i];
	//		tempBuffer2[(j * 2 + 1) * (resolution.x * 2) + (i * 2 + 1)] = buffer[j * resolution.x + i];
	//	}
	//}

	for(j = 0 ; j < resolution.y; ++j)
	{
		for(i = 0 ; i < resolution.x; ++i)
			tempBuffer3[j * resolution.x + i] = buffer[j * resolution.x + i];
	}

	int atSource = 1;
	int atDoubled = 2;
	while (atDoubled <= COLLISION_HEIGHTMAP_MULT)
	{
		int sResX = resolution.x * atSource;
		int sResY = resolution.y * atSource;
		int dResX = resolution.x * atDoubled;
		int dResY = resolution.y * atDoubled;

		for(j = 0 ; j < dResY; j += 2)
		{
			for(i = 0 ; i < dResX; i += 2)
			{
				int val = tempBuffer3[(j / 2) * sResX + (i / 2)];
				tempBuffer2[j * dResX + i] = (unsigned short)val;
			}
		}
		for(j = 0 ; j < dResY; j += 2)
		{
			for(i = 1 ; i < dResX; i += 2)
			{
				int val = tempBuffer3[(j / 2) * sResX + (i / 2)];
				val += tempBuffer3[(j / 2) * sResX + ((i + 1) / 2)];
				val /= 2;
				tempBuffer2[j * dResX + i] = (unsigned short)val;
			}
		}
		for(j = 1 ; j < dResY-1; j += 2)
		{
			for(i = 0 ; i < dResX; i += 2)
			{
				int val = tempBuffer3[(j / 2) * sResX + (i / 2)];
				val += tempBuffer3[((j + 1) / 2) * sResX + (i / 2)];
				val /= 2;
				tempBuffer2[j * dResX + i] = (unsigned short)val;
			}
		}
		for(j = 1 ; j < dResY; j += 2)
		{
			for(i = 1 ; i < dResX-1; i += 2)
			{
				int val = tempBuffer3[(j / 2) * sResX + (i / 2)];
				val += tempBuffer3[((j + 1) / 2) * sResX + ((i+1) / 2)];
				val /= 2;
				tempBuffer2[j * dResX + i] = (unsigned short)val;
			}
		}
		for(i = 0; i < dResX; ++i)
		{
			tempBuffer2[dResY * dResX + i] = 0;
			tempBuffer2[(dResY - 1) * dResX + i] = 0;
		}
		for(j = 0; j < dResY; ++j)
		{
			tempBuffer2[j * dResX + dResX - 1] = 0;
		}

		for(j = 0 ; j < dResY; ++j)
		{
			for(i = 0 ; i < dResX; ++i)
				tempBuffer3[j * dResX + i] = tempBuffer2[j * dResX + i];
		}	

		atDoubled *= 2;
		atSource *= 2;
	}

	data->collisionHeightMap.swap(tempBuffer2);
*/

	data->resolution = resolution + VC2I(1, 1);
	data->size = size;
	data->heightMap.swap(tempBuffer);
	data->textureDetail = float(textureDetail);
	data->forcemap = forceMap;

	data->createBuffers();
}

void Storm3D_TerrainHeightmap::setClipMap(const unsigned char *buffer)
{
	if(!buffer)
	{
		assert(!"Null clip buffer");
		return;
	}

	data->setClipMap(buffer);
}

void Storm3D_TerrainHeightmap::updateHeightMap(const unsigned short *buffer, const VC2I &start, const VC2I &end)
{
	for(int y = start.y; y < end.y; ++y)
	for(int x = start.x; x < end.x; ++x)
	{
		assert(x >= 0 && x < data->resolution.x);
		assert(y >= 0 && y < data->resolution.y);

		data->heightMap[y * data->resolution.x + x] = buffer[y * (data->resolution.x - 1) + x];
	}

	// ToDo: update collision map too!

	data->updateHeightBuffers(start, end);
}

void Storm3D_TerrainHeightmap::setObstacleHeightmap(const unsigned short *obstacleHeightmap, const util::AreaMap *areaMap)
{
	data->obstacleHeightmap = obstacleHeightmap;
	data->areaMap = areaMap;
}

void Storm3D_TerrainHeightmap::recreateCollisionMap()
{
}

unsigned short *Storm3D_TerrainHeightmap::getCollisionHeightmap()
{
	return data->collisionHeightMap.get();
}

void Storm3D_TerrainHeightmap::forcemapHeight(const VC2 &position, float radius, bool above, bool below)
{
	if(!data->forcemap) 
		return;

	//VC2I pint=data->convertWorldToMap(position) * COLLISION_HEIGHTMAP_MULT;
	VC2I pint=data->convertWorldToCollisionMap(position);
	int bint=int(radius*(float)data->collResolution.x/data->size.x);

	// Precalc
	int hx=data->collResolution.x/2;
	int hy=data->collResolution.y/2;
	float mx=data->size.x/(float)data->collResolution.x;
	float my=data->size.z/(float)data->collResolution.y;
	float hmx=mx/2;
	float hmy=my/2;

	int xmin=(pint.x-bint-1);
	int xmax=(pint.x+bint+1);
	int ymin=(pint.y-bint-1);
	int ymax=(pint.y+bint+1);

	if (xmin < 0) 
		xmin = 0;
	if (ymin < 0) 
		ymin = 0;
	if (xmax >= data->collResolution.x) 
		xmax = data->collResolution.x - 1;
	if (ymax >= data->collResolution.y) 
		ymax = data->collResolution.y - 1;

	unsigned short *forcemap = data->forcemap;

	int hmapsh = 0;
	{
		for (int i = 0; i < 32-1; i++)
		{
			if ((1<<i) == data->collResolution.x)
			{
				hmapsh = i;
				break; 
			}
		}
	}
	assert(hmapsh != 0);

	//bool affected = false;

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
			int newval=data->collisionHeightMap[(y<<hmapsh) + x];
			if (newval<0) newval=0; else if (newval>65535) newval=65535;

			// TEMP!!!!
			//WORD forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];
			//int forceval = forcemap[((y>>1)<<(hmapsh-1))+(x>>1)];
			int forceval = forcemap[(y<<hmapsh)+x];

			if (forceval != 0)
			{
				if ((newval < forceval && above)
					|| (newval > forceval && below)) 
				{
					newval = forceval;
					data->collisionHeightMap[(y<<hmapsh) + x]=newval;
				}
			}
		}
	}
}

void Storm3D_TerrainHeightmap::calculateVisibility(Storm3D_Scene &scene)
{
	std::vector<RenderBlock> &visibleBlocks = data->visibleBlocks;

	visibleBlocks.clear();
	data->collectVisibleBlocks(scene, visibleBlocks);
}

void Storm3D_TerrainHeightmap::renderTextures(Storm3D_Scene &scene, bool atiShader)
{
	IDirect3DDevice9 &device = data->device;

	D3DXMATRIX dm;
	D3DXMatrixIdentity(&dm);

	D3DXMATRIX tm = dm;
	Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(device, tm);

	if(atiShader)
	{
		data->atiDefaultShader.apply();
	}
	else
		data->nvDefaultShader.apply();

	if(data->ps13)
		data->pixelShader.apply();
	else
	{
		device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		device.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		device.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		device.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		device.SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}

	data->vertexBuffer.apply(device, 1);
	std::vector<RenderBlock> &visibleBlocks = data->visibleBlocks;

	// Additive
	device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	for(int j = 0; j < 1; ++j)
	{
		device.SetSamplerState(j, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device.SetSamplerState(j, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}

	// Render texture splats
	for(unsigned int i = 0; i < visibleBlocks.size(); ++i)
	{
		RenderBlock &renderBlock = visibleBlocks[i];
		TerrainBlock &block = data->blocks[renderBlock.indexY * data->blockAmount.x + renderBlock.indexX];

		if(block.passes.empty() || !block.indexBuffer)
			continue;

		block.heightBuffer.apply(device, 0);

		int textureA = -2;
		int textureB = -2;

		for(unsigned int pass = 0; pass < block.passes.size(); ++pass)
		{
			if(pass == 0)
				device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			else if(pass == 1)
				device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

			TexturePass &p = block.passes[pass];
			p.weights->Apply(0);

			if(p.textureA != textureA)
			{
				data->textures[p.textureA].texture->Apply(1);
				textureA = p.textureA;
			}
			if(p.textureB != textureB)
			{
				if(p.textureB >= 0)
					data->textures[p.textureB].texture->Apply(2);
				else
					device.SetTexture(2, 0);

				textureB = p.textureB;
			}

			float range1 = data->getRange(scene, renderBlock.indexX - 1, renderBlock.indexY);
			float range2 = data->getRange(scene, renderBlock.indexX, renderBlock.indexY - 1);
			float range3 = data->getRange(scene, renderBlock.indexX + 1, renderBlock.indexY);
			float range4 = data->getRange(scene, renderBlock.indexX, renderBlock.indexY + 1);

			//data->indexBuffer.render(scene, p.subMask, renderBlock.range, range1, range2, range3, range4);
			block.indexBuffer->render(scene, p.subMask, renderBlock.range, range1, range2, range3, range4);
		}
	}

	device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device.SetPixelShader(0);

	device.SetTexture(1, 0);
	device.SetTexture(2, 0);

	for(int k = 0; k < 1; ++k)
	{
		device.SetSamplerState(k, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(k, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	device.SetTexture(0, 0);
	device.SetStreamSource(1, 0, 0, 0);
}

void Storm3D_TerrainHeightmap::renderDepth(Storm3D_Scene &scene, Storm3D_Camera *camera, RenderMode mode, RenderType type, IStorm3D_Spotlight::Type spot_type, Storm3D_Spotlight *spot)
{
	IDirect3DDevice9 &device = data->device;
	std::vector<RenderBlock> &visibleBlocks = data->visibleBlocks;

	Frustum *frustum1 = 0;
	Frustum realFrustum1;
	Frustum *frustum2 = 0;
	Frustum realFrustum2;

	if(mode == Projection)
	{
		D3DXMATRIX dm;
		D3DXMatrixIdentity(&dm);

		D3DXMATRIX tm = dm;
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(device, tm, false, true);

		if(camera)
		{
			realFrustum1 = static_cast<Storm3D_Camera *> (scene.GetCamera())->getFrustum();
			frustum1 = &realFrustum1;
		}
		if(spot)
		{
			realFrustum2 = spot->getCamera().getFrustum();
			frustum2 = &realFrustum2;
		}

		if(type == Ati)
		{
			if(spot_type == IStorm3D_Spotlight::Directional)
				data->atiShadowShaderDirectional.apply();
			if(spot_type == IStorm3D_Spotlight::Point)
				data->atiShadowShaderPoint.apply();
			if(spot_type == IStorm3D_Spotlight::Flat)
				data->atiShadowShaderFlat.apply();
			if(spot_type == -1)
				data->atiDefaultShader.apply();
		}
		else if(type == Nv)
		{
			if(spot_type == IStorm3D_Spotlight::Directional)
				data->nvShadowShaderDirectional.apply();
			if(spot_type == IStorm3D_Spotlight::Point)
				data->nvShadowShaderPoint.apply();
			if(spot_type == IStorm3D_Spotlight::Flat)
				data->nvShadowShaderFlat.apply();
			if(spot_type == -1)
				data->nvDefaultShader.apply();
		}
	}
	else if(mode == Lighting)
	{
		if(camera)
		{
			realFrustum1 = static_cast<Storm3D_Camera *> (scene.GetCamera())->getFrustum();
			frustum1 = &realFrustum1;
		}

		D3DXMATRIX dm;
		D3DXMatrixIdentity(&dm);

		D3DXMATRIX tm = dm;
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(device, tm, false, true);
		Storm3D_ShaderManager::GetSingleton()->ApplyForceAmbient(device);

		if(type == Ati)
			data->atiLightingShader.apply();
		else if(type == Nv)
			data->nvLightingShader.apply();

		data->lightPixelShader.apply();

		device.SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device.SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}
	else
	{
		if(spot)
		{
			realFrustum1 = spot->getCamera().getFrustum();
			frustum1 = &realFrustum1;
		}

		D3DXMATRIX dm;
		D3DXMatrixIdentity(&dm);

		D3DXMATRIX tm = dm;
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(device, tm, false, true);

		data->atiDepthShader.apply();
	}

	if(spot_type == -1)
	{
		device.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}

	data->vertexBuffer.apply(device, 1);
	device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	Sphere sphere;
	for(unsigned int i = 0; i < visibleBlocks.size(); ++i)
	{
		RenderBlock &renderBlock = visibleBlocks[i];
		TerrainBlock &block = data->blocks[renderBlock.indexY * data->blockAmount.x + renderBlock.indexX];

		if(block.passes.empty() || !block.indexBuffer)
			continue;

		if(camera)
		{
			VC3 blockPosition(-data->size.x/2 + data->positionDelta.x/2, 0, -data->size.z/2 + data->positionDelta.y/2);
			blockPosition.x += data->positionDelta.x * renderBlock.indexX;
			blockPosition.z += data->positionDelta.y * renderBlock.indexY;
			blockPosition.y = block.height;

			//if(!camera->TestSphereVisibility(blockPosition, data->radius))
			//	continue;
			sphere.position = blockPosition;
			sphere.radius = data->radius;
			if(block.heightRadius > sphere.radius)
				sphere.radius = block.heightRadius;

			/*
			if(!frustum.visibility(sphere, true))
				continue;
			if(!frustum.visibility(renderBlock.oobb))
				continue;
			*/

			if(frustum1)
			{
				if(!frustum1->visibility(sphere, true))
					continue;
				if(!frustum1->visibility(renderBlock.oobb))
					continue;
			}
			if(frustum2)
			{
				if(!frustum2->visibility(sphere, true))
					continue;
				if(!frustum2->visibility(renderBlock.oobb))
					continue;
			}
		}

		if(spot_type == -1)
		{
			if(block.lightMap)
			{
				//device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
				block.lightMap->Apply(0);
			}
			else
			{
				device.SetTexture(0, 0);
			}
		}

		block.heightBuffer.apply(device, 0);
		float range1 = data->getRange(scene, renderBlock.indexX - 1, renderBlock.indexY);
		float range2 = data->getRange(scene, renderBlock.indexX, renderBlock.indexY - 1);
		float range3 = data->getRange(scene, renderBlock.indexX + 1, renderBlock.indexY);
		float range4 = data->getRange(scene, renderBlock.indexX, renderBlock.indexY + 1);

		block.indexBuffer->render(scene, -1, renderBlock.range, range1, range2, range3, range4);
	}

	//if(mode != Projection)
	//	device.SetPixelShader(0);

	device.SetStreamSource(1, 0, 0, 0);
	//device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	if(spot_type == -1)
	{
		device.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

		device.SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}
}

int Storm3D_TerrainHeightmap::addTerrainTexture(Storm3D_Texture &texture)
{
	data->textures.push_back(TerrainTexture(frozenbyte::storm::createSharedTexture(&texture)));
	return data->textures.size() - 1;
}

void Storm3D_TerrainHeightmap::removeTerrainTextures()
{
	data->textures.clear();
}

void Storm3D_TerrainHeightmap::setBlendMap(int blockIndex, Storm3D_Texture &blend, int textureA, int textureB)
{
	assert(!data->blocks.empty());
	assert(blockIndex >= 0 && blockIndex < int(data->blocks.size())); 
	assert(textureA >= 0 && textureA < int(data->textures.size()));
	assert(textureB >= -1 && textureB < int(data->textures.size()));

	if(!data->ps13)
		assert(textureB == -1);

	std::vector<TexturePass> &passes = data->blocks[blockIndex].passes;
	
	passes.push_back(TexturePass(frozenbyte::storm::createSharedTexture(&blend), textureA, textureB));
	std::sort(passes.begin(), passes.end());	
}

void Storm3D_TerrainHeightmap::setPartialBlendMap(int blockIndex, int subMask, Storm3D_Texture &blend, int textureA, int textureB)
{
	assert(!data->blocks.empty());
	assert(blockIndex >= 0 && blockIndex < int(data->blocks.size())); 
	assert(subMask > 0 && subMask < 16);
	assert(textureA >= 0 && textureA < int(data->textures.size()));
	assert(textureB >= 0 && textureB < int(data->textures.size()));

	std::vector<TexturePass> &passes = data->blocks[blockIndex].passes;
	
	passes.push_back(TexturePass(frozenbyte::storm::createSharedTexture(&blend), textureA, textureB, subMask));
	std::sort(passes.begin(), passes.end());	
}

void Storm3D_TerrainHeightmap::resetBlends(int blockIndex)
{
	data->blocks[blockIndex].passes.clear();
}

/*
void Storm3D_TerrainHeightmap::setLightmap(Storm3D_Texture &texture)
{
	data->lightmap = frozenbyte::storm::createSharedTexture(&texture);
}
*/

void Storm3D_TerrainHeightmap::setLightMap(int blockIndex, Storm3D_Texture &texture)
{
	if(blockIndex < 0 || blockIndex >= int(data->blocks.size()))
	{
		assert(!"setLightMap -- blockIndex out of bounds");
		return;
	}

	data->blocks[blockIndex].lightMap = frozenbyte::storm::createSharedTexture(&texture);
}

VC3 Storm3D_TerrainHeightmap::getNormal(const VC2I &position) const
{
	return data->getNormal(position);
}

VC3 Storm3D_TerrainHeightmap::getFaceNormal(const VC2 &position) const
{
	VC2I pos = data->convertWorldToCollisionMap(position);
	if(pos.x <= 0 || pos.x >= data->collResolution.x - 1 || pos.y <= 0 || pos.y >= data->collResolution.y - 1)
		return VC3(0, 1.f, 0);

	VC3 positionU;
	VC3 positionN;
	VC3 positionUR;
	VC3 result;

	VC3 scale(data->size.x / (data->collResolution.x - 1), data->size.y / 65535.f, data->size.z / (data->collResolution.y - 1));
	getPosition(positionN, pos.x, pos.y, data->collisionHeightMap.get(), data->size, data->collResolution, scale);
	getPosition(positionU, pos.x, pos.y - 1, data->collisionHeightMap.get(), data->size, data->collResolution, scale);
	getPosition(positionUR, pos.x + 1, pos.y - 1, data->collisionHeightMap.get(), data->size, data->collResolution, scale);
	getFaceNormalImp(result, positionU, positionN, positionUR);

	result.Normalize();
	return result;
}

VC3 Storm3D_TerrainHeightmap::getInterpolatedNormal(const VC2 &position) const
{
	VC2I pos = data->convertWorldToCollisionMap(position);
	if(pos.x <= 0 || pos.x >= data->collResolution.x - 1 || pos.y <= 0 || pos.y >= data->collResolution.y - 1)
		return VC3(0, 1.f, 0);

	return VC3(0, 1.f, 0);
}

float Storm3D_TerrainHeightmap::getHeight(const VC2 &position) const
{
	float x = (position.x + data->size.x / 2) / data->size.x * data->collResolution.x;
	float y = (position.y + data->size.z / 2) / data->size.z * data->collResolution.y;
	int ix = int(x);
	int iy = int(y);

	if(ix < 0 || iy < 0)
		return 0.f;
	if(ix >= data->collResolution.x || iy >= data->collResolution.y)
		return 0.f;

	float ipX = x - ix;
	float ipY = y - iy;
	float scaleY = data->size.y / 65535.f;

	if(ipX + ipY <= 1)
	{
		float p0 = data->collisionHeightMap[(iy * data->collResolution.x) + ix] * scaleY;
		float px = data->collisionHeightMap[(iy * data->collResolution.x) + ix + 1] * scaleY;
		float py = data->collisionHeightMap[((iy + 1) * data->collResolution.x) + ix] * scaleY;

		return p0 + (px-p0) * ipX + (py - p0) * ipY;
	}
	else
	{
		float pxy = data->collisionHeightMap[((iy + 1) * data->collResolution.x) + ix + 1] * scaleY;
		float px = data->collisionHeightMap[(iy * (data->collResolution.x)) + ix + 1] * scaleY;
		float py = data->collisionHeightMap[((iy + 1) * data->collResolution.x) + ix] * scaleY;

		return pxy + (py - pxy) * (1.0f - ipX) + (px - pxy) * (1.0f - ipY);
	}

/*
	float x = (position.x + data->size.x / 2) / data->size.x * (data->resolution.x - 1);
	float y = (position.y + data->size.z / 2) / data->size.z * (data->resolution.y - 1);
	int ix = int(x);
	int iy = int(y);

	if(ix < 0 || iy < 0)
		return 0.f;
	if(ix >= data->resolution.x || iy >= data->resolution.y)
		return 0.f;

	float ipX = x - ix;
	float ipY = y - iy;
	float scaleY = data->size.y / 65535.f;

	if(ipX + ipY <= 1)
	{
		float p0 = data->heightMap[(iy * data->resolution.x) + ix] * scaleY;
		float px = data->heightMap[(iy * data->resolution.x) + ix + 1] * scaleY;
		float py = data->heightMap[((iy + 1) * data->resolution.x) + ix] * scaleY;

		return p0 + (px-p0) * ipX + (py - p0) * ipY;
	}
	else
	{
		float pxy = data->heightMap[((iy + 1) * data->resolution.x) + ix + 1] * scaleY;
		float px = data->heightMap[(iy * (data->resolution.x)) + ix + 1] * scaleY;
		float py = data->heightMap[((iy + 1) * data->resolution.x) + ix] * scaleY;

		return pxy + (py - pxy) * (1.0f - ipX) + (px - pxy) * (1.0f - ipY);
	}
*/
	return 0.f;
}


float Storm3D_TerrainHeightmap::getPartiallyInterpolatedHeight(const VC2 &position) const
{
	// same as getHeight, but does not interpolate steep slopes.

	float x = (position.x + data->size.x / 2) / data->size.x * data->collResolution.x;
	float y = (position.y + data->size.z / 2) / data->size.z * data->collResolution.y;
	int ix = int(x);
	int iy = int(y);

	if(ix < 0 || iy < 0)
		return 0.f;
	if(ix >= data->collResolution.x || iy >= data->collResolution.y)
		return 0.f;

	float ipX = x - ix;
	float ipY = y - iy;
	float scaleY = data->size.y / 65535.f;

	if(ipX + ipY <= 1)
	{
		float p0 = data->collisionHeightMap[(iy * data->collResolution.x) + ix] * scaleY;
		float px = data->collisionHeightMap[(iy * data->collResolution.x) + ix + 1] * scaleY;
		float py = data->collisionHeightMap[((iy + 1) * data->collResolution.x) + ix] * scaleY;

		// TEMP HACK: no interpolation for steep slopes
		if (fabs(p0 - py) >= 1.0f)
			py = p0;
		if (fabs(px - py) >= 1.0f)
			px = p0;

		return p0 + (px-p0) * ipX + (py - p0) * ipY;
	}
	else
	{
		float pxy = data->collisionHeightMap[((iy + 1) * data->collResolution.x) + ix + 1] * scaleY;
		float px = data->collisionHeightMap[(iy * (data->collResolution.x)) + ix + 1] * scaleY;
		float py = data->collisionHeightMap[((iy + 1) * data->collResolution.x) + ix] * scaleY;

		// TEMP HACK: no interpolation for steep slopes
		float p0 = data->collisionHeightMap[(iy * data->collResolution.x) + ix] * scaleY;
		if (fabs(p0 - py) >= 1.0f)
		{
			py = p0;
			pxy = px;
		}
		if (fabs(p0 - px) >= 1.0f)
		{
			px = p0;
			pxy = py;
		}
		if (fabs(p0 - pxy) >= 1.0f)
			pxy = p0;

		return pxy + (py - pxy) * (1.0f - ipX) + (px - pxy) * (1.0f - ipY);
	}

	return 0.f;
}

VC3 Storm3D_TerrainHeightmap::solveObstacleNormal(const VC2I &obstaclePosition, const VC3& fromDirection) const
{
	int obstacle_map_mult_shift = data->obstaclemapShiftMult;
	int collision_heightmap_shift = data->heightmapShiftMult;

  int maxx = (data->resolution.x-1)<<obstacle_map_mult_shift;
  int maxy = (data->resolution.y-1)<<obstacle_map_mult_shift;

	VC3 mmult_world_map=VC3((float)data->collResolution.x,65535.0f,(float)data->collResolution.y)/data->size;

	int hmapsh = 0;
	{
		for (int i = 0; i < 32-1; i++)
		{
			if ((1<<i) == data->collResolution.x)
			{
				hmapsh = i;
				break; 
			}
		}
	}
	assert(hmapsh != 0);
  int obstacleShift = hmapsh + obstacle_map_mult_shift - collision_heightmap_shift;

	int x = obstaclePosition.x;
	int y = obstaclePosition.y;

	if (x <= 1 || x >= maxx-(1<<obstacle_map_mult_shift)
		|| y <= 1 || y >= maxy-(1<<obstacle_map_mult_shift))
	{
		assert(!"Storm3D_TerrainHeightmap::solveObstacleNormal - requested position out of map bounds.");
		return VC3(0,1,0);
	}

  int obstacleBlockIndex = (y<<obstacleShift) + x;
  int obstacleBlockIndexU = (y<<obstacleShift) + (x-1);
  int obstacleBlockIndexD = (y<<obstacleShift) + (x+1);
  int obstacleBlockIndexL = ((y-1)<<obstacleShift) + x;
  int obstacleBlockIndexR = ((y+1)<<obstacleShift) + x;

	// NOTE: does not contain heightmap (ground) height... just the obstacle
	// (thus, may give incorrect results near sudden drops / tilted surfaces...
	int obstacleHeight = ((int)(data->obstacleHeightmap[obstacleBlockIndex] & OBSTACLE_MAP_MASK_HEIGHT));
	int obstacleHeightU = ((int)(data->obstacleHeightmap[obstacleBlockIndexU] & OBSTACLE_MAP_MASK_HEIGHT));
	int obstacleHeightD = ((int)(data->obstacleHeightmap[obstacleBlockIndexD] & OBSTACLE_MAP_MASK_HEIGHT));
	int obstacleHeightL = ((int)(data->obstacleHeightmap[obstacleBlockIndexL] & OBSTACLE_MAP_MASK_HEIGHT));
	int obstacleHeightR = ((int)(data->obstacleHeightmap[obstacleBlockIndexR] & OBSTACLE_MAP_MASK_HEIGHT));

	// changes over 0.2m are considered to be significant...
	int steepLimit = (int)(0.2f * mmult_world_map.y);

	bool blockedU = false;
	bool blockedD = false;
	bool blockedL = false;
	bool blockedR = false;

	// some great logic here. :)
	if (obstacleHeightU >= obstacleHeight - steepLimit) blockedU = true;
	if (obstacleHeightD >= obstacleHeight - steepLimit) blockedD = true;
	if (obstacleHeightL >= obstacleHeight - steepLimit) blockedL = true;
	if (obstacleHeightR >= obstacleHeight - steepLimit) blockedR = true;

	if (blockedD && blockedU && blockedL && blockedR)
	{
		VC3 unsolved = -fromDirection;
		return unsolved;
	}
		
	if (!blockedR && blockedL && blockedU && blockedD)
		return VC3(0,0,1);
	if (blockedR && !blockedL && blockedU && blockedD)
		return VC3(0,0,-1);
	if (blockedR && blockedL && !blockedU && blockedD)
		return VC3(-1,0,0);
	if (blockedR && blockedL && blockedU && !blockedD)
		return VC3(1,0,0);

	if (blockedR && !blockedL && blockedU && !blockedD)
		return VC3(1,0,-1).GetNormalized();
	if (!blockedR && blockedL && blockedU && !blockedD)
		return VC3(1,0,1).GetNormalized();
	if (blockedR && !blockedL && !blockedU && blockedD)
		return VC3(-1,0,-1).GetNormalized();
	if (!blockedR && blockedL && !blockedU && blockedD)
		return VC3(-1,0,1).GetNormalized();

	if (blockedR && !blockedL && !blockedU && !blockedD)
		return VC3(0,0,-1);
	if (!blockedR && blockedL && !blockedU && !blockedD)
		return VC3(0,0,1);
	if (!blockedR && !blockedL && blockedU && !blockedD)
		return VC3(1,0,0);
	if (!blockedR && !blockedL && !blockedU && blockedD)
		return VC3(-1,0,0);

	// what about thin U-D (or, L-R) lines... (should the result be L or R (or, U or D) normal)

  //return VC3(0,1,0);
	VC3 unsolved = -fromDirection;
  return unsolved;
}

void Storm3D_TerrainHeightmap::rayTrace(const VC3 &position, const VC3 &directionNormalized, float rayLength, Storm3D_CollisionInfo &rti, ObstacleCollisionInfo &oci, bool accurate, bool lineOfSight) const
{
	if(!data->collisionHeightMap)
		return;

	// That accurate thingy is just too messy for now ;-)
	// -- psd

	if (accurate)
	{
		int obstacle_map_mult_shift = data->obstaclemapShiftMult;
		int collision_heightmap_shift = data->heightmapShiftMult;

		int obstacle_minus_heightmap_shift = obstacle_map_mult_shift - collision_heightmap_shift;

		VC3 mmult_world_map=VC3((float)data->collResolution.x,65535.0f,(float)data->collResolution.y)/data->size;
		VC3 mmult_map_world=data->size/VC3((float)data->collResolution.x,65535.0f,(float)data->collResolution.y);	

		// damn, can't have this here if the raytrace is const... =/
		//if (collisionMap == NULL)
		//  RecreateCollisionMap();

    // alternative method for terrain raytrace, more accurate, but slower
    // not very clear code, quickly transformed from a 2d line drawing
    // routine. -jpk

    VC3 epos(position+directionNormalized*rayLength);

    VC2I ispos=data->convertWorldToObstacleMap(VC2(position.x,position.z));
	  VC2I iepos=data->convertWorldToObstacleMap(VC2(epos.x,epos.z));

    int i;
    int steep = 0;
    int sx, sy;
    int dx, dy;
    int e;

		// the generic skip mask
		WORD skipObstacleMask;
		// and the more specific skip mask with specific expected value
		WORD skipObstacle2Mask;
		WORD skipObstacle2Value;
		if (lineOfSight)
		{
			skipObstacleMask = OBSTACLE_AREA_SEETHROUGH;
			// (this mask/value pair always results into false value)
			skipObstacle2Mask = 0;
			skipObstacle2Value = 1;
		} else {
			skipObstacleMask = OBSTACLE_AREA_UNHITTABLE;
			// (skip if buildingwall but not breakable)
			skipObstacle2Mask = OBSTACLE_AREA_BUILDINGWALL | OBSTACLE_AREA_BREAKABLE;
			skipObstacle2Value = OBSTACLE_AREA_BUILDINGWALL;
		}

    int x, x2;
    int y, y2;
    int h;
	  int hs = (int)(position.y * mmult_world_map.y);
	  int he = (int)(epos.y * mmult_world_map.y);
    int hdiff = he - hs;

		int hmapsh = 0;
		{
			for (int i = 0; i < 32-1; i++)
			{
				if ((1<<i) == data->collResolution.x)
				{
					hmapsh = i;
					break; 
				}
			}
		}
		assert(hmapsh != 0);

		/*
		int hmapsh_swapped = 0;
		{
			for (int i = 0; i < 32-1; i++)
			{
				if ((1<<i) == data->collResolution.y)
				{
					hmapsh_swapped = i;
					break; 
				}
			}
		}
		assert(hmapsh_swapped != 0);
		*/

    int obstacleShift = hmapsh + obstacle_map_mult_shift - collision_heightmap_shift;
    //int obstacleShift_swapped = hmapsh_swapped + OBSTACLE_MAP_MULT_SHIFT - COLLISION_HEIGHTMAP_SHIFT;
    //int collShift = hmapsh - COLLISION_MAP_DIV_SHIFT;

    x = ispos.x;
    y = ispos.y;
    x2 = iepos.x;
    y2 = iepos.y;

    dx = abs(x2 - x);
    sx = ((x2 - x) > 0) ? 1 : -1;
    dy = abs(y2 - y);
    sy = ((y2 - y) > 0) ? 1 : -1;

		int prevObstacleHeight = 0;

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

    int maxx = (data->resolution.x-1)<<obstacle_map_mult_shift;
    int maxy = (data->resolution.y-1)<<obstacle_map_mult_shift;
    int maxx_minus_one = maxx - (1<<obstacle_map_mult_shift);
    int maxy_minus_one = maxy - (1<<obstacle_map_mult_shift);

    e = 2 * dy - dx;
    for (i = 0; i < dx; i++) 
    {
      //h = hs + hdiff * i / dx;
      // for better accuracy...
      h = 3 * hs + (3 * hdiff * i) / dx;

      // interpolation (below) will f*ck up if we don't keep a little
      // safety distance to map edges...
			//assert(maxx == 2048);

			// wtf were these two?
			//h = h + 1;
			//h = h - 1;

			//assert(maxy == 2048);
			if (steep)
			{
				if (y <= 1 || y >= maxx_minus_one
					|| x <= 1 || x >= maxy_minus_one)
				{
					//assert(!"TEMP, max assert failed.");
					return;
				}
			} else {
				if (x <= 1 || x >= maxx_minus_one
					|| y <= 1 || y >= maxy_minus_one)
				{
					//assert(!"TEMP, max assert failed.");
					return;
				}
			}

      int blockIndex;
      int obstacleBlockIndex;

      // improve raytrace accuracy with hmap interpolation... slow :(
      // designed for obstacle/height map multiplier 2 (should work for 
      // other multiplier values too, but result may be less desirable)
//    int blockIndexR;
//    int blockIndexD;
      //int blockIndexU;
      //int blockIndexL;

      if (steep) 
      {
        blockIndex = ((x>>obstacle_minus_heightmap_shift)<<hmapsh) + (y>>obstacle_minus_heightmap_shift);
// TEMP: heightmap interpolation disabled!
// TODO: should be enabled, but ONLY for outdoor heightmaps, not inside buildings
//        blockIndexD = ((x>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT))<<hmapsh) + ((y+1)>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT));
//        blockIndexR = (((x+1)>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT))<<hmapsh) + (y>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT));
				// old
        //blockIndexU = ((x>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh_swapped) + ((y-1)>>OBSTACLE_MAP_MULT_SHIFT);
        //blockIndexL = (((x-1)>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh_swapped) + (y>>OBSTACLE_MAP_MULT_SHIFT);
        obstacleBlockIndex = (x<<obstacleShift) + y;
      } else {
        blockIndex = ((y>>obstacle_minus_heightmap_shift)<<hmapsh) + (x>>obstacle_minus_heightmap_shift);
// TEMP: heightmap interpolation disabled!
// TODO: should be enabled, but ONLY for outdoor heightmaps, not inside buildings
//        blockIndexD = ((y>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT))<<hmapsh) + ((x+1)>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT));
//        blockIndexR = (((y+1)>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT))<<hmapsh) + (x>>(OBSTACLE_MAP_MULT_SHIFT-COLLISION_HEIGHTMAP_SHIFT));
				// old
        //blockIndexU = ((y>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + ((x-1)>>OBSTACLE_MAP_MULT_SHIFT);
        //blockIndexL = (((y-1)>>OBSTACLE_MAP_MULT_SHIFT)<<hmapsh) + (x>>OBSTACLE_MAP_MULT_SHIFT);
        obstacleBlockIndex = (y<<obstacleShift) + x;
      }

		  // Test hit
// TEMP: heightmap interpolation disabled!
// TODO: should be enabled, but ONLY for outdoor heightmaps, not inside buildings
//      int mapHeight = ((int)data->collisionHeightMap[blockIndex] + (int)data->collisionHeightMap[blockIndexR] 
//        + (int)data->collisionHeightMap[blockIndexD]);
      int mapHeight = (int)data->collisionHeightMap[blockIndex] * 3;

		  if (mapHeight > h)
		  {
			  rti.hit = true;
			  rti.model = NULL;
			  rti.object = NULL;
        VC2 hitpos;
        if (steep) 
        {
          hitpos = data->convertObstacleMapToWorld(VC2I(y,x));
        } else {
          hitpos = data->convertObstacleMapToWorld(VC2I(x,y));
        }

				VC3 hitvec;
				if (fabs(directionNormalized.y) > 0.0001f)
				{
					float planeHeight = (mapHeight / 3)*mmult_map_world.y;
					float planeYDist = planeHeight - position.y;

					VC3 tmp = VC3(hitpos.x, (h / 3)*mmult_map_world.y, hitpos.y);

					float planeRange = planeYDist / directionNormalized.y;
					VC3 planeHitPos = position + directionNormalized * planeRange;
					
					VC2 planeHit2D = VC2(planeHitPos.x, planeHitPos.z);
					float mapHeightAtPlaneHit = getHeight(planeHit2D);

					//float planeHitError = planeHitPos.y - tmp.y;
					float planeHitError = mapHeightAtPlaneHit - planeHitPos.y;
				
					// HACK: if too much height difference, don't calculate position on plane...
					if (fabs(planeHitError) >= 1.0f)
					{
						hitvec = tmp - position;
					} else {
						hitvec = planeHitPos - position;
					}
				} else {
	        hitvec = VC3(hitpos.x, (mapHeight / 3)*mmult_map_world.y, hitpos.y) - position;
				}

			  rti.range = hitvec.GetLength();
			  rti.position = position + directionNormalized*rti.range;

				// FIXME: why exactly does this assert sometimes fail near the edge 
				// of the map? x and y should be inside boundaries, why does this go outside.
				// (note, hitvec direction != directionNormalized direction - possibly explains that?)
				if (rti.position.x < -(data->collResolution.x * mmult_map_world.x)/2)
					rti.position.x = -(data->collResolution.x * mmult_map_world.x)/2;
				if (rti.position.x > (data->collResolution.x * mmult_map_world.x)/2)
					rti.position.x = (data->collResolution.x * mmult_map_world.x)/2;
				if (rti.position.z < -(data->collResolution.y * mmult_map_world.z)/2)
					rti.position.z = -(data->collResolution.y * mmult_map_world.z)/2;
				if (rti.position.z > (data->collResolution.y * mmult_map_world.z)/2)
					rti.position.z = (data->collResolution.y * mmult_map_world.z)/2;
				/*
        assert(!(rti.position.x <= -(data->collResolution.x * mmult_map_world.x)/2 
          || rti.position.x >= (data->collResolution.x * mmult_map_world.x)/2
          || rti.position.z <= -(data->collResolution.y * mmult_map_world.z)/2
          || rti.position.z >= (data->collResolution.y * mmult_map_world.z)/2)
        );
				*/

				// TODO: solve heightmap hit plane normal..
				rti.plane_normal = VC3(0,1,0);

			  return;
      } else {
        // hit obstacle?
        //assert(obstacleHeightmap != NULL);
				if (data->obstacleHeightmap != NULL
					&& data->areaMap != NULL)
				{
					int obstacleHeight = mapHeight + 3 * ((int)(data->obstacleHeightmap[obstacleBlockIndex] & OBSTACLE_MAP_MASK_HEIGHT));
  				if (obstacleHeight > h)
					{
						// make sure that the obstacle is something we want to 
						// hit... (not seethrough or unhittable)
						//if ((data->obstacleHeightmap[obstacleBlockIndex] & skipObstacleMask) == 0)
						if (!data->areaMap->isAreaAnyValue(obstacleBlockIndex, skipObstacleMask)
							&& !data->areaMap->isAreaValue(obstacleBlockIndex, skipObstacle2Mask, skipObstacle2Value))
						{
							oci.hit = true;
							VC2 hitpos;
							if (steep) 
							{
								hitpos = data->convertObstacleMapToWorld(VC2I(y,x));
							} else {
								hitpos = data->convertObstacleMapToWorld(VC2I(x,y));
							}
							//VC3 hitvec = VC3(hitpos.x, hmap[blockIndex]*mmult_map_world.y, hitpos.y) - position;
							VC3 hitvec = VC3(hitpos.x, (h / 3) *mmult_map_world.y, hitpos.y) - position;
							float hitveclen = hitvec.GetLength();
							VC3 realpos = position + directionNormalized*hitveclen;
							if (oci.hitAmount == 0)
							{
  	  					oci.range = hitveclen;
    						oci.position = realpos;
							}
							// did we perhaps hit the obstacle from above?
							// (the logic is that if previous block was almost as high but did not 
							// collide to that, now we must have just hit the top of the obstacle...
							// (0.2m is the difference limit...)
							VC3 hitnormal;
							if (fabs(float(obstacleHeight/3 - prevObstacleHeight/3)) * mmult_map_world.y < 0.2f)
							{
								hitnormal = VC3(0,1,0);
							} else {
								// otherwise, we have probably hit the side of the obstacle...
								// TODO: solve proper hit normal!!!
								VC2I opos;
								if (steep)
								{
									opos = VC2I(y,x);
								} else {
									opos = VC2I(x,y);
								}
								hitnormal = solveObstacleNormal(opos, directionNormalized);
							}
							if (oci.hitAmount < MAX_OBSTACLE_COLLISIONS)
							{
								oci.ranges[oci.hitAmount] = hitveclen;
								oci.positions[oci.hitAmount] = realpos;
								oci.plane_normals[oci.hitAmount] = hitnormal;
							}
							oci.hitAmount++;
						}
					}
					prevObstacleHeight = obstacleHeight;
        }
      }

      while (e >= 0) 
      {
	      y += sy;
	      e -= 2 * dx;
	    }
	    x += sx;
	    e += 2 * dy;

			// HACK: LOS skips every second "pixel"...
			if (lineOfSight)
			{
				i++;
				if (i >= dx)
					break;

				while (e >= 0) 
				{
					y += sy;
					e -= 2 * dx;
				}
				x += sx;
				e += 2 * dy;
			}

    }
	} else {

		assert(!"don't use this. :)");

		// We have lower res map now
		static const int accuracyShift = 3;

		VC3 endPosition(position + directionNormalized * rayLength);

		VC2I iStartPosition = data->convertWorldToMap(VC2(position.x, position.z));
		VC2I iEndPosition = data->convertWorldToMap(VC2(endPosition.x, endPosition.z));

		int deltaX = iEndPosition.x - iStartPosition.x;
		int deltaY = iEndPosition.y - iStartPosition.y;
		int length = int(sqrtf(float(deltaX * deltaX + deltaY * deltaY))) + 1;

		int addX = (deltaX << accuracyShift) / length;
		int addY = (deltaY << accuracyShift) / length;
		float scaleY = 65535.f / data->size.y;

		int heightStart = int(position.y * scaleY);
		int heightEnd = int(endPosition.y * scaleY);
		int heightPosition = heightStart << accuracyShift;
		int heightAdd = ((heightEnd - heightStart) << accuracyShift) / length;

		// Test pixels
		int positionX = iStartPosition.x << accuracyShift;
		int positionY = iStartPosition.y << accuracyShift;
		int maxIndex = data->collResolution.x * data->collResolution.y;
		int blockIndex = 0;

		for(int i = 0; i < length; ++i)
		{
			if(heightPosition >= 16777215 << accuracyShift)
				continue;

			blockIndex = ((positionY >> accuracyShift) * data->collResolution.x) + (positionX >> accuracyShift);
			if(blockIndex < 0 || blockIndex >= maxIndex) 
				return;

			if(data->collisionHeightMap[blockIndex] > (heightPosition >> accuracyShift))
			{
				rti.hit = true;
				rti.model = 0;
				rti.object = 0;
				rti.range = float(i) / float(length) * rayLength;
				
				rti.position = position + directionNormalized * rti.range;
				//rti.plane_normal = ?;
				return;
			}

			positionX += addX;
			positionY += addY;
			heightPosition += heightAdd;
		}

	}
}
