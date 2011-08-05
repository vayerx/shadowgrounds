// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORTER_OBJECTS_H
#define INCLUDED_EDITOR_EXPORTER_OBJECTS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_MAP
#define INCLUDED_MAP
#include <map>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

#include <vector>

namespace frozenbyte {
namespace editor {

struct ExportOptions;
struct ExporterObjectsData;
struct Animation;

class ExporterObjects
{
	boost::scoped_ptr<ExporterObjectsData> data;

public:
	enum CollisionType
	{
		CollisionNone = 0,
		CollisionBox = 1,
		CollisionCylinder = 2,
		CollisionMapped = 3
	};

	enum FallType
	{
		FallStatic = 0,
		FallTree = 1,
		FallPlant = 2
	};

	ExporterObjects();
	~ExporterObjects();

	int addTerrainObject(const std::string &fileName, CollisionType type, FallType fallType, float height, float radius, bool fireThrough, const std::string &explosionObject, const std::string &explosionScript, const std::string &explosionProjectile, const std::string &explosionEffect, const std::vector<std::string> &sounds, const std::string &material, int hp, const Animation &animation, int breakTexture, int physicsType, float physicsMass, const std::string &physicsSoundMaterial, const VC3 &physicsData1, const VC3 &physicsData2, const std::string &durabilityType, const std::map<std::string, std::string> &metaValues);
	void addObject(int id, const VC3 &position, const VC3 &rotation, const COL &color, float height1, signed short int *lightIndices, bool lightmapped, bool inBuilding, const VC3 &sunDir, float sunStrength);

	int addTerrainBuilding(const std::string &fileName, bool cutTerrain);
	void addBuilding(int id, const VC2 &position, float yRotation);

	void save(const ExportOptions &options) const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
