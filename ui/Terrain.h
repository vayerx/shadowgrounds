#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <string>
//#include "../util/Parser.h"
#include <IStorm3D_Model.h>
#include <boost/scoped_ptr.hpp>

#include "../game/tracking/ITrackableUnifiedHandleObjectImplementationManager.h"
#include "../util/GridOcclusionCuller.h"
#include "../game/unified_handle.h"
#include "../util/Parser.h"

// Forward declarations
struct TerrainGroup;
class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Model;
class IStorm3D_Terrain;
typedef unsigned short WORD;

namespace game
{
	class GamePhysics;
	class GameMap;
}

namespace util
{
	class AreaMap;
	class GridOcclusionCuller;
}

namespace ui {
	class LightManager;
	class AmbientSoundManager;
}

#include "DatatypeDef.h"
#include "Storm3D_Common.h"

#define TERRAIN_OBSTACLE_TYPE_NONE 0
#define TERRAIN_OBSTACLE_TYPE_CYLINDER 1
#define TERRAIN_OBSTACLE_TYPE_MAPPED 2
#define TERRAIN_OBSTACLE_TYPE_BOX 3

struct TerrainObstacle
{
	std::string modelFilename;
	std::string material;

	VC2 position;
	VC3 rotation;

	int terrainObstacleType;
	float height;
	float heightOffset;
	float radius;
	bool fireThrough;
	bool breakable;

	TerrainObstacle();
};

struct ExplosionEvent
{
	VC3 position;
	VC3 velocity;
	VC3 rotation;

	VC3 explosionPosition;
	bool useExplosion;

	std::string script;
	std::string projectile;
	std::string effect;
	std::string sound;
	UnifiedHandle unifiedHandle;

	ExplosionEvent()
	:	useExplosion(false),
		unifiedHandle(UNIFIED_HANDLE_NONE)
	{
	}
};

struct TerrainData;

class Terrain : public game::tracking::ITrackableUnifiedHandleObjectImplementationManager
{
	boost::scoped_ptr<TerrainData> data;

public:
	Terrain(IStorm3D *storm, IStorm3D_Scene *scene, const char *dirName, const char *forceMapName, const util::AreaMap *areaMap, game::GameMap *gameMap, ui::LightManager *lightManager, ui::AmbientSoundManager *ambientSoundManager);
	~Terrain();

	IStorm3D_Terrain *GetTerrain();
	bool ValidatePosition(const Vector2D &position, float radius = 1.f) const;

	WORD *GetHeightMap();
	WORD *GetDoubleHeightMap();
	WORD *GetForceMap();

	VC2I getHeightMapSize() const;
	VC2 getTerrainSize() const;
	float getTerrainHeight() const;
	float getModelScale() const;
	float getCameraRange() const;

	const VC3 &getSunDirection() const;
	float getSunlightAmount() const;
	COL getAmbient() const;
	void setAmbient(COL ambientColor);
	void setToColorMultiplier(const COL &color);
	void setToOutdoorColorMultiplier(const COL &color, const VC3 &origo, float radius);

	void getObstacles(std::vector<TerrainObstacle> &obstacles);
	void clearObstacles();
	
	void setLightManager(ui::LightManager *lightManager);
	void createPhysics(game::GamePhysics *gamePhysics, unsigned char *clipMap, bool sleepPhysicsObjects, const char *currentMap);
	void updatePhysics(game::GamePhysics *gamePhysics, std::vector<TerrainObstacle> &objectsMovedFirstTime, util::GridOcclusionCuller *culler);
	void deletePhysics(game::GamePhysics *gamePhysics);
	void releasePhysicsResources(void);

	void loadPhysicsCache(game::GamePhysics *gamePhysics, char *mapFilename);
	void savePhysicsCache(game::GamePhysics *gamePhysics, char *mapFilename);
	void updateAllPhysicsObjectsLighting();

	// For guns
	//void BurnLand(const Vector2D &position, const Vector2D &direction, float length, int max_change, int max_value);
	void BlastHoleCircle(const VC3 &position, const VC3 &direction, float radius, float depth, std::vector<TerrainObstacle> &removedObjects);
	void BlastHoleSphere(const VC3 &position, const VC3 &direction, float radius, float depth, std::vector<TerrainObstacle> &removedObjects);
	//void BlendDamage(const Vector2D &position, float radius, int max_change, int max_value);
	void BlastTerrainObjects(const VC3 &position, float radius, std::vector<TerrainObstacle> &removedObjects, float blastHeight);
	void BreakTerrainObjects(const VC3 &position, const VC3 &velocity, float radius, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, int damage, bool closestOnly, bool only_breaktexture);
	void BreakTerrainObject(UnifiedHandle uh, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, int damage, bool only_breaktexture);

