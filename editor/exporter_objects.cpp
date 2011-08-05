// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "exporter_objects.h"
#include "export_options.h"
#include "explosion_scripts.h"
#include "UniqueEditorObjectHandle.h"
#include "../filesystem/output_file_stream.h"

#include <vector>
#include <fstream>

namespace frozenbyte {
namespace editor {
namespace  {

	struct ObjectData
	{
		std::string fileName;
		ExporterObjects::CollisionType type;
		ExporterObjects::FallType fallType;

		float height;
		float radius;
		bool fireThrough;

		std::string explosionObject;
		std::string explosionScript;
		std::string explosionProjectile;
		std::string explosionEffect;
		std::vector<std::string> explosionSounds;
		std::string material;
		int hp;
		int breakTexture;

		int physicsType;
		float physicsMass;
		std::string physicsSoundMaterial;
		VC3 physicsData1;
		VC3 physicsData2;
		std::string durabilityType;

		Animation animation;
		std::map<std::string, std::string> metaValues;

		ObjectData()
		:	type(ExporterObjects::CollisionNone),
			fallType(ExporterObjects::FallStatic),

			height(0),
			radius(0),
			fireThrough(true),
			hp(0),
			breakTexture(0),

			physicsType(0),
			physicsMass(5.f)
		{
		}
	};

	struct ObjectInstanceData
	{
		VC3 position;
		VC3 rotation;
		float height;
		COL color;

		/*
		VC3 lightPosition1;
		COL lightColor1;
		float lightRange1;
		VC3 lightPosition2;
		COL lightColor2;
		float lightRange2;
		*/

		signed short int lightIndex[LIGHT_MAX_AMOUNT];
		VC3 sunDirection;
		float sunStrength;
		bool lightmapped;
		bool inBuilding;
		UniqueEditorObjectHandle uniqueEditorObjectHandle;

		ObjectInstanceData()
		:	height(0),
			//lightRange1(1.f),
			//lightRange2(1.f),
			sunStrength(0),
			lightmapped(false),
			inBuilding(false),
			uniqueEditorObjectHandle(0)
		{
			
			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				lightIndex[i] = -1;
		}
	};

	struct ObjectInstances
	{
		ObjectData objectData;
		std::vector<ObjectInstanceData> instances;
	};

	struct BuildingData
	{
		std::string fileName;
		bool cutTerrain;

		BuildingData()
		:	cutTerrain(true)
		{
		}
	};
	
	struct BuildingInstanceData
	{
		VC2 position;
		float yRotation;
	};

	struct BuildingInstances
	{
		BuildingData buildingData;
		std::vector<BuildingInstanceData> instances;
	};

	filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const VC2 &vector)
	{
		return stream << vector.x << vector.y;
	}

	filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const VC3 &vector)
	{
		return stream << vector.x << vector.y << vector.z;
	}

	filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const COL &color)
	{
		return stream << color.r << color.g << color.b;
	}
}

using namespace std;

struct ExporterObjectsData
{
	std::vector<ObjectInstances> objects;
	std::vector<BuildingInstances> buildings;

