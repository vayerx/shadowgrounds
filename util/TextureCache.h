#ifndef INCLUDED_TEXTURECACHE_H
#define INCLUDED_TEXTURECACHE_H

#include <boost/scoped_ptr.hpp>
class IStorm3D;
class IStorm3D_Texture;

namespace frozenbyte {

class TextureCache
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	TextureCache(IStorm3D &storm);
	~TextureCache();

	// loads texture data to cpu side memory
	void loadTextureDataToMemory(const char *fileName);

	void loadTexture(const char *fileName, bool temporaryCache);
	void clearTemporary();
	void update(int ms);

	void setLoadFlags(int flags);
	int getLoadFlags();

	IStorm3D_Texture *getTexture(const char *fileName, bool temporaryCache = true);
};

} // frozenbyte

#endif
