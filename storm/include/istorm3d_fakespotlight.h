// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_FAKESPOTLIGHT_H
#define INCLUDED_ISTORM3D_FAKESPOTLIGHT_H

#include <DatatypeDef.h>
#include <boost/shared_ptr.hpp>

class IStorm3D_FakeSpotlight
{
public:
	virtual ~IStorm3D_FakeSpotlight() {}

	virtual void enable(bool enable) = 0;
	virtual void setPosition(const VC3 &position) = 0;
	virtual void setDirection(const VC3 &direction) = 0;
	virtual void setFov(float fov) = 0;
	virtual void setRange(float range) = 0;
	virtual void setColor(const COL &color, float fadeFactor) = 0;
	virtual void renderObjectShadows(bool render) = 0;

	virtual void setPlane(float height, const VC2 &minCorner, const VC2 &maxCorner) = 0;
};

#endif
