
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "Decoration.h"

#include <assert.h>
#include "../game/gamedefs.h"
#include "VisualObject.h"
#include "VisualObjectModel.h"

namespace ui
{
	const char *decorEffectNames[DECORATION_MAX_EFFECTS + 1] =
	{
		"_invalid_",
		"wave_x",
		"wave_y",
		"wave_z",
		"shake_x",
		"shake_y",
		"shake_z",
		"move_and_warp_x_positive",
		"move_and_warp_x_negative",
		"move_and_warp_z_positive",
		"move_and_warp_z_negative",
		"rotate_x",
		"rotate_y",
		"rotate_z",
		"swing_x",
		"swing_z",
		"gear1_y",
		"gear2_y",
		"gear3_y",
		"***"
	};


  Decoration::DECORATION_EFFECT Decoration::getDecorationEffectByName(
		const char *effectName)
	{
#ifdef _DEBUG
		if (decorEffectNames[DECORATION_MAX_EFFECTS] != NULL)
		{
			if (strcmp(decorEffectNames[DECORATION_MAX_EFFECTS], "***") != 0)
			{
				assert(!"Decoration - Decoration name array not ok. Fix it.");
			}
		} else {
			assert(!"Decoration - Decoration name array not ok. Fix it.");
		}
#endif
		
		for (int i = 0; i < DECORATION_MAX_EFFECTS; i++)
		{
			if (decorEffectNames[i] != NULL)
			{
				if (strcmp(decorEffectNames[i], effectName) == 0)
				{
					return (DECORATION_EFFECT)i;
				}
			}
		}
		assert(0);
		return DECORATION_EFFECT_INVALID;
	}


  Decoration::Decoration()
	{
		name = NULL;
		position = VC3(0,0,0);
		rotation = VC3(0,0,0);
		startRotation = VC3(0,0,0);
		visualObject = NULL;
		visualObjectModel = NULL;
		stretched = false;
		for (int i = 0; i < DECORATION_MAX_EFFECTS; i++)
		{
			effectOn[i] = false;
			effectValue[i] = 0;
		}
		boundingQuadSizeX = 0.0f;
		boundingQuadSizeY = 0.0f;
		tickCount = 0;
		animationSpeed = 1;
  }


  Decoration::~Decoration()
	{
		if (this->name != NULL)
		{
			delete [] this->name;
			this->name = NULL;
		}
		if (visualObject != NULL)
		{
			delete visualObject;
			visualObject = NULL;
		}
		if (visualObjectModel != NULL)
		{
			delete visualObjectModel;
			visualObjectModel = NULL;
		}
	}


	ui::VisualObject *Decoration::getVisualObject()
	{
		return visualObject;
	}

	void Decoration::loadModel(const char *filename)
	{
		if (visualObject != NULL)
		{
			delete visualObject;
			visualObject = NULL;
		}
		if (visualObjectModel != NULL)
		{
			delete visualObjectModel;
			visualObjectModel = NULL;
		}
		visualObjectModel = new VisualObjectModel(filename);
		visualObject = visualObjectModel->getNewObjectInstance();
		visualObject->setCollidable(false);
		visualObject->setInScene(true);
		visualObject->setVisible(true);
		//visualObject->setEffect(VISUALOBJECTMODEL_EFFECT_ADDITIVE);
		//visualObject->setEffect(VISUALOBJECTMODEL_EFFECT_MULTIPLY);
		updateVisual();
		// parse water boundaries:
		parseBoundingQuadSize(filename);
	}


	void Decoration::getBoundingQuadSize(float *xSize, float *zSize) const
	{
		*xSize = boundingQuadSizeX;
		*zSize = boundingQuadSizeY;
	}

	// Modifications by Ilkka: All increments are multiplied by animationSpeed to allow speed adjustment
  void Decoration::run()
	{
		tickCount++;
		bool needUpdate = false;
		for (int i = 0; i < DECORATION_MAX_EFFECTS; i++)
		{
			if (effectOn[i])
			{
				switch(i)
				{
					case Decoration::DECORATION_EFFECT_WAVE_X:
						// NOTICE: inaccuracy grows by time!!!
						position.x += (float)sinf(effectValue[i]) / GAME_TICKS_PER_SECOND * 0.5f;
						effectValue[i] += animationSpeed*0.001f;
						if (effectValue[i] >= 2*3.1415927f)
							effectValue[i] -= 2*3.1415927f;
						break;

					case Decoration::DECORATION_EFFECT_WAVE_Y:
						// NOTICE: inaccuracy grows by time!!!
						position.y += (float)sinf(effectValue[i]) / GAME_TICKS_PER_SECOND * 0.5f;
						effectValue[i] += animationSpeed*0.0012f;
						if (effectValue[i] >= 2*3.1415927f)
							effectValue[i] -= 2*3.1415927f;
						break;

					case Decoration::DECORATION_EFFECT_WAVE_Z:
						// NOTICE: inaccuracy grows by time!!!
						position.z += (float)sinf(effectValue[i]) / GAME_TICKS_PER_SECOND * 0.5f;
						effectValue[i] += animationSpeed*0.0013f;
						if (effectValue[i] >= 2*3.1415927f)
							effectValue[i] -= 2*3.1415927f;
						break;

					case Decoration::DECORATION_EFFECT_ROTATE_X:
						// NOTICE: inaccuracy grows by time!!!
						rotation.x += animationSpeed*360.0f / GAME_TICKS_PER_SECOND;
						if (rotation.x >= 360)
							rotation.x -= 360;
						break;

					case Decoration::DECORATION_EFFECT_ROTATE_Y:
						// NOTICE: inaccuracy grows by time!!!
						rotation.y += animationSpeed*360.0f / GAME_TICKS_PER_SECOND;
						if (rotation.y >= 360)
							rotation.y -= 360;
						break;

					case Decoration::DECORATION_EFFECT_ROTATE_Z:
						// NOTICE: inaccuracy grows by time!!!
						rotation.z += animationSpeed*360.0f / GAME_TICKS_PER_SECOND;
						if (rotation.z >= 360)
							rotation.z -= 360;
						break;						

					case Decoration::DECORATION_EFFECT_GEAR1_Y:
						// NOTICE: inaccuracy grows by time!!!
						if ((tickCount % 500) < 100)
						{
							rotation.y += animationSpeed*36.0f / GAME_TICKS_PER_SECOND;
							if (rotation.y >= 360)
								rotation.y -= 360;
						}
						break;

					case Decoration::DECORATION_EFFECT_GEAR2_Y:
						// NOTICE: inaccuracy grows by time!!!
						if ((tickCount % 500) > 100
							&& (tickCount % 500) < 300)
						{
							rotation.y -= animationSpeed*36.0f / GAME_TICKS_PER_SECOND;
							if (rotation.y < 0)
								rotation.y += 360;
						}
						break;

					case Decoration::DECORATION_EFFECT_GEAR3_Y:
						// NOTICE: inaccuracy grows by time!!!
						if ((tickCount % 500) > 250)
						{
							rotation.y += animationSpeed*16.0f / GAME_TICKS_PER_SECOND;
							if (rotation.y >= 360)
								rotation.y -= 360;
						}
						break;

					default:
						break;
						// TODO... (nop ?)
				}
				needUpdate = true;
			}
		}
		if (needUpdate)
			updateVisual();
	}


