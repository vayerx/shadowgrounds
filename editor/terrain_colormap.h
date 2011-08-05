// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_COLORMAP_H
#define INCLUDED_EDITOR_TERRAIN_COLORMAP_H

#include <boost/scoped_ptr.hpp>
#include <datatypedef.h>

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
} // filesystem

namespace editor {

struct Storm;
class Exporter;

class TerrainColorMap
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	TerrainColorMap(Storm &storm);
	~TerrainColorMap();

	void reset();
	void create();
	void debugRender();

	COL getColor(const VC2 &position) const;

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainColorMap &map)
{ 
	return map.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainColorMap &map)
{ 
	return map.readStream(stream);
}

} // editor
} // frozenbyte

#endif
