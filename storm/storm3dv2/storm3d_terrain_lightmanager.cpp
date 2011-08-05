// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <string>

#include "storm3d_terrain_lightmanager.h"
#include "istorm3d_terrain_rendererbase.h"
#include "storm3d.h"
#include "storm3d_texture.h"
#include "storm3d_spotlight.h"
#include "storm3d_fakespotlight.h"
#include "storm3d_scene.h"
#include "Storm3D_ShaderManager.h"
#include "VertexFormats.h"
#include <atlbase.h>
#include <d3d9.h>

#include "../../util/Debug_MemoryManager.h"

using namespace std;
using namespace boost;

typedef vector<shared_ptr<Storm3D_Spotlight> > SpotList;
typedef vector<shared_ptr<Storm3D_FakeSpotlight> > FakeSpotList;
typedef vector<Storm3D_LightTexture> FakeLightList;

namespace {

	void setSpotBufferProperties(IDirect3DDevice9 &device, bool atiShaders)
	{
	}
}

// Storm3D_LightTexture

Storm3D_LightTexture::Storm3D_LightTexture(const VC2 &start_, const VC2 &end_, IStorm3D_Texture &texture_, const COL &color_)
:	start(start_),
	end(end_),
	color(color_)
{
	texture = boost::shared_ptr<Storm3D_Texture>(static_cast<Storm3D_Texture *> (&texture_), std::mem_fun(&Storm3D_Texture::Release));
	texture->AddRef();
}

Storm3D_LightTexture::~Storm3D_LightTexture()
{
}

// Storm3D_LightManager::Data

struct Storm3D_TerrainLightManager::Data
{
	Storm3D &storm;
	IStorm3D_TerrainRendererBase &renderer;
	
	SpotList &spots;
	FakeSpotList &fakeSpots;
	FakeLightList &fakeLights;

	IDirect3DDevice9 &device;

	frozenbyte::storm::VertexShader coneAtiVertexShader;
	frozenbyte::storm::VertexShader coneNvVertexShader;

	Data(Storm3D &storm_, IStorm3D_TerrainRendererBase &renderer_, SpotList &spots_, FakeSpotList &fakeSpots_, FakeLightList &fakeLights_)
	:	storm(storm_),
		renderer(renderer_),
		spots(spots_),
		fakeSpots(fakeSpots_),
		fakeLights(fakeLights_),
		device(*storm.GetD3DDevice()),

		coneAtiVertexShader(device),
		coneNvVertexShader(device)
	{
		coneAtiVertexShader.createAtiConeShader();
		coneNvVertexShader.createNvConeShader();
	}

	void renderSpotBuffers(Storm3D_Scene &scene, bool renderShadows)
	{
		bool atiShaders = false;
		int spotType = Storm3D_Spotlight::getSpotType();
		if(spotType == Storm3D_Spotlight::AtiBuffer)
			atiShaders = true;

		if(atiShaders)
			Storm3D_ShaderManager::GetSingleton()->setAtiDepthShaders();
		else
		{
			device.SetRenderState(D3DRS_COLORWRITEENABLE, 0);
			Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		}

		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());
		Storm3D_Spotlight::clearCache();

		if(renderShadows)
		{
			SpotList::iterator it = spots.begin();
			for(; it != spots.end(); ++it)
			{
				Storm3D_Spotlight *spot = it->get();
				if(!spot || !spot->enabled() || !spot->featureEnabled(IStorm3D_Spotlight::Shadows))
					continue;

				// Test spot visibility with foo scissor rect -- not really an optimal way of doing this
				if(!spot->setScissorRect(camera, VC2I(100, 100), scene))
					continue;
				device.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

				const float *cameraView = camera.GetView4x4Matrix();
				if(!spot->setAsRenderTarget(cameraView))
					continue;

                /* debug
				static unsigned int count = 0;
				count++;
				device.Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, float(count % 21) / 20.0f, 0);
                */
				//glClearDepth(float(count % 11) / 10.0f);
				//glClear(GL_DEPTH_BUFFER_BIT);

				renderer.render(IStorm3D_TerrainRendererBase::SpotBuffer, scene, spot);
			}
		}

		device.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		device.SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		device.SetPixelShader(0);

