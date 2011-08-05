// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_FAKESPOTLIGHT_H
#define INCLUDED_STORM3D_FAKESPOTLIGHT_H

#include <istorm3d_fakespotlight.h>
#include <boost/scoped_ptr.hpp>

class Storm3D;
class Storm3D_Camera;
struct IDirect3D9;
struct IDirect3DDevice9;
struct D3DXMATRIX;

class Storm3D_FakeSpotlight: public IStorm3D_FakeSpotlight
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	Storm3D_FakeSpotlight(Storm3D &storm, IDirect3D9 &d3d, IDirect3DDevice9 &device);
	~Storm3D_FakeSpotlight();

	void testVisibility(Storm3D_Camera &camera);
	void disableVisibility();
	void enable(bool enable);
	bool enabled() const;

	void setPosition(const VC3 &position);
	void setDirection(const VC3 &direction);
	void setFov(float fov);
	void setRange(float range);
	void setColor(const COL &color, float fadeFactor);
	void setPlane(float height, const VC2 &minCorner, const VC2 &maxCorner);
	void renderObjectShadows(bool render);
	void setFogFactor(float f);

	void getPlane(VC2 &min, VC2 &max) const;
	float getPlaneHeight() const;
	bool shouldRenderObjectShadows() const;

	Storm3D_Camera &getCamera();
	void setClipPlanes(const float *cameraView);
	void setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize);

	bool setAsRenderTarget(const D3DXMATRIX &cameraView);
	void applyTextures(const D3DXMATRIX &cameraView);
	void renderProjection();
	void debugRender();

	void releaseDynamicResources();
	void recreateDynamicResources();

	static void filterBuffers(Storm3D &storm, IDirect3DDevice9 &device);
	static void querySizes(Storm3D &storm, int shadowQuality);
	static void createBuffers(Storm3D &storm, IDirect3D9 &d3d, IDirect3DDevice9 &device, int shadowQuality);
	static void freeBuffers();
	static void clearCache();
};

#endif
