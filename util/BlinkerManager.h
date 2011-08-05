#ifndef INCLUDED_UTIL_BLINKER_MANAGER_H
#define INCLUDED_UTIL_BLINKER_MANAGER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace util {

class BuildingBlinker;

class BlinkerManager 
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	BlinkerManager();
	~BlinkerManager();

	void addBlinker(boost::shared_ptr<BuildingBlinker> blinker);
	void update(int timeDelta);
}; 

} // util

#endif
