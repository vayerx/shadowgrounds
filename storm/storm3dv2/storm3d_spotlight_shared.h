// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_SPOTLIGHT_SHARED_H
#define INCLUDED_STORM3D_SPOTLIGHT_SHARED_H

#include <d3dx9.h>
#include <DatatypeDef.h>

class Storm3D_Camera;
class Storm3D_Scene;

struct Storm3D_SpotlightShared
{
	IDirect3DDevice9 &device;

	VC3 position;
	VC3 direction;
	float range;
	float fov; // in degrees
	COL color;

	// shadowProjection is from camera space -> light projection space
	// used to transform screen z coordinate for shadow comparison?
	D3DXMATRIX shadowProjection[2];

	// shaderProjection = world -> light projection space
	D3DXMATRIX shaderProjection[2];

	// light view and projection matrices
	D3DXMATRIX lightView[2];
	D3DXMATRIX lightViewProjection[2];
	D3DXMATRIX lightProjection;

	// world space -> shadowmap space
	D3DXMATRIX targetProjection;
	VC2I targetPos;

	float soffsetX;
	float soffsetY;
	float scaleY;
	float scaleX;
	unsigned int resolutionX;
	unsigned int resolutionY;

	Storm3D_SpotlightShared(IDirect3DDevice9 &device);
	~Storm3D_SpotlightShared();

	void updateMatrices(const D3DXMATRIX &cameraView, float bias);
	void updateMatricesOffCenter(const D3DXMATRIX &cameraView, const VC2 &minplane, const VC2 &maxplane, float height, Storm3D_Camera &camera);

	void setClipPlanes(const float *cameraView);
	bool setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene *scene = 0);
};

#endif
