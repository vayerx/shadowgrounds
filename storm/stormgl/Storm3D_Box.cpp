/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Box

  Bounding box (min/max)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_Box::Storm3D_Box
//------------------------------------------------------------------
Storm3D_Box::Storm3D_Box(const VC3 &_pmin,const VC3 &_pmax) :
	pmin(_pmin),
	pmax(_pmax)
{
}



//------------------------------------------------------------------
// Storm3D_Box::GetSize
//------------------------------------------------------------------
VC3 Storm3D_Box::GetSize() const
{
	return pmax-pmin;
}



//------------------------------------------------------------------
// Storm3D_Box::IsPointInside
//------------------------------------------------------------------
bool Storm3D_Box::IsPointInside(const VC3 &point) const
{
	// Test if inside
	if ((point.x>=pmin.x)&&
		(point.x<=pmax.x)&&
		(point.z>=pmin.z)&&
		(point.z<=pmax.z)&&
		(point.y>=pmin.y)&&
		(point.y<=pmax.y)) return true;

	// Not inside
	return false;
}


