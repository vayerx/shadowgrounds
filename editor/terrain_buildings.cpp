// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_buildings.h"
#include "storm.h"
#include "storm_geometry.h"
#include "collision_model.h"
#include "terrain_mode.h"
#include "exporter.h"
#include "exporter_objects.h"
#include "storm_model_utils.h"
#include "mouse.h"
#include "ieditor_state.h"

#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../util/buildingmap.h"
#include "../util/buildinghandler.h"
#include "../ui/lightmanager.h"
#include <boost/shared_ptr.hpp>

#include <istorm3d.h>
#include <istorm3d_model.h>
#include <map>
#include <vector>
#include <cassert>

namespace frozenbyte {
namespace editor {
namespace {
	struct BuildingInstance
	{
		boost::shared_ptr<IStorm3D_Model> model;
		boost::shared_ptr<IStorm3D_Model> floorModel;

		VC2 position;
		float height;
		float rotation;

		BuildingInstance()
		:	height(0),
			rotation(0)
		{
		}
	};

	struct TerrainBuilding
	{
		boost::shared_ptr<IStorm3D_Model> model;
		std::vector<BuildingInstance> instances;

		float height;
		float radiusX;
		float radiusZ;
		bool cutTerrain;

		TerrainBuilding()
		:	height(0),
			radiusX(0),
			radiusZ(0),
			cutTerrain(true)
		{
		}

		void hideRoof(bool hide)
		{
			for(unsigned int i = 0; i < instances.size(); ++i)
			{
				boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(instances[i].model->ITObject->Begin());
				for(; !objectIterator->IsEnd(); objectIterator->Next())
				{
					IStorm3D_Model_Object *object = objectIterator->GetCurrent();
					if(!object)
						continue;

					std::string oname = object->GetName();
					if(oname.find("BuildingRoof") == oname.npos)
						continue;

					object->SetNoRender(hide);
				}
			}
		}

		void roofCollision(bool collision)
		{
			for(unsigned int i = 0; i < instances.size(); ++i)
			{
				boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(instances[i].model->ITObject->Begin());
				for(; !objectIterator->IsEnd(); objectIterator->Next())
				{
					IStorm3D_Model_Object *object = objectIterator->GetCurrent();
					if(!object)
						continue;

					std::string oname = object->GetName();
					if(oname.find("BuildingRoof") == oname.npos)
						continue;

					object->SetNoCollision(!collision);
				}
			}
		}
	};

	void removeNotFloors(IStorm3D_Model *m)
	{
		std::vector<IStorm3D_Model_Object *> objects;

		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(m->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			std::string oname = object->GetName();
			if(oname.find("BuildingFloor") == oname.npos)
				objects.push_back(object);
			else
				object->SetNoCollision(false);
		}

		for(unsigned int i = 0; i < objects.size(); ++i)
			m->Object_Delete(objects[i]);
	}
/*
	void cloneModel(const boost::shared_ptr<IStorm3D_Model> original, boost::shared_ptr<IStorm3D_Model> clone)
	{
		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(original->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			IStorm3D_Mesh *mesh = object->GetMesh();
			if(!mesh)
				continue;

			IStorm3D_Model_Object *newObject = clone->Object_New(object->GetName());
			newObject->SetMesh(mesh);
			newObject->SetPosition(object->GetPosition());
			newObject->SetRotation(object->GetRotation());
		}

		clone->SetNoCollision(true);
	}
*/
	VC2I convertCollisionToMap(const VC2I &source, const VC2I &mapSize, const VC3 &realSize)
	{
		VC2I result;
		result.x = int(source.x * mapSize.x / realSize.x / 2);
		result.y = int(source.y * mapSize.y / realSize.z / 2);

		return result;
	}

	VC2I convertSceneToMap(const VC2 &source, const VC2I &mapSize, const VC3 &realSize)
	{
		VC2I result;
		result.x = mapSize.x / 2;
		result.y = mapSize.y / 2;

		result.x += int(source.x * mapSize.x / realSize.x);
		result.y += int(source.y * mapSize.y / realSize.z);

		return result;
	}

	VC2I convertWorldToMap(const VC2 &source, const VC2I &mapSize, const VC3 &realSize)
	{
		VC2I result;
		
		result.x = int(source.x * mapSize.x / realSize.x);
		result.y = int(source.y * mapSize.y / realSize.z);

		return result;
	}

	int convertSceneToMap(float source, const VC2I &mapSize, const VC3 &realSize)
	{
		return int(source * mapSize.x / realSize.x);
	}

