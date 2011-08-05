
#include "precompiled.h"

#include "Spotlight.h"
#include "IStorm3D.h"
#include "IStorm3D_Scene.h"
#include "IStorm3D_Terrain.h"
#include "istorm3D_terrain_renderer.h"
#include "IStorm3D_Texture.h"
#include "istorm3d_spotlight.h"
#include <boost/shared_ptr.hpp>

// TEMP
#include <stdlib.h>

#include "../system/Logger.h"
#include "../util/AngleRotationCalculator.h"
#include "../util/PositionsDirectionCalculator.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "igios.h"

namespace ui {

struct SpotlightData
{
	IStorm3D_Terrain &terrain;
	IStorm3D_Scene &scene;
	boost::shared_ptr<IStorm3D_Spotlight> spot;
	boost::shared_ptr<IStorm3D_Spotlight> spot2;
	VC3 currentDirection;
	int spotAmount;
	float fadeRange;
	boost::shared_ptr<IStorm3D_Texture> spotTexture;
	boost::shared_ptr<IStorm3D_Texture> coneTexture;

	bool fakeLight;
	COL fakeLightColor;
	COL fakeLightOriginalColor;
	VC3 fakeLightPosition;
	COL spotlightColor;

	VC3 positionOffset;

	bool spotlightClippingEnabled;

	int effectTimeElapsed;

	SpotTypeProperties::FLASH_TYPE flashType;
	boost::scoped_ptr<IStorm3D_Model> lightModel;

	static std::vector<SpotTypeProperties> spotTypes;
	static int currentlyAddingSpotTypeIndex;

	SpotlightData(IStorm3D &storm, IStorm3D_Terrain &terrain_,
		IStorm3D_Scene &scene_, IStorm3D_Model *originModel,
		const std::string &spottype)
	:	terrain(terrain_),
		scene(scene_),
		fakeLight(false),
		positionOffset(0,0,0),
		spotlightClippingEnabled(true),
		effectTimeElapsed(0),
		flashType(SpotTypeProperties::FLASH_TYPE_NONE)
	{
#ifdef LEGACY_FILES
		// the old hardcoded spotlight types...

		spotAmount = 0;
		spotlightColor = COL(1,1,1);

		if (spottype == "flashlight")
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\flashlight.tga");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			IStorm3D_Texture *coneStormTexture = storm.CreateNewTexture("cone.dds");
			if(coneStormTexture)
			{
				boost::shared_ptr<IStorm3D_Texture> t(coneStormTexture, NullDeleter());
				coneTexture = t;
			}

			spot = terrain.getRenderer().createSpot();
			spot->setProjectionTexture(spotTexture);
			spot->setFov(50.0f);
			spot->setConeFov(40.0f);
			spot->setRange(20.f);
			spot->setType(IStorm3D_Spotlight::Point);

			if(coneTexture)
			{
				spot->setConeTexture(coneTexture);
				spot->setConeMultiplier(0.3f);
			}

			lightModel.reset(storm.CreateNewModel());
			if(lightModel)
			{
				scene.AddModel(lightModel.get());
				lightModel->LoadS3D("Data\\Models\\Effects\\flashlight_flare.s3d");
				lightModel->CastShadows(false);
				lightModel->SetNoCollision(true);
			}

			spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
			spot->enableFeature(IStorm3D_Spotlight::Fade, true);
			spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, true);

#ifdef PROJECT_CLAW_PROTO
			spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, false);
#endif

			spot->setNoShadowModel(originModel);
			spot->setType(IStorm3D_Spotlight::Directional);

			fadeRange = 10;
			spotAmount = 1;

