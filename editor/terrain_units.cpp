// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_units.h"
#include "terrain_colormap.h"
#include "storm.h"
#include "storm_geometry.h"
#include "unit_scripts.h"
#include "unit_properties.h"
#include "collision_model.h"
#include "exporter.h"
#include "exporter_scripts.h"
#include "exporter_units.h"
#include "string_conversions.h"
#include "string_properties.h"
#include "storm_model_utils.h"
#include "ieditor_state.h"
#include "../ui/lightmanager.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "UniqueEditorObjectHandle.h"
#include "UniqueEditorObjectHandleManager.h"

#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_mesh.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <time.h>

using namespace std;

namespace frozenbyte {
namespace editor {
namespace {
	struct UnitInstance
	{
		ScriptConfig scripts;
		UnitHelpers helpers;
		UnitProperties properties;

		int side;

		VC2 position;
		float cachedHeight;
		float heightOffset;
		float yRotation;

		boost::shared_ptr<IStorm3D_Model> model;

		UniqueEditorObjectHandle uniqueEditorObjectHandle;

		UnitInstance()
		:	side(0),
			cachedHeight(0),
			heightOffset(0),
			yRotation(0)
		{
			//unsigned int internalHandle = time(NULL);
			unsigned int internalHandle = _time32(NULL);

			uniqueEditorObjectHandle = UniqueEditorObjectHandleManager::createNewUniqueHandle(internalHandle);
		}

		UnitInstance(const VC2 &position_, float yRotation_)
		:	position(position_),
			cachedHeight(0),
			heightOffset(0),
			yRotation(yRotation_)
		{
			//unsigned int internalHandle = time(NULL);
			unsigned int internalHandle = _time32(NULL);

			uniqueEditorObjectHandle = UniqueEditorObjectHandleManager::createNewUniqueHandle(internalHandle);
		}


		~UnitInstance()
		{
		}
	};

	struct TerrainUnit
	{
		VC3 size;
		std::vector<UnitInstance> instances;

		TerrainUnit()
		:	size(0, 0, 0)
		{
		}
	};

	VC2I convertSceneToMap(const VC2 &source, const VC2I &mapSize, const VC3 &realSize)
	{
		VC2I result;

		result.x = mapSize.x / 2;
		result.y = mapSize.y / 2;

		result.x += int(source.x * mapSize.x / realSize.x);
		result.y += int(source.y * mapSize.y / realSize.z);

		result.y = mapSize.y - result.y - 1;
		return result;
	}

	typedef std::map<std::string, std::vector<VC2> > HelperList;
} // unnamed

//---

UnitHelpers::UnitHelpers()
{
}

UnitHelpers::~UnitHelpers()
{
}

int UnitHelpers::getHelperAmount() const
{
	return usedHelpers.size();
}

const std::string &UnitHelpers::getHelperName(int helperIndex) const
{
	assert(helperIndex >= 0 && helperIndex < int(usedHelpers.size()));
	return usedHelpers[helperIndex].name;
}

const std::string &UnitHelpers::getHelperExportName(int helperIndex) const
{
	assert(helperIndex >= 0 && helperIndex < int(usedHelpers.size()));
	return usedHelpers[helperIndex].exportName;
}

bool UnitHelpers::isWayPoint(int helperIndex) const
{
	assert(helperIndex >= 0 && helperIndex < int(usedHelpers.size()));
	return usedHelpers[helperIndex].isPath;
}

int UnitHelpers::getPointAmount(int helperIndex) const
{
	HelperList::const_iterator it = helpers.find(getHelperName(helperIndex));
	if(it == helpers.end())
		return 0;

	return it->second.size();
}

VC2 UnitHelpers::getPoint(int helperIndex, int index) const
{
	HelperList::const_iterator it = helpers.find(getHelperName(helperIndex));
	if(it == helpers.end())
	{
		assert(!"whoops");
		return VC2();
	}

	return it->second[index];
}

void UnitHelpers::addPoint(int helperIndex, const VC2 &point)
{
	//assert(helpers.find(getHelperName(helperIndex)) != helpers.end());
	helpers[getHelperName(helperIndex)].push_back(point);
}

void UnitHelpers::setPoint(int helperIndex, int index, const VC2 &point)
{
	assert(helpers.find(getHelperName(helperIndex)) != helpers.end());
	assert(index < int(helpers[getHelperName(helperIndex)].size()));

	helpers[getHelperName(helperIndex)][index] = point;
}

void UnitHelpers::deletePoint(int helperIndex, int index)
{
	assert(helpers.find(getHelperName(helperIndex)) != helpers.end());
	assert(index < int(helpers[getHelperName(helperIndex)].size()));

	std::vector<VC2> &points = helpers[getHelperName(helperIndex)];
	points.erase(points.begin() + index);
}

void UnitHelpers::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(1);
	stream << helpers.size();

