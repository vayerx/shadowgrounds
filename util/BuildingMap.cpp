#include "precompiled.h"

#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <keyb3.h>

#include "BuildingMap.h"
#include <Storm3D_UI.h>

#include "../system/FileTimestampChecker.h"
#include "../convert/str2int.h"

#ifdef BUILDINGMAP_USE_OPTIONS
#include "../game/SimpleOptions.h"
#include "../game/options/options_precalc.h"
#endif
#include "../system/Logger.h"

#ifdef BUILDINGMAP_SHOW_LOADINGMESSAGE
#include "../ui/LoadingMessage.h"
#endif
#include "../editor/align_units.h"

#define BMAP_FLOODFILL_REACHABLE_BYTE 99

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif
#include "../filesystem/input_stream_wrapper.h"

namespace frozenbyte {

// Resolution of maps
//const float mapResolution = 0.5f;
//const float mapResolution = 0.125f;
const float mapResolution = 0.25f;

// Collision spheres properties. Should use more spheres?
// Now 2 spheres -jpk
const float sphereHeight = 0.5f;
const float sphere2Height = 1.3f;
const float sphereRadius = 0.40f; 
//const float sphereSafeArea = 0.2f;
const float sphereSafeArea = 0.2f;

// Height scaling factor.
const float heightScale = 0.1f;

// Collision rays properties
const float rayHeight = 400.f;

#ifdef BUILDINGMAP_SHOW_LOADINGMESSAGE
static char bmap_msgbuf[256];
#endif

#define CURRENT_BUILDINGMAP_VERSION 3



// to assist in floodfilling, implements the required mapper interface.
// - jpk
class BuildingMapFillMapper : public util::IFloodfillByteMapper
{
private:
	BuildingMapData *data;

public:
	BuildingMapFillMapper(BuildingMapData *data);

	virtual unsigned char getByte(int x, int y);
	virtual void setByte(int x, int y, unsigned char value);
};

// and the same for heightmap...
class BuildingMapHeightFillMapper : public util::IFloodfillByteMapper
{
private:
	BuildingMapData *data;

public:
	BuildingMapHeightFillMapper(BuildingMapData *data);

	virtual unsigned char getByte(int x, int y);
	virtual void setByte(int x, int y, unsigned char value);
};

static bool isEscDown()
{
	Keyb3_UpdateDevices();
	return (Keyb3_IsKeyDown(KEYCODE_ESC));
}

struct BuildingMapData
{
	std::vector<std::vector<unsigned char> > collisionMap;
	std::vector<std::vector<unsigned char> > heightMap;
	std::vector<std::vector<char> > floorHeightMap;
	std::vector<std::pair<int, int> > doors;

	bool floorMapExists;
	bool versionWasOld;
	bool negativeHeights;

	// Even numbers, model position on center
	int xResolution;
	int yResolution;

	BuildingMapData()
	{
		xResolution = 0;
		yResolution = 0;
		floorMapExists = false;
		versionWasOld = false;
		negativeHeights = false;
	}

	void calculateSizes(IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ)
	{
		float rotationRadX = rotationX * 3.1415927f / 180.0f;
		float rotationRadY = rotationY * 3.1415927f / 180.0f;
		float rotationRadZ = rotationZ * 3.1415927f / 180.0f;

		float xRadius = 0.f;
		float yRadius = 0.f;

		Vector modelPosition = model->GetPosition();
		Rotation modelRotation = model->GetRotation();

		// Set to identitiy -- why to set rotation here??? -- psd
		model->SetPosition(Vector());
		model->SetRotation(frozenbyte::editor::getRotation(VC3(rotationRadX, rotationRadY, rotationRadZ)));
		//model->SetRotation(Rotation(0, rotationRad, 0));

		Iterator<IStorm3D_Model_Object *> *objectIterator = 0;
		for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			IStorm3D_Mesh *mesh = object->GetMesh();
			
			Matrix positionTm;
			Matrix rotationTm;
			
			// rotate based on given angle
			// -jpk
			/*
			VC3 pos = object->GetPosition();
			float origpx = pos.x;
			pos.x = pos.x * cosf(rotationRad) + pos.z * sinf(rotationRad);
			pos.z = pos.z * cosf(rotationRad) - origpx * sinf(rotationRad);
			QUAT rot = object->GetRotation();
			// TODO: rotate the rot
			*/

			Matrix objectTm = object->GetMXG();

			const Storm3D_Vertex *vertexBuffer = mesh->GetVertexBufferReadOnly();
			for(int j = 0; j < mesh->GetVertexCount(); ++j)
			{
				const Vector &originalV = vertexBuffer[j].position;
				Vector v = objectTm.GetTransformedVector(originalV);

				// Add some extra space
				float x = fabsf(v.x) + 2.f;
				float y = fabsf(v.z) + 2.f;

				if(x > xRadius)
					xRadius = x;

				if(y > yRadius)
					yRadius = y;
			}
		}

		delete objectIterator;

		// Map size 2 * radius + 1
		xResolution = int(xRadius / mapResolution * 2.f + 1.f);
		yResolution = int(yRadius / mapResolution * 2.f + 1.f);

		// Resize
		collisionMap.resize(xResolution);
		heightMap.resize(xResolution);
		floorHeightMap.resize(xResolution);

		for(int i = 0; i < xResolution; ++i)
		{
			collisionMap[i].resize(yResolution);
			heightMap[i].resize(yResolution);
			floorHeightMap[i].resize(yResolution);
		}

		model->SetPosition(modelPosition);
		model->SetRotation(modelRotation);
	}

