#ifndef INCLUDED_UTIL_ITRACKABLE_H
#define INCLUDED_UTIL_ITRACKABLE_H

#include <DatatypeDef.h>

namespace util {

class ITrackable
{
public:
	virtual ~ITrackable() {}

	virtual VC3 getTrackablePosition() const = 0;
	virtual float getTrackableRadius2d() const = 0;
};

} // util

#endif
