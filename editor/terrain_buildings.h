// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_BUILDINGS_H
#define INCLUDED_EDITOR_TERRAIN_BUILDINGS_H

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

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

class Building
{
	int index;
	std::string fileName;
	float rotation;

public:
	Building()
		:	index(-1), rotation(0) {}

	float getRotation() 
	{
		return rotation;
	}

	friend struct TerrainBuildingsData;
};

class Exporter;
struct HeightmapData;
struct Storm;
struct TerrainBuildingsData;
class Mouse;
class IEditorState;

class TerrainBuildings
{
	boost::scoped_ptr<TerrainBuildingsData> data;

public:
	TerrainBuildings(Storm &storm, Mouse &mouse, IEditorState &state);
	~TerrainBuildings();

	void addModel(const std::string &fileName);
	void removeModel(int modelIndex);
	void setCutTerrain(int modelIndex, bool cut);
	void updateLighting();

	void clear();
	void setToTerrain();
	void addBuilding(int modelIndex, const VC2 &position, float rotation);

	Building traceActiveCollision(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength);
	void rotateObject(Building &building, int yDelta);
	void moveObject(Building &building, const VC2 &delta);
	void removeObject(Building &building);

	void hideRoofs(bool hide);
	void roofCollision(bool collision);
	void showDoors(bool show);

	int getModelCount() const;
	std::string getModel(int index) const;
	bool hasCutTerrain(int modelIndex) const;

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};


inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainBuildings &objects)
{ 
	return objects.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainBuildings &objects)
{ 
	return objects.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
