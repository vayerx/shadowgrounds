// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_SPHERE_H
#define INCLUDED_C2_SPHERE_H

#pragma once
#include "c2_vectors.h"

template<class T>
struct TSphere
{
	Vec3<T> position;
	float radius;

	TSphere<T>()
	:	radius(0)
	{
	}

	TSphere<T>(const Vec3<T> &position_, float radius_)
	:	position(position_),
		radius(radius_)
	{
	}
};

typedef TSphere<float> Sphere;

#endif
