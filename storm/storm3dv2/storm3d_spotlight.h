// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_SPOTLIGHT_H
#define INCLUDED_STORM3D_SPOTLIGHT_H

#include <istorm3d_spotlight.h>
#include <boost/scoped_ptr.hpp>

class Storm3D;
class Storm3D_Camera;
class Storm3D_Scene;
struct Storm3D_SpotlightData;
struct IDirect3D9;
struct IDirect3DDevice9;

class Storm3D_Spotlight: public IStorm3D_Spotlight
{
	boost::scoped_ptr<Storm3D_SpotlightData> data;

public:
	enum SpotType
	{
		Legacy = 0,
		DepthBuffer = 1,
		AtiBuffer = 2
	};

	Storm3D_Spotlight(Storm3D &storm, IDirect3D9 &d3d, IDirect3DDevice9 &device, bool ps14, bool ps20);
	~Storm3D_Spotlight();

	void testVisibility(Storm3D_Camera &camera);
	void enable(bool enable);
	bool enabled() const;

	void setPosition(const VC3 &position);
	void setDirection(const VC3 &direction);
	void setFov(float fov);
	void setConeFov(float fov);
	void setRange(float range);
	void setClipRange(float range);

	void setEnableClip( bool enableClip );

	void enableFeature(Feature feature, bool enable);
	bool featureEnabled(Feature feature) const;

	void setType(Type type);
	Type getType() const;
	void setNoShadowModel(const IStorm3D_Model *model);
	const IStorm3D_Model *getNoShadowModel() const;

	void setClipPlanes(const float *cameraView);
	void enableSmoothing(bool enable);
	bool setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene &scene);

	void setProjectionTexture(boost::shared_ptr<IStorm3D_Texture> texture);
	void setConeTexture(boost::shared_ptr<IStorm3D_Texture> texture);
	bool hasConeTexture() const;
	void setColorMultiplier(const COL &color);
	void setConeMultiplier(float scalar);
	void setSmoothness(float smoothness);
	const COL &getColorMultiplier() const;
	QUAT getOrientation() const;

	bool setAsRenderTarget(const float *cameraView);
	Storm3D_Camera &getCamera();

	void renderStencilCone(Storm3D_Camera &camera);
	void applyTextures(const float *cameraView, const float *cameraViewProjection, Storm3D &storm, bool renderShadows);
	void applyTerrainShader(bool renderShadows);
	void applySolidShader(bool renderShadows);
	void applyNormalShader(bool renderShadows);
	void renderCone(Storm3D_Camera &camera, float timeFactor, bool renderGlows);
	void debugRender();

	void releaseDynamicResources();
	void recreateDynamicResources();

	static void querySizes(Storm3D &storm, bool ps14, int shadowQuality);
	static void createShadowBuffers(Storm3D &storm, IDirect3D9 &d3d, IDirect3DDevice9 &device, bool ps14, bool ps20, int shadowQuality);
	static void freeShadowBuffers();
	static void clearCache();
	static SpotType getSpotType();
};

#endif
