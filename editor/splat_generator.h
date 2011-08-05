// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_SPLAT_GENERATOR_H
#define INCLUDED_SPLAT_GENERATOR_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

namespace frozenbyte {
namespace editor {

class TerrainTextures;
struct Storm;
struct SplatGeneratorData;

class SplatGenerator
{
	boost::scoped_ptr<SplatGeneratorData> data;

public:
	SplatGenerator(Storm &storm, TerrainTextures &textures);
	~SplatGenerator();

	void generate(int textureId, const VC3 &position, int size, float strength);
};

} // editor
} // frozenbyte

#endif
