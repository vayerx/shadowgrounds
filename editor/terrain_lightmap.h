// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_LIGHTMAP_H
#define INCLUDED_EDITOR_TERRAIN_LIGHTMAP_H

#include "terrain_lightmap.h"
#include <boost/scoped_ptr.hpp>
#include <datatypedef.h>
#include <vector>

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
} // filesystem

namespace editor {

struct Storm;
class Exporter;

class TerrainLightMap
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:

	struct PointLight
	{
		VC3 position;
		float range;
		float strength;
		COL color;

		PointLight()
		:	range(0.f),
			strength(0.f)
		{
		}
	};

	TerrainLightMap(Storm &storm);
	~TerrainLightMap();

	void reset();
	void create(const std::vector<PointLight> &lights, int area, int quality, const VC3 &sunDir);
	void debugRender();
	void apply();

	COL getColor(const VC2 &position) const;
	void setShadow(const VC2 &position, float value, const VC2 &rect, const std::vector<PointLight> &lights, const VC3 &sunDir);

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainLightMap &map)
{ 
	return map.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainLightMap &map)
{ 
	return map.readStream(stream);
}

} // editor
} // frozenbyte

#endif
