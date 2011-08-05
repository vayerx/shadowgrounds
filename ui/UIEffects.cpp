
#include "precompiled.h"

#include "igios.h"

#include "UIEffects.h"

#include <assert.h>
#include "uidefaults.h"
#include "../game/gamedefs.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "../game/options/options_display.h"
#include "MovieAspectWindow.h"
#include "../system/Timer.h"
#include "../ogui/Ogui.h"
#include "../system/Logger.h"

#include <Storm3D_UI.h>
#include <istorm3D_terrain_renderer.h>

#include "../util/Debug_MemoryManager.h"

// total duration of the effects in msec

#define EXPLOSION_FLASH_EFFECT_DURATION 1000
#define NUKE_EXPLOSION_FLASH_EFFECT_DURATION 1000
#define ENVIRONMENT_LIGHTNING_FLASH_EFFECT_DURATION 500
#define PLAYER_HIT_FLASH_EFFECT_DURATION 1500
#define PLAYER_POISON_FLASH_EFFECT_DURATION 6000

#define LIGHTNING_GUN_FLASH_EFFECT_DURATION 100
//#define LIGHTNING_GUN_FLASH_EFFECT_DURATION 1000

#ifdef LEGACY_FILES
#define PLAYERHIT_WINDOWED_FILE "Data/GUI/Windows/playerhit_windowed.tga"
#else
#define PLAYERHIT_WINDOWED_FILE "data/gui/effect/playerhit_windowed.tga"
#endif


namespace ui
{

	UIEffects::UIEffects(Ogui *ogui, IStorm3D *storm3d)
	{
    this->ogui = ogui;
    this->storm3d = storm3d;
		this->movieWindow = NULL;

		for (int i = 0; i < UIEFFECTS_MASK_PICTURE_LAYERS; i++)
		{
			this->maskWindow[i] = NULL;
			this->maskPosition[i] = UIEFFECTS_MASK_PICTURE_POS_DEFAULT;
			this->maskPositionX[i] = 0;
			this->maskPositionY[i] = 0;
			this->maskSizeX[i] = 1024;
			this->maskSizeY[i] = 768;

			this->maskFont[i] = NULL;
			this->maskTextButton[i] = NULL;
			this->maskText[i] = NULL;
			this->maskTextPositionX[i] = 0;
			this->maskTextPositionY[i] = 0;
			this->maskTextAreaSizeX[i] = 1024;
			this->maskTextAreaSizeY[i] = 768;
		}

		this->activeMaskLayer = 0;

		this->fadeWindow = NULL;
		this->fadeImageFilename = NULL;
		
		setDefaultFadeImage();

    this->flashEffectRunning = false;
    this->flashStartTime = 0;
    this->flashEffectType = FLASH_EFFECT_TYPE_NONE;

		this->fadeTimeLeft = 0;
		this->fadeTotalTime = 0;
		this->fadingIn = false;
		this->fadingOut = false;

		for(int i = 0; i < NUM_FILTER_EFFECTS; i++)
		{
			filterEffectEnabled[i] = false;
		}
	}


	UIEffects::~UIEffects()
	{
		setNoFade();
		if (fadeImageFilename != NULL)
		{
			delete [] fadeImageFilename;
		}
		if (movieWindow != NULL)
		{
			delete movieWindow;
		}
		for (int i = 0; i < UIEFFECTS_MASK_PICTURE_LAYERS; i++)
		{
			if (maskTextButton[i] != NULL)
			{
				delete maskTextButton[i];
			}
			if (maskText[i] != NULL)
			{
				delete [] maskText[i];
			}
			if (maskFont[i] != NULL)
			{
				delete maskFont[i];
			}
			if (maskWindow[i] != NULL)
			{
				delete maskWindow[i];
			}
		}
	}

	void UIEffects::setDefaultFadeImageIfHitImage()
	{
		// restores the default fade-in/out image, if this is currently the hit
		// effect image
		if (this->fadeImageFilename != NULL
			&& strcmp(this->fadeImageFilename, PLAYERHIT_WINDOWED_FILE) == 0)
		{
			setDefaultFadeImage();
		}
	}


	void UIEffects::setDefaultFadeImage()
	{
#ifdef LEGACY_FILES
		setFadeImageFilename("Data/GUI/Windows/faded.dds");
#else
		setFadeImageFilename("data/gui/effect/faded.tga");
#endif
	}

	void UIEffects::setFadeImageFilename(const char *fadeImageFilename)
	{
		assert(fadeImageFilename != NULL);
		if (this->fadeImageFilename != NULL
			&& strcmp(this->fadeImageFilename, fadeImageFilename) == 0)
		{
			// no need to change image, as we are already using that image. just return.
			return;
		}

		if (this->fadeImageFilename != NULL)
		{
			delete [] this->fadeImageFilename;
			this->fadeImageFilename = NULL;		
		}
		if (fadeImageFilename != NULL)
		{
			this->fadeImageFilename = new char[strlen(fadeImageFilename) + 1];
			strcpy(this->fadeImageFilename, fadeImageFilename);
		} else {
			assert(!"UIEffects::setFadeImageFilename - Attempt to set null filename for fade image.");
		}
	}