			fakeLight = false;
		} 
		else if(spottype == "environmentlightning") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\lightninglight.tga");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spot = terrain.getRenderer().createSpot();
			spot->setProjectionTexture(spotTexture);
			spot->setFov(80.0f);
			spot->setRange(50.f);
			spot->setType(IStorm3D_Spotlight::Point);
			spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
			spot->enableFeature(IStorm3D_Spotlight::Fade, false);
			spot->enableFeature(IStorm3D_Spotlight::ScissorRect, false);
			spot->setDirection(VC3(-1, -0.5f, 0).GetNormalized());
			spot->setColorMultiplier(COL(1.0f, 1.0f, 1.0f));

			spotAmount = 1;
			fadeRange = 50;
		}
		else if(spottype == "explosion") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\explosionlight.dds");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			fadeRange = 0; //30;

			fakeLight = true;
			fakeLightColor = COL(0,0,0);
			fakeLightOriginalColor = fakeLightColor;
			flashType = SpotTypeProperties::FLASH_TYPE_EXPLOSION;
		}
		else if(spottype == "electric_flow") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\electriclight.tga");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			fadeRange = 0; //30;

			fakeLight = true;
			fakeLightColor = COL(0,0,0);
			fakeLightOriginalColor = fakeLightColor;
			flashType = SpotTypeProperties::FLASH_TYPE_ELECTRIC_FLOW;
		}
		else if(spottype == "flamethrower") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\flamethrowerlight.tga");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			fadeRange = 0; //30;

			fakeLight = true;
			fakeLightColor = COL(0,0,0);
			fakeLightOriginalColor = fakeLightColor;
			flashType = SpotTypeProperties::FLASH_TYPE_FLAMETHROWER;
		}
		else if (spottype == "playerhalo") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			//fadeRange = 25;
			fadeRange = 8;

			fakeLight = true;
			//fakeLightColor = COL(0.3f,0.3f,0.3f);
			fakeLightColor = COL(0.1f,0.1f,0.1f);
			fakeLightOriginalColor = fakeLightColor;
		}
		else if (spottype == "crystalhalo") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			fadeRange = 12;

			fakeLight = true;
			fakeLightColor = COL(0.2f,0.25f,0.3f);
			fakeLightOriginalColor = fakeLightColor;
		}
		else if (spottype == "weaponhalo" || spottype == "healthhalo") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\ItemHalo.tga");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			fadeRange = 3;

			fakeLight = true;
			if (spottype == "weaponhalo")
				fakeLightColor = COL(0.5f,0.7f,0.7f);
			else
				fakeLightColor = COL(0.7f,0.5f,0.5f);
			fakeLightOriginalColor = fakeLightColor;
		}
		else if (spottype == "streetlight") 
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spotAmount = 0;
			fadeRange = 20;

			fakeLight = true;
			fakeLightColor = COL(0.7f,0.7f,0.7f);
			fakeLightOriginalColor = fakeLightColor;
		}
		else if(spottype == "muzzleflash" || spottype == "muzzleflash_low")
		{
			if(game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) <= 50)
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spotAmount = 0;
				fadeRange = 15;

				fakeLight = true;
				fakeLightColor = COL(0.3f,0.3f,0.2f);
				flashType = SpotTypeProperties::FLASH_TYPE_MUZZLE;
			}
			else
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\muzzleflash_light.tga");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spot = terrain.getRenderer().createSpot();
				spot->setProjectionTexture(spotTexture);
				spot->setFov(145.0f);	
				spot->setRange(8.f);
				spot->setClipRange(3.f);

				spot->setType(IStorm3D_Spotlight::Point);

				spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
				spot->enableFeature(IStorm3D_Spotlight::Fade, true);
				spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, false);

				//spot->setNoShadowModel(originModel);
				if (spottype == "muzzleflash_low")
					spot->setColorMultiplier(COL(0.5f, 0.5f, 0.5f));

				fadeRange = 10;
				spotAmount = 1;

				fakeLight = false;
				flashType = SpotTypeProperties::FLASH_TYPE_MUZZLE;
			}
		}
		else if(spottype == "electricflash")
		{
			if(game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) <= 50)
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spotAmount = 0;
				fadeRange = 10;

				fakeLight = true;
				fakeLightColor = COL(0.2f,0.25f,0.4f);
				flashType = SpotTypeProperties::FLASH_TYPE_MUZZLE;
			}
			else
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\electricflash_light.tga");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spot = terrain.getRenderer().createSpot();
				spot->setProjectionTexture(spotTexture);
				spot->setFov(145.0f);	
				spot->setRange(6.f);
				spot->setClipRange(2.5f);

				spot->setType(IStorm3D_Spotlight::Point);

				spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
				spot->enableFeature(IStorm3D_Spotlight::Fade, true);
				fadeRange = 7;
				spotAmount = 1;

				fakeLight = false;
				flashType = SpotTypeProperties::FLASH_TYPE_MUZZLE;
			}
		}
		else if(spottype == "flamerflash")
		{
			if(game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) <= 50)
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spotAmount = 0;
				fadeRange = 10;

				fakeLight = true;
				fakeLightColor = COL(0.3f,0.3f,0.2f);
				flashType = SpotTypeProperties::FLASH_TYPE_FLAMER_MUZZLE;
			}
			else
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\flamerflash_light.tga");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spot = terrain.getRenderer().createSpot();
				spot->setProjectionTexture(spotTexture);
				spot->setFov(145.0f);	
				spot->setRange(6.f);
				spot->setClipRange(2.5f);

				spot->setType(IStorm3D_Spotlight::Point);

				spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
				spot->enableFeature(IStorm3D_Spotlight::Fade, true);
				fadeRange = 7;
				spotAmount = 1;

				fakeLight = false;
				flashType = SpotTypeProperties::FLASH_TYPE_FLAMER_MUZZLE;
			}
		}
		else if(spottype == "muzzleflash_noshadow")
		{
			if(game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) <= 50)
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Lights\\PlayerDefaultVision.dds");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spotAmount = 0;
				fadeRange = 10;

				fakeLight = true;
				fakeLightColor = COL(0.3f,0.3f,0.2f);
				flashType = SpotTypeProperties::FLASH_TYPE_MUZZLE;
			}
			else
			{
				IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\muzzleflash_light.tga");
				if(!texture)
					return;

				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				spot = terrain.getRenderer().createSpot();
				spot->setProjectionTexture(spotTexture);
				spot->setFov(125.0f);	
				spot->setRange(8.f);
				spot->setClipRange(3.f);
				spot->setType(IStorm3D_Spotlight::Point);
				spot->enableFeature(IStorm3D_Spotlight::Shadows, false);
				spot->enableFeature(IStorm3D_Spotlight::Fade, true);

				fadeRange = 10;
				spotAmount = 1;

				fakeLight = false;
				flashType = SpotTypeProperties::FLASH_TYPE_MUZZLE;
			}
		}
		else if (spottype == "sunlight")
		{
			IStorm3D_Texture *texture = storm.CreateNewTexture("Data\\Textures\\Projective_Lights\\lightninglight.tga");
			if(!texture)
				return;

			spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

			spot = terrain.getRenderer().createSpot();
			spot->setProjectionTexture(spotTexture);
			spot->setFov(90.0f);
			spot->setRange(500.f);

			spot->enableFeature(IStorm3D_Spotlight::Shadows, false);
			//spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
			spot->enableFeature(IStorm3D_Spotlight::Fade, false);
			spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, false);
			spot->enableFeature(IStorm3D_Spotlight::ScissorRect, false);

			spot->setType(IStorm3D_Spotlight::Directional);

			spotAmount = 1;

			fakeLight = false;
		} 
		else 
		{
			assert(!"Unknown spotlight type.");
		}
		// TODO: initial direction should be...?
		currentDirection = VC3(0,0,0);
		fakeLightOriginalColor = fakeLightColor;
