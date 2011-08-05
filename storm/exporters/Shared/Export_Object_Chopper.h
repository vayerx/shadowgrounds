// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_OBJECT_CHOPPER_H
#define INCLUDED_OBJECT_CHOPPER_H

//#include "Export_Object.h"
#include <vector>
#include <boost/shared_ptr.hpp>

namespace frozenbyte {
namespace exporter {

class Object;
class Material;

double calculateSize(Object &object);
void chopFaces(Object &object);
void chopObjects(std::vector<boost::shared_ptr<Object> > &objects);
bool needChop(std::vector<boost::shared_ptr<Object> > &objects);

void chopObjectToLimits(std::vector<boost::shared_ptr<Object> > &objects);
void removeJunctions(std::vector<boost::shared_ptr<Object> > &objects);
void snapVertices(std::vector<boost::shared_ptr<Object> > &objects);

} // end of namespace export
} // end of namespace frozenbyte

#endif
