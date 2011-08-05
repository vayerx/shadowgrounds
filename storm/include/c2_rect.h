// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once



//------------------------------------------------------------------
// Protos
//------------------------------------------------------------------
template <class A> class Vec3;
template <class A> class Vec2;



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "c2_vectors.h"



//------------------------------------------------------------------
// Rect
//------------------------------------------------------------------
template <class A> class Rect
{

public:

	// Data (public for speed)
	Vec2<A> upper_left;
	Vec2<A> lower_right;

	// Constructors
	inline Rect(const Vec2<A> &_upper_left=Vec2<A>(0,0),const Vec2<A> &_lower_right=Vec2<A>(0,0)) : upper_left(_upper_left),lower_right(_lower_right) {};
};



//------------------------------------------------------------------
// Box
//------------------------------------------------------------------
template <class A> class Box
{

public:

	// Data (public for speed)
	Vec3<A> center;
	Vec3<A> size;

	// Constructors
	inline Box(const Vec3<A> &_center=Vec3<A>(0,0,0),const Vec3<A> &_size=Vec3<A>(0,0,0)) : center(_center),size(_size) {};
};