	struct ModelDeleter
	{
		IStorm3D_Scene *s;

		ModelDeleter(IStorm3D_Scene *s_)
		:	s(s_)
		{
		}

		void operator () (IStorm3D_Model *m)
		{
			if(s)
				s->RemoveModel(m);

			delete m;
		}
	};
}

struct TerrainBuildingsData
{
	Storm &storm;
	Mouse &mouse;

	std::vector<std::string> models;
	boost::shared_ptr<CollisionModel> activeCollision;

	typedef std::map<std::string, TerrainBuilding> BuildingMap;
	mutable BuildingMap buildings;

	BuildingHandler buildingHandler;
	IEditorState &state;

	TerrainBuildingsData(Storm &storm_, Mouse &mouse_, IEditorState &state_)
	:	storm(storm_),
		mouse(mouse_),
		state(state_)
	{
	}

	void addTerrain(TerrainBuilding &terrainBuilding, int index)
	{
		if(!terrainBuilding.model)
			loadModels();

		BuildingInstance &instance = terrainBuilding.instances[index];
		if(!instance.model || !instance.floorModel)
		{
			//instance.model = boost::shared_ptr<IStorm3D_Model> (storm.storm->CreateNewModel());
			//instance.floorModel = boost::shared_ptr<IStorm3D_Model> (storm.storm->CreateNewModel(), ModelDeleter(storm.floorScene));
			//addCloneModel(terrainBuilding.model, instance.model);
			//addCloneModel(terrainBuilding.model, instance.floorModel);
			instance.model = boost::shared_ptr<IStorm3D_Model> (terrainBuilding.model->GetClone(true, true, true));
			instance.floorModel = boost::shared_ptr<IStorm3D_Model> (terrainBuilding.model->GetClone(true, true, true), ModelDeleter(storm.floorScene));
			removeNotFloors(instance.floorModel.get());

			buildingHandler.addBuilding(instance.model.get());
			
			storm.floorModels.insert(instance.floorModel);
			storm.floorScene->AddModel(instance.floorModel.get());
		}

		if(terrainBuilding.model)
			terrainBuilding.model->FreeMemoryResources();

		QUAT rotation;
		rotation.MakeFromAngles(0, instance.rotation, 0);

		VC3 position(instance.position.x, 0, instance.position.y);
		position.y = storm.terrain->getHeight(instance.position);

		VC3 sunDir = state.getSunDirection();
		instance.model->SetPosition(position);
		instance.model->SetRotation(rotation);
		instance.model->SetDirectional(sunDir, 1.f);
		instance.floorModel->SetPosition(position);
		instance.floorModel->SetRotation(rotation);
		instance.floorModel->SetDirectional(sunDir, 1.f);
		storm.scene->AddModel(instance.model.get());
	}

	void loadModels()
	{
		std::map<std::string, TerrainBuilding>::iterator it = buildings.begin();
		for(; it != buildings.end(); ++it)
		{
			TerrainBuilding &terrainBuilding = (*it).second;
			if(!terrainBuilding.model)
			{
				const std::string &fileName = (*it).first;
				//terrainBuilding.model = boost::shared_ptr<IStorm3D_Model> (storm.storm->CreateNewModel());
				//terrainBuilding.model->LoadS3D(fileName.c_str());

				terrainBuilding.model = createEditorModel(*storm.storm, fileName);

				VC3 size = getSize(terrainBuilding.model.get());
				terrainBuilding.radiusX = size.x;
				terrainBuilding.height = size.y;
				terrainBuilding.radiusZ = size.z;

				/*
				frozenbyte::BuildingMap buildingMap(fileName.c_str(), terrainBuilding.model.get());
				const std::vector<std::vector<unsigned char> > &heightMap = buildingMap.getHeightMap();

				for(unsigned int i = 0; i < heightMap.size(); ++i)
				for(unsigned int j = 0; j < heightMap[i].size(); ++j)
				{
					float value = heightMap[i][j] * .2f;
					if(value > terrainBuilding.height)
						terrainBuilding.height = value;
				}
				*/

				//float factor = buildingMap.getMapResolution() * .5f;
				//terrainBuilding.radiusX = float(heightMap.size() * factor);
				//terrainBuilding.radiusZ = float(heightMap[0].size() * factor);
			}
		}
	}

