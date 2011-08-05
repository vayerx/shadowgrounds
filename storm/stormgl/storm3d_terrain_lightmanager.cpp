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
#include "igios3D.h"

#include "../../util/Debug_MemoryManager.h"

using namespace std;
using namespace boost;

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
	
	StormSpotList &spots;
	FakeSpotList &fakeSpots;
	FakeLightList &fakeLights;

	frozenbyte::storm::VertexShader coneNvVertexShader;

	Data(Storm3D &storm_, IStorm3D_TerrainRendererBase &renderer_, StormSpotList &spots_, FakeSpotList &fakeSpots_, FakeLightList &fakeLights_)
	:	storm(storm_),
		renderer(renderer_),
		spots(spots_),
		fakeSpots(fakeSpots_),
		fakeLights(fakeLights_),
		coneNvVertexShader()
	{
		coneNvVertexShader.createNvConeShader();
	}

	void renderSpotBuffers(Storm3D_Scene &scene, bool renderShadows)
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());
		Storm3D_Spotlight::clearCache();

		if(renderShadows)
		{
			StormSpotList::iterator it = spots.begin();
			for(; it != spots.end(); ++it)
			{
				Storm3D_Spotlight *spot = it->get();
				if(!spot || !spot->enabled() || !spot->featureEnabled(IStorm3D_Spotlight::Shadows))
					continue;

				// Test spot visibility with foo scissor rect -- not really an optimal way of doing this
				if(!spot->setScissorRect(camera, VC2I(100, 100), scene))
					continue;
				glDisable(GL_SCISSOR_TEST);

				if(!spot->setAsRenderTarget(camera.GetViewMatrix()))
					continue;

				glErrors();
				renderer.render(IStorm3D_TerrainRendererBase::SpotBuffer, scene, spot);
				glErrors();
			}
		}

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		frozenbyte::storm::PixelShader::disable();

		camera.Apply();
	}

	void renderFakeSpotBuffers(Storm3D_Scene &scene, bool renderShadows)
	{
		Storm3D_ShaderManager::GetSingleton()->setFakeDepthShaders();
		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());

		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		bool rendered = false;

		FakeSpotList::iterator it = fakeSpots.begin();
		for(; it != fakeSpots.end(); ++it)
		{
			Storm3D_FakeSpotlight *spot = it->get();
			if(!spot || !spot->enabled())
				continue;

			if(!spot->setAsRenderTarget(camera.GetViewMatrix()))
				continue;

			renderer.render(IStorm3D_TerrainRendererBase::FakeSpotBuffer, scene, 0, spot);
			rendered = true;
		}

		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		frozenbyte::storm::PixelShader::disable();

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
		Storm3D_ShaderManager::GetSingleton()->setProjectedShaders();

		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);

		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());
		StormSpotList::iterator it = spots.begin();
		for(; it != spots.end(); ++it)
		{
			Storm3D_Spotlight *spot = it->get();
			if(!spot || !spot->enabled())
				continue;

			// Test spot visibility with foo scissor rect -- not really an optimal way of doing this
			if(!spot->setScissorRect(camera, VC2I(100, 100), scene))
				continue;
			// glDisable below makes at least some spotlights visible through movie top & bottom "black bars"
			//glDisable(GL_SCISSOR_TEST);

			spot->applyTextures(camera.GetViewMatrix(), storm, renderShadows);

			for(int i = 0; i < 4; ++i)
			{
				if(i != 2)
				{
					glActiveTexture(GL_TEXTURE0 + i);
					glClientActiveTexture(GL_TEXTURE0 + i);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				}
			}
	
			if(type == RealSolid)
				renderer.render(IStorm3D_TerrainRendererBase::SpotProjectionSolid, scene, spot);
			else
			{
				renderer.render(IStorm3D_TerrainRendererBase::SpotProjectionDecal, scene, spot);
				renderer.render(IStorm3D_TerrainRendererBase::SpotProjectionAlpha, scene, spot);
			}
		}

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE2);
		glClientActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE3);
		glClientActiveTexture(GL_TEXTURE3);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE4);
		glClientActiveTexture(GL_TEXTURE4);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);

		frozenbyte::storm::PixelShader::disable();
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_CLIP_PLANE1);
		glDisable(GL_CLIP_PLANE2);
	}

	void renderFakeSpotLights(Storm3D_Scene &scene, bool renderShadows)
	{
		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());
		Storm3D_ShaderManager::GetSingleton()->setFakeShadowShaders();

		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);

		FakeSpotList::iterator it = fakeSpots.begin();
		for(; it != fakeSpots.end(); ++it)
		{
			Storm3D_FakeSpotlight *spot = it->get();
			if(!spot || !spot->enabled())
				continue;

			spot->applyTextures(camera.GetViewMatrix());

			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
			renderer.render(IStorm3D_TerrainRendererBase::FakeSpotProjection, scene, 0, spot);
		}

		frozenbyte::storm::PixelShader::disable();
		frozenbyte::storm::VertexShader::disable();
 
		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_BLEND);
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_CLIP_PLANE1);
		glDisable(GL_CLIP_PLANE2);
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
		}
	}

	//! Render fake lights
	/*
	\param renderSize  size of real screen eg 1024x768
	render target is set to texture that is next pow2 of renderSize
	*/
	void renderFakeLights(const VC2I &renderSize)
	{
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);		
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);

		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);

		frozenbyte::storm::VertexShader::disable();
		frozenbyte::storm::PixelShader::disable();

		FakeLightList::iterator it = fakeLights.begin();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glClientActiveTexture(GL_TEXTURE0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		Storm3D_SurfaceInfo screen = storm.GetScreenSize();
		glOrtho(0, screen.width, screen.height, 0, -1, 1);

		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		for (unsigned int i = 1; i < 7; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		for(; it != fakeLights.end(); ++it)
		{
			Storm3D_LightTexture &lightTexture = *it;
			DWORD color = lightTexture.color.GetAsD3DCompatibleARGB();

			float x1 = lightTexture.start.x * renderSize.x;
			float y1 = renderSize.y - lightTexture.start.y * renderSize.y;
			float x2 = lightTexture.end.x * renderSize.x;
			float y2 = renderSize.y - lightTexture.end.y * renderSize.y;

			VXFORMAT_2D buffer[4];
			buffer[0] = VXFORMAT_2D(VC3(x1, y2, 1.f), 1.f, color, VC2(0.f, 1.f));
			buffer[1] = VXFORMAT_2D(VC3(x1, y1, 1.f), 1.f, color, VC2(0.f, 0.f));
			buffer[2] = VXFORMAT_2D(VC3(x2, y2, 1.f), 1.f, color, VC2(1.f, 1.f));
			buffer[3] = VXFORMAT_2D(VC3(x2, y1, 1.f), 1.f, color, VC2(1.f, 0.f));

			lightTexture.texture->Apply(0);

			glVertexPointer(3, GL_FLOAT, sizeof(VXFORMAT_2D), &buffer[0].position);
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VXFORMAT_2D), &buffer[0].color);
			glTexCoordPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), &buffer[0].texcoords);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		glBlendFunc(GL_ONE, GL_ONE);
	}

	void renderCones(Storm3D_Scene &scene, bool renderShadows, float timeFactor, bool renderGlows)
	{
		// this draws spotlight cone
		frozenbyte::storm::PixelShader::disable();

		Storm3D_ShaderManager::GetSingleton()->setProjectedShaders();
		coneNvVertexShader.apply();

		Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());

		glDepthMask(GL_FALSE);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		StormSpotList::iterator it = spots.begin();
		for(; it != spots.end(); ++it)
		{
			Storm3D_Spotlight *spot = it->get();
			if(!spot || !spot->enabled() || !spot->featureEnabled(IStorm3D_Spotlight::ConeVisualization))
				continue;

			if(renderShadows)
				spot->renderCone(camera, timeFactor, renderGlows);
		}

		Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
		frozenbyte::storm::PixelShader::disable();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
};

