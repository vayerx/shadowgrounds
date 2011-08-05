// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_PROCEDURALMANAGER_H
#define INCLUDED_ISTORM3D_PROCEDURALMANAGER_H

#pragma once

#include <string>
#include <DatatypeDef.h>

class IStorm3D_ProceduralManager
{
public:
	virtual ~IStorm3D_ProceduralManager() {}

	struct Layer
	{
		std::string texture;
		VC2 scale;
		VC2 speed;

		Layer() :
			texture(""),
			scale(0.0f),
			speed(0.0f)
		{
		}
	};

	struct Source
	{
		Layer texture;
		Layer offset;

		VC2 radius;
		VC2 linearSpeed;

		Source() :
			texture(),
			offset(),
			radius(0.0f),
			linearSpeed(0.0f)
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

		Effect() :
			source1(),
			source2(),
			distortion1(),
			distortion2(),
			fallback(""),
			enableDistortion(false)
		{
		}
	};

	virtual void addEffect(const std::string &name, const Effect &effect) = 0;
};

#endif
