// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_AABB_H
#define INCLUDED_C2_AABB_H

#pragma once
#include "c2_vectors.h"

template<class T>
struct TAABB
{
	Vec3<T> mmin;
	Vec3<T> mmax;

	TAABB(const Vec3<T> &min_, const Vec3<T> &max_)
	:	mmin(min_),
		mmax(max_)
	{
	}

	TAABB()
	{
	}
};

typedef TAABB<float> AABB;

#endif
