#ifndef INCLUDED_FROZENBYTE_COOKER_H
#define INCLUDED_FROZENBYTE_COOKER_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>

class NxPhysicsSDK;
class NxScene;

class IStorm3D_Model;
class IStorm3D_Model_Object;

namespace frozenbyte {
namespace physics {

class PhysicsLib;

class Cooker
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	Cooker();
	~Cooker();

	bool cookMesh(const char *filename, IStorm3D_Model *model);
	bool cookHeightmap(const unsigned short *heightmap, const unsigned char *clipmap, const VC2I &resolution, const VC3 &size, const char *filename);
	bool cookCylinder(const char *filename, float height, float radius, float offset = 0.f, int upvectorAxis = 1);
	bool cookApproxConvex(const char *filename, IStorm3D_Model_Object *object);
};

} // physics
} // frozenbyte

#endif
