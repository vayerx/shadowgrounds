
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "procedural_applier.h"
#include "procedural_properties.h"
#include <IStorm3D.h>
#include <IStorm3D_Terrain.h>
#include <istorm3D_terrain_renderer.h>
#include <IStorm3D_ProceduralManager.h>

namespace util {
namespace {

	void mapLayer(IStorm3D_ProceduralManager::Source &destination, const ProceduralProperties::Source &source)
	{
		destination.texture.texture = source.texture.texture;
		destination.texture.scale = source.texture.scale;
		destination.texture.speed = source.texture.speed;
		destination.offset.texture = source.offset.texture;
		destination.offset.scale = source.offset.scale;
		destination.offset.speed = source.offset.speed;
		destination.radius = source.radius;
		destination.linearSpeed = source.linearSpeed;
	}

	void mapDistortion(IStorm3D_ProceduralManager::Source &destination, const ProceduralProperties::Source &source)
	{
		destination.texture.scale = source.texture.scale;
		destination.texture.speed = source.texture.speed;
		destination.offset.texture = source.offset.texture;
		destination.offset.scale = source.offset.scale;
		destination.offset.speed = source.offset.speed;
		destination.radius = source.radius;
		destination.linearSpeed = source.linearSpeed;
	}

} // unnamed

void applyStorm(IStorm3D &storm, const ProceduralProperties &properties)
{
	const VC2I &size = properties.getTextureSize();
	storm.setProceduralTextureSize(size);
}

void applyRenderer(IStorm3D &storm, const ProceduralProperties &properties)
{
	IStorm3D_ProceduralManager &manager = storm.getProceduralManager();
	
	int effectAmount = properties.getEffectAmount();
	for(int i = 0; i < effectAmount; ++i)
	{
		const ProceduralProperties::Effect &source = properties.getEffect(i);
		IStorm3D_ProceduralManager::Effect effect;

		mapLayer(effect.source1, source.source1);
		mapLayer(effect.source2, source.source2);
		mapDistortion(effect.distortion1, source.distortion1);
		mapDistortion(effect.distortion2, source.distortion2);
		
		effect.enableDistortion = source.enableDistortion;
		effect.fallback = source.fallback;

		manager.addEffect(source.name, effect);
	}
}

} // util
