// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Face.h"
#include <cassert>

namespace frozenbyte {
namespace exporter {

Face::Face()
:	materialId(-1)
{
	indices[0] = 0;
	indices[1] = 0;
	indices[2] = 0;
}

Face::~Face()
{
}

int Face::getVertexIndex(int index) const
{
	assert((index >= 0) && (index < 3));
	return indices[index];
}

int Face::getMaterialId() const
{
	return materialId;
}

const FBVector &Face::getNormal() const
{
	return normal;
}

void Face::setVertexIndex(int index, int vertex_index)
{
	assert((index >= 0) && (index < 3));
	indices[index] = vertex_index;
}

void Face::setMaterialId(int id)
{
	materialId = id;
}

void Face::setNormal(const FBVector &normal_)
{
	normal = normal_;
}

void Face::correctMaterialIndices(const std::vector<int> &newIndices)
{
	materialId = newIndices[materialId];
}

} // end of namespace export
} // end of namespace frozenbyte
