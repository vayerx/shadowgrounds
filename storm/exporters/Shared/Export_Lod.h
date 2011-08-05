// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LOD_H
#define INCLUDED_LOD_H

#ifdef _MSC_VER
#pragma warning(disable: 4514) // removed unreferenced inline function (stl)
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_EXPORT_FACE_H
#include "Export_Face.h"
#endif
#ifndef INCLUDED_EXPORT_VERTEX_H
#include "Export_Vertex.h"
#endif

namespace frozenbyte {
namespace exporter {

struct LodData;

class Lod
{
	boost::scoped_ptr<LodData> data;

public:
	Lod(const std::vector<Vertex> &vertices, const std::vector<Face> &faces, bool optimizeVcache);
	~Lod();

	void generateLods(int lodDetail);

	int getFaceBufferCount() const; // 1 if no lods
	const std::vector<Face> &getFaceBuffer(int index) const;
	const std::vector<Vertex> &getVertices() const;
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
