
#include "precompiled.h"

#include "DecalSpawner.h"
#include "DecalManager.h"
#include <istorm3d_terrain_decalsystem.h>

using namespace boost;

namespace frozenbyte {

struct DecalSpawner::Data
{
	DecalManager &manager;
	DecalSpawner::Type type;

	int materialId;
	VC2 size;
	float sizeFactor;

	int waitTime;
	int fadeInTime;
	DecalSpawner::FadeType fadeType;

	int fadeOutWaitTime;
	bool forceSpawn;

	Data(DecalManager &manager_, IStorm3D_Material &material)
	:	manager(manager_),
		type(DecalSpawner::Dynamic),
		materialId(-1),
		size(1.f, 1.f),
		sizeFactor(1.f),
		fadeType(DecalSpawner::Blend),
		fadeOutWaitTime(0),
		forceSpawn(false)
	{
		materialId = manager.getDecalSystem().addMaterial(&material);
	}
};

DecalSpawner::DecalSpawner(DecalManager &manager, IStorm3D_Material &material)
{
	scoped_ptr<Data> tempData(new Data(manager, material));
	data.swap(tempData);
}

DecalSpawner::~DecalSpawner()
{
}

void DecalSpawner::setType(Type type)
{
	data->type = type;
}

void DecalSpawner::setSize(const VC2 &size)
{
	data->size = size;
}

void DecalSpawner::setSizeFactor(float factor)
{
	data->sizeFactor = factor;
}

void DecalSpawner::setSpawnProperties(int waitTime, int fadeInTime, FadeType type)
{
	data->waitTime = waitTime;
	data->fadeInTime = fadeInTime;
	data->fadeType = type;
}

void DecalSpawner::setAutoFadeOut(int time)
{
	data->fadeOutWaitTime = time;
}

void DecalSpawner::setForceSpawn(bool forceSpawn)
{
	data->forceSpawn = forceSpawn;
}

void DecalSpawner::spawnDecal(DecalIdentifier &identifier, const VC3 &position, const QUAT &rotation, const COL &light, bool inBuilding)
{
	DecalManager::Type type = DecalManager::Dynamic;
	if(data->type == Static)
		type = DecalManager::Static;

	DecalManager::FadeType fadeType = DecalManager::Blend;
	if(data->fadeType == Scale)
		fadeType = DecalManager::Scale;

	VC2 size = data->size;
	size.y *= data->sizeFactor;

	data->manager.setSpawnProperties(data->waitTime, data->fadeInTime, fadeType);
	data->manager.addDecal(identifier, type, data->materialId, position, size, rotation, light, inBuilding, data->fadeOutWaitTime, data->forceSpawn);
}

} // frozenbyte
