// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_SPOTLIGHT_H
#define INCLUDED_ISTORM3D_SPOTLIGHT_H

#include <DatatypeDef.h>
#include <boost/shared_ptr.hpp>

class IStorm3D_Texture;
class IStorm3D_Model;

class IStorm3D_Spotlight
{
public:
	virtual ~IStorm3D_Spotlight() {}

	virtual void enable(bool enable) = 0;
	virtual void setPosition(const VC3 &position) = 0;
	virtual void setDirection(const VC3 &direction) = 0;
	virtual void setFov(float fov) = 0;
	virtual void setConeFov(float fov) = 0;
	virtual void setRange(float range) = 0;
	virtual void setClipRange(float range) = 0;
	virtual void setEnableClip( bool enableClip ) = 0;

	enum Feature
	{
		Fade,
		ConeVisualization,
		Shadows,
		ScissorRect
	};

	virtual void enableFeature(Feature feature, bool enable) = 0;

	enum Type
	{
        None = -1,
		Flat = 0,
		Point = 1,
		Directional = 2
	};

	virtual void setType(Type type) = 0;
	virtual void setNoShadowModel(const IStorm3D_Model *model) = 0;
	virtual void setProjectionTexture(boost::shared_ptr<IStorm3D_Texture> texture) = 0;
	virtual void setConeTexture(boost::shared_ptr<IStorm3D_Texture> texture) = 0;
	virtual void setColorMultiplier(const COL &color) = 0;
	virtual void setConeMultiplier(float scalar) = 0;
	virtual void setSmoothness(float smoothness) = 0;

	virtual QUAT getOrientation() const = 0;
};

#endif
