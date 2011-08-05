// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_OBJECTS
#define INCLUDED_EDITOR_TERRAIN_OBJECTS

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

#include "group_list.h"

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

class Exporter;
class ObjectSettings;
class IEditorState;
struct Storm;
struct TerrainObjectsData;
class TerrainObjects;
class EditorObjectState;


class TerrainObject
{
	std::string fileName;
	int index;
	VC3 rotation;
	COL color;
	float height;

public:
	TerrainObject()
		:	index(-1), height(0) {}

	bool hasObject()
	{
		if(index >= 0 && !fileName.empty())
			return true;

		return false;
	}

	const std::string &getFileName() const
	{
		return fileName;
	}

	const VC3 &getRotation() const
	{
		return rotation;
	}

	const COL &getColor() const
	{
		return color;
	}

	float getHeight() const
	{
		return height;
	}

	bool operator == (const TerrainObject &other)
	{
		if(index == other.index && fileName == other.fileName)
			return true;

		return false;
	}

	friend class TerrainObjects;
	friend struct TerrainObjectsData;
};

class TerrainObjects
{
	boost::scoped_ptr<TerrainObjectsData> data;

	// TODO: move to data.
	void hideObjectsImpl(const char **filters, int filterAmount, bool invert);
	void showObjectsImpl(const char **filters, int filterAmount, bool invert);

public:
	TerrainObjects(Storm &storm, IEditorState &state);
	~TerrainObjects();

	void clear();
	void resetTerrain();
	void setToTerrain();
	TerrainObject addObject(const std::string &fileName, const VC2 &position, const VC3 &rotation, float height);

	void drawCollision(bool drawState, const std::string &fileName);
	ObjectSettings &getObjectSettings();

	TerrainObject traceActiveCollision(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength);
	void moveObject(TerrainObject &terrainObject, const VC2 &delta);
	void moveObject(TerrainObject &terrainObject, float height);
	void rotateObject(TerrainObject &terrainObject, const VC3 &rotation);
	void setColor(TerrainObject &terrainObject, int color);
	void setLightMultiplier(TerrainObject &terrainObject, float delta);
	void removeObject(TerrainObject &terrainObject);
	void nudgeObjects(const VC3 &position, const VC3 &direction, float radius, const char **filter, int filterAmount, bool invert);
	void removeObjects(const VC3 &position, float radius, const char **filter, int filterAmount, bool invert);
	void copyObjects(const VC3 &position, float radius, GroupList::ObjectGroup &group, const char **filter, int filterAmount, bool invert);

	void hideObjects();
	void showObjects();
	void updateColors();
	void updateLightmapStates();
	void updateLighting();
	void getEditorObjectStates(EditorObjectState &states);
	void setEditorObjectStates(const EditorObjectState &states);

	void hideHelperObjects();
	void showHelperObjects();
	void hideIncompleteObjects();
	void showIncompleteObjects();
	void hideStaticObjects();
	void showStaticObjects();
	void hideDynamicObjects();
	void showDynamicObjects();
	void hideNoCollisionObjects();
	void showNoCollisionObjects();

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainObjects &objects)
{ 
	return objects.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainObjects &objects)
{ 
	return objects.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
