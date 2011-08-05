// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef IPARTICLE_COLLISION_H
#define IPARTICLE_COLLISION_H

#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
namespace particle {

class IParticleCollision
{
public:
	virtual ~IParticleCollision() {}

	virtual bool spawnPosition(const VC3 &emitter, const VC3 &dir, VC3 &position) const = 0;
	virtual bool getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const = 0;
};

} // particle
} // frozenbyte

#endif
