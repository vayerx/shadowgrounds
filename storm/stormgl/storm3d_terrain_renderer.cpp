// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <queue>
#include <vector>

#include "storm3d_terrain_renderer.h"
#include "storm3d_terrain_heightmap.h"
#include "storm3d_terrain_groups.h"
#include "storm3d_terrain_models.h"
#include "storm3d_terrain_lightmanager.h"
#include "storm3d_terrain_decalsystem.h"
#include "storm3d_spotlight.h"
#include "storm3d_particle.h"
#include "storm3d_fakespotlight.h"
#include "storm3d_scene.h"
#include "storm3d_texture.h"
#include "storm3d.h"
#include "storm3d_terrain_utils.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_model.h"
#include <IStorm3D_Logger.h>
#include "VertexFormats.h"
#include "igios3D.h"

#include "../../util/Debug_MemoryManager.h"

using namespace std;
using namespace boost;

namespace {
	static const int MAX_SIZES = 2;
};

extern int active_visibility;

struct Storm3D_TerrainRendererData
{
	Storm3D &storm;
	Storm3D_TerrainHeightmap &heightMap;
	Storm3D_TerrainGroup &groups;
	Storm3D_TerrainModels &models;
	Storm3D_TerrainDecalSystem &decalSystem;

	StormSpotList spots;
	FakeSpotList fakeSpots;
	FakeLightList fakeLights;
	Storm3D_TerrainLightManager lightManager;

	COL ambient;
	COL clearColor;
	COL lightmapMultiplier;
	COL outdoorLightmapMultiplier;
	Storm3D_Model *skyBox;

	static boost::shared_ptr<glTexWrapper> renderTexture;
	static boost::shared_ptr<glTexWrapper> terrainTexture;
	static boost::shared_ptr<glTexWrapper> fakeTexture;
	static boost::shared_ptr<glTexWrapper> glowTexture1;
	static boost::shared_ptr<glTexWrapper> glowTexture2;
	static boost::shared_ptr<glTexWrapper> offsetTexture;

	static VC2I renderSize;
	static VC2I glowSize;

	boost::shared_ptr<glTexWrapper> depthLookupTexture;
	boost::shared_ptr<glTexWrapper> spotFadeTexture;
	boost::shared_ptr<glTexWrapper> fakeFadeTexture;
	boost::shared_ptr<glTexWrapper> fake2DFadeTexture;
	boost::shared_ptr<glTexWrapper> coneFadeTexture;
	boost::shared_ptr<glTexWrapper> noFadeTexture;

	frozenbyte::storm::PixelShader skyboxPixelShader;
	frozenbyte::storm::PixelShader glowShader;
	frozenbyte::storm::PixelShader lightShader;
	frozenbyte::storm::PixelShader colorEffectShader;
	frozenbyte::storm::PixelShader colorEffectOffsetShader;
	frozenbyte::storm::PixelShader colorEffectOffsetShader_NoGamma;
	frozenbyte::storm::PixelShader blackWhiteShader;

	IStorm3D_TerrainRenderer::RenderMode renderMode;
	bool renderRenderTargets;
	bool renderGlows;
	bool renderOffsets;
	bool renderCones;
	bool renderFakeLights;
	bool renderFakeShadows;
	bool renderSpotShadows;
	bool renderWireframe;
	bool renderBlackWhite;
	bool renderCollision;
	bool renderModels;
	bool renderTerrainObjects;
	bool renderHeightmap;

	bool renderSkyBox;
	bool renderDecals;
	bool renderSpotDebug;
	bool renderFakeShadowDebug;
	bool renderGlowDebug;
	bool renderBoned;
	bool renderTerrainTextures;
	bool renderParticles;
	bool renderParticleReflection;
	bool skyModelGlowAllowed;
	bool additionalAlphaTestPassAllowed;

	bool freezeCameraCulling;
	bool freezeSpotCulling;
	bool movieAspect;
	bool multipassGlow;
	bool smoothShadows;
	bool proceduralFallback;
	bool reflection;
	bool halfRendering;

	D3DXMATRIX terrainTextureProjectionSizes[MAX_SIZES];
	D3DXMATRIX terrainTextureProjection;
	D3DXMATRIX fakeTextureProjection;

	float contrast;
	float brightness;
	COL colorFactors;
	bool colorEffectOn;

	VC2I backbufferSize;
	SDL_Rect scissorRectSizes[MAX_SIZES];
	SDL_Rect scissorRect;	// NOTE: x, y are LOWER left corner of rectangle!
	int movieAspectPad;
	int movieAspectStartTime;

	SDL_Rect viewportSizes[MAX_SIZES];
	SDL_Rect viewport;		// NOTE: x, y are LOWER left corner of rectangle!

	bool fogEnable;
	float fogStart;
	float fogEnd;
	COL fogColor;
	float glowFactor;
	float glowAdditiveFactor;
	float glowTransparencyFactor;

	bool hasStretchFiltering;
	bool forceDraw;

	Storm3D_ParticleSystem *particleSystem;
	boost::shared_ptr<Storm3D_Texture> offsetFade;

	int activeSize;

	bool forcedDirectionalLightEnabled;
	COL forcedDirectionalLightColor;
	VC3 forcedDirectionalLightDirection;

	// TODO: have several fbos for different textures
	Framebuffer *fbo;

	Storm3D_TerrainRendererData(Storm3D &storm_, IStorm3D_TerrainRendererBase &rendererBase, Storm3D_TerrainHeightmap &heightMap_, Storm3D_TerrainGroup &groups_, Storm3D_TerrainModels &models_, Storm3D_TerrainDecalSystem &decalSystem_)
	:	storm(storm_),
		heightMap(heightMap_),
		groups(groups_),
		models(models_),
		decalSystem(decalSystem_),
		lightManager(storm, rendererBase, spots, fakeSpots, fakeLights),
		lightmapMultiplier(1.f, 1.f, 1.f),
		outdoorLightmapMultiplier(1.f, 1.f, 1.f),
		skyBox(0),
		glowShader(),
		lightShader(),
		colorEffectShader(),
		colorEffectOffsetShader(),
		colorEffectOffsetShader_NoGamma(),
		blackWhiteShader(),

		renderMode(IStorm3D_TerrainRenderer::Normal),
		renderRenderTargets(true),
		renderGlows(false),
		renderOffsets(false),
		renderCones(true),
		renderFakeLights(true),
		renderFakeShadows(true),
		renderSpotShadows(true),
		renderWireframe(false),
		renderBlackWhite(false),
		renderCollision(false),
		renderModels(true),
		renderTerrainObjects(true),
		renderHeightmap(true),

		renderSkyBox(true),
		renderDecals(true),
		renderSpotDebug(false),
		renderFakeShadowDebug(false),
		renderGlowDebug(false),
		renderBoned(true),
		renderTerrainTextures(true),
		renderParticles(true),
		renderParticleReflection(false),
		skyModelGlowAllowed(true),
		additionalAlphaTestPassAllowed(false),

		freezeCameraCulling(false),
		freezeSpotCulling(false),
		movieAspect(false),
		multipassGlow(false),
		smoothShadows(false),
		proceduralFallback(false),
		reflection(true),
		halfRendering(false),

		contrast(0.f),
		brightness(0.f),
		colorFactors(0.f, 0.f, 0.f),
		colorEffectOn(false),

		fogEnable(false),
		fogStart(50.f),
		fogEnd(-50.f),
		glowFactor(0.f),
		glowAdditiveFactor(1.0f),
		glowTransparencyFactor(0.0f),
		hasStretchFiltering(false),
		forceDraw(false),

		particleSystem(0),
		activeSize(0),

