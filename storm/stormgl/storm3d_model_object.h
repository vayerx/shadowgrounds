// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Model.h"
#include "storm3d_animation.h"
#include "storm3d.h"
#include "storm3d_resourcemanager.h"
#include "c2_sphere.h"
#include "c2_aabb.h"
#include "c2_oobb.h"


// default value for render pass bits, no bits set.
#define RENDER_PASS_MASK_VALUE_NONE 0


class Storm3D_ShaderManager;
class Storm3D_Bone;

//------------------------------------------------------------------
// Storm3D_Model_Object
//------------------------------------------------------------------
class Storm3D_Model_Object : public IStorm3D_Model_Object
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;
	Storm3D_ResourceManager &resourceManager;

	// Objects name (for model's object search)
	char *name;

	// Mesh
	Storm3D_Mesh *mesh;

	// Sets of child objects, lights and helpers
	set<IStorm3D_Model_Object*> child_objects;
	set<IStorm3D_Helper*> child_helpers;

	void InformChangeToChilds();	// Called when object changes

	// v3 optimization
	bool gpos_update_needed;
	VC3 gpos;

	// Transformation (position/rotation/scale)
	VC3 position;
	QUAT rotation;
	VC3 scale;
	MAT mx;
	MAT mxg;		// Global (multiplied with parent's mxg)
	bool mx_update;
	bool mxg_update;

	// Switch between these. Aaargh
	Storm3D_Model *parent_model;		// the model that owns this object
	Storm3D_Model_Object *parent;		// link (see childs)
	Storm3D_Bone *parent_bone;

	// Special properties
	bool no_collision;
	bool no_render;
	bool light_object;

	// Added for objects that are checked for sphere collision only in raytrace
	// (the face collision check won't be done if this is set)
	bool sphere_collision_only;

	// Creation/delete (only models use these)
	Storm3D_Model_Object(Storm3D *Storm3D2,const char *name,Storm3D_Model *_parent_model, Storm3D_ResourceManager &resourceManager);
	~Storm3D_Model_Object();

	mutable Sphere bounding_sphere;
	mutable bool sphere_ok;
	mutable AABB bounding_box;
	mutable bool box_ok;
	mutable OOBB object_bounding_box;
	mutable bool object_box_ok;

	int visibilityFlag;
	float force_alpha;
	bool force_lighting_alpha_enable;
	float force_lighting_alpha;
	//int light_index1;
	//int light_index2;
	short int light_index[LIGHT_MAX_AMOUNT];
	float spot_transparency_factor;
	bool distortion_only;

  int renderPassMask;
	VC3 renderPassOffset[RENDER_PASS_BITS_AMOUNT];
	VC3 renderPassScale[RENDER_PASS_BITS_AMOUNT];

	bool alphaTestPassConditional;
	int alphaTestValue;

public:
	float sort_data;
	int visibility_id;
	int real_visibility_id;

	bool isDistortionOnly() const { return distortion_only; }
	const Sphere &GetBoundingSphere();
	bool SphereOk() const { return sphere_ok; }
	const OOBB &GetObjectBoundingBox();
	bool ObjectBoundingBoxOk() const { return object_box_ok; }

	const AABB &GetBoundingBox();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();

	// Get object's mesh (new in v2.6)
	void SetMesh(IStorm3D_Mesh *mesh);
	IStorm3D_Mesh *GetMesh();

	// Get object matrices (recreates if not up to date)
	MAT &GetMXG();

	// Set position/rotation/scale
	void SetPosition(const VC3 &_position);
	void SetRotation(const QUAT &_rotation);
	void SetScale(const VC3 &_scale);

	// Get position/rotation/scale
	VC3 &GetPosition();
	QUAT &GetRotation();
	VC3 &GetScale();
	VC3 GetGlobalPosition();

	IStorm3D_Bone *GetParentBone();

	// Change special properties
	void SetNoCollision(bool no_collision);	// Object is not used on collision detection if set
	void SetNoRender(bool no_render);		// Object is not rendered if set (but it's still used in collision)
	void SetSphereCollisionOnly(bool sphere_collision_only);	// raytrace collision check done for bounding sphere only, not for faces
	void UpdateVisibility();

	// Get special properties
	bool GetNoCollision() const;
	bool GetNoRender() const;
	bool IsLightObject() const;
	void SetAsLightObject();
	void SetForceAlpha(float value) { force_alpha = value; };
	void SetForceLightingAlpha(bool enable, float value) { force_lighting_alpha_enable = enable; force_lighting_alpha = (enable) ? value : 0.f; };
	void SetLight(int index, int id)
	{ 
		if(index >= 0 && index < LIGHT_MAX_AMOUNT)
			light_index[index] = id;
	};

	virtual void EnableRenderPass(int renderPassBit);
	virtual void SetRenderPassParams(int renderPassBit, const VC3 &offset, const VC3 &scale);
	virtual void DisableRenderPass(int renderPassBit);

	virtual void SetAlphaTestPassParams(bool conditional, int alphaValue);

	// these are here just for object cloning outside storm...
	virtual int GetRenderPassMask();
	virtual void SetRenderPassMask(int renderPassMask);

	void SetSpotTransparencyFactor(float factor) { spot_transparency_factor = factor; };

	// Child fuctions
	void AddChild(IStorm3D_Model_Object *object);
	void AddChild(IStorm3D_Helper *helper);
	void RemoveChild(IStorm3D_Model_Object *object);
	void RemoveChild(IStorm3D_Helper *helper);

	// Copy (v3). Copies everything, but: name(identification), childs(objects can have only 1 parent)
	// Do not copy object from a model to another.
	void CopyFrom(IStorm3D_Model_Object *other, bool only_render_settings = false);
	
	// Test ray collision
	void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate = false);
	void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate);

	friend class Storm3D_Model;
	friend class Storm3D_Scene;
	friend class Storm3D_Mesh;
	friend class Storm3D_ShaderManager;
	friend class Storm3D_Bone;
	friend struct Storm3D_TerrainModelsData;
};


