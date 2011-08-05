// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_SPOTLIGHT_SHARED_H
#define INCLUDED_STORM3D_SPOTLIGHT_SHARED_H

#include <DatatypeDef.h>
#include "igios3D.h"

class Storm3D_Camera;
class Storm3D_Scene;

struct Storm3D_SpotlightShared
{
	VC3 position;
	VC3 direction;
	float range;
	float fov; // in degrees
	COL color;

	// shadowProjection is from camera space -> light projection space
	// used to transform screen z coordinate for shadow comparison?
	D3DXMATRIX shadowProjection;

	// shaderProjection = world -> light projection space
	D3DXMATRIX shaderProjection;

	// light view and projection matrices
	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;

	// world space -> shadowmap space
	D3DXMATRIX targetProjection;

	unsigned int resolutionX;
	unsigned int resolutionY;

	Storm3D_SpotlightShared();
	~Storm3D_SpotlightShared();

	void updateMatrices(const D3DXMATRIX &cameraView, float bias);

	void setClipPlanes(const float *cameraView);
	bool setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene *scene = 0);
};

#endif
