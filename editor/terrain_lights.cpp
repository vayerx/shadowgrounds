// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_lights.h"
#include "storm.h"
#include "storm_geometry.h"
#include "object_settings.h"
#include "collision_model.h"
#include "exporter.h"
#include "exporter_lights.h"
#include "storm_texture.h"
#include "ieditor_state.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../ui/lightmanager.h"
#include "UniqueEditorObjectHandle.h"
#include "UniqueEditorObjectHandleManager.h"
#include <istorm3d_terrain.h>
#include <istorm3d_model.h>
#include <istorm3d.h>
#include <vector>
#include <deque>
#include <time.h>

using namespace boost;
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

bool shadows = true;

struct Visualization
{
	shared_ptr<IStorm3D_Model> model;
};

struct Spot
{
	VC3 position;
	float yAngle;

	TerrainLights::SpotProperties properties;
	Visualization visualization;

	UniqueEditorObjectHandle uniqueEditorObjectHandle;

	Spot()
	:	yAngle(0)
	{
		//unsigned int internalHandle = time(NULL);
		unsigned int internalHandle = _time32(NULL);

		uniqueEditorObjectHandle = UniqueEditorObjectHandleManager::createNewUniqueHandle(internalHandle);
		uniqueEditorObjectHandle = 0;
	}

	void setVisualization(const Visualization &visualization_)
	{
		visualization = visualization_;
		if(visualization.model)
		{
			IStorm3D_Model &model = *visualization.model;
			model.SetPosition(position);
		}

		setRotation(yAngle);
	}

	void setRotation(float rotation)
	{
		yAngle = rotation;

		if(visualization.model)
			rotateModel(visualization.model, yAngle);
	}
};

typedef vector<Spot> SpotList;
typedef deque<bool> BoolList;

} // unnamed

TerrainLights::SpotProperties::SpotProperties()
:	type(Lighting),
	range(0),
	fov(0),
	height(0),
	heightOffset(0),
	angle(0),
	color(1.f, 1.f, 1.f),

	group(0),
	priority(1),

	blink(false),
	blinkTime(0),
	rotate(false),
	rotateRange(0),
	rotateTime(0),
	fade(false),
	fadeTime(0),
	cone(0),
	smoothness(5.f),
	shadow(true),

	minPlane(0,0),
	maxPlane(1.f, 1.f),
	strength(.2f),
	sourceHeight(0),

	lightMapped(false),
	pointLight(true),
	building(false),

	lightingModelType(Pointlight),
	lightingModelFade(true)
{
}

struct TerrainLights::Data
{
	SpotList spots;
	Storm &storm;
	IEditorState &state;

	int active;
	bool visualization;
	int shadowLevel;
	BoolList groups;

	Data(Storm &storm_, IEditorState &state_)
	:	storm(storm_),
		state(state_),
		active(-1),
		visualization(false),
		shadowLevel(100),
		groups(16, false)
	{
		groups[0] = true;
	}

	void setPosition(int index, const VC2 &position)
	{
		Spot &spot = spots[index];
		spot.position.x = position.x;
		spot.position.z = position.y;

	}

	int trace(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength)
	{
		ObjectData object;
		object.radiusX = .5f;
		object.radiusZ = .5f;

		CollisionVolume volume(object);
		CollisionData data;
		data.rayOrigin = rayOrigin;
		data.rayDirection = rayDirection;
		data.rayLength = rayLength;

		int index = -1;
		for(unsigned int i = 0; i < spots.size(); ++i)
		{
			const Spot &spot = spots[i];
			VC3 position = spot.position;
			position.y += spot.properties.height + spot.properties.heightOffset;

			if(volume.testCollision(position, VC3(), data, 0.f))
				index = i;
		}

		return index;
	}

	void updateSpot(int index)
	{
		if(index == -1)
			return;

		VisualizationType type = Inactive;
		if(index == active)
			type = Active;

		Spot &spot = spots[index];
		if(storm.terrain)
		{
			//spot.position.y = storm.terrain->getHeight(VC2(spot.position.x, spot.position.z));
			spot.position.y = storm.getHeight(VC2(spot.position.x, spot.position.z));
		}

		spot.setVisualization(getVisualization(spot.properties, type));
	}