//! Constructor
Storm3D_TerrainLightManager::Storm3D_TerrainLightManager(Storm3D &storm, IStorm3D_TerrainRendererBase &renderer, StormSpotList &spots, FakeSpotList &fakeSpots, FakeLightList &fakeLights)
{
	scoped_ptr<Data> tempData(new Data(storm, renderer, spots, fakeSpots, fakeLights));
	data.swap(tempData);
}

//! Destructor
Storm3D_TerrainLightManager::~Storm3D_TerrainLightManager()
{
}

//! Set fog parameters
/*!
	\param start fog start distance
	\param end fog end distance
*/
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

//! Render projected render targets
/*!
	\param scene scene
	\param renderShadows true to render shadows
	\param renderFakeBuffers true to render fake buffers
*/
void Storm3D_TerrainLightManager::renderProjectedRenderTargets(Storm3D_Scene &scene, bool renderShadows, bool renderFakeBuffers)
{
	data->renderBuffers(scene, renderShadows, renderFakeBuffers);
}

//! Render projected fake lights
/*!
	\param scene scene
	\param renderShadows true to render shadows
*/
void Storm3D_TerrainLightManager::renderProjectedFakeLights(Storm3D_Scene &scene, bool renderShadows)
{
	data->renderLights(Data::Fake, scene, renderShadows);
}

//! Render projected lights as solid
/*!
	\param scene scene
	\param renderShadows true to render shadows
*/
void Storm3D_TerrainLightManager::renderProjectedLightsSolid(Storm3D_Scene &scene, bool renderShadows)
{
	data->renderLights(Data::RealSolid, scene, renderShadows);
}

//! Render projected lights with alpha
/*!
	\param scene scene
	\param renderShadows true to render shadows
*/
void Storm3D_TerrainLightManager::renderProjectedLightsAlpha(Storm3D_Scene &scene, bool renderShadows)
{
	data->renderLights(Data::RealAlpha, scene, renderShadows);
}

//! Render fake lights
/*!
	\param renderSize size of render texture
*/
void Storm3D_TerrainLightManager::renderFakeLights(const VC2I &renderSize)
{
	data->renderFakeLights(renderSize);
}

//! Render cones
/*!
	\param scene scene
	\param renderShadows true to render shadows
	\param renderGlows true to render glows
*/
void Storm3D_TerrainLightManager::renderCones(Storm3D_Scene &scene, bool renderShadows, bool renderGlows)
{
	float timeFactor = float(scene.time_dif) * 0.001f;
	data->renderCones(scene, renderShadows, timeFactor, renderGlows);
}

void Storm3D_TerrainLightManager::setDebug(void) {
	static unsigned int count = 0;
	// set first spotlight depth texture as stage 0 texture
	if (data->spots.size() != 0) {
		unsigned int i = (count / 10) % data->spots.size();
		//igiosWarning("Storm3D_TerrainLightManager::setDebug: spotlight %d\n", i);
		Storm3D_Spotlight *spot = data->spots[i].get();
		spot->setDebug();
	} else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	++count;
}
