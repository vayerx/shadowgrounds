
#include "precompiled.h"

#include "OptionApplier.h"

#include "Game.h"
#include "GameUI.h"
#include "../ogui/Ogui.h"
#include "GameScene.h"
#include "GameOptionManager.h"
#include "SimpleOptions.h"
#include "../sound/SoundMixer.h"
#include "../ui/LightManager.h"
#include "../ui/VisualEffectManager.h"
#include "../particle_editor2/particleeffect.h"
#include "options/options_all.h"

#ifdef PHYSICS_PHYSX
#include "../physics/physics_lib.h"
#include "physics/GamePhysics.h"
#endif

#include <Storm3D_UI.h>
#include <istorm3D_terrain_renderer.h>

#include "../system/Logger.h"

namespace game
{

	void OptionApplier::applyLoggerOptions()
	{
		int loglevel = 1;
		int showloglevel = 1;

		loglevel = SimpleOptions::getInt(DH_OPT_I_LOGLEVEL);
		if (loglevel < 0) loglevel = 0;
		if (loglevel > 4) loglevel = 4;
		showloglevel = SimpleOptions::getInt(DH_OPT_I_CONSOLE_LOGLEVEL);
		if (showloglevel < 0) showloglevel = 0;
		if (showloglevel > 4) showloglevel = 4;

		Logger::getInstance()->setLogLevel(loglevel);
		Logger::getInstance()->setListenerLogLevel(showloglevel);
		if (loglevel == LOGGER_LEVEL_DEBUG)
		{
			Logger::getInstance()->debug("Logger on debug level.");
		}
	}

	void OptionApplier::applySoundOptions(sfx::SoundMixer *soundMixer)
	{
		if (soundMixer != NULL)
		{
			soundMixer->setVolume(SimpleOptions::getInt(DH_OPT_I_MASTER_VOLUME),
				SimpleOptions::getInt(DH_OPT_I_FX_VOLUME),
				SimpleOptions::getInt(DH_OPT_I_SPEECH_VOLUME),
				SimpleOptions::getInt(DH_OPT_I_MUSIC_VOLUME),
				SimpleOptions::getInt(DH_OPT_I_AMBIENT_VOLUME));

			soundMixer->setMute(!SimpleOptions::getBool(DH_OPT_B_FX_ENABLED),
				!SimpleOptions::getBool(DH_OPT_B_SPEECH_ENABLED),
				!SimpleOptions::getBool(DH_OPT_B_MUSIC_ENABLED));
		}
	}

	void OptionApplier::applyGammaOptions(IStorm3D *s3d)
	{
		float gamma = 1.0f;
		float brightness = 1.0f;
		float contrast = 1.0f;
		float red_correction = 1.0f;
		float green_correction = 1.0f;
		float blue_correction = 1.0f;
		bool calibrate = false;

		gamma = SimpleOptions::getFloat(DH_OPT_F_GAMMA);
		brightness = SimpleOptions::getFloat(DH_OPT_F_BRIGHTNESS);
		contrast = SimpleOptions::getFloat(DH_OPT_F_CONTRAST);
		red_correction = SimpleOptions::getFloat(DH_OPT_F_RED_CORRECTION);
		green_correction = SimpleOptions::getFloat(DH_OPT_F_GREEN_CORRECTION);
		blue_correction = SimpleOptions::getFloat(DH_OPT_F_BLUE_CORRECTION);
		calibrate = SimpleOptions::getBool(DH_OPT_B_CALIBRATE_GAMMA);

		s3d->SetGammaRamp(gamma, brightness, contrast, 
			red_correction, green_correction, blue_correction, calibrate);
	}

