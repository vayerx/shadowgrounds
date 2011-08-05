#include <boost/shared_ptr.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>

#include "precompiled.h"

#include "DecalSystem.h"
#include "DecalManager.h"
#include "DecalSpawner.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include "../system/Logger.h"
#include <IStorm3D.h>
#include <IStorm3D_Texture.h>
#include <IStorm3D_Material.h>

using namespace std;
using namespace boost;
using namespace frozenbyte;
using namespace frozenbyte::editor;

namespace frozenbyte {
	struct Effect;
#ifdef LEGACY_FILES
	static const char *fileName = "Data/Effects/decals.txt";
#else
	static const char *fileName = "data/effect/decals.txt";
#endif

	typedef vector<Effect> EffectList;
	typedef vector<shared_ptr<DecalSpawner> > SpawnerList;

	struct Effect
	{
		string name;

		DecalSpawner::Type type;
		int waitTime;
		DecalSpawner::FadeType fadeType;
		int fadeTime;
		int fadeOutWaitTime;

		float sizeMin;
		float sizeMax;
		bool randomRotation;
		bool forceSpawn;

		SpawnerList spawners;

		Effect()
		:	type(DecalSpawner::Dynamic),
			waitTime(0),
			fadeType(DecalSpawner::Blend),
			fadeTime(0),
			fadeOutWaitTime(0),
			sizeMin(0.1f),
			sizeMax(0.2f),
			randomRotation(true),
			forceSpawn(false)
		{
		}
	};

	static Effect parseEffectProperties(const string &name, const ParserGroup &properties)
	{
		Effect result;
		result.name = name;

		const string &type = properties.getValue("type");
		if(type == "static")
			result.type = DecalSpawner::Static;
		else if(type == "dynamic")
			result.type = DecalSpawner::Dynamic;
		result.waitTime = convertFromString<int> (properties.getValue("wait_time"), 0);

		const string &fadeType = properties.getValue("fade_type");
		if(fadeType == "scale")
			result.fadeType = DecalSpawner::Scale;
		else if(fadeType == "blend")
			result.fadeType = DecalSpawner::Blend;
		result.fadeTime = convertFromString<int> (properties.getValue("fade_time"), 100);
		result.fadeOutWaitTime = convertFromString<int> (properties.getValue("fade_out_wait_time"), 0);

		result.sizeMin = convertFromString<float> (properties.getValue("size_min"), .1f);
		result.sizeMax = convertFromString<float> (properties.getValue("size_max"), .2f);
		result.randomRotation = convertFromString<int> (properties.getValue("random_rotation"), 1) == 1;
		result.forceSpawn = convertFromString<int> (properties.getValue("force_spawn"), 0) == 1;

		return result;
	}

	static void addSpawner(Effect &effect, DecalManager &manager, const std::string &textureName, IStorm3D &storm)
	{
		IStorm3D_Texture *texture = storm.CreateNewTexture(textureName.c_str());

		if(!texture)
		{
			Logger::getInstance()->warning("Decal texture not found.");

			string message = effect.name;
			message += ": ";
			message += textureName;

			Logger::getInstance()->warning(message.c_str());
			texture = storm.CreateNewTexture("missing.dds");
		}

		Storm3D_SurfaceInfo info = texture->GetSurfaceInfo();
		float sizeFactor = float(info.height) / float(info.width);

		IStorm3D_Material *material = storm.CreateNewMaterial(effect.name.c_str());
		material->SetBaseTexture(texture);

		shared_ptr<DecalSpawner> spawner(new DecalSpawner(manager, *material));
		spawner->setType(effect.type);
		spawner->setSpawnProperties(effect.waitTime, effect.fadeTime, effect.fadeType);
		spawner->setSizeFactor(sizeFactor);
		spawner->setAutoFadeOut(effect.fadeOutWaitTime);
		spawner->setForceSpawn(effect.forceSpawn);

		effect.spawners.push_back(spawner);
	}


struct DecalSystem::Data
{
	IStorm3D &storm;
	IStorm3D_TerrainDecalSystem &system;

	DecalManager manager;
	EffectList effects;

	Data(IStorm3D &storm_, IStorm3D_TerrainDecalSystem &system_)
	:	storm(storm_),
		system(system_),
		manager(system)
	{
	}