		forcedDirectionalLightEnabled(false)
	{
		fbo = new Framebuffer();
		skyboxPixelShader.createSkyboxShader();
		glowShader.createGlowShader();
		lightShader.createLightShader();
		colorEffectShader.createColorEffectPixelShader();
		blackWhiteShader.createBlackWhiteShader();

		colorEffectOffsetShader.createColorEffectOffsetPixelShader();
		colorEffectOffsetShader_NoGamma.createColorEffectOffsetPixelShader_NoGamma();

		DWORD *buffer = new DWORD[2048];
		for(int i = 0; i < 2048; ++i)
			buffer[i] = COLOR_RGBA(i & 0xFF, (i & 0xFF00) >> 3, 0, 0);

		depthLookupTexture = glTexWrapper::rgbaTexture(2048, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, depthLookupTexture->getFmt(), 2048, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		delete[] buffer;

		// Spot fade texture
		{
			DWORD *buffer = new DWORD[128];
			for(int i = 0; i < 128; ++i)
			{
				int value = 255;
				if(i > 96)
					value -= ((i - 96) * 8);
				if(i < 2)
					value = 0;

				unsigned char c = 0;
				if(value > 0 && value <= 255)
					c = value;

				buffer[i] = COLOR_RGBA(c, c, c, c);
			}

			spotFadeTexture = glTexWrapper::rgbaTexture(128, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, spotFadeTexture->getFmt(), 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[] buffer;
		}

		// Fake spot fade texture
		{
			DWORD *buffer = new DWORD[128];
			for(int i = 0; i < 128; ++i)
			{
				int value = 0;
				value = i * 2;

				unsigned char c = 0;
				if(value >= 0 && value <= 255)
					c = value;

				buffer[i] = COLOR_RGBA(c, c, c, 0);
			}

			fakeFadeTexture = glTexWrapper::rgbaTexture(128, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, fakeFadeTexture->getFmt(), 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[] buffer;
		}

		// Fake 2D texture
		{
			DWORD *buffer = new DWORD[4096];
			int pitch = 64;

			float max = sqrtf(31*31 + 31*31);

			for(int y = 0; y < 64; ++y)
				for(int x = 0; x < 64; ++x)
				{
					float yd = float(y - 32);
					float xd = float(x - 32);
					float result = sqrtf(xd*xd + yd*yd) / max;

					if(result < 0.5f)
						result = 0.f;
					else
						result = (result - 0.5f) * 2.f;

					if(result > 1.f)
						result = 1.f;

					unsigned char c = (unsigned char)(result * 255);
					buffer[y * pitch + x] = COLOR_RGBA(c, c, c, 0);
				}

			fake2DFadeTexture = glTexWrapper::rgbaTexture(64, 64);
			glTexImage2D(GL_TEXTURE_2D, 0, fake2DFadeTexture->getFmt(), 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[] buffer;
		}

		// No-fade texture
		{
			DWORD *buffer = new DWORD[128];
			for(int i = 0; i < 128; ++i)
			{
				unsigned char c = 255;
				buffer[i] = COLOR_RGBA(c, c, c, 0);
			}

			noFadeTexture = glTexWrapper::rgbaTexture(128, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, noFadeTexture->getFmt(), 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[] buffer;
		}

		// Cone fade
		{
			DWORD *buffer = new DWORD[128];
			for(int i = 0; i < 128; ++i)
			{
				int value = 255 - (i * 6);
				
				unsigned char c = 0;
				if(value > 0)
					c = value;

				buffer[i] = COLOR_RGBA(c, c, c, c);
			}

			coneFadeTexture = glTexWrapper::rgbaTexture(128, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, coneFadeTexture->getFmt(), 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[] buffer;
		}

		if(glowTexture1 && glowTexture2)
			renderGlows = true;

		float xf = float(renderSize.x) / fakeTexture->getWidth() * .5f;
		float yf = float(renderSize.y) / fakeTexture->getHeight() * .5f;
		fakeTextureProjection = D3DXMATRIX (xf,		0.0f,	0.0f,	xf,
											0.0f,	yf,	0.0f,	yf,
											0.0f,	0.0f,	0.0f,	0.0f,
											0.0f,   0.0f,	0.0f,	1.0f);

		if(terrainTexture)
		{
			{
				float xs = float(renderSize.x) / (terrainTexture->getWidth()) * .5f;
				float ys = float(renderSize.y) / (terrainTexture->getHeight()) * .5f;
				float xd = 1.f / float(terrainTexture->getWidth()) * .5f;
				float yd = 1.f / float(terrainTexture->getHeight()) * .5f;

				terrainTextureProjectionSizes[0] = D3DXMATRIX (	 xs,	0.0f,	0.0f,	xs + xd,
																0.0f,    ys,	0.0f,	ys + yd,
																0.0f,    0.0f,	0.0f,	0.0f,
																0.0f,    0.0f,	0.0f,	1.0f);
			}
			{
				float xs = float(renderSize.x) / (terrainTexture->getWidth()) * .25f;
				float ys = float(renderSize.y) / (terrainTexture->getHeight()) * .25f;
				float xd = 1.f / float(terrainTexture->getWidth()) * .25f;
				float yd = 1.f / float(terrainTexture->getHeight()) * .25f;

				terrainTextureProjectionSizes[1] = D3DXMATRIX (	 xs,	0.0f,	0.0f,	xs + xd,
																0.0f,	ys,	0.0f,	ys + yd,
																0.0f,    0.0f,	0.0f,	0.0f,
																0.0f,    0.0f,	0.0f,	1.0f);
			}

			terrainTextureProjection = terrainTextureProjectionSizes[activeSize];
		}

		{
			Storm3D_SurfaceInfo info = storm.GetScreenSize();
			backbufferSize.x = info.width;
			backbufferSize.y = info.height;

			viewportSizes[0].x = 0;
			viewportSizes[0].y = 0;
			viewportSizes[0].w = backbufferSize.x;
			viewportSizes[0].h = backbufferSize.y;
			viewportSizes[1] = viewportSizes[0];
			viewportSizes[1].w /= 2;
			viewportSizes[1].h /= 2;
			viewport = viewportSizes[activeSize];

			movieAspect = false;
			scissorRectSizes[0].x = 0;
			scissorRectSizes[0].y = 0;
			scissorRectSizes[0].w = backbufferSize.x;
			scissorRectSizes[0].h = backbufferSize.y;
			scissorRectSizes[1].x = scissorRectSizes[0].x / 2;
			scissorRectSizes[1].y = scissorRectSizes[0].y / 2;
			scissorRectSizes[1].w = scissorRectSizes[0].w / 2;
			scissorRectSizes[1].h = scissorRectSizes[0].h / 2;
			scissorRect = scissorRectSizes[activeSize];
			setMovieAspectRatio(false, false);
		}

		offsetFade.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture("distortion_fade.dds")));
		if(!offsetFade && storm.getLogger())
			storm.getLogger()->error("Missing distortion mask texture. Distortion will not work properly.");

		igios_unimplemented();
		//hasStretchFiltering = storm.adapters[storm.active_adapter].stretchFilter;
	}

	~Storm3D_TerrainRendererData()
	{
		if (fbo != NULL) {
			delete fbo; fbo = NULL;
		}
	}

	void updateMovieAspectRatio(void)
	{
		if(movieAspectPad == scissorRectSizes[0].y)
		{
			// borders faded out
			if(movieAspectPad == 0)
				movieAspect = false;

			return;
		}

		// interpolate towards target
		int time = SDL_GetTicks() - movieAspectStartTime;
		int target = movieAspectPad * (time) / 500;
		int current = (7*scissorRectSizes[0].y + target) / 8;

		// clamping (note: needs to handle both larger and smaller targets)
		if((current < target && current >= movieAspectPad) || (current > target && current < movieAspectPad))
		{
			current = movieAspectPad;
		}

		scissorRectSizes[0].y = current;
		scissorRectSizes[0].h = backbufferSize.y;
		scissorRectSizes[1].y = scissorRectSizes[0].y / 2;
		scissorRectSizes[1].h = scissorRectSizes[0].h / 2;

		scissorRect.y = scissorRectSizes[activeSize].y;
		scissorRect.h = scissorRectSizes[activeSize].h;
	}

	void setMovieAspectRatio(bool enable, bool fade)
	{
		if(enable)
		{
			if(!fade)
			{
				// claw: borked 'real' aspect ratio
				int height = backbufferSize.x / 2;
				int padd = (backbufferSize.y - height) / 2;
				movieAspectPad = padd;
			}
			else
			{
				// survivor: nice looking aspect ratio
				int padd = backbufferSize.y / 6;
				movieAspectPad = padd;
			}
		}
		else
		{
			movieAspectPad = 0;
		}

		if(fade)
		{
			movieAspect = true;
			movieAspectStartTime = SDL_GetTicks();
			updateMovieAspectRatio();
		}
		else
		{
			movieAspect = enable;

			scissorRectSizes[0].x = 0;
			scissorRectSizes[0].y = movieAspectPad;
			scissorRectSizes[0].w = backbufferSize.x;
			scissorRectSizes[0].h = backbufferSize.y - movieAspectPad;

			scissorRectSizes[1].x = scissorRectSizes[0].x / 2;
			scissorRectSizes[1].y = scissorRectSizes[0].y / 2;
			scissorRectSizes[1].w = scissorRectSizes[0].w / 2;
			scissorRectSizes[1].h = scissorRectSizes[0].h / 2;

			scissorRect = scissorRectSizes[activeSize];
		}
	}

	void renderConeLights(Storm3D_Scene &scene, bool glowsEnabled)
	{
		glActiveTexture(GL_TEXTURE2);
		glClientActiveTexture(GL_TEXTURE2);
		coneFadeTexture->bind();

		lightManager.renderCones(scene, renderSpotShadows, glowsEnabled);
	}

	void renderLightTargets(Storm3D_Scene &scene)
	{
		lightManager.renderProjectedRenderTargets(scene, renderSpotShadows, renderFakeShadows);
	}

	enum Pass
	{
		Solid,
		Alpha
	};

	void renderPass(Storm3D_Scene &scene, Pass pass)
	{
		Storm3D_ShaderManager::GetSingleton()->setLightingShaders();

		D3DXMATRIX dm;
		D3DXMatrixIdentity(dm);

		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);
		Storm3D_ShaderManager::GetSingleton()->SetAmbient(ambient);
		Storm3D_ShaderManager::GetSingleton()->SetModelAmbient(COL(0,0,0));
		Storm3D_ShaderManager::GetSingleton()->SetObjectAmbient(COL(0,0,0));
		Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(COL(1.f, 1.f, 1.f));
		Storm3D_ShaderManager::GetSingleton()->SetShaderAmbient(ambient);

		// update only in first pass
		if(movieAspect && pass == Solid)
		{
			updateMovieAspectRatio();
		}
		glScissor(scissorRect.x, scissorRect.y, scissorRect.w, scissorRect.h);
		glEnable(GL_SCISSOR_TEST);

		if(pass == Solid)
		{

			if(renderHeightmap)
			{
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);

				// fakeTexture contains lights
				glActiveTexture(GL_TEXTURE2);
				glClientActiveTexture(GL_TEXTURE2);
				fakeTexture->bind();
				// terrainTexture contains terrain textures
				glActiveTexture(GL_TEXTURE3);
				glClientActiveTexture(GL_TEXTURE3);
				terrainTexture->bind();

				const COL  &c = outdoorLightmapMultiplier;
				float mult[4] = { c.r, c.g, c.b, 1.f };
				glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, mult);

				D3DXMATRIX tm;
				D3DXMatrixIdentity(tm);
				Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm);

				setTextureMatrix(2, fakeTextureProjection);
				setTextureMatrix(3, terrainTextureProjection);

				glActiveTexture(GL_TEXTURE2);
				glClientActiveTexture(GL_TEXTURE2);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glActiveTexture(GL_TEXTURE3);
				glClientActiveTexture(GL_TEXTURE3);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				if(forcedDirectionalLightEnabled)
				{
					VC3 sunDir = forcedDirectionalLightDirection;
					float sunStrength = (forcedDirectionalLightColor.r + forcedDirectionalLightColor.g + forcedDirectionalLightColor.b) / 3.f;
					sunDir *= sunStrength;

					Storm3D_ShaderManager::GetSingleton()->SetSun(sunDir, sunStrength);
				}
				else
					Storm3D_ShaderManager::GetSingleton()->SetSun(VC3(), 0.f);

				heightMap.renderDepth(scene, 0, Storm3D_TerrainHeightmap::Lighting, IStorm3D_Spotlight::None, 0);

				Storm3D_ShaderManager::GetSingleton()->SetSun(VC3(), 0.f);

				glActiveTexture(GL_TEXTURE3);
				glClientActiveTexture(GL_TEXTURE3);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glDisable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			if(renderModels)
			{
				setTextureMatrix(2, fakeTextureProjection);

				glActiveTexture(GL_TEXTURE2);
				glClientActiveTexture(GL_TEXTURE2);
				fakeTexture->bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				Storm3D_ShaderManager::GetSingleton()->SetAmbient(ambient);
				Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(COL(1.f, 1.f, 1.f));

				const COL  &c = lightmapMultiplier;
				float mult[4] = { c.r, c.g, c.b, 1.f };
				glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, mult);

				models.renderLighting(Storm3D_TerrainModels::SolidOnly, scene);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

		}
		else if(pass == Alpha)
		{
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			fakeTexture->bind();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			setTextureMatrix(2, fakeTextureProjection);

			if(renderDecals)
			{
				glEnable(GL_FOG);
				decalSystem.renderTextures(scene);
				glDisable(GL_FOG);
			}

			Storm3D_ShaderManager::GetSingleton()->SetAmbient(ambient);
			Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(COL(1.f, 1.f, 1.f));

			if(renderModels)
			{
				glActiveTexture(GL_TEXTURE2);
				glClientActiveTexture(GL_TEXTURE2);
				fakeTexture->bind();

				if(renderWireframe)
					models.setFillMode(Storm3D_TerrainModels::Wireframe);
				else
					models.setFillMode(Storm3D_TerrainModels::Solid);

				const COL  &c = lightmapMultiplier;
				float mult[4] = { c.r, c.g, c.b, 1.f };
				glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, mult);

				setTextureMatrix(2, fakeTextureProjection);

				glActiveTexture(GL_TEXTURE2);
				glClientActiveTexture(GL_TEXTURE2);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				models.renderLighting(Storm3D_TerrainModels::AlphaOnly, scene);
				glDisable(GL_FOG);
			}
		}
	}

