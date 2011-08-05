// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatypes
#include "DatatypeDef.h"
#include "c2_aabb.h"
#include "c2_sphere.h"


// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Model.h"
#include "IStorm3D_Material.h"
#include "IStorm3D_Light.h"
#include "IStorm3D_Helper.h"


// special render pass defs... --jpk
// bits, 0 - 30 (31 unused, as it is the signed bit)
// the "cheap-ass" effects (cheap-ass-motionblur and cheap-ass-reflection)
#define RENDER_PASS_BIT_CAMOTIONBLUR1 0
#define RENDER_PASS_BIT_CAMOTIONBLUR2 1
#define RENDER_PASS_BIT_CAMOTIONBLUR3 2
#define RENDER_PASS_BIT_CAREFLECTION_DEPTH_MASKS 3
#define RENDER_PASS_BIT_CAREFLECTION_REFLECTED 4
// the delayed render pass alpha objects...
#define RENDER_PASS_BIT_DELAYED_ALPHA 5
// the early render pass objects...
#define RENDER_PASS_BIT_EARLY_ALPHA 6
// an additional alpha test pass for alphablended objects...
#define RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS 7

#define RENDER_PASS_BITS_AMOUNT 8




//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Model;
class IStorm3D_Model_Object;
class IStorm3D_Mesh;
class IStorm3D_BoneAnimation;
class IStorm3D_Bone;
class IStorm3D_Model_Data;

//------------------------------------------------------------------
// IStorm3D_Model (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Model
{

public:

	// Create empty / load (".S3D"-file) / copy
	virtual bool LoadS3D(const char *filename) = 0;
	virtual bool LoadBones(const char *filename) = 0;
	
	//virtual void CopyModel(IStorm3D_Model *other)=0;
	virtual void Empty(bool leave_geometry = false, bool leave_bones = false)=0;
	virtual IStorm3D_Model *GetClone(bool cloneGeometry, bool cloneHelpers, bool cloneBones) const = 0;

	// Objects
	virtual IStorm3D_Model_Object *Object_New(const char *name)=0;
	virtual void Object_Delete(IStorm3D_Model_Object *object)=0;
	virtual IStorm3D_Model_Object *SearchObject(const char *name)=0;

	virtual void CastShadows(bool shadows) = 0;
	virtual void EnableBoneCollision(bool enable) = 0;
	virtual void ForceBoneUpdate() = 0;
	virtual void FreeMemoryResources() = 0;

	// Helpers
	virtual void Helper_Delete(IStorm3D_Helper *help)=0;
	virtual IStorm3D_Helper_Point *Helper_Point_New(const char *name, const VC3 &position)=0;
	virtual IStorm3D_Helper_Vector *Helper_Vector_New(const char *name, const VC3 &position, const VC3 &direction)=0;
	virtual IStorm3D_Helper_Camera *Helper_Camera_New(const char *name, const VC3 &position, const VC3 &direction, const VC3 &up)=0;
	virtual IStorm3D_Helper_Box *Helper_Box_New(const char *name, const VC3 &position, const VC3 &size)=0;
	virtual IStorm3D_Helper_Sphere *Helper_Sphere_New(const char *name, const VC3 &_position, float radius)=0;
	virtual IStorm3D_Helper *SearchHelper(const char *name)=0;

	// Bones
	virtual IStorm3D_Bone *SearchBone(const char *name) = 0;

	// Set position/rotation/scale
	virtual void SetPosition(const VC3 &position)=0;
	virtual void SetRotation(const QUAT &rotation)=0;
	virtual void SetScale(const VC3 &scale)=0;
	
	// Lights
	virtual void SetSelfIllumination(const COL &color) = 0;
	//virtual void SetLighting(int index, const VC3 &position, const COL &color, float range) = 0;
	virtual void SetLighting(int index, signed short light_index_) = 0; 

	virtual void useAlwaysDirectional(bool use) = 0;
	virtual void SetDirectional(const VC3 &direction, float strength) = 0;
	virtual void ResetObjectLights() = 0;

	// Bone animations, returns false if incompatible bone structures
	virtual bool SetRandomAnimation(IStorm3D_BoneAnimation *animation) = 0;
	virtual bool SetAnimation(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, bool loop) = 0;
	virtual bool BlendToAnimation(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, int blend_time, bool loop = true) = 0;
	// Blending with animations
	virtual bool BlendWithAnimationIn(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, int blend_in_time, bool loop = true, float blend_factor = 1.f) = 0;
	virtual bool BlendWithAnimationOut(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, int blend_out_time) = 0;
	// Manual blending
	virtual void BlendBoneIn(IStorm3D_Bone *bone, const QUAT &rotation, int blend_time) = 0;
	virtual void BlendBoneOut(IStorm3D_Bone *bone, int blend_time) = 0;

	// Animation speeds
	virtual void SetAnimationSpeedFactor(IStorm3D_BoneAnimation *animation, float speedFactor) = 0;
	virtual void SetAnimationPaused(bool paused) = 0;
	virtual int GetAnimationTime(IStorm3D_BoneAnimation *animation) = 0;

	// Get position/rotation/scale
	virtual VC3 &GetPosition()=0;
	virtual QUAT &GetRotation()=0;
	virtual VC3 &GetScale()=0;

	// Get model matrices (recreates if not up to date)
	virtual MAT &GetMX()=0;
	virtual VC3 GetApproximatedPosition()=0;

	virtual void SetOccluded(bool occluded) = 0;
	virtual bool GetOccluded() const = 0;

	// Dummy data 
	virtual void SetCustomData(IStorm3D_Model_Data *data) = 0;
	virtual IStorm3D_Model_Data *GetCustomData() = 0;
	virtual void SetTypeFlag(unsigned int flag) = 0;
	virtual unsigned int GetTypeFlag() const = 0;

	// Iterators
	ICreate<IStorm3D_Model_Object*> *ITObject;
	ICreate<IStorm3D_Helper*> *ITHelper;

	// Test collision (to each object in model)
	virtual void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate = false)=0;
	virtual void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate = true)=0;
	virtual float GetRadius() const = 0;
	virtual const AABB &GetBoundingBox() const = 0;

	virtual bool hasBones() = 0;

	// Special properties
	virtual void SetNoCollision(bool no_collision)=0;	// Model is not used on collision detection if set
	virtual void GetVolume(VC3 &min, VC3 &max) = 0;
	virtual void GetVolumeApproximation(VC3 &min, VC3 &max) = 0;

	// Hacky things...
	virtual void MakeSkyModel() = 0;

	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_Model() {};
};



