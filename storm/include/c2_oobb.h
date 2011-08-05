// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_OOBB_H
#define INCLUDED_C2_OOBB_H

#pragma once
#include "c2_vectors.h"

template<class T>
struct TOOBB
{
	Vec3<T> center;
	Vec3<T> axes[3];
	Vec3<T> extents;

	TOOBB()
	{
	}
};

typedef TOOBB<float> OOBB;

#endif