#else

		bool foundIt = false;
		for (int i = 0; i < (int)spotTypes.size(); i++)
		{
			// TODO: handle low detail version...?
			// (if low detail settings, automagically select the spot that has the lowdetail property name of the 
			// originally matching spot... clearly stated :P)

			if (spotTypes[i].name == spottype)
			{
				SpotTypeProperties &props = spotTypes[i];

				spotAmount = 0;

				if (props.textureFilename.empty())
				{
					LOG_ERROR_W_DEBUG("Spotlight - Spot type has no texture filename.", spottype.c_str());
					// stop here immediately if no texture.
					return;
				}
				IStorm3D_Texture *texture = storm.CreateNewTexture(props.textureFilename.c_str());
				if(texture == NULL)
				{
					LOG_ERROR_W_DEBUG("Spotlight - Failed to load texture.", props.textureFilename.c_str());
					// stop here immediately if texture load fails.
					return;
				}
				spotTexture = boost::shared_ptr<IStorm3D_Texture>(texture, std::mem_fun(&IStorm3D_Texture::Release));

				if (props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_DIRECTIONAL
					|| props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_POINT
					|| props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_FLAT)
				{
					// TODO: support for dual spot effects?
					spotAmount = 1;

					IStorm3D_Texture *coneStormTexture = NULL;
					if (props.coneTextureFilename.empty() 
						&& props.coneAlphaMultiplier != 0.0f)
					{
						LOG_WARNING_W_DEBUG("Spotlight - Spot type has no cone texture filename but non-zero cone alpha multiplier.", spottype.c_str());
					} else {
						if (!props.coneTextureFilename.empty())
						{
							coneStormTexture = storm.CreateNewTexture(props.coneTextureFilename.c_str());
							if(coneStormTexture != NULL)
							{
								boost::shared_ptr<IStorm3D_Texture> t(coneStormTexture, NullDeleter());
								coneTexture = t;
							} else {
								LOG_ERROR_W_DEBUG("Spotlight - Failed to load cone texture.", props.coneTextureFilename.c_str());
							}
						}
					}

					spot = terrain.getRenderer().createSpot();
					spot->setProjectionTexture(spotTexture);
					if(coneTexture)
					{
						spot->setConeTexture(coneTexture);
					}
					spot->setFov(props.fov);
					spot->setConeFov(props.coneFov);
					spot->setRange(props.range);

					if (props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_DIRECTIONAL)
					{
						spot->setType(IStorm3D_Spotlight::Directional);
					}
					else if (props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_POINT)
					{
						spot->setType(IStorm3D_Spotlight::Point);
					}
					else if (props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_FLAT)
					{
						spot->setType(IStorm3D_Spotlight::Flat);
					}

					if (!props.modelFilename.empty())
					{
						lightModel.reset(storm.CreateNewModel());
						if(lightModel)
						{
							scene.AddModel(lightModel.get());
							lightModel->LoadS3D(props.modelFilename.c_str());
							lightModel->CastShadows(false);
							lightModel->SetNoCollision(true);
						}
					}

					if (props.shadows)
						spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
					else
						spot->enableFeature(IStorm3D_Spotlight::Shadows, false);

					if (props.fade)
						spot->enableFeature(IStorm3D_Spotlight::Fade, true);
					else
						spot->enableFeature(IStorm3D_Spotlight::Fade, false);

					if (props.scissor)
						spot->enableFeature(IStorm3D_Spotlight::ScissorRect, true);
					else
						spot->enableFeature(IStorm3D_Spotlight::ScissorRect, false);

					if (coneTexture 
						&& props.coneAlphaMultiplier != 0.0f)
					{
						spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, true);
						spot->setConeMultiplier(props.coneAlphaMultiplier);
					} else {
						spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, false);
					}

					if (props.noShadowFromOrigin)
					{
						spot->setNoShadowModel(originModel);
					}

					spot->setColorMultiplier(props.color);

					// (direction should already be normalized)
					assert((props.direction - props.direction.GetNormalized()).GetSquareLength() < 0.001f);

					spot->setDirection(props.direction);

					this->positionOffset = props.positionOffset;
				}
				else if (props.type == SpotTypeProperties::SPOT_LIGHT_MODEL_TYPE_NONE)
				{
					// fake only. (...or in some very silly case, none at all?)
					if (!props.fakelight)
					{
						LOG_WARNING_W_DEBUG("Spotlight - Spot lighting model is none, and fakelight is not on (probably unintended?).", spottype.c_str());
					}
				} else {
					assert(!"SpotlightData - Invalid spot type.");
				}

				fakeLight = props.fakelight;
				fadeRange = props.fakeFadeRange;
				fakeLightColor = props.fakelightColor;

				flashType = props.flashType;
				if (props.flashType == SpotTypeProperties::FLASH_TYPE_SCRIPT)
				{
					// TODO: ...
					//flashTypeScript = props.flashTypeScript;
					// game->startCustomScriptProcess... ??
				}

				foundIt = true;
				break;
			}
		}

		if (!foundIt)
		{
			LOG_ERROR_W_DEBUG("Spotlight - No spotlight type with given name found.", spottype.c_str());
		}

