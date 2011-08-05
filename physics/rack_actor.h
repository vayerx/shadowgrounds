#ifndef INCLUDED_FROZENBYTE_RACK_ACTOR_H
#define INCLUDED_FROZENBYTE_RACK_ACTOR_H

#include "actor_base.h"

namespace frozenbyte {
namespace physics {

class RackActor: public ActorBase
{

public:
	RackActor(NxScene &scene, const VC3 &position);
	~RackActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
