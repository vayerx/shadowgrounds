
#include "precompiled.h"

#include "GameScene.h"

#include <Storm3D_UI.h>
#include "../ui/Terrain.h"
#include "../ui/VisualObject.h"
#include "Unit.h"
#include "Building.h"
#include "GameMap.h"
#include "CoverMap.h"
//#include "HideMap.h"
#include "materials.h"
#include "../util/AreaMap.h"
#include "areamasks.h"
#include "SimpleOptions.h"
#include "options/options_debug.h"
#include "../util/BuildingMap.h"
#include "../util/AI_PathFind.h"
#include "../util/PathSimplifier.h"
#include "../util/fb_assert.h"
#include "../system/Timer.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../ui/LoadingMessage.h"

// HACK: Just to get horrible unit conditional blocking count hack...
#include "UnitActor.h"

#include "../util/Debug_MemoryManager.h"

using namespace ui;

// for perf stats...
int gamescene_raytraces_since_last_clear = 0;
int gamescene_lostraces_since_last_clear = 0;

// HACK: ...
game::GameScene *gameScene_instance = NULL;
const char *default_terrain_material = "sand";
std::string terrain_material = default_terrain_material;

#ifdef PROJECT_AOV
// the minimum terrain obstacle height to actually add as obstacle, used by sideways (AOV) only
#define TERR_OBJ_OBST_HEIGHT_BLOCK_THRESHOLD 1.9f
// and the maximum height (obstacle origin height) over which obstacles cease to block
#define TERR_OBJ_OBST_NO_BLOCK_OVER 2.5f
#else
// the minimum terrain obstacle height to actually add as obstacle, used by sideways (AOV) only
#define TERR_OBJ_OBST_HEIGHT_BLOCK_THRESHOLD 0.1f
// and the maximum height (obstacle origin height) over which obstacles cease to block
#define TERR_OBJ_OBST_NO_BLOCK_OVER 99999.9f
#endif

#define MAX_RAYTRACE_DEBUG_LINES 256

namespace game
{

	IStorm3D_Line *raytrace_debug_lines[MAX_RAYTRACE_DEBUG_LINES] = { NULL };
	int next_raytrace_debug_line = 0;


	GameScene::GameScene(IStorm3D *storm3D, IStorm3D_Scene *stormScene,
		Terrain *terrain, GameMap *gameMap)
	{
		this->storm3D = storm3D;
		this->stormScene = stormScene;
		this->terrain = terrain;
		this->stormTerrain = terrain->GetTerrain();
		this->gameMap = gameMap;

		// notice: getting the heightmap in a bad manner here...
		pathFinder = new frozenbyte::ai::PathFind();
		pathFinder->setHeightMap(gameMap->pathfindHeightMap, 
			gameMap->getHeightmapSizeX(), gameMap->getHeightmapSizeY(), 
			GAMEMAP_PATHFIND_ACCURACY);
		pathFinder->setHeuristicWeight(2.1f);
		pathFinder->setCoverMap(gameMap->getCoverMap());
		pathFinder->setLightMap(gameMap->lightMap);

#ifdef DUMP_GAMESCENE_STATS
		statPathfindFailedAmount = 0;

		statLOStracePartialAmount = 0;
		statLOStraceToTerrainAmount = 0;
		statLOStraceToUnitAmount = 0;
		statLOStraceToObstacleAmount = 0;

		statRaytraceToTerrainAmount = 0;
		statRaytraceToUnitAmount = 0;
		statRaytraceToObstacleAmount = 0;

		statPathfindAmount = 0;
		statRaytraceAmount = 0;
		statLOStraceAmount = 0;
		statLongPathfindAmount = 0;
		statLongRaytraceAmount = 0;
		statLongLOStraceAmount = 0;
		statPathfindTime = 0;
		statRaytraceTime = 0;
		statLOStraceTime = 0;
		statStartTime = Timer::getTime();
#endif

		for (int i = 0; i < MAX_RAYTRACE_DEBUG_LINES; i++)
		{
			raytrace_debug_lines[i] = NULL;
		}
		next_raytrace_debug_line = 0;

		// HACK: ...
		gameScene_instance = this;
	}


	GameScene::~GameScene()
	{
		// HACK: ...
		gameScene_instance = NULL;

		if (pathFinder != NULL)
			delete pathFinder;

		for (int i = 0; i < MAX_RAYTRACE_DEBUG_LINES; i++)
		{
			if (raytrace_debug_lines[i] != NULL)
			{
				ui::VisualObjectModel::visualStormScene->RemoveLine(raytrace_debug_lines[i]);
				while (raytrace_debug_lines[i]->GetPointCount() > 0)
				{
					raytrace_debug_lines[i]->RemovePoint(0);
				}
				delete raytrace_debug_lines[i];
				raytrace_debug_lines[i] = NULL;
			}
		}

#ifdef DUMP_GAMESCENE_STATS
		int totalTime = Timer::getTime() - statStartTime;
		if (totalTime == 0)
			totalTime = 1;

		Logger::getInstance()->debug("Time spent while game initialized:");
		Logger::getInstance()->debug(int2str(totalTime / 1000));

		Logger::getInstance()->debug("Pathfind amount:");
		Logger::getInstance()->debug(int2str(statPathfindAmount));
		Logger::getInstance()->debug("Pathfind failed amount:");
		Logger::getInstance()->debug(int2str(statPathfindFailedAmount));
		Logger::getInstance()->debug("Long pathfind amount:");
		Logger::getInstance()->debug(int2str(statLongPathfindAmount));
		Logger::getInstance()->debug("Long pathfind avg time (msec):");
		if (statLongPathfindAmount > 0)
			Logger::getInstance()->debug(int2str((int)(statPathfindTime / statLongPathfindAmount)));
		else
			Logger::getInstance()->debug("n/a");
		Logger::getInstance()->debug("Pathfinds per second:");
		Logger::getInstance()->debug(int2str(1000 * statPathfindAmount / totalTime));

		Logger::getInstance()->debug("LOStrace amount:");
		Logger::getInstance()->debug(int2str(statLOStraceAmount));
		Logger::getInstance()->debug("LOStrace to unit amount:");
		Logger::getInstance()->debug(int2str(statLOStraceToUnitAmount));
		Logger::getInstance()->debug("LOStrace to terrain amount:");
		Logger::getInstance()->debug(int2str(statLOStraceToTerrainAmount));
		Logger::getInstance()->debug("LOStrace to obstacle amount:");
		Logger::getInstance()->debug(int2str(statLOStraceToObstacleAmount));
		Logger::getInstance()->debug("Partial LOStrace amount:");
		Logger::getInstance()->debug(int2str(statLOStracePartialAmount));
		Logger::getInstance()->debug("Long LOStrace amount:");
		Logger::getInstance()->debug(int2str(statLongLOStraceAmount));
		Logger::getInstance()->debug("Long LOStrace avg time (msec):");
		if (statLongLOStraceAmount > 0)
			Logger::getInstance()->debug(int2str((int)(statLOStraceTime / statLongLOStraceAmount)));
		else
			Logger::getInstance()->debug("n/a");
		Logger::getInstance()->debug("LOStraces per second:");
		Logger::getInstance()->debug(int2str(1000 * statLOStraceAmount / totalTime));

		Logger::getInstance()->debug("Raytrace amount:");
		Logger::getInstance()->debug(int2str(statRaytraceAmount));
		Logger::getInstance()->debug("Raytrace to unit amount:");
		Logger::getInstance()->debug(int2str(statRaytraceToUnitAmount));
		Logger::getInstance()->debug("Raytrace to terrain amount:");
		Logger::getInstance()->debug(int2str(statRaytraceToTerrainAmount));
		Logger::getInstance()->debug("Raytrace to obstacle amount:");
		Logger::getInstance()->debug(int2str(statRaytraceToObstacleAmount));
		Logger::getInstance()->debug("Long raytrace amount:");
		Logger::getInstance()->debug(int2str(statLongRaytraceAmount));
		Logger::getInstance()->debug("Long raytrage avg time (msec):");
		if (statLongRaytraceAmount > 0)
			Logger::getInstance()->debug(int2str((int)(statRaytraceTime / statLongRaytraceAmount)));
		else
			Logger::getInstance()->debug("n/a");
		Logger::getInstance()->debug("Raytraces per second:");
		Logger::getInstance()->debug(int2str(1000 * statRaytraceAmount / totalTime));
#endif
	}



