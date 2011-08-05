#ifndef INCLUDED_ISTORM3D_VIDEOSTREAMER_H
#define INCLUDED_ISTORM3D_VIDEOSTREAMER_H

class IStorm3D_Scene;
class IStorm3D_Material;
#include <DatatypeDef.h>

class IStorm3D_VideoStreamer
{
public:
	virtual ~IStorm3D_VideoStreamer() {};

	virtual IStorm3D_Material *getMaterial() = 0;

	virtual int getTime() const = 0;
	virtual bool isPlaying() const = 0;

	virtual void setPosition(const VC2 &position, const VC2 &size) = 0;
	virtual void setAlpha(float alpha) = 0;
	virtual void update() = 0;
	virtual void render(IStorm3D_Scene *scene) = 0;

	virtual void getTextureCoords(float &x, float &y) = 0;

};

#endif
