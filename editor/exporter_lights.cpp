// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "exporter_lights.h"
#include "export_options.h"
#include "../ui/lightmanager.h"
#include <vector>
#include <fstream>
#include <cassert>

using namespace boost;
using namespace std;
using namespace ui;

namespace frozenbyte {
namespace editor {
namespace {

struct Spot
{
	VC3 position;
	float yAngle;
	bool pointLight;

	SpotProperties properties;

	Spot()
	:	yAngle(0),
		pointLight(true)
	{
	}
};

struct Point
{
	VC3 position;
	COL color;
	float range;

	Point()
	:	range(1.f)
	{
	}
};

typedef vector<Spot> SpotList;
typedef vector<Point> PointList;

string makeString(SpotProperties::Type type)
{
	if(type == SpotProperties::Lighting)
		return "lighting";
	else if(type == SpotProperties::ShadowCaster)
		return "shadow_caster";
	else
		assert(!"Invalid light type");

	return "";
}

} // unnamed

struct ExporterLights::Data
{
	SpotList spots;
	PointList buildingPoints;

	void exportSpots(ostream &stream)
	{
		{
			SpotList::iterator it = spots.begin();
			for(; it != spots.end(); ++it)
			{
				const Spot &spot = *it;
				const VC3 &pos = spot.position;
				const SpotProperties &p = spot.properties;

				stream.setf(ios::showpoint);

				// Basic
				stream << "   setPosition s," << pos.x << ", " << pos.z << endl;
				//stream << "   setPositionHeightOnGround " << endl;
				stream << "   setPositionHeight " << pos.y << endl;
				stream << "   setLightAngleY " << spot.yAngle << endl;

				// Properties
				stream << "   setLightType " << makeString(p.type) << endl;
				stream << "   setLightRange " << p.range << endl;
				stream << "   setLightFOV " << p.fov << endl;
				stream << "   setLightHeight " << p.height + pos.y << endl;
				stream << "   setLightAngleX " << p.angle << endl;
				stream << "   setLightColorRed " << p.color.r << endl;
				stream << "   setLightColorGreen " << p.color.g << endl;
				stream << "   setLightColorBlue " << p.color.b << endl;
				stream << "   setLightGroup " << p.group << endl;
				stream << "   setLightPriority " << p.priority << endl;
				stream << "   setLightBlink " << p.blink << endl;
				stream << "   setLightBlinkTime " << p.blinkTime << endl;
				stream << "   setLightRotate " << p.rotate << endl;
				stream << "   setLightRotateRange " << p.rotateRange << endl;
				stream << "   setLightRotateTime " << p.rotateTime << endl;
				stream << "   setLightFade " << p.fade << endl;
				stream << "   setLightFadeTime " << p.fadeTime << endl;
				stream << "   setLightCone " << p.cone << endl;
				stream << "   setLightShadows " << p.shadow << endl;
				stream << "   setLightTexture \"" << p.texture << "\"" << endl;
				stream << "   setLightConeTexture \"" << p.coneTexture << "\"" << endl;
				stream << "   setLightModel \"" << p.lightModel << "\"" << endl;
				stream << "   setLightMinPlaneX " << p.minPlane.x << endl;
				stream << "   setLightMinPlaneY " << p.minPlane.y << endl;
				stream << "   setLightMaxPlaneX " << p.maxPlane.x << endl;
				stream << "   setLightMaxPlaneY " << p.maxPlane.y << endl;
				stream << "   setLightStrength " << p.strength << endl;
				stream << "   setLightSourceHeight " << p.sourceHeight << endl;
				stream << "   setLightDisableObjectRendering " << p.disableObjectRendering << endl;
				stream << "   setLightPointLight " << spot.pointLight << endl;
				stream << "   setLightLightingModelType " << (int)(p.lightingModelType) << endl;
				stream << "   setLightLightingModelFade " << p.lightingModelFade << endl;

				stream << "   addLight" << endl;
				stream << endl;
			}
		}

		{
			PointList::iterator it = buildingPoints.begin();
			for(; it != buildingPoints.end(); ++it)
			{
				const Point &point = *it;
				const VC3 &pos = point.position;

				stream << "   setPosition s," << pos.x << ", " << pos.z << endl;
				stream << "   setPositionHeight " << pos.y << endl;
				stream << "   setLightRange " << point.range << endl;
				stream << "   setLightColorRed " << point.color.r << endl;
				stream << "   setLightColorGreen " << point.color.g << endl;
				stream << "   setLightColorBlue " << point.color.b << endl;
				stream << "   addBuildingLight" << endl;
				stream << endl;
			}
		}
	}

	void save(const std::string &fileName, const std::string &id)
	{
		ofstream stream(fileName.c_str());

#ifdef LEGACY_FILES
		stream << "script " << id << "_lights" << endl;
#else
		stream << "#!dhs -nopp" << endl << endl;
		stream << "script " << id << "_lights" << endl << endl;
#endif
		stream << "sub addlights" << endl;

		exportSpots(stream);

		stream << "endSub" << endl;
		stream << "endScript" << endl;
	}
};

ExporterLights::ExporterLights()
{
	scoped_ptr<Data> tempData(new Data());
	data.swap(tempData);
}

ExporterLights::~ExporterLights()
{
}

void ExporterLights::addSpot(const VC3 &position, float yAngle, const ui::SpotProperties &properties, bool pointLight)
{
	Spot spot;
	spot.position = position;
	spot.yAngle = yAngle;
	spot.properties = properties;
	spot.pointLight = pointLight;

	data->spots.push_back(spot);
}

void ExporterLights::addBuildingLight(const VC3 &position, const COL &color, float range)
{
	Point point;
	point.position = position;
	point.color = color;
	point.range = range;

	data->buildingPoints.push_back(point);
}

void ExporterLights::save(const ExportOptions &options) const
{
	std::string fileName = options.fileName + std::string("\\") + options.id + std::string("_lights.dhs");
	data->save(fileName, options.id);
}

} // editor
} // frozenbyte