	void GameScene::rayTrace(const VC3 &origin, const VC3 &direction, float rayLength, GameCollisionInfo &cinfo, bool accurate, bool loscheck, bool terrainOnly, bool terrainOnlyForReal)
	{
#ifdef DUMP_GAMESCENE_STATS
		Timer::update();
		int startTime = Timer::getTime();
#endif

#ifndef PHYSICS_NONE
		//PSDHAX
		if (!terrainOnlyForReal)
		{
			terrainOnly = false;
		}
#endif

		if (loscheck)
		{
			gamescene_lostraces_since_last_clear++;
		} else {
			gamescene_raytraces_since_last_clear++;
		}

		// no raytrace if length zero (or negative?)
		if (rayLength <= 0.0f)
		{
			cinfo.hit = false;
			return;
		}

		Storm3D_CollisionInfo sceneColl;
		Storm3D_CollisionInfo terrainColl;
		ObstacleCollisionInfo obstacleColl;

		// must not already be a used collisioninfo.
		assert(!cinfo.hit);

#ifdef PROJECT_CLAW_PROTO
		// HACK: ...
		stormTerrain->rayTrace(origin, direction, 
			rayLength, terrainColl, obstacleColl, true, true);
#else
		stormTerrain->rayTrace(origin, direction, 
			rayLength, terrainColl, obstacleColl, true, loscheck);
#endif

		// if this is los check, we can just fail in case of terrain hit
		// (last 4 meters don't count, because of possible inaccuracy)
		if (loscheck && terrainColl.hit
			&& terrainColl.range < rayLength - 4)
		{
			cinfo.hit = true;
			cinfo.hitGround = true;
			cinfo.hitUnit = false;
			cinfo.range = terrainColl.range;
			cinfo.position = terrainColl.position;
			cinfo.hitPlaneNormal = terrainColl.plane_normal;
#ifdef DUMP_GAMESCENE_STATS
			statLOStraceAmount++;
			Timer::update();
			int usedTime = Timer::getTime() - startTime;
			if (usedTime > 10)
			{
				statLongLOStraceAmount++;
				statLOStraceTime += usedTime;
			}
			statLOStracePartialAmount++;
			statLOStraceToTerrainAmount++;
#endif
			return;
		}
		// in case of los check we must collide at least 3 obstacles for it 
		// to block the view. Also, they must be at least in range of 15 meters.
		// any obstacle closer to that won't have any effect.
		// - this crap just won't apply to buildings and obstacles like that
		// sure, it may be okay for trees, but not for others

		if (loscheck && obstacleColl.hit
			&& obstacleColl.hitAmount >= 1)
		{
			int obstacles = 0;
			//if (obstacleColl.hitAmount <= MAX_OBSTACLE_COLLISIONS)
			// wtf, the above check would have always been true??? should be like this instead
			if (obstacleColl.hitAmount < MAX_OBSTACLE_COLLISIONS)
			{
				// less than 8 hits, check their range
				for (int i = 0; i < obstacleColl.hitAmount; i++)
				{
					//if (obstacleColl.ranges[i] >= 15)
					//if (obstacleColl.ranges[i] >= 2)
					//if (obstacleColl.ranges[i] >= 0.5f)
					if (obstacleColl.ranges[i] >= 0.25f)
					{ 					
						obstacles++;
						if (obstacles == 1)
						{
							cinfo.range = obstacleColl.ranges[i];
							cinfo.hitPlaneNormal = obstacleColl.plane_normals[i];
						}
					}
				}
			} else {
				// 8 or more hits, that should be more than enough
				obstacles = obstacleColl.hitAmount;
				cinfo.range = obstacleColl.ranges[0];
				cinfo.hitPlaneNormal = obstacleColl.plane_normals[0];

				// TEST HACK: if we have another hit right after the first one...
				// then take the average position (range) from those??
				if (obstacleColl.hitAmount >= 2
					&& obstacleColl.ranges[1] - obstacleColl.ranges[0] < 0.2f)
				{
					cinfo.range = (obstacleColl.ranges[0] + obstacleColl.ranges[1]) / 2.0f;
				}
			}
			if (obstacles >= 1)
			{
				cinfo.position = origin + (direction * cinfo.range);
				cinfo.hit = true;
				cinfo.hitTerrainObject = true;
				cinfo.hitUnit = false;
				//????
				//cinfo.range = terrainColl.range;
				//cinfo.position = terrainColl.position;
#ifdef DUMP_GAMESCENE_STATS
				statLOStraceAmount++;
				Timer::update();
				int usedTime = Timer::getTime() - startTime;
				if (usedTime > 10)
				{
					statLongLOStraceAmount++;
					statLOStraceTime += usedTime;
				}
				statLOStracePartialAmount++;
				statLOStraceToObstacleAmount++;
#endif
				return; 			 
			}
		}

		// skip obstacles closer than 0.5meters, is a further one hit maybe?
		//float minObstacleHitRange = 0.5f;
		float minObstacleHitRange = 0.25f;
		// HACK: small raytraces (used by grenade at least) still collide to t.obstacles 
		// even at very near ranges...
		if (rayLength <= 2.0f)
		{
			minObstacleHitRange = 0.2f;
		}

		bool farObstacleHit = false;
		VC3 farObstaclePlane = VC3(0,1,0);
		VC3 farObstaclePosition = VC3(0,0,0);
		float farObstacleRange = 0;
		if (obstacleColl.hit)
		{
			int checkAmount = MAX_OBSTACLE_COLLISIONS;
			if (obstacleColl.hitAmount < checkAmount) 
				checkAmount = obstacleColl.hitAmount;
			for (int i = 0; i < checkAmount; i++)
			{
				// first person needs more accurate...
				if (obstacleColl.ranges[i] >= minObstacleHitRange)
				{
					farObstacleHit = true;
					farObstacleRange = obstacleColl.ranges[i];
					farObstaclePlane = obstacleColl.plane_normals[i];
					farObstaclePosition = obstacleColl.positions[i];
					break;
				}
			}
		}

		// if terrain collision, set scene raylength to terrain hit's range
		float sceneRayLength = rayLength;
		if (terrainColl.hit)
		{
			// 2 meters extra just to be safe.
			if (terrainColl.range < rayLength - 2)
				sceneRayLength = terrainColl.range + 2;
		}

		if(!terrainOnly)
		{
 			stormScene->RayTrace(origin, direction, 
				sceneRayLength, sceneColl, accurate);
		}

		// horrible if mixture... redo before extending...
		float terrHitRange = terrainColl.range;
		if (farObstacleHit)
			terrHitRange = farObstacleRange;

		bool terrAndScene = false;
		if (sceneColl.hit && (terrainColl.hit || farObstacleHit))
		{
			// both collided, choose closest
			// scenecollision given 0.6 meters preference
			terrAndScene = true;
			if (sceneColl.range <= terrHitRange + 0.6f)
			{
				cinfo.hit = true;
				cinfo.hitUnit = true;
				cinfo.hitBuilding = false;
				cinfo.hitGround = false;
				cinfo.hitTerrainObject = false;
				cinfo.hitPlaneNormal = sceneColl.plane_normal;
			} else {
				cinfo.hit = true;
				cinfo.hitUnit = false;
				cinfo.hitBuilding = false;
				if (farObstacleHit)
				{
					cinfo.hitTerrainObject = true;
					cinfo.hitGround = false;
					cinfo.hitPlaneNormal = farObstaclePlane;
				} else {
					cinfo.hitTerrainObject = false;
					cinfo.hitGround = true;
					cinfo.hitPlaneNormal = terrainColl.plane_normal;
				}
			}
		} else {
			if (sceneColl.hit && (!terrainColl.hit && !farObstacleHit))
			{
				cinfo.hit = true;
				cinfo.hitUnit = true;
				cinfo.hitBuilding = false;
				cinfo.hitGround = false;
				cinfo.hitTerrainObject = false;
				cinfo.hitPlaneNormal = sceneColl.plane_normal;
			} else {
				// (obstacle collision is closer than possible terrain collision.)
				if (farObstacleHit)
				{
					cinfo.hit = true;
					cinfo.hitUnit = false;
					cinfo.hitBuilding = false;
					cinfo.hitGround = false;
					cinfo.hitTerrainObject = true;
					cinfo.hitPlaneNormal = farObstaclePlane;
				} else {
					if (!sceneColl.hit && terrainColl.hit)
					{
						cinfo.hit = true;
						cinfo.hitUnit = false;
						cinfo.hitBuilding = false;
						cinfo.hitGround = true;
						cinfo.hitTerrainObject = false;
						cinfo.hitPlaneNormal = terrainColl.plane_normal;
					} else {
						// no collision
						cinfo.hit = false;
					}
				}
			}
		}
		if (cinfo.hitUnit)
		{
			Unit *hitu = NULL;
			/*
			LinkedList *ulist = units->getAllUnits();
			LinkedListIterator iter = LinkedListIterator(ulist);
			while (iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (u->getVisualObject() != NULL)
				{
					if (u->getVisualObject()->model == sceneColl.model)
					{
						hitu = u;
						break;
					}
				}
			}
			*/

			// convert storm model back to visual object and then to unit...
			IStorm3D_Model_Data *d = sceneColl.model->GetCustomData();
			if (d != NULL)
			{
#ifndef NDEBUG
				void *id = d->GetID();
				assert(id == (void *)&ui::visualObjectID);
#endif

				VisualObject *hitvo = (VisualObject *)d;
				IVisualObjectData *d2 = hitvo->getDataObject();
				if (d2 != NULL)
				{
					void *id2 = d2->getVisualObjectDataId();
					if (id2 == (void *)&game::unitDataId)
					{
						hitu = (Unit *)d2;	
					}
					else if (id2 == (void *)&game::buildingDataId)
					{
						hitu = NULL;
						//hitb = (Building *)d2;
						cinfo.hitBuilding = true;
					} else {
						Logger::getInstance()->error("GameScene::rayTrace - Raytrace hit a visual object with unknown data id.");
						assert(0);
					}
				} else {
					Logger::getInstance()->error("GameScene::rayTrace - Raytrace hit a visual object without data id.");
					assert(0);
				}
			} else {
				//PSDHAX
				//Logger::getInstance()->error("GameScene::rayTrace - Raytrace hit a model which is not owned by a visual object.");
				//assert(0);
			}

			if (hitu != NULL)
			{
				// ray hit a unit
				cinfo.range = sceneColl.range;
				cinfo.unit = hitu;
				cinfo.position = sceneColl.position;
				cinfo.part = hitu->getRootPart();
				int slotAmount = cinfo.part->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					Part *subp = cinfo.part->getSubPart(i);
					if (subp != NULL)
					{
						// this is just horrible! redo!
						if (cinfo.part->getType()->getSlotPosition(i) == SLOT_POSITION_HEAD
							&& strcmp(sceneColl.object->GetName(), "Head") == 0)
						{
							cinfo.part = subp;
							break;
						}
						if (cinfo.part->getType()->getSlotPosition(i) == SLOT_POSITION_LEFT_ARM
							&& strcmp(sceneColl.object->GetName(), "LeftArm") == 0)
						{
							cinfo.part = subp;
							break;
						}
						if (cinfo.part->getType()->getSlotPosition(i) == SLOT_POSITION_RIGHT_ARM
							&& strcmp(sceneColl.object->GetName(), "RightArm") == 0)
						{
							cinfo.part = subp;
							break;
						}
						if (cinfo.part->getType()->getSlotPosition(i) == SLOT_POSITION_LEFT_LEG
							&& strcmp(sceneColl.object->GetName(), "LeftLeg") == 0)
						{
							cinfo.part = subp;
							break;
						}
						if (cinfo.part->getType()->getSlotPosition(i) == SLOT_POSITION_RIGHT_LEG
							&& strcmp(sceneColl.object->GetName(), "RightLeg") == 0)
						{
							cinfo.part = subp;
							break;
						}
					}
				}
			} else {
				// did not hit a unit...
				// then maybe it was a building...
				if (cinfo.hitBuilding)
				{
					cinfo.hitUnit = false;
					cinfo.unit = NULL;
					cinfo.part = NULL;
					cinfo.range = sceneColl.range;
					cinfo.position = sceneColl.position;
				} else {
					// hit something else???
					// ignore or hit ground if that was a choice...

					cinfo.hitUnit = false;
					cinfo.unit = NULL;
					cinfo.part = NULL;
					if (terrAndScene)
					{
						if (farObstacleHit)
						{
							cinfo.hitGround = false;
							cinfo.hitTerrainObject = true;
						} else {
							cinfo.hitGround = true;
							cinfo.hitTerrainObject = false;
						}
					} else {
						//psdhax
						//assert(0);
					}
				}
			}
		}
		if (cinfo.hitGround)
		{
			cinfo.position = terrainColl.position;
			cinfo.range = terrainColl.range;
		}
		if (cinfo.hitTerrainObject)
		{
			//cinfo.position = obstacleColl.position;
			//cinfo.range = obstacleColl.range;
			//cinfo.position = origin + direction * farObstacleRange;
			cinfo.position = farObstaclePosition;
			cinfo.range = farObstacleRange;
		}

#ifndef PHYSICS_NONE
		//PSDHAX
		if (!terrainOnlyForReal)
		{
			if(sceneColl.hit && sceneColl.model && sceneColl.model->GetCustomData() == 0)
			{
				cinfo.hit = true;
				cinfo.hitUnit = false;
				cinfo.unit = NULL;
				cinfo.part = NULL;
				cinfo.range = sceneColl.range;
				cinfo.position = sceneColl.position;
				cinfo.hitGround = false;
				cinfo.hitTerrainObject = true;
				cinfo.terrainInstanceId = sceneColl.terrainInstanceId;
				cinfo.terrainModelId = sceneColl.terrainModelId;
			}
		}
#endif