	HelperList::const_iterator it = helpers.begin();
	for(; it != helpers.end(); ++it)
	{
		stream << it->first;

		const std::vector<VC2> &points = it->second;
		stream << int(points.size());

		for(unsigned int i = 0; i < points.size(); ++i)
			stream << points[i].x << points[i].y;
	}
}

void UnitHelpers::readStream(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;

	if(!version)
		return;

	int helperAmount = 0;
	stream >> helperAmount;

	helpers.clear();
	usedHelpers.clear();

	for(int i = 0; i < helperAmount; ++i)
	{
		std::string name;
		stream >> name;

		int pointAmount = 0;
		stream >> pointAmount;

		std::vector<VC2> &points = helpers[name];
		points.resize(pointAmount);

		for(int j = 0; j < pointAmount; ++j)
			stream >> points[j].x >> points[j].y;
	}
}

//---

void UnitProperties::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(1);

	stream << int(strings.size());
	map<string, string>::const_iterator it = strings.begin();

	for(; it != strings.end(); ++it)
		stream << it->first << it->second;

	stream << int(difficulty);
	stream << layout;
}

void UnitProperties::readStream(filesystem::InputStream &stream)
{
	int version = 1;
	stream >> version;

	int stringAmount = 0;
	stream >> stringAmount;
	
	strings.clear();
	for(int i = 0; i < stringAmount; ++i)
	{
		string key, data;
		stream >> key >> data;

		strings[key] = data;
	}

	int foo = 0;
	stream >> foo;
	difficulty =  static_cast<Difficulty> (foo);
	stream >> layout;
}


//---

UnitHandle::UnitHandle()
{
	unitIndex = -1;
	height = 0;
}

UnitHandle::~UnitHandle()
{
}

const VC2 &UnitHandle::getPosition() const
{
	return position;
}

float UnitHandle::getHeight() const
{
	return height;
}

bool UnitHandle::hasUnit() const
{
	if(unitName.empty())
		return false;
	if(unitIndex == -1)
		return false;

	return true;
}

bool UnitHandle::operator == (const UnitHandle &rhs)
{
	return (unitName == rhs.unitName && unitIndex == rhs.unitIndex);
}

//---

typedef map<string, TerrainUnit> UnitMap;

struct TerrainUnitsData
{
	Storm &storm;
	const UnitScripts &unitScripts;

	std::map<std::string, boost::shared_ptr<IStorm3D_Model> > originalModels;
	UnitMap units;

	boost::shared_ptr<CollisionModel> activeUnit;
	IEditorState &state;

	TerrainUnitsData(Storm &storm_, const UnitScripts &unitScripts_, IEditorState &state_)
	:	storm(storm_),
		unitScripts(unitScripts_),
		state(state_)
	{
	}

	boost::shared_ptr<IStorm3D_Model> getModel(const std::string &fileName)
	{
		boost::shared_ptr<IStorm3D_Model> result = originalModels[fileName];
		if(!result)
		{
			//result = boost::shared_ptr<IStorm3D_Model> (storm.storm->CreateNewModel());
			//if(!fileName.empty())
			//	result->LoadS3D(fileName.c_str());
			
			result = createEditorModel(*storm.storm, fileName);
			originalModels[fileName] = result;
		}

		return result;
	}

	boost::shared_ptr<IStorm3D_Model> getModel(const Unit &unit)
	{
		boost::shared_ptr<IStorm3D_Model> model = getModel(unit.model);
		return model;
	}

	boost::shared_ptr<IStorm3D_Model> getClone(boost::shared_ptr<IStorm3D_Model> original, const Unit &unit)
	{
		boost::shared_ptr<IStorm3D_Model> model(storm.storm->CreateNewModel());
		if(!unit.bones.empty())
			model->LoadBones(unit.bones.c_str());

		addCloneModel(original, model);

		if(!unit.weapon_model.empty())
		{
			boost::shared_ptr<IStorm3D_Model> weapon = getModel(unit.weapon_model);
			addCloneModel(weapon, model, "HELPER_BONE_Weapon");
		}

		model->SetNoCollision(true);
		model->useAlwaysDirectional(true);
		return model;
	}

	void addScene(TerrainUnit &terrainUnit, int index, const std::string &unitName, const Unit &unit)
	{
		boost::shared_ptr<IStorm3D_Model> originalModel = getModel(unit);
		boost::shared_ptr<IStorm3D_Model> model = getClone(originalModel, unit);

		UnitInstance &instance = terrainUnit.instances[index];
		if(instance.model)
			storm.scene->RemoveModel(instance.model.get());

		instance.model = model;

		VC3 position(instance.position.x, 0, instance.position.y);
		if(fabsf(instance.cachedHeight) < 0.001f)
			instance.cachedHeight = storm.getHeight(instance.position);
		position.y = instance.cachedHeight + instance.heightOffset;
		QUAT rotation;
		rotation.MakeFromAngles(0, instance.yRotation, 0);

		if(index == 0)
			terrainUnit.size = getSize(model.get());

		instance.model->SetPosition(position);
		instance.model->SetRotation(rotation);

		updateLighting(*instance.model);
		storm.scene->AddModel(instance.model.get());
	}