	void setActive(int index)
	{
		assert(index >= -1 && index < int(spots.size()));
		if(active == index)
			return;

		if(active >= 0)
		{
			Spot &oldSpot = spots[active];
			oldSpot.setVisualization(getVisualization(oldSpot.properties, Inactive));
		}
		if(index >= 0)
		{
			Spot &newSpot = spots[index];
			newSpot.setVisualization(getVisualization(newSpot.properties, Active));
		}

		active = index;
	}

	void reset()
	{
		spots.clear();
	}

	void update()
	{
		for(unsigned int i = 0; i < spots.size(); ++i)
			updateSpot(i);
	}

	void redrawSpots()
	{
		if(!storm.lightManager)
			return;

		storm.lightManager->clearSpots();
		storm.lightManager->clearLights();

		SpotList::iterator it = spots.begin();
		for(; it != spots.end(); ++it)
		{
			const Spot &spot = *it;

			if(spot.properties.type != SpotProperties::ShadowCaster || spot.properties.shadow)
				storm.lightManager->addSpot(spot.position, spot.yAngle, convertProperties(spot.properties));

			if(spot.properties.pointLight && spot.properties.type == SpotProperties::ShadowCaster)
			{
				ui::Light light;
				light.position = spot.position + VC3(0, spot.properties.sourceHeight + spot.properties.heightOffset, 0);
				light.range = spot.properties.range;
				light.minPlane = spot.properties.minPlane;
				light.maxPlane = spot.properties.maxPlane;
				light.color = spot.properties.color;

				light.minPlane *= -light.range;
				light.maxPlane *= light.range;

				light.minPlane.x += light.position.x;
				light.minPlane.y += light.position.z;
				light.maxPlane.x += light.position.x;
				light.maxPlane.y += light.position.z;

				storm.lightManager->addLight(light);
			}
		}

		for(unsigned int i = 0; i < groups.size(); ++i)
			storm.lightManager->enableGroup(i, groups[i]);

		storm.lightManager->setShadowLevel(shadowLevel);
		//state.updateLighting();
	}

	void enableGroup(int group, bool enable)
	{
		groups[group] = enable;
		redrawSpots();
	}

	void setShadowLevel(int level)
	{
		shadowLevel = level;
		redrawSpots();
	}

	enum VisualizationType
	{
		Active,
		Inactive
	};

	shared_ptr<IStorm3D_Model> getModel(const SpotProperties &properties, const COL &colorMul, VisualizationType type) const
	{
		shared_ptr<IStorm3D_Model> model(storm.storm->CreateNewModel());
		model->CastShadows(false);
		model->SetNoCollision(true);

		if(type != Active && !visualization)
			return model;

		if(type == Active)
			model->SetSelfIllumination(COL(.5f, .5f, .5f));
		else
			model->SetSelfIllumination(COL(.35f, .35f, .35f));

		IStorm3D_Mesh *red = createWireframeObject(storm, model.get(), COL(.5f, 0, 0) * colorMul, "...");
		IStorm3D_Mesh *green = createWireframeObject(storm, model.get(), COL(0, .5f, 0) * colorMul, "...");
		IStorm3D_Mesh *blue = createWireframeObject(storm, model.get(), COL(0, 0, .5f) * colorMul, "...");
		IStorm3D_Mesh *yellow = createWireframeObject(storm, model.get(), COL(.5f, .5f, 0) * colorMul, "...");

		// Height lines
		if(properties.height > 0.01f)
		{
			addLine(red, VC3(), VC3(0, properties.height + properties.heightOffset, 0), 0.01f, VC3(1.f, 0, 0));
			addLine(red, VC3(), VC3(0, properties.height + properties.heightOffset, 0), 0.01f, VC3(0, 0, 1.f));
		}

		if(properties.type == SpotProperties::ShadowCaster)
		{
			float xmin = -properties.range * properties.minPlane.x;
			float xmax =  properties.range * properties.maxPlane.x;
			float x = (xmin + xmax) * .5f;
			float zmin = -properties.range * properties.minPlane.y;
			float zmax =  properties.range * properties.maxPlane.y;
			float z = (zmin + zmax) * .5f;
			float thickness = xmax - xmin;

			VC3 minValue(x, .4f, zmin);
			VC3 maxValue(x, .4f, zmax);

			// Projection plane
			addLine(red, minValue, maxValue, thickness, VC3(0, 1.f, 0));

			if(type == Active)
			{
				// Helper boxes
				VC3 r(xmin, .4f, z);
				VC3 b(xmax, .4f, z);

				addBox(red, r, 0.15f);
				addBox(green, minValue, 0.15f);
				addBox(blue, b, 0.15f);
				addBox(yellow, maxValue, 0.15f);
			}

			// Light source
			IStorm3D_Mesh *lightBlue = createWireframeObject(storm, model.get(), COL(.12f, .12f, .5f) * colorMul, "...");
			addBox(lightBlue, VC3(0, properties.sourceHeight +  properties.heightOffset, 0), .125f);
		}

		// Cone
		if(properties.type != SpotProperties::ShadowCaster)
		{
			IStorm3D_Mesh *cone = createWireframeObject(storm, model.get(), COL(.5f, .5f, .5f) * colorMul, "cone");
			
			VC3 start = VC3(0, properties.height + properties.heightOffset, 0);
			addCone(cone, start, properties.angle, properties.fov * .9f, properties.range, 20);
		}

		// Collision box
		addBox(red, VC3(0, properties.height + properties.heightOffset, 0), .25f);
		return model;
	}