//------------------------------------------------------------------
// IStorm3D_Model_Object (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Model_Object
{

public:

	// Name stuff (used on model's SearchObject)
	virtual void SetName(const char *name)=0;
	virtual const char *GetName()=0;

	// Get/Set object's mesh (new in v2.6)
	virtual void SetMesh(IStorm3D_Mesh *mesh)=0;
	virtual IStorm3D_Mesh *GetMesh()=0;

	// Get object (global, linking applied) matrix (recreates if not up to date)
	virtual MAT &GetMXG()=0;
	virtual const Sphere &GetBoundingSphere() = 0;

	// Set position/rotation/scale
	virtual void SetPosition(const VC3 &position)=0;
	virtual void SetRotation(const QUAT &rotation)=0;
	virtual void SetScale(const VC3 &scale)=0;

	// Get position/rotation/scale
	virtual VC3 &GetPosition()=0;
	virtual QUAT &GetRotation()=0;
	virtual VC3 &GetScale()=0;
	virtual VC3 GetGlobalPosition()=0;
	virtual IStorm3D_Bone *GetParentBone() = 0;

	// Change special properties

	// why was this SetNoCollision commented out here?? -jpk
	virtual void SetNoCollision(bool no_collision)=0;	// Object is not used on collision detection if set

	virtual void SetNoRender(bool no_render)=0;			// Object is not rendered if set (but it's still used in collision)
	virtual void SetSphereCollisionOnly(bool sphere_collision_only)=0;	// Only bouding sphere is used to determine ray trace hits
	virtual void UpdateVisibility() = 0;

	virtual bool GetNoCollision() const = 0;
	virtual bool GetNoRender() const = 0;
	virtual bool IsLightObject() const = 0;
	virtual void SetAsLightObject() = 0;
	virtual void SetForceAlpha(float value) = 0;
	virtual void SetForceLightingAlpha(bool enable, float value) = 0;
	virtual void SetLight(int index, int id) = 0;
	virtual void SetSpotTransparencyFactor(float factor) = 0;

	virtual void EnableRenderPass(int renderPassBit) = 0;
	virtual void SetRenderPassParams(int renderPassBit, const VC3 &offset, const VC3 &scale) = 0;
	virtual void DisableRenderPass(int renderPassBit) = 0;

	virtual int GetRenderPassMask() = 0;
	virtual void SetRenderPassMask(int renderPassMask) = 0;

	virtual const AABB &GetBoundingBox() = 0;

	// Child fuctions (hierarchial linking)
	virtual void AddChild(IStorm3D_Model_Object *object)=0;
	virtual void AddChild(IStorm3D_Helper *helper)=0;
	virtual void RemoveChild(IStorm3D_Model_Object *object)=0;
	virtual void RemoveChild(IStorm3D_Helper *helper)=0;

	// Copy (v3). Copies everything, but: name(identification), childs(objects can have only 1 parent)
	// Do not copy object from a model to another.
	virtual void CopyFrom(IStorm3D_Model_Object *other, bool only_render_settings = false)=0;

	// Test collision
	virtual void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate = false)=0;
	virtual void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate = true)=0;

	// Virtual destructor (do not use, for Storm3D only)
	virtual ~IStorm3D_Model_Object() {};
};

// Implement this by yourself to add custom data to model
class IStorm3D_Model_Data
{
public:
	// Creator
	virtual ~IStorm3D_Model_Data() { }

	// Use pointer of static variable as class id
	virtual void *GetID() = 0;
};