	UnitHandle testCollisions(CollisionData &collisionData)
	{
		UnitHandle result;

		std::map<std::string, TerrainUnit>::iterator it = units.begin();
		for(; it != units.end(); ++it)
		{
			TerrainUnit &terrainUnit = it->second;			
			ObjectData objectData;
			objectData.height = terrainUnit.size.y;
			objectData.radiusX = terrainUnit.size.x;
			objectData.radiusZ = terrainUnit.size.z;

			CollisionVolume collisionVolume(objectData);

			for(unsigned int i = 0; i < terrainUnit.instances.size(); ++i)
			{
				UnitInstance &instance = terrainUnit.instances[i];
				VC3 position(instance.position.x, 0, instance.position.y);
				
				if(fabsf(instance.cachedHeight) < 0.001f)
					instance.cachedHeight = storm.getHeight(instance.position);
				
				//position.y = storm.getHeight(instance.position) + instance.heightOffset;
				position.y = instance.cachedHeight + instance.heightOffset;
				
				if(collisionVolume.testCollision(position, VC3(0, -instance.yRotation, 0), collisionData, 0.f))
				{
					result.unitName = it->first;
					result.unitIndex = i;
					result.position = instance.position;
					result.height = instance.heightOffset;
				}
			}
		}

		return result;
	}

	void setScript(int id, ExporterScripts::SubType subType, UnitScripts::ScriptType scriptType, ExporterScripts &exporterScripts, const std::string &scriptName)
	{
		for(int i = 0; i < unitScripts.getScriptCount(scriptType); ++i)
		{
			const Script &script = unitScripts.getScript(scriptType, i);

			if(script.name == scriptName)
			{	
				exporterScripts.setScript(id, subType, script.script);
				return;
			}
		}
	}

	void findUsedHelpers(UnitInstance &instance)
	{
		UnitHelpers &helpers = instance.helpers;
		helpers.usedHelpers.clear();

		const ScriptConfig &scripts = instance.scripts;

		addHelpers(helpers, unitScripts.findScript(UnitScripts::Main, scripts.mainScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Spotted, scripts.spottedScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Alerted, scripts.alertedScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Hit, scripts.hitScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Hitmiss, scripts.hitmissScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Noise, scripts.noiseScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Execute, scripts.executeScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Pointed, scripts.pointedScript));
		addHelpers(helpers, unitScripts.findScript(UnitScripts::Special, scripts.specialScript));

		std::sort(helpers.usedHelpers.begin(), helpers.usedHelpers.end());
	}

	vector<string> findUsedProperties(UnitInstance &instance, const std::string &unitName)
	{
		vector<string> result;
		result.push_back("id");

		const ScriptConfig &scripts = instance.scripts;

		addProperties(result, unitScripts.findScript(UnitScripts::Main, scripts.mainScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Spotted, scripts.spottedScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Alerted, scripts.alertedScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Hit, scripts.hitScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Hitmiss, scripts.hitmissScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Noise, scripts.noiseScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Execute, scripts.executeScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Pointed, scripts.pointedScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Special, scripts.specialScript));

		int unitIndex = unitScripts.getUnitIndex(unitName);
		const Unit &unit = unitScripts.getUnit(unitIndex);

		for(int i = 0; i < unit.stringProperties.getPropertyAmount(); ++i)
		{
			const std::string &name = unit.stringProperties.getProperty(i);

			if(find(result.begin(), result.end(), name) == result.end())
				result.push_back(name);
		}

		sort(result.begin(), result.end());
		return result;
	}

	StringProperties findStringProperties(UnitInstance &instance, const std::string &unitName)
	{
		StringProperties result;
		const ScriptConfig &scripts = instance.scripts;

		addProperties(result, unitScripts.findScript(UnitScripts::Main, scripts.mainScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Spotted, scripts.spottedScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Alerted, scripts.alertedScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Hit, scripts.hitScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Hitmiss, scripts.hitmissScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Noise, scripts.noiseScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Execute, scripts.executeScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Pointed, scripts.pointedScript));
		addProperties(result, unitScripts.findScript(UnitScripts::Special, scripts.specialScript));

		int unitIndex = unitScripts.getUnitIndex(unitName);
		const Unit &unit = unitScripts.getUnit(unitIndex);

		result.add(unit.stringProperties);
		return result;
	}