	void saveObjects(const std::string &fileName)
	{
		filesystem::OutputStream stream = filesystem::createOutputFileStream(fileName);

		// version number
		stream << int(22);

		stream << float(200) << float(450) << float(450); // Ranges
		stream << int(objects.size());

		for(unsigned int i = 0; i < objects.size(); ++i)
		{
			ObjectData &data = objects[i].objectData;

			int type = 0;
			if(data.type == ExporterObjects::CollisionBox)
				type = 3;
			if(data.type == ExporterObjects::CollisionCylinder)
				type = 1;
			if(data.type == ExporterObjects::CollisionMapped)
				type = 2;

			stream << data.fileName << int(type) << int(data.fallType) << data.height << data.radius << data.fireThrough;
			stream << data.explosionObject << data.explosionEffect;
			stream << data.explosionScript << data.explosionProjectile;

			stream << int(data.explosionSounds.size());
			for(unsigned int k = 0; k < data.explosionSounds.size(); ++k)
				stream << data.explosionSounds[k];

			stream << data.material;

			{
				stream << data.animation.bones;
				stream << data.animation.idleAnimation;
				stream << int(data.animation.animations.size());

				for(unsigned int k = 0; k < data.animation.animations.size(); ++k)
					stream << data.animation.animations[k];
			}

			stream << data.hp;
			stream << data.breakTexture;

			stream << data.physicsType;
			stream << data.physicsMass;
			stream << data.physicsSoundMaterial;
			stream << data.physicsData1.x << data.physicsData1.y << data.physicsData1.z;
			stream << data.physicsData2.x << data.physicsData2.y << data.physicsData2.z;
			stream << data.durabilityType;

			// v21
			stream << int(data.metaValues.size());
			for(std::map<std::string, std::string>::iterator it = data.metaValues.begin(); it != data.metaValues.end(); ++it)
				stream << it->first << it->second;

			stream << int(objects[i].instances.size());
			for(unsigned int j = 0; j < objects[i].instances.size(); ++j)
			{
				ObjectInstanceData &instance = objects[i].instances[j];
				stream << instance.position.x;
				stream << instance.position.z;
				stream << instance.rotation.x << instance.rotation.y << instance.rotation.z;
				stream << instance.color;
				stream << instance.height;
				stream << instance.position.y;

				/*
				const VC3 &p1 = objects[i].instances[j].lightPosition1;
				stream << p1.x << p1.y << p1.z;
				stream << objects[i].instances[j].lightColor1;
				stream << objects[i].instances[j].lightRange1;

				const VC3 &p2 = objects[i].instances[j].lightPosition2;
				stream << p2.x << p2.y << p2.z;
				stream << objects[i].instances[j].lightColor2;
				stream << objects[i].instances[j].lightRange2;
				*/

				//for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
				//	stream << objects[i].instances[j].lightIndex[k];

				const VC3 &d = objects[i].instances[j].sunDirection;
				stream << d.x << d.y << d.z;
				stream << objects[i].instances[j].sunStrength;
				stream << objects[i].instances[j].lightmapped;
				stream << objects[i].instances[j].inBuilding;

				stream << (unsigned int)(objects[i].instances[j].uniqueEditorObjectHandle & (((UniqueEditorObjectHandle)1<<32)-1));
				stream << (unsigned int)(objects[i].instances[j].uniqueEditorObjectHandle >> (UniqueEditorObjectHandle)32);
			}
		}
	}

	void saveBuildings(const std::string &fileName, const std::string &id)
	{
		std::ofstream stream(fileName.c_str());

		stream << "script " << id << "_buildings" << std::endl;
		stream << "sub create" << std::endl;

		for(unsigned int i = 0; i < buildings.size(); ++i)
		{
			std::string &fileName = buildings[i].buildingData.fileName;

			for(unsigned int j = 0; j < buildings[i].instances.size(); ++j)
			{
				BuildingInstanceData &instance = buildings[i].instances[j];

				stream << std::endl;
				stream << "   setPosition s," << instance.position.x << "," << instance.position.y << std::endl;
				stream << "   setTerrainCut " << buildings[i].buildingData.cutTerrain << std::endl;
				stream << "   addBuildingToPosition " << fileName;

				float yRotation = instance.yRotation;
				if(fabs(yRotation) > 0.001)
				{
					int degree = int((yRotation / PI * 180.f) + 0.5f);
					while(degree < 0)
						degree += 360;

					stream << "@" << degree % 360;
				}

				stream << std::endl;
			}
		}

		stream << "endSub" << std::endl;
		stream << "endScript" << std::endl;
	}
};

ExporterObjects::ExporterObjects()
{
	boost::scoped_ptr<ExporterObjectsData> tempData(new ExporterObjectsData());
	data.swap(tempData);
}

ExporterObjects::~ExporterObjects()
{
}

