// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_TEXTURES_H
#define INCLUDED_EDITOR_TERRAIN_TEXTURES_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOr
#include <vector>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
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
struct TerrainTexturesData;

struct TextureSplat
{
	VC2I position;
	VC2I size;

	std::vector<unsigned char> weights;

	TextureSplat(const VC2I &position_, const VC2I &size_)
	:	position(position_),
		size(size_),
		weights(size.x * size.y)
	{
	}
};

class TerrainTextures
{
	boost::scoped_ptr<TerrainTexturesData> data;

public:
	explicit TerrainTextures(Storm &storm);
	~TerrainTextures();

	void addTexture(const std::string &fileName);
	void removeTexture(int index);

	void clear();	
	void applyToTerrain();

	int getTextureCount() const;
	std::string getTexture(int index) const;

	void optimize();
	void resetBlendMap(const VC2I &size);
	void addSplat(int textureIndex, const TextureSplat &splat);
	const VC2I &getMapSize() const;

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TerrainTextures &textures)
{ 
	return textures.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, TerrainTextures &textures)
{ 
	return textures.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