	void renderFakelights(Storm3D_Scene &scene)
	{
		if(renderFakeShadows)
			lightManager.renderProjectedFakeLights(scene, renderSpotShadows);
	}

	void renderProjectedLightsSolid(Storm3D_Scene &scene)
	{
		lightManager.renderProjectedLightsSolid(scene, renderSpotShadows);
	}

	void renderProjectedLightsAlpha(Storm3D_Scene &scene)
	{
		lightManager.renderProjectedLightsAlpha(scene, renderSpotShadows);
	}

	static void querySizes(Storm3D &storm, bool enableGlow)
	{
		Storm3D_SurfaceInfo screen = storm.GetScreenSize();
		VC2I screenSize(screen.width, screen.height);

		storm.setNeededDepthTarget(screenSize);
		storm.setNeededSecondaryColorTarget(screenSize);
		if (enableGlow) storm.setNeededColorTarget(screenSize);
	}

	static void createTextures(Storm3D &storm, int lightmapQuality)
	{
		Storm3D_SurfaceInfo screen = storm.GetScreenSize();

		renderSize.x = screen.width;
		renderSize.y = screen.height;
		glowSize = renderSize;

		int tempx = renderSize.x, tempy = renderSize.y;
		toNearestPow(tempx); toNearestPow(tempy);
		renderTexture = glTexWrapper::rgbaTexture(tempx, tempy);

		terrainTexture = storm.getColorSecondaryTarget();

		fakeTexture = glTexWrapper::rgbaTexture(tempx, tempy);

		if(!renderTexture || !terrainTexture || !fakeTexture)
		{
			IStorm3D_Logger *logger = storm.getLogger();
			if(logger)
				logger->error("Failed creating base rendertargets - GAME WILL NOT RUN CORRECTLY!");

			return;
		}
	}

	static void createSecondaryTextures(Storm3D &storm, bool enableGlow)
	{
		if(enableGlow)
		{
			glowTexture1 = storm.getColorTarget();
			glowTexture2 = storm.getColorSecondaryTarget();

			if(!glowTexture1 || !glowTexture2)
			{
				if (glowTexture1) {
					glowTexture1.reset();
				}

				if (glowTexture2) {
					glowTexture2.reset();
				}

				IStorm3D_Logger *logger = storm.getLogger();
				if(logger)
					logger->warning("Failed creating glow rendertargets - glow disabled!");
			}
		}

		Storm3D_SurfaceInfo screen = storm.GetScreenSize();

		int tempx = screen.width, tempy = screen.height;
		toNearestPow(tempx); toNearestPow(tempy);
		offsetTexture = glTexWrapper::rgbaTexture(tempx, tempy);
	}

	static void freeTextures()
	{
		if (renderTexture) {
			renderTexture.reset();
		}

		if (terrainTexture) {
			terrainTexture.reset();
		}

		if (fakeTexture) {
			fakeTexture.reset();
		}
	}

	static void freeSecondaryTextures()
	{
		if (glowTexture1) {
			glowTexture1.reset();
		}

		if (glowTexture2) {
			glowTexture2.reset();
		}

		if (offsetTexture) {
			offsetTexture.reset();
		}
	}
};

boost::shared_ptr<glTexWrapper> Storm3D_TerrainRendererData::renderTexture;
boost::shared_ptr<glTexWrapper> Storm3D_TerrainRendererData::terrainTexture;
boost::shared_ptr<glTexWrapper> Storm3D_TerrainRendererData::fakeTexture;
boost::shared_ptr<glTexWrapper> Storm3D_TerrainRendererData::glowTexture1;
boost::shared_ptr<glTexWrapper> Storm3D_TerrainRendererData::glowTexture2;
boost::shared_ptr<glTexWrapper> Storm3D_TerrainRendererData::offsetTexture;

VC2I Storm3D_TerrainRendererData::renderSize;
VC2I Storm3D_TerrainRendererData::glowSize;

//! Constructor
Storm3D_TerrainRenderer::Storm3D_TerrainRenderer(Storm3D &storm, Storm3D_TerrainHeightmap &heightMap, Storm3D_TerrainGroup &groups, Storm3D_TerrainModels &models, Storm3D_TerrainDecalSystem &decalSystem)
{
	boost::scoped_ptr<Storm3D_TerrainRendererData> tempData(new Storm3D_TerrainRendererData(storm, *this, heightMap, groups, models, decalSystem));
	data.swap(tempData);

	setFog(false, 150.f, -50.f, COL(1.f, 0.5f, 0.5f));
}

//! Destructor
Storm3D_TerrainRenderer::~Storm3D_TerrainRenderer()
{
}

//! Create spotlight
/*!
	\return new spotlight
*/
boost::shared_ptr<IStorm3D_Spotlight> Storm3D_TerrainRenderer::createSpot()
{
	boost::shared_ptr<Storm3D_Spotlight> spot(new Storm3D_Spotlight(data->storm));
	spot->enableSmoothing(data->smoothShadows);
	data->spots.push_back(spot);

	return boost::static_pointer_cast<IStorm3D_Spotlight> (spot);
}

//! Delete spotlight
/*!
	\param spot spotlight to delete
*/
void Storm3D_TerrainRenderer::deleteSpot(boost::shared_ptr<IStorm3D_Spotlight> &spot)
{
	for(unsigned int i = 0; i < data->spots.size(); ++i)
	{
		if(data->spots[i].get() == spot.get())
		{
			data->spots.erase(data->spots.begin() + i);
			return;
		}
	}
}

//! Create fake spotlight
/*!
	\return new fake spotlight
*/
boost::shared_ptr<IStorm3D_FakeSpotlight> Storm3D_TerrainRenderer::createFakeSpot()
{
	boost::shared_ptr<Storm3D_FakeSpotlight> spot(new Storm3D_FakeSpotlight(data->storm));
	data->fakeSpots.push_back(spot);

	return boost::static_pointer_cast<IStorm3D_FakeSpotlight> (spot);
}

//! Delete fake spotlight
/*!
	\param spot fake spotlight to delete
*/
void Storm3D_TerrainRenderer::deleteFakeSpot(boost::shared_ptr<IStorm3D_FakeSpotlight> &spot)
{
	for(unsigned int i = 0; i < data->fakeSpots.size(); ++i)
	{
		if(data->fakeSpots[i].get() == spot.get())
		{
			data->fakeSpots.erase(data->fakeSpots.begin() + i);
			return;
		}
	}
}

