// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_CAMERA_H
#define INCLUDED_EDITOR_CAMERA_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

#include <datatypedef.h>

namespace frozenbyte {
namespace editor {

class Mouse;
struct Storm;
struct CameraData;

class Camera
{
	boost::scoped_ptr<CameraData> data;

public:
	Camera(Storm &storm);
	~Camera() ;

	void forceGameCamera(bool force);
	const VC3 &getPosition() const;

	void nudgeCamera(const VC3 &direction);

	void update(const Mouse &mouse, bool hasFocus);
	void setToOrigo();
	void setToSky();

	float getHorizontal();
	float getVertical();
};

float getTimeDelta();
void setBoundary(const VC2 &min, const VC2 &max);

} // end of namespace editor
} // end of namespace frozenbyte

#endif

