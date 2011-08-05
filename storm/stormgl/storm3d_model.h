// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Model.h"
#include <vector>
#include <list>
#include <string>
#include <c2_aabb.h>
#include <deque>

class Storm3D_Bone;
class Storm3D_BoneAnimation;
class IStorm3D_BoneAnimation;
class Storm3D_ShaderManager;
class Storm3D_Model;

class IModelObserver
{
public:
	virtual ~IModelObserver() {}

	virtual void updatePosition(const VC3 &position) = 0;
	virtual void updateRadius(float radius) = 0;
	virtual void updateVisibility(Storm3D_Model_Object *) = 0;
	virtual void remove(Storm3D_Model *) = 0;
	virtual void remove(Storm3D_Model_Object *) = 0;
	virtual void destroy(Storm3D_Model *) = 0;
};

//------------------------------------------------------------------
// Model_BoneAnimation
//	-> Stores per instance data for each animation
//	-> Move this to storm3d_bone.h?
//------------------------------------------------------------------
class Model_BoneAnimation
{
	int animation_time;
	int elapsed_time;
	int blend_time;
	int speed_factor;

	float blend_factor;

	bool animation_loop;
	Storm3D_BoneAnimation *transition;
	Storm3D_BoneAnimation *animation;

public:
	enum AnimationState { Normal = 0, BlendIn = 1, BlendOut = 2 };

private:
	AnimationState state;

public:
	Model_BoneAnimation();
	Model_BoneAnimation(const Model_BoneAnimation &animation_);
	Model_BoneAnimation(Storm3D_BoneAnimation *transition, Storm3D_BoneAnimation *animation, bool looping = true, int blend_time = 0, int animation_time = 0);
	~Model_BoneAnimation();

	void SetBlendFactor(float speed_factor);
	void SetSpeedFactor(float speed_factor);

	void AdvanceAnimation(int time_delta, bool doUpdate);
	void SetLooping(bool loop);
	void SetState(AnimationState state, int blend_time);

	// Operators
	Model_BoneAnimation &operator = (const Model_BoneAnimation &animation);
	bool operator < (const Model_BoneAnimation &animation) const; // based on blend time

	// Result contain interpolated value IFF returns true
	bool GetRotation(int bone_index, Rotation *result) const;
	bool GetPosition(int bone_index, Vector *result) const;

	// Get misc properties
	bool GetBlendProperties(float *blend_factor) const;
	int GetAnimationTime() const;
	Storm3D_BoneAnimation *GetAnimation() const;

	bool HasAnimation() const;
	bool HasEnded() const;
	AnimationState GetState() const;

	friend class Storm3D_Model;
};

class Model_BoneBlend
{
	Model_BoneAnimation::AnimationState state;
	QUAT rotation;
	int bone_index;
	int blend_time;
	int elapsed_time;

public:	
	Model_BoneBlend();
	Model_BoneBlend(int bone_index, const QUAT rotation, int blend_time);

	void AdvanceAnimation(int time_delta);
	void SetState(Model_BoneAnimation::AnimationState state, int blend_time);

	// Result contain interpolated value IFF returns true
	bool GetBlendProperties(float *blend_factor) const;
	bool GetRotation(int bone_index, Rotation *result) const;

	Model_BoneAnimation::AnimationState GetState() const;
	bool HasEnded() const;
	int GetIndex() const;
};

//------------------------------------------------------------------
// Storm3D_Model
//------------------------------------------------------------------
class Storm3D_Model : public IStorm3D_Model
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	VC3 position;
	QUAT rotation;
	VC3 scale;	
	MAT mx;
	bool mx_update;
	bool no_collision;

	// Sets of objects, lights, helpers & bones in model
	set<IStorm3D_Model_Object*> objects;
	set<IStorm3D_Helper*> helpers;
	vector<Storm3D_Bone*> bones;

	set<Storm3D_Model_Object *> collision_objects;
	vector<Storm3D_Model_Object *> objects_array;
	set<IStorm3D_Model_Object *> light_objects;

	// Bone animations
	//Model_BoneAnimation current_animation;
	std::list<Model_BoneAnimation> normal_animations;
	std::list<Model_BoneAnimation> blending_with_animations;
	std::list<Model_BoneBlend> manual_blendings;

	bool animation_paused;

	// Bone id's
	int bone_boneid;
	int model_boneid;

	int lodLevel;

	// Client data
	IStorm3D_Model_Data *custom_data;
	IModelObserver *observer;

	void InformChangeToChilds();	// Called when model changes

	// Creation/delete (only Storm3D uses these)
	Storm3D_Model(Storm3D *Storm3D2);
	~Storm3D_Model();

	// Model bounding box
	float bounding_radius;
	float original_bounding_radius;
	COL self_illumination;

	bool cast_shadows;
	bool bone_collision;
	unsigned int type_flag;
	bool terrain_object;
	bool terrain_lightmapped_object;
	bool terrain_inbuilding_object;
	int terrainInstanceId;
	int terrainModelId;

	signed short light_index[LIGHT_MAX_AMOUNT];

	VC3 sun_direction;
	float sun_strength;
	bool always_use_sun;

	bool occluded;

	std::string effectTextureName;
	mutable AABB bounding_box;
	mutable bool box_ok;

	float max_scale;

	bool use_cylinder_collision;
	VC3 approx_position;

	bool need_cull_adding;

