// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_TERRAIN_RENDERER_H
#define INCLUDED_ISTORM3D_TERRAIN_RENDERER_H

#include <boost/shared_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D_Spotlight;
class IStorm3D_FakeSpotlight;
class IStorm3D_Texture;

class IStorm3D_TerrainRenderer
{
public:
	virtual ~IStorm3D_TerrainRenderer() {}

	// Normalized coordinates (0 .. 1)
	virtual void renderLightTexture(const VC2 &start, const VC2 &end, IStorm3D_Texture &texture, const COL &color) = 0;

	virtual boost::shared_ptr<IStorm3D_Spotlight> createSpot() = 0;
	virtual void deleteSpot(boost::shared_ptr<IStorm3D_Spotlight> &spot) = 0;
	virtual boost::shared_ptr<IStorm3D_FakeSpotlight> createFakeSpot() = 0;
	virtual void deleteFakeSpot(boost::shared_ptr<IStorm3D_FakeSpotlight> &spot) = 0;

	virtual void setMovieAspectRatio(bool enable, bool fade = false) = 0;

	enum RenderMode
	{
		Normal,
		LightOnly,
		TexturesOnly
	};

	enum RenderFeature
	{
		RenderTargets,
		Glow,
		BetterGlowSampling,
		Distortion,
		SmoothShadows,
		Cone,
		FakeLights,
		SpotShadows,
		Wireframe,
		LightmapFiltering,
		BlackAndWhite,
		Collision,
		ModelRendering,
		TerrainObjectRendering,
		HeightmapRendering,
		TerrainTextures,
		Particles,
		AlphaTest,
		MaterialAmbient,

		FreezeCameraCulling,
		FreezeSpotCulling,
		RenderDecals,
		RenderSpotDebug,
		RenderFakeShadowDebug,
		RenderGlowDebug,
		RenderBoned,
		RenderSkyModel,

		LightmappedCollision,
		ProceduralFallback,
		Reflection,
		HalfRendering,
		FakeShadows,
		ParticleReflection,

		ForcedDirectionalLightEnabled,

		SkyModelGlow,
		AdditionalAlphaTestPass,
	};

	virtual void setRenderMode(RenderMode mode) = 0;
	virtual bool enableFeature(RenderFeature feature, bool enable) = 0;
	
	enum FloatValue
	{
		LightmapMultiplier,
		OutdoorLightmapMultiplier,
		ForceAmbient,
		GlowFactor,
		GlowTransparencyFactor,
		GlowAdditiveFactor
	};

	virtual void setFloatValue(FloatValue type, float value) = 0;

	enum ColorValue
	{
		LightmapMultiplierColor,
		OutdoorLightmapMultiplierColor,

		TerrainObjectOutsideFactor,
		TerrainObjectInsideFactor,

		ForcedDirectionalLightColor
	};

	virtual void setColorValue(ColorValue type, const COL &value) = 0;
	virtual COL getColorValue(ColorValue type) const = 0;

	enum VectorValue
	{
		ForcedDirectionalLightDirection
	};

	virtual void setVectorValue(VectorValue type, const VC3 &value) = 0;
	virtual VC3 getVectorValue(VectorValue type) const = 0;

	// contrast (-1..1), brightness (-1..1), colorFactors ((-1..1),(-1..1),(-1..1))
	virtual void setColorEffect(float contrast, float brightness, const COL &colorFactors) = 0;
};

#endif