		assert(!cinfo.hit
			|| (cinfo.hit
				&& (cinfo.hitUnit
				|| cinfo.hitBuilding
				|| cinfo.hitTerrainObject
				|| cinfo.hitGround)
			));

// --- HACK, show raytrace with a line ---

		if (SimpleOptions::getBool(DH_OPT_B_SHOW_DEBUG_RAYTRACES))
		{
			int mask = SimpleOptions::getInt(DH_OPT_I_DEBUG_RAYTRACES_TYPE_MASK);
			if (((mask & 1) != 0 && !loscheck) || ((mask & 2) != 0 && loscheck) || ((mask & 4) != 0 && accurate) || ((mask & 8) != 0 && !accurate))
			{
				next_raytrace_debug_line++;
				if (next_raytrace_debug_line >= MAX_RAYTRACE_DEBUG_LINES)
					next_raytrace_debug_line = 0;

				if (raytrace_debug_lines[next_raytrace_debug_line])
				{
					ui::VisualObjectModel::visualStormScene->RemoveLine(raytrace_debug_lines[next_raytrace_debug_line]);
					while (raytrace_debug_lines[next_raytrace_debug_line]->GetPointCount() > 0)
					{
						raytrace_debug_lines[next_raytrace_debug_line]->RemovePoint(0);
					}
					delete raytrace_debug_lines[next_raytrace_debug_line];
				}
			
				IStorm3D_Line *lineObject = ui::VisualObjectModel::visualStorm->CreateNewLine();
				unsigned int alpha = 0x80000000;
				unsigned int color = 0x0000ff00;
				float thickness = 0.4f;
				thickness = 0.03f;
				alpha = 0x30000000;

				if (cinfo.hitBuilding)
				{
					color = 0x00ffff00;
				}
				if (cinfo.hitTerrainObject)
				{
					color = 0x00b0ff00;
				}
				if (cinfo.hitUnit)
				{
					color = 0x00ff0000;
				}
				if (!cinfo.hit)
				{
					alpha = 0x15000000;
				}

				if (loscheck)
				{
					color |= 0x000000ff;
				}

				if (!accurate)
				{
					thickness *= 2.0f;
				}

				lineObject->SetThickness(thickness);
				lineObject->SetColor(color | alpha);

				ui::VisualObjectModel::visualStormScene->AddLine(lineObject, true);
				lineObject->AddPoint(origin);
				VC3 line_endpoint = origin + direction * rayLength;
				if (cinfo.hit)
				{
					line_endpoint = origin + direction * cinfo.range;
				}
				lineObject->AddPoint(line_endpoint);

				raytrace_debug_lines[next_raytrace_debug_line] = lineObject;
			}
		}

// -------


#ifdef DUMP_GAMESCENE_STATS
		if (loscheck)
		{
			statLOStraceAmount++;
			Timer::update();
			int usedTime = Timer::getTime() - startTime;
			if (usedTime > 10)
			{
				statLongLOStraceAmount++;
				statLOStraceTime += usedTime;
			}
			if (cinfo.hitUnit)
				statLOStraceToUnitAmount++;
			if (cinfo.hitGround)
				statLOStraceToTerrainAmount++;
			if (cinfo.hitTerrainObject)
				statLOStraceToObstacleAmount++;
		} else {
			statRaytraceAmount++;
			Timer::update();
			int usedTime = Timer::getTime() - startTime;
			if (usedTime > 10)
			{
				statLongRaytraceAmount++;
				statRaytraceTime += usedTime;
			}
			if (cinfo.hitUnit)
				statRaytraceToUnitAmount++;
			if (cinfo.hitGround)
				statRaytraceToTerrainAmount++;
			if (cinfo.hitTerrainObject)
				statRaytraceToObstacleAmount++;
		}
#endif

	}



	bool GameScene::findPath(frozenbyte::ai::Path *path, float startX, float startY, 
		float endX, float endY, float maxHeightDifference, float climbPenalty, 
		int coverAvoidDistance, int coverBlockDistance, int depth, int lightAvoidAmount)
	{
#ifdef DUMP_GAMESCENE_STATS
		Timer::update();
		int startTime = Timer::getTime();
#endif

#ifndef NDEBUG
		bool startOk = gameMap->isInScaledBoundaries(startX, startY);
		bool endOk = gameMap->isInScaledBoundaries(endX, endY);
		fb_assert(startOk);
		fb_assert(endOk);
#endif

		assert(pathFinder != NULL);
		int sx = gameMap->scaledToPathfindX(startX);
		int sy = gameMap->scaledToPathfindY(startY);
		int ex = gameMap->scaledToPathfindX(endX);
		int ey = gameMap->scaledToPathfindY(endY);
		int maxdiff = (int)(maxHeightDifference * (gameMap->getScaledSizeX() / gameMap->getHeightmapSizeX()) / gameMap->getScaleHeight());
		float cost = climbPenalty * (gameMap->getScaledSizeX() / gameMap->getHeightmapSizeX()) / gameMap->getScaleHeight();

		pathFinder->setCoverAvoidDistance(coverAvoidDistance);
		pathFinder->setCoverBlockDistance(coverBlockDistance);
		pathFinder->setLightAvoidAmount(lightAvoidAmount);
		pathFinder->setPathfindDepthByPercentage(depth);
	
		frozenbyte::ai::Path *tmp = new frozenbyte::ai::Path();

		bool ret = pathFinder->findRoute(tmp, sx, sy, ex, ey, maxdiff, cost, VC3(startX, 0, startY), VC3(endX, 0, endY));

		if (ret)
		{
			// FIXME: this does not work, as nowadays cover avoid is 
			// no longer blocking, it's just movement cost. (i think)
			//if (coverAvoidDistance < 2)
			//	pathFinder->setCoverAvoidDistance(2);
			// replaced with this:
			if (coverBlockDistance < 2)
				pathFinder->setCoverBlockDistance(2);

			frozenbyte::ai::Path *simpl = frozenbyte::ai::PathSimplifier::getSimplifiedPath(pathFinder, tmp, 3, maxdiff);
			if (simpl != NULL)
			{
				for (int i = simpl->getSize() - 1; i >= 0; i--)
				{
					path->addPoint(simpl->getPointX(i), simpl->getPointY(i));
				}
				delete simpl;
			}
			// to disable path simplifier, use this instead...
			/*
			if (tmp != NULL)
			{
				for (int i = tmp->getSize() - 1; i >= 0; i--)
				{
					path->addPoint(tmp->getPointX(i), tmp->getPointY(i));
				}
			}
			*/

		}

		delete tmp;

#ifdef DUMP_GAMESCENE_STATS
		statPathfindAmount++;
		Timer::update();
		int usedTime = Timer::getTime() - startTime;
		if (usedTime > 10)
		{
			statLongPathfindAmount++;
			statPathfindTime += usedTime;
		}
		if (!ret)
			statPathfindFailedAmount++;
#endif
		return ret;
	}



	void GameScene::moveObstacle(int fromX, int fromY, int toX, int toY, int height)
	{
		assert(!"TODO: GameScene::moveObstacle - check this code, possibly should be moving obstacles...");

		assert(pathFinder != NULL);
		if (fromX != toX || fromY != toY)
		{
			pathFinder->removeObstacle(fromX, fromY);
			pathFinder->addObstacle(toX, toY);
			if (height > 0)
			{
				gameMap->removeObstacleHeight(fromX, fromY, height, AREAVALUE_OBSTACLE_TERRAINOBJECT);
				gameMap->addObstacleHeight(toX, toY, height, AREAVALUE_OBSTACLE_TERRAINOBJECT);
			}
		}
	}


	void GameScene::addMovingObstacle(int x, int y, int height)
	{
		assert(pathFinder != NULL);
		if (height > 0)
		{
			pathFinder->addObstacle(x, y);
			gameMap->addMovingObstacleHeight(x, y, height, AREAVALUE_OBSTACLE_NORMAL_UNIT);
		}
	}


	void GameScene::removeMovingObstacle(int x, int y, int height)
	{
		assert(pathFinder != NULL);
		if (height > 0)
		{
			pathFinder->removeObstacle(x, y);
			gameMap->removeMovingObstacleHeight(x, y, height, AREAVALUE_OBSTACLE_NORMAL_UNIT);
		}
	}


	void GameScene::addDoorObstacle(int x, int y, int height)
	{
		assert(pathFinder != NULL);
		if (height > 0)
		{
			pathFinder->addObstacle(x, y);
			gameMap->addMovingObstacleHeight(x, y, height, AREAVALUE_OBSTACLE_DOOR_UNIT);
		}
	}


	void GameScene::removeDoorObstacle(int x, int y, int height)
	{
		assert(pathFinder != NULL);
		if (height > 0)
		{
			pathFinder->removeObstacle(x, y);
			gameMap->removeMovingObstacleHeight(x, y, height, AREAVALUE_OBSTACLE_DOOR_UNIT);
		}
	}


	bool GameScene::isBlockedAtScaled(float x, float y, float height)
	{
		assert(pathFinder != NULL);
		int ox = gameMap->scaledToObstacleX(x);
		int oy = gameMap->scaledToObstacleY(y);
		if (pathFinder->isBlocked(ox, oy))
		{
			if (gameMap->getScaledHeightAt(x, y) 
				+ (float)gameMap->getObstacleHeight(ox, oy) * gameMap->getScaleHeight()
				> height)
			{
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}


	bool GameScene::isBlocked(int x, int y, float height)
	{
		if (getBlockingCount(x, y, height) > 0)
			return true;
		else
			return false;
	}


	int GameScene::getBlockingCount(int x, int y, float height)
	{
		assert(pathFinder != NULL);
		assert(x >= 0 && y >= 0 
			&& x < gameMap->getObstacleSizeX() && y < gameMap->getObstacleSizeY());
		int blockingCount = pathFinder->getBlockingCount(x, y);
		if (blockingCount > 0)
		{
			if ((float)gameMap->getHeightmapHeightAt(x / GAMEMAP_PATHFIND_ACCURACY, 
				y / GAMEMAP_PATHFIND_ACCURACY) * gameMap->getScaleHeight()
				+ (float)gameMap->getObstacleHeight(x, y) * gameMap->getScaleHeight()
				> height)
			{
				return blockingCount;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}


	int GameScene::getConditionalBlockingCountForUnit(int x, int y, float height)
	{
		assert(pathFinder != NULL);
		int blockingCount = pathFinder->getBlockingCount(x, y);
		if (blockingCount > 0)
		{
			// HACK: REAL HACK HERE!!
			/*
			int obstSizeX = gameMap->getObstacleSizeX();
			unsigned short *ohmap = gameMap->getObstacleHeightMap();
			if ((ohmap[x + y * obstSizeX] & (OBSTACLE_MAP_MASK_SEETHROUGH | OBSTACLE_MAP_MASK_UNHITTABLE)) 
				!= (OBSTACLE_MAP_MASK_SEETHROUGH | OBSTACLE_MAP_MASK_UNHITTABLE))
			{
				blockingCount += 1;
				height -= UNIT_OBSTACLE_HEIGHT;
			}
			*/
			// TODO: is this really the same as the old above check??? what if there
			// is a 0 height moving obstacle, does this return true?
			if (!gameMap->isMovingObstacle(x, y))
			{
				blockingCount += 1;
				height -= UNIT_OBSTACLE_HEIGHT;
			}
			// END OF HACK

			if ((float)gameMap->getHeightmapHeightAt(x / GAMEMAP_PATHFIND_ACCURACY, 
				y / GAMEMAP_PATHFIND_ACCURACY) * gameMap->getScaleHeight()
				+ (float)gameMap->getObstacleHeight(x, y) * gameMap->getScaleHeight()
				> height)
			{
				return blockingCount;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}



	void GameScene::modifyTerrainObstaclesImpl(std::vector<TerrainObstacle> &obstacleList, bool add)
	{
		if (add)
			Logger::getInstance()->debug("GameScene::modifyTerrainObstaclesImpl - Adding terrain obstacles, count follows.");
		else
			Logger::getInstance()->debug("GameScene::modifyTerrainObstaclesImpl - Removing terrain obstacles, count follows.");
		Logger::getInstance()->debug(int2str(obstacleList.size()));

		int noneAmount = 0;
		int cylinderAmount = 0;
		int boxAmount = 0;
		int mappedAmount = 0;
		int revisAmount = 0;
		int remapAmount = 0;

		// for caching the last model / buidingmap, so we don't need 
		// to re-create the same buildingmap over and over again.
		const char *lastModelFilename = NULL;
		VisualObjectModel *lastVisualObjectModel = NULL;
		VisualObject *lastVisualObject = NULL;
		frozenbyte::BuildingMap *lastBuildingMap = NULL;
		bool lastModelIsFirethru = false;
		bool lastModelIsSeethru = false;
		bool lastModelIsBreakable = false;
		int lastRotationX = 0;
		int lastRotationY = 0;
		int lastRotationZ = 0;

		// get default material
		//
		int buildingMaterialInPalette = 0;
		for (int mat = 0; mat < MATERIAL_PALETTE_AMOUNT; mat++)
		{
			if (getMaterialByPalette(mat) == MATERIAL_CONCRETE)
			{
				buildingMaterialInPalette = mat;
				break;
			}
		}

		for(unsigned int i = 0; i < obstacleList.size(); ++i)
		{
			TerrainObstacle *to = &obstacleList[i];

			int lastMaterialAtPalette = buildingMaterialInPalette;
			bool materialOk = false;
			for (int i = 0; i < MATERIAL_PALETTE_AMOUNT; i++)
			{
				int j = materialsInUse[i];
				assert(j >= 0 && j < MATERIAL_AMOUNT);
				assert(materialName[j] != NULL);
				if (strcmp(to->material.c_str(), materialName[j]) == 0)
				{
					materialOk = true;
					lastMaterialAtPalette = i;
					break;
				}
			}
			if (!materialOk)
			{
				// set default material
				lastMaterialAtPalette = buildingMaterialInPalette;

				if (!to->material.empty() || SimpleOptions::getBool(DH_OPT_B_MATERIAL_MISSING_WARNING))
				{
// TODO: restore this as warning.
//					Logger::getInstance()->warning("GameScene::modifyTerrainObstaclesImpl - Terrain object material name unknown or not selected in current material palette.");
					Logger::getInstance()->debug("GameScene::modifyTerrainObstaclesImpl - Terrain object material name unknown or not selected in current material palette.");
					Logger::getInstance()->debug(to->modelFilename.c_str());
					Logger::getInstance()->debug(to->material.c_str());
				}
			}

			// HACK: if objects have fallen down, do nothing.
			// FIXME: this really is not correct, but might work most of the time
			// (should look at instance.height, not heightOffset, but notice that instance.height != to->height)
			if (to->heightOffset < -50.0f)
			{
				continue;
			}

			if (to->terrainObstacleType == TERRAIN_OBSTACLE_TYPE_NONE)
			{
				noneAmount++;
				// nop
			}
			else if (to->terrainObstacleType == TERRAIN_OBSTACLE_TYPE_CYLINDER)
			{
				cylinderAmount++;

				float radfloat;
				int radsq;
				int radhalved;
				int toRadInt = (int)(to->radius*2.0f * gameMap->getObstacleSizeX() / gameMap->getScaledSizeX());
				if (toRadInt == 1)
				{
					radfloat = 0.5f;
					radsq = 1;
					radhalved = 0;
				} else {
					radfloat = ((float)toRadInt) / 2.0f;
					radsq = (int)(radfloat * radfloat);
					// area checked is twice as large as needed (notice radius / 2).
					// but inaccuracy of int causes that
					radhalved = toRadInt / 2;
				}

				assert(pathFinder != NULL);
				int ox = gameMap->scaledToObstacleX(to->position.x);
				int oy = gameMap->scaledToObstacleY(to->position.y);

				for (int ry = -radhalved; ry <= radhalved; ry++)
				{
					for (int rx = -radhalved; rx <= radhalved; rx++)
					{
						if (rx * rx + ry * ry <= radsq)
						{
							if (to->height > 0)
							{
								int hmapheight = (int)(to->height/ gameMap->getScaleHeight());

								// FIXME: possible crash here if outside heightmap boundary!!!

								if (add)
								{
									pathFinder->addObstacle(ox + rx, oy + ry);
									gameMap->addObstacleHeight(ox + rx, oy + ry, hmapheight, AREAVALUE_OBSTACLE_TERRAINOBJECT);
									gameMap->getAreaMap()->setAreaValue(ox + rx, oy + ry, AREAMASK_MATERIAL, (lastMaterialAtPalette << AREASHIFT_MATERIAL));
								} else {
									pathFinder->removeObstacle(ox + rx, oy + ry);
									gameMap->removeObstacleHeight(ox + rx, oy + ry, hmapheight, AREAVALUE_OBSTACLE_TERRAINOBJECT);

									if(!gameMap->getObstacleHeight(ox + rx, oy + ry))
										gameMap->getCoverMap()->removeCover(ox + rx, oy + ry);
								}
							}
						}
					}
				}
		
			} // end if type CYLINDER


			else if (to->terrainObstacleType == TERRAIN_OBSTACLE_TYPE_BOX)
			{
				boxAmount++;

				float radfloat;
				int radhalved;
				int toRadInt = (int)(to->radius*2.0f * gameMap->getObstacleSizeX() / gameMap->getScaledSizeX());
				if (toRadInt == 1)
				{
					radfloat = 0.5f;
					radhalved = 0;
				} else {
					radfloat = ((float)toRadInt) / 2.0f;
					// area checked is twice as large as needed (notice radius / 2).
					// but inaccuracy of int causes that
					radhalved = toRadInt / 2;
				}

				assert(pathFinder != NULL);
				int ox = gameMap->scaledToObstacleX(to->position.x);
				int oy = gameMap->scaledToObstacleY(to->position.y);

				int radhalved2 = radhalved;
				if ((toRadInt % 2) == 0)
					radhalved2--;

				for (int ry = -radhalved2; ry <= radhalved; ry++)
				{
					for (int rx = -radhalved2; rx <= radhalved; rx++)
					{
						if (to->height > 0)
						{

							// FIXME: possible crash here if outside heightmap boundary!!!

							int hmapheight = (int)(to->height / gameMap->getScaleHeight());
							if (add)
							{
								pathFinder->addObstacle(ox + rx, oy + ry);
								gameMap->addObstacleHeight(ox + rx, oy + ry, hmapheight, AREAVALUE_OBSTACLE_TERRAINOBJECT);
								gameMap->getAreaMap()->setAreaValue(ox + rx, oy + ry, AREAMASK_MATERIAL, (lastMaterialAtPalette << AREASHIFT_MATERIAL));
							} else {
								pathFinder->removeObstacle(ox + rx, oy + ry);
								gameMap->removeObstacleHeight(ox + rx, oy + ry, hmapheight, AREAVALUE_OBSTACLE_TERRAINOBJECT);
								
								if(!gameMap->getObstacleHeight(ox + rx, oy + ry))
									gameMap->getCoverMap()->removeCover(ox + rx, oy + ry);
							}
						}
					}
				}
		
			} // end if type BOX


			else if (to->terrainObstacleType == TERRAIN_OBSTACLE_TYPE_MAPPED)
			{
				mappedAmount++;

				// Nice COPY & PASTE programming... from the building
				// obstacle maps below... (modified quite a bit though)

				assert(!to->modelFilename.empty());
				// have we the current buildingmap in the cache?
				// if not, set it to cache
				if (lastModelFilename == NULL 
					|| strcmp(lastModelFilename, to->modelFilename.c_str()) != 0)
			 {
					revisAmount++;

					if (lastVisualObject != NULL)
						delete lastVisualObject;
					if (lastVisualObjectModel != NULL)
						delete lastVisualObjectModel;

					// NOTICE: not a copy, a direct pointer!
					// so, don't delete the "to" object until we're done 
					lastModelFilename = to->modelFilename.c_str();
					lastVisualObjectModel = new VisualObjectModel(lastModelFilename);
					lastVisualObject = lastVisualObjectModel->getNewObjectInstance();
					lastRotationX = -99999;
					lastRotationY = -99999;
					lastRotationZ = -99999;
					lastModelIsFirethru = to->fireThrough;
					// TODO: own seethrough flag for this one!!
					lastModelIsSeethru = lastModelIsFirethru;
					lastModelIsBreakable = to->breakable;
				}

				int newRotX = (int)((to->rotation.x * 180.0f/3.1415927f) + 0.5f);
				int newRotY = (int)((to->rotation.y * 180.0f/3.1415927f) + 0.5f);
				int newRotZ = (int)((to->rotation.z * 180.0f/3.1415927f) + 0.5f);

				// LEGACY: round y rotations to 2 deg accuracy
#ifdef PROJECT_SHADOWGROUNDS
				newRotY -= newRotY % 2;
#endif

				newRotX -= newRotX % 4;
				newRotZ -= newRotZ % 4;
				if(newRotX != lastRotationX || newRotY != lastRotationY || newRotZ != lastRotationZ)
				{
					if (lastBuildingMap != NULL)
						delete lastBuildingMap;

					lastRotationX = newRotX;
					lastRotationY = newRotY;
					lastRotationZ = newRotZ;
					lastBuildingMap = new frozenbyte::BuildingMap(to->modelFilename.c_str(), lastVisualObject->model, lastRotationX, lastRotationY, lastRotationZ);

					remapAmount++;
				}

				float x = to->position.x;
				float y = to->position.y;

				float height = to->heightOffset;				
				if (gameMap->isWellInScaledBoundaries(x,y))
				{
					height += gameMap->getScaledHeightAt(x,y);
				}

				const std::vector<std::vector<unsigned char> > &collisionMap = lastBuildingMap->getObstacleMap();
				const std::vector<std::vector<unsigned char> > &heightMap = lastBuildingMap->getHeightMap();

				bool useFloormap = lastBuildingMap->hasFloorHeightMap();
				const std::vector<std::vector<char> > &floorMap = lastBuildingMap->getFloorHeightMap();

				int startMapX = - static_cast<int> (heightMap.size() / 2);
				int startMapY = - static_cast<int> (heightMap[0].size() / 2); // ysize is constant

				int startTargX = gameMap->scaledToObstacleX(x + ((startMapX + 0) * lastBuildingMap->getMapResolution()));
				int startTargY = gameMap->scaledToObstacleY(y + ((startMapY + 0) * lastBuildingMap->getMapResolution()));
				int endTargX = gameMap->scaledToObstacleX(x + ((startMapX + (heightMap.size() - 1)) * lastBuildingMap->getMapResolution()));
				int endTargY = gameMap->scaledToObstacleY(y + ((startMapY + (heightMap[0].size() - 1)) * lastBuildingMap->getMapResolution()));
#ifdef LEGACY_FILES
				// skewed positions (off by 0.5 obstacle blocks)
#else
				// corrected
				/*
				float obstacleScaleX = gameMap->getScaledSizeX() / gameMap->getObstacleSizeX();
				float obstacleScaleY = gameMap->getScaledSizeY() / gameMap->getObstacleSizeY();

				int startTargX = gameMap->scaledToObstacleX(x - obstacleScaleX * 0.5f + ((startMapX + 0) * lastBuildingMap->getMapResolution()));
				int startTargY = gameMap->scaledToObstacleY(y - obstacleScaleY * 0.5f + ((startMapY + 0) * lastBuildingMap->getMapResolution()));
				int endTargX = gameMap->scaledToObstacleX(x - obstacleScaleX * 0.5f + ((startMapX + (heightMap.size() - 1)) * lastBuildingMap->getMapResolution()));
				int endTargY = gameMap->scaledToObstacleY(y - obstacleScaleY * 0.5f + ((startMapY + (heightMap[0].size() - 1)) * lastBuildingMap->getMapResolution()));
				*/
				// maybe this is okay. (the above would just move the error by 1 obst block, this scales it down)
				endTargX -= 1;
				endTargY -= 1;
#endif

				//bool sideways = SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS);
#ifdef GAME_SIDEWAYS
				bool sideways = true;
#else
				bool sideways = false;
#endif

				if (!sideways || to->heightOffset < TERR_OBJ_OBST_NO_BLOCK_OVER)
				{
					for(int oy = startTargY; oy < endTargY; ++oy)
					for(int ox = startTargX; ox < endTargX; ++ox)
					{ 		
						float normedX = float(ox - startTargX) / float(endTargX - startTargX);
						float normedY = float(oy - startTargY) / float(endTargY - startTargY);

						int i = int(normedX * float(heightMap.size()));
						int j = int(normedY * float(heightMap[0].size()));

						assert(i >= 0 && i < (int)heightMap.size());
						assert(j >= 0 && j < (int)heightMap[0].size());

						// This is blocked
						bool blocked = collisionMap[i][j] > 0 && heightMap[i][j] > 0;
						if(blocked) // this test could be moved further so we can write material without blocking
						{
							// Model's origo is in center of this map

							float xPosition = gameMap->obstacleToScaledX(ox);
							float yPosition = gameMap->obstacleToScaledY(oy);

							if (gameMap->isWellInScaledBoundaries(xPosition, yPosition))
							{
								//if (!pathFinder->isBlocked(ox, oy))
								//{
									if (add)
									{
										AREAMAP_DATATYPE mask = AREAVALUE_OBSTACLE_TERRAINOBJECT;
										if (lastModelIsFirethru) 
											mask |= AREAVALUE_OBSTACLE_UNHITTABLE_YES;
										if (lastModelIsSeethru) 
											mask |= AREAVALUE_OBSTACLE_SEETHROUGH_YES;

										int heightval = (int)((float)heightMap[i][j] * lastBuildingMap->getHeightScale() / gameMap->getScaleHeight());

										heightval += (int)(to->heightOffset / gameMap->getScaleHeight());

										if (heightval > 0// && blocked
											&& (!sideways || heightval > (int)(TERR_OBJ_OBST_HEIGHT_BLOCK_THRESHOLD / gameMap->getScaleHeight())))
										{
											pathFinder->addObstacle(ox, oy);
											if (lastModelIsBreakable 
												|| gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_BREAKABLE))
											{
												gameMap->addObstacleHeight(ox, oy, heightval, mask);
											} else {
												// NEW: other than breakables are limited to max height of all
												// overlapping obstacles (not just added on top of each other)
												if (heightval > gameMap->getObstacleHeight(ox, oy))
												{
													int maxaddval = heightval - gameMap->getObstacleHeight(ox, oy);
													gameMap->addObstacleHeight(ox, oy, maxaddval, mask);
												}
											}
										}

										if (lastModelIsBreakable)
										{
											gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_BREAKABLE, AREAVALUE_BREAKABLE_YES);
										}
										gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_MATERIAL, (lastMaterialAtPalette << AREASHIFT_MATERIAL));
									}
									else// if(blocked)
									{
										AREAMAP_DATATYPE mask = AREAVALUE_OBSTACLE_TERRAINOBJECT;
										if (lastModelIsFirethru) 
											mask |= AREAVALUE_OBSTACLE_UNHITTABLE_YES;
										if (lastModelIsSeethru) 
											mask |= AREAVALUE_OBSTACLE_SEETHROUGH_YES;
										pathFinder->removeObstacle(ox, oy);

										int heightval = (int)((float)heightMap[i][j] * lastBuildingMap->getHeightScale() / gameMap->getScaleHeight());

										heightval += (int)(to->heightOffset / gameMap->getScaleHeight());

										// FIXME: isn't this bugged?
										// the above add branch checks that heightval > 0, this does not. should fix it?

										if (!sideways || heightval > (int)(TERR_OBJ_OBST_HEIGHT_BLOCK_THRESHOLD / gameMap->getScaleHeight()))
										{
											gameMap->removeObstacleHeight(ox, oy, heightval, mask);
											//gameMap->removeObstacleHeight(ox, oy, (int)((float)heightMap[i][j] * lastBuildingMap->getHeightScale() / gameMap->getScaleHeight()), mask);

											if(!gameMap->getObstacleHeight(ox, oy))
												gameMap->getCoverMap()->removeCover(ox, oy);
										}
									}
								//}
							}
						}

						// NOTE: floormap is applied only when adding a terrainobject!
						// it cannot be removed after that.
						if (useFloormap && add)
						{
							float xPosition = gameMap->obstacleToScaledX(ox);
							float yPosition = gameMap->obstacleToScaledY(oy);

							if (gameMap->isWellInScaledBoundaries(xPosition, yPosition))
							{
								char floorVal = floorMap[i][j];
								if (floorVal != BUILDINGMAP_NO_FLOOR_BLOCK)
								{
									int hx = gameMap->scaledToHeightmapX(xPosition);
									int hy = gameMap->scaledToHeightmapY(yPosition);
									int floorValInt = (int)(((float)floorVal) * lastBuildingMap->getHeightScale() / gameMap->getScaleHeight());
									int hmapInt = gameMap->getHeightmapHeightAt(hx, hy);
									int heightInt = (int)(height / gameMap->getScaleHeight());
									int objOffset = heightInt - hmapInt;

									int newHmap = hmapInt + objOffset + floorValInt;

									// note: raise floor only, never lower it
									// (because some rocks may be submerged into terrain)
									if (newHmap > hmapInt)
									{
										gameMap->setHeightmapHeightAt(hx, hy, newHmap);
									}

									//float floorScaled = ((float)floorVal) * lastBuildingMap->getHeightScale();
									//float floorDiff = floorScaled - gameMap->getScaledHeightAt(xPosition, yPosition);
									//int floorDiff = ;
									
								}
							}
						}
					}
				}

			} // end if type MAPPED

			else
			{
				assert(!"Unknown terrain obstacle type.");
			}


			// TODO: some sort of property for the obstacle which tells
			// the hiddeness... now just using static values...
/*
			if (!gameMap->isHideMapLoaded())
			{
				// need to create, was not loaded (bin not up-to-date).
				int hidx = gameMap->scaledToObstacleX(to->position.x);
				int hidy = gameMap->scaledToObstacleY(to->position.y);
				gameMap->getHideMap()->addHiddenessToArea(hidx, hidy, 2, HideMap::maxHiddeness, HideMap::HIDDENESS_TYPE_VEGETATION);
				gameMap->getHideMap()->addHiddenessToArea(hidx, hidy, 4, HideMap::maxHiddeness / 2, HideMap::HIDDENESS_TYPE_VEGETATION);
				gameMap->getHideMap()->addHiddenessToArea(hidx, hidy, 6, HideMap::maxHiddeness / 4, HideMap::HIDDENESS_TYPE_VEGETATION);
			}
*/

		}

		if (lastBuildingMap != NULL)
			delete lastBuildingMap;
		if (lastVisualObject != NULL)
			delete lastVisualObject;
		if (lastVisualObjectModel != NULL)
			delete lastVisualObjectModel;


		if (obstacleList.size() > 1)
		{
			Logger::getInstance()->debug("GameScene::modifyTerrainObstaclesImpl - Done, stats follow.");
			Logger::getInstance()->debug("no-collisions");
			Logger::getInstance()->debug(int2str(noneAmount));
			Logger::getInstance()->debug("cylinders");
			Logger::getInstance()->debug(int2str(cylinderAmount));
			Logger::getInstance()->debug("boxes");
			Logger::getInstance()->debug(int2str(boxAmount));
			Logger::getInstance()->debug("mapped");
			Logger::getInstance()->debug(int2str(mappedAmount));
			Logger::getInstance()->debug("visual recreate amount");
			Logger::getInstance()->debug(int2str(revisAmount));
			Logger::getInstance()->debug("buildingmap recreate amount");
			Logger::getInstance()->debug(int2str(remapAmount));
		}

	}

	void GameScene::initTerrainMaterial(void)
	{
		if (gameMap->isObstacleAndAreaMapLoaded())
		{
			// reset default material for next mission
			terrain_material = default_terrain_material;
			return;
		}

		// resolve default terrain material
		int terrain_material_id = -1;
		for (int i = 0; i < MATERIAL_PALETTE_AMOUNT; i++)
		{
			int j = materialsInUse[i];
			assert(j >= 0 && j < MATERIAL_AMOUNT);
			assert(materialName[j] != NULL);
			if (strcmp(terrain_material.c_str(), materialName[j]) == 0)
			{
				terrain_material_id = i;
				break;
			}
		}
		if(terrain_material_id < 0)
		{
			Logger::getInstance()->debug("GameScene::initTerrainMaterial - Terrain material name unknown or not selected in current material palette.");
			Logger::getInstance()->debug(terrain_material.c_str());
			terrain_material_id = 0;
		}
		// fill map with default terrain material
		gameMap->getAreaMap()->fillAreaValue(AREAMASK_MATERIAL, (terrain_material_id << AREASHIFT_MATERIAL));

		// reset default material for next mission
		terrain_material = default_terrain_material;
	}

	void GameScene::addTerrainObstacles(std::vector<TerrainObstacle> &obstacleList)
	{
		if (gameMap->isObstacleAndAreaMapLoaded())
			return;

#ifdef DUMP_GAMESCENE_STATS
		Timer::update();
		int startTime = Timer::getTime();
#endif

		modifyTerrainObstaclesImpl(obstacleList, true);
		// actually unnecessary, called by Game class soon after this...
		//applyObstacleHeightChanges();

#ifdef DUMP_GAMESCENE_STATS
		Timer::update();
		int usedTime = Timer::getTime() - startTime;
		Logger::getInstance()->debug("GameScene::addTerrainObstacles - Time used follows:");
		Logger::getInstance()->debug(int2str(usedTime));
#endif
	}


	void GameScene::removeTerrainObstacles(std::vector<TerrainObstacle> &obstacleList)
	{
		if (gameMap->isObstacleAndAreaMapLoaded())
			return;

		modifyTerrainObstaclesImpl(obstacleList, false);
		// too inefficient, should update only the changed area...
		//gameMap->applyObstacleHeightChanges();
	}


	void GameScene::addBuildingObstacle(Building *b, bool terrainCut)
	{
#ifdef DUMP_GAMESCENE_STATS
		Timer::update();
		int startTime = Timer::getTime();
#endif

		VisualObject *vo = b->getVisualObject();
		if (vo != NULL)
		{
			VC3 pos = b->getPosition();
			float x = pos.x;
			float y = pos.z;
			vo->prepareForRender();

			// heightmap height at the middle, needed to flatten the terrain
			int midhx = gameMap->scaledToHeightmapX(x);
			int midhy = gameMap->scaledToHeightmapY(y);
			int midheight = gameMap->getHeightmapHeightAt(midhx, midhy);
			/*
			int edgeheight = midheight - (int)(1.0f / gameMap->getScaleHeight());
			if (edgeheight < 0) edgeheight = 0;
			int edgeheight2 = midheight - (int)(2.0f / gameMap->getScaleHeight());
			if (edgeheight2 < 0) edgeheight2 = 0;
			int edgeheight3 = midheight - (int)(4.0f / gameMap->getScaleHeight());
			if (edgeheight3 < 0) edgeheight3 = 0;
			*/

			// psd. obstacle maps
			// NOTE: assumes rotation 0 (rotation done with @xx thingy)
			frozenbyte::BuildingMap builder(b->getModelFilename(), vo->model, 0, 0, 0);
			const std::vector<std::vector<unsigned char> > &collisionMap = builder.getObstacleMap();
			const std::vector<std::vector<unsigned char> > &heightMap = builder.getHeightMap();
			const std::vector<std::vector<char> > &floorMap = builder.getFloorHeightMap();

			int startMapX = - static_cast<int> (collisionMap.size() / 2);
			int startMapY = - static_cast<int> (collisionMap[0].size() / 2); // ysize is constant
			//int endMapX = startMapX + collisionMap.size() - 1;
			//int endMapY = startMapX + collisionMap[0].size() - 1;

			// We have to do this for every obstacle map 'pixel' on maps area
			// FIXME: currently done for each collision map pixel

			// Add blocks to pathfind
			float scaledToPathX = gameMap->getPathfindSizeX() / gameMap->getScaledSizeX();
			float scaledToPathY = gameMap->getPathfindSizeY() / gameMap->getScaledSizeY();
			
			frozenbyte::ai::Pathblock pathBlock;
			int xBlockSize = static_cast<int> (scaledToPathX * collisionMap.size() * builder.getMapResolution());
			int yBlockSize = static_cast<int> (scaledToPathY * collisionMap[0].size() * builder.getMapResolution());
			pathBlock.setSize(xBlockSize, yBlockSize);
			pathBlock.setModel(vo->model);

			//int xBlockPosition = (scaledToPathX * x * .5f) - xBlockSize/2;
			//int yBlockPosition = (scaledToPathY * y * .5f) - yBlockSize/2;
			int xBlockPosition = gameMap->scaledToObstacleX(x) - xBlockSize/2;
			int yBlockPosition = gameMap->scaledToObstacleY(y) - yBlockSize/2;					
			pathBlock.setPosition(xBlockPosition, yBlockPosition);
			
			// Doors
			const std::vector<std::pair<int, int> > &doors = builder.getDoors();
			for(unsigned int k = 0; k < doors.size(); ++k)
			{
				int x = static_cast<int> (scaledToPathX * doors[k].first * builder.getMapResolution());
				int y = static_cast<int> (scaledToPathY * doors[k].second * builder.getMapResolution());
				
				pathBlock.addPortal(x + xBlockSize/2, y + yBlockSize/2);
			}
			
			// add building id to pathfind
			if(doors.empty() == false)
				pathFinder->addPathblock(pathBlock);
			else
			{
				std::string foo = b->getModelFilename();
				foo += " has no doors set.";
				
				// Info should really take const char *'s
				Logger::getInstance()->debug(const_cast<char *> (foo.c_str()));
			}

			// NOTE: this appears to be at this late stage in order to add portals, building ids etc. properly 
			// (would be more effective at very early stage, at the beginning of this method)
			if (gameMap->isObstacleAndAreaMapLoaded())
				return;

			WORD *forcemap = terrain->GetForceMap();

			// forcemap size
			int fsizex = gameMap->getHeightmapSizeX();
			int fsizey = gameMap->getHeightmapSizeY();
			// need to save the old forcemap temporarely, so we can clear
			// it during this building addition...
			WORD *oldforcemap = new WORD[fsizex * fsizey];
			for (int ofi = 0; ofi < fsizex * fsizey; ofi++)
			{
				oldforcemap[ofi] = forcemap[ofi];
				forcemap[ofi] = 0;
			}

			// Need this to fix problems with heightmap resolution != obstaclemap resolution
			// as obstacle heights would be incorrect when the heightmap (floormap)
			// is not what it is expected to be) - this is noticeable 
			// near steep floormap changes.
			int obstsizex = gameMap->getObstacleSizeX();
			int obstsizey = gameMap->getObstacleSizeY();
			int *absObstHeightmap = new int[obstsizex * obstsizey];
			int *absCollmap = new int[obstsizex * obstsizey];
			for (int aoi = 0; aoi < obstsizex * obstsizey; aoi++)
			{
				absObstHeightmap[aoi] = 0;
				absCollmap[aoi] = 0;
			}

			int buildingMaterialInPalette = 0;
			for (int mat = 0; mat < MATERIAL_PALETTE_AMOUNT; mat++)
			{
				if (getMaterialByPalette(mat) == MATERIAL_CONCRETE)
				{
					buildingMaterialInPalette = mat;
					break;
				}
			}

			for(int i = 0; i < static_cast<int> (collisionMap.size()); ++i)
			{
				for(int j = 0; j < static_cast<int> (collisionMap[0].size()); ++j)
				{ 		
					float xPosition = x + ((startMapX + i) * builder.getMapResolution());
					float yPosition = y + ((startMapY + j) * builder.getMapResolution());

					if (!gameMap->isWellInScaledBoundaries(xPosition, yPosition))
						continue;
					
					//addObstacle(xPosition, yPosition, BUILDING_OBSTACLE_HEIGHT);
					int ox = gameMap->scaledToObstacleX(xPosition);
					int oy = gameMap->scaledToObstacleY(yPosition);
					
					// This is blocked
					if(collisionMap[i][j] > 0)
					{
						// and height over 0
						if(heightMap[i][j] > 0)
						{
							// Model's origo is in center of this map
							// Resolution is 0.5 units
						
							//if (gameMap->isWellInScaledBoundaries(xPosition, yPosition))
							{
								// NEW BEHAVIOUR: building is now added even on terrainobjects...
								//if (!pathFinder->isBlocked(ox, oy))
								{
									char floorVal = floorMap[i][j];
									if (floorVal == BUILDINGMAP_NO_FLOOR_BLOCK)
										floorVal = 0;
									absObstHeightmap[ox + oy * obstsizex] = (int)(((float)floorVal + (float)heightMap[i][j]) * builder.getHeightScale() / gameMap->getScaleHeight());
									// if firethrough, overwrite make this abs block firethrough only if it is not yet used.
									// solid, non-firethrough blocks won't be changed.
									if (collisionMap[i][j] == 2
										&& absCollmap[ox + oy * obstsizex] == 0)
									{
										absCollmap[ox + oy * obstsizex] = collisionMap[i][j];
									}
									/*
									// delayed, as need to fix heightmap inaccuracy errors
									pathFinder->addObstacle(ox, oy);
									gameMap->addObstacleHeight(ox, oy,
										(int)((float)heightMap[i][j] * builder.getHeightScale() / gameMap->getScaleHeight()),
										AREAVALUE_OBSTACLE_BUILDING);
									*/
									//gameMap->addBuildingObstacleHeight(ox, oy,
									//	(int)((float)heightMap[i][j] * builder.getHeightScale() / gameMap->getScaleHeight()));

									// TODO: optimize, this is not very good, lots of overlapping...
									// TODO: some sort of property for the obstacle which tells
									// the hiddeness... now just using static values...
/*
									if (!gameMap->isHideMapLoaded())
									{
										// need to create, was not loaded (bin not up-to-date).
										gameMap->getHideMap()->addHiddenessToArea(ox, oy, 2, HideMap::maxHiddeness / 3, HideMap::HIDDENESS_TYPE_SOLID);
									}
*/
								}
							}

							// NEW behaviour:
							// If blocked and has height add to block area
							int x = static_cast<int> (scaledToPathX * i * builder.getMapResolution());
							int y = static_cast<int> (scaledToPathY * j * builder.getMapResolution());
						
							//assert(x == ox - xBlockPosition);
							//assert(y == oy - yBlockPosition);
						
							pathBlock.setBlockArea(x, y);

							// add to areamap too, as "inbuilding=yes"
							if (terrainCut)
							{
								// FIXME: should have another mask for terrain cut,
								// should not use the inbuilding mask fot that!
								gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_INBUILDING, AREAVALUE_INBUILDING_YES);
							}
						}
					}
					else
					{
						// If not blocked AND has height add to block area
						if(heightMap[i][j] > 0)
						{
							int x = static_cast<int> (scaledToPathX * i * builder.getMapResolution());
							int y = static_cast<int> (scaledToPathY * j * builder.getMapResolution());
							
							//assert(x == ox - xBlockPosition);
							//assert(y == oy - yBlockPosition);
							
							pathBlock.setBlockArea(x, y);

							// add to areamap too, as "inbuilding=yes"
							if (terrainCut)
							{
								// FIXME: should have another mask for terrain cut,
								// should not use the inbuilding mask fot that!
								gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_INBUILDING, AREAVALUE_INBUILDING_YES);
							}
							gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_MATERIAL, (buildingMaterialInPalette << AREASHIFT_MATERIAL));
						}
					}
					
					//if (heightMap[i][j] > 0 || collisionMap[i][j] > 0)
					char floorVal = floorMap[i][j];
					if (floorVal != BUILDINGMAP_NO_FLOOR_BLOCK)
					{
						if (floorVal < 0)
						{
							/*
							if (i < static_cast<int> (collisionMap.size()) && floorMap[i + 1][j] > floorVal)
								floorVal = floorMap[i + 1][j];
							if (j < static_cast<int> (collisionMap[0].size()) && floorMap[i][j + 1] > floorVal)
								floorVal = floorMap[i][j + 1];
							*/
							if (i > 0 && floorMap[i - 1][j] > floorVal)
								floorVal = floorMap[i - 1][j];
							if (j > 0 && floorMap[i][j - 1] > floorVal)
								floorVal = floorMap[i][j - 1];
							//if (i > 0 && j > 0 && floorMap[i - 1][j - 1] > floorVal)
							//	floorVal = floorMap[i - 1][j - 1];
						} else {
							if (i > 0 && floorMap[i - 1][j] < floorVal)
								floorVal = floorMap[i - 1][j];
							if (j > 0 && floorMap[i][j - 1] < floorVal)
								floorVal = floorMap[i][j - 1];
							//if (i > 0 && j > 0 && floorMap[i - 1][j - 1] < floorVal)
							//	floorVal = floorMap[i - 1][j - 1];
						}
						// else if (floorVal < 0) ...
					}
						
					if (floorVal != BUILDINGMAP_NO_FLOOR_BLOCK)
					{
						// NOTICE: this may be called multiple times per
						// one obstacle block (buildingmap x2 resolution) (?)

						// flatten the map under the building by using the 
						// forcemap... (notice! forcemap is half the size of the
						// heightmap)
						int fox = gameMap->scaledToHeightmapX(xPosition);
						int foy = gameMap->scaledToHeightmapY(yPosition);
						//fox /= 2;
						//foy /= 2;
						int forceval = midheight + 
							(int)(float(floorVal) * builder.getHeightScale() / gameMap->getScaleHeight());

						if (floorVal < 0)
						{
							if ((forcemap[fox + foy * fsizex] > forceval
								|| forcemap[fox + foy * fsizex] == 0)
								&& (oldforcemap[fox + foy * fsizex] > forceval
								|| oldforcemap[fox + foy * fsizex] == 0))
								forcemap[fox + foy * fsizex] = forceval;
						} else {
							if (forcemap[fox + foy * fsizex] < forceval
								&& oldforcemap[fox + foy * fsizex] < forceval)
								forcemap[fox + foy * fsizex] = forceval;
						}

						// NEW BEHAVIOUR: 
						// add floormap to building area id...
						int bax = static_cast<int> (scaledToPathX * i * builder.getMapResolution());
						int bay = static_cast<int> (scaledToPathY * j * builder.getMapResolution());
						
						//assert(x == ox - xBlockPosition);
						//assert(y == oy - yBlockPosition);
						
						pathBlock.setBlockArea(bax, bay);

						// add to areamap too, as "inbuilding=yes"
						if (terrainCut)
						{
							// FIXME: should have another mask for terrain cut,
							// should not use the inbuilding mask fot that!
							gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_INBUILDING, AREAVALUE_INBUILDING_YES);
						}
						gameMap->getAreaMap()->setAreaValue(ox, oy, AREAMASK_MATERIAL, (buildingMaterialInPalette << AREASHIFT_MATERIAL));
					}

				}
			}

			// now apply the forcemap...
			// TODO: check that this radius is even nearly correct...
			float tmpx = xBlockSize * (gameMap->getScaledSizeX() / (float)gameMap->getObstacleSizeX());
			float tmpy = yBlockSize * (gameMap->getScaledSizeX() / (float)gameMap->getObstacleSizeY());
			float forceradius = sqrtf(tmpx * tmpx + tmpy * tmpy);
			VC2 pos2d = VC2(x, y);

			// first cut off too high terrain...
			terrain->ForcemapHeight(pos2d, forceradius, false, true);
			
			// then raise up too low terrain...
			terrain->ForcemapHeight(pos2d, forceradius, true, false);

			// then combine this forcemap with the old forcemap...
			// WARNING: this is not really correct behaviour for negative 
			// floor heights
			for (int ofi2 = 0; ofi2 < fsizex * fsizey; ofi2++)
			{
				if (forcemap[ofi2] < oldforcemap[ofi2])
				{
					forcemap[ofi2] = oldforcemap[ofi2];
				}
			}
			delete [] oldforcemap;

			//for (int aoi2 = 0; aoi2 < obstsizex * obstsizey; aoi2++)
			//{
			for (int oy = 0; oy < obstsizey; oy++)
			{
				for (int ox = 0; ox < obstsizex; ox++)
				{
					if (absObstHeightmap[ox + oy * obstsizex] != 0)
					{
						int hx = ox / GAMEMAP_PATHFIND_ACCURACY;
						int hy = oy / GAMEMAP_PATHFIND_ACCURACY;
						int floorRelHeight = gameMap->getHeightmapHeightAt(hx, hy) - midheight;

						// TEMP: floormap should never make more than about 13m (12.5m) height difference to heightmap...
//						FB_ASSERT(floorRelHeight >= -13.0f / gameMap->getScaleHeight()
//							&& floorRelHeight <= 13.0f / gameMap->getScaleHeight());
						if(floorRelHeight < -13.0f / gameMap->getScaleHeight()
							|| floorRelHeight > 13.0f / gameMap->getScaleHeight())
						{
							Logger::getInstance()->warning("GameScene::addBuildingObstacle - Building floormap makes height difference over limit.");
						}

						int fixedHeight = absObstHeightmap[ox + oy * obstsizex] - floorRelHeight;

						if (fixedHeight > 0)
						{
							// TEMP: buildings should be the first obstacles, and they should not overlap eachother?
							// NEW BEHAVIOUR: buildings may overlap terrainobjects...
							// (as terrainobjects have been added earlier)
							//FB_ASSERT(!pathFinder->isBlocked(ox, oy));
							//if (!pathFinder->isBlocked(ox, oy))
							{
								pathFinder->addObstacle(ox, oy);
								AREAMAP_DATATYPE mask = AREAVALUE_OBSTACLE_BUILDING;
								if (absCollmap[ox + oy * obstsizex] == 2)
								{
									mask |= AREAVALUE_OBSTACLE_UNHITTABLE_YES;
									mask |= AREAVALUE_OBSTACLE_SEETHROUGH_YES;
								}
								gameMap->addObstacleHeight(ox, oy, fixedHeight, mask);
							}
						}
					}
				}
			}
			delete [] absCollmap;
			delete [] absObstHeightmap;

		}