	void sphereCollision(IStorm3D_Model *model, const VC3 &position, float radius, Storm3D_CollisionInfo &collisionInfo)
	{
		collisionInfo.hit = false;
		collisionInfo.range = static_cast<float> (HUGE);

// TODO: what is this????? is it ok???
model->SphereCollision(position, radius, collisionInfo);
return;

		for(int x = -1; x <= 1; ++x)
		for(int y = -1; y <= 1; ++y)
		{
			VC3 spherePosition = position;
			spherePosition.x += x * sphereSafeArea;
			spherePosition.z += y * sphereSafeArea;

			model->SphereCollision(spherePosition, radius, collisionInfo);
			if(collisionInfo.hit)
			{
				return;
			}
		}
	}

	bool calculateMaps(IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ)
	{
		float rotationRadX = rotationX * 3.1415927f / 180.0f;
		float rotationRadY = rotationY * 3.1415927f / 180.0f;
		float rotationRadZ = rotationZ * 3.1415927f / 180.0f;

		Storm3D_CollisionInfo collisionInfo;
		Vector spherePosition;
		
		Vector rayPosition;
		Vector rayDirection(0.f,-1.f,0.f);

		Vector modelPosition = model->GetPosition();
		Rotation modelRotation = model->GetRotation();

		// Set to identitiy
		model->SetPosition(Vector());
		model->SetRotation(frozenbyte::editor::getRotation(VC3(rotationRadX, rotationRadY, rotationRadZ)));

		for(int i = 0; i < xResolution; ++i)
		{

#ifdef BUILDINGMAP_SHOW_LOADINGMESSAGE
			if (xResolution > 200 && (i % 4) == 0)
			{
				char prog_msgbuf[256+32];
				strcpy(prog_msgbuf, bmap_msgbuf);
				strcat(prog_msgbuf, " -- ");
				strcat(prog_msgbuf, int2str((i * 100) / xResolution));
				ui::LoadingMessage::showLoadingMessage(prog_msgbuf);
			}
#endif
			
			for(int j = 0; j < yResolution; ++j)
			{
				if(isEscDown())
				{
					model->SetPosition(modelPosition);
					model->SetRotation(modelRotation);
					return false;
				}

				spherePosition.x = (-xResolution/2 + i) * mapResolution;
				spherePosition.z = (-yResolution/2 + j) * mapResolution;

				// rotate based on given angle
				// -jpk
				//float origpx = spherePosition.x;
				//spherePosition.x = spherePosition.x * cosf(-rotationRad) + spherePosition.z * sinf(-rotationRad);
				//spherePosition.z = spherePosition.z * cosf(-rotationRad) - origpx * sinf(-rotationRad);

				rayPosition.x = spherePosition.x;
				rayPosition.z = spherePosition.z;

				spherePosition.y = sphereHeight;
				rayPosition.y = rayHeight;

				// if floormap exists, add the floor height... -jpk
				if (floorMapExists)
				{
					if (floorHeightMap[i][j] != BUILDINGMAP_NO_FLOOR_BLOCK)
					{
						spherePosition.y += (float)(floorHeightMap[i][j]) * heightScale;
						// HACK: add 0.2m if on/near elevated surface...
						if (floorHeightMap[i][j] != 0
							|| (i > 0 && floorHeightMap[i - 1][j] != 0)
							|| (i < xResolution && floorHeightMap[i + 1][j] != 0)
							|| (j > 0 && floorHeightMap[i][j - 1] != 0)
							|| (j > yResolution && floorHeightMap[i][j + 1] != 0))
							spherePosition.y += 0.20f;
						//Logger::getInstance()->error(int2str((int)(10 * spherePosition.y)));
					}
				}

				// Blocked -> sphere test 1
				sphereCollision(model, spherePosition, sphereRadius, collisionInfo);
				if(collisionInfo.hit == true)
				{
					// TODO: this is not a very optimal solution!!!
					// (definately should not do a strstr for every friggin map block)
					// (instead should somehow flag those firethrough objects or
					// keeps some small list of pointers to them)
					const char *collObjName = collisionInfo.object->GetName();
					if(strstr(collObjName, "FireThrough") != NULL)
						collisionMap[i][j] = 2;
					else
						collisionMap[i][j] = 1;
				}

				if (!collisionInfo.hit)
				{
					collisionInfo.hit = false;
					collisionInfo.range = float(HUGE);

					// check with 2 spheres instead of one. -jpk
					// Blocked -> sphere test 2
					spherePosition.y = sphere2Height;

					// if floormap exists, add the floor height... -jpk
					if (floorMapExists)
					{
						if (floorHeightMap[i][j] != BUILDINGMAP_NO_FLOOR_BLOCK)
						{
							spherePosition.y += (float)(floorHeightMap[i][j]) * heightScale;
							// HACK: add 0.2m if on elevated surface...
							if (floorHeightMap[i][j] != 0
								|| (i > 0 && floorHeightMap[i - 1][j] != 0)
								|| (i < xResolution && floorHeightMap[i + 1][j] != 0)
								|| (j > 0 && floorHeightMap[i][j - 1] != 0)
								|| (j > yResolution && floorHeightMap[i][j + 1] != 0))
								spherePosition.y += 0.20f;
							//Logger::getInstance()->error(int2str((int)(10 * spherePosition.y)));
						}
					}

					sphereCollision(model, spherePosition, sphereRadius, collisionInfo);
					if(collisionInfo.hit == true)
						collisionMap[i][j] = 1;
				}

				collisionInfo.hit = false;
				collisionInfo.range = float(HUGE);

				// Height -> raytrace down
				// (don't take all the way to zero height, -0.01 for float
				// rounding things..)
				float rayLen = rayHeight;
				if (negativeHeights)
				{
					rayLen = rayHeight * 2;
				}
// TEMP!!!
// raytrace disabled, to avoid "inconsistency" in collisions...
// (only sphere collision used currently, giving more consistent results)
//				model->RayTrace(rayPosition, rayDirection, rayLen, collisionInfo, true);
collisionInfo.hit = false;

				if(collisionInfo.hit == true)
				{
					// note: height now scaled by heightScale.
					// make sure we don't have too high buildings. -jpk
					float obstH = (collisionInfo.position.y / heightScale);

					if (floorMapExists)
					{
						if (floorHeightMap[i][j] != BUILDINGMAP_NO_FLOOR_BLOCK)
						{
							obstH -= (float)(floorHeightMap[i][j]);
						}
					}

					if (negativeHeights)
					{
						if (obstH > 124.0f) 
							obstH = 124.0f;
						if (obstH < -124.0f) 
							obstH = -124.0f;
					} else {
						if (obstH > 250.0f) 
							obstH = 250.0f;
					}

					heightMap[i][j] = (unsigned char) (obstH + 0.5f);

					// floormap cannot be exactly 0, if it is, raise by one...
					if (negativeHeights && heightMap[i][j] == 0)
					{
						heightMap[i][j] = 1;
					}

					if(heightMap[i][j] != 0 || negativeHeights)
						continue;
				} else {
					if (negativeHeights)
					{
						heightMap[i][j] = (unsigned char) (BUILDINGMAP_NO_FLOOR_BLOCK);
					} else {
						// NOTE: added this when adding negative values..
						// assuming this was the default behaviour anyway.
						heightMap[i][j] = 0;
					}
				}

				// Try spheres if no raytrace hit
				// (with negative heights, do this always i think.
				// in other words, raytrace won't collide to negative height floors)
				float kfact = 0.5f;
// TEMP!!!
//				int mink = 0;
				int mink = -30;

				int maxk = 30; // high enough? 30/2 = 15m?

				if (floorMapExists)
				{
					assert(!negativeHeights);
					if (floorHeightMap[i][j] != BUILDINGMAP_NO_FLOOR_BLOCK)
					{
						// FIXME: how about negative heights???
						int tmp = (int)((float)(floorHeightMap[i][j]) * heightScale / kfact);
						if (tmp > 1) mink = tmp - 1;
					}
				}

				if (negativeHeights)
				{
					maxk = 30;
					mink = -30 + 1;
					// TEMP HACK: negative heights (floormap) also uses a more
					// accurate stepping...
					// NOTE: normal 0.5 stepping should work just as well!
					//kfact = 0.20f;
					kfact = 0.5f;
				}
				
				//bool someSphereHit = false;

				for(int k = maxk; k >= mink; --k)
				{
					spherePosition.y = k * kfact;

					sphereCollision(model, spherePosition, sphereRadius, collisionInfo);
					if(collisionInfo.hit)
					{
						//someSphereHit = true;

						float obstH = (collisionInfo.position.y / heightScale);

						if (floorMapExists)
						{
							// floormap itself should not have floorMapExists set
							assert(!negativeHeights); 

							if (floorHeightMap[i][j] != BUILDINGMAP_NO_FLOOR_BLOCK)
							{
								obstH -= (float)(floorHeightMap[i][j]);
							}
						}

						if (negativeHeights)
						{
							if (obstH > 124.0f) 
								obstH = 124.0f;
							if (obstH < -124.0f) 
								obstH = -124.0f;
							if(obstH > char(heightMap[i][j])
								|| char(heightMap[i][j]) == BUILDINGMAP_NO_FLOOR_BLOCK)
								heightMap[i][j] = (unsigned char) (obstH + 0.5f);
							//Logger::getInstance()->error(int2str(k));
						} else {
							if (obstH > 250.0f) 
								obstH = 250.0f;
							if(obstH > heightMap[i][j])
								heightMap[i][j] = (unsigned char) (obstH + 0.5f);
						}

						break;
					}
				}

				//if (k < mink)
				//{
				//	Logger::getInstance()->error("N");
				//	Logger::getInstance()->error(int2str(k));
				//}

				// changed: keep the no_floor_block values (-126)
				// (this way we can differentiate 0 floor heights from non
				// existing floor heights)
				//if (negativeHeights)
				//{
				//	if (heightMap[i][j] == unsigned char(BUILDINGMAP_NO_FLOOR_BLOCK))
				//		heightMap[i][j] = 0;
				//}
			}
		}

		model->SetPosition(modelPosition);
		model->SetRotation(modelRotation);
		return true;
	}