	void exportHelpers(int id, ExporterScripts &scripts, const UnitHelpers &helpers)
	{
		for(int i = 0; i < helpers.getHelperAmount(); ++i)
		{
			if(!helpers.isWayPoint(i))
				continue;

			int indices = helpers.getPointAmount(i);
			std::vector<VC2> points(indices);

			for(int j = 0; j < indices; ++j)
				points[j] = helpers.getPoint(i, j);

			scripts.addPath(id, helpers.getHelperExportName(i), points);
		}
	}

	const Unit &getUnit(const std::string &name) const
	{
		for(int j = 0; j < unitScripts.getUnitCount(); ++j)
		if(unitScripts.getUnit(j).name == name)
			return unitScripts.getUnit(j);

		static Unit emptyUnit;
		return emptyUnit;
	}

	void addHelpers(UnitHelpers &helpers, const Script *script)
	{
		if(!script)
			return;

		for(unsigned int i = 0; i < script->helpers.size(); ++i)
		{
			UnitHelpers::HelperInfo usedHelper;
			usedHelper.name = script->helpers[i].name;
			usedHelper.exportName = script->helpers[i].exportName;

			if(script->helpers[i].type == ScriptHelper::Path)
				usedHelper.isPath = true;
			else
				usedHelper.isPath = false;

			helpers.usedHelpers.push_back(usedHelper);
		}
	}

	void addProperties(vector<string> &result, const Script *script)
	{
		if(!script)
			return;

		for(int i = 0; i < script->stringProperties.getPropertyAmount(); ++i)
		{
			const std::string &name = script->stringProperties.getProperty(i);

			if(find(result.begin(), result.end(), name) == result.end())
				result.push_back(name);
		}
	}

	void addProperties(StringProperties &result, const Script *script)
	{
		if(!script)
			return;

		result.add(script->stringProperties);
	}

	void updateLighting(IStorm3D_Model &model)
	{
		const VC3 &position = model.GetPosition();
		VC2 position2(position.x, position.z);

		/*
		COL color = state.getColorMap().getColor(position2);
		color += state.getLightMap().getColor(position2);

		VC3 lightPosition;
		COL ambient = color;
		COL lightColor;
		float range = 1.f;

		if(storm.lightManager)
			storm.lightManager->getLighting(color, position, lightPosition, lightColor, range, ambient);

		VC3 sunDirection = state.getSunDirection();
		if(storm.onFloor(position2))
			model.SetDirectional(VC3(), 0.f);
		else
			model.SetDirectional(sunDirection, 1.f);

		model.SetSelfIllumination(ambient);
		model.SetLighting(lightPosition, lightColor, range);
		*/

		ui::PointLights lights;
		lights.ambient = state.getColorMap().getColor(position2) + state.getLightMap().getColor(position2); 
		if(storm.lightManager)
			storm.lightManager->getLighting(position, lights, ui::getRadius(&model), true, true, &model);

		model.SetSelfIllumination(lights.ambient);
		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
			model.SetLighting(i, lights.lightIndices[i]);
			//model.SetLighting(i, lights.lights[i].position, lights.lights[i].color, lights.lights[i].range);

		VC3 sunDirection = state.getSunDirection();
		if(storm.onFloor(position2))
			model.SetDirectional(VC3(), 0.f);
		else
			model.SetDirectional(sunDirection, 1.f);

	}
};

//---

TerrainUnits::TerrainUnits(Storm &storm, const UnitScripts &unitScripts, IEditorState &state)
{
	boost::scoped_ptr<TerrainUnitsData> tempData(new TerrainUnitsData(storm, unitScripts, state));
	data.swap(tempData);
}

TerrainUnits::~TerrainUnits()
{
}

void TerrainUnits::clear()
{
	std::map<std::string, TerrainUnit>::iterator it = data->units.begin();
	for(; it != data->units.end(); ++it)
	{
		TerrainUnit &terrainUnit = it->second;
		for(unsigned int i = 0; i < terrainUnit.instances.size(); ++i)
			data->storm.scene->RemoveModel(terrainUnit.instances[i].model.get());
	}
	
	data->originalModels.clear();
	data->units.clear();

	data->activeUnit.reset();
}

void TerrainUnits::setToTerrain()
{
	std::map<std::string, TerrainUnit>::iterator it = data->units.begin();
	for(; it != data->units.end(); ++it)
	{
		TerrainUnit &terrainUnit = it->second;

		const Unit &unit = data->getUnit(it->first);
		for(unsigned int i = 0; i < terrainUnit.instances.size(); ++i)
			data->addScene(terrainUnit, i, it->first, unit);
	}
}

boost::shared_ptr<IStorm3D_Model> TerrainUnits::getUnitModel(int unitIndex)
{
	const Unit &unit = data->unitScripts.getUnit(unitIndex);

	boost::shared_ptr<IStorm3D_Model> original = data->getModel(unit);
	boost::shared_ptr<IStorm3D_Model> clone = data->getClone(original, unit);

	return clone;
}

