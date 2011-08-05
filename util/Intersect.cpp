
#include "precompiled.h"

#include "Intersect.h"

#include <Storm3D_datatypes.h>

namespace util {


bool isPointInsideTriangle(const VC2& point, const VC2& a, const VC2& b, const VC2& c)
{
	// Depends on the cyclic order of triangle points, not sure if works correctly. (returns inverse result?)
	// I might of used wrong winding order or something.
	/*
	float ab=(point.y - a.y) * (b.x - a.y) - (point.x - a.x) * (b.x - a.x);
	float ca=(point.y - c.y) * (a.x - c.y) - (point.x - c.x) * (a.x - c.x);
	float bc=(point.y - b.y) * (c.x - b.y) - (point.x - b.x) * (c.x - b.x);

	bool inside=((ab*bc) > 0 ) && ((bc*ca) > 0 );

	return inside;*/

	// Does not depend on the cyclic order. Seems to work.
	float b0 =  (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
	float b1 = ((b.x - point.x) * (c.y - point.y) - (c.x - point.x) * (b.y - point.y)) / b0;
	float b2 = ((c.x - point.x) * (a.y - point.y) - (a.x - point.x) * (c.y - point.y)) / b0;
	float b3 = ((a.x - point.x) * (b.y - point.y) - (b.x - point.x) * (a.y - point.y)) / b0;

	bool strictlyInside=(b1>0) && (b2>0) && (b3>0);
	bool onEdgeAB=(b1>0 ) && (b2>0 ) && (b3==0);
	bool onEdgeBC=(b1==0) && (b2>0 ) && (b3>0 );
	bool onEdgeCA=(b1>0 ) && (b2==0) && (b3>0 );

	return strictlyInside || onEdgeAB || onEdgeBC || onEdgeCA;

}


} /* namespace util */


/* EOF */