//! Set aspect ratio to movie mode or normal
/*!
	\param enable true to set movie aspect ratio
*/
void Storm3D_TerrainRenderer::setMovieAspectRatio(bool enable, bool fade)
{
	data->setMovieAspectRatio(enable, fade);
}

//! Set particle system
/*!
	\param particlesystem particle system to use
*/
void Storm3D_TerrainRenderer::setParticleSystem(Storm3D_ParticleSystem *particlesystem)
{
	data->particleSystem = particlesystem;
}

//! Set render mode
/*!
	\param mode render mode
*/
void Storm3D_TerrainRenderer::setRenderMode(IStorm3D_TerrainRenderer::RenderMode mode)
{
	data->renderMode = mode;

	if(mode == LightOnly)
		data->models.forceWhiteBaseTexture(true);
	else
		data->models.forceWhiteBaseTexture(false);
}

//! Enable or disable feature
/*!
	\param feature feature to enable or disable
	\param enable true to enable feature
	\return previous status of feature
*/
bool Storm3D_TerrainRenderer::enableFeature(RenderFeature feature, bool enable)
{
	bool oldValue = false;

	if(feature == IStorm3D_TerrainRenderer::RenderTargets)
	{
		oldValue = data->renderRenderTargets;
		data->renderRenderTargets = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::Glow)
	{
		oldValue = data->renderGlows;
		if(data->glowTexture1 && data->glowTexture2)
			data->renderGlows = enable;
		else
			data->renderGlows = false;
	}
	else if(feature == IStorm3D_TerrainRenderer::BetterGlowSampling)
	{
		oldValue = data->multipassGlow;
		data->multipassGlow = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::Distortion)
	{
		oldValue = data->renderOffsets;
		if(data->offsetTexture)
			data->renderOffsets = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::SmoothShadows)
	{
		oldValue = data->smoothShadows;
		data->smoothShadows = enable;
		for(unsigned int i = 0; i < data->spots.size(); ++i)
			data->spots[i]->enableSmoothing(enable);
	}
	else if(feature == IStorm3D_TerrainRenderer::SpotShadows)
	{
		oldValue = data->renderSpotShadows;
		data->renderSpotShadows = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::FakeLights)
	{
		oldValue = data->renderFakeLights;
		data->renderFakeLights = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::Wireframe)
	{
		oldValue = data->renderWireframe;
		data->renderWireframe = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::BlackAndWhite)
	{
		oldValue = data->renderBlackWhite;
		data->renderBlackWhite = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::LightmapFiltering)
	{
		data->models.filterLightmap(enable);
	}
	else if(feature == IStorm3D_TerrainRenderer::Collision)
	{
		oldValue = data->renderCollision;
		data->renderCollision = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::ModelRendering)
	{
		oldValue = data->renderModels;
		data->renderModels = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::TerrainObjectRendering)
	{
		oldValue = data->renderTerrainObjects;
		data->renderTerrainObjects = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::HeightmapRendering)
	{
		oldValue = data->renderHeightmap;
		data->renderHeightmap = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::TerrainTextures)
	{
		oldValue = data->renderTerrainTextures;
		data->renderTerrainTextures = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::Particles)
	{
		oldValue = data->renderParticles;
		data->renderParticles = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::AlphaTest)
	{
		data->models.enableAlphaTest(enable);
	}
	else if(feature == IStorm3D_TerrainRenderer::MaterialAmbient)
	{
		data->models.enableMaterialAmbient(enable);
	}
	else if(feature == IStorm3D_TerrainRenderer::FreezeCameraCulling)
	{
		oldValue = data->freezeCameraCulling;
		data->freezeCameraCulling = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::FreezeSpotCulling)
	{
		oldValue = data->freezeSpotCulling;
		data->freezeSpotCulling = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::LightmappedCollision)
	{
		data->groups.enableLightmapCollision(enable);
	}
	else if(feature == IStorm3D_TerrainRenderer::RenderSkyModel)
	{
		oldValue = data->renderSkyBox;
		data->renderSkyBox = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::RenderDecals)
	{
		oldValue = data->renderDecals;
		data->renderDecals = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::RenderSpotDebug)
	{
		oldValue = data->renderSpotDebug;
		data->renderSpotDebug = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::RenderFakeShadowDebug)
	{
		oldValue = data->renderFakeShadowDebug;
		data->renderFakeShadowDebug = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::RenderGlowDebug)
	{
		oldValue = data->renderGlowDebug;
		data->renderGlowDebug = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::ProceduralFallback)
	{
		oldValue = data->proceduralFallback;
		data->proceduralFallback = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::Reflection)
	{
		oldValue = data->reflection;
		data->reflection = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::HalfRendering)
	{
		oldValue = data->halfRendering;
		if(enable)
		{
			data->halfRendering = true;
			data->activeSize = 1;
		}
		else
		{
			data->halfRendering = false;
			data->activeSize = 0;
		}

		data->terrainTextureProjection = data->terrainTextureProjectionSizes[data->activeSize];
		data->scissorRect = data->scissorRectSizes[data->activeSize];
		data->viewport = data->viewportSizes[data->activeSize];
	}
	else if(feature == IStorm3D_TerrainRenderer::RenderBoned)
	{
		oldValue = data->renderBoned;
		data->models.renderBoned(enable);
		data->renderBoned = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::FakeShadows)
	{
		oldValue = data->renderFakeShadows;
		data->renderFakeShadows = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::ParticleReflection)
	{
		oldValue = data->renderParticleReflection;
		data->renderParticleReflection = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::ForcedDirectionalLightEnabled)
	{
		oldValue = data->forcedDirectionalLightEnabled;
		data->forcedDirectionalLightEnabled = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::AdditionalAlphaTestPass)
	{
		oldValue = data->additionalAlphaTestPassAllowed;
		data->additionalAlphaTestPassAllowed = enable;
	}
	else if(feature == IStorm3D_TerrainRenderer::SkyModelGlow)
	{
		oldValue = data->skyModelGlowAllowed;
		data->skyModelGlowAllowed = enable;
	}

	return oldValue;
}

//! Set float value
/*!
	\param type float value to set
	\param value new value
*/
void Storm3D_TerrainRenderer::setFloatValue(FloatValue type, float value)
{
	if(type == LightmapMultiplier)
	{
		data->lightmapMultiplier = COL(value, value, value);
		data->decalSystem.setLightmapFactor(data->lightmapMultiplier);
	}
	else if(type == OutdoorLightmapMultiplier)
	{
		data->outdoorLightmapMultiplier = COL(value, value, value);
		data->decalSystem.setOutdoorLightmapFactor(data->outdoorLightmapMultiplier);
	}
	else if(type == ForceAmbient)
	{
		COL c(value, value, value);
		Storm3D_ShaderManager::GetSingleton()->SetForceAmbient(c);
	}
	else if(type == GlowFactor)
		data->glowFactor = value;
	else if(type == GlowTransparencyFactor)
		data->glowTransparencyFactor = value;
	else if(type == GlowAdditiveFactor)
		data->glowAdditiveFactor = value;
}

//! Set color value
/*!
	\param type color value to set
	\param value new value
*/
void Storm3D_TerrainRenderer::setColorValue(ColorValue type, const COL &value)
{
	if(type == LightmapMultiplierColor)
	{
		data->lightmapMultiplier = value;
		data->decalSystem.setLightmapFactor(data->lightmapMultiplier);
	}
	else if(type == OutdoorLightmapMultiplierColor)
	{
		data->outdoorLightmapMultiplier = value;
		data->decalSystem.setOutdoorLightmapFactor(data->outdoorLightmapMultiplier);
	}
	else if(type == TerrainObjectOutsideFactor)
	{
		data->models.setTerrainObjectColorFactorOutside(value);
	}
	else if(type == TerrainObjectInsideFactor)
	{
		data->models.setTerrainObjectColorFactorBuilding(value);
	}
	else if(type == ForcedDirectionalLightColor)
		data->forcedDirectionalLightColor = value;
}

//! Get color value
/*!
	\param type color value to get
	\return value
*/
COL Storm3D_TerrainRenderer::getColorValue(ColorValue type) const
{
	if(type == LightmapMultiplierColor)
		return data->lightmapMultiplier;
	else if(type == ForcedDirectionalLightColor)
		return data->forcedDirectionalLightColor;

	assert(!"Invalid color type");
	return COL();
}

void Storm3D_TerrainRenderer::setVectorValue(VectorValue type, const VC3 &value)
{
	if(type == ForcedDirectionalLightDirection)
		data->forcedDirectionalLightDirection = value;
}

VC3 Storm3D_TerrainRenderer::getVectorValue(VectorValue type) const
{
	if(type == ForcedDirectionalLightDirection)
		return data->forcedDirectionalLightDirection;

	assert(!"Invalid vector type");
	return VC3();
}

//! Set color effect values
/*!
	\param contrast contrast
	\param brightness brightness
	\param colorFactors color factors
*/
void Storm3D_TerrainRenderer::setColorEffect(float contrast, float brightness, const COL &colorFactors)
{
	data->contrast = contrast;
	data->brightness = brightness;
	data->colorFactors = colorFactors;

	if(fabsf(contrast) > 0.01f)
		data->colorEffectOn = true;
	else if(fabsf(brightness) > 0.01f)
		data->colorEffectOn = true;
	else if(fabsf(colorFactors.r) > 0.01f)
		data->colorEffectOn = true;
	else if(fabsf(colorFactors.g) > 0.01f)
		data->colorEffectOn = true;
	else if(fabsf(colorFactors.b) > 0.01f)
		data->colorEffectOn = true;
	else
		data->colorEffectOn = false;
}