Unit TerrainUnits::getUnitSettings(int unitIndex)
{
	return data->unitScripts.getUnit(unitIndex);
}

UnitHandle TerrainUnits::addUnit(int unitIndex, VC3 &position, float yRotation, float height)
{
	const Unit &unit = data->unitScripts.getUnit(unitIndex);
	UnitInstance unitInstance(VC2(position.x, position.z), yRotation);
	unitInstance.heightOffset = height;

	TerrainUnit &terrainUnit = data->units[unit.name];
	terrainUnit.instances.push_back(unitInstance);

	data->addScene(terrainUnit, terrainUnit.instances.size() - 1, unit.name, unit);

	UnitHandle handle;
	handle.unitIndex = terrainUnit.instances.size() - 1;
	handle.unitName = unit.name;

	return handle;
}

void TerrainUnits::rotateUnit(const UnitHandle &unitHandle, int wheelDelta)
{
	if(!unitHandle.hasUnit() || !wheelDelta)
		return;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];

	instance.yRotation = data->storm.unitAligner.getRotation(instance.yRotation, wheelDelta, true);

	QUAT rotation;
	rotation.MakeFromAngles(0, instance.yRotation, 0);
	instance.model->SetRotation(rotation);

	// Update collision

	VC3 position(instance.position.x, 0, instance.position.y);
	if(fabsf(instance.cachedHeight) < 0.001f)
		instance.cachedHeight = data->storm.getHeight(instance.position);
	
	position.y = instance.cachedHeight + instance.heightOffset;
	//position.y = data->storm.getHeight(instance.position) + instance.heightOffset;

	ObjectData objectData;
	objectData.height = terrainUnit.size.y;
	objectData.radiusX = terrainUnit.size.x;
	objectData.radiusZ = terrainUnit.size.z;

	data->activeUnit = boost::shared_ptr<CollisionModel> (new CollisionModel(objectData, COL(1,0,0), position, VC3(0, instance.yRotation, 0), data->storm));
	data->activeUnit->create();
	data->activeUnit->scale();
}

void TerrainUnits::removeUnit(const UnitHandle &unitHandle)
{
	if(!unitHandle.hasUnit())
		return;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	data->storm.scene->RemoveModel(terrainUnit.instances[unitHandle.unitIndex].model.get());

	terrainUnit.instances.erase(terrainUnit.instances.begin() + unitHandle.unitIndex);
}

void TerrainUnits::setHeight(UnitHandle &unitHandle, float height)
{
	if(!unitHandle.hasUnit())
		return;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];

	VC3 position(instance.position.x, 0, instance.position.y);
	instance.heightOffset = height;

	if(fabsf(instance.cachedHeight) < 0.001f)
		instance.cachedHeight = data->storm.getHeight(instance.position);

	//position.y = data->storm.terrain->getHeight(instance.position) + instance.height;
	//position.y = data->storm.getHeight(instance.position) + instance.heightOffset;
	position.y = instance.cachedHeight + instance.heightOffset;
	instance.model->SetPosition(position);

	data->updateLighting(*instance.model);
	unitHandle.height = height;

	// Update collision
	ObjectData objectData;
	objectData.height = terrainUnit.size.y;
	objectData.radiusX = terrainUnit.size.x;
	objectData.radiusZ = terrainUnit.size.z;

	data->activeUnit = boost::shared_ptr<CollisionModel> (new CollisionModel(objectData, COL(1,0,0), position, VC3(0, instance.yRotation, 0), data->storm));
	data->activeUnit->create();
	data->activeUnit->scale();
}

void TerrainUnits::setPosition(UnitHandle &unitHandle, const VC3 &position_)
{
	if(!unitHandle.hasUnit())
		return;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];

	VC3 position = position_;
	instance.position.x = position.x;
	instance.position.y = position.z;

	//if(fabsf(instance.cachedHeight) < 0.001f)
		instance.cachedHeight = data->storm.getHeight(instance.position);

	//position.y = data->storm.getHeight(instance.position) + instance.heightOffset;
	position.y = instance.cachedHeight + instance.heightOffset;
	instance.model->SetPosition(position);

	data->updateLighting(*instance.model);
	unitHandle.position = instance.position;

	// Update collision
	ObjectData objectData;
	objectData.height = terrainUnit.size.y;
	objectData.radiusX = terrainUnit.size.x;
	objectData.radiusZ = terrainUnit.size.z;

	data->activeUnit = boost::shared_ptr<CollisionModel> (new CollisionModel(objectData, COL(1,0,0), position, VC3(0,instance.yRotation,0), data->storm));
	data->activeUnit->create();
	data->activeUnit->scale();
}

