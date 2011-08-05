#ifndef INCLUDED_FROZENBYTE_PHYSICS_VISUALIZER_H
#define INCLUDED_FROZENBYTE_PHYSICS_VISUALIZER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D_Scene;

namespace frozenbyte {
namespace physics {

class PhysicsLib;
void visualize(PhysicsLib &physics, IStorm3D_Scene &scene, float range);

} // physics
} // frozenbyte

#endif