	void findDoors(IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ)
	{
		VC3 modelPosition = model->GetPosition();
		QUAT modelRotation = model->GetRotation();

		std::string idString = "HELPER_MODEL_BuildingEntrance";
		std::string idString2 = "HELPER_MODEL_FloorEntrance";
		float rotationRadX = rotationX * 3.1415927f / 180.0f;
		float rotationRadY = rotationY * 3.1415927f / 180.0f;
		float rotationRadZ = rotationZ * 3.1415927f / 180.0f;

		model->SetPosition(VC3());
		model->SetRotation(frozenbyte::editor::getRotation(VC3(rotationRadX, rotationRadY, rotationRadZ)));

		Iterator<IStorm3D_Helper *> *helperIterator = model->ITHelper->Begin();
		for(; !helperIterator->IsEnd(); helperIterator->Next())
		{
			IStorm3D_Helper *helper = helperIterator->GetCurrent();
			std::string name = helper->GetName();

			bool buildingEntr = false;
			if((name.size() >= idString.size()
				&& name.substr(0, idString.size()) == idString))
				buildingEntr = true;

			bool floorEntr = false;
			if((name.size() >= idString2.size()
				&& name.substr(0, idString2.size()) == idString2))
				floorEntr = true;

			if (buildingEntr || floorEntr)
			{
				Vector position = helper->GetTM().GetTranslation();

				// rotate based on given angle
				// -jpk
				//float origpx = position.x;
				//position.x = position.x * cosf(rotationRad) + position.z * sinf(rotationRad);
				//position.z = position.z * cosf(rotationRad) - origpx * sinf(rotationRad);

				int x = static_cast<int> (position.x / mapResolution);
				int y = static_cast<int> (position.z / mapResolution);

				doors.push_back(std::pair<int, int> (x, y));
			}
		}
	
		delete helperIterator;
		model->SetPosition(modelPosition);
		model->SetRotation(modelRotation);
	}

