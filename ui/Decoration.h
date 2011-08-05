
#ifndef DECORATION_H
#define DECORATION_H

#include <DatatypeDef.h>

#define DECORATION_MAX_EFFECTS 19

namespace ui
{
	class VisualObject;
	class VisualObjectModel;
  class DecorationManager;

  class Decoration
  {
    public:
      enum DECORATION_EFFECT
      {
				DECORATION_EFFECT_INVALID = 0,
        DECORATION_EFFECT_WAVE_X = 1,
        DECORATION_EFFECT_WAVE_Y = 2,
        DECORATION_EFFECT_WAVE_Z = 3,
        DECORATION_EFFECT_SHAKE_X = 4,
        DECORATION_EFFECT_SHAKE_Y = 5,
        DECORATION_EFFECT_SHAKE_Z = 6,
        DECORATION_EFFECT_MOVE_AND_WARP_X_POSITIVE = 7,
        DECORATION_EFFECT_MOVE_AND_WARP_X_NEGATIVE = 8,
        DECORATION_EFFECT_MOVE_AND_WARP_Z_POSITIVE = 9,
        DECORATION_EFFECT_MOVE_AND_WARP_Z_NEGATIVE = 10,
        DECORATION_EFFECT_ROTATE_X = 11,
        DECORATION_EFFECT_ROTATE_Y = 12,
        DECORATION_EFFECT_ROTATE_Z = 13,
        DECORATION_EFFECT_SWING_X = 14,
        DECORATION_EFFECT_SWING_Z = 15,
        DECORATION_EFFECT_GEAR1_Y = 16,
        DECORATION_EFFECT_GEAR2_Y = 17,
        DECORATION_EFFECT_GEAR3_Y = 18
      };

      static DECORATION_EFFECT getDecorationEffectByName(const char *effectName);

      Decoration();

      ~Decoration();

      void run();

			void loadModel(const char *filename);

      void setName(const char *name);
   
      void setPosition(const VC3 &position);

      const VC3 &getPosition();

      void setStartRotation(const VC3 &rotation);

      void setHeight(float height);

	  void setSpeed(float speed);
      
	  void setEndPosition(const VC3 &position);

      void setEndHeight(float height);

      void stretchBetweenPositions();

      void setEffect(DECORATION_EFFECT effect, bool enabled);

			void setNoShadow(bool noShadow);

      //void setEffectSpeed(float duration);

			// (special function for water decors...)
			void getBoundingQuadSize(float *xSize, float *zSize) const;

			ui::VisualObject *getVisualObject();

    private:

			void updateVisual();

      char *name;

      VC3 position;

      VC3 endPosition;

      bool stretched;

      VC3 startRotation;
      VC3 rotation;
      
      VisualObject *visualObject;
      VisualObjectModel *visualObjectModel;

      bool effectOn[DECORATION_MAX_EFFECTS];
      float effectValue[DECORATION_MAX_EFFECTS];
			
			int tickCount;

      //float effectDuration;
	  
      void parseBoundingQuadSize(const char* modelFileName);
	  
      float boundingQuadSizeX;
      float boundingQuadSizeY;

	  float animationSpeed;

      friend class DecorationManager;
  };
}

#endif