		camera.Apply();
	}

	void renderFakeSpotBuffers(Storm3D_Scene &scene, bool renderShadows)
	{
		// Renders fake shadows to texture?
		Storm3D_ShaderManager::GetSingleton()->setFakeDepthShaders();
		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());

		device.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		//device.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);

		Storm3D_FakeSpotlight::clearCache();
		bool rendered = false;

		FakeSpotList::iterator it = fakeSpots.begin();
		for(; it != fakeSpots.end(); ++it)
		{
			Storm3D_FakeSpotlight *spot = it->get();
			if(!spot || !spot->enabled())
				continue;

			const float *cameraView = camera.GetView4x4Matrix();
			if(!spot->setAsRenderTarget(cameraView))
				continue;

			renderer.render(IStorm3D_TerrainRendererBase::FakeSpotBuffer, scene, 0, spot);
			rendered = true;
		}

		//if(rendered)
		//	Storm3D_FakeSpotlight::filterBuffers(storm, device);

		device.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device.SetPixelShader(0);

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		camera.Apply();
	}

	enum LightType
	{
		RealSolid,
		RealAlpha,
		Fake
	};

	void renderSpotLights(Storm3D_Scene &scene, bool renderShadows, LightType type)
	{
		// this renders spotlight light & shadows
		//setTracing(true);
		bool atiShaders = false;
		int spotType = Storm3D_Spotlight::getSpotType();
		if(spotType == Storm3D_Spotlight::AtiBuffer)
			atiShaders = true;

		if(atiShaders)
			Storm3D_ShaderManager::GetSingleton()->setAtiShadowShaders();
		else
			Storm3D_ShaderManager::GetSingleton()->setProjectedShaders();

		device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device.SetRenderState(D3DRS_ALPHAREF, 0x1);
		device.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		for(int i = 0; i < 4; ++i)
		{
			if(i != 2)
			{
				device.SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				device.SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			}
		}

		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());
		SpotList::iterator it = spots.begin();
		for(; it != spots.end(); ++it)
		{
			Storm3D_Spotlight *spot = it->get();
			if(!spot || !spot->enabled())
				continue;

			// Test spot visibility with foo scissor rect -- not really an optimal way of doing this
			if(!spot->setScissorRect(camera, VC2I(100, 100), scene))
				continue;
			device.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

			if(!atiShaders)
			{
				device.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
				device.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
				device.SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
				device.SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			}

			const float *cameraView = camera.GetView4x4Matrix();
			const float *cameraViewProjection = camera.GetViewProjection4x4Matrix();
			spot->applyTextures(cameraView, cameraViewProjection, storm, renderShadows);

			if(type == RealSolid)
				renderer.render(IStorm3D_TerrainRendererBase::SpotProjectionSolid, scene, spot);
			else
			{
				renderer.render(IStorm3D_TerrainRendererBase::SpotProjectionDecal, scene, spot);
				renderer.render(IStorm3D_TerrainRendererBase::SpotProjectionAlpha, scene, spot);
			}
		}

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		for(int i = 0; i < 4; ++i)
		{
			device.SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			device.SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		}
		//setTracing(false);

		device.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

		device.SetTexture(1, 0);
		device.SetTexture(2, 0);
		device.SetTexture(3, 0);
		device.SetTexture(4, 0);

		device.SetPixelShader(0);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		device.SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
	}

	void renderFakeSpotLights(Storm3D_Scene &scene, bool renderShadows)
	{
		// Renders fake shadows to screen?
		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());
		Storm3D_ShaderManager::GetSingleton()->setFakeShadowShaders();

		device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device.SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		device.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
		device.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
		device.SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
		device.SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);

		device.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		frozenbyte::storm::enableMipFiltering(device, 0, 4, false);

		FakeSpotList::iterator it = fakeSpots.begin();
		for(; it != fakeSpots.end(); ++it)
		{
			Storm3D_FakeSpotlight *spot = it->get();
			if(!spot || !spot->enabled())
				continue;

			const float *cameraView = camera.GetView4x4Matrix();
			spot->applyTextures(cameraView);

			renderer.render(IStorm3D_TerrainRendererBase::FakeSpotProjection, scene, 0, spot);
		}

		frozenbyte::storm::enableMipFiltering(device, 0, 4, true);

		device.SetPixelShader(0);
		device.SetVertexShader(0);
		device.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		device.SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

		device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device.SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
	}

	void renderBuffers(Storm3D_Scene &scene, bool renderShadows, bool renderFakeBuffers)
	{
		renderSpotBuffers(scene, renderShadows);

		if(renderFakeBuffers)
			renderFakeSpotBuffers(scene, renderShadows);
	}

	void renderLights(LightType type, Storm3D_Scene &scene, bool renderShadows)
	{
		if(type == Fake)
			renderFakeSpotLights(scene, renderShadows);
		else if(type == RealSolid || type == RealAlpha)
		{
			renderSpotLights(scene, renderShadows, type);
			frozenbyte::storm::enableMinMagFiltering(device, 0, 1, true);
		}
	}

	void renderFakeLights(const VC2I &renderSize)
	{
		// Renders fake light (not shadows)?
		device.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device.SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

		device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		device.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		device.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		device.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

		//device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
		device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		device.SetVertexShader(0);
		device.SetPixelShader(0);
		device.SetFVF(FVF_VXFORMAT_2D);

		FakeLightList::iterator it = fakeLights.begin();
		for(; it != fakeLights.end(); ++it)
		{
			Storm3D_LightTexture &lightTexture = *it;
			DWORD color = lightTexture.color.GetAsD3DCompatibleARGB();

			float x1 = lightTexture.start.x * renderSize.x;
			float y1 = lightTexture.start.y * renderSize.y;
			float x2 = lightTexture.end.x * renderSize.x;
			float y2 = lightTexture.end.y * renderSize.y;

			VXFORMAT_2D buffer[4];
			buffer[0] = VXFORMAT_2D(VC3(x1, y2, 1.f), 1.f, color, VC2(0.f, 1.f));
			buffer[1] = VXFORMAT_2D(VC3(x1, y1, 1.f), 1.f, color, VC2(0.f, 0.f));
			buffer[2] = VXFORMAT_2D(VC3(x2, y2, 1.f), 1.f, color, VC2(1.f, 1.f));
			buffer[3] = VXFORMAT_2D(VC3(x2, y1, 1.f), 1.f, color, VC2(1.f, 0.f));

			lightTexture.texture->Apply(0);
			device.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, buffer, sizeof(VXFORMAT_2D));
		}

		device.SetTexture(0, 0);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		device.SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

		device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}

	void renderCones(Storm3D_Scene &scene, bool renderShadows, float timeFactor, bool renderGlows)
	{
		// this draws spotlight cone
		bool atiShaders = false;
		int spotType = Storm3D_Spotlight::getSpotType();
		if(spotType == Storm3D_Spotlight::AtiBuffer)
			atiShaders = true;

		if(atiShaders)
		{
			Storm3D_ShaderManager::GetSingleton()->setAtiShadowShaders();
			coneAtiVertexShader.apply();
		}
		else
		{
			device.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
			device.SetPixelShader(0);

			Storm3D_ShaderManager::GetSingleton()->setProjectedShaders();
			coneNvVertexShader.apply();
		}

		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());

		device.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		SpotList::iterator it = spots.begin();
		for(; it != spots.end(); ++it)
		{
			Storm3D_Spotlight *spot = it->get();
			if(!spot || !spot->enabled() || !spot->featureEnabled(IStorm3D_Spotlight::ConeVisualization))
				continue;

			if(renderShadows)
				spot->renderCone(camera, timeFactor, renderGlows);
		}

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		device.SetPixelShader(0);

		device.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
};