	// fill the interior parts of the map that are not reachable.
	// - jpk
	void floodfillInteriors()
	{
    // first, the collisionmap
		
		{
			// the mapper object for floodfill.
			BuildingMapFillMapper mapper = BuildingMapFillMapper(this);
    
			// start filling from the corners of the map with byte 2
			if (collisionMap[0][0] == 0)
				collisionMap[0][0] = BMAP_FLOODFILL_REACHABLE_BYTE;
			if (collisionMap[xResolution - 1][0] == 0)
				collisionMap[xResolution - 1][0] = BMAP_FLOODFILL_REACHABLE_BYTE;
			if (collisionMap[0][yResolution - 1] == 0)
				collisionMap[0][yResolution - 1] = BMAP_FLOODFILL_REACHABLE_BYTE;
			if (collisionMap[xResolution - 1][yResolution - 1] == 0)
				collisionMap[xResolution - 1][yResolution - 1] = BMAP_FLOODFILL_REACHABLE_BYTE;

			Logger::getInstance()->debug("BuildingMap::floodfillInteriors - Total number of entrance/door points follows:");
			Logger::getInstance()->debug(int2str((int)doors.size()));

			// and from door points...
			for (int i = 0; i < (int)doors.size(); i++)
			{
				int x = doors[i].first + xResolution / 2;
				int y = doors[i].second + yResolution / 2;
				if (x >= 0 && x < xResolution
					&& y >= 0 && y < yResolution)
				{
					collisionMap[x][y] = BMAP_FLOODFILL_REACHABLE_BYTE;
				} else {
//#ifdef BUILDINGMAP_USE_OPTIONS
					Logger::getInstance()->warning("BuildingMap::floodfillInteriors - Encountered door outside buildingmap area.");
//#endif
				}
			}

			util::Floodfill::fillWithByte(BMAP_FLOODFILL_REACHABLE_BYTE, 0, xResolution, yResolution, &mapper, false, false);
			
			// now we have byte 99 on reachable areas, byte 0 left to interior
			// unreachable areas. now "invert" by converting 0 -> 1 and 2 -> 0.
			
			for(int y = 0; y < yResolution; ++y)
			for(int x = 0; x < xResolution; ++x)
			{
				if (collisionMap[x][y] == 0)
					collisionMap[x][y] = 2;
				else if (collisionMap[x][y] == BMAP_FLOODFILL_REACHABLE_BYTE)
					collisionMap[x][y] = 0;
			}
		}

		// then the heightmap

		{

			// the mapper object for floodfill.
			BuildingMapHeightFillMapper mapper2 = BuildingMapHeightFillMapper(this);
    
			// start filling from the corners of the map with byte 255
			if (heightMap[0][0] == 0)
				heightMap[0][0] = 255;
			if (heightMap[xResolution - 1][0] == 0)
				heightMap[xResolution - 1][0] = 255;
			if (heightMap[0][yResolution - 1] == 0)
				heightMap[0][yResolution - 1] = 255;
			if (heightMap[xResolution - 1][yResolution - 1] == 0)
				heightMap[xResolution - 1][yResolution - 1] = 255;

			// and from door points...
			for (int i = 0; i < (int)doors.size(); i++)
			{
				int x = doors[i].first + xResolution / 2;
				int y = doors[i].second + yResolution / 2;
				if (x >= 0 && x < xResolution
					&& y >= 0 && y < yResolution)
				{
					heightMap[x][y] = 255;
				} else {
#ifdef BUILDINGMAP_USE_OPTIONS
					Logger::getInstance()->warning("BuildingMap::floodfillInteriors - Encountered door outside buildingmap area.");
#endif
				}
			}

			util::Floodfill::fillWithByte(255, 0, xResolution, yResolution, &mapper2, false, false);
			
			// now we have byte 255 on reachable areas, byte 0 left to interior
			// unreachable areas. now "invert" by converting 0 -> 1 and 255 -> 0.
			
			for(int y = 0; y < yResolution; ++y)
			for(int x = 0; x < xResolution; ++x)
			{
				if (heightMap[x][y] == 0 && collisionMap[x][y] != 0)
					heightMap[x][y] = 1;
				else if (heightMap[x][y] == 255)
					heightMap[x][y] = 0;
			}
		}

	}