//! Render light texture
/*!
	\param start
	\param end
	\param texture
	\param color
*/
void Storm3D_TerrainRenderer::renderLightTexture(const VC2 &start, const VC2 &end, IStorm3D_Texture &texture, const COL &color)
{
	data->fakeLights.push_back(Storm3D_LightTexture(start, end, texture, color));
}

//! Set ambient color
/*!
	\param color color
*/
void Storm3D_TerrainRenderer::setAmbient(const COL &color)
{
	data->ambient = color;
}

//! Set clear color
/*!
	\param color color
*/
void Storm3D_TerrainRenderer::setClearColor(const COL &color)
{
	data->clearColor = color;
}

//! Set skybox
/*!
	\param model skybox model
*/
void Storm3D_TerrainRenderer::setSkyBox(Storm3D_Model *model)
{
	data->skyBox = model;
}

//! Set fog parameters
/*!
	\param enable true to enable fog
	\param startHeight fog start height
	\param endHeight fog end height
	\param color fog color
*/
void Storm3D_TerrainRenderer::setFog(bool enable, float startHeight, float endHeight, const COL &color)
{
	data->fogEnable = enable;
	data->fogStart = startHeight;
	data->fogEnd = endHeight;
	data->fogColor = color;

	if(!data->fogEnable)
	{
		data->fogStart = -100000.f;
		data->fogEnd = -1000000.f;
	}

	Storm3D_ShaderManager::GetSingleton()->SetFog(data->fogStart, data->fogStart - data->fogEnd, data->fogColor);
	data->decalSystem.setFog(data->fogEnd, data->fogStart - data->fogEnd);
}

//! Update object visibility data
/*!
	\param scene scene
	\param timeDelta time change
*/
void Storm3D_TerrainRenderer::updateVisibility(Storm3D_Scene &scene, int timeDelta)
{
	Storm3D_Camera &camera = static_cast<Storm3D_Camera &> (*scene.GetCamera());

	StormSpotList::iterator is = data->spots.begin();
	for(; is != data->spots.end(); ++is)
		is->get()->testVisibility(camera);

	FakeSpotList::iterator isf = data->fakeSpots.begin();
	for(; isf != data->fakeSpots.end(); ++isf)
		isf->get()->testVisibility(camera);

	if(!data->freezeCameraCulling && !data->freezeSpotCulling)
	{
		data->heightMap.calculateVisibility(scene);
		data->models.calculateVisibility(scene, timeDelta);
	}
}

