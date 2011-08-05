// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_ALIGN_UNITS_H
#define INCLUDED_EDITOR_ALIGN_UNITS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DatatypeDef.h>
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

} // end of namespace editor
} // end of namespace frozenbyte

#endif