	void testCollisions(CollisionData &collisionData, Building &building)
	{
		//BuildingMap::iterator it = buildings.begin();
		std::map<std::string, TerrainBuilding>::iterator it = buildings.begin();

		for(; it != buildings.end(); ++it)
		{
			TerrainBuilding &terrainBuilding = (*it).second;			
			ObjectData objectData;
			objectData.height = terrainBuilding.height;
			objectData.radiusX = terrainBuilding.radiusX;
			objectData.radiusZ = terrainBuilding.radiusZ;

			CollisionVolume collisionVolume(objectData);

			for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
			{
				BuildingInstance &instance = terrainBuilding.instances[i];
				if(collisionVolume.testCollision(instance.model->GetPosition(), VC3(0,instance.rotation,0), collisionData, 0.f))
				{
					building.fileName = (*it).first;
					building.index = i;
					building.rotation = instance.rotation;
				}
			}
		}
	}

	void rotateObject(Building &building, int yDelta)
	{
		//BuildingMap::iterator it = buildings.find(building.fileName);
		std::map<std::string, TerrainBuilding>::iterator it = buildings.find(building.fileName);
		if(it == buildings.end())
			return;

		BuildingInstance &instance = (*it).second.instances[building.index];
		instance.rotation = storm.unitAligner.getRotation(instance.rotation, yDelta);
		//instance.rotation += yDelta;

		QUAT rotation;
		rotation.MakeFromAngles(0, instance.rotation, 0);
		instance.model->SetRotation(rotation);
		instance.floorModel->SetRotation(rotation);
	}

	void moveObject(Building &building, const VC2 &delta)
	{
		std::map<std::string, TerrainBuilding>::iterator it = buildings.find(building.fileName);
		if(it == buildings.end())
			return;

		BuildingInstance &instance = (*it).second.instances[building.index];
		instance.position += delta;

		VC3 position(instance.position.x, 0, instance.position.y);
		position.y = storm.terrain->getHeight(instance.position);

		instance.model->SetPosition(position);
		instance.floorModel->SetPosition(position);
	}

	void removeObject(Building &building)
	{
		//BuildingMap::iterator it = buildings.find(building.fileName);
		std::map<std::string, TerrainBuilding>::iterator it = buildings.find(building.fileName);
		if(it == buildings.end())
			return;

		TerrainBuilding &terrainBuilding = (*it).second;
		int foo = terrainBuilding.instances.size();
		BuildingInstance &instance = terrainBuilding.instances[building.index];

		storm.scene->RemoveModel(instance.model.get());
		terrainBuilding.instances.erase(terrainBuilding.instances.begin() + building.index);
	}

	void removeAll()
	{
		//BuildingMap::iterator it = buildings.begin();
		std::map<std::string, TerrainBuilding>::iterator it = buildings.begin();
		for(; it != buildings.end(); ++it)
		{
			TerrainBuilding &terrainBuilding = (*it).second;
			if(terrainBuilding.model)
				terrainBuilding.model.reset();

			for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
			{
				BuildingInstance &instance = terrainBuilding.instances[i];
				if(instance.model)
				{
					storm.scene->RemoveModel(instance.model.get());
					instance.model.reset();
				}
			}
		}

		buildingHandler.clear();
	}

	void writeStream(filesystem::OutputStream &stream) const
	{
		stream << int(1);
		stream << int(models.size());

		for(unsigned int i = 0; i < models.size(); ++i)
		{
			const std::string &fileName = models[i];
			const TerrainBuilding &building = buildings[fileName];

			stream << fileName;
			stream << int(building.instances.size());
			stream << int(building.cutTerrain);

			for(unsigned int j = 0; j < building.instances.size(); ++j)
			{
				const BuildingInstance &instance = building.instances[j];
				stream << instance.position.x << instance.position.y << instance.rotation;
			}
		}
	}

