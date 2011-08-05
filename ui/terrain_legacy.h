#ifndef INCLUDED_UI_TERRAIN_LEGACY_H
#define INCLUDED_UI_TERRAIN_LEGACY_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

class IStorm3D;
class IStorm3D_Terrain;

namespace Parser {
	class ParserGroup;
}
namespace frozenbyte {
namespace ui {

struct TerrainLegacyData;

class TerrainLegacy
{
	boost::scoped_ptr<TerrainLegacyData> data;

public:
	TerrainLegacy();
	~TerrainLegacy();

	void setTexturing(int baseIndex, int *indexArray);
	void setStorm(IStorm3D *storm, IStorm3D_Terrain *terrain);
	void setProperties(const VC2I &mapSize, float terrainScale);

	void apply(const Parser::ParserGroup &group);
	void clear();
};

} // ui
} // frozenbyte

#endif
