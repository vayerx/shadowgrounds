// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_OBJECTS
#define INCLUDED_EDITOR_TERRAIN_OBJECTS

#include "terrain_lightmap.h"
#include <vector>

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif
#include <istorm3d_model.h>

#include <boost/shared_ptr.hpp>
#include <string>

namespace ui {
	struct SpotProperties;
} // ui

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

struct Storm;
class Exporter;
class IEditorState;

class TerrainLights
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:

	struct SpotProperties
	{
		enum Type
		{
			Lighting = 0,
			ShadowCaster = 1
		};

		Type type;

		float range;
		float fov;
		float height;
		float heightOffset;

		float angle;
		COL color;

		int group;
		int priority;
		std::string texture;
		std::string coneTexture;
		std::string lightModel;

		bool blink;
		int blinkTime;
		bool rotate;
		float rotateRange;
		int rotateTime;
		bool fade;
		int fadeTime;
		float cone;
		float smoothness;
		bool shadow;

		VC2 minPlane;
		VC2 maxPlane;
		float strength;
		float sourceHeight;

		bool lightMapped;
		bool pointLight;
		bool building;

		enum LightingModelType
		{
			Flat = 0,
			Pointlight = 1,
			Directional = 2,
			NumLightingModelTypes = 3
		};
		LightingModelType lightingModelType;
		bool lightingModelFade;

		SpotProperties();
	};

	TerrainLights(Storm &storm, IEditorState &state);
	~TerrainLights();

	int addSpot(const VC2 &position, const SpotProperties &properties);
	boost::shared_ptr<IStorm3D_Model> getModel(const SpotProperties &properties, const COL &colorMul) const;

	int getSpotAmount() const;
	const SpotProperties &getProperties(int index) const;
	VC2 getPosition(int index) const;
	float getRotation(int index) const;
	void setProperties(int index, const SpotProperties &properties);
	void moveSpot(int index, const VC2 &position);
	void setRotation(int index, float rotation);
	void deleteSpot(int index);

	int traceActiveCollision(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength);
	void setActiveLight(int index);
	void reset();
	void update();

	enum BoolProperty
	{
		RenderVisualization,
		RenderShadows
	};

	void setProperty(BoolProperty property, bool value);
	void enableGroup(int group, bool enable);
	void setShadowLevel(int level);
	bool isGroupEnabled(int group) const;

	void removeLights();
	void getLights(std::vector<TerrainLightMap::PointLight> &lights, bool onlyBuilding) const;

	void nudgeLights(const VC3 &position, const VC3 &direction, float radius);

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainLights &lights)
{ 
	return lights.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainLights &lights)
{ 
	return lights.readStream(stream);
}

ui::SpotProperties convertProperties(const TerrainLights::SpotProperties &properties);
void rotateModel(boost::shared_ptr<IStorm3D_Model> model, float yRotation);

} // end of namespace editor
} // end of namespace frozenbyte

#endif