	Visualization getVisualization(const SpotProperties &properties, VisualizationType type) const
	{
		COL color(1.f, 1.f, 1.f);
		if(type == Active)
			color = COL(2.f, 2.f, 2.f);

		Visualization result;
		result.model = getModel(properties, color, type);
		storm.scene->AddModel(result.model.get());

		return result;
	}

	void save(filesystem::OutputStream &stream) const
	{
		int version = 15;
		stream << version;
		stream << int(spots.size());

		SpotList::const_iterator it = spots.begin();
		for(; it != spots.end(); ++it)
		{
			const Spot &spot = *it;
			stream << spot.position.x << spot.position.z << spot.yAngle;

			const SpotProperties &p = spot.properties;
			stream << int(p.type);
			stream << p.range << p.fov << p.height << p.angle;
			stream << p.color.r << p.color.g << p.color.b;
			stream << p.group << p.priority;
			stream << p.texture;
			stream << p.coneTexture;
			stream << p.lightModel;

			stream << p.blink << p.blinkTime;
			stream << p.rotate << p.rotateTime << p.rotateRange;
			stream << p.fade << p.fadeTime;
			stream << p.cone << p.shadow;

			stream << p.minPlane.x << p.minPlane.y;
			stream << p.maxPlane.x << p.maxPlane.y;
			stream << p.strength;
			stream << p.sourceHeight;
			stream << p.lightMapped;
			stream << p.pointLight;
			stream << p.building;
			stream << p.heightOffset;

			stream << (unsigned int)(spot.uniqueEditorObjectHandle & (((UniqueEditorObjectHandle)1<<32)-1));
			stream << (unsigned int)(spot.uniqueEditorObjectHandle >> (UniqueEditorObjectHandle)32);
		}
	}

	void load(filesystem::InputStream &stream)
	{
		reset();
		spots.clear();

		int version = 0;
		stream >> version;

		int spotAmount = 0;
		stream >> spotAmount;

		for(int i = 0; i < spotAmount; ++i)
		{
			Spot spot;
			stream >> spot.position.x >> spot.position.z >> spot.yAngle;

			SpotProperties &p = spot.properties;

			int type = 0;
			stream >> type;
			p.type = (SpotProperties::Type) type;

			stream >> p.range >> p.fov >> p.height >> p.angle;
			stream >> p.color.r >> p.color.g >> p.color.b;
			stream >> p.group >> p.priority;
			if(version >= 3)
				stream >> p.texture;
			if(version >= 7)
				stream >> p.coneTexture;
			if(version >= 8)
				stream >> p.lightModel;

			stream >> p.blink >> p.blinkTime;
			stream >> p.rotate >> p.rotateTime;
			if(version >= 5)
				stream >> p.rotateRange;
			stream >> p.fade >> p.fadeTime;
			if(version >= 2 && version <= 5)
			{
				bool cone;
				stream >> cone;

				if(cone)
					p.cone = 0.3f;
				else
					p.cone = 0;
			}
			if(version >= 6)
			{
				stream >> p.cone;
			}
			if(version >= 4)
				stream >> p.shadow;

			stream >> p.minPlane.x >> p.minPlane.y;
			stream >> p.maxPlane.x >> p.maxPlane.y;

			if(version >= 9)
				stream >> p.strength;

			if(version >= 11)
				stream >> p.sourceHeight;
			else
				p.sourceHeight = p.height;

			if(version >= 10)
				stream >> p.lightMapped;
			if(version >= 12)
				stream >> p.pointLight;
			if(version >= 13)
				stream >> p.building;
			if(version >= 14)
				stream >> p.heightOffset;

			if(version >= 15)
			{
				unsigned int lower;
				unsigned int upper;
				stream >> lower;
				stream >> upper;
				spot.uniqueEditorObjectHandle = 0;
				spot.uniqueEditorObjectHandle |= (UniqueEditorObjectHandle)lower;
				spot.uniqueEditorObjectHandle |= ((UniqueEditorObjectHandle)upper << 32);
			}

			spots.push_back(spot);
		}

		update();
	}
};