int ExporterObjects::addTerrainObject(const std::string &fileName, CollisionType type, FallType fallType, float height, float radius, bool fireThrough, const string &explosionObject, const string &explosionScript, const string &explosionProjectile, const string &explosionEffect, const vector<string> &explosionSounds, const std::string &material, int hp, const Animation &animation, int breakTexture, int physicsType, float physicsMass, const std::string &physicsSoundMaterial, const VC3 &physicsData1, const VC3 &physicsData2, const std::string &durabilityType, const std::map<std::string, std::string> &metaValues)
{
	ObjectInstances objectInstances;
	objectInstances.objectData.fileName = fileName;
	objectInstances.objectData.type = type;
	objectInstances.objectData.fallType = fallType;
	objectInstances.objectData.height = height;
	objectInstances.objectData.radius = radius;
	objectInstances.objectData.fireThrough = fireThrough;
	objectInstances.objectData.explosionObject = explosionObject;
	objectInstances.objectData.explosionScript = explosionScript;
	objectInstances.objectData.explosionProjectile = explosionProjectile;
	objectInstances.objectData.explosionEffect = explosionEffect;
	objectInstances.objectData.explosionSounds = explosionSounds;
	objectInstances.objectData.material = material;
	objectInstances.objectData.animation = animation;
	objectInstances.objectData.hp = hp;
	objectInstances.objectData.breakTexture = breakTexture;
	objectInstances.objectData.physicsType = physicsType;
	objectInstances.objectData.physicsMass = physicsMass;
	objectInstances.objectData.physicsData1 = physicsData1;
	objectInstances.objectData.physicsData2 = physicsData2;
	objectInstances.objectData.physicsSoundMaterial = physicsSoundMaterial;
	objectInstances.objectData.durabilityType = durabilityType;
	objectInstances.objectData.metaValues = metaValues;

	if(objectInstances.objectData.physicsSoundMaterial.empty())
#ifdef LEGACY_FILES
		objectInstances.objectData.physicsSoundMaterial = "NoSound";
#else
		objectInstances.objectData.physicsSoundMaterial = "no_sound";
#endif

	if(objectInstances.objectData.durabilityType.empty())
		objectInstances.objectData.durabilityType = "no_durability";

	data->objects.push_back(objectInstances);
	return data->objects.size() - 1;
}

void ExporterObjects::addObject(int id, const VC3 &position, const VC3 &rotation, const COL &color, float height, signed short int *lightIndices, bool lightmapped, bool inBuilding, const VC3 &sunDir, float sunStrength)
{
	if(!lightIndices)
		return;

	ObjectInstanceData instance;
	instance.position = position;
	instance.rotation = rotation;
	instance.color = color;
	instance.height = height;
	/*
	instance.lightPosition1 = lightPos1;
	instance.lightColor1 = lightCol1;
	instance.lightRange1 = lightRange1;
	instance.lightPosition2 = lightPos2;
	instance.lightColor2 = lightCol2;
	instance.lightRange2 = lightRange2;
	*/
	for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
		instance.lightIndex[i] = lightIndices[i];

	instance.lightmapped = lightmapped;
	instance.inBuilding = inBuilding;
	instance.sunDirection = sunDir;
	instance.sunStrength = sunStrength;

	ObjectInstances &objectInstances = data->objects[id];
	objectInstances.instances.push_back(instance);
}

int ExporterObjects::addTerrainBuilding(const std::string &fileName, bool cutTerrain)
{
	BuildingInstances buildingInstances;
	buildingInstances.buildingData.fileName = fileName;
	buildingInstances.buildingData.cutTerrain = cutTerrain;

	data->buildings.push_back(buildingInstances);
	return data->buildings.size() - 1;
}

void ExporterObjects::addBuilding(int id, const VC2 &position, float yRotation)
{
	BuildingInstanceData instance;
	instance.position = position;
	instance.yRotation = yRotation;

	BuildingInstances &buildingInstances = data->buildings[id];
	buildingInstances.instances.push_back(instance);
}

void ExporterObjects::save(const ExportOptions &options) const
{
	if(!options.onlyScripts)
	{
		std::string objectFileName = options.fileName + "\\bin\\objects.bin";
		data->saveObjects(objectFileName);
	}

	std::string buildingFileName = options.fileName + std::string("\\") + options.id + std::string("_buildings.dhs");
	data->saveBuildings(buildingFileName, options.id);
}

} // end of namespace editor
} // end of namespace frozenbyte
