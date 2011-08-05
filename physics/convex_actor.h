#ifndef INCLUDED_FROZENBYTE_CONVEX_ACTOR_H
#define INCLUDED_FROZENBYTE_CONVEX_ACTOR_H

#include "actor_base.h"
#include <boost/shared_ptr.hpp>

class NxPhysicsSDK;
class NxScene;
class NxConvexMesh;

namespace frozenbyte {
namespace physics {

class ConvexActor;

class ConvexMesh
{
	NxPhysicsSDK &sdk;
	NxConvexMesh *mesh;

public:
	ConvexMesh(NxPhysicsSDK &sdk, const char *filename);
	~ConvexMesh();

	bool isValidForHardware() const;
	bool isValid() const;

	friend class ConvexActor;
};

class ConvexActor: public ActorBase
{
	boost::shared_ptr<ConvexMesh> mesh;

public:
	ConvexActor(NxScene &scene, const boost::shared_ptr<ConvexMesh> &mesh, const VC3 &position);
	~ConvexActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
