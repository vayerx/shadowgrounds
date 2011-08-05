
#include "precompiled.h"

#include "Animator.h"

#include <Storm3D_UI.h>
#include <assert.h>

#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "animdefs.h"
#include "VisualObject.h"
#include "IAnimatable.h"
#include "../util/SimpleParser.h"
//#include "../util/fb_assert.h"

#include "../util/Debug_MemoryManager.h"

namespace ui
{

char *animfiles[MAX_ANIMATIONS];
  
IStorm3D_BoneAnimation **Animator::animations = NULL;
IStorm3D *Animator::storm3d = NULL;

void Animator::init(IStorm3D *s3d)
{
  if (animations != NULL)
  {
    Logger::getInstance()->warning("Animator::init - Animator was already initialized.");
    return;
  }

  storm3d = s3d;
  animations = new IStorm3D_BoneAnimation *[MAX_ANIMATIONS];
  for (int i = 0; i < MAX_ANIMATIONS; i++)
  {
    animations[i] = NULL;
  }

  util::SimpleParser parser = util::SimpleParser();

#ifdef LEGACY_FILES
  bool loadok = parser.loadFile("Data/Animations/animation_files.txt");
#else
  bool loadok = parser.loadFile("data/animation/animation_files.txt");
#endif
  if (!loadok)
  {
    Logger::getInstance()->error("Animator::init - Initialization failed.");
    return;
  }

  while (parser.next())
  {
    char *key = parser.getKey();
    int num = 0;
    if (key != NULL)
    {
      num = str2int(key);
    }
    if (num == 0)
    {
      parser.error("Animator::init - Animation id number and filename expected.");
    } else {
      char *value = parser.getValue();
      assert(value != NULL);

      if (animfiles[num] != NULL)
      {
        parser.error("Animator::init - Duplicate animation id number.");
      } else {
        animfiles[num] = new char[strlen(value) + 1];
        strcpy(animfiles[num], value);
      }
    }
  }
}

void Animator::uninit()
{
	for (int i = 0; i < MAX_ANIMATIONS; i++)
	{
		if (animfiles[i] != NULL)
		{
			delete [] animfiles[i];
			animfiles[i] = NULL;
		}
	}

  if (animations != NULL)
  {
    clearAnimations();
    delete [] animations;
    animations = NULL;
  } else {
    Logger::getInstance()->warning("Animator::uninit - Animator was already uninitialized.");
  }
}

/*
int Animator::getAnimationLength(int animNumber)
{
  assert(animNumber >= 0 || animNumber < TOTAL_ANIMATIONS);
  return animlengths[animNumber];
}
*/


int Animator::addAnimationFilename(const char *filename)
{
	assert(filename != NULL);
	int prev = getAnimationNumberByFilename(filename);
	if (prev != -1)
	{
    Logger::getInstance()->warning("Animator::addAnimationFilename - Animation filename already exists.");
		return prev;
	}
	for (int i = 1; i < MAX_ANIMATIONS; i++)
	{
		if (animfiles[i] == NULL)
		{
			animfiles[i] = new char[strlen(filename) + 1];
			strcpy(animfiles[i], filename);
			return i;
		}
	}
	Logger::getInstance()->error("Animator::addAnimationFilename - Unable to add animation, too many animations.");
	return -1;
}


int Animator::getAnimationNumberByFilename(const char *filename)
{
	assert(filename != NULL);
  for (int i = 0; i < MAX_ANIMATIONS; i++)
  {
    if (animfiles[i] != NULL)
		{
			if (strcmp(animfiles[i], filename) == 0)
			{
				return i;
			}
		}
  }	
	return -1;
}


void Animator::setAnimation(IAnimatable *animatable, int transitionNumber, int animNumber, bool blend, int blendTime, bool loop, bool noInterpolate)
{
  if (animatable->getAnimation() == animNumber)
  {
    // already running the animation.
    return;
  }

  VisualObject *visualObject = animatable->getVisualObject();
  if (visualObject == NULL)
  {
    assert(!"Animator::setAnimation - Null visual object.");
    //return;
  }

	if (animations != NULL)
  {
    if (animNumber < 0 || animNumber >= MAX_ANIMATIONS)
    {
      Logger::getInstance()->error("Animator::setAnimation - Animation number out of bounds.");
    } else {
      if (visualObject == NULL)
      {
        Logger::getInstance()->error("Animator::setAnimation - Null visual object given.");
      } else {
        // load animation if it has not been loaded yet...
        if (animations[animNumber] == NULL 
          && animfiles[animNumber] != NULL)
        {
          animations[animNumber] = 
            storm3d->CreateNewBoneAnimation(animfiles[animNumber]); 
					if (animations[animNumber] == NULL)
					{
						Logger::getInstance()->warning("Animator::setAnimation - Failed to create new bone animation.");
						Logger::getInstance()->debug(animfiles[animNumber]);
					}
        }


				IStorm3D_BoneAnimation *transitionAnim = 0;
				if (transitionNumber < 0 || transitionNumber >= MAX_ANIMATIONS)
				{
					if(transitionNumber < -1 || transitionNumber >= MAX_ANIMATIONS)
						Logger::getInstance()->error("Animator::setAnimation - Animation number out of bounds.");
				} 
				else 
				{
					// load animation if it has not been loaded yet...
					if (animations[transitionNumber] == NULL 
					&& animfiles[transitionNumber] != NULL)
					{
						animations[transitionNumber] = 
							storm3d->CreateNewBoneAnimation(animfiles[transitionNumber]); 
						if (animations[transitionNumber] == NULL)
						{
							Logger::getInstance()->warning("Animator::setAnimation - Failed to create new bone animation.");
							Logger::getInstance()->debug(animfiles[transitionNumber]);
						}
					}

					transitionAnim = animations[transitionNumber];
				}


        if (visualObject->animation != animations[animNumber])
        {
          visualObject->animation = animations[animNumber];
          if (blend)
          {
            visualObject->model->BlendToAnimation(transitionAnim, visualObject->animation, blendTime, loop);
            animatable->setAnimation(animNumber);
						animatable->setAnimationSpeedFactor(1.0f);
            //assert(success);
          } else {
						// temp...
						//fb_assert(visualObject->animation != NULL);

            visualObject->model->SetAnimation(transitionAnim, visualObject->animation, loop);
            animatable->setAnimation(animNumber);
						animatable->setAnimationSpeedFactor(1.0f);
          }
          // visualObject->model->SetAnimation(visualObject->animation, true);
        }
      }
    }
  } else {
    Logger::getInstance()->error("Animator::setAnimation - Animator uninitialized.");
  }
}


int Animator::getAnimationTime(IAnimatable *animatable, int animNumber)
{
  VisualObject *visualObject = animatable->getVisualObject();
  if (visualObject == NULL)
		return 0;

  if (animNumber < 0 || animNumber >= MAX_ANIMATIONS)
		return 0;

	return visualObject->model->GetAnimationTime(animations[animNumber]);
}

void Animator::setAnimationSpeedFactor(IAnimatable *animatable, float speedFactor)
{
  if (animatable->getAnimationSpeedFactor() == speedFactor)
  {
    // already running the animation at this speed.
    return;
  }

	int animNumber = animatable->getAnimation();

	// consistency check
	//VisualObject *visualObject = animatable->getVisualObject();
	//assert(visualObject->animation == animations[animNumber]);

	setAnimationSpeedFactor(animatable, animNumber, speedFactor);
	animatable->setAnimationSpeedFactor(speedFactor);
}

void Animator::setAnimationSpeedFactor(IAnimatable *animatable, int animNumber, float speedFactor)
{
	VisualObject *visualObject = animatable->getVisualObject();
  if (visualObject == NULL)
  {
    assert(!"Animator::setAnimationSpeedFactor - Null visual object.");
    //return;
  }

  if (animations != NULL)
  {
    if (animNumber < 0 || animNumber >= MAX_ANIMATIONS)
    {
      Logger::getInstance()->error("Animator::setAnimationSpeedFactor - Animation number out of bounds (internal error).");
    } else {
      if (visualObject == NULL)
      {
        Logger::getInstance()->error("Animator::setAnimationSpeedFactor - Null visual object given.");
      } else {
				// check that animation is loaded
        if (animations[animNumber] == NULL
          && animfiles[animNumber] != NULL)
				{
					Logger::getInstance()->error("Animator::setAnimationSpeedFactor - Animation not loaded.");
					Logger::getInstance()->debug(animfiles[animNumber]);
				} else {
					visualObject->model->SetAnimationSpeedFactor(animations[animNumber], speedFactor);
				}
      }
    }
  } else {
    Logger::getInstance()->error("Animator::setAnimationSpeedFactor - Animator uninitialized.");
  }
}


void Animator::setBlendAnimation(IAnimatable *animatable, int blendNumber, int transitionNumber, int animNumber, bool blend, int blendTime_, bool loop, bool noInterpolate)
{
  if (animatable->getBlendAnimation(blendNumber) == animNumber)
  {
    // already running the blended animation.
    return;
  }

  if (animatable->getBlendAnimation(blendNumber) != 0)
	{
		// running some other animation, need to end it first		
		endBlendAnimation(animatable, blendNumber, false);
	}


  VisualObject *visualObject = animatable->getVisualObject();
  if (visualObject == NULL)
  {
    assert(!"Animator::setBlendAnimation - Null visual object.");
    //return;
  }

  if (animations != NULL)
  {
    if (animNumber < 0 || animNumber >= MAX_ANIMATIONS)
    {
      Logger::getInstance()->error("Animator::setBlendAnimation - Animation number out of bounds.");
    } else {
      if (visualObject == NULL)
      {
        Logger::getInstance()->error("Animator::setBlendAnimation - Null visual object given.");
      } else {
        // load animation if it has not been loaded yet...
        if (animations[animNumber] == NULL 
          && animfiles[animNumber] != NULL)
        {
          animations[animNumber] = 
            storm3d->CreateNewBoneAnimation(animfiles[animNumber]); 
        }


				IStorm3D_BoneAnimation *transitionAnim = 0;
				if (transitionNumber < 0 || transitionNumber >= MAX_ANIMATIONS)
				{
					if(transitionNumber < -1 || transitionNumber >= MAX_ANIMATIONS)
						Logger::getInstance()->error("Animator::setAnimation - Animation number out of bounds.");
				} 
				else 
				{
					// load animation if it has not been loaded yet...
					if (animations[transitionNumber] == NULL && animfiles[transitionNumber] != NULL)
					{
						animations[transitionNumber] = storm3d->CreateNewBoneAnimation(animfiles[transitionNumber]); 
						if (animations[transitionNumber] == NULL)
						{
							Logger::getInstance()->warning("Animator::setAnimation - Failed to create new bone animation.");
							Logger::getInstance()->debug(animfiles[transitionNumber]);
						}
					}

					transitionAnim = animations[transitionNumber];
				}


        int blendTime;
        if (blend) 
				{
          blendTime = blendTime_;
				} else {
          blendTime = 1;
				}
        visualObject->model->BlendWithAnimationIn(transitionAnim, animations[animNumber], blendTime, loop);
        animatable->setBlendAnimation(blendNumber, animNumber);
      }
    }
  } else {
    Logger::getInstance()->error("Animator::setBlendAnimation - Animator uninitialized.");
  }
}


void Animator::endBlendAnimation(IAnimatable *animatable, int blendNumber, bool blend)
{
  if (animatable->getBlendAnimation(blendNumber) == 0)
  //if (animatable->getBlendAnimation(blendNumber) != animNumber)
  {
    // not running the blended animation!
    //assert(!"Animator::endBlendAnimation - Not running the blended animation.");
    return;
  }

  int animNumber = animatable->getBlendAnimation(blendNumber);

  VisualObject *visualObject = animatable->getVisualObject();
  if (visualObject == NULL)
  {
    assert(!"Animator::endBlendAnimation - Null visual object.");
    return;
  }

  if (animations != NULL)
  {
    if (animNumber < 0 || animNumber >= MAX_ANIMATIONS)
    {
      Logger::getInstance()->error("Animator::endBlendAnimation - Animation number out of bounds.");
    } else {
      if (visualObject == NULL)
      {
        Logger::getInstance()->error("Animator::endBlendAnimation - Null visual object given.");
      } else {
        // animation should be loaded...
        //assert(animations[animNumber] != NULL);
				if (!animations[animNumber])
				{
	        Logger::getInstance()->error("Animator::endBlendAnimation - Requested animation was not loaded (animation file missing?), possibly causing erronous animation behaviour.");
	        Logger::getInstance()->debug("animation number follows:");
	        Logger::getInstance()->debug(int2str(animNumber));
					//assert(!"Animator::endBlendAnimation - Requested animation was not loaded (animation file missing?), possibly causing erronous animation behaviour.");
				}

        int blendTime;
        if (blend) 
          blendTime = 200;
        else
          blendTime = 1;
        visualObject->model->BlendWithAnimationOut(0, animations[animNumber], blendTime);
        animatable->setBlendAnimation(blendNumber, 0);
      }
    }
  } else {
    Logger::getInstance()->error("Animator::endBlendAnimations - Animator uninitialized.");
  }
}

void Animator::clearAnimations()
{
  if (animations != NULL)
  {
    // TODO! (something?)
  } else {
    Logger::getInstance()->error("Animator::clearAnimations - Animator uninitialized.");
  }
}

}