	bool loadBinary(const std::string &fileName)
	{
		filesystem::FB_FILE *fp = filesystem::fb_fopen(fileName.c_str(), "rb");
		if(fp == 0)
			return false;

		bool hasHeader = false;

		char idbuf[4 + 1];
		filesystem::fb_fread(&idbuf, sizeof(char) * 5, 1, fp);
		idbuf[4] = '\0';
		if (strcmp(idbuf, "BMAP") == 0)
		{
			hasHeader = true;
		} else {
			hasHeader = false;
			//fseek(fp, 0, SEEK_SET);

			filesystem::fb_fclose(fp);
			fp = filesystem::fb_fopen(fileName.c_str(), "rb");
		}

		int versionNum = 0;
		if (hasHeader)
		{
			filesystem::fb_fread(&versionNum, sizeof(int), 1, fp);			
		}

		if (versionNum < CURRENT_BUILDINGMAP_VERSION)
		{
#ifdef BUILDINGMAP_USE_OPTIONS
			if (game::SimpleOptions::getBool(DH_OPT_B_OLD_MODEL_BIN_RECREATE))
			{
				Logger::getInstance()->warning("BuildingMap - Buildingmap version is old so it will be recreated.");
				filesystem::fb_fclose(fp);
				versionWasOld = true;
				return false;
			}
#endif
		}

		int tmp = 0;
		if (versionNum >= 1)
		{
			filesystem::fb_fread(&tmp, sizeof(int), 1, fp);
			if (tmp != 0)
				floorMapExists = true;
			else
				floorMapExists = false;
		} else {
			floorMapExists = false;
		}

		// Size
		filesystem::fb_fread(&xResolution, sizeof(int), 1, fp);
		filesystem::fb_fread(&yResolution, sizeof(int), 1, fp);

		// Resize
		collisionMap.resize(xResolution);
		heightMap.resize(xResolution);
		floorHeightMap.resize(xResolution);

		for(int i = 0; i < xResolution; ++i)
		{
			collisionMap[i].resize(yResolution);
			heightMap[i].resize(yResolution);
			floorHeightMap[i].resize(yResolution);
			for(int j = 0; j < yResolution; ++j)
			{
				floorHeightMap[i][j] = BUILDINGMAP_NO_FLOOR_BLOCK;
			}
		}

		// Read
		for(int y = 0; y < yResolution; ++y)
		for(int x = 0; x < xResolution; ++x)
			filesystem::fb_fread(&collisionMap[x][y], sizeof(unsigned char), 1, fp);

		for(int y = 0; y < yResolution; ++y)
		for(int x = 0; x < xResolution; ++x)
			filesystem::fb_fread(&heightMap[x][y], sizeof(unsigned char), 1, fp);

		if (versionNum >= 1 && floorMapExists)
		{
			for(int y = 0; y < yResolution; ++y)
			for(int x = 0; x < xResolution; ++x)
				filesystem::fb_fread(&floorHeightMap[x][y], sizeof(unsigned char), 1, fp);
		} 
		else 
		{
			for(int y = 0; y < yResolution; ++y)
			for(int x = 0; x < xResolution; ++x)
				floorHeightMap[x][y] = BUILDINGMAP_NO_FLOOR_BLOCK;
		}

		// Doors
		int doorAmount = 0;
		filesystem::fb_fread(&doorAmount, sizeof(int), 1, fp);

		for(int i = 0; i < doorAmount; ++i)
		{
			int x = 0, y = 0;
			filesystem::fb_fread(&x, sizeof(int), 1, fp);
			filesystem::fb_fread(&y, sizeof(int), 1, fp);

			doors.push_back(std::pair<int, int> (x, y));
		}
		
		filesystem::fb_fclose(fp);
		return true;
	}

	void saveBinary(const std::string &fileName)
	{
		FILE *fp = fopen(fileName.c_str(), "wb");
		if(fp == 0)
			return;

		int versionNum = CURRENT_BUILDINGMAP_VERSION;

		if (versionNum >= 1)
		{
			char idbuf[4 + 1] = "BMAP";
			fwrite(&idbuf, sizeof(char) * 5, 1, fp);
			fwrite(&versionNum, sizeof(int), 1, fp);			
		}

		int tmp = 0;
		if (floorMapExists)
			tmp = 1;
		fwrite(&tmp, sizeof(int), 1, fp);

		// Size
		fwrite(&xResolution, sizeof(int), 1, fp);
		fwrite(&yResolution, sizeof(int), 1, fp);

		// Store maps
		for(int y = 0; y < yResolution; ++y)
		for(int x = 0; x < xResolution; ++x)
			fwrite(&collisionMap[x][y], sizeof(unsigned char), 1, fp);

		for(int y = 0; y < yResolution; ++y)
		for(int x = 0; x < xResolution; ++x)
			fwrite(&heightMap[x][y], sizeof(unsigned char), 1, fp);

		if (versionNum >= 1 && floorMapExists)
		{
			for(int y = 0; y < yResolution; ++y)
			for(int x = 0; x < xResolution; ++x)
				fwrite(&floorHeightMap[x][y], sizeof(unsigned char), 1, fp);
		}

		// Doors
		int doorAmount = doors.size();
		fwrite(&doorAmount, sizeof(int), 1, fp);

		for(int i = 0; i < doorAmount; ++i)
		{
			fwrite(&doors[i].first, sizeof(int), 1, fp);
			fwrite(&doors[i].second, sizeof(int), 1, fp);
		}

		fclose(fp);
		return;
	}


	bool calculateFloorHeight(IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ)
	{
		// first check if floor exists...
		//IStorm3D_Model_Object *floor = NULL;
		bool hasFloor = false;
		Iterator<IStorm3D_Model_Object *> *objectIterator;
		for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			const char *name = object->GetName();

			int namelen = strlen(name);

			if(namelen < 13)
				continue;

			// Test name tag
			for(int i = 0; i < namelen - 13 + 1; ++i)
			{
				if (strncmp(&name[i], "BuildingFloor", 13) == 0)
				{
					//floor = object;
					hasFloor = true;
					break;
				}
			}
		}

		delete objectIterator;

		// if there was a floor layer, make all others uncollidable
		if (hasFloor)
		{
			// list for restoring the collisions
			// NOTE: presumes that the object iterator is "stable" and
			// returns the objects in same order every time...
			std::vector<bool> originalCollisionOn;

			for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();

				const char *name = object->GetName();
				int namelen = strlen(name);
				if(namelen < 13)
					continue;

				// Test name tag
				bool isFloor = false;
				for(int i = 0; i < namelen - 13 + 1; ++i)
				{
					if (strncmp(&name[i], "BuildingFloor", 13) == 0)
					{
						isFloor = true;
						break;
					}
				}

				originalCollisionOn.push_back(object->GetNoCollision());

				if (isFloor)
				{
					object->SetNoCollision(false);
				} else {
					object->SetNoCollision(true);
				}
			}

