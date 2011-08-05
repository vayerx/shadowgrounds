#ifndef INCLUDED_UTIL_ITRIGGERLISTENER_H
#define INCLUDED_UTIL_ITRIGGERLISTENER_H

#include <DatatypeDef.h>

namespace util {

class ITriggerListener
{
public:
	virtual ~ITriggerListener() {}

	virtual void activate(int triggerId, void *data) = 0;
};

} // util

#endif