TerrainLights::TerrainLights(Storm &storm, IEditorState &state)
{
	scoped_ptr<Data> tempData(new Data(storm, state));
	tempData.swap(data);
}

TerrainLights::~TerrainLights()
{
}

int TerrainLights::addSpot(const VC2 &position, const SpotProperties &properties)
{
	int index = data->spots.size();
	data->spots.resize(index + 1);

	moveSpot(index, position);
	setProperties(index, properties);

	Spot &spot = data->spots[index];
	spot.setVisualization(data->getVisualization(properties, Data::Inactive));

	data->redrawSpots();
	return index;
}

shared_ptr<IStorm3D_Model> TerrainLights::getModel(const SpotProperties &properties, const COL &colorMul) const
{
	return data->getModel(properties, colorMul, Data::Active);
}

int TerrainLights::getSpotAmount() const
{
	return data->spots.size();
}

const TerrainLights::SpotProperties &TerrainLights::getProperties(int index) const
{
	assert(index >= 0 && index < getSpotAmount());
	return data->spots[index].properties;
}

VC2 TerrainLights::getPosition(int index) const
{
	assert(index >= 0 && index < getSpotAmount());
	const VC3 &pos = data->spots[index].position;

	return VC2(pos.x, pos.z);
}

float TerrainLights::getRotation(int index) const
{
	assert(index >= 0 && index < getSpotAmount());
	return data->spots[index].yAngle;
}

void TerrainLights::setProperties(int index, const SpotProperties &properties)
{
	assert(index >= 0 && index < getSpotAmount());
	data->spots[index].properties = properties;
	
	data->updateSpot(index);

	data->redrawSpots();
}

void TerrainLights::moveSpot(int index, const VC2 &position)
{
	assert(index >= 0 && index < getSpotAmount());
	data->setPosition(index, position);

	data->updateSpot(index);

	data->redrawSpots();
}

void TerrainLights::setRotation(int index, float rotation)
{
	assert(index >= 0 && index < getSpotAmount());
	data->spots[index].setRotation(rotation);

	data->redrawSpots();
}

void TerrainLights::deleteSpot(int index)
{
	assert(index >= 0 && index < getSpotAmount());
	data->spots.erase(data->spots.begin() + index);

	data->redrawSpots();
}

int TerrainLights::traceActiveCollision(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength)
{
	return data->trace(rayOrigin, rayDirection, rayLength);
}

void TerrainLights::setActiveLight(int index)
{
	data->setActive(index);
}

void TerrainLights::reset()
{
	data->reset();
}

void TerrainLights::update()
{
	data->update();

	data->redrawSpots();
}

void TerrainLights::setProperty(BoolProperty property, bool value)
{
	if(property == RenderShadows)
		shadows = value;
	else if(property == RenderVisualization)
		data->visualization = value;

	update();
}

void TerrainLights::enableGroup(int group, bool enable)
{
	data->enableGroup(group, enable);
}

void TerrainLights::setShadowLevel(int level)
{
	data->setShadowLevel(level);
}

bool TerrainLights::isGroupEnabled(int group) const
{
	return data->groups[group];
}

