// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_STORM_TEXTURE_H
#define INCLUDED_EDITOR_TERRAIN_STORM_TEXTURE_H

#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

class Storm;
class IStorm3D_Texture;

namespace frozenbyte {
namespace editor {

boost::shared_ptr<IStorm3D_Texture> loadTexture(const std::string &fileName, Storm &storm);
boost::shared_ptr<IStorm3D_Texture> createTexture(int width, int height, Storm &storm);

} // end of namespace editor
} // end of namespace frozenbyte

#endif