	void parseEffects()
	{
		EditorParser parser(false, true);
		filesystem::InputStream file = filesystem::FilePackageManager::getInstance().getFile(fileName);
		parser.readStream(file);

		ParserGroup &global = parser.getGlobals();
		int groupAmount = global.getSubGroupAmount();

		for(int i = 0; i < groupAmount; ++i)
		{
			const std::string &name = global.getSubGroupName(i);
			const ParserGroup &group = global.getSubGroup(i);
			const ParserGroup &textures = group.getSubGroup("textures");

			if(name.empty())
				continue;

			effects.push_back(parseEffectProperties(name, group));
			Effect &effect = effects[effects.size() - 1];

			int lineCount = textures.getLineCount();
			for(int j = 0; j < lineCount; ++j)
			{
				const string &texture = textures.getLine(j);
				addSpawner(effect, manager, texture, storm);
			}

			if(lineCount == 0)
			{
				Logger::getInstance()->warning("Decal effect with no textures defined");
				Logger::getInstance()->warning(name.c_str());
			}
		}
	}

	void spawn(DecalIdentifier &identifier, int id, const VC3 &position, const QUAT &rotation, const COL &light, bool inBuilding)
	{
		assert(id >= 0 && id < int(effects.size()));

		Effect &effect = effects[id];
		if(effect.spawners.empty())
			return;

		float size = effect.sizeMin;
		float sizeDelta = effect.sizeMax - effect.sizeMin;
		size += (rand() / float(RAND_MAX)) * sizeDelta;

		int spawnerIndex = rand() % effect.spawners.size();

		DecalSpawner &spawner = *effect.spawners[spawnerIndex];
		spawner.setSize(VC2(size, size));

		if(!effect.randomRotation)
			spawner.spawnDecal(identifier, position, rotation, light, inBuilding);
		else
		{
			float angle = rand() % 6000 / 1000.f;
			QUAT r;
			r.MakeFromAngles(0, 0, angle);
			r = r * rotation;

			spawner.spawnDecal(identifier, position, r, light, inBuilding);
		}
	}

	int getId(const string &name) const
	{
		EffectList::const_iterator it = effects.begin();
		for(; it != effects.end(); ++it)
		{
			if(it->name == name)
				return it - effects.begin();
		}

		return -1;
	}

	float getMaxSize(int id) const
	{
		assert(id >= 0 && id < int(effects.size()));
		return effects[id].sizeMax;
	}
};

DecalSystem::DecalSystem(IStorm3D &storm, IStorm3D_TerrainDecalSystem &system)
{
	scoped_ptr<Data> tempData(new Data(storm, system));
	tempData->parseEffects();

	data.swap(tempData);
}

DecalSystem::~DecalSystem()
{
}

int DecalSystem::getEffectId(const char *name) const
{
	if(!name)
	{
		assert(!"Null pointer for getEffectId");
		return -1;
	}

	return data->getId(name);
}

float DecalSystem::getMaxSize(int id) const
{
	if(id < 0)
	{
		assert(!"getMaxSize -- negative effect id");
		return 0.f;
	}

	return data->getMaxSize(id);
}

void DecalSystem::spawnDecal(DecalIdentifier &identifier, int id, const VC3 &position, const QUAT &rotation, const COL &light, bool inBuilding)
{
	if(id < 0)
	{
		assert(!"Create decal -- negative effect id");
		return;
	}

	// TEMP
	//char buf[64];
	//sprintf(buf, "%d - %f,%f,%f", id, position.x, position.y, position.z);
	//Logger::getInstance()->error(buf);

	data->spawn(identifier, id, position, rotation, light, inBuilding);
}

void DecalSystem::eraseDecal(const DecalIdentifier &identifier)
{
	data->manager.eraseDecal(identifier);
}

void DecalSystem::update(int timeDelta)
{
	data->manager.update(timeDelta);
}

void DecalSystem::setMaxDecalAmount(int amount)
{
	data->manager.setMaxDecalAmount(amount);
}

void DecalSystem::setEraseProperties(int fadeOutTime)
{
	data->manager.setEraseProperties(fadeOutTime);
}


} // frozenbyte