			delete objectIterator;

			// original maps...
			std::vector<std::vector<unsigned char> > origCollisionMap;
			std::vector<std::vector<unsigned char> > origHeightMap;

			// move
			origCollisionMap.resize(xResolution);
			origHeightMap.resize(xResolution);

			int i;
			for(i = 0; i < xResolution; ++i)
			{
				origCollisionMap[i].resize(yResolution);
				origHeightMap[i].resize(yResolution);
				for(int j = 0; j < yResolution; ++j)
				{
					origCollisionMap[i][j] = collisionMap[i][j];
					origHeightMap[i][j] = heightMap[i][j];
					collisionMap[i][j] = 0;
					heightMap[i][j] = 0;
				}				
			}

			// new, floor only map...
			// with negative heights too.. (signed instead of unsigned)
			negativeHeights = true;
			if(!calculateMaps(model, rotationX, rotationY, rotationZ))
				return false;
			negativeHeights = false;

			// floor maps...
			std::vector<std::vector<unsigned char> > floorCollisionMap;

			// yet another copy
			floorCollisionMap.resize(xResolution);

			for(i = 0; i < xResolution; ++i)
			{
				floorCollisionMap[i].resize(yResolution);
				for(int j = 0; j < yResolution; ++j)
				{
					floorCollisionMap[i][j] = collisionMap[i][j];
					floorHeightMap[i][j] = heightMap[i][j];
				}	
			}

			// restore original height/collision maps
			// NOTE: why not restore border areas???
			for(int x = 1; x < xResolution - 1; ++x)
			for(int y = 1; y < yResolution - 1; ++y)
			{
				collisionMap[x][y] = origCollisionMap[x][y];
				heightMap[x][y] = origHeightMap[x][y];
			}

			// finally restore all models collidable
			// FIXME: should restore objects to original nocollision state
			// not all objects are supposed to be collidable.
			int colNum = 0;
			for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				//object->SetNoCollision(false);
				object->SetNoCollision(originalCollisionOn[colNum]);
				colNum++;
			}
			originalCollisionOn.clear();

			delete objectIterator;
		}

		// save the resulting map flag (if the floor existed).
		floorMapExists = hasFloor;
		return true;
	}


	bool calculateWindowShootThrough(IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ)
	{
		// first check if window helper exists...
		//IStorm3D_Model_Object *windowHelper = NULL;
		bool hasWindows = false;
		Iterator<IStorm3D_Model_Object *> *objectIterator;
		for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			const char *name = object->GetName();

			int namelen = strlen(name);

			if(namelen < 19)
				continue;

			// Test name tag
			for(int i = 0; i < namelen - 19 + 1; ++i)
			{
				if (strncmp(&name[i], "_WindowShootThrough", 19) == 0)
				{
					// NOTE: only one window helper allowed!
					//windowHelper = object;
					hasWindows = true;
					break;
				}
			}
		}

		delete objectIterator;

		// if there was a window helper, make all others uncollidable
		if (hasWindows)
		{
			std::vector<bool> originalCollisionOn;

			for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();

				const char *name = object->GetName();
				int namelen = strlen(name);
				if(namelen < 19)
					continue;

				// Test name tag
				bool isWindow = false;
				for(int i = 0; i < namelen - 19 + 1; ++i)
				{
					if (strncmp(&name[i], "_WindowShootThrough", 19) == 0)
					{
						isWindow = true;
						break;
					}
				}

				originalCollisionOn.push_back(object->GetNoCollision());

				if (isWindow)
					object->SetNoCollision(false);
				else
					object->SetNoCollision(true);
			}

			delete objectIterator;

			// original maps...
			std::vector<std::vector<unsigned char> > origCollisionMap;
			std::vector<std::vector<unsigned char> > origHeightMap;

			// move
			origCollisionMap.resize(xResolution);
			origHeightMap.resize(xResolution);

			int i;
			for(i = 0; i < xResolution; ++i)
			{
				origCollisionMap[i].resize(yResolution);
				origHeightMap[i].resize(yResolution);
				for(int j = 0; j < yResolution; ++j)
				{
					origCollisionMap[i][j] = collisionMap[i][j];
					origHeightMap[i][j] = heightMap[i][j];
					collisionMap[i][j] = 0;
					heightMap[i][j] = 0;
				}				
			}

			// new, window helper only map...
			if(!calculateMaps(model, rotationX, rotationY, rotationZ))
				return false;

			// window maps...
			std::vector<std::vector<unsigned char> > windowCollisionMap;
			std::vector<std::vector<unsigned char> > windowHeightMap;

			// yet another copy
			windowCollisionMap.resize(xResolution);
			windowHeightMap.resize(xResolution);

			for(i = 0; i < xResolution; ++i)
			{
				windowCollisionMap[i].resize(yResolution);
				windowHeightMap[i].resize(yResolution);
				for(int j = 0; j < yResolution; ++j)
				{
					windowCollisionMap[i][j] = collisionMap[i][j];
					windowHeightMap[i][j] = heightMap[i][j];
				}				
			}

			// combine the maps...
			for(int x = 1; x < xResolution - 1; ++x)
			for(int y = 1; y < yResolution - 1; ++y)
			{
				if (windowCollisionMap[x][y] == 0
					&& windowHeightMap[x][y] == 0)
				{
					if (windowCollisionMap[x][y - 1] == 0
						&& windowCollisionMap[x][y + 1] == 0
						&& windowCollisionMap[x - 1][y] == 0
						&& windowCollisionMap[x + 1][y] == 0)
					{
						// no window here (or near), use the original map block
						collisionMap[x][y] = origCollisionMap[x][y];
						heightMap[x][y] = origHeightMap[x][y];
					} else {
						// there was a window near, make this one like that window too
						if (windowCollisionMap[x][y - 1] != 0
							&& origHeightMap[x][y] > windowHeightMap[x][y - 1])
						{
							collisionMap[x][y] = windowCollisionMap[x][y - 1];
							heightMap[x][y] = windowHeightMap[x][y - 1];
						} 
						else if (windowCollisionMap[x][y + 1] != 0
							&& origHeightMap[x][y] > windowHeightMap[x][y + 1])
						{
							collisionMap[x][y] = windowCollisionMap[x][y + 1];
							heightMap[x][y] = windowHeightMap[x][y + 1];
						}
						else if (windowCollisionMap[x - 1][y] != 0
							&& origHeightMap[x][y] > windowHeightMap[x - 1][y])
						{
							collisionMap[x][y] = windowCollisionMap[x - 1][y];
							heightMap[x][y] = windowHeightMap[x - 1][y];
						} 
						else if (windowCollisionMap[x + 1][y] != 0
							&& origHeightMap[x][y] > windowHeightMap[x + 1][y])
						{
							collisionMap[x][y] = windowCollisionMap[x + 1][y];
							heightMap[x][y] = windowHeightMap[x + 1][y];
						} else {
							// window near, but this is not wall, so keep it that way
							collisionMap[x][y] = origCollisionMap[x][y];
							heightMap[x][y] = origHeightMap[x][y];
						}
					}
				} else {
					// there was a window, keep the window map block
					// actually no need for this (these maps are the same)
					//collisionMap[x][y] = windowCollisionMap[x][y];
					//heightMap[x][y] = windowHeightMap[x][y];
				}
			}

			// finally restore all models collidable
			// FIXME: should restore objects to original nocollision state
			// not all objects are supposed to be collidable.
			int colNum = 0;
			for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				//object->SetNoCollision(false);
				object->SetNoCollision(originalCollisionOn[colNum]);
				colNum++;
			}
			originalCollisionOn.clear();
			delete objectIterator;
		}

		return true;
	}

};