	UnifiedHandle changeObjectTo(UnifiedHandle uh, int newModelId);

	void deleteTerrainObject(UnifiedHandle uh);

	UnifiedHandle createTerrainObject(int newModelId, UnifiedHandle cloneFrom, const VC3 &position, const QUAT &rotation, const VC3 &velocity);

	// returns appropriate model id for given filename or -1 if not available (model of given filename not loaded or something)
	int getModelIdForFilename(const char *filename);

  // HACK: for physics contact damage object destruction...
	bool breakObjects(int modelId, int objectId, int damage, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, const VC2 &position, const VC3 &velocity_, const VC3 &explosion_position, bool use_explosion, bool only_breaktexture);

	void physicsImpulse(const VC3 &position, const VC3 &velocity, float radius, float factor, bool closestOnly);

	UnifiedHandle findClosestContainer(const VC3 &position, float maxRadius);
	UnifiedHandle findClosestTerrainObjectOfMaterial(const VC3 &position, const char *material, float maxRadius);
	UnifiedHandle findTerrainObjectByIdString(const char *idString);

	void ForcemapHeight(const Vector2D &position, float radius, bool above, bool below);
	void AddPaint(const Parser::ParserGroup &parser_group);

	// individual terrain object manipulation...
	UnifiedHandle getUnifiedHandle(int terrainModelId, int terrainObstacleId) const;
	void unifiedHandleToTerrainIds(UnifiedHandle unifiedHandle, int *terrainModelIdOut, int *terrainObstacleIdOut) const;

	bool doesTerrainObjectExist(int terrainModelId, int terrainObstacleId) const;
	VC3 getTerrainObjectPosition(int terrainModelId, int terrainObstacleId) const;
	QUAT getTerrainObjectRotation(int terrainModelId, int terrainObstacleId) const;
	VC3 getTerrainObjectVelocity(int terrainModelId, int terrainObstacleId) const;

	// trackable unified handle interface (ITrackableUnifiedHandleObjectImplementationManager)
	virtual bool doesTrackableUnifiedHandleObjectExist(UnifiedHandle unifiedHandle) const;
	virtual VC3 getTrackableUnifiedHandlePosition(UnifiedHandle unifiedHandle) const;
	virtual QUAT getTrackableUnifiedHandleRotation(UnifiedHandle unifiedHandle) const;
	virtual VC3 getTrackableUnifiedHandleVelocity(UnifiedHandle unifiedHandle) const;
	virtual game::tracking::ITrackableUnifiedHandleObjectIterator *getTrackableUnifiedHandleObjectsFromArea(const VC3 &position, float radius, TRACKABLE_TYPEID_DATATYPE typeMask);
	// ---

	void setAmbientSoundManager(ui::AmbientSoundManager *ambientSoundManager);

	// returns the id string of a terrain object instance, or NULL if the object has no id string.
	// (the returned pointer points to internal data, should make a copy of it, if intended to be
	// referred later on - currently, the pointer should be valid as long as the terrain instance
	// exists, but should not rely on that.)
	const char *getTerrainObjectIdString(UnifiedHandle unifiedHandle) const;

	UnifiedHandle getReplacementForUnifiedHandleObject(UnifiedHandle unifiedHandle) const;

	// note, these take in unified handles, but actually just deal with the model id part, not with instance id...
	bool hasObjectTypeMetaValue(UnifiedHandle unifiedHandle, const std::string &metaKey);
	std::string getObjectTypeMetaValue(UnifiedHandle unifiedHandle, const std::string &metaKey);

	int getObjectVariableNumberByName(const std::string &variableName);
	int getObjectVariableValue(UnifiedHandle unifiedHandle, int variableNumber);
	void setObjectVariableValue(UnifiedHandle unifiedHandle, int variableNumber, int value);

	int getMaterialForObject(int terrainModelId, int terrainObstacleId);

	int getLastObjectEffectTime(int terrainModelId, int terrainObstacleId);
	void setLastObjectEffectTime(int terrainModelId, int terrainObstacleId, int tick);

	void updateOcclusionForAllObjects(util::GridOcclusionCuller *culler, GRIDOCCLUSIONCULLER_DATATYPE cameraArea);
	void updateOcclusionForObject(util::GridOcclusionCuller *culler, int terrainModelId, int terrainObstacleId, GRIDOCCLUSIONCULLER_DATATYPE cameraArea);
	void calculateLighting();

	void updateLighting(const VC3 &position, float radius);

	// Update animations
	void setFogId(const std::string &id);
	void setFogInterpolate(int type);
	void Animate(int time_elapsed); // ms

	void setUseDynamicObstacles(bool dynamicObstacles);
	bool doesUseDynamicObstacles();

	void setInstanceDamageTexture(UnifiedHandle uh, float damageTextureFadeFactor);

};

#endif
