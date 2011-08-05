#ifndef INCLUDED_FROZENBYTE_CAR_ACTOR_H
#define INCLUDED_FROZENBYTE_CAR_ACTOR_H

#include "actor_base.h"

namespace frozenbyte {
namespace physics {

class CarActor: public ActorBase
{

public:
	CarActor(NxScene &scene, const VC3 &position);
	~CarActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