BuildingMapFillMapper::BuildingMapFillMapper(BuildingMapData *data)
{
	this->data = data;
}

unsigned char BuildingMapFillMapper::getByte(int x, int y)
{
	return data->collisionMap[x][y];
}

void BuildingMapFillMapper::setByte(int x, int y, unsigned char value)
{
	data->collisionMap[x][y] = value;
}


BuildingMapHeightFillMapper::BuildingMapHeightFillMapper(BuildingMapData *data)
{
	this->data = data;
}

unsigned char BuildingMapHeightFillMapper::getByte(int x, int y)
{
	return data->heightMap[x][y];
}

void BuildingMapHeightFillMapper::setByte(int x, int y, unsigned char value)
{
	data->heightMap[x][y] = value;
}


/* BuildingMap */

BuildingMap::BuildingMap(const char *fileNameRaw, IStorm3D_Model *model, int rotationX, int rotationY, int rotationZ)
{
	data = new BuildingMapData();

	// gotta remove the @xx rotation part from filename first...
	// --jpk
#ifndef WIN32
  std::string fileNameStr(fileNameRaw);
  int fileNameLen = fileNameStr.length();
  for(int i=0;i<fileNameLen;++i)
    if (fileNameStr[i] == '\\') fileNameStr[i] = '/';
  const char *fileName = fileNameStr.c_str();
#else
	int fileNameLen = strlen(fileNameRaw);
  const char *fileName = fileNameRaw;
#endif

	int cutpos = 0;
#ifdef LEGACY_FILES
	for (int ri = 0; ri < fileNameLen; ri++)
	{
		if (fileName[ri] == '@')
		{
			cutpos = fileNameLen - ri;
			break;
		}
	}
#endif

	// Binarys file name
	std::string binFileName(fileName);
	binFileName = binFileName.substr(0, binFileName.size() - 4 - cutpos);
	if (cutpos > 0)
	{
		// rather don't re-rotate hard-vertex-rotated models.
		assert(!rotationX && !rotationY && !rotationZ);

		binFileName += "_R_";
		binFileName += &fileName[fileNameLen - cutpos + 1];
	} 
	else 
	{
		while (rotationX < 0) rotationX += 360;
			rotationX += 360;
		rotationX %= 360;
		while (rotationY < 0) rotationY += 360;
			rotationY += 360;
		rotationY %= 360;
		while (rotationZ < 0) rotationZ += 360;
			rotationZ += 360;
		rotationZ %= 360;

		if(rotationX || rotationY || rotationZ)
		{
			binFileName += "_R_";
			binFileName += boost::lexical_cast<std::string> (rotationX);
			binFileName += "_";
			binFileName += boost::lexical_cast<std::string> (rotationY);
			binFileName += "_";
			binFileName += boost::lexical_cast<std::string> (rotationZ);
		}

		/*
		if (rotationDegrees != 0)
		{
			binFileName += "_R_";
			char tmp[32];
			
			// WARNING: unsafe sprintf.
			sprintf(tmp, "%d", rotationDegrees);
			binFileName += tmp;
		}
		*/
	}

	binFileName += ".bin";

	// Check if binary file is up to date...
	// -- jpk
	char *filenameStripped = new char[strlen(fileName) + 1];
	strcpy(filenameStripped, fileName);
	if (cutpos > 0)
	{
		filenameStripped[strlen(fileName) - cutpos] = '\0';
	}
	
	//bool upToDate = FileTimestampChecker::isFileNewerOrAlmostSameThanFile(binFileName.c_str(), filenameStripped);
	bool upToDate = FileTimestampChecker::isFileUpToDateComparedTo(binFileName.c_str(), filenameStripped);

	// what if the file did not even exist? but model did exist -
	// then not up to date.
	filesystem::FB_FILE *f = filesystem::fb_fopen(binFileName.c_str(), "rb");
	if (f != NULL)
	{
		filesystem::fb_fclose(f);
	} else {
		filesystem::FB_FILE *f2 = filesystem::fb_fopen(binFileName.c_str(), "rb");
		if (f2 != NULL)
		{
			filesystem::fb_fclose(f2);
			upToDate = false;
		}
	}

	delete [] filenameStripped;

#ifdef BUILDINGMAP_USE_OPTIONS
	if (!upToDate 
		&& !game::SimpleOptions::getBool(DH_OPT_B_AUTO_MODEL_BIN_RECREATE))
	{
		Logger::getInstance()->warning("BuildingMap - Buildingmap is not up to date, but it will not be recreated (auto recreate off).");
		upToDate = true;
	}
	if (game::SimpleOptions::getBool(DH_OPT_B_FORCE_MODEL_BIN_RECREATE))
	{
		Logger::getInstance()->debug("BuildingMap - Buildingmap is recreated (force recreate on).");
		upToDate = false;
	}
#endif

	bool totalFailure = false;

	// if up to date, try to load it...
	if (upToDate)
	{
		// If found, stop here
		Logger::getInstance()->debug("BuildingMap - About to load binary file.");
		Logger::getInstance()->debug(binFileName.c_str());
		if(data->loadBinary(binFileName) == true)
		{
			Logger::getInstance()->debug("BuildingMap - Load binary done.");
			return;
		}

		if (!data->versionWasOld)
		{
			// oh my.. some strange failure to load the data.
			totalFailure = true;
		} else {
			// if version was old, just continue to recreate the map.
			// (and reset this old version flag)
			data->versionWasOld = false;
		}
	}

	Logger::getInstance()->debug("BuildingMap - About to recalculate data.");
#ifdef BUILDINGMAP_SHOW_LOADINGMESSAGE
	strcpy(bmap_msgbuf, "Recreating binary file: ");
	if (binFileName.length() > 20 && binFileName.length() < 128)
	{
		const char *fname = binFileName.c_str();
		strcat(bmap_msgbuf, "...");
		strcat(bmap_msgbuf, &fname[strlen(fname) - 20]);
	} else {
		strcat(bmap_msgbuf, "...");
	}
	ui::LoadingMessage::showLoadingMessage(bmap_msgbuf);
#endif

	// enable collisions for firethrough layers, as they may be
	// disabled. (as the game likes it that way)
	Iterator<IStorm3D_Model_Object *> *objectIterator;
	for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();

		const char *name = object->GetName();

		if (strstr(name, "FireThrough") != NULL)
		{
			object->SetNoCollision(false);
		}
	}


	Logger::getInstance()->debug("BuildingMap - Calculate sizes.");
	data->calculateSizes(model, rotationX, rotationY, rotationZ);

	// first calculate the floor??
	// -jpk
	bool skipCalculation = false;
	Logger::getInstance()->debug("BuildingMap - Calculate floorheight.");
	if(!data->calculateFloorHeight(model, rotationX, rotationY, rotationZ))
		skipCalculation = true;

	if(!skipCalculation)
	{
		// first calculate the normal obstacle map...
		Logger::getInstance()->debug("BuildingMap - Calculate maps.");
		if(!data->calculateMaps(model, rotationX, rotationY, rotationZ))
			skipCalculation = true;
	}

	if(!skipCalculation)
	{
		// then check if we have a window helper, if so calculate another
		// map using the helper only, then combine the two maps.
		// -jpk
		Logger::getInstance()->debug("BuildingMap - Calculate window shoot through.");
		if(!data->calculateWindowShootThrough(model, rotationX, rotationY, rotationZ))
			skipCalculation = true;
	}

	if(skipCalculation)
	{
		Logger::getInstance()->warning("BuildingMap - Skipped binary creation!.");
		Logger::getInstance()->warning(binFileName.c_str());

		delete data;
		data = new BuildingMapData();
		if(data->loadBinary(binFileName))
			Logger::getInstance()->debug("BuildingMap - Load binary done.");

		return;
	}

	Logger::getInstance()->debug("BuildingMap - Find doors.");
	data->findDoors(model, rotationX, rotationY, rotationZ);

	Logger::getInstance()->debug("BuildingMap - Floodfill interiors.");
	data->floodfillInteriors();

	Logger::getInstance()->debug("BuildingMap - Done calculating data.");

	if (totalFailure)
	{
//#ifdef BUILDINGMAP_USE_OPTIONS
		Logger::getInstance()->error("BuildingMap - Load failed, recreated binary file will not be saved.");
//#endif
		//assert(0);
		return;
	}

	Logger::getInstance()->debug("BuildingMap - About to save binary file.");
	// Save
	data->saveBinary(binFileName);

	Logger::getInstance()->debug("BuildingMap - Save binary done.");
}

BuildingMap::~BuildingMap()
{
	delete data;
}

const std::vector<std::vector<unsigned char> > &BuildingMap::getObstacleMap() const
{
	return data->collisionMap;
}

const std::vector<std::vector<unsigned char> > &BuildingMap::getHeightMap() const
{
	return data->heightMap;
}

const std::vector<std::vector<char> > &BuildingMap::getFloorHeightMap() const
{
	return data->floorHeightMap;
}

const std::vector<std::pair<int, int> > &BuildingMap::getDoors() const
{
	return data->doors;
}

float BuildingMap::getHeightScale() const
{
	return heightScale;  // static variable for all buildingmaps
}

float BuildingMap::getMapResolution() const
{
	return mapResolution;
}

bool BuildingMap::hasFloorHeightMap() const
{
	return data->floorMapExists;
}



} // end of namespace frozenbyte
