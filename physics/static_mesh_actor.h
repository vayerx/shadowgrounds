#ifndef INCLUDED_FROZENBYTE_STATIC_MESH_ACTOR_H
#define INCLUDED_FROZENBYTE_STATIC_MESH_ACTOR_H

#include "actor_base.h"
#include <boost/shared_ptr.hpp>

class NxPhysicsSDK;
class NxScene;
class NxTriangleMesh;
class NxTriangleMeshShape;

namespace frozenbyte {
namespace physics {

class StaticMeshActor;

class StaticMesh
{
	NxPhysicsSDK &sdk;
	NxTriangleMesh *mesh;

public:
	StaticMesh(NxPhysicsSDK &sdk, const char *filename);
	~StaticMesh();

	inline NxTriangleMesh *getMesh() const { return mesh; }
	friend class StaticMeshActor;
};

class StaticMeshActor: public ActorBase
{
	//NxTriangleMeshShape *shape;
	boost::shared_ptr<StaticMesh> mesh;

public:
	StaticMeshActor(NxScene &scene, const boost::shared_ptr<StaticMesh> &mesh, const VC3 &position, const QUAT &rotation);
	~StaticMeshActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