UnitHandle TerrainUnits::changeUnitType(const UnitHandle &unitHandle, int newIndex)
{
	if(!unitHandle.hasUnit())
		return UnitHandle();

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance instance = terrainUnit.instances[unitHandle.unitIndex];
	removeUnit(unitHandle);

	instance.model.reset();

	// Create new

	const Unit &unit = data->unitScripts.getUnit(newIndex);
	TerrainUnit &newUnit = data->units[unit.name];
	newUnit.instances.push_back(instance);

	data->addScene(newUnit, newUnit.instances.size() - 1, unit.name, unit);

	UnitHandle handle = unitHandle;
	handle.unitIndex = newUnit.instances.size() - 1;
	handle.unitName = unit.name;

	return handle;
}

UnitHandle TerrainUnits::traceCursor(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength)
{
	CollisionData collisionData;
	collisionData.rayOrigin = rayOrigin;
	collisionData.rayDirection = rayDirection;
	collisionData.rayLength = rayLength;

	UnitHandle result = data->testCollisions(collisionData);
	return result;
}

void TerrainUnits::setActiveUnit(const UnitHandle &unitHandle)
{
	if(!unitHandle.hasUnit())
	{
		data->activeUnit.reset();
		return;
	}

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &unitInstance = terrainUnit.instances[unitHandle.unitIndex];

	VC3 position(unitInstance.position.x, 0, unitInstance.position.y);
	if(fabsf(unitInstance.cachedHeight) < 0.001f)
		unitInstance.cachedHeight = data->storm.getHeight(unitInstance.position);

	//position.y = data->storm.getHeight(unitInstance.position) + unitInstance.heightOffset;
	position.y = unitInstance.cachedHeight + unitInstance.heightOffset;

	ObjectData objectData;
	objectData.height = terrainUnit.size.y;
	objectData.radiusX = terrainUnit.size.x;
	objectData.radiusZ = terrainUnit.size.z;

	data->activeUnit = boost::shared_ptr<CollisionModel> (new CollisionModel(objectData, COL(1,0,0), position, VC3(0,unitInstance.yRotation,0), data->storm));
	data->activeUnit->create();
	data->activeUnit->scale();
}

void TerrainUnits::setUnitScripts(const UnitHandle &unitHandle, const ScriptConfig &scriptConfig)
{
	if(!unitHandle.hasUnit())
		return;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &unitInstance = terrainUnit.instances[unitHandle.unitIndex];

	unitInstance.scripts = scriptConfig;
}

void TerrainUnits::setUnitSide(const UnitHandle &unitHandle, int side)
{
	if(!unitHandle.hasUnit())
		return;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &unitInstance = terrainUnit.instances[unitHandle.unitIndex];

	unitInstance.side = side;
}

std::string TerrainUnits::getUnitName(const UnitHandle &unitHandle) const
{
	if(!unitHandle.hasUnit())
		return "";

	return unitHandle.unitName;
}

Unit TerrainUnits::getUnit(const UnitHandle &unitHandle) const
{
	if(!unitHandle.hasUnit())
		return Unit();

	int unitIndex = data->unitScripts.getUnitIndex(unitHandle.unitName);
	return data->unitScripts.getUnit(unitIndex);
}

ScriptConfig TerrainUnits::getUnitScripts(const UnitHandle &unitHandle) const
{
	if(!unitHandle.hasUnit())
		return ScriptConfig();

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &unitInstance = terrainUnit.instances[unitHandle.unitIndex];
	
	return unitInstance.scripts;
}

int TerrainUnits::getUnitSide(const UnitHandle &unitHandle) const
{
	if(!unitHandle.hasUnit())
		return 0;

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &unitInstance = terrainUnit.instances[unitHandle.unitIndex];

	return unitInstance.side;
}

UnitProperties &TerrainUnits::getUnitProperties(const UnitHandle &unitHandle)
{
	assert(unitHandle.hasUnit());

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];	
	UnitProperties &properties = instance.properties;

	return properties;
}

std::vector<std::string> TerrainUnits::getUnitUsedProperties(const UnitHandle &unitHandle)
{
	assert(unitHandle.hasUnit());
	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];	

	std::vector<std::string> result;
	result = data->findUsedProperties(instance, unitHandle.unitName);

	return result;
}

StringProperties TerrainUnits::getUnitStringProperties(const UnitHandle &unitHandle)
{
	assert(unitHandle.hasUnit());
	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];	

	StringProperties result;
	result = data->findStringProperties(instance, unitHandle.unitName);

	return result;
}

UnitHelpers &TerrainUnits::getUnitHelpers(const UnitHandle &unitHandle)
{
	assert(unitHandle.hasUnit());

	TerrainUnit &terrainUnit = data->units[unitHandle.unitName];
	UnitInstance &instance = terrainUnit.instances[unitHandle.unitIndex];	
	UnitHelpers &helpers = instance.helpers;

	data->findUsedHelpers(instance);
	return helpers;
}

