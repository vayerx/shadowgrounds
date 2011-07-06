// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_SPOTLIGHT_SHARED_H
#define INCLUDED_STORM3D_SPOTLIGHT_SHARED_H

#include <d3dx9.h>
#include <datatypedef.h>

class Storm3D_Camera;
class Storm3D_Scene;

struct Storm3D_SpotlightShared
{
	IDirect3DDevice9 &device;

	VC3 position;
	VC3 direction;
	float range;
	float fov;
	COL color;

	D3DXMATRIX shadowProjection[2];
	D3DXMATRIX shaderProjection[2];
	D3DXMATRIX lightView[2];
	D3DXMATRIX lightViewProjection[2];
	D3DXMATRIX lightProjection;

	D3DXMATRIX targetProjection;
	VC2I targetPos;

	float soffsetX;
	float soffsetY;
	float scaleY;
	float scaleX;
	int resolutionX;
	int resolutionY;

	Storm3D_SpotlightShared(IDirect3DDevice9 &device);
	~Storm3D_SpotlightShared();

	void updateMatrices(const float *cameraView, float bias);
	void updateMatricesOffCenter(const float *cameraView, const VC2 &minplane, const VC2 &maxplane, float height, Storm3D_Camera &camera);

	void setClipPlanes(const float *cameraView);
	bool setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene *scene = 0);
};

#endif
