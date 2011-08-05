#ifndef INCLUDED_FROZENBYTE_CAPSULE_ACTOR_H
#define INCLUDED_FROZENBYTE_CAPSULE_ACTOR_H

#include "actor_base.h"

namespace frozenbyte {
namespace physics {

class CapsuleActor: public ActorBase
{

public:
	CapsuleActor(NxScene &scene, float height, float radius, const VC3 &position, float offset = 0.0f, int axisNumber = 1);
	~CapsuleActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