void TerrainUnits::hideUnitsImpl(const char **filters, int filterAmount, bool invert)
{
	UnitMap::iterator it = data->units.begin();
	for(; it != data->units.end(); ++it)
	{
		TerrainUnit &unit = it->second;
		bool filtOk = false;
		if (filters != NULL)
		{
			for (int i = 0; i < filterAmount; i++)
			{
				if (strstr(it->first.c_str(), filters[i]) != NULL)
				{
					filtOk = true;
					break;
				}
			}
		} else {
			filtOk = true;
		}
		if (invert) filtOk = !filtOk;
		if (filtOk)
		{
			for(unsigned int i = 0; i < unit.instances.size(); ++i)
			{
				UnitInstance &instance = unit.instances[i];
				data->storm.scene->RemoveModel(instance.model.get());
			}
		}
	}

	if(data->activeUnit)
		data->activeUnit->hide();
}

void TerrainUnits::showUnitsImpl(const char **filters, int filterAmount, bool invert)
{
	UnitMap::iterator it = data->units.begin();
	for(; it != data->units.end(); ++it)
	{
		TerrainUnit &unit = it->second;
		bool filtOk = false;
		if (filters != NULL)
		{
			for (int i = 0; i < filterAmount; i++)
			{
				if (strstr(it->first.c_str(), filters[i]) != NULL)
				{
					filtOk = true;
					break;
				}
			}
		} else {
			filtOk = true;
		}
		if (invert) filtOk = !filtOk;
		if (filtOk)
		{
			for(unsigned int i = 0; i < unit.instances.size(); ++i)
			{
				UnitInstance &instance = unit.instances[i];
				data->storm.scene->AddModel(instance.model.get());
			}
		}
	}

	if(data->activeUnit)
		data->activeUnit->show();

	updateLighting();
}

void TerrainUnits::hideUnits()
{
	hideUnitsImpl(NULL, 0, false);
}

void TerrainUnits::showUnits()
{
	showUnitsImpl(NULL, 0, false);
}

void TerrainUnits::hideNormalUnits()
{
	const char *filters[2] = { "scripting/", "editor/grid" };
	hideUnitsImpl(filters, 2, true);
}

void TerrainUnits::showNormalUnits()
{
	const char *filters[2] = { "scripting/", "editor/grid" };
	showUnitsImpl(filters, 2, true);
}

void TerrainUnits::hideGridUnits()
{
	const char *filters[1] = { "editor/grid" };
	hideUnitsImpl(filters, 1, false);
}

void TerrainUnits::showGridUnits()
{
	const char *filters[1] = { "editor/grid" };
	showUnitsImpl(filters, 1, false);
}

void TerrainUnits::hideTriggerUnits()
{
	const char *filters[1] = { "scripting/" };
	hideUnitsImpl(filters, 1, false);
}

void TerrainUnits::showTriggerUnits()
{
	const char *filters[1] = { "scripting/" };
	showUnitsImpl(filters, 1, false);
}

void TerrainUnits::updateLighting()
{
	UnitMap::iterator it = data->units.begin();
	for(; it != data->units.end(); ++it)
	{
		TerrainUnit &unit = it->second;
		for(unsigned int i = 0; i < unit.instances.size(); ++i)
		{
			UnitInstance &instance = unit.instances[i];
			if(instance.model)
				data->updateLighting(*instance.model);
		}
	}

	if(data->activeUnit)
		data->activeUnit->show();
}