//! Render targets
/*!
	\param scene scene
*/
void Storm3D_TerrainRenderer::renderTargets(Storm3D_Scene &scene)
{
	if(!data->forceDraw && !data->renderRenderTargets)
	{
		data->decalSystem.clearShadowDecals();
		if(data->particleSystem)
			data->particleSystem->Clear();

		return;
	}

	data->models.setForcedDirectional(data->forcedDirectionalLightEnabled, data->forcedDirectionalLightDirection, data->forcedDirectionalLightColor);

	data->forceDraw = false;
	data->storm.getProceduralManagerImp().enableDistortionMode(data->renderOffsets);
	data->storm.getProceduralManagerImp().useFallback(data->proceduralFallback);

	data->models.enableGlow(data->renderGlows);
	data->models.enableDistortion(data->renderOffsets);
	data->models.enableReflection(data->reflection);
	data->models.enableAdditionalAlphaTestPass(data->additionalAlphaTestPassAllowed);
	data->models.enableSkyModelGlow(data->skyModelGlowAllowed);

	Storm3D_Camera &camera = static_cast<Storm3D_Camera &> (*scene.GetCamera());
	camera.Apply();

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	igios_unimplemented();
	//device.SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	glDisable(GL_NORMALIZE);

	frozenbyte::storm::setCulling(CULL_CCW);

	D3DXMATRIX dm;
	D3DXMatrixIdentity(dm);

	data->models.setCollisionRendering(data->renderCollision);

	// Rendertarget stuff
	{
		SDL_Rect frameRect = { 0 };
		frameRect.w = data->renderSize.x;
		frameRect.h = data->renderSize.y;
		if(data->halfRendering)
		{
			frameRect.w /= 2;
			frameRect.h /= 2;
		}

		glDisable(GL_SCISSOR_TEST);

		boost::shared_ptr<glTexWrapper> tempDepthSurface = data->storm.getDepthTarget();

		// 2D lights
		// draws lights to fakeTexture
		{
			data->fbo->setRenderTarget(data->fakeTexture);
			if (!data->fbo->validate()) {
				igiosWarning("renderTargets: renderTarget validate failed with fakeTexture\n");
				data->fbo->disable();
				return;
			}

			glViewport(0, 0, data->renderSize.x, data->renderSize.y);
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			// why?
			glDisable(GL_CULL_FACE);

			if(data->renderFakeLights)
				data->lightManager.renderFakeLights(data->renderSize);
		}

		// Light targets
		// spotlight depth maps

		{
			// FIXME: broken?
			igios_unimplemented();
			glPolygonOffset(1, 1);
			glEnable(GL_POLYGON_OFFSET_FILL);

			data->lightManager.setFog(data->fogStart, data->fogEnd);

			Storm3D_ShaderManager::GetSingleton()->SetAmbient(COL(0,0,0));
			Storm3D_ShaderManager::GetSingleton()->SetModelAmbient(COL(0,0,0));
			Storm3D_ShaderManager::GetSingleton()->SetObjectAmbient(COL(0,0,0));
			Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(COL(1.f, 1.f, 1.f));
			data->renderLightTargets(scene);

			Storm3D_ShaderManager::GetSingleton()->setLightingShaders();
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		data->fbo->disable();
		glViewport(data->viewport.x, data->viewport.y, data->viewport.w, data->viewport.h);

		glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_FOG);
		GLfloat fogcol[4];
		fogcol[0] = data->fogColor.r; fogcol[1] = data->fogColor.g; fogcol[2] = data->fogColor.b; fogcol[3] = 1.0f;
		glFogfv(GL_FOG_COLOR, fogcol);

		glDisable(GL_FOG);

		// Terrain textures
		{
			data->fbo->setRenderTarget(data->terrainTexture);
			GLbitfield clearFlag = GL_DEPTH_BUFFER_BIT;
			if(data->storm.support_stencil)
				clearFlag |= GL_STENCIL_BUFFER_BIT;

			glClearDepth(1.0);
			glClearStencil(0);
			if(data->renderTerrainTextures)
				glClearColor(0, 0, 0, 0);
			else
				glClearColor(1, 1, 1, 1);
			glDisable(GL_SCISSOR_TEST);
			glClear(clearFlag);

			glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
			glEnable(GL_SCISSOR_TEST);

			Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);
			// this draws terrain textures (snow etc.)
			// (not rocks etc.)
			if(data->renderHeightmap && data->renderTerrainTextures)
				data->heightMap.renderTextures(scene);

			data->fbo->disable();
		}

		glEnable(GL_FOG);
		frozenbyte::storm::PixelShader::disable();

		// (Ambient + lightmaps + fake) * base to fb
		// this renders pretty much everything...
		{
			data->fbo->setRenderTarget(data->renderTexture);

			// this renders terrain
			// not lights and terrain models
			glDisable(GL_SCISSOR_TEST);
			glClearColor(data->clearColor.r, data->clearColor.g, data->clearColor.b, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
			glEnable(GL_SCISSOR_TEST);

			glEnable(GL_FOG);
			data->renderPass(scene, data->Solid);
			glDisable(GL_FOG);

			if(data->skyBox && !data->renderWireframe && data->renderSkyBox)
			{
				data->skyBox->SetPosition(camera.GetPosition());
				// sideways mode, rotate skybox --jpk
				VC3 camup = camera.GetUpVec();
				if (camup.x == 0
					&& camup.y == 0
					&& camup.z == 1)
				{
					data->skyBox->SetRotation(QUAT(3.1415926f/2.0f,0,0));
				} else {
					data->skyBox->SetRotation(QUAT(0,0,0));
				}
				data->skyBox->SetScale(VC3(20.f, 20.f, 20.f));

				data->skyboxPixelShader.apply();
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glDisable(GL_TEXTURE_CUBE_MAP);

				glDepthMask(GL_FALSE);
				data->models.renderBackground(data->skyBox);
			}

			// this tries to draw blob shadows under models
			// but it's broken and even on direct3d seems to have no effect
			// call it anyway to clear the list of decals
			// FIXME
			data->decalSystem.renderShadows(scene);

			// does nothing?
			// FIXME: produces interesting artifacts
			data->renderFakelights(scene);

			Storm3D_ShaderManager::GetSingleton()->SetAmbient(COL(0,0,0));
			Storm3D_ShaderManager::GetSingleton()->SetModelAmbient(COL(0,0,0));
			Storm3D_ShaderManager::GetSingleton()->SetObjectAmbient(COL(0,0,0));
			Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(COL(1.f, 1.f, 1.f));

			// this renders spot lights
			data->renderProjectedLightsSolid(scene);
			glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
			glEnable(GL_SCISSOR_TEST);

			data->renderPass(scene, data->Alpha);
			data->renderProjectedLightsAlpha(scene);
			glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
			glEnable(GL_SCISSOR_TEST);

			if(data->renderCones)
				data->renderConeLights(scene, data->renderGlows);

			// this renders bullet trails, fire etc.
			if(data->particleSystem && data->renderParticles)
			{
				data->particleSystem->RenderImp(&scene, false);
				frozenbyte::storm::setCulling(CULL_CCW);
			}

			data->fbo->disable();
		}

		for (unsigned int i = 0; i < 4; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glClientActiveTexture(GL_TEXTURE0 + i);		
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
		}

		if(data->renderGlows)
		{
			Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

			// Material self illuminations to glow texture
			{
				data->fbo->setRenderTarget(data->glowTexture2);
				if (!data->fbo->validate()) {
					igiosWarning("renderTargets: renderTarget validate failed with glowTexture2\n");
					data->fbo->disable();
					return;
				}

				// FIXME: depth/stencil mismatch on medium graphics quality
				glDisable(GL_SCISSOR_TEST);
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);

				glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
				glEnable(GL_SCISSOR_TEST);
				glViewport(data->viewport.x, data->viewport.y, data->viewport.w, data->viewport.h);

				if(data->glowFactor > 0.001f)
				{
					glActiveTexture(GL_TEXTURE1);
					glClientActiveTexture(GL_TEXTURE1);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_TEXTURE_3D);
					glDisable(GL_TEXTURE_CUBE_MAP);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);

					glActiveTexture(GL_TEXTURE0);
					glClientActiveTexture(GL_TEXTURE0);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
					glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
					glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);

					int c = (unsigned char)(data->glowFactor * 255.f);
					if(c > 255)
						c = 255;
					else if(c < 1)
						c = 1;

					DWORD color = c << 24 | c << 16 | c << 8 | c;
					VXFORMAT_2D buffer[4];
					float x2 = float(data->renderSize.x);
					float y2 = float(data->renderSize.y);
					buffer[0] = VXFORMAT_2D(VC3(0, y2, 1.f),  1.f, color, VC2(0.f, 1.f));
					buffer[1] = VXFORMAT_2D(VC3(0, 0, 1.f),   1.f, color, VC2(0.f, 0.f));
					buffer[2] = VXFORMAT_2D(VC3(x2, y2, 1.f), 1.f, color, VC2(1.f, 1.f));
					buffer[3] = VXFORMAT_2D(VC3(x2, 0, 1.f),  1.f, color, VC2(1.f, 0.f));

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();

					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();

					glOrtho(0, data->renderSize.x, data->renderSize.y, 0, -1, 1);

					glEnable(GL_BLEND);
					glBlendFunc(GL_DST_COLOR, GL_ZERO);
					glDepthFunc(GL_ALWAYS);
					glDepthMask(GL_FALSE);

					glVertexPointer(3, GL_FLOAT, sizeof(VXFORMAT_2D), buffer);
					glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VXFORMAT_2D), &buffer[0].color);
					glTexCoordPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), &buffer[0].texcoords);

					glEnableClientState(GL_VERTEX_ARRAY);
					glEnableClientState(GL_COLOR_ARRAY);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

					glDisableClientState(GL_VERTEX_ARRAY);
					glDisableClientState(GL_COLOR_ARRAY);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);

					glDepthFunc(GL_LEQUAL);
				}

				if(data->renderModels)
				{
					glEnable(GL_BLEND);
					glDisable(GL_ALPHA_TEST);
					glBlendFunc(GL_ONE, GL_ONE);

					data->models.renderGlows(scene);

					if(data->renderCones)
						data->renderConeLights(scene, data->renderGlows);
				}

				glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);

				data->fbo->disable();
			}

			frozenbyte::storm::setCulling(CULL_NONE);
			frozenbyte::storm::VertexShader::disable();

			int glowPasses = (data->multipassGlow) ? 2 : 1;
					
			int stages = 4;

			data->glowShader.apply();

			float af = .6f;
			float bf = .3f;
			float cf = .15f;
			float df = .075f;
			float ef = .8f;

			if(data->multipassGlow)
				ef = 0.65f;

			float a[] = { af, af, af, 1 };
			float b[] = { bf, bf, bf, 1 };
			float c[] = { cf, cf, cf, 1 };
			float d[] = { df, df, df, 1 };
			float e[] = { ef, ef, ef, 1 };

			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, a);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, b);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, c);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, d);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, e);

			float offset = 0;
			float width = float(data->viewport.w) + offset;
			float height = float(data->viewport.h) + offset;

			for(int i = 0; i < glowPasses; ++i)
			{
				// Filter glow texture
				{
					{
						data->fbo->setRenderTarget(data->glowTexture1);
						if (!data->fbo->validate()) {
							igiosWarning("renderTargets: renderTarget validate failed with glowTexture1\n");
							data->fbo->disable();
							return;
						}
						glDepthMask(GL_FALSE);
						glDisable(GL_DEPTH_TEST);
						
						// FIXME ?
						GLint sbox[4];
						glGetIntegerv(GL_SCISSOR_BOX, sbox);
						glScissor(data->glowSize.x - 1, 0, 12, data->glowSize.y);
						glClear(GL_COLOR_BUFFER_BIT);
						glScissor(0, data->glowSize.y - 1, data->glowSize.x, 12);
						glClear(GL_COLOR_BUFFER_BIT);
						glScissor(sbox[0], sbox[1], sbox[2], sbox[3]);

						float xs = float(data->viewport.w) / (data->glowTexture2->getWidth());
						float ys = float(data->viewport.h) / (data->glowTexture2->getHeight());

						float dp = (1.f / data->viewport.w) * xs;		
						float d1 = 0.5f * dp;
						float d2 = 2.5f * dp;
						float d3 = 4.5f * dp;
						float d4 = 6.5f * dp;

						float buffer1[] = 
						{
							// Position,          w,    uv1,           uv2,           uv3,           uv4
							offset, height,  1.f, 1.f,  0.f-d1, 0.f,  0.f-d2, 0.f,  0.f-d3, 0.f,  0.f-d4, 0.f,
							offset, offset,  1.f, 1.f,  0.f-d1, ys,   0.f-d2, ys,   0.f-d3, ys,   0.f-d4, ys,
							width,  height,  1.f, 1.f,  xs-d1,  0.f,  xs-d2,  0.f,  xs-d3,  0.f,  xs-d4,  0.f,
							width,  offset,  1.f, 1.f,  xs-d1,  ys,   xs-d2,  ys,   xs-d3,  ys,   xs-d4,  ys,
						};

						float buffer2[] = 
						{
							// Position,          w,    uv1,           uv2,           uv3,           uv4
							offset, height,  1.f, 1.f,  0.f+d1, 0.f,  0.f+d2, 0.f,  0.f+d3, 0.f,  0.f+d4, 0.f,
							offset, offset,  1.f, 1.f,  0.f+d1, ys,   0.f+d2, ys,   0.f+d3, ys,   0.f+d4, ys,
							width,  height,  1.f, 1.f,  xs+d1,  0.f,  xs+d2, 0.f,   xs+d3,  0.f,  xs+d4,  0.f,
							width,  offset,  1.f, 1.f,  xs+d1,  ys,   xs+d2, ys,    xs+d3,  ys,   xs+d4,  ys,
						};

						for(int stage = 0; stage < stages; ++stage)
						{
							glActiveTexture(GL_TEXTURE0 + stage);
							glClientActiveTexture(GL_TEXTURE0 + stage);
							data->glowTexture2->bind();
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
						}

						renderUP(D3DFVF_XYZRHW|D3DFVF_TEX4, GL_TRIANGLE_STRIP, 2, sizeof(float) * 12, (char *)buffer1);

						glEnable(GL_BLEND);
						glDisable(GL_ALPHA_TEST);
						glBlendFunc(GL_ONE, GL_ONE);

						renderUP(D3DFVF_XYZRHW|D3DFVF_TEX4, GL_TRIANGLE_STRIP, 2, sizeof(float) * 12, (char *)buffer2);

						glDisable(GL_BLEND);
						glDisable(GL_ALPHA_TEST);
					}

					// Blur to final
					{
						data->fbo->setRenderTarget(data->glowTexture2);
						if (!data->fbo->validate()) {
							igiosWarning("renderTargets: renderTarget validate failed with glowTexture2\n");
							data->fbo->disable();
							return;
						}

						float xs = float(data->viewport.w) / (data->glowTexture1->getWidth());
						float ys = float(data->viewport.h) / (data->glowTexture1->getHeight());

						float dp = (1.f / data->viewport.h) * ys;		
						float d1 = 0.5f * dp;
						float d2 = 2.5f * dp;
						float d3 = 4.5f * dp;
						float d4 = 6.5f * dp;

						float buffer1[] = 
						{
							// Position,          w,    uv1,           uv2,           uv3,           uv4
							offset, height,  1.f, 1.f,  0.f, 0.f-d1,  0.f, 0.f-d2,  0.f, 0.f-d3,  0.f, 0.f-d4,
							offset, offset,  1.f, 1.f,  0.f, ys-d1,   0.f, ys-d2,   0.f, ys-d3,   0.f, ys-d4,
							width,  height,  1.f, 1.f,  xs,  0.f-d1,  xs,  0.f-d2,  xs,  0.f-d3,  xs,  0.f-d4,
							width,  offset,  1.f, 1.f,  xs,  ys-d1,   xs,  ys-d2,   xs,  ys-d3,   xs,  ys-d4
						};

						float buffer2[] = 
						{
							// Position,          w,    uv1,           uv2,           uv3,           uv4
							offset, height,  1.f, 1.f,  0.f, 0.f+d1,  0.f, 0.f+d2,  0.f, 0.f+d3,  0.f, 0.f+d4,
							offset, offset,  1.f, 1.f,  0.f, ys+d1,   0.f, ys+d2,   0.f, ys+d3,   0.f, ys+d4,
							width,  height,  1.f, 1.f,  xs,  0.f+d1,  xs,  0.f+d2,  xs,  0.f+d3,  xs,  0.f+d4,
							width,  offset,  1.f, 1.f,  xs,  ys+d1,   xs,  ys+d2,   xs,  ys+d3,   xs,  ys+d4
						};

						for(int stage = 0; stage < stages; ++stage)
						{
							glActiveTexture(GL_TEXTURE0 + stage);
							glClientActiveTexture(GL_TEXTURE0 + stage);
							data->glowTexture1->bind();
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
						}

						glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
						glEnable(GL_SCISSOR_TEST);

						renderUP(D3DFVF_XYZRHW|D3DFVF_TEX4, GL_TRIANGLE_STRIP, 2, sizeof(float) * 12, (char *)buffer1);

						glEnable(GL_BLEND);

						renderUP(D3DFVF_XYZRHW|D3DFVF_TEX4, GL_TRIANGLE_STRIP, 2, sizeof(float) * 12, (char *)buffer2);

						glDisable(GL_BLEND);
					}
				}
			}

			frozenbyte::storm::setCulling(CULL_CCW);
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);

			for (unsigned int i = 0; i < 4; ++i) {
				glActiveTexture(GL_TEXTURE0 + i);
				glClientActiveTexture(GL_TEXTURE0 + i);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_3D);
				glDisable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			frozenbyte::storm::PixelShader::disable();
			data->fbo->disable();
			glViewport(data->viewport.x, data->viewport.y, data->viewport.w, data->viewport.h);
		}

		if(data->renderOffsets && data->offsetTexture)
		{
			data->fbo->setRenderTarget(data->offsetTexture);
			if (!data->fbo->validate()) {
				igiosWarning("renderTargets: renderTarget validate failed with offsetTexture\n");
				data->fbo->disable();
				return;
			}
			glDepthMask(GL_FALSE);

			glScissor(data->scissorRect.x, data->scissorRect.y, data->scissorRect.w, data->scissorRect.h);
			glClearColor(0.5, 0.5, 0.5, 0.5);
			glEnable(GL_SCISSOR_TEST);
			glClear(GL_COLOR_BUFFER_BIT);

			// render the distortion effect
			if(data->particleSystem && data->renderParticles)
			{
				Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
				if(data->renderModels)
					data->models.renderDistortion(scene);

				data->particleSystem->RenderImp(&scene, true);
				frozenbyte::storm::setCulling(CULL_CCW);
			}

			glDepthMask(GL_TRUE);
			data->fbo->disable();
		}

		glDepthMask(GL_FALSE);

		if(data->renderGlows)
		{
			frozenbyte::storm::setCulling(CULL_NONE);
			glDisable(GL_DEPTH_TEST);
			data->fbo->setRenderTarget(data->renderTexture);
			if (!data->fbo->validate()) {
				igiosWarning("renderTargets: renderTarget validate failed with renderTexture\n");
				data->fbo->disable();
				return;
			}
			glViewport(data->viewport.x, data->viewport.y, data->viewport.w, data->viewport.h);

			float width = data->viewport.w;
			float height = data->viewport.h;
			float textureOffset = 0.f;
			float glowWidth = float(data->viewport.w) / data->glowTexture2->getWidth();
			float glowHeight = float(data->viewport.h) / data->glowTexture2->getHeight();

			if(data->halfRendering)
			{
				width *= 0.5f;
				height *= 0.5f;
			}

			float glowTex1[] = 
			{
				textureOffset, height,         1.f, 1.f, 0.f, 0.f,
				textureOffset, textureOffset,  1.f, 1.f, 0.f, glowHeight,
				width, height,                 1.f, 1.f, glowWidth, 0.f,
				width, textureOffset,          1.f, 1.f, glowWidth, glowHeight
			};

			glEnable(GL_BLEND);

			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_CUBE_MAP);

			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE); //PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

			data->glowTexture2->bind();

			if (data->renderGlowDebug)
			{
				glDisable(GL_BLEND);
			}

			frozenbyte::storm::PixelShader::disable();
			frozenbyte::storm::VertexShader::disable();


			// Transparency
			if(data->glowTransparencyFactor > 0.001f)
			{
				float c = data->glowTransparencyFactor;
				if(c > 1)
					c = 1;
				else if(c < 0)
					c = 0;
				glBlendColor(1, 1, 1, 1 - c);

				glBlendFunc(GL_CONSTANT_COLOR, GL_ONE); //_MINUS_CONSTANT_COLOR);
				renderUP(D3DFVF_XYZRHW|D3DFVF_TEX1, GL_TRIANGLE_STRIP, 2, sizeof(float) * 6, (char *)glowTex1);
			}

			// Additive
			if(data->glowAdditiveFactor > 0.001f)
			{
				int c = (int)(data->glowAdditiveFactor * 255.f);
				if(c > 255)
					c = 255;
				else if(c < 1)
					c = 1;
				GLint color[4];
				color[0] = color[1] = color[2] = color[3] = c;
				glTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);

				glActiveTexture(GL_TEXTURE0);
				glClientActiveTexture(GL_TEXTURE0);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE); //CONSTANT);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

				glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
				renderUP(D3DFVF_XYZRHW|D3DFVF_TEX1, GL_TRIANGLE_STRIP, 2, sizeof(float) * 6, (char *)glowTex1);
			}

			glEnable(GL_DEPTH_TEST);
			data->fbo->disable();
			frozenbyte::storm::setCulling(CULL_CCW);
			glViewport(data->viewport.x, data->viewport.y, data->viewport.w, data->viewport.h);
		}

		glDepthMask(GL_TRUE);
	}

	Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	if(data->particleSystem && active_visibility == 0)
		data->particleSystem->Clear();

	data->fakeLights.clear();
}

