// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_RENDERER_H
#define INCLUDED_STORM3D_TERRAIN_RENDERER_H

#include <istorm3D_terrain_renderer.h>
#include "istorm3d_terrain_rendererbase.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <DatatypeDef.h>

class Storm3D_TerrainHeightmap;
class Storm3D_TerrainGroup;
class Storm3D_TerrainModels;
class Storm3D_TerrainDecalSystem;
class Storm3D_Spotlight;
class Storm3D_Model;
class Storm3D_Scene;
class Storm3D;
class IStorm3D_Spotlight;
class IStorm3D_FakeSpotlight;
class IStorm3D_Texture;
class Storm3D_ParticleSystem;

struct Storm3D_TerrainRendererData;

class Storm3D_TerrainRenderer: 
	public IStorm3D_TerrainRenderer,
	public IStorm3D_TerrainRendererBase
{
	boost::scoped_ptr<Storm3D_TerrainRendererData> data;

public:
	Storm3D_TerrainRenderer(Storm3D &storm, Storm3D_TerrainHeightmap &heightMap, Storm3D_TerrainGroup &groups, Storm3D_TerrainModels &models, Storm3D_TerrainDecalSystem &decalSystem);
	~Storm3D_TerrainRenderer();

	boost::shared_ptr<IStorm3D_Spotlight> createSpot();
	void deleteSpot(boost::shared_ptr<IStorm3D_Spotlight> &spot);
	boost::shared_ptr<IStorm3D_FakeSpotlight> createFakeSpot();
	void deleteFakeSpot(boost::shared_ptr<IStorm3D_FakeSpotlight> &spot);

	void setMovieAspectRatio(bool enable, bool fade);
	void setParticleSystem(Storm3D_ParticleSystem *particlesystem);

	void setRenderMode(IStorm3D_TerrainRenderer::RenderMode mode);
	bool enableFeature(RenderFeature feature, bool enable);
	void setFloatValue(FloatValue type, float value);
	void setColorValue(ColorValue type, const COL &value);
	COL getColorValue(ColorValue type) const;
	void setVectorValue(VectorValue type, const VC3 &value);
	VC3 getVectorValue(VectorValue type) const;

	// contrast (-1..1), brightness (-1..1), colorFactors ((-1..1),(-1..1),(-1..1))
	void setColorEffect(float contrast, float brightness, const COL &colorFactors);

	void renderLightTexture(const VC2 &start, const VC2 &end, IStorm3D_Texture &texture, const COL &color);
	void setAmbient(const COL &color);
	void setClearColor(const COL &color);
	void setSkyBox(Storm3D_Model *model);
	void setFog(bool enable, float startHeight, float endHeight, const COL &color);

	void updateVisibility(Storm3D_Scene &scene, int timeDelta);
	void renderTargets(Storm3D_Scene &scene);
	void renderBase(Storm3D_Scene &scene);
	void render(IStorm3D_TerrainRendererBase::RenderMode mode, Storm3D_Scene &scene, Storm3D_Spotlight *spot, Storm3D_FakeSpotlight *fakeSpot);

	void releaseDynamicResources();
	void recreateDynamicResources();

	static void querySizes(Storm3D &storm, bool enableGlow);
	static void createRenderBuffers(Storm3D &storm, int lightmapQuality);
	static void freeRenderBuffers();
	static void createSecondaryRenderBuffers(Storm3D &storm, bool enableGlow);
	static void freeSecondaryRenderBuffers();
	static bool hasNeededBuffers();
};

#endif
