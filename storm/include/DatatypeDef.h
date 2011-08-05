/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  This file includes current common library datatypes, and typedefs
  them in the way Storm3Dv2 uses them.

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <c2_UI.h>


//------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------
typedef Vec3<float> VC3;
typedef Vec2<float> VC2;
typedef Vec3<int> VC3I;
typedef Vec2<int> VC2I;
typedef TMatrix<float> MAT;
typedef Quat<float> QUAT;
typedef TColor<float> COL;
typedef TPlane<float> PLANE;

// These are better ;-)
typedef Vec3<float> Vector;
typedef Vec2<float> Vector2D;
typedef TMatrix<float> Matrix;
typedef Quat<float> Rotation;
typedef TColor<float> Color;
typedef TPlane<float> Plane;