#endif
	}

	~SpotlightData()
	{
		if(spot)
			terrain.getRenderer().deleteSpot(spot);
		if(spot2 && spotAmount >= 2)
			terrain.getRenderer().deleteSpot(spot2);
	}

	void setSpotPosition(const VC3 &pos)
	{
		if(spot)
		{
			//spot->setPosition(pos);
			VC3 spot1pos = (pos - (currentDirection * 0.15f)) + positionOffset;
			spot->setPosition(spot1pos);

			if(lightModel)
				lightModel->SetPosition((VC3 &) pos);
		}

		if(spot2 && spotAmount >= 2)
		{
			VC3 spot2pos = pos + positionOffset;
			spot2->setPosition(spot2pos);
		}
	}

	void setSpotDir(const VC3 &dir)
	{
		if(spot)
		{
			currentDirection = dir;
			spot->setDirection(currentDirection);

			if(lightModel)
			{
				VC3 y(0.f, 1.f, 0.f);
				VC3 z = dir;
				VC3 x = y.GetCrossWith(z);
				x.Normalize();
				y = -x.GetCrossWith(z);

				MAT m;
				m.Set(0, x.x);
				m.Set(1, x.y);
				m.Set(2, x.z);
				m.Set(4, y.x);
				m.Set(5, y.y);
				m.Set(6, y.z);
				m.Set(8, z.x);
				m.Set(9, z.y);
				m.Set(10, z.z);

				lightModel->SetRotation(m.GetRotation());
			}
		}

		if(spot2 && spotAmount >= 2)
			spot2->setDirection(-currentDirection);
	}
};

