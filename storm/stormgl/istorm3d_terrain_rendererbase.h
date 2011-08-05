// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_ISTORM3D_TERRAIN_RENDERERBASE_H
#define INCLUDED_ISTORM3D_TERRAIN_RENDERERBASE_H

class Storm3D_Scene;
class Storm3D_Spotlight;
class Storm3D_FakeSpotlight;

class IStorm3D_TerrainRendererBase
{
public:
	virtual ~IStorm3D_TerrainRendererBase() {}

	enum RenderMode
	{
		FakeSpotBuffer,
		FakeSpotProjection,
		SpotBuffer,
		SpotProjectionSolid,
		SpotProjectionDecal,
		SpotProjectionAlpha,
		Glow
	};

	virtual void render(RenderMode mode, Storm3D_Scene &scene, Storm3D_Spotlight *spot = 0, Storm3D_FakeSpotlight *fakeSpot = 0) = 0;
};

#endif
