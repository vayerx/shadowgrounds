// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_PROCEDURALMANAGER_H
#define INCLUDED_ISTORM3D_PROCEDURALMANAGER_H

#pragma once

#include <string>
#include <datatypedef.h>

class IStorm3D_ProceduralManager
{
public:
	virtual ~IStorm3D_ProceduralManager() {}

	struct Layer
	{
		std::string texture;
		VC2 scale;
		VC2 speed;

		Layer()
		{
		}
	};

	struct Source
	{
		Layer texture;
		Layer offset;

		VC2 radius;
		VC2 linearSpeed;

		Source()
		{
		}
	};

	struct Effect
	{
		Source source1;
		Source source2;
		Source distortion1;
		Source distortion2;

		std::string fallback;
		bool enableDistortion;

		Effect()
		:	enableDistortion(false)
		{
		}
	};

	virtual void addEffect(const std::string &name, const Effect &effect) = 0;
};

#endif