void Spotlight::setOriginModel(IStorm3D_Model *originModel)
{
	data->spot->setNoShadowModel(originModel);
}


Spotlight::Spotlight(IStorm3D &storm, IStorm3D_Terrain &terrain,
	IStorm3D_Scene &scene, IStorm3D_Model *originModel,
	const std::string &spottype)
{
	boost::scoped_ptr<SpotlightData> tempData(new SpotlightData(storm, terrain, scene, originModel, spottype));
	data.swap(tempData);
}

Spotlight::~Spotlight()
{
}

void Spotlight::prepareForRender()
{
	if (data->fakeLight)
	{
		IStorm3D_TerrainRenderer &tRend = data->terrain.getRenderer();

		VC3 result = VC3(0,0,0);
		float rhw = 0;
		float real_z = 0;
		IStorm3D_Camera *cam = data->scene.GetCamera();
		bool infront = cam->GetTransformedToScreen(data->fakeLightPosition, result, rhw, real_z);
		if (!infront)
		{
			return;
		}

		float fadeFactor = 0.f;
		float halfRange = 3.f * data->fadeRange * .5f;
		if(real_z < 5)
			fadeFactor = (real_z - 1.5f) / 3.5f;
		else if(real_z > halfRange)
			fadeFactor = 1.f - ((real_z - halfRange) / halfRange);
		else
			fadeFactor = 1.f;
		if(fadeFactor <= 0.f)
			return;

		VC2 ul = VC2(result.x - 0.25f / real_z * data->fadeRange, result.y - 0.25f / real_z * data->fadeRange);
		VC2 dr = VC2(result.x + 0.25f / real_z * data->fadeRange, result.y + 0.25f / real_z * data->fadeRange);
		tRend.renderLightTexture(ul, dr, *data->spotTexture.get(), data->fakeLightColor * fadeFactor);
	}
}

void Spotlight::setFakelightParams(float fadeRange, COL color)
{
	//assert(data->fakeLight);
	data->fadeRange = fadeRange;
	data->fakeLightColor = color;
}

void Spotlight::setFakelightBrightness(float brightness)
{
	assert(data->fakeLight);
	data->fakeLightColor = data->fakeLightOriginalColor * brightness;
}

void Spotlight::setSpotlightParams(COL color)
{
	data->spotlightColor = color;
	if(data->spot)
	{
		data->spot->setColorMultiplier(color);

		if(data->lightModel)
		{
			float alpha = std::min(color.r, color.g);
			alpha = std::min(alpha, color.b);

			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(data->lightModel->ITObject->Begin());
			for(; !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				if(!object)
					continue;

				object->SetForceAlpha(1.f - alpha);
			}
		}
	}

}

void Spotlight::setPosition(const VC3 &position)
{
	if (data->fakeLight)
		data->fakeLightPosition = position;

	if(!data->spot)
		return;

	//data->spot->setPosition(position);
	//if(data->lightModel)
	//	data->lightModel->SetPosition((VC3 &) position);
	data->setSpotPosition(position);

	//if (data->spot2 && data->spotAmount >= 2)
	//	data->spot2->setPosition(position);
}

void Spotlight::setFakePosition(const VC3 &position)
{
	if (data->fakeLight)
		data->fakeLightPosition = position;
}

