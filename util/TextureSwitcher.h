#ifndef INCLUDED_UTIL_TEXTURE_SWITCHER_H
#define INCLUDED_UTIL_TEXTURE_SWITCHER_H

#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
	class TextureCache;
}

namespace util {

class TextureSwitcher 
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	explicit TextureSwitcher(frozenbyte::TextureCache &cache);
	~TextureSwitcher();

	void switchTexture(const char *from, const char *to);
}; 

} // util

#endif