#ifdef DUMP_GAMESCENE_STATS
		Timer::update();
		int usedTime = Timer::getTime() - startTime;
		Logger::getInstance()->debug("GameScene::addBuildingObstacle - Time used follows:");
		Logger::getInstance()->debug(int2str(usedTime));
#endif
	}


	IStorm3D_Model *GameScene::getBuildingModelAtScaled(float x, float y)
	{
		int px = gameMap->scaledToPathfindX(x);
		int py = gameMap->scaledToPathfindY(y);
		IStorm3D_Model *ret = pathFinder->getModelAt(px, py, VC3(x, 0, y));
		return ret;
	}


	IStorm3D_Model *GameScene::getBuildingModelAtPathfind(int x, int y)
	{
		VC3 scaledPos = VC3(gameMap->pathfindToScaledX(x),0,gameMap->pathfindToScaledY(y));
		IStorm3D_Model *ret = pathFinder->getModelAt(x, y, scaledPos);

		// A little trick to remove possible roof flickering inside
		// a building... check the adjacent blocks too...
		// -jpk

		// TODO: a more efficient implementation!

		// NOW THE BULDING ID (BLOCK) MAP BEHAVES DIFFERENTLY
		// IT HAS BLOCKED AREAS TOO, THUS, THIS WOULD SUCK NEAR THE
		// BUILDING'S OUTER WALLS.
		/*
		if (ret == 0 && x > 0)
			if (gameMap->getObstacleHeight(x - 1, y) == 0
				|| gameMap->isMovingObstacle(x - 1, y))
			ret = pathFinder->getModelAt(x - 1, y);
		if (ret == 0 && y > 0)
			if (gameMap->getObstacleHeight(x, y - 1) == 0
				|| gameMap->isMovingObstacle(x, y - 1))
			ret = pathFinder->getModelAt(x, y - 1);
		if (ret == 0 && x < gameMap->getObstacleSizeX()-1)
			if (gameMap->getObstacleHeight(x + 1, y) == 0
				|| gameMap->isMovingObstacle(x + 1, y))
			ret = pathFinder->getModelAt(x + 1, y);
		if (ret == 0 && y < gameMap->getObstacleSizeY()-1)
			if (gameMap->getObstacleHeight(x, y + 1) == 0
				|| gameMap->isMovingObstacle(x, y + 1))
			ret = pathFinder->getModelAt(x, y + 1);
		*/

		return ret;
	}


	// REgenerate actually...
	unsigned char *GameScene::generateTerrainTexturing()
	{
		//stormTerrain->RegenerateTexturing();

		// NOTE: render heightmap size, not collision heightmap size
		// (hmap size / 2)
		int hmapSizeX = gameMap->getHeightmapSizeX() / GAMEMAP_HEIGHTMAP_MULTIPLIER;
		int hmapSizeY = gameMap->getHeightmapSizeY() / GAMEMAP_HEIGHTMAP_MULTIPLIER;

		util::AreaMap *areaMap = gameMap->getAreaMap();
		unsigned char *buf = new unsigned char[hmapSizeX * hmapSizeY];

		int obstPerHmapX = gameMap->getObstacleSizeX() / hmapSizeX;
		int obstPerHmapY = gameMap->getObstacleSizeY() / hmapSizeY;

		for (int y = 0; y < hmapSizeY; y++)
		{
			for (int x = 0; x < hmapSizeX; x++)
			{
				bool anyFloorUnderZero = false;
				int underBuilding = 0;

				for (int ty = 0; ty < obstPerHmapY; ty++)
				{
					for (int tx = 0; tx < obstPerHmapX; tx++)
					{
						int indexX = x * obstPerHmapX + tx - (obstPerHmapX / 2);
						int indexY = y * obstPerHmapY + ty - (obstPerHmapY / 2);
						if (indexX > 0 && indexY > 0)
						{
							if (areaMap->isAreaAnyValue(indexX, indexY, AREAMASK_INBUILDING))
							{
								underBuilding++;
							}
						}
					}
				}
				
				//if (x + y > hmapSizeX / 2 - 10
				//	&& x + y < hmapSizeX / 2 + 10)
				if (underBuilding >= (obstPerHmapX * obstPerHmapY)
					|| anyFloorUnderZero)
				{
					buf[x + y * hmapSizeX] = 1;
				} else {
					buf[x + y * hmapSizeX] = 0;
				}

			}
		}

		SHOW_LOADING_BAR(59);

		stormTerrain->setClipMap(buf);

		//delete [] buf;
		return buf;
	}

	frozenbyte::ai::PathFind *GameScene::getPathFinder()
	{
		return this->pathFinder;
	}

	void GameScene::setTerrainMaterial(const std::string &material)
	{
		terrain_material = material;
	}

}

