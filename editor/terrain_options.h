// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_OPTIONS_H
#define INCLUDED_EDITOR_TERRAIN_OPTIONS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

class IStorm3D_Texture;

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

class Exporter;
struct Storm;
struct TerrainOptionsData;

class TerrainOptions
{
	boost::scoped_ptr<TerrainOptionsData> data;

public:
	explicit TerrainOptions(Storm &storm);
	~TerrainOptions();

	void applyToTerrain();
	void clear();

	void setDetailTexture(int index, const std::string &textureName);
	bool setDetailRepeat(int index, int value);
	bool setHeight(int index, int value); // Returns true if changed
	bool setSlopeDivider(int value);
	bool setSlopeStart(int value);

	std::string getDetailTexture(int index) const;
	int getDetailRepeat(int index) const;
	int getHeight(int index) const;
	int getSlopeDivider() const;
	int getSlopeStart() const;

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainOptions &options)
{ 
	return options.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainOptions &options)
{ 
	return options.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
