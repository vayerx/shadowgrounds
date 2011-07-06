// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_ALIGN_UNITS_H
#define INCLUDED_EDITOR_ALIGN_UNITS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

class IStorm3D_Camera;

namespace frozenbyte {
namespace editor {

struct AlignUnitsData;

class AlignUnits
{
	boost::scoped_ptr<AlignUnitsData> data;

public:
	AlignUnits();
	~AlignUnits();

	void getGridSize(float &x, float &y) const;
	void setGridSize(float x, float y);

	VC3 getAlignedPosition(const VC3 &position) const;
	VC2 getAlignedPosition(const VC2 &position) const;

	float getRotation(float oldRotation, int delta, bool cap = true) const;
	float getHeight(const float &height, int delta) const;

	VC3 getMovedPosition(const VC3 &position, IStorm3D_Camera &camera, bool *updated = 0) const;
};

inline QUAT getRotation(const VC3 &angles)
{
	QUAT qx;
	qx.MakeFromAngles(angles.x, 0, 0);
	QUAT qy;
	qy.MakeFromAngles(0, angles.y, 0);
	QUAT qz;
	qz.MakeFromAngles(0, angles.z, 0);

	return qz * qx * qy;
}

inline VC3 getEulerAngles(QUAT quat)
{
	quat.Inverse();
	MAT tm;
	tm.CreateRotationMatrix(quat);

	float heading = 0.f;
	float attitude = 0.f;
	float bank = 0.f;

	float m00 = tm.Get(0);
	float m02 = tm.Get(2);
	float m10 = tm.Get(4);
	float m11 = tm.Get(5);
	float m12 = tm.Get(6);
	float m20 = tm.Get(8);
	float m22 = tm.Get(10);

	if(m10 > 0.998f)
	{
		heading = atan2f(m02, m22);
		attitude = PI/2.f;
		bank = 0.f;
	}
	else if(m10 < -0.998f)
	{
		heading = atan2f(m02, m22);
		attitude = -PI/2;
		bank = 0.f;
	}
	else
	{
		heading = atan2f(-m20, m00);
		attitude = asinf(m10);
		bank = atan2f(-m12, m11);
	}

	return VC3(bank, heading, attitude);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
