// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_LIGHTMANAGER_H
#define INCLUDED_STORM3D_TERRAIN_LIGHTMANAGER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "DatatypeDef.h"

class IStorm3D_Texture;
class IStorm3D_TerrainRendererBase;
class Storm3D;
class Storm3D_TerrainModels;
class Storm3D_Scene;
class Storm3D_Texture;
class Storm3D_Spotlight;
class Storm3D_FakeSpotlight;

struct Storm3D_LightTexture
{
	VC2 start;
	VC2 end;

	boost::shared_ptr<Storm3D_Texture> texture;
	COL color;

	Storm3D_LightTexture(const VC2 &start_, const VC2 &end_, IStorm3D_Texture &texture_, const COL &color_);
	~Storm3D_LightTexture();
};


class Storm3D_TerrainLightManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	Storm3D_TerrainLightManager(Storm3D &storm, IStorm3D_TerrainRendererBase &renderer, std::vector<boost::shared_ptr<Storm3D_Spotlight> > &spots, std::vector<boost::shared_ptr<Storm3D_FakeSpotlight> > &fakeSpots, std::vector<Storm3D_LightTexture> &fakeLights);
	~Storm3D_TerrainLightManager();

	void setFog(float start, float end);
	void renderProjectedRenderTargets(Storm3D_Scene &scene, bool renderShadows, bool renderFakeBuffers);
	void renderProjectedFakeLights(Storm3D_Scene &scene, bool renderShadows);
	void renderProjectedLightsSolid(Storm3D_Scene &scene, bool renderShadows);
	void renderProjectedLightsAlpha(Storm3D_Scene &scene, bool renderShadows);
	void renderFakeLights(const VC2I &renderSize);

	void renderCones(Storm3D_Scene &scene, bool renderShadows, bool renderGlows);
};

#endif
