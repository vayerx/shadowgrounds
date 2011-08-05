// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_DECORATORS_H
#define INCLUDED_EDITOR_TERRAIN_DECORATORS_H

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

class Exporter;
struct Storm;
struct TerrainDecoratorsData;

class TerrainDecorators
{
	boost::scoped_ptr<TerrainDecoratorsData> data;

public:
	TerrainDecorators(Storm &storm);
	~TerrainDecorators();

	void reset();
	void setWaterModel(int index, const std::string &fileName);
	void setWaterHeight(int index, float height);

	const std::string &getWaterName(int index) const;
	float getWaterHeight(int index) const;

	void tick();

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainDecorators &objects)
{ 
	return objects.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainDecorators &objects)
{ 
	return objects.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