void Spotlight::setDirection(const VC3 &direction)
{
	if(!data->spot)
		return;

	data->setSpotDir(direction);
	//data->currentDirection = direction;
	//data->spot->setDirection(data->currentDirection);

	//if (data->spot2 && data->spotAmount >= 2)
	//{
	//	data->spot2->setDirection(-data->currentDirection);		
	//}
}

void Spotlight::resetFadeOut(int atTime)
{
	data->effectTimeElapsed = atTime;
}

void Spotlight::fadeOut(int timeElapsed)
{
	data->effectTimeElapsed += timeElapsed;

	if(data->flashType == SpotTypeProperties::FLASH_TYPE_EXPLOSION)
	{
		// TODO: fix the faderange change to absolute values, not to relative
		// (otherwise won't work with resetFadeOut)
		// NOTE: but take care of handling different fadeRanges!!! 
		// (need to store the original faderange value somewhere)


		//data->fadeRange -= 0.05f * (float)timeElapsed;

		if(data->effectTimeElapsed < 100)
		{
			// HACK: halved range
			data->fadeRange += 0.5f * 0.3f * float(timeElapsed);

			float value = timeElapsed * .01f;
			// HACK: halve intensity
			value *= 0.5f;
			data->fakeLightColor += COL(value, value, value);
		}
		else if(data->effectTimeElapsed < 500)
		{
		}
		else
		{
			float value = timeElapsed * .002f;
			// HACK: halve intensity
			value *= 0.5f;
			data->fakeLightColor -= COL(value, value, value);
			data->fakeLightColor.ClampNegative();

			// HACK: halved range
			data->fadeRange -= 0.5f * 0.02f * float(timeElapsed);
			if(data->fadeRange < 1.f)
				data->fadeRange = 1.f;
		}
	}
	else if(data->flashType == SpotTypeProperties::FLASH_TYPE_ELECTRIC_FLOW)
	{
		float value = 0.08f * float(data->effectTimeElapsed) * 0.002f;
		if (value < 0.0f)
			value = 0.0f;
		data->fakeLightColor = COL(value, value, value);
		data->fadeRange = 5.0f;
	}
	else if(data->flashType == SpotTypeProperties::FLASH_TYPE_FLAMETHROWER)
	{
		float value = 0.08f * float(data->effectTimeElapsed) * 0.002f * (float)(rand() % 10) / 10.0f;
		if (value < 0.0f)
			value = 0.0f;
		data->fakeLightColor = COL(value, value, value);
		data->fadeRange = 6.0f;
	}
	else if(data->flashType == SpotTypeProperties::FLASH_TYPE_MUZZLE)
	{
		//if(data->spot)
		{
			float value = 1.0f;
			if(data->effectTimeElapsed > 30)
				value = 1.0f - (float((data->effectTimeElapsed - 30)) * 0.025f);
			if(value < 0.0f) 
				value = 0.0f;

			if(data->spot)
				data->spot->setColorMultiplier(COL(value, value, value));
			else
				setFakelightBrightness(value);
		}

		/*
		if(data->effectTimeElapsed < 100)
		{
			data->fadeRange += 0.1f * float(timeElapsed);

			float value = timeElapsed * .005f;
			data->fakeLightColor += COL(value, value, value);
		}
		else if(data->effectTimeElapsed < 300)
		{
			data->fadeRange -= 0.05f * float(timeElapsed);

			float value = timeElapsed * .0025f;
			data->fakeLightColor -= COL(value, value, value);
		}
		*/
	}
	else if(data->flashType == SpotTypeProperties::FLASH_TYPE_FLAMER_MUZZLE)
	{
		if(data->spot)
		{
			float value = 1.0f;
			if (data->effectTimeElapsed < 100)
			{
				value = 0.5f + float(data->effectTimeElapsed) * 0.01f * 0.5f;
			} else {
				if (data->effectTimeElapsed > 150)
				{
					value = 1.0f - (float(data->effectTimeElapsed-150) * (1.3f / 500.0f));
				}
			}
			value += (float)(rand() % 10) / 50.0f;
			if (value < 0.0f) value = 0.0f;

			// (...too bright...)
			value *= 0.75f;

			data->spot->setColorMultiplier(COL(value, value, value));
		}
	}

	/*
	if(!data->spot)
		return;

	data->spot->setRange(data->fadeRange);

	if (data->spot2 && data->spotAmount >= 2)
	{		
		data->spot2->setRange(data->fadeRange);
	}
	*/
}

