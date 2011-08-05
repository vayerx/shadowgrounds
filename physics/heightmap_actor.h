#ifndef INCLUDED_FROZENBYTE_HEIGHTMAP_ACTOR_H
#define INCLUDED_FROZENBYTE_HEIGHTMAP_ACTOR_H

#include "actor_base.h"

class NxHeightField;
class NxPhysicsSDK;
class NxScene;

namespace frozenbyte {
namespace physics {

class HeightmapActor: public ActorBase
{
	NxPhysicsSDK &sdk;
	NxHeightField *heightField;

public:
	HeightmapActor(NxPhysicsSDK &sdk, NxScene &scene, const unsigned short *buffer, int samplesX, int samplesY, const VC3 &scale);
	~HeightmapActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