void TerrainLights::removeLights()
{
	if(!data->storm.lightManager)
		return;

	data->storm.lightManager->clearSpots();
	data->storm.lightManager->clearLights();
}

void TerrainLights::nudgeLights(const VC3 &position, const VC3 &direction, float radius)
{
	float radiusSq = radius * radius;
	SpotList::iterator it = data->spots.begin();
	for(; it != data->spots.end(); ++it)
	{
		Spot &spot = *it;
		VC3 diffVec = spot.position - position;
		if (diffVec.GetSquareLength() < radiusSq)
		{
			spot.position += direction;
		}
	}
	update();
}

void TerrainLights::getLights(std::vector<TerrainLightMap::PointLight> &lights, bool onlyBuilding) const
{
	SpotList::iterator it = data->spots.begin();
	for(; it != data->spots.end(); ++it)
	{
		Spot &spot = *it;
		if(spot.properties.type != SpotProperties::ShadowCaster)
			continue;

		if(onlyBuilding)
		{
			if(!spot.properties.building)
				continue;
		}
		else
		{
			if(!spot.properties.lightMapped)
				continue;
		}

		TerrainLightMap::PointLight light;
		light.position = spot.position + VC3(0, spot.properties.height + spot.properties.heightOffset, 0);
		light.range = spot.properties.range;
		light.strength = spot.properties.strength;
		light.color = spot.properties.color;

		lights.push_back(light);
	}
}

void TerrainLights::doExport(Exporter &exporter) const
{
	ExporterLights &lights = exporter.getLights();

	SpotList::iterator it = data->spots.begin();
	for(; it != data->spots.end(); ++it)
	{
		Spot &spot = *it;
		lights.addSpot(spot.position, spot.yAngle, convertProperties(spot.properties), spot.properties.pointLight);
	}

	std::vector<TerrainLightMap::PointLight> buildingLights;
	getLights(buildingLights, true);

	for(unsigned int i = 0; i < buildingLights.size(); ++i)
	{
		const TerrainLightMap::PointLight &l = buildingLights[i];
		lights.addBuildingLight(l.position, l.color, l.range);
	}
}

filesystem::OutputStream &TerrainLights::writeStream(filesystem::OutputStream &stream) const
{
	data->save(stream);
	return stream;
}

filesystem::InputStream &TerrainLights::readStream(filesystem::InputStream &stream)
{
	data->load(stream);
	return stream;
}

void rotateModel(boost::shared_ptr<IStorm3D_Model> model, float yRotation)
{
	IStorm3D_Model_Object *o = model->SearchObject("cone");
	if(!o)
		return;

	QUAT q;
	q.MakeFromAngles(0, yRotation, 0);
	o->SetRotation(q);
}

ui::SpotProperties convertProperties(const TerrainLights::SpotProperties &properties)
{
	ui::SpotProperties result;
	result.type = (ui::SpotProperties::Type) properties.type;

	result.range = properties.range;
	result.fov = properties.fov;
	result.height = properties.height + properties.heightOffset;
	result.angle = properties.angle;
	result.color = properties.color;
	result.group = properties.group;
	result.priority = properties.priority;

	result.blink = properties.blink;
	result.blinkTime = properties.blinkTime;
	result.rotate = properties.rotate;
	result.rotateRange = properties.rotateRange;
	result.rotateTime = properties.rotateTime;
	result.fade = properties.fade;
	result.fadeTime = properties.fadeTime;
	result.cone = properties.cone;
	result.smoothness = properties.smoothness;
	result.shadow = properties.shadow;
	if(!shadows)
		result.shadow = false;

	if(result.type == ui::SpotProperties::ShadowCaster)
	{
		if(properties.lightMapped)
			result.disableObjectRendering = true;
	}

	result.texture = properties.texture;
	result.coneTexture = properties.coneTexture;
	result.lightModel = properties.lightModel;

	result.minPlane = properties.minPlane;
	result.maxPlane = properties.maxPlane;
	result.strength = properties.strength;
	result.sourceHeight = properties.sourceHeight + properties.heightOffset;

	result.lightingModelType = (ui::SpotProperties::LightingModelType)properties.lightingModelType;
	result.lightingModelFade = properties.lightingModelFade;
	return result;
}

} // editor
} // frozenbyte