	void readStream(filesystem::InputStream &stream)
	{
		buildings.clear();
		models.clear();

		int version = 0;
		stream >> version;

		int modelCount = 0;
		stream >> modelCount;

		for(int i = 0; i < modelCount; ++i)
		{
			std::string fileName;
			stream >> fileName;

			int buildingCount = 0;
			stream >> buildingCount;

			TerrainBuilding terrainBuilding;
			terrainBuilding.instances.resize(buildingCount);

			if(version > 0)
			{
				int remove = 0;
				stream >> remove;

				if(remove)
					terrainBuilding.cutTerrain = true;
				else
					terrainBuilding.cutTerrain = false;
			}

			for(int j = 0; j < buildingCount; ++j)
			{
				BuildingInstance &instance = terrainBuilding.instances[j];				
				stream >> instance.position.x >> instance.position.y;
				stream >> instance.rotation;
			}

			models.push_back(fileName);
			buildings[fileName] = terrainBuilding;
		}
	}
};

TerrainBuildings::TerrainBuildings(Storm &storm, Mouse &mouse, IEditorState &state)
{
	boost::scoped_ptr<TerrainBuildingsData> tempData(new TerrainBuildingsData(storm, mouse, state));
	data.swap(tempData);
}

TerrainBuildings::~TerrainBuildings()
{
}

void TerrainBuildings::addModel(const std::string &fileName)
{
	if(std::find(data->models.begin(), data->models.end(), fileName) == data->models.end())
	{
		data->models.push_back(fileName);
		std::sort(data->models.begin(), data->models.end());
	}
}

void TerrainBuildings::removeModel(int modelIndex)
{
	assert((modelIndex >= 0) && (modelIndex < getModelCount()));

	std::string &fileName = data->models[modelIndex];
	TerrainBuilding &terrainBuilding = data->buildings[fileName];

	if(terrainBuilding.instances.empty())
	{
		std::map<std::string, TerrainBuilding>::iterator it = data->buildings.find(fileName);
		data->buildings.erase(it);

		data->models.erase(data->models.begin() + modelIndex);
	}
}

void TerrainBuildings::setCutTerrain(int modelIndex, bool cut)
{
	assert((modelIndex >= 0) && (modelIndex < getModelCount()));
	TerrainBuilding &terrainBuilding = data->buildings[data->models[modelIndex]];

	terrainBuilding.cutTerrain = cut;
}

void TerrainBuildings::updateLighting()
{
	VC3 sunDir = data->state.getSunDirection();

	std::vector<TerrainLightMap::PointLight> lights;
	data->state.getBuildingLights(lights);

	::ui::LightManager *manager = data->storm.lightManager;
	if(!manager)
		return;

	manager->clearBuildingLights();
	for(unsigned int i = 0; i < lights.size(); ++i)
	{
		TerrainLightMap::PointLight &l = lights[i];
		manager->addBuildingLight(l.position, l.color, l.range);
	}

	std::map<std::string, TerrainBuilding>::iterator it = data->buildings.begin();
	for(; it != data->buildings.end(); ++it)
	{
		TerrainBuilding &terrainBuilding = (*it).second;		
		for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
		{
			IStorm3D_Model *m = terrainBuilding.instances[i].model.get();
			if(!m)
				continue;

			m->SetDirectional(sunDir, 1.f);
			manager->setBuildingLights(*m);
		}
	}

	/*
	std::map<std::string, TerrainBuilding>::iterator it = data->buildings.begin();
	for(; it != data->buildings.end(); ++it)
	{
		TerrainBuilding &terrainBuilding = (*it).second;		
		for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
		{
			IStorm3D_Model *m = terrainBuilding.instances[i].model.get();
			m->SetDirectional(sunDir, 1.f);
			m->ResetObjectLights();

			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(m->ITObject->Begin());
			for(; !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				if(!object)
					continue;
				std::string oname = object->GetName();
				if(oname.find("OuterWall") == oname.npos && oname.find("BuildingRoof") == oname.npos)
					continue;

				Sphere sphere = object->GetBoundingSphere();
				m->GetMX().TransformVector(sphere.position);

				// Find lights
				int closest[2] = { -1, -1 };
				{
					for(unsigned int j = 0; j < lights.size(); ++j)
					{
						const TerrainLightMap::PointLight &light = lights[j];
						float dist = sphere.position.GetRangeTo(light.position);
						//if(dist > light.range - radius)
						//	continue;

						if(closest[0] == -1)
						{
							closest[0] = j;
							continue;
						}

						float dist1 = sphere.position.GetRangeTo(lights[closest[0]].position);
						if(dist < dist1)
						{
							closest[1] = closest[0];
							closest[0] = j;
							continue;
						}

						if(closest[1] == -1)
						{
							closest[1] = j;
							continue;
						}

						float dist2 = sphere.position.GetRangeTo(lights[closest[1]].position);
						if(dist < dist2)
						{
							closest[1] = j;
							continue;
						}
					}
				}

				// Apply
				{
					for(int j = 0; j < 2; ++j)
					{
						int index = closest[j];
						if(index == -1)
							continue;

						const TerrainLightMap::PointLight &light = lights[index];

						int id = m->AddObjectLight(light.position, light.color, light.range);
						object->SetLight(j, id);
					}
				}
			}
		}
	}
	*/
}

void TerrainBuildings::clear()
{
	data->removeAll();
	data->models.clear();
	data->buildings.clear();
	data->buildingHandler.clear();
}

void TerrainBuildings::setToTerrain()
{
	data->removeAll();

	std::map<std::string, TerrainBuilding>::iterator it = data->buildings.begin();
	for(; it != data->buildings.end(); ++it)
	{
		TerrainBuilding &terrainBuilding = (*it).second;
		
		for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
			data->addTerrain(terrainBuilding, i);
	}
}

void TerrainBuildings::addBuilding(int modelIndex, const VC2 &position, float rotation)
{
	assert((modelIndex >= 0) && (modelIndex < getModelCount()));
	TerrainBuilding &terrainBuilding = data->buildings[data->models[modelIndex]];

	BuildingInstance instance;
	instance.position = position;
	instance.rotation = rotation;

	terrainBuilding.instances.push_back(instance);
	data->addTerrain(terrainBuilding, terrainBuilding.instances.size() - 1);
}

Building TerrainBuildings::traceActiveCollision(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength)
{
	CollisionData collisionData;
	collisionData.rayOrigin = rayOrigin;
	collisionData.rayDirection = rayDirection;
	collisionData.rayLength = rayLength;

	if(data->activeCollision)
		data->activeCollision.reset();

	Building building;
	data->testCollisions(collisionData, building);

	if(!collisionData.hasCollision)
		return Building();

	data->activeCollision = boost::shared_ptr<CollisionModel> (new CollisionModel(collisionData.objectData, COL(1,0,0), collisionData.collisionPosition, VC3(0,building.getRotation(),0), data->storm));
	data->activeCollision->create();
	data->activeCollision->scale();

	return building;
}

void TerrainBuildings::rotateObject(Building &building, int yDelta)
{
	data->rotateObject(building, yDelta);
}

void TerrainBuildings::moveObject(Building &building, const VC2 &delta)
{
	data->moveObject(building, delta);
}

void TerrainBuildings::removeObject(Building &building)
{
	data->removeObject(building);
}

void TerrainBuildings::hideRoofs(bool hide)
{
	//data->buildingHandler.beginUpdate();

	//if(hide)
	{
		std::map<std::string, TerrainBuilding>::iterator it = data->buildings.begin();
		for(; it != data->buildings.end(); ++it)
		{
			TerrainBuilding &terrainBuilding = it->second;
			terrainBuilding.hideRoof(hide);

			//for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
			//	data->buildingHandler.removeTopFrom(terrainBuilding.instances[i].model.get());
		}
	}

	//data->buildingHandler.endUpdate(true);
}

void TerrainBuildings::roofCollision(bool collision)
{
	std::map<std::string, TerrainBuilding>::iterator it = data->buildings.begin();
	for(; it != data->buildings.end(); ++it)
	{
		TerrainBuilding &terrainBuilding = it->second;
		terrainBuilding.roofCollision(collision);
	}
}

void TerrainBuildings::showDoors(bool show)
{
}

int TerrainBuildings::getModelCount() const
{
	return data->models.size();
}

std::string TerrainBuildings::getModel(int index) const
{
	assert((index >= 0) && (index < getModelCount()));
	return data->models[index];
}

bool TerrainBuildings::hasCutTerrain(int modelIndex) const
{
	assert((modelIndex >= 0) && (modelIndex < getModelCount()));
	TerrainBuilding &terrainBuilding = data->buildings[data->models[modelIndex]];

	return terrainBuilding.cutTerrain;
}

void TerrainBuildings::doExport(Exporter &exporter) const
{
	ExporterObjects &exporterObjects = exporter.getObjects();

	std::map<std::string, TerrainBuilding>::iterator it = data->buildings.begin();
	for(; it != data->buildings.end(); ++it)
	{
		TerrainBuilding &terrainBuilding = it->second;
		int id = exporterObjects.addTerrainBuilding(it->first, terrainBuilding.cutTerrain);
		
		for(unsigned int i = 0; i < terrainBuilding.instances.size(); ++i)
		{
			VC2 &scenePosition = terrainBuilding.instances[i].position;
			exporterObjects.addBuilding(id, scenePosition, terrainBuilding.instances[i].rotation);
		}
	}
}

filesystem::OutputStream &TerrainBuildings::writeStream(filesystem::OutputStream &stream) const
{
	data->writeStream(stream);
	return stream;
}

filesystem::InputStream &TerrainBuildings::readStream(filesystem::InputStream &stream)
{
	data->readStream(stream);
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