#ifdef WORLD_FOLDING_ENABLED
	const MAT *mfold;
	const int *mfold_key;
	int last_mfold_key_value;
	MAT mx_folded;
#endif

	bool skyModel;

public:

	void updateEffectTexture();
	bool fits(const AABB &area);

	// Create empty / load (".S3D"-file) / copy
	bool LoadS3D(const char *filename);
	bool LoadBones(const char *filename);
	
	//void CopyModel(IStorm3D_Model *other);
	void Empty(bool leave_geometry = false, bool leave_bones = false);

	IStorm3D_Model *GetClone(bool cloneGeometry, bool cloneHelpers, bool cloneBones) const;

	// Objects.
	IStorm3D_Model_Object *Object_New(const char *name);
	void Object_Delete(IStorm3D_Model_Object *object);
	IStorm3D_Model_Object *SearchObject(const char *name);

	void CastShadows(bool shadows);
	void EnableBoneCollision(bool enable);
	void ForceBoneUpdate() { AdvanceAnimation(0); }
	void FreeMemoryResources();

	bool hasCastShadows() const { return cast_shadows; }

	// Helpers
	void Helper_Delete(IStorm3D_Helper *help);
	IStorm3D_Helper_Point *Helper_Point_New(const char *name, const VC3 &_position);
	IStorm3D_Helper_Vector *Helper_Vector_New(const char *name, const VC3 &_position, const VC3 &_direction);
	IStorm3D_Helper_Camera *Helper_Camera_New(const char *name, const VC3 &_position, const VC3 &_direction, const VC3 &_up);
	IStorm3D_Helper_Box *Helper_Box_New(const char *name, const VC3 &_position, const VC3 &_size);
	IStorm3D_Helper_Sphere *Helper_Sphere_New(const char *name, const VC3 &_position, float radius);

	IStorm3D_Helper *SearchHelper(const char *name);

	// Bones
	IStorm3D_Bone *SearchBone(const char *name);

	// Bone animations
	bool SetRandomAnimation(IStorm3D_BoneAnimation *animation);
	bool SetAnimation(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, bool loop);
	bool BlendToAnimation(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, int blend_time, bool loop = true);
	// Blending with animations.
	bool BlendWithAnimationIn(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, int blend_in_time, bool loop = true, float blend_factor = 1.f);
	bool BlendWithAnimationOut(IStorm3D_BoneAnimation *transition, IStorm3D_BoneAnimation *animation, int blend_out_time);
	// Manual
	void BlendBoneIn(IStorm3D_Bone *bone, const QUAT &rotation, int blend_time);
	void BlendBoneOut(IStorm3D_Bone *bone, int blend_time);

	// Animation speeds
	void SetAnimationSpeedFactor(IStorm3D_BoneAnimation *animation, float speedFactor);
	void SetAnimationPaused(bool paused);
	int GetAnimationTime(IStorm3D_BoneAnimation *animation);

	// Bone implementation
	void AdvanceAnimation(int time_delta);
	void ApplyAnimations();

	// Set position/rotation/scale
	void SetPosition(const VC3 &_position);
	void SetRotation(const QUAT &_rotation);
	void SetScale(const VC3 &_scale);
	void SetSelfIllumination(const COL &color) { self_illumination = color; }
	
	void SetLighting(int index, signed short light_index_) 
	{ 
		if(index >= 0 && index < LIGHT_MAX_AMOUNT)
			light_index[index] = light_index_;
	}

	void SetDirectional(const VC3 &direction, float strength) { sun_direction = direction; sun_strength = strength; }
	void useAlwaysDirectional(bool use) { always_use_sun = use; }
	const COL &GetSelfIllumination() { return self_illumination; }
	void ResetObjectLights();

	// Get position/rotation/scale
	VC3 &GetPosition();
	QUAT &GetRotation();
	VC3 &GetScale();

	// Special properties
	void SetNoCollision(bool no_collision);
	bool GetNoCollision() const { return no_collision; }

	// Special properties
	void SetOccluded(bool occluded) { this->occluded = occluded; }
	bool GetOccluded() const { return occluded; }

	// Get model matrices (recreates if not up to date)
	MAT &GetMX();
	VC3 GetApproximatedPosition();

	void SetCustomData(IStorm3D_Model_Data *data);
	IStorm3D_Model_Data *GetCustomData();
	void SetTypeFlag(unsigned int flag);
	unsigned int GetTypeFlag() const;

	// update model's bouding sphere radius (called by model_objects)
	void updateRadiusToContain(const VC3 &pos, float radius);

	// Test ray collision (to each object in model)
	void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate = false);
	void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate);
	float GetRadius() const { return bounding_radius * max_scale; }
	const AABB &GetBoundingBox() const;

	bool hasBones ( );

	void RemoveCollision(Storm3D_Model_Object *object);
	void SetCollision(Storm3D_Model_Object *object);
	void GetVolume(VC3 &min, VC3 &max);
	void GetVolumeApproximation(VC3 &min, VC3 &max);

	void MakeSkyModel();

	friend class Storm3D_Scene;
	friend class Storm3D;
	friend class Storm3D_Model_Object;
	friend class Storm3D_Mesh;
	friend class TRBlock;
	friend class Storm3D_ShaderManager;
	friend class Storm3D_TerrainModels;
	friend struct Storm3D_TerrainModelsData;
	friend class Storm3D_TerrainGroup;
};