void TerrainUnits::doExport(Exporter &exporter) const
{
	ExporterUnits &units = exporter.getUnits();
	ExporterScripts &scripts = exporter.getScripts();

	std::map<std::string, TerrainUnit>::iterator it = data->units.begin();
	for(int index = 0; it != data->units.end(); ++it)
	{
		TerrainUnit &terrainUnit = it->second;

		const Unit &unit = data->getUnit(it->first);
		if(unit.name.empty())
			continue;

		for(unsigned int j = 0; j < terrainUnit.instances.size(); ++j)
		{
			UnitInstance &instance = terrainUnit.instances[j];
			std::string scriptName = std::string("script") + convertToString<int> (++index);

			StringProperties stringProperties = data->findStringProperties(instance, it->first);

			int id = scripts.addScript(scriptName);
			data->setScript(id, ExporterScripts::Main, UnitScripts::Main, scripts, instance.scripts.mainScript);
			data->setScript(id, ExporterScripts::Spotted, UnitScripts::Spotted, scripts, instance.scripts.spottedScript);
			data->setScript(id, ExporterScripts::Alerted, UnitScripts::Alerted, scripts, instance.scripts.alertedScript);
			data->setScript(id, ExporterScripts::Hit, UnitScripts::Hit, scripts, instance.scripts.hitScript);
			data->setScript(id, ExporterScripts::Hitmiss, UnitScripts::Hitmiss, scripts, instance.scripts.hitmissScript);
			data->setScript(id, ExporterScripts::Noise, UnitScripts::Noise, scripts, instance.scripts.noiseScript);
			data->setScript(id, ExporterScripts::Execute, UnitScripts::Execute, scripts, instance.scripts.executeScript);
			data->setScript(id, ExporterScripts::Pointed, UnitScripts::Pointed, scripts, instance.scripts.pointedScript);
			data->setScript(id, ExporterScripts::Special, UnitScripts::Special, scripts, instance.scripts.specialScript);

			data->findUsedHelpers(instance);

			scripts.setInitialization(id, unit.initialization);
			scripts.setNoScript(id, unit.noScript);
			scripts.setProperties(id, instance.properties);
			scripts.setProperties(id, stringProperties);
			data->exportHelpers(id, scripts, instance.helpers);

#ifdef LEGACY_FILES
			if(unit.name == "C, Player spawnpoint")
#else
			if(unit.name == "C, scripting/spawnpoint")
#endif
			{
				units.setSpawn(convertSceneToMap(instance.position, data->storm.heightmapResolution, data->storm.heightmapSize));
			}

			//float height = data->storm.terrain->getHeight(VC2(instance.position)) + instance.height;
			if(fabsf(instance.cachedHeight) < 0.001f)
				instance.cachedHeight = data->storm.getHeight(instance.position);

			float height = instance.cachedHeight + instance.heightOffset;
			units.addUnit(unit.spawnText, instance.position, instance.yRotation, height, scriptName, instance.side, instance.properties, stringProperties);
		}
	}
}

filesystem::OutputStream &TerrainUnits::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(8);
	stream << int(data->units.size());

	std::map<std::string, TerrainUnit>::iterator it = data->units.begin();
	for(; it != data->units.end(); ++it)
	{
		TerrainUnit &terrainUnit = it->second;
		
		stream << it->first;
		stream << int(terrainUnit.instances.size());

		for(unsigned int i = 0; i < terrainUnit.instances.size(); ++i)
		{
			UnitInstance &instance = terrainUnit.instances[i];

			stream << instance.position.x << instance.position.y << instance.yRotation;
			stream << instance.heightOffset;
			stream << instance.side;

			stream << instance.scripts.mainScript;
			stream << instance.scripts.spottedScript;
			stream << instance.scripts.alertedScript;
			stream << instance.scripts.hitScript;
			stream << instance.scripts.hitmissScript;
			stream << instance.scripts.noiseScript;
			stream << instance.scripts.executeScript;
			stream << instance.scripts.pointedScript;
			stream << instance.scripts.specialScript;

			instance.helpers.writeStream(stream);
			instance.properties.writeStream(stream);

			stream << (unsigned int)(instance.uniqueEditorObjectHandle & (((UniqueEditorObjectHandle)1<<32)-1));
			stream << (unsigned int)(instance.uniqueEditorObjectHandle >> (UniqueEditorObjectHandle)32);
		}
	}

	return stream;
}

filesystem::InputStream &TerrainUnits::readStream(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;

	int unitAmount = 0;
	stream >>unitAmount;

	for(int i = 0; i < unitAmount; ++i)
	{
		std::string unitName;
		stream >> unitName;

		int instanceAmount = 0;
		stream >> instanceAmount;

		TerrainUnit &terrainUnit = data->units[unitName];
		terrainUnit.instances.resize(instanceAmount);

		for(int j = 0; j < instanceAmount; ++j)
		{
			UnitInstance &instance = terrainUnit.instances[j];

			stream >> instance.position.x >> instance.position.y >> instance.yRotation;
			if(version >= 6)
				stream >> instance.heightOffset;
			if(version >= 3)
				stream >> instance.side;

			stream >> instance.scripts.mainScript;
			stream >> instance.scripts.spottedScript;
			stream >> instance.scripts.alertedScript;
			stream >> instance.scripts.hitScript;
			stream >> instance.scripts.hitmissScript;
			stream >> instance.scripts.noiseScript;
			if(version >= 4)
				stream >> instance.scripts.executeScript;
			if(version >= 5)
			{
				stream >> instance.scripts.pointedScript;
				stream >> instance.scripts.specialScript;
			}

			if(version >= 2)
				instance.helpers.readStream(stream);

			if(version >= 7)
				instance.properties.readStream(stream);

			if(version >= 8)
			{
				unsigned int lower;
				unsigned int upper;
				stream >> lower;
				stream >> upper;
				instance.uniqueEditorObjectHandle = 0;
				instance.uniqueEditorObjectHandle |= (UniqueEditorObjectHandle)lower;
				instance.uniqueEditorObjectHandle |= ((UniqueEditorObjectHandle)upper << 32);
			}

		}
	}

	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
