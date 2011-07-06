#ifndef INCLUDED_FROZENBYTE_BOX_ACTOR_H
#define INCLUDED_FROZENBYTE_BOX_ACTOR_H

#include "actor_base.h"
#include <string>

class NxBoxShape;

namespace frozenbyte {
namespace physics {

class BoxActor: public ActorBase
{

public:
	BoxActor(NxScene &scene, const VC3 &sizes, const VC3 &position, const VC3 &localPosition, bool ccd, float ccdMaxThickness);
	BoxActor(NxScene &scene, const std::string &shapes, const VC3 &position, bool ccd, float ccdMaxThickness);
	~BoxActor();

	// Extended stuff
	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
