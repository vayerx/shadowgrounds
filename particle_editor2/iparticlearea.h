#ifndef INCLUDED_IPARTICLE_AREA_H
#define INCLUDED_IPARTICLE_AREA_H

#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
namespace particle {

class IParticleArea
{
public:
	virtual ~IParticleArea() {}

	virtual void biasValues(const VC3 &position, VC3 &velocity) const = 0;
	virtual float getObstacleHeight(const VC3 &position) const = 0;
	virtual float getBaseHeight(const VC3 &position) const = 0;
	virtual bool isInside(const VC3 &position) const = 0;
};

} // particle
} // frozenbyte

#endif
