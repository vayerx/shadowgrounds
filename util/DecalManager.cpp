
#include "precompiled.h"

#include "DecalManager.h"
#include <istorm3d_terrain_decalsystem.h>
#include <deque>

using namespace std;

namespace frozenbyte {

	
	static const int DECAL_COLLECTION_AMOUNT = 2;

	static int MAX_DECAL_AMOUNT = 500;
	static int SPAWN_WAIT_TIME = 0;
	static int FADE_IN_TIME = 200;
	static int FADE_OUT_TIME = 10000;
	static DecalManager::FadeType FADE_TYPE = DecalManager::Blend;

	struct Decal
	{
		DecalManager::FadeType fadeType;

		int materialId;
		int decalId;
		int id;

		int fadeOutWaitTime;
		int currentTime;

		VC2 size;
		QUAT rotation;
		COL light;

		Decal()
		:	fadeType(DecalManager::Blend),
			materialId(-1),
			decalId(-1),
			id(0),
			fadeOutWaitTime(0),
			currentTime(0)
		{
		}	
	};

	struct FadeDecal
	{
		Decal decal;
		int fadeTime;
		int currentTime;

		FadeDecal()
		:	fadeTime(0),
			currentTime(0)
		{
		}

		float getFadeFactor() const
		{
			return float(currentTime) / float(fadeTime);
		}
	};

	struct WaitFadeDecal
	{
		FadeDecal fadeDecal;
		int waitTime;
		int currentTime;

		WaitFadeDecal()
		:	waitTime(0),
			currentTime(0)
		{
		}
	};

	typedef deque<Decal> DecalList;
	typedef deque<FadeDecal> FadeDecalList;
	typedef deque<WaitFadeDecal> WaitFadeDecalList;

	struct DecalCollection
	{
		WaitFadeDecalList waitFadeInDecals;
		FadeDecalList fadeInDecals;
		DecalList decals;
		FadeDecalList fadeOutDecals;

		void add(const Decal &decal)
		{
			FadeDecal fadeDecal;
			fadeDecal.decal = decal;
			fadeDecal.currentTime = 0;
			fadeDecal.fadeTime = FADE_IN_TIME;

			if(SPAWN_WAIT_TIME == 0)
			{
				fadeInDecals.push_back(fadeDecal);
				return;
			}

			WaitFadeDecal waitFadeDecal;
			waitFadeDecal.waitTime = SPAWN_WAIT_TIME;
			waitFadeDecal.fadeDecal = fadeDecal;
			waitFadeInDecals.push_back(waitFadeDecal);
		}

		void updateWaitFadeInDecals(IStorm3D_TerrainDecalSystem &system, int timeDelta)
		{
			WaitFadeDecalList::iterator it = waitFadeInDecals.begin();
			while(it != waitFadeInDecals.end())
			{
				WaitFadeDecal &waitFadeDecal = *it;
				waitFadeDecal.currentTime += timeDelta;

				if(waitFadeDecal.currentTime >= waitFadeDecal.waitTime)
				{
					fadeInDecals.push_back(waitFadeDecal.fadeDecal);
					it = waitFadeInDecals.erase(it);

					continue;
				}

				++it;
			}
		}

		void updateFadeInDecals(IStorm3D_TerrainDecalSystem &system, int timeDelta)
		{
			FadeDecalList::iterator it = fadeInDecals.begin();
			while(it != fadeInDecals.end())
			{
				FadeDecal &fadeDecal = *it;
				fadeDecal.currentTime += timeDelta;
				Decal &decal = fadeDecal.decal;

				if(fadeDecal.currentTime >= fadeDecal.fadeTime)
				{
					float alpha = 1.f;
					system.setAlpha(decal.materialId, decal.decalId, decal.id, alpha);
					system.setSize(decal.materialId, decal.decalId, decal.id, decal.size);

					decals.push_back(decal);
					it = fadeInDecals.erase(it);

					continue;
				}

				if(decal.fadeType == DecalManager::Blend)
				{
					float alpha = fadeDecal.getFadeFactor();
					system.setAlpha(decal.materialId, decal.decalId, decal.id, alpha);
				}
				else
				{
					VC2 size = decal.size;
					size *= fadeDecal.getFadeFactor();

					system.setSize(decal.materialId, decal.decalId, decal.id, size);
				}

				++it;
			}
		}

		void updateFadeOutDecals(IStorm3D_TerrainDecalSystem &system, int timeDelta)
		{
			FadeDecalList::iterator it = fadeOutDecals.begin();
			while(it != fadeOutDecals.end())
			{
				FadeDecal &fadeDecal = *it;
				fadeDecal.currentTime += timeDelta;
				Decal &decal = fadeDecal.decal;

				if(fadeDecal.currentTime >= fadeDecal.fadeTime)
				{
					system.setAlpha(decal.materialId, decal.decalId, decal.id, 0.f);
					system.eraseDecal(decal.materialId, decal.decalId, decal.id);
					it = fadeOutDecals.erase(it);

					continue;
				}

				float alpha = 1.f - fadeDecal.getFadeFactor();
				system.setAlpha(decal.materialId, decal.decalId, decal.id, alpha);

				++it;
			}
		}

		void updateAutoFadeOutDecals(IStorm3D_TerrainDecalSystem &system, int timeDelta)
		{
			DecalList::iterator it = decals.begin();
			while(it != decals.end())
			{
				Decal &decal = *it;
				if(decal.fadeOutWaitTime)
				{
					decal.currentTime += timeDelta;
					if(decal.currentTime >= decal.fadeOutWaitTime)
					{
						FadeDecal fadeDecal;
						fadeDecal.decal = decal;
						fadeDecal.currentTime = 0;
						fadeDecal.fadeTime = FADE_OUT_TIME;
						fadeOutDecals.push_back(fadeDecal);

						it = decals.erase(it);
						continue;
					}
				}

				++it;
			}
		}

