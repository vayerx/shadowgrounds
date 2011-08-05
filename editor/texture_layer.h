// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TEXTURE_LAYER_H
#define INCLUDED_EDITOR_TEXTURE_LAYER_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace editor {

class TerrainTextures;
struct Storm;
struct TextureLayerData;

class TextureLayer
{
	boost::scoped_ptr<TextureLayerData> data;

public:
	TextureLayer(TerrainTextures &textures, Storm &storm);
	~TextureLayer();

	bool show();
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