//! Render base
/*!
	\param scene scene
*/
void Storm3D_TerrainRenderer::renderBase(Storm3D_Scene &scene)
{
	Storm3D_Camera &camera = static_cast<Storm3D_Camera &> (*scene.GetCamera());
	camera.Apply();

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	//device.SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	glDisable(GL_NORMALIZE);
	glPolygonMode(GL_FRONT, GL_FILL);

	frozenbyte::storm::setCulling(CULL_NONE);

	D3DXMATRIX dm;
	D3DXMatrixIdentity(dm);

	data->models.setCollisionRendering(data->renderCollision);
	data->models.setForcedDirectional(data->forcedDirectionalLightEnabled, data->forcedDirectionalLightDirection, data->forcedDirectionalLightColor);

	Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

	float width = float(data->renderTexture->getWidth()) - .5f;
	float height = float(data->renderTexture->getHeight()) - .5f;

	float textureOffset = -1.f;

	textureOffset = -.5f;
    /*
	float bufferTex2[] = 
	{
		textureOffset, height,         1.f, 1.f, 0.f, 1.f,
		textureOffset, textureOffset,  1.f, 1.f, 0.f, 0.f,
		width, height,                 1.f, 1.f, 1.f, 1.f,
		width, textureOffset,          1.f, 1.f, 1.f, 0.f
	};

	float bufferTex3[] = 
	{
		textureOffset, height,         1.f, 1.f, 0.f, 1.f, 0.f, 1.f,
		textureOffset, textureOffset,  1.f, 1.f, 0.f, 0.f, 0.f, 0.f,
		width, height,                 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
		width, textureOffset,          1.f, 1.f, 1.f, 0.f, 1.f, 0.f
	};
	*/

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	frozenbyte::storm::VertexShader::disable();
	applyFVF(D3DFVF_XYZRHW|D3DFVF_TEX1, 6 * sizeof(float));

	Storm3D_ShaderManager::GetSingleton()->setNormalShaders();

	glViewport(data->viewportSizes[0].x, data->viewportSizes[0].y, data->viewportSizes[0].w, data->viewportSizes[0].h);
	glScissor(data->scissorRectSizes[0].x, data->scissorRectSizes[0].y, data->scissorRectSizes[0].w, data->scissorRectSizes[0].h);
	glEnable(GL_SCISSOR_TEST);

/*  This made the bottom black bar in in-game movies disappear. FIXME?
	if(data->movieAspect && data->halfRendering)
		glDisable(GL_SCISSOR_TEST);
*/
	// Render map
	if(data->renderMode == IStorm3D_TerrainRenderer::Normal || data->renderMode == IStorm3D_TerrainRenderer::LightOnly)
	{
		for (unsigned int i = 0; i < 6; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glDisable(GL_TEXTURE_2D);
			//			glDisable(GL_TEXTURE_3D);
			//			glDisable(GL_TEXTURE_CUBE_MAP);
		}
		/* causes gui to turn black in spider trouble
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		*/

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		//glEnable(GL_ALPHA_TEST);

		float offsetWidth = 1.f;
		float offsetHeight = 1.f;
		{
			if(data->offsetTexture)
			{
				offsetWidth = float(data->renderSize.x) / data->offsetTexture->getWidth();
				offsetHeight = float(data->renderSize.y) / data->offsetTexture->getHeight();
				// FIXME: This doesn't make any fucking sense since the offset texture is only partially rendered
				offsetWidth = offsetWidth > 0.5f ? 1 : 0.5f;
				offsetHeight = offsetHeight > 0.5f ? 1 : 0.5f;
			}
		}

		float xv = 1.f;
		float yv = 1.f;
		if(data->halfRendering)
		{
			xv = 0.5f;
			yv = 0.5f;
			offsetWidth *= 0.5f;
			offsetHeight *= 0.5f;
		}

		float bufferTex[] =
		{
			textureOffset, height,         1.f, 1.f, 0.f, yv,  0.f,         offsetHeight, 0.f, 1.f,
			textureOffset, textureOffset,  1.f, 1.f, 0.f, 0.f, 0.f,         0.f,          0.f, 0.f,
			width,         height,         1.f, 1.f, xv,  yv,  offsetWidth, offsetHeight, 1.f, 1.f,
			width,         textureOffset,  1.f, 1.f, xv,  0.f, offsetWidth, 0.f,          1.f, 0.f
		};

		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		// renderTexture has everything
		data->renderTexture->bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		if(data->renderOffsets && data->offsetTexture)
		{
			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			// offsetTexture has distortion displacement texture
			data->offsetTexture->bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			if(data->offsetFade)
				data->offsetFade->Apply(2);
		}


		if(!data->renderBlackWhite && (data->colorEffectOn || data->renderOffsets))
		{

			if(data->renderOffsets && data->offsetTexture)
			{
				if(data->colorEffectOn)
					data->colorEffectOffsetShader.apply();
				else
					data->colorEffectOffsetShader_NoGamma.apply();
			}
			else
				data->colorEffectShader.apply();

			float c = data->contrast;
			float b = data->brightness;
			const COL &col = data->colorFactors;

			float c2[4] = { c, c, c, c };
			float c3[4] = { b * (col.r + 1.f), b * (col.g + 1.f), b * (col.b + 1.f), 0 };
			float c4[4] = { col.r, col.g, col.b, 1.f };

			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, c2);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, c3);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, c4);
			
		}
		else if(data->renderBlackWhite)
		{
			data->blackWhiteShader.apply();
		}
		else
		{
			frozenbyte::storm::PixelShader::disable();
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			/*
			data->colorEffectShader.apply();

			float c = data->contrast;
			float b = data->brightness;
			const COL &col = data->colorFactors;

			float c2[4] = { c, c, c, c };
			float c3[4] = { b * (col.r + 1.f), b * (col.g + 1.f), b * (col.b + 1.f), 0 };
			float c4[4] = { col.r, col.g, col.b, 1.f };

			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, c2);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, c3);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, c4);
			*/
		}

		float offsetScale = 0.06f;
		offsetScale *= data->renderSize.x / 1024.f;
		if(data->halfRendering)
			offsetScale *= 0.5f;
		float offsetValues[4] = { offsetScale, offsetScale, offsetScale, offsetScale };
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 5, offsetValues);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(float) * 10, bufferTex);
		for (unsigned int i = 0; i < 4; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 10, bufferTex + (4 + i * 2));
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0, data->renderSize.x, data->renderSize.y, 0, -1, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		frozenbyte::storm::PixelShader::disable();
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	data->fakeLights.clear();

	if(data->renderSpotDebug && data->spots[0])
		data->spots[0]->debugRender();
	if(data->renderFakeShadowDebug && data->fakeSpots[0])
		data->fakeSpots[0]->debugRender();

	glDisable(GL_SCISSOR_TEST);
	frozenbyte::storm::setCulling(CULL_CCW);

	// debugging code
	if (false) {
		glErrors();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		for (int i = 1; i < 8; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		data->glowTexture2->bind();
		//data->offsetTexture->bind();
		//data->lightManager.setDebug();
		//data->terrainTexture->bind();
		//data->renderTexture->bind();
		//data->fakeTexture->bind();
		glErrors();

		glErrors();
		frozenbyte::storm::PixelShader::disable();
		frozenbyte::storm::VertexShader::disable();
//		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glErrors();
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_FOG);
		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glErrors();
		float x2 = 300, y2 = 300;
		VXFORMAT_2D buffer[4];
		DWORD color = 0xFFFFFFFF;
		buffer[0] = VXFORMAT_2D(VC3(0, y2, 1.f),  1.f, color, VC2(0.f, 1.f));
		buffer[1] = VXFORMAT_2D(VC3(0, 0, 1.f),   1.f, color, VC2(0.f, 0.f));
		buffer[2] = VXFORMAT_2D(VC3(x2, y2, 1.f), 1.f, color, VC2(1.f, 1.f));
		buffer[3] = VXFORMAT_2D(VC3(x2, 0, 1.f),  1.f, color, VC2(1.f, 0.f));
		renderUP(FVF_VXFORMAT_2D, GL_TRIANGLE_STRIP, 2, sizeof(VXFORMAT_2D), (char *) buffer);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glPopClientAttrib();
		glPopAttrib();
		glErrors();
	}

}

