// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_RAY_H
#define INCLUDED_C2_RAY_H

#pragma once
#include "c2_vectors.h"

template<class T>
struct TRay
{
	Vec3<T> origin;
	Vec3<T> direction;
	float range;

	TRay()
	:	range(0)
	{
	}

	TRay(const Vec3<T> &origin_, const Vec3<T> &direction_, float range_)
	:	origin(origin_),
		direction(direction_),
		range(range_)
	{
	}
};

typedef TRay<float> Ray;

#endif

