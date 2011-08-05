// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LIGHTOBJECT_H
#define INCLUDED_LIGHTOBJECT_H

#ifdef _MSC_VER
#pragma warning(disable: 4514) // removed unreferenced inline function (stl)
#endif

#include <vector>
#include <boost/shared_ptr.hpp>

namespace frozenbyte {
namespace exporter {

class Object;
class Material;

struct LightObjects
{
	std::vector<boost::shared_ptr<Object> >objects;
	std::vector<boost::shared_ptr<Object> > roofObjects;
	std::vector<Material> materials;

	void combine();
};

void insertLightObjects(LightObjects &result, const Object &source, const std::vector<Material> &materials);
void adjustMaterialIndices(LightObjects &result, int baseIndex);

} // export
} //frozenbyte

#endif