//! Render terrain
/*!
	\param mode render mode
	\param scene scene
	\param spot spotlight
	\param fakeSpot fake spotlight
*/
void Storm3D_TerrainRenderer::render(IStorm3D_TerrainRendererBase::RenderMode mode, Storm3D_Scene &scene, Storm3D_Spotlight *spot, Storm3D_FakeSpotlight *fakeSpot)
{
	Storm3D_Camera &camera = *static_cast<Storm3D_Camera *> (scene.GetCamera());

	if(mode == SpotBuffer && spot)
	{
		// removing this will break shadows
		if(data->renderModels)
			data->models.renderDepth(scene, camera, *spot, spot->getNoShadowModel());

		if(data->renderHeightmap)
			data->heightMap.renderDepth(scene, &camera, Storm3D_TerrainHeightmap::Depth, spot->getType(), spot);
	}
	else if((mode == SpotProjectionSolid || mode == SpotProjectionDecal || mode == SpotProjectionAlpha) && spot)
	{
		Storm3D_Camera spotCamera = spot->getCamera();
		VC2I renderSize = data->renderSize;
		if(data->halfRendering)
			renderSize /= 2;

		if(!spot->setScissorRect(camera, renderSize, scene))
			return;

		if(spot->featureEnabled(IStorm3D_Spotlight::Fade))
		{
			glActiveTexture(GL_TEXTURE3);
			glClientActiveTexture(GL_TEXTURE3);
			data->spotFadeTexture->bind();
		}
		else
		{
			glActiveTexture(GL_TEXTURE3);
			glClientActiveTexture(GL_TEXTURE3);
			data->noFadeTexture->bind();
		}

		// removing this breaks spotlight shadows
		if(mode == SpotProjectionSolid)
		{
			if(data->renderModels)
				data->models.renderProjection(Storm3D_TerrainModels::SolidOnly, scene, *spot);

			spot->applyTerrainShader(data->renderSpotShadows);
			if(data->renderHeightmap)
			{
				Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(COL(1.f, 1.f, 1.f));
				glActiveTexture(GL_TEXTURE2);
				glClientActiveTexture(GL_TEXTURE2);
				data->terrainTexture->bind();
				for (int i = 0; i < 4; i++) {
					glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 18 + i, data->terrainTextureProjection.raw + (4 * i));
				}

				data->heightMap.renderDepth(scene, &spotCamera, Storm3D_TerrainHeightmap::Projection, spot->getType(), spot);
			}
		}
		else if(mode == SpotProjectionDecal)
		{
			if(data->renderDecals)
				data->decalSystem.renderProjection(scene, spot);
		}
		else
		{
			if(data->renderModels)
				data->models.renderProjection(Storm3D_TerrainModels::AlphaOnly, scene, *spot);
		}
	}
	else if(mode == FakeSpotBuffer && fakeSpot)
	{
		bool needRender = false;

		if(data->renderModels)
		{
			if(data->models.renderDepth(scene, camera, *fakeSpot))
				needRender = true;
		}

		if(!needRender)
			fakeSpot->disableVisibility();
	}
	else if(mode == FakeSpotProjection && fakeSpot)
	{
		fakeSpot->renderProjection();
	}
}

//! Release dynamic resources
void Storm3D_TerrainRenderer::releaseDynamicResources()
{
	FakeSpotList::iterator itf = data->fakeSpots.begin();
	for(; itf != data->fakeSpots.end(); ++itf)
		(*itf)->releaseDynamicResources();
}

//! Recreate dynamic resources
void Storm3D_TerrainRenderer::recreateDynamicResources()
{
	data->forceDraw = true;

	FakeSpotList::iterator itf = data->fakeSpots.begin();
	for(; itf != data->fakeSpots.end(); ++itf)
		(*itf)->recreateDynamicResources();
}

//! Query sizes
/*!
	\param storm Storm3D
	\param enableGlow is glow enabled
*/
void Storm3D_TerrainRenderer::querySizes(Storm3D &storm, bool enableGlow)
{
	Storm3D_TerrainRendererData::querySizes(storm, enableGlow);
}

//! Create render buffers
/*!
	\param storm Storm3D
	\param lightmapQuality lightmap quality
*/
void Storm3D_TerrainRenderer::createRenderBuffers(Storm3D &storm, int lightmapQuality)
{
	Storm3D_TerrainRendererData::createTextures(storm, lightmapQuality);
}

//! Free render buffers
void Storm3D_TerrainRenderer::freeRenderBuffers()
{
	Storm3D_TerrainRendererData::freeTextures();
}

//! Create secondary render buffers
/*!
	\param storm Storm3D
	\param enableGlow is glow enabled
*/
void Storm3D_TerrainRenderer::createSecondaryRenderBuffers(Storm3D &storm, bool enableGlow)
{
	Storm3D_TerrainRendererData::createSecondaryTextures(storm, enableGlow);
}

//! Free secondary render buffers
void Storm3D_TerrainRenderer::freeSecondaryRenderBuffers()
{
	Storm3D_TerrainRendererData::freeSecondaryTextures();
}

//! Are required buffers present?
/*!
	\return true if necessary buffers exist
*/
bool Storm3D_TerrainRenderer::hasNeededBuffers()
{
	if(!Storm3D_TerrainRendererData::renderTexture)
		return false;
	if(!Storm3D_TerrainRendererData::fakeTexture)
		return false;

	return true;
}