  void Decoration::setName(const char *name)
	{
		if (this->name != NULL)
		{
			delete [] this->name;
			this->name = NULL;
		}
		if (name != NULL)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);
		}
	}


  void Decoration::setPosition(const VC3 &position)
	{
		this->position.x = position.x;
		this->position.z = position.z;
		// height ignored
		updateVisual();
	}


  
	void Decoration::setStartRotation(const VC3 &rotation)
	{
		this->startRotation = rotation;
		this->rotation = rotation;
		updateVisual();
	}


  void Decoration::setHeight(float height)
	{
		this->position.y = height;
		updateVisual();
	}

  void Decoration::setSpeed(float speed)
  {
	  this->animationSpeed = speed;
	  updateVisual(); // Is this necessary?
  }


  void Decoration::setEndPosition(const VC3 &position)
	{
		this->endPosition.x = position.x;
		this->endPosition.z = position.z;
		// height ignored
		updateVisual();
	}


  void Decoration::setEndHeight(float height)
	{
		this->endPosition.y = height;
		updateVisual();
	}


  void Decoration::stretchBetweenPositions()
	{
		// TODO!
		stretched = true;
	}


  void Decoration::setEffect(Decoration::DECORATION_EFFECT effect, 
		bool enabled)
	{
		effectOn[effect] = enabled;
	}


  void Decoration::updateVisual()
	{
		if (stretched)
		{
			// TODO
		} else {
			if (visualObject != NULL)
			{
				visualObject->setPosition(position);
				visualObject->setRotation(rotation.x, rotation.y, rotation.z);
				visualObject->prepareForRender();
			}
		}
	}

  void Decoration::parseBoundingQuadSize(const char* modelFileName) 
  {
	 boundingQuadSizeX = 0.0f;
	 boundingQuadSizeY = 0.0f;

	  if(visualObject)
	  {
			IStorm3D_Model *model = visualObject->getStormModel();
			if(model)
			{
				const AABB &aabb = model->GetBoundingBox();

				boundingQuadSizeX = (aabb.mmax.x - aabb.mmin.x) * .5f;
				boundingQuadSizeY = (aabb.mmax.z - aabb.mmin.z) * .5f;
			}
	  }
	  /*
	  // damn this would be ALOT easier with stl :)
	  char xSize[16];
	  char ySize[16];
	  memset(xSize, 0, sizeof(xSize));
	  memset(ySize, 0, sizeof(ySize));
	  int len = strlen(modelFileName) - 1;
	  int i;
	  for(i = len; i > 0; i--) {
		if(modelFileName[i] == '_')
			break;
	  }
	  if(i == 0) {
		 // decoration model file name doesn't have boundaries
		 boundingQuadSizeX = 0.0f;
		 boundingQuadSizeY = 0.0f;
		 return;
	  }
	  i++; // skip _
	  int a = 0;
	  for(i; i < len; i++) {
		if((modelFileName[i] == 'x') || (modelFileName[i] == 'X'))
			break;
		xSize[a++] = modelFileName[i];
	  }
	  if(i == len) {
		 // decoration model file name doesn't have Y boundaries
		 boundingQuadSizeX = 0.0f;
		 boundingQuadSizeY = 0.0f;
		 return;	  
	  }
	  i++; // skip x
	  a = 0;
	  for(i; i < len; i++) {
		if((modelFileName[i] == '.'))
			break;
		ySize[a++] = modelFileName[i];
	  }
	  boundingQuadSizeX = static_cast<float>(atof(xSize));
	  boundingQuadSizeY = static_cast<float>(atof(ySize));
	  */
  }

  const VC3 &Decoration::getPosition()
	{
		return this->position;
	}

	void Decoration::setNoShadow(bool noShadow)
	{
		// TODO: keep this flag in memory, apply later if visualobject
		// does not yet exist (or is recreated for some reason)
		if (visualObject != NULL)
		{
			visualObject->setNoShadow(noShadow);
		}
	}

}