// Storm3D_LightManager

Storm3D_TerrainLightManager::Storm3D_TerrainLightManager(Storm3D &storm, IStorm3D_TerrainRendererBase &renderer, SpotList &spots, FakeSpotList &fakeSpots, FakeLightList &fakeLights)
{
	scoped_ptr<Data> tempData(new Data(storm, renderer, spots, fakeSpots, fakeLights));
	data.swap(tempData);
}

Storm3D_TerrainLightManager::~Storm3D_TerrainLightManager()
{
}

void Storm3D_TerrainLightManager::setFog(float start, float end)
{
	float range = start - end;

	FakeSpotList::iterator it = data->fakeSpots.begin();
	for(; it != data->fakeSpots.end(); ++it)
	{
		float factor = (*it)->getPlaneHeight() - end;
		factor /= range;
		if(factor < 0.f)
			factor = 0.f;
		if(factor > 1.f)
			factor = 1.f;

		factor = 1.f - factor;
		(*it)->setFogFactor(factor);
	}
}

void Storm3D_TerrainLightManager::renderProjectedRenderTargets(Storm3D_Scene &scene, bool renderShadows, bool renderFakeBuffers)
{
	data->renderBuffers(scene, renderShadows, renderFakeBuffers);
}

void Storm3D_TerrainLightManager::renderProjectedFakeLights(Storm3D_Scene &scene, bool renderShadows)
{
	data->renderLights(Data::Fake, scene, renderShadows);
}

void Storm3D_TerrainLightManager::renderProjectedLightsSolid(Storm3D_Scene &scene, bool renderShadows)
{
	data->renderLights(Data::RealSolid, scene, renderShadows);
}

void Storm3D_TerrainLightManager::renderProjectedLightsAlpha(Storm3D_Scene &scene, bool renderShadows)
{
	data->renderLights(Data::RealAlpha, scene, renderShadows);
}

void Storm3D_TerrainLightManager::renderFakeLights(const VC2I &renderSize)
{
	data->renderFakeLights(renderSize);
}

void Storm3D_TerrainLightManager::renderCones(Storm3D_Scene &scene, bool renderShadows, bool renderGlows)
{
	float timeFactor = float(scene.time_dif) * 0.001f;
	data->renderCones(scene, renderShadows, timeFactor, renderGlows);
}
