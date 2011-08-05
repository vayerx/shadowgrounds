#pragma once

#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"

class ST3D_EXP_DLLAPI IStorm3D_Line
{
public:
	virtual ~IStorm3D_Line() {}

	// Add as many as you like (>= 2)
	virtual void AddPoint(const Vector &position) = 0;
	virtual int GetPointCount() = 0;
	virtual void RemovePoint(int index) = 0;

	// Units in world space
	virtual void SetThickness(float thickness) = 0;
	virtual void SetColor(int color) = 0;
};
