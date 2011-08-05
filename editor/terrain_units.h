// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_UNITS_H
#define INCLUDED_EDITOR_TERRAIN_UNITS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif
#ifndef INCLUDED_MAP
#define INCLUDED_MAP
#include <map>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

class IStorm3D_Model;

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

class UnitScripts;
class Exporter;
class IEditorState;
struct ScriptConfig;
struct Unit;
struct Storm;
struct TerrainUnitsData;
struct UnitProperties;
struct StringProperties;

class UnitHandle
{
	std::string unitName;
	int unitIndex;

	VC2 position;
	float height;

	friend class TerrainUnits;
	friend struct TerrainUnitsData;

public:	
	UnitHandle();
	~UnitHandle();

	const VC2 &getPosition() const;
	float getHeight() const;

	bool hasUnit() const;
	bool operator == (const UnitHandle &rhs);
};

class UnitHelpers
{
public:
	struct HelperInfo
	{
		std::string name;
		std::string exportName;

		bool isPath;

		HelperInfo()
		:	isPath(false)
		{
		}

		friend bool operator < (const HelperInfo &a, const HelperInfo &b)
		{
			return a.name < b.name;
		}
	};

private:
	std::map<std::string, std::vector<VC2> > helpers;
	std::vector<HelperInfo> usedHelpers;

	friend class TerrainUnits;
	friend struct TerrainUnitsData;

public:
	UnitHelpers();
	~UnitHelpers();

	int getHelperAmount() const;
	const std::string &getHelperName(int helperIndex) const;
	const std::string &getHelperExportName(int helperIndex) const;
	bool isWayPoint(int helperIndex) const;

	int getPointAmount(int helperIndex) const;
	VC2 getPoint(int helperIndex, int index) const;

	void addPoint(int helperIndex, const VC2 &point);
	void setPoint(int helperIndex, int index, const VC2 &point);
	void deletePoint(int helperIndex, int index);

	void writeStream(filesystem::OutputStream &stream) const;
	void readStream(filesystem::InputStream &stream);
};

class TerrainUnits
{
	boost::scoped_ptr<TerrainUnitsData> data;

	// TODO: move to data.
	void hideUnitsImpl(const char **filters, int filterAmount, bool invert);
	void showUnitsImpl(const char **filters, int filterAmount, bool invert);

public:
	TerrainUnits(Storm &storm, const UnitScripts &unitScript, IEditorState &state);
	~TerrainUnits();

	void clear();
	void setToTerrain();

	boost::shared_ptr<IStorm3D_Model> getUnitModel(int unitIndex);
	Unit getUnitSettings(int unitIndex);

	UnitHandle addUnit(int unitIndex, VC3 &position, float yRotation, float height);
	void rotateUnit(const UnitHandle &unitHandle, int wheelDelta);
	void removeUnit(const UnitHandle &unitHandle);
	void setHeight(UnitHandle &unitHandle, float height);
	void setPosition(UnitHandle &unitHandle, const VC3 &position);
	UnitHandle changeUnitType(const UnitHandle &unitHandle, int newIndex);

	UnitHandle traceCursor(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength);
	void setActiveUnit(const UnitHandle &unitHandle);
	void setUnitScripts(const UnitHandle &unitHandle, const ScriptConfig &scriptConfig);
	void setUnitSide(const UnitHandle &unitHandle, int side);

	std::string getUnitName(const UnitHandle &unitHandle) const;
	Unit getUnit(const UnitHandle &unitHandle) const;
	ScriptConfig getUnitScripts(const UnitHandle &unitHandle) const;
	int getUnitSide(const UnitHandle &unitHandle) const;
	UnitProperties &getUnitProperties(const UnitHandle &unitHandle);
	std::vector<std::string> getUnitUsedProperties(const UnitHandle &unitHandle);
	StringProperties getUnitStringProperties(const UnitHandle &unitHandle);
	UnitHelpers &getUnitHelpers(const UnitHandle &unitHandle);

	void hideUnits();
	void showUnits();
	void updateColors();
	void updateLighting();

	void hideNormalUnits();
	void showNormalUnits();
	void hideGridUnits();
	void showGridUnits();
	void hideTriggerUnits();
	void showTriggerUnits();

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainUnits &units)
{ 
	return units.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainUnits &units)
{ 
	return units.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
