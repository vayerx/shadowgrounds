#ifndef INCLUDED_UTIL_BUILDING_BLINKER_H
#define INCLUDED_UTIL_BUILDING_BLINKER_H

#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
	class TextureCache;
}

namespace util {

class BuildingBlinker 
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	BuildingBlinker(frozenbyte::TextureCache &cache);
	~BuildingBlinker();

	void addBlinkTexture(const char *original, const char *to, int delta);
	void update(int timeDelta);
}; 

} // util

#endif
