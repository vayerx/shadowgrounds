// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <vector>

#include "storm3d_terrain_lod.h"
#include "storm3d_terrain_utils.h"
#include "storm3d.h"
#include "storm3d_scene.h"

#include <boost/static_assert.hpp>
#include <boost/scoped_array.hpp>

#include "../../util/Debug_MemoryManager.h"

	static const int LOD_AMOUNT = 3;

	static const int CENTER_BLOCK_SIZE = IStorm3D_Terrain::CENTER_BLOCK_SIZE;
	static const int CENTER_BLOCK_AMOUNT = 16;

	BOOST_STATIC_ASSERT(CENTER_BLOCK_SIZE == 2);

	struct LodIndexBuffer
	{
		frozenbyte::storm::IndexBuffer indexBuffer;
		int faceAmount;
		int lodFactor;

		enum Type
		{ 
			FullBuffer, 
			CenterBuffer,
			LinkBuffer
		};

		LodIndexBuffer()
		:	faceAmount(0),
			lodFactor(1)
		{
		}

		void generate(Type type, int resolution, int lodFactor_, bool leftLod, bool rightLod, bool upLod, bool downLod, unsigned char *clipBuffer)
		{
			faceAmount = 0;
			lodFactor = lodFactor_;

			int bufferSize = (resolution - 1) * (resolution - 1) * 6 * sizeof(short);
			indexBuffer.create((resolution - 1) * (resolution - 1) * 2, false);

			boost::scoped_array<unsigned short> tempBuffer(new unsigned short[bufferSize / sizeof(short)]);
			if(!tempBuffer)
				return;

			if(type == FullBuffer)
				generateCenter(tempBuffer.get(), resolution, clipBuffer);

			generateLink(tempBuffer.get(), resolution, leftLod, rightLod, upLod, downLod, clipBuffer);

			unsigned short *buffer = indexBuffer.lock();
			memcpy(buffer, tempBuffer.get(), faceAmount * sizeof(short));

			faceAmount /= 3;
			indexBuffer.unlock();
		}

		void generateCenters(int resolution, int lodFactor_, int mask, unsigned char *clipBuffer)
		{
			faceAmount = 0;
			lodFactor = lodFactor_;

			int bufferSize = (resolution - lodFactor) * (resolution - lodFactor) * 2;
			indexBuffer.create(bufferSize, false);

			unsigned short *buffer = indexBuffer.lock();
			if(!buffer)
				return;

			int startVertex = 1;
			int endVertex = resolution - 1;
			int delta = (endVertex - startVertex) / lodFactor;

			// ToDo: general version
			assert(mask >= 0);
			assert(mask <= CENTER_BLOCK_AMOUNT);

			if(mask & 1)
			{
				int x = 0;
				int y = 0;

				int x1 = (startVertex + x * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y1 = (startVertex + y * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int x2 = (startVertex + (x + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y2 = (startVertex + (y + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;

				generateTriangles(buffer, resolution, x1, y1, x2, y2, clipBuffer);
			}
			if(mask & 2)
			{
				int x = 1;
				int y = 0;

				int x1 = (startVertex + x * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y1 = (startVertex + y * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int x2 = (startVertex + (x + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y2 = (startVertex + (y + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;

				generateTriangles(buffer, resolution, x1, y1, x2, y2, clipBuffer);
			}
			if(mask & 4)
			{
				int x = 0;
				int y = 1;

				int x1 = (startVertex + x * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y1 = (startVertex + y * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int x2 = (startVertex + (x + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y2 = (startVertex + (y + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;

				generateTriangles(buffer, resolution, x1, y1, x2, y2, clipBuffer);
			}
			if(mask & 8)
			{
				int x = 1;
				int y = 1;

				int x1 = (startVertex + x * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y1 = (startVertex + y * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int x2 = (startVertex + (x + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;
				int y2 = (startVertex + (y + 1) * delta) / CENTER_BLOCK_SIZE * lodFactor;

				generateTriangles(buffer, resolution, x1, y1, x2, y2, clipBuffer);
			}

			faceAmount /= 3;
			indexBuffer.unlock();
		}

	private:
		void generateCenter(unsigned short *buffer, int resolution, unsigned char *clipBuffer)
		{
			int startVertex = lodFactor;
			int endVertex = resolution - lodFactor - 1;

			generateTriangles(buffer, resolution, startVertex, startVertex, endVertex, endVertex, clipBuffer);
		}

		void generateLink(unsigned short *buffer, int resolution, bool leftLod, bool rightLod, bool upLod, bool downLod, unsigned char *clipBuffer)
		{
			int startVertex = lodFactor;
			int endVertex = resolution - lodFactor - 1;

			if(!leftLod)
			{
				generateTriangles(buffer, resolution, 0, startVertex, startVertex, endVertex, clipBuffer);

				createFace(buffer, -1, 0, lodFactor * resolution, lodFactor * resolution + lodFactor, resolution, clipBuffer);
				createFace(buffer, -1, (resolution - 1 - lodFactor) * resolution, (resolution - 1 ) * resolution, (resolution - 1 - lodFactor) * resolution + lodFactor, resolution, clipBuffer);
			}
			else
				generateHorizontal(buffer, resolution, startVertex, -lodFactor, clipBuffer);

			if(!rightLod)
			{
				generateTriangles(buffer, resolution, endVertex, startVertex, resolution - 1, endVertex, clipBuffer);
				
				createFace(buffer, -1, resolution - 1, resolution * lodFactor + resolution - 1 - lodFactor, resolution * lodFactor + resolution - 1, resolution, clipBuffer);
				createFace(buffer, -1, (resolution - 1 - lodFactor) * resolution + resolution - 1 - lodFactor, (resolution - 1) * resolution + resolution - 1, (resolution - 1 - lodFactor) * resolution + resolution - 1, resolution, clipBuffer);
			}
			else
				generateHorizontal(buffer, resolution, endVertex, lodFactor, clipBuffer);
			
			if(!upLod)
			{
				generateTriangles(buffer, resolution, startVertex, 0, endVertex, startVertex, clipBuffer);

				createFace(buffer, -1, 0, resolution * lodFactor + lodFactor, lodFactor, resolution, clipBuffer);
				createFace(buffer, -1, resolution - 1, endVertex, lodFactor * resolution + endVertex, resolution, clipBuffer);
			}
			else
				generateVertical(buffer, resolution, startVertex, -lodFactor, clipBuffer);

			if(!downLod)
			{
				generateTriangles(buffer, resolution, startVertex, endVertex, endVertex, resolution - 1, clipBuffer);

				createFace(buffer, -1, (resolution - 1) * resolution, (resolution - 1) * resolution + lodFactor, (resolution - 1 - lodFactor) * resolution + lodFactor, resolution, clipBuffer);
				createFace(buffer, -1, (resolution - 1) * resolution + resolution - 1 - lodFactor, (resolution - 1) * resolution + resolution - 1, (resolution - 1 - lodFactor) * resolution + resolution - 1 - lodFactor, resolution, clipBuffer);
			}
			else
				generateVertical(buffer, resolution, endVertex, lodFactor, clipBuffer);
		}

		void generateTriangles(unsigned short *buffer, int resolution, int x1, int y1, int x2, int y2, unsigned char *clipBuffer)
		{
			for(int y = y1; y < y2; y += lodFactor)
			for(int x = x1; x < x2; x += lodFactor)
			{
				int position = y * resolution + x;

				int f1 = position + lodFactor + resolution * lodFactor;
				int f2 = position;
				int f3 = position + resolution * lodFactor;
				if(!clipped(f1, f2, f3, clipBuffer))
				{
					buffer[faceAmount++] = f1;
					buffer[faceAmount++] = f2;
					buffer[faceAmount++] = f3;
				}

				int f4 = position;
				int f5 = position + resolution * lodFactor + lodFactor;
				int f6 = position + lodFactor;
				if(!clipped(f1, f2, f3, clipBuffer))
				{
					buffer[faceAmount++] = f4;
					buffer[faceAmount++] = f5;
					buffer[faceAmount++] = f6;
				}
			}
		}

		void generateHorizontal(unsigned short *buffer, int resolution, int column, int direction, unsigned char *clipBuffer)
		{
			for(int y = lodFactor; y < resolution - lodFactor; y += 2 * lodFactor)
			{
				int position = y * resolution + column;

				{
					int f1 = position;
					int f2 = position + direction - resolution * lodFactor;
					int f3 = position + direction + resolution * lodFactor;

					createFace(buffer, direction, f1, f2, f3, resolution, clipBuffer);
				}

				if(y < resolution - 2 * lodFactor)
				{
					int f1 = position;
					int f2 = position + direction + resolution * lodFactor;
					int f3 = position + resolution * lodFactor;

					createFace(buffer, direction, f1, f2, f3, resolution, clipBuffer);

					f1 = position + resolution * lodFactor;
					f2 = position + direction + resolution * lodFactor;
					f3 = position + resolution * lodFactor * 2;

					createFace(buffer, direction, f1, f2, f3, resolution, clipBuffer);
				}
			}
		}

		void generateVertical(unsigned short *buffer, int resolution, int row, int direction, unsigned char *clipBuffer)
		{
			for(int x = lodFactor; x < resolution - lodFactor; x += 2 * lodFactor)
			{
				int position = row * resolution + x;

				{
					int f1 = position;
					int f2 = position + lodFactor + direction * resolution;
					int f3 = position - lodFactor + direction * resolution;

					createFace(buffer, direction, f1, f2, f3, resolution, clipBuffer);
				}

				if(x < resolution - 2 * lodFactor)
				{
					int f1 = position;
					int f2 = position + lodFactor;
					int f3 = position + lodFactor + direction * resolution;

					createFace(buffer, direction, f1, f2, f3, resolution, clipBuffer);

					f1 = position + lodFactor;
					f2 = position + 2 * lodFactor;
					f3 = position + lodFactor + direction * resolution;

					createFace(buffer, direction, f1, f2, f3, resolution, clipBuffer);
				}
			}
		}

		bool clipped(int index, unsigned char *clipBuffer) const
		{
			if(clipBuffer && clipBuffer[index])
				return true;

			return false;
		}

		bool clipped(int f1, int f2, int f3, unsigned char *clipBuffer) const
		{
			bool f1Clip = clipped(f1, clipBuffer);
			bool f2Clip = clipped(f2, clipBuffer);
			bool f3Clip = clipped(f3, clipBuffer);

			if(f1Clip && f2Clip && f3Clip)
				return true;

			return false;
		}

		void createFace(unsigned short *buffer, int direction, int f1, int f2, int f3, int resolution, unsigned char *clipBuffer)
		{
			if(clipped(f1, f2, f3, clipBuffer))
				return;

			if(direction < 0)
			{
				buffer[faceAmount++] = f1;
				buffer[faceAmount++] = f2;
				buffer[faceAmount++] = f3;
			}
			else
			{
				buffer[faceAmount++] = f2;
				buffer[faceAmount++] = f1;
				buffer[faceAmount++] = f3;
			}
		}
	};

	struct LOD
	{
		// left (no lod / lod), right, up, down

		// Center + links
		LodIndexBuffer fullBuffers[16];
		// Links only
		LodIndexBuffer linkBuffers[16];

		// Center in 16 pieces (2x2 chunks, same ordering)
		LodIndexBuffer centerBuffers[16];

		void generate(int resolution, int lod, unsigned char *clipBuffer, IStorm3D_Logger *logger)
		{
			for(int l = 0; l < 2; ++l)
			for(int r = 0; r < 2; ++r)
			for(int u = 0; u < 2; ++u)
			for(int d = 0; d < 2; ++d)
			{
				bool leftLod = l == 1;
				bool rightLod = r == 1;
				bool upLod = u == 1;
				bool downLod = d == 1;

				int index = l + r*2 + u*4 + d*8;
				
				fullBuffers[index].generate(LodIndexBuffer::FullBuffer, resolution, lod, leftLod, rightLod, upLod, downLod, clipBuffer);
				linkBuffers[index].generate(LodIndexBuffer::LinkBuffer, resolution, lod, leftLod, rightLod, upLod, downLod, clipBuffer);
			}

			for(int i = 0; i < CENTER_BLOCK_AMOUNT; ++i)
				centerBuffers[i].generateCenters(resolution, lod,  i, clipBuffer);

			{
				for(int i = 0; i < 16; ++i)
				{
					fullBuffers[i].indexBuffer.setLogger(logger);
					linkBuffers[i].indexBuffer.setLogger(logger);
					centerBuffers[i].indexBuffer.setLogger(logger);
				}
			}
		}
	};

struct Storm3D_TerrainLodData
{
	Storm3D &storm;
	LOD lodBuffers[LOD_AMOUNT];

	int maxVertex;
	float blockSize;

	Storm3D_TerrainLodData(Storm3D &storm_)
	:	storm(storm_),
		maxVertex(0),
		blockSize(1)
	{
	}

	int getLOD(float range)
	{
		return 0;
		// What's this? -tn
		return LOD_AMOUNT - 1;
	}
};

//! Constructor
Storm3D_TerrainLod::Storm3D_TerrainLod(Storm3D &storm)
{
	boost::scoped_ptr<Storm3D_TerrainLodData> tempData(new Storm3D_TerrainLodData(storm));
	data.swap(tempData);
}

//! Destructor
Storm3D_TerrainLod::~Storm3D_TerrainLod()
{
}

//! Generate LODs
/*!
	\param resolution LOD resolution
	\param clipBuffer pointer to clip buffer
*/
void Storm3D_TerrainLod::generate(int resolution, unsigned char *clipBuffer)
{
	data->maxVertex = (resolution * resolution);
	for(int i = 0; i < LOD_AMOUNT; ++i)
	{
		LOD &lod = data->lodBuffers[i];
		lod.generate(resolution, 1 << i, clipBuffer, data->storm.getLogger());
	}
}

//! Set LOD block radius
/*!
	\param size block size
*/
void Storm3D_TerrainLod::setBlockRadius(float size)
{
	data->blockSize = size;
}

//! Render LODs
/*!
	\param scene scene
	\param subMask
	\param renge
	\param rangeX1
	\param rangeY1
	\param rangeX2
	\param rangeY2
*/
void Storm3D_TerrainLod::render(Storm3D_Scene &scene, int subMask, float range, float rangeX1, float rangeY1, float rangeX2, float rangeY2)
{
	int lod = data->getLOD(range);

	bool lodX1 = data->getLOD(rangeX1) > lod;
	bool lodY1 = data->getLOD(rangeY1) > lod;
	bool lodX2 = data->getLOD(rangeX2) > lod;
	bool lodY2 = data->getLOD(rangeY2) > lod;

	int index = 0;
	if(lodX1)
		index += 1;
	if(lodX2)
		index += 2;
	if(lodY1)
		index += 4;
	if(lodY2)
		index += 8;

	// Render as whole
	if(subMask == -1)
	{
		LodIndexBuffer &indexBuffer = data->lodBuffers[lod].fullBuffers[index];
		// this renders the terrain
		indexBuffer.indexBuffer.render(indexBuffer.faceAmount, data->maxVertex);

		scene.AddPolyCounter(indexBuffer.faceAmount);
	}
	else
	{
		LodIndexBuffer &indexBuffer = data->lodBuffers[lod].centerBuffers[subMask];
		indexBuffer.indexBuffer.render(indexBuffer.faceAmount, data->maxVertex);

		scene.AddPolyCounter(indexBuffer.faceAmount);

		LodIndexBuffer &linkBuffer = data->lodBuffers[lod].linkBuffers[index];
		linkBuffer.indexBuffer.render(linkBuffer.faceAmount, data->maxVertex);

		scene.AddPolyCounter(linkBuffer.faceAmount);
	}
}
