
#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

#include <c2_vectors.h>
#include <c2_quat.h>

#include <IStorm3D_Model.h>

class IStorm3D;
//class IStorm3D_Model;
class IStorm3D_Scene;
class IStorm3D_BoneAnimation;
class IStorm3D_Bone;


// max rotation/position interpolation (history size)

#ifdef GAME_SIDEWAYS
// sideways gameplay has bigger tendency to make the units "shake" as they stand on obstacles,
// thus, larger position interpolation seems necessary
// rotation however, is less likely to require interpolation, as units don't really rotate that much.
#define VISUALOBJECT_MAX_POSITION_INTERPOLATION 16
#define VISUALOBJECT_MAX_ROTATION_INTERPOLATION 4
#else
#define VISUALOBJECT_MAX_POSITION_INTERPOLATION 8
#define VISUALOBJECT_MAX_ROTATION_INTERPOLATION 8
#endif



// bad dependencies!
namespace game
{
  class GameUI;
  class Game;
  class Projectile;
  class ProjectileActor;
  class GameScene;
  class BuildingAdder;
}

namespace frozenbyte
{
	class TextureCache;
}


namespace ui
{

  // this one bad too
  class GamePointers;

  class VisualObjectModel;
  class Animator;


  extern const int visualObjectID;

  class IVisualObjectData
  {
  public:
	  virtual ~IVisualObjectData() {}
    virtual void *getVisualObjectDataId() const = 0;
  };

  class VisualObject : public IStorm3D_Model_Data
  {
  public:
    VisualObject();
    virtual ~VisualObject();

		static void setTextureCache(frozenbyte::TextureCache *textureCache);

		// call this before every render
    void prepareForRender(float interpolate_factor = 1.0f);

		// call this after every game tick (if interpolation is used)
    void advanceHistory();

    virtual void *GetID();

		void setSideways(bool sideways);

    void setPosition(const VC3 &position);
    void addPosition(const VC3 &position);
    void setScale(const VC3 &scale);
    void setRotation(float xAngle, float yAngle, float zAngle);
	
		// hacked to allow straight quaternion rotations 
		// overrided the setRotation euler angles
		void setRotationQuaternion( const QUAT& quat );

    //void addRotation(QUAT &rotation);
    void setSelfIllumination(const COL &color);

    IVisualObjectData *getDataObject();
    void setDataObject(IVisualObjectData *dataObject);

    void setCustomData(IStorm3D_Model_Data *data);

    // combines the given visual object to this one
    // used to combine units from parts
    // attach point is the helper name to attach to (null if none)
    void combine(VisualObject *sourceObject, const char *newObjectName, const char *attachTo);

		void removeObject(const char *objectName);

    // sets this object to be rendered to scene or not, affects raytrace
    void setInScene(bool inScene);

    void setCollidable(bool collidable);

    void setForcedNoCollision(bool forcedNoCollision) { this->forcedNoCollision = forcedNoCollision; }

    void setVisible(bool visible);
	 inline bool isVisible() { return this->visible; };

    void setSphereCollisionOnly(bool sphereCollisionOnly);

    // a hack really...
    void clearObjects();

		void rotateBone(const char *boneName, float angle, float betaAngle);

		void releaseRotatedBone(const char *boneName);

		VisualObjectModel *getVisualObjectModel();

		void setEffect(int modelEffect);
		
		// NOTE: currently used by some effects only (others just ignore this)
		void setEffectDuration(int durationMsec);

		void clearEffects();

		void disableSharedObjects();
		
		void createEffectLayer();
		void createEffectLayer2();

		IStorm3D_Model *getStormModel() { return model; }

		void setStaticRotationYAngle(float staticRotationY);

		void setPositionInterpolationAmount(int interpolationAmount);
		void setPositionInterpolationTreshold(float interpolationTreshold);
		void setRotationInterpolationAmount(int interpolationAmount);
		void setRotationInterpolationTreshold(float interpolationTreshold);

		void setNoShadow(bool noShadow);

		void resetMaterialEffects();

		void setVisibilityFactor(float visFactor);
		float getVisibilityFactor() const;
		void setLightVisibilityFactor(float visFactor, bool enable);
		float getLightVisibilityFactor() const;

		// NOTE: does not actually change anything, just keeps the given value in memory.
		void setLightingFactor(float lightFactor);
		float getLightingFactor() const;

		float getRenderYAngle() const { return renderYAngle; }

		void forcePositionUpdate();

		// HACK: for burned special effect...
		void setBurnedCrispyAmount(int amount);

		// to get the actual position to which the model is rendered...
		// (so that camera may track that instead of the game unit position)
		VC3 getRenderPosition() { return interpolatedRenderPosition; }

		void setCAReflect(bool reflect, const VC3 &reflectionPosition, const VC3 &reflectionScale);

		// max sensible length is 1.0 (which maps to position interpolation history length)
		// (others can be used too, but a too big length will get clamped and implementation uses integer sampling...)
		void setCAMotionBlur(bool motionBlur, float length);

		// flag this as skymodel.
		void makeSkyModel();
		bool isSkyModel() { return skyModel; }

		void setPerFrameInterpolation(bool enabled) { interpolatePerFrame = enabled; }
		bool getPerFrameInterpolation(bool enabled) { return interpolatePerFrame; }
  private:
		VC3 renderPosition;
		float renderXAngle;
		float renderYAngle;
		float renderZAngle;
		bool renderNeedRotationUpdate;
		bool renderNeedPositionUpdate;
		IVisualObjectData *dataObject;

		IStorm3D_Model *model;
		IStorm3D_BoneAnimation *animation;
		IStorm3D_Scene *scene;
		IStorm3D *storm3d;

    bool inScene;
    bool collidable;
    bool sphereCollisionOnly;
    bool visible;

		bool interpolatePerFrame;
		VC3 lastRenderPosition;
		VC3 interpolatedRenderPosition;

		bool reflected;
		bool motionBlurred;
		float motionBlurLength;

		bool sideways;

		bool forcedNoCollision;

		int positionInterpolationAmount;
		float positionInterpolationTreshold;
		int rotationInterpolationAmount;
		float rotationInterpolationTreshold;

		VC3 positionHistory[VISUALOBJECT_MAX_POSITION_INTERPOLATION];
		int atPositionHistory;
		float rotationYHistory[VISUALOBJECT_MAX_ROTATION_INTERPOLATION];
		int atRotationHistory;
		// (note, only y-axel rotation interpolated)

		IStorm3D_Bone *interpolatedRotateBone;
		float interpolatedBoneAngle;
		float interpolatedBoneBetaAngle;

		float renderVisibility;
		float lightVisibility;
		float renderLighting;

    // the name of the storm model_object to be used
    // all other objects in the model should be ingnored
    char *objectName;

		int effects;
		int effectDuration;

		int burnedCrispyAmount;

		float staticRotationYAngle;

		void applyEffects();

    // for shared model freeing...
    VisualObjectModel *visualObjectModel;

	// for the quaternion rotation hack
	bool needQuatRotation;
	QUAT quatRotation;

		bool skyModel;

    friend class VisualObjectModel;
    friend class Animator;
    
    friend class game::GameUI;
    // this is just a quick hack to access storm models, because we need to
    // seek the right unit (visual object) based on storm's collision info
    friend class GamePointers;
    // and this one too
    friend class game::Game;
    // and especially this!
    friend class game::Projectile;
    // and this.
    friend class game::ProjectileActor;
    // and this too.
    friend class game::GameScene;
    // and this too.
    friend class game::BuildingAdder;
    // and this too.

  };

}

#endif