	void UIEffects::setGammaEffect(float brightness, float contrast, float redCorrection,
		float greenCorrection, float blueCorrection, IStorm3D_TerrainRenderer *terrainRenderer)
	{
		for(int i = 0; i < NUM_FILTER_EFFECTS; i++)
		{
			if(!filterEffectEnabled[i]) continue;

			switch(i)
			{
				case FILTER_EFFECT_NVGOGGLE:
				{
					brightness *= 2.0f;
					contrast *= 2.0f;
					redCorrection *= 0.5f;
					blueCorrection *= 0.5f;
					greenCorrection *= 1.0f;
				}
				break;

				case FILTER_EFFECT_SLOMO:
				case FILTER_EFFECT_SLOMO_ENDING:
				{
					int delta = (Timer::getUnfactoredTime() - slomoEffectStartTime);
					float timeFactor = 1.0f;
					if(i == FILTER_EFFECT_SLOMO_ENDING)
					{
						if(delta < 1000)
						{
							timeFactor = 1 - (delta / 1000.0f);
						}
						else
						{
							timeFactor = 0.0f;
							filterEffectEnabled[i] = false;
						}
					}
					else
					{
						if(delta < 500)
							timeFactor = delta / 500.0f;
						else
							timeFactor = 1.0f;
					}

					brightness += 0.5f * timeFactor;
					contrast += 1.0f * timeFactor;
					redCorrection += 0.1f * timeFactor;
					greenCorrection += 0.1f * timeFactor;
				}
				break;

				case FILTER_EFFECT_NAPALM:
				{
					int delta = (Timer::getTime() - napalmEffectStartTime);
					float timeFactor = 0.0f;
					if (delta < 300)
					{
						timeFactor = (delta / 300.0f);
					}
					else if(delta < 700)
					{
						timeFactor = 1.0f - ((delta - 300) / 400.0f);
					}
					else
					{
						filterEffectEnabled[i] = false;
					}

					brightness += 1.0f * timeFactor;
					contrast += 2.0f * timeFactor;
					redCorrection += 0.5f * timeFactor;
					greenCorrection += 0.2f * timeFactor;
				}
				break;

				case FILTER_EFFECT_HEAL:
				{
					int delta = (Timer::getTime() - healEffectStartTime);
					float timeFactor = 0.0f;
					if (delta < 50)
					{
						timeFactor = (delta / 50.0f);
					}
					else if(delta < 600)
					{
						timeFactor = 1.0f - ((delta - 50.0f) / 550.0f);
					}
					else
					{
						filterEffectEnabled[i] = false;
					}

					if(filterEffectEnabled[FILTER_EFFECT_SLOMO] || filterEffectEnabled[FILTER_EFFECT_SLOMO_ENDING])
					{
						// darken in slomo
						brightness -= 0.2f * timeFactor;
						contrast -= 0.3f * timeFactor;
						blueCorrection += 0.25f * timeFactor;
					}
					else
					{
						brightness += 0.2f * timeFactor;
						contrast += 0.3f * timeFactor;
						blueCorrection += 0.1f * timeFactor;
					}
				}
				break;

				case FILTER_EFFECT_CUSTOM:
				case FILTER_EFFECT_CUSTOM_ENDING:
				{
					int delta = (Timer::getUnfactoredTime() - customEffectStartTime);
					float timeFactor = 1.0f;
					if(i == FILTER_EFFECT_CUSTOM_ENDING)
					{
						if(delta < 1000)
						{
							timeFactor = 1 - (delta / 1000.0f);
						}
						else
						{
							timeFactor = 0.0f;
							filterEffectEnabled[i] = false;
						}
					}
					else
					{
						if(delta < 1000)
							timeFactor = delta / 1000.0f;
						else
							timeFactor = 1.0f;
					}

					brightness += customEffectBrightness * 0.01f * timeFactor;
					contrast += customEffectContrast * 0.01f * timeFactor;
					redCorrection += customEffectRed * 0.01f * timeFactor;
					greenCorrection += customEffectGreen * 0.01f * timeFactor;
					blueCorrection += customEffectBlue * 0.01f * timeFactor;
				}
				break;

				default:
					break;
			}
		}



		brightness -= 1.0f;
		contrast -= 1.0f;
		redCorrection -= 1.0f;
		greenCorrection -= 1.0f;
		blueCorrection -= 1.0f;

		if (game::SimpleOptions::getBool(DH_OPT_B_SHADER_GAMMA_EFFECTS))
		{
			if (terrainRenderer != NULL && game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) > 0)
				terrainRenderer->setColorEffect(contrast, brightness, COL(redCorrection, greenCorrection, blueCorrection));
		} else {
      float opt_gamma = game::SimpleOptions::getFloat(DH_OPT_F_GAMMA);
      float opt_brightness = game::SimpleOptions::getFloat(DH_OPT_F_BRIGHTNESS);
      float opt_contrast = game::SimpleOptions::getFloat(DH_OPT_F_CONTRAST);
      float opt_red_correction = game::SimpleOptions::getFloat(DH_OPT_F_RED_CORRECTION);
      float opt_green_correction = game::SimpleOptions::getFloat(DH_OPT_F_GREEN_CORRECTION);
      float opt_blue_correction = game::SimpleOptions::getFloat(DH_OPT_F_BLUE_CORRECTION);

      //bool calibrate = game::SimpleOptions::getBool(DH_OPT_B_CALIBRATE_GAMMA);
      bool calibrate = false;

	    storm3d->SetGammaRamp(opt_gamma, opt_brightness + brightness, opt_contrast + contrast, 
		    opt_red_correction + redCorrection, opt_green_correction + greenCorrection, opt_blue_correction + blueCorrection, calibrate);
		}
	}


	void UIEffects::run(int msec, IStorm3D_TerrainRenderer *terrainRenderer, IStorm3D_Camera *stormCamera)
	{
		if (movieWindow != NULL)
		{
			movieWindow->update();
		}
		for (int i = 0; i < UIEFFECTS_MASK_PICTURE_LAYERS; i++)
		{
			if (maskWindow[i] != NULL)
			{
				maskWindow[i]->Raise();
			}
		}
		if (fadeWindow != NULL)
		{
			fadeWindow->Raise();
			if (fadeTimeLeft > 0)
			{
				fadeTimeLeft -= msec;
				if (fadeTimeLeft <= 0)
				{
					fadeTimeLeft = 0;
					if (fadingOut)
					{
						setFaded();
						fadeWindow->SetTransparency(0);
					}
					else
					{
						setNoFade();
					}
				} else {
					if (fadingOut)
					{
						fadeWindow->SetTransparency(fadeTimeLeft * 100 / fadeTotalTime);
					}
					else if (fadingIn)
					{
						fadeWindow->SetTransparency((fadeTotalTime - fadeTimeLeft) * 100 / fadeTotalTime);
					} else {
						assert(!"Fade time exists, but not fading in or out.");
					}
				}
			}
		}

		// find if any filter effect is enabled
		bool hasFilterEffect = false;
		for(int i = 0; i < NUM_FILTER_EFFECTS; i++)
		{
			if(filterEffectEnabled[i])
			{
				hasFilterEffect = true;
				break;
			}
		}

    if (flashEffectRunning || hasFilterEffect)
    {
      // TODO: move this to another method.
      // Timer::update();
  
      bool effectOver = true;
      int ctime = Timer::getTime() - flashStartTime;

      float brightness = 1.0f;
      float contrast = 1.0f;
      float red_correction = 1.0f;
      float green_correction = 1.0f;
      float blue_correction = 1.0f;
			if(hasFilterEffect)
			{
				this->setGammaEffect(brightness, contrast, red_correction, green_correction, blue_correction, terrainRenderer);
				if(!flashEffectRunning) return;
			}

			switch(flashEffectType)
      {
        case FLASH_EFFECT_TYPE_ENVIRONMENT_LIGHTNING:
          if (ctime < ENVIRONMENT_LIGHTNING_FLASH_EFFECT_DURATION)
          {
            // atmospheric lightning
            if (ctime < 200
              || (ctime >= 400 && ctime < 500 && (rand() % 5) == 0))
            {
              brightness += 0.4f;
              contrast += 0.6f;
              blue_correction += 0.2f + 0.3f * (rand() / float(RAND_MAX));
            }
						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        case FLASH_EFFECT_TYPE_EXPLOSION:
          if (ctime < EXPLOSION_FLASH_EFFECT_DURATION)
          {
            float timeFactor;
            if (ctime < (EXPLOSION_FLASH_EFFECT_DURATION / 3))
            {
              timeFactor = (ctime / float(EXPLOSION_FLASH_EFFECT_DURATION / 3));
            } else {
              timeFactor = 1.0f - ((ctime - EXPLOSION_FLASH_EFFECT_DURATION / 3) / float(EXPLOSION_FLASH_EFFECT_DURATION * 2 / 3));
            }

            // nuke
            brightness += 0.5f * timeFactor;
            contrast += 1.0f * timeFactor;
            red_correction += 0.1f * timeFactor;
            green_correction += 0.1f * timeFactor;

						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        case FLASH_EFFECT_TYPE_NUKE_EXPLOSION:
          if (ctime < NUKE_EXPLOSION_FLASH_EFFECT_DURATION)
          {
            float timeFactor;
            if (ctime < (NUKE_EXPLOSION_FLASH_EFFECT_DURATION / 3))
            {
              timeFactor = (ctime / float(NUKE_EXPLOSION_FLASH_EFFECT_DURATION / 3));
            } else {
              timeFactor = 1.0f - ((ctime - NUKE_EXPLOSION_FLASH_EFFECT_DURATION / 3) / float(NUKE_EXPLOSION_FLASH_EFFECT_DURATION * 2 / 3));
            }

            // nuke
            brightness += 1.0f * timeFactor;
            contrast += 2.0f * timeFactor;
            red_correction += 0.5f * timeFactor;
            green_correction += 0.2f * timeFactor;
        
						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        case FLASH_EFFECT_TYPE_LIGHTNING_GUN:
          if (ctime < LIGHTNING_GUN_FLASH_EFFECT_DURATION)
          {
            float timeFactor;
            timeFactor = 1.0f - (ctime / float(LIGHTNING_GUN_FLASH_EFFECT_DURATION));

            // lightning gun?
            brightness += 0.1f * timeFactor;
            contrast += 0.3f * timeFactor;
            blue_correction += (0.2f + 0.3f * (rand() / float(RAND_MAX))) * timeFactor;

            //float peakFactor = 0.8f - (0.8f * ((Timer::getTime() % 2000) / float(2000)));
            //storm3d->SetGammaPeak(true, peakFactor, 0.01f, 0.01f, 0.0f, 0.0f, 1.5f * peakFactor);

						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        case FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL:
        case FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM:
        case FLASH_EFFECT_TYPE_PLAYER_HIT_BIG:
          if (ctime < PLAYER_HIT_FLASH_EFFECT_DURATION)
          {
            //float timeFactor = 1.0f - ((float)ctime / PLAYER_HIT_FLASH_EFFECT_DURATION);
            float timeFactor;
            if (ctime < (PLAYER_HIT_FLASH_EFFECT_DURATION / 4))
            {
              timeFactor = (ctime / float(PLAYER_HIT_FLASH_EFFECT_DURATION / 4));
            } else {
              timeFactor = 1.0f - ((ctime - PLAYER_HIT_FLASH_EFFECT_DURATION / 4) / float(PLAYER_HIT_FLASH_EFFECT_DURATION * 3 / 4));
            }

            // player hit(s)
            //brightness -= 0.2f * timeFactor;
						if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL)
						{
							contrast -= 0.2f * timeFactor;
							red_correction += 0.1f * timeFactor;
							green_correction -= 0.3f * timeFactor;
							blue_correction -= 0.3f * timeFactor;
						}
						else if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM)
						{
							contrast -= 0.4f * timeFactor;
							red_correction += 0.2f * timeFactor;
							green_correction -= 0.5f * timeFactor;
							blue_correction -= 0.5f * timeFactor;
						}
						else
						{
							contrast -= 0.6f * timeFactor;
							red_correction += 0.3f * timeFactor;
							green_correction -= 0.7f * timeFactor;
							blue_correction -= 0.7f * timeFactor;
						}

						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        case FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL:
        case FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM:
        case FLASH_EFFECT_TYPE_PLAYER_POISON_BIG:
          if (ctime < PLAYER_POISON_FLASH_EFFECT_DURATION)
          {
            //float timeFactor = 1.0f - ((float)ctime / PLAYER_HIT_FLASH_EFFECT_DURATION);
            float timeFactor;
            if (ctime < (PLAYER_POISON_FLASH_EFFECT_DURATION / 4))
            {
              timeFactor = (ctime / float(PLAYER_POISON_FLASH_EFFECT_DURATION / 4));
            } else {
              timeFactor = 1.0f - ((ctime - PLAYER_POISON_FLASH_EFFECT_DURATION / 4) / float(PLAYER_POISON_FLASH_EFFECT_DURATION * 3 / 4));
            }

            // player poisoned
						if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL)
						{
							//brightness -= 0.1f * timeFactor;
							//contrast += 0.2f * timeFactor;
							red_correction -= 0.4f * timeFactor;
							green_correction += 0.1f * timeFactor;
							blue_correction -= 0.4f * timeFactor;
						}
						else if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM)
						{
							//brightness -= 0.2f * timeFactor;
							//contrast += 0.4f * timeFactor;
							red_correction -= 0.7f * timeFactor;
							green_correction += 0.2f * timeFactor;
							blue_correction -= 0.7f * timeFactor;
						}
						else
						{
							//brightness -= 0.3f * timeFactor;
							//contrast += 0.6f * timeFactor;
							red_correction -= 1.0f * timeFactor;
							green_correction += 0.3f * timeFactor;
							blue_correction -= 1.0f * timeFactor;
						}

						// TODO: some new option to tell if shear effect is used.
						stormCamera->SetShearEffectFactor(timeFactor * 0.5f);

						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        case FLASH_EFFECT_TYPE_CLUSTER_EXPLOSION:
          if (ctime < 300)
          {
            float timeFactor;
            if (ctime < 50)
            {
              timeFactor = (ctime / float(50));
            } else {
              timeFactor = 1.0f - ((ctime - 50) / float(250));
            }

            // nuke
            brightness += 0.4f * timeFactor;
            contrast += 0.7f * timeFactor;
            red_correction += 0.025f * timeFactor;
            green_correction += 0.025f * timeFactor;

						this->setGammaEffect(brightness, contrast, red_correction, green_correction,
							blue_correction, terrainRenderer);

            effectOver = false;
          }
          break;

        default:
          assert(!"Unsupported flasheffect type.");
          break;
      }

      if (effectOver) 
      {
        // FIXME: this won't do, if support for multiple simultaneous 
        // flash effects is added.
        endAllFlashEffects(terrainRenderer);

				// TODO: call only if shear was used. (- not really important though)
				stormCamera->SetShearEffectFactor(0.0f);

				if(hasFilterEffect)
				{
					this->setGammaEffect(brightness, contrast, red_correction, green_correction, blue_correction, terrainRenderer);
				}
      }
    }
	}

	void UIEffects::startFlashEffect(FLASH_EFFECT_TYPE flashEffectType)
	{
    // TODO: support for multiple flash effects at same time

		// check if weather effects are on, if this is lightning.
		if (!game::SimpleOptions::getBool(DH_OPT_B_WEATHER_EFFECTS)
			&& flashEffectType == FLASH_EFFECT_TYPE_ENVIRONMENT_LIGHTNING)
		{ 
			return;
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_EXTRA_GAMMA_EFFECTS))
		{
			bool wasHitEffect = 
				this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL
				|| this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM
				|| this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_BIG;
			if (!flashEffectRunning)
				wasHitEffect = false;

			bool wasPoisonEffect = 
				this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL
				|| this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM
				|| this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_BIG;
			if (!flashEffectRunning)
				wasPoisonEffect = false;

			bool isHitEffect = 
				flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_BIG;

			bool isPoisonEffect = 
				flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_BIG;

			// NOTE: environment lightning is overridden by all others
			// NOTE: player hit effect overrides all others

			if (this->flashEffectType == FLASH_EFFECT_TYPE_ENVIRONMENT_LIGHTNING
				|| !flashEffectRunning
				|| (this->flashEffectType == FLASH_EFFECT_TYPE_LIGHTNING_GUN
				&& flashEffectType == FLASH_EFFECT_TYPE_LIGHTNING_GUN)
				|| isHitEffect || 
				(isPoisonEffect && (!wasPoisonEffect || (Timer::getTime() - flashStartTime) > PLAYER_POISON_FLASH_EFFECT_DURATION * 6 / 8)))
				//|| (wasHitEffect && isHitEffect)
				//|| (!wasHitEffect) && isHitEffect)
			{
				if (!isHitEffect
					|| !game::SimpleOptions::getBool(DH_OPT_B_HIT_EFFECT_IMAGE))
				{
					if (wasPoisonEffect)
					{
						if (this->flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL
							&& flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL)
						{
							flashEffectType = FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM;
						}
						else if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM)
						{
							flashEffectType = FLASH_EFFECT_TYPE_PLAYER_POISON_BIG;
						}
					}
					Timer::update();
					flashStartTime = Timer::getTime();
					flashEffectRunning = true;
					this->flashEffectType = flashEffectType;
					// HACK: multiple hit effects start from max effect level
					if (isHitEffect && wasHitEffect)
					{
						flashStartTime -= PLAYER_HIT_FLASH_EFFECT_DURATION / 4;
					}
					//if (isPoisonEffect && wasPoisonEffect)
					//{
					//	flashStartTime -= PLAYER_POISON_FLASH_EFFECT_DURATION / 12;
					//}
				}
			}

			if (isHitEffect)
			{
				if (game::SimpleOptions::getBool(DH_OPT_B_HIT_EFFECT_IMAGE))
				{
					if (this->fadeWindow == NULL 
						|| (this->fadeImageFilename != NULL
						&& strcmp(this->fadeImageFilename, PLAYERHIT_WINDOWED_FILE) == 0))
					{
						this->setFadeImageFilename(PLAYERHIT_WINDOWED_FILE);
						this->setNoFade();
						this->startFadeIn(PLAYER_HIT_FLASH_EFFECT_DURATION);
						if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL)
							this->fadeTimeLeft = int(this->fadeTimeLeft / 3);
						else if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM)
							this->fadeTimeLeft = int(this->fadeTimeLeft / 2);
						else if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_BIG)
							this->fadeTimeLeft = int(this->fadeTimeLeft / 1.5f);
					}
				}
			}
		}
  }

	void UIEffects::endAllFlashEffects(IStorm3D_TerrainRenderer *terrainRenderer)
	{
    if (flashEffectRunning)
    {
      flashEffectRunning = false;

			if (flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_HIT_BIG
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM
				|| flashEffectType == FLASH_EFFECT_TYPE_PLAYER_POISON_BIG)
			{
				if (game::SimpleOptions::getBool(DH_OPT_B_WINDOWED))
				{
					if(this->fadeImageFilename != NULL && strcmp(this->fadeImageFilename, PLAYERHIT_WINDOWED_FILE) == 0)
					{
						this->setNoFade();
						this->setDefaultFadeImage();
					}
				}
			}

			if (game::SimpleOptions::getBool(DH_OPT_B_SHADER_GAMMA_EFFECTS))
			{
				if (terrainRenderer != NULL && game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) > 0)
				{
					terrainRenderer->setColorEffect(0.0f, 0.0f, COL(0.0f, 0.0f, 0.0f));
				}
			} else {
				float gamma = game::SimpleOptions::getFloat(DH_OPT_F_GAMMA);
				float brightness = game::SimpleOptions::getFloat(DH_OPT_F_BRIGHTNESS);
				float contrast = game::SimpleOptions::getFloat(DH_OPT_F_CONTRAST);
				float red_correction = game::SimpleOptions::getFloat(DH_OPT_F_RED_CORRECTION);
				float green_correction = game::SimpleOptions::getFloat(DH_OPT_F_GREEN_CORRECTION);
				float blue_correction = game::SimpleOptions::getFloat(DH_OPT_F_BLUE_CORRECTION);
				bool calibrate = game::SimpleOptions::getBool(DH_OPT_B_CALIBRATE_GAMMA);

				//storm3d->SetGammaPeak(false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
				storm3d->SetGammaRamp(gamma, brightness, contrast, 
					red_correction, green_correction, blue_correction, calibrate);
			}
		}
  }

	void UIEffects::setFaded()
	{
		if (fadeWindow == NULL)
		{
			fadeWindow = ogui->CreateSimpleWindow(0, 0, 1024, 768, this->fadeImageFilename);
		}
		fadeWindow->Raise();
		fadeWindow->SetTransparency(0);
		this->fadeTimeLeft = 0;
		this->fadeTotalTime = 0;
		this->fadingIn = false;
		this->fadingOut = false;
	}

	void UIEffects::setNoFade()
	{
		if (fadeWindow != NULL)
		{
			delete fadeWindow;
			fadeWindow = NULL;
		}		
		this->fadeTimeLeft = 0;
		this->fadeTotalTime = 0;
		this->fadingIn = false;
		this->fadingOut = false;
	}


	void UIEffects::startFadeIn(int msecDuration)
	{
		// TODO: if already fading in/out, smooth transition, not just 
		// immediate change
		if (msecDuration <= GAME_TICK_MSEC)
		{
			Logger::getInstance()->warning("UIEffects::startFadeIn - Given fade-in duration too small.");
			msecDuration = GAME_TICK_MSEC;
		}
		setFaded();
		fadeWindow->SetTransparency(0);
		this->fadeTimeLeft = msecDuration;
		this->fadeTotalTime = msecDuration;
		this->fadingIn = true;
		this->fadingOut = false;
	}


	void UIEffects::startFadeOutIfNotFaded(int msecDuration)
	{
		if (this->fadeWindow == NULL)
		{
			startFadeOut(msecDuration);
		}
	}


	void UIEffects::startFadeOut(int msecDuration)
	{
		// TODO: if already fading in/out, smooth transition, not just 
		// immediate change
		if (msecDuration <= GAME_TICK_MSEC)
		{
			Logger::getInstance()->warning("UIEffects::startFadeOut - Given fade-out duration too small.");
			msecDuration = GAME_TICK_MSEC;
		}
		setFaded();
		fadeWindow->SetTransparency(100);
		this->fadeTimeLeft = msecDuration;
		this->fadeTotalTime = msecDuration;
		this->fadingIn = false;
		this->fadingOut = true;
	}


	void UIEffects::setMovieAspectRatio(bool movie)
	{
		if (movie)
		{
			if (movieWindow == NULL)
			{
				movieWindow = new MovieAspectWindow(ogui);
				// keep the mask pictures on top.
				for (int i = 0; i < UIEFFECTS_MASK_PICTURE_LAYERS; i++)
				{
					if (maskWindow[i] != NULL)
					{
						maskWindow[i]->Raise();
					}
				}
			}
		} else {
			if (movieWindow != NULL)
			{
				delete movieWindow;
				movieWindow = NULL;
			}
		}
	}


	void UIEffects::setActiveMaskPictureLayer(int layer)
	{
		if (layer >= 0 && layer < UIEFFECTS_MASK_PICTURE_LAYERS)
		{
			this->activeMaskLayer = layer;
		} else {
			Logger::getInstance()->error("UIEffects::setActiveMaskPictureLayer - Layer number out of bounds.");
		}
	}


	void UIEffects::setMaskPicture(const char *filename)
	{
		clearMaskPictureText();
		if (maskWindow[activeMaskLayer] != NULL)
		{
			delete maskWindow[activeMaskLayer];
			maskWindow[activeMaskLayer] = NULL;
		}
		int sizex = maskSizeX[activeMaskLayer];
		int sizey = maskSizeY[activeMaskLayer];
		maskWindow[activeMaskLayer] = ogui->CreateSimpleWindow(0, 0, sizex, sizey, filename);
		maskWindow[activeMaskLayer]->SetMoveBoundaryType(OguiWindow::MOVE_BOUND_NO_PART_IN_SCREEN);
		maskWindow[activeMaskLayer]->SetReactMask(0);
		maskWindow[activeMaskLayer]->SetUnmovable();
		maskWindow[activeMaskLayer]->Raise();
		maskWindow[activeMaskLayer]->SetEffectListener(this);
		setMaskWindowPos();
	}


	bool UIEffects::isMaskPicture()
	{
		if (maskWindow[activeMaskLayer] != NULL)
		{
			return true;
		} else {
			return false;
		}
	}


	void UIEffects::clearAllMaskPictures()
	{
		int prevActive = activeMaskLayer;
		for (int i = 0; i < UIEFFECTS_MASK_PICTURE_LAYERS; i++)
		{
			activeMaskLayer = i;
			clearMaskPicture();
			clearMaskPictureText();
		}
		activeMaskLayer = prevActive;
	}


	void UIEffects::clearMaskPicture()
	{
		clearMaskPictureText();
		if (maskWindow[activeMaskLayer] != NULL)
		{
			delete maskWindow[activeMaskLayer];
			maskWindow[activeMaskLayer] = NULL;
		}
		maskPosition[activeMaskLayer] = UIEFFECTS_MASK_PICTURE_POS_DEFAULT;
		maskSizeX[activeMaskLayer] = 1024;
		maskSizeY[activeMaskLayer] = 768;
	}


	void UIEffects::setMaskPicturePosition(int position)
	{
		// TODO: check value limits.
		if (position >= 0 && position < UIEFFECTS_MASK_PICTURE_POS_AMOUNT)
		{
			maskPosition[activeMaskLayer] = position;
			setMaskWindowPos();
		} else {
			Logger::getInstance()->error("UIEffects::setMaskPicturePosition - Position number out of bounds.");
		}
	}

	void UIEffects::setMaskPicturePositionX(int x)
	{
		maskPositionX[activeMaskLayer] = x;
		if (maskWindow[activeMaskLayer] == NULL) return;
		maskWindow[activeMaskLayer]->MoveTo(maskPositionX[activeMaskLayer], maskPositionY[activeMaskLayer]);
	}

	void UIEffects::setMaskPicturePositionY(int y)
	{
		maskPositionY[activeMaskLayer] = y;
		if (maskWindow[activeMaskLayer] == NULL) return;
		maskWindow[activeMaskLayer]->MoveTo(maskPositionX[activeMaskLayer], maskPositionY[activeMaskLayer]);
	}

	void UIEffects::setMaskPictureSizeX(int sizex)
	{
		maskSizeX[activeMaskLayer] = sizex;
		if (maskWindow[activeMaskLayer] != NULL)
			maskWindow[activeMaskLayer]->Resize(maskSizeX[activeMaskLayer], maskSizeY[activeMaskLayer]);
		maskTextAreaSizeX[activeMaskLayer] = sizex;
	}


	void UIEffects::setMaskPictureSizeY(int sizey)
	{
		maskSizeY[activeMaskLayer] = sizey;
		if (maskWindow[activeMaskLayer] != NULL)
			maskWindow[activeMaskLayer]->Resize(maskSizeX[activeMaskLayer], maskSizeY[activeMaskLayer]);
		maskTextAreaSizeX[activeMaskLayer] = sizey;
	}


	void UIEffects::setMaskWindowPos()
	{
		if (maskWindow[activeMaskLayer] == NULL) return;

		int x = (1024 - maskSizeX[activeMaskLayer]) / 2;
		int y = (768 - maskSizeY[activeMaskLayer]) / 2;
		switch (maskPosition[activeMaskLayer])
		{
			case UIEFFECTS_MASK_PICTURE_POS_UPPER_LEFT:
				x = 0;
				y = 0;
				break;
			case UIEFFECTS_MASK_PICTURE_POS_UPPER_CENTER:
				y = 0;
				break;
			case UIEFFECTS_MASK_PICTURE_POS_UPPER_RIGHT:
				x = 1024 - maskSizeX[activeMaskLayer];
				y = 0;
				break;
			case UIEFFECTS_MASK_PICTURE_POS_MIDDLE_LEFT:
				x = 0;
				break;
			case UIEFFECTS_MASK_PICTURE_POS_MIDDLE_RIGHT:
				x = 1024 - maskSizeX[activeMaskLayer];
				break;
			case UIEFFECTS_MASK_PICTURE_POS_LOWER_LEFT:
				x = 0;
				y = 768 - maskSizeY[activeMaskLayer];
				break;
			case UIEFFECTS_MASK_PICTURE_POS_LOWER_CENTER:
				y = 0;
				y = 768 - maskSizeY[activeMaskLayer];
				break;
			case UIEFFECTS_MASK_PICTURE_POS_LOWER_RIGHT:
				x = 1024 - maskSizeX[activeMaskLayer];
				y = 768 - maskSizeY[activeMaskLayer];
				break;
			case UIEFFECTS_MASK_PICTURE_POS_XY_COORDINATES:
				x = maskPositionX[activeMaskLayer];
				y = maskPositionY[activeMaskLayer];
				break;
			default:
				// nop
				break;
		}

		maskWindow[activeMaskLayer]->MoveTo(x, y);
	}


	void UIEffects::setMaskPictureFont(const char *font)
	{
		assert(font != NULL);

		if (maskFont[activeMaskLayer] != NULL)
		{
			delete maskFont[activeMaskLayer];
			maskFont[activeMaskLayer] = NULL;
		}
		maskFont[activeMaskLayer] = ogui->LoadFont(font);
		if (maskTextButton[activeMaskLayer] != NULL
			&& maskFont[activeMaskLayer] != NULL)
		{
			maskTextButton[activeMaskLayer]->SetFont(maskFont[activeMaskLayer]);
		}
	}


	void UIEffects::setMaskPictureText(const char *text, OguiButton::TEXT_H_ALIGN align )
	{
		assert(text != NULL);
		if (text == NULL)
		{
			return;
		}
		if (maskWindow[activeMaskLayer] == NULL)
		{
			Logger::getInstance()->warning("UIEffects::setMaskPictureText - Cannot set mask picture text when no picture set.");
			assert(!"Cannot set mask picture text when no picture set.");
			return;
		}
		if (maskTextButton[activeMaskLayer] == NULL)
		{
			IOguiFont *font;
			if (maskFont[activeMaskLayer] != NULL)
				font = maskFont[activeMaskLayer];
			else
				font = ui::defaultFont;
			maskTextButton[activeMaskLayer] = ogui->CreateTextArea(maskWindow[activeMaskLayer], maskTextPositionX[activeMaskLayer], maskTextPositionY[activeMaskLayer], maskTextAreaSizeX[activeMaskLayer], maskTextAreaSizeY[activeMaskLayer], text);
			maskTextButton[activeMaskLayer]->SetLinebreaks(true);
			maskTextButton[activeMaskLayer]->SetFont(maskFont[activeMaskLayer]);
			maskTextButton[activeMaskLayer]->SetTextHAlign( align );
		} else {
			maskTextButton[activeMaskLayer]->SetText(text);
		}

		if (maskText[activeMaskLayer] != NULL)
		{
			delete [] maskText[activeMaskLayer];
		}
		char *newText = new char[strlen(text) + 1];
		maskText[activeMaskLayer] = newText;
		strcpy(maskText[activeMaskLayer], text);

	}


	void UIEffects::addMaskPictureTextLine(const char *textline)
	{
		assert(textline != NULL);
		if (textline == NULL)
		{
			return;
		}
		if (maskText[activeMaskLayer] == NULL
			|| maskTextButton[activeMaskLayer] == NULL)
		{
			Logger::getInstance()->warning("Cannot add mask picture text line when no mask text set.");
			assert(!"Cannot add mask picture text line when no mask text set.");
			return;
		}

		char *newText = new char[strlen(maskText[activeMaskLayer]) + strlen(textline) + 2];
		strcpy(newText, maskText[activeMaskLayer]);
		strcat(newText, "\n");
		strcat(newText, textline);
		delete [] maskText[activeMaskLayer];
		maskText[activeMaskLayer] = newText;

		maskTextButton[activeMaskLayer]->SetText(maskText[activeMaskLayer]);
	}


	void UIEffects::setMaskPictureTextPositionX(int x)
	{
		maskTextPositionX[activeMaskLayer] = x;
		if (maskText[activeMaskLayer] != NULL)
		{
			maskTextButton[activeMaskLayer]->Move(
				maskTextPositionX[activeMaskLayer], 
				maskTextPositionY[activeMaskLayer]);
		}
	}


	void UIEffects::setMaskPictureTextPositionY(int y)
	{
		maskTextPositionY[activeMaskLayer] = y;
		if (maskText[activeMaskLayer] != NULL)
		{
			maskTextButton[activeMaskLayer]->Move(
				maskTextPositionX[activeMaskLayer], 
				maskTextPositionY[activeMaskLayer]);
		}
	}


	void UIEffects::setMaskPictureTextAreaSizeX(int sizex)
	{
		maskTextAreaSizeX[activeMaskLayer] = sizex;
		if (maskText[activeMaskLayer] != NULL)
		{
			// TODO
			/*
			maskText[activeMaskLayer]->Resize(
				maskTextAreaSizeX[activeMaskLayer], 
				maskTextAreaSizeY[activeMaskLayer]);
			*/
		}
	}


	void UIEffects::setMaskPictureTextAreaSizeY(int sizey)
	{
		maskTextAreaSizeY[activeMaskLayer] = sizey;
		if (maskText[activeMaskLayer] != NULL)
		{
			// TODO
			/*
			maskText[activeMaskLayer]->Resize(
				maskTextAreaSizeX[activeMaskLayer], 
				maskTextAreaSizeY[activeMaskLayer]);
			*/
		}
	}

	void UIEffects::startMaskPictureFadeIn(int duration)
	{
		if (maskWindow[activeMaskLayer] != NULL)
		{
			maskWindow[activeMaskLayer]->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, duration);
		} else {
			Logger::getInstance()->warning("UIEffects::startMaskPictureFadeIn - Active mask layer has no mask picture.");
		}
	}

	void UIEffects::startMaskPictureFadeOut(int duration)
	{
		if (maskWindow[activeMaskLayer] != NULL)
		{
			maskWindow[activeMaskLayer]->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, duration);
		} else {
			Logger::getInstance()->warning("UIEffects::startMaskPictureFadeOut - Active mask layer has no mask picture.");
		}
	}

	void UIEffects::startMaskPictureMoveIn(int duration)
	{
		if (maskWindow[activeMaskLayer] != NULL)
		{
			maskWindow[activeMaskLayer]->StartEffect(OGUI_WINDOW_EFFECT_MOVEIN, duration);
		} else {
			Logger::getInstance()->warning("UIEffects::startMaskPictureMoveIn - Active mask layer has no mask picture.");
		}
	}

	void UIEffects::startMaskPictureMoveOut(int duration)
	{
		if (maskWindow[activeMaskLayer] != NULL)
		{
			maskWindow[activeMaskLayer]->StartEffect(OGUI_WINDOW_EFFECT_MOVEOUT, duration);
		} else {
			Logger::getInstance()->warning("UIEffects::startMaskPictureMoveOut - Active mask layer has no mask picture.");
		}
	}


	void UIEffects::clearMaskPictureText()
	{
		if (maskText[activeMaskLayer] != NULL)
		{
			delete [] maskText[activeMaskLayer];
			maskText[activeMaskLayer] = NULL;
		}
		if (maskTextButton[activeMaskLayer] != NULL)
		{
			delete maskTextButton[activeMaskLayer];
			maskTextButton[activeMaskLayer] = NULL;
		}
		if (maskFont[activeMaskLayer] != NULL)
		{
			delete maskFont[activeMaskLayer];
			maskFont[activeMaskLayer] = NULL;
		}
		maskTextPositionX[activeMaskLayer] = 0;
		maskTextPositionY[activeMaskLayer] = 0;
		maskTextAreaSizeX[activeMaskLayer] = 1024;
		maskTextAreaSizeY[activeMaskLayer] = 768;
	}

	void UIEffects::EffectEvent(OguiEffectEvent *eve)
	{
		if (eve->eventType == OguiEffectEvent::EVENT_TYPE_MOVEDOUT
			|| eve->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
		{
			int prevActive = activeMaskLayer;
			for (int i = 0; i < UIEFFECTS_MASK_PICTURE_LAYERS; i++)
			{
				if (maskWindow[i] != NULL
					&& eve->triggerWindow == maskWindow[i])
				{
					activeMaskLayer = i;
					clearMaskPicture();
					clearMaskPictureText();
				}
			}
			activeMaskLayer = prevActive;
		}
	}

	int UIEffects::getActiveMaskPictureLayer()
	{
		return activeMaskLayer;
	}

	void UIEffects::setNVGoggleEffect(bool enabled)
	{
		filterEffectEnabled[FILTER_EFFECT_NVGOGGLE] = enabled;
	}

	void UIEffects::setSlomoEffect(bool enabled)
	{
		slomoEffectStartTime = Timer::getUnfactoredTime();
		if(enabled)
		{
			filterEffectEnabled[FILTER_EFFECT_SLOMO] = true;
			filterEffectEnabled[FILTER_EFFECT_SLOMO_ENDING] = false;
		}
		else
		{
			filterEffectEnabled[FILTER_EFFECT_SLOMO] = false;
			filterEffectEnabled[FILTER_EFFECT_SLOMO_ENDING] = true;
		}
	}

	void UIEffects::setNapalmEffect(bool enabled)
	{
		napalmEffectStartTime = Timer::getTime();
		filterEffectEnabled[FILTER_EFFECT_NAPALM] = enabled;
	}

	void UIEffects::setHealEffect(bool enabled)
	{
		healEffectStartTime = Timer::getTime();
		filterEffectEnabled[FILTER_EFFECT_HEAL] = enabled;
	}

	void UIEffects::setCustomFilterEffect(bool enabled, int brightness, int contrast, int red, int green, int blue)
	{
		// FIXME: doesn't work in trunk?
		customEffectStartTime = Timer::getUnfactoredTime();
		if(enabled)
		{
			customEffectBrightness = brightness;
			customEffectContrast = contrast;
			customEffectRed = red;
			customEffectGreen = green;
			customEffectBlue = blue;
			filterEffectEnabled[FILTER_EFFECT_CUSTOM] = true;
			filterEffectEnabled[FILTER_EFFECT_CUSTOM_ENDING] = false;
		}
		else
		{
			filterEffectEnabled[FILTER_EFFECT_CUSTOM] = false;
			filterEffectEnabled[FILTER_EFFECT_CUSTOM_ENDING] = true;
		}
	}

	void UIEffects::clearFilterEffects()
	{
		for(int i = 0; i < NUM_FILTER_EFFECTS; i++)
		{
			filterEffectEnabled[i] = false;
		}
	}

}

