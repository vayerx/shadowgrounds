
#ifndef UIEFFECTS_H
#define UIEFFECTS_H

#include "../ogui/IOguiEffectListener.h"
#include "../ogui/OguiButton.h"

class IStorm3D;
class IStorm3D_TerrainRenderer;
class IStorm3D_Camera;
class Ogui;
class OguiWindow;
class OguiTextLabel;
class IOguiFont;

#define UIEFFECTS_MASK_PICTURE_POS_DEFAULT 0
#define UIEFFECTS_MASK_PICTURE_POS_UPPER_LEFT 1
#define UIEFFECTS_MASK_PICTURE_POS_UPPER_CENTER 2
#define UIEFFECTS_MASK_PICTURE_POS_UPPER_RIGHT 3
#define UIEFFECTS_MASK_PICTURE_POS_MIDDLE_LEFT 4
#define UIEFFECTS_MASK_PICTURE_POS_MIDDLE_CENTER 5
#define UIEFFECTS_MASK_PICTURE_POS_MIDDLE_RIGHT 6
#define UIEFFECTS_MASK_PICTURE_POS_LOWER_LEFT 7
#define UIEFFECTS_MASK_PICTURE_POS_LOWER_CENTER 8
#define UIEFFECTS_MASK_PICTURE_POS_LOWER_RIGHT 9
#define UIEFFECTS_MASK_PICTURE_POS_XY_COORDINATES 10

#define UIEFFECTS_MASK_PICTURE_POS_AMOUNT 11

#define UIEFFECTS_MASK_PICTURE_LAYERS 12



namespace ui
{
  class MovieAspectWindow;

  /**
   * A class for setting different kinds of graphical UI effects.
	 * Such as fade-in, fade-out, etc.
   * @version 1.0, 30.12.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see MovieAspectWindow
   */

	class UIEffects : public IOguiEffectListener
	{
		public:
      typedef enum {
        FLASH_EFFECT_TYPE_NONE = 1,
        FLASH_EFFECT_TYPE_EXPLOSION = 2,
        FLASH_EFFECT_TYPE_NUKE_EXPLOSION = 3,
//        FLASH_EFFECT_TYPE_NUKE_EXPLOSION_FAR = 4,
        FLASH_EFFECT_TYPE_ENVIRONMENT_LIGHTNING = 5,
        FLASH_EFFECT_TYPE_LIGHTNING_GUN = 6,
        //FLASH_EFFECT_TYPE_NVGOGGLE_ENABLE = 7,
        //FLASH_EFFECT_TYPE_NVGOGGLE_DISABLE = 8,
        FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL = 9,
        FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM = 10,
        FLASH_EFFECT_TYPE_PLAYER_HIT_BIG = 11,
        FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL = 12,
        FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM = 13,
        FLASH_EFFECT_TYPE_PLAYER_POISON_BIG = 14,
				FLASH_EFFECT_TYPE_CLUSTER_EXPLOSION = 15
      } FLASH_EFFECT_TYPE;

			UIEffects(Ogui *ogui, IStorm3D *storm3d);

			~UIEffects();

			void run(int msec, IStorm3D_TerrainRenderer *terrainRenderer, IStorm3D_Camera *stormCamera);

			void setMovieAspectRatio(bool movie);

			int getActiveMaskPictureLayer();

			void setMaskPicture(const char *filename);
			void setActiveMaskPictureLayer(int layer);
			bool isMaskPicture();
			void clearMaskPicture();
			void clearAllMaskPictures();
			void setMaskPictureSizeX(int sizex);
			void setMaskPictureSizeY(int sizey);
			void setMaskPicturePosition(int position);
			void setMaskPicturePositionX(int x);
			void setMaskPicturePositionY(int y);

			void setMaskPictureFont(const char *font);
			void setMaskPictureText(const char *text, OguiButton::TEXT_H_ALIGN align = OguiButton::TEXT_H_ALIGN_LEFT );
			void setMaskPictureTextPositionX(int x);
			void setMaskPictureTextPositionY(int y);
			void setMaskPictureTextAreaSizeX(int sizex);
			void setMaskPictureTextAreaSizeY(int sizey);
			void clearMaskPictureText();
			void addMaskPictureTextLine(const char *text);

			void startMaskPictureFadeIn(int duration);
			void startMaskPictureFadeOut(int duration);
			void startMaskPictureMoveIn(int duration);
			void startMaskPictureMoveOut(int duration);

			void startFadeIn(int msecDuration);
			void startFadeOut(int msecDuration);
			void startFadeOutIfNotFaded(int msecDuration);
			void setFaded();
			void setNoFade();
			void setFadeImageFilename(const char *fadeImageFilename);
			void setDefaultFadeImage();
			void setDefaultFadeImageIfHitImage();

			// filter effects
			void setNVGoggleEffect(bool enabled);
			void setSlomoEffect(bool enabled);
			void setNapalmEffect(bool enabled);
			void setHealEffect(bool enabled);
			void setCustomFilterEffect(bool enabled, int brightness, int contrast, int red, int green, int blue);
			void clearFilterEffects();

      // TODO: does not support multiple simultaneous flash effects yet.
      void startFlashEffect(FLASH_EFFECT_TYPE flashEffectType);

      void endAllFlashEffects(IStorm3D_TerrainRenderer *terrainRenderer);

			virtual void EffectEvent(OguiEffectEvent *eve);


		private:
			Ogui *ogui;
      IStorm3D *storm3d;
			MovieAspectWindow *movieWindow;

			OguiWindow *maskWindow[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskPosition[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskPositionX[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskPositionY[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskSizeX[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskSizeY[UIEFFECTS_MASK_PICTURE_LAYERS];

			IOguiFont *maskFont[UIEFFECTS_MASK_PICTURE_LAYERS];
			char *maskText[UIEFFECTS_MASK_PICTURE_LAYERS];
			OguiTextLabel *maskTextButton[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskTextPositionX[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskTextPositionY[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskTextAreaSizeX[UIEFFECTS_MASK_PICTURE_LAYERS];
			int maskTextAreaSizeY[UIEFFECTS_MASK_PICTURE_LAYERS];

			int activeMaskLayer;

			OguiWindow *fadeWindow;
			char *fadeImageFilename;
			
			int fadeTimeLeft;
			int fadeTotalTime;
			bool fadingIn;
			bool fadingOut;

      bool flashEffectRunning;
      int flashStartTime;
      FLASH_EFFECT_TYPE flashEffectType;

			enum FilterEffect
			{
				FILTER_EFFECT_NVGOGGLE = 0,
				FILTER_EFFECT_SLOMO = 1,
				FILTER_EFFECT_SLOMO_ENDING = 2,
				FILTER_EFFECT_NAPALM = 3,
				FILTER_EFFECT_HEAL = 4,
				FILTER_EFFECT_CUSTOM = 5,
				FILTER_EFFECT_CUSTOM_ENDING = 6,
				NUM_FILTER_EFFECTS = 7
			};

			bool filterEffectEnabled[NUM_FILTER_EFFECTS];

			int slomoEffectStartTime;
			int napalmEffectStartTime;
			int healEffectStartTime;

			int customEffectStartTime;
			int customEffectBrightness;
			int customEffectContrast;
			int customEffectRed;
			int customEffectGreen;
			int customEffectBlue;

			void setMaskWindowPos();

			void setGammaEffect(float brightness, float contrast, float redCorrection,
				float greenCorrection, float blueCorrection, IStorm3D_TerrainRenderer *terrainRenderer);

	};
}

#endif