	void OptionApplier::applyCameraOptions(IStorm3D_Scene *scene)
	{
		if (SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS))
		{
			scene->GetCamera()->SetUpVec(VC3(0,0,1));
		} else {
			scene->GetCamera()->SetUpVec(VC3(0,1,0));
		}
	}

	void OptionApplier::applyDisplayOptions(IStorm3D *s3d, IStorm3D_Scene *scene,
		IStorm3D_Terrain *terrain, ui::LightManager *lightManager)
	{
		applyGammaOptions(s3d);

		int anisotrophy = SimpleOptions::getInt(DH_OPT_I_ANISOTROPHY);
		scene->SetAnisotropicFilteringLevel(anisotrophy);

		if (terrain != NULL)
		{
			IStorm3D_TerrainRenderer &r = terrain->getRenderer();
			if (SimpleOptions::getInt(DH_OPT_I_RENDER_MODE) == 0)
				r.setRenderMode(IStorm3D_TerrainRenderer::Normal);
			else if (SimpleOptions::getInt(DH_OPT_I_RENDER_MODE) == 1)
				r.setRenderMode(IStorm3D_TerrainRenderer::LightOnly);
			else if (SimpleOptions::getInt(DH_OPT_I_RENDER_MODE) == 2)
				r.setRenderMode(IStorm3D_TerrainRenderer::TexturesOnly);			

			r.enableFeature(IStorm3D_TerrainRenderer::SpotShadows, SimpleOptions::getBool(DH_OPT_B_RENDER_SPOT_SHADOWS));
			r.enableFeature(IStorm3D_TerrainRenderer::Glow, SimpleOptions::getBool(DH_OPT_B_RENDER_GLOW));
			r.enableFeature(IStorm3D_TerrainRenderer::BetterGlowSampling, SimpleOptions::getBool(DH_OPT_B_BETTER_GLOW_SAMPLING));
			r.enableFeature(IStorm3D_TerrainRenderer::Distortion, SimpleOptions::getBool(DH_OPT_B_RENDER_DISTORTION));
			r.enableFeature(IStorm3D_TerrainRenderer::FakeLights, SimpleOptions::getBool(DH_OPT_B_RENDER_FAKELIGHTS));
			r.enableFeature(IStorm3D_TerrainRenderer::ModelRendering, SimpleOptions::getBool(DH_OPT_B_RENDER_MODELS));
			r.enableFeature(IStorm3D_TerrainRenderer::TerrainObjectRendering, SimpleOptions::getBool(DH_OPT_B_RENDER_TERRAINOBJECTS));
			r.enableFeature(IStorm3D_TerrainRenderer::HeightmapRendering, SimpleOptions::getBool(DH_OPT_B_RENDER_HEIGHTMAP));
			r.enableFeature(IStorm3D_TerrainRenderer::FreezeCameraCulling, SimpleOptions::getBool(DH_OPT_B_FREEZE_CAMERA_CULLING));
			r.enableFeature(IStorm3D_TerrainRenderer::FreezeSpotCulling, SimpleOptions::getBool(DH_OPT_B_FREEZE_SPOT_CULLING));
			r.enableFeature(IStorm3D_TerrainRenderer::Wireframe, SimpleOptions::getBool(DH_OPT_B_RENDER_WIREFRAME));
			r.enableFeature(IStorm3D_TerrainRenderer::Collision, SimpleOptions::getBool(DH_OPT_B_RENDER_COLLISION));
			r.enableFeature(IStorm3D_TerrainRenderer::LightmapFiltering, SimpleOptions::getBool(DH_OPT_B_LIGHTMAP_FILTERING));
			r.enableFeature(IStorm3D_TerrainRenderer::BlackAndWhite, SimpleOptions::getBool(DH_OPT_B_RENDER_BLACK_AND_WHITE));

			r.enableFeature(IStorm3D_TerrainRenderer::RenderDecals, SimpleOptions::getBool(DH_OPT_B_RENDER_DECALS));
			r.enableFeature(IStorm3D_TerrainRenderer::RenderSpotDebug, SimpleOptions::getBool(DH_OPT_B_RENDER_SPOT_DEBUG));
			r.enableFeature(IStorm3D_TerrainRenderer::RenderFakeShadowDebug, SimpleOptions::getBool(DH_OPT_B_RENDER_FAKESHADOW_DEBUG));
			r.enableFeature(IStorm3D_TerrainRenderer::RenderGlowDebug, SimpleOptions::getBool(DH_OPT_B_RENDER_GLOW_DEBUG));
			r.enableFeature(IStorm3D_TerrainRenderer::RenderBoned, SimpleOptions::getBool(DH_OPT_B_RENDER_BONED));

			r.enableFeature(IStorm3D_TerrainRenderer::Cone, SimpleOptions::getBool(DH_OPT_B_RENDER_CONE));
			r.enableFeature(IStorm3D_TerrainRenderer::Particles, SimpleOptions::getBool(DH_OPT_B_RENDER_PARTICLES));
			r.enableFeature(IStorm3D_TerrainRenderer::ParticleReflection, SimpleOptions::getBool(DH_OPT_B_RENDER_PARTICLE_REFLECTION));
			r.enableFeature(IStorm3D_TerrainRenderer::TerrainTextures, SimpleOptions::getBool(DH_OPT_B_RENDER_TERRAIN_TEXTURES));
			r.enableFeature(IStorm3D_TerrainRenderer::SmoothShadows, SimpleOptions::getBool(DH_OPT_B_RENDER_SMOOTHED_SHADOWS));
			r.enableFeature(IStorm3D_TerrainRenderer::ProceduralFallback, SimpleOptions::getBool(DH_OPT_B_PROCEDURAL_FALLBACK));

			r.enableFeature(IStorm3D_TerrainRenderer::HalfRendering, SimpleOptions::getBool(DH_OPT_B_RENDER_HALF_RESOLUTION));
			r.enableFeature(IStorm3D_TerrainRenderer::Reflection, SimpleOptions::getBool(DH_OPT_B_RENDER_REFLECTION));

			r.enableFeature(IStorm3D_TerrainRenderer::SkyModelGlow, SimpleOptions::getBool(DH_OPT_B_RENDER_SKY_BLOOM));
#ifndef PROJECT_SURVIVOR
			r.enableFeature(IStorm3D_TerrainRenderer::AdditionalAlphaTestPass, SimpleOptions::getBool(DH_OPT_B_RENDER_SKY_BLOOM));
#endif
		}

		if (lightManager != NULL)
		{
			lightManager->setShadowLevel(SimpleOptions::getInt(DH_OPT_I_SHADOWS_LEVEL));
			lightManager->setLightingLevel(SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL));
			lightManager->setMaxLightAmount(SimpleOptions::getInt(DH_OPT_I_RENDER_MAX_POINTLIGHTS));
		}
	}

	void OptionApplier::applyOptions(Game *game, GameOptionManager *oman, Ogui *ogui)
	{
		// TODO: more more more...

		applyLoggerOptions();

		applySoundOptions(game->gameUI->getSoundMixer());

		IStorm3D_Terrain *terrain = NULL;
		if(game->gameUI->getTerrain() != NULL)
		{
			terrain = game->gameUI->getTerrain()->GetTerrain();
			applyDisplayOptions(game->gameUI->getStorm3D(), game->getGameScene()->getStormScene(), terrain, game->gameUI->getLightManager());
		} else {
			applyGammaOptions(game->gameUI->getStorm3D());
		}

		if (game->getGameScene() != NULL
			&& game->getGameScene()->getStormScene() != NULL)
		{
			applyCameraOptions(game->getGameScene()->getStormScene());
		}

#ifdef PHYSICS_PHYSX
		if (game->getGamePhysics() != NULL
			&& game->getGamePhysics()->getPhysicsLib() != NULL)
		{
			game->getGamePhysics()->getPhysicsLib()->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_COLLISION_SHAPES, SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_COLLISION_SHAPES));
			game->getGamePhysics()->getPhysicsLib()->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_DYNAMIC, SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_DYNAMIC));
			game->getGamePhysics()->getPhysicsLib()->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_STATIC, SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_STATIC));
			game->getGamePhysics()->getPhysicsLib()->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_COLLISION_CONTACTS, SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_COLLISION_CONTACTS));
			game->getGamePhysics()->getPhysicsLib()->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_FLUIDS, SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_FLUIDS));
			game->getGamePhysics()->getPhysicsLib()->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_CCD, SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_CCD));
		}

		int maxParticles = SimpleOptions::getInt(DH_OPT_I_PHYSICS_MAX_MODEL_PARTICLES);
		int maxSpawnParticles = SimpleOptions::getInt(DH_OPT_I_PHYSICS_MAX_MODEL_PARTICLES_SPAWN_PER_TICK);

		if(game && game->getGameUI() && game->getGameUI()->getVisualEffectManager() && game->getGameUI()->getVisualEffectManager()->getParticleEffectManager())
			game->getGameUI()->getVisualEffectManager()->getParticleEffectManager()->setModelParticleParameters(maxParticles, maxSpawnParticles);
#endif

		if(ogui)
		{
			ogui->setVisualizeWindows(SimpleOptions::getBool(DH_OPT_B_GUI_VISUALIZE_WINDOWS));
		}

	}

}