		void updateNormalDecals(IStorm3D_TerrainDecalSystem &system)
		{
			int removeAmount = decals.size() - MAX_DECAL_AMOUNT;
			if(removeAmount <= 0)
				return;

			for(int i = 0; i < removeAmount; ++i)
			{
				FadeDecal fadeDecal;
				fadeDecal.decal = decals.front();
				fadeDecal.currentTime = 0;
				fadeDecal.fadeTime = FADE_OUT_TIME;

				fadeOutDecals.push_back(fadeDecal);
				decals.pop_front();
			}
		}

		void update(IStorm3D_TerrainDecalSystem &system, int timeDelta)
		{
			updateWaitFadeInDecals(system, timeDelta);
			updateFadeInDecals(system, timeDelta);
			updateFadeOutDecals(system, timeDelta);

			updateAutoFadeOutDecals(system, timeDelta);
		}
	};


struct DecalManager::Data
{
	IStorm3D_TerrainDecalSystem &system;
	DecalCollection decalCollections[DECAL_COLLECTION_AMOUNT];

	Data(IStorm3D_TerrainDecalSystem &system_)
	:	system(system_)
	{
	}

	void add(DecalIdentifier &id, Type type, int materialId, const VC3 &position, const VC2 &size, const QUAT &rotation, const COL &light, bool inBuilding, int fadeOutWaitTime, bool forceSpawn)
	{
		assert(type >= 0 && type < DECAL_COLLECTION_AMOUNT);

		IStorm3D_TerrainDecalSystem::Type decalType = IStorm3D_TerrainDecalSystem::Outside;
		if(inBuilding)
			decalType = IStorm3D_TerrainDecalSystem::Inside;

		Decal decal;
		decal.fadeType = FADE_TYPE;
		decal.materialId = materialId;
		decal.decalId = system.addDecal(materialId, decalType, position, decal.id, forceSpawn);
		decal.rotation = rotation;
		decal.light = light;
		decal.size = size;
		decal.fadeOutWaitTime = fadeOutWaitTime;

		system.setRotation(materialId, decal.decalId, decal.id, decal.rotation);

		if(decal.fadeType == DecalManager::Blend)
		{
			system.setSize(materialId, decal.decalId, decal.id, decal.size);
			system.setAlpha(materialId, decal.decalId, decal.id, 0.f);
		}
		else
		{
			system.setSize(materialId, decal.decalId, decal.id, VC2());
			system.setAlpha(materialId, decal.decalId, decal.id, 1.f);
		}

		system.setLighting(materialId, decal.decalId, decal.id, decal.light);
		decalCollections[type].add(decal);

		id.materialId = decal.materialId;
		id.decalId = decal.decalId;
		id.id = decal.id;
	}

	void eraseDecal(const DecalIdentifier &id)
	{
		Decal decal;
		decal.materialId = id.materialId;
		decal.decalId = id.decalId;
		decal.id = id.id;
		FadeDecal fadeDecal;
		fadeDecal.currentTime = 0;
		fadeDecal.fadeTime = 11000;
		fadeDecal.decal = decal;

		decalCollections[1].fadeOutDecals.push_back(fadeDecal);
	}

	void update(int timeDelta)
	{
		for(int i = 0; i < DECAL_COLLECTION_AMOUNT; ++i)
		{
			decalCollections[i].update(system, timeDelta);

			if(i == DecalManager::Dynamic)
				decalCollections[i].updateNormalDecals(system);
		}
	}
};

DecalManager::DecalManager(IStorm3D_TerrainDecalSystem &system)
:	data(new Data(system))
{
}

DecalManager::~DecalManager()
{
}

void DecalManager::setMaxDecalAmount(int amount)
{
	assert(amount >= 0);
	MAX_DECAL_AMOUNT = amount;
}

void DecalManager::setEraseProperties(int fadeOutTime)
{
	if(fadeOutTime < 0)
	{
		assert(!"Invalid erase properties");
		return;
	}

	FADE_OUT_TIME = fadeOutTime;
}

void DecalManager::eraseDecal(const DecalIdentifier &decal)
{
	data->eraseDecal(decal);
}

void DecalManager::setSpawnProperties(int waitTime, int fadeInTime, FadeType type)
{
	if(waitTime < 0 || fadeInTime < 0)
	{
		assert(!"Invalid spawn properties");
		return;
	}

	SPAWN_WAIT_TIME = waitTime;
	FADE_IN_TIME = fadeInTime;
	FADE_TYPE = type;
}

void DecalManager::addDecal(DecalIdentifier &id, Type type, int materialId, const VC3 &position, const VC2 &size, const QUAT &rotation, const COL &light, bool inBuilding, int fadeOutWaitTime, bool forceSpawn)
{
	if(materialId < 0 || size.x < 0 || size.y < 0)
	{
		assert(!"Invalid decal parameters");
		return;
	}

	data->add(id, type, materialId, position, size, rotation, light, inBuilding, fadeOutWaitTime, forceSpawn);
}

void DecalManager::update(int timeDelta)
{
	data->update(timeDelta);
}

IStorm3D_TerrainDecalSystem &DecalManager::getDecalSystem()
{
	return data->system;
}

} // frozenbyte