void Spotlight::setEnableClip(bool enable)
{
	data->spotlightClippingEnabled = enable;
	data->spot->setEnableClip (enable);
}

void Spotlight::setDirectionToward(const VC3 &direction, int timeElapsed)
{
	if(!data->spot)
		return;

	float timeFactor = float(timeElapsed) / 50.0f;

	// not a very effective solution, but the fastest one to implement..

	VC3 oldDir = VC3(data->currentDirection.x, 0, data->currentDirection.z);
	VC3 newDir = VC3(direction.x, 0, direction.z);

	float currentAngle = util::PositionDirectionCalculator::calculateDirection(VC3(0,0,0), oldDir);
	float preferredAngle = util::PositionDirectionCalculator::calculateDirection(VC3(0,0,0), newDir);
	float rotSpeed = util::AngleRotationCalculator::getFactoredRotationForAngles(currentAngle, preferredAngle, 2.0f);

	float newAngle = currentAngle + rotSpeed * timeFactor;
	if (rotSpeed < 0) newAngle -= 0.01f * timeFactor;
	if (rotSpeed > 0) newAngle += 0.01f * timeFactor;

	// did we turn the flashlight too much?
	// if so, just jump directly to preferred angle.
	while (newAngle >= 360) newAngle -= 360.0f;
	while (newAngle < 0) newAngle += 360.0f;
	float overTurn = util::AngleRotationCalculator::getRotationForAngles(newAngle, preferredAngle, 0.01f);
	if ((overTurn < 0 && rotSpeed > 0)
		|| (overTurn > 0 && rotSpeed < 0))
		newAngle = preferredAngle;
	
	float angleRad = UNIT_ANGLE_TO_RAD(newAngle);
	data->currentDirection = VC3(-sinf(angleRad), -0.3f, -cosf(angleRad));

	/*
	data->spot->setDirection(data->currentDirection);

	if (data->spot2 && data->spotAmount >= 2)
	{
		data->spot2->setDirection(-data->currentDirection);		
	}
	*/
	data->setSpotDir(data->currentDirection);

/*
	// !! psdhax
	if(data->spot)
	{
		COL c(data->currentDirection.x, 0, data->currentDirection.z);
		c.g = c.r + c.b;

		c.r = fabsf(c.r);
		c.g = fabsf(c.g);
		c.b = fabsf(c.b);

		c.Clamp();
		data->spot->setColorMultiplier(c);
	}
*/
}

std::vector<SpotTypeProperties> SpotlightData::spotTypes;
int SpotlightData::currentlyAddingSpotTypeIndex = -1;

void Spotlight::addNewType(const char *spotTypeName)
{
	if (SpotlightData::currentlyAddingSpotTypeIndex != -1)
	{
		LOG_ERROR("Spotlight::addNewType - Attempt to add spot type when previous adding has not been finished.");
		return;
	}

	SpotlightData::currentlyAddingSpotTypeIndex = SpotlightData::spotTypes.size();

	SpotTypeProperties newType;
	newType.name = spotTypeName;
	SpotlightData::spotTypes.push_back(newType);
}

SpotTypeProperties *Spotlight::getAddingSpotTypeProperties()
{
	if (SpotlightData::currentlyAddingSpotTypeIndex == -1)
		return NULL;

	if (SpotlightData::currentlyAddingSpotTypeIndex < 0
		|| SpotlightData::currentlyAddingSpotTypeIndex >= (int)SpotlightData::spotTypes.size())
	{
		LOG_ERROR("Spotlight::getAddingSpotTypeProperties - Internal error, index out of range.");
		assert(!"Spotlight::getAddingSpotTypeProperties - Internal error, index out of range.");
		return NULL;
	}

	return &SpotlightData::spotTypes[SpotlightData::currentlyAddingSpotTypeIndex];
}

void Spotlight::addNewTypeDone()
{
	if (SpotlightData::currentlyAddingSpotTypeIndex == -1)
	{
		LOG_ERROR("Spotlight::addNewTypeDone - Called to finish adding spot type when not currently adding one.");
		return;
	}

	SpotlightData::currentlyAddingSpotTypeIndex = -1;
}

void Spotlight::setRange ( float range )
{
	data->spot->setRange (range);
}

} // ui


