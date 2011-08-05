
#ifndef ANIMATOR_H
#define ANIMATOR_H

class IStorm3D;
class IStorm3D_BoneAnimation;

namespace ui
{

  class VisualObject;
  class IAnimatable;

  /**
   * Class for setting animations for animatable objects.
   * @version 1.1, 23.10.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see IAnimatable
   * @see VisualObject
   */

  class Animator
  {
  public:
    static void init(IStorm3D *s3d);
    static void uninit();
    static void setAnimation(IAnimatable *animatable, int transitionNumber, int animNumber, bool blend = true, int blendTime = 200, bool loop = true, bool noInterpolate = false);
    static void setBlendAnimation(IAnimatable *animatable, int blendNumber, int transitionNumber, int animNumber, bool blend = true, int blendTime = 200, bool loop = true, bool noInterpolate = false);
    static void endBlendAnimation(IAnimatable *animatable, int blendNumber, bool blend = true);
    static void clearAnimations();
    static int getAnimationLength(int animNumber);
		static int getAnimationTime(IAnimatable *animatable, int animNumber);

    static void setAnimationSpeedFactor(IAnimatable *animatable, float speedFactor);
		static void setAnimationSpeedFactor(IAnimatable *animatable, int animNumber, float speedFactor);

		static int addAnimationFilename(const char *filename);
		static int getAnimationNumberByFilename(const char *filename);

  private:
    static IStorm3D_BoneAnimation **animations;
    static IStorm3D *storm3d;
  };

}

#endif

