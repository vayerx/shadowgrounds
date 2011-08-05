// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef STORM3D_BONE_H
#define STORM3D_BONE_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // debug info truncate
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Bone.h"

#include "Storm3D_Datatypes.h"
#include <vector>

//------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------
class IStorm3D_Helper;
class IStorm3D_Object;
class Storm3D_Object;
class Storm3D_Helper_AInterface;

typedef std::pair<int, Rotation> BoneRotationKey;
typedef std::pair<int, Vector> BonePositionKey;

//------------------------------------------------------------------
// SnowStorm_BoneAnimation
//------------------------------------------------------------------
class Storm3D_BoneAnimation: public IStorm3D_BoneAnimation
{
	// [bone_index][key_index] which contains (time/property) pair
	std::vector<std::vector<BoneRotationKey> > bone_rotations;
	std::vector<std::vector<BonePositionKey> > bone_positions;
	
	int bone_id;
	int loop_time; // in ms
	int reference_count;

	bool successfullyLoaded;
	
	// Not implemented
	Storm3D_BoneAnimation(const Storm3D_BoneAnimation &animation);
	Storm3D_BoneAnimation &operator = (const Storm3D_BoneAnimation &animation);

	// Use reference methods instead
	~Storm3D_BoneAnimation();
	
public:	
	Storm3D_BoneAnimation(const char *file_name);

	// Reference counting
	void AddReference();
	void Release();

	// Animation length (ms)
	int GetLength();

	// Bone id
	int GetId();

	// Some way to query if the animation was actually loaded or not
	// --jpk
	bool WasSuccessfullyLoaded() const;

	// Result contain interpolated value IFF returns true
	bool GetRotation(int bone_index, int time, Rotation *result) const;
	bool GetPosition(int bone_index, int time, Vector *result) const;
};

//------------------------------------------------------------------
// SnowStorm_Bone
//------------------------------------------------------------------
class Storm3D_Bone: public IStorm3D_Bone
{
	char *name;

	// Original data
	Vector position;
	Rotation rotation;
	Matrix original_inverse_tm;

	// Special properties
	float length;
	float thickness;

	// Current transform
	Matrix model_tm; // normal tm in model space
	Matrix parent_tm; // Parent models tm
	mutable Matrix global_tm; // Global tm
	mutable Matrix vertex_tm; // transform vertices with this

	mutable bool global_tm_ok; // For lazy evaluation
	mutable bool vertex_tm_ok; 
	bool has_childs;

	// Animated properties (relative to parent bone)
	Rotation current_rotation;
	Vector current_position;

	// State
	int parent_index;

	// Childs
	std::set<Storm3D_Model_Object *> objects;
	std::set<Storm3D_Helper_AInterface *> helpers;

	VC3 forcePosition;
	QUAT forceRotation;
	bool useForceTransform;

public:
	Storm3D_Bone();
	~Storm3D_Bone();

	QUAT GetOriginalGlobalRotation();

	// Get transforms
	const Matrix &GetVertexTransform() const 
	{ 
		if(vertex_tm_ok)
			return vertex_tm; 
		else
		{
			vertex_tm = original_inverse_tm;
			vertex_tm.Multiply(model_tm);

			vertex_tm_ok = true;
			return vertex_tm;
		}
	}

	const Matrix &GetTM() const
	{
		if(!global_tm_ok)
		{
			global_tm = model_tm;
			global_tm.Multiply(parent_tm);
			global_tm_ok = true;
		}

		return global_tm;
	}

	const Matrix &GetLocalTM() const
	{
		return model_tm;
	}

	// Set properties
	void SetOriginalProperties(const Vector &position, const Rotation &rotation, const Vector &model_position, const Rotation &model_rotation);
	void SetSpecialProperties(float length, float thickness);
	void SetParentIndex(int index);

	// Get properties
	float GetLenght() const
	{
		return length;
	}
	float GetThickness() const
	{
		return thickness;
	}
	int GetParentIndex() const
	{
		return parent_index;
	}

	const VC3 &getPosition() const
	{
		return current_position;
	}

	// Animating
	void ResetAnimations() // Call before applying animations
	{
		// Reset animation to default pose
		current_position = position;
		current_rotation = rotation;
	}

	void AnimatePosition(const Vector &position, float interpolate_amount = 0, bool interpolate = false);
	void AnimateRotation(const Rotation &rotation, float interpolate_amount = 0, bool interpolate = false);
	void AnimateRelativeRotation(const Rotation &rotation, float interpolate_amount = 0, bool interpolate = false);

	// Name
	void SetName(const char *name);
	const char *GetName() const;

	// Child fuctions
	void ParentMoved(const Matrix &parent_tm_);
	void ParentPositionMoved(const VC3 &pos);
	void InformChangeToChilds();

	// All types can be attached to this
	// Passed as interfaces, needs casting on implementation. Ugh
	void AddChild(IStorm3D_Model_Object *object);
	void AddChild(IStorm3D_Helper *helper);
	void RemoveChild(IStorm3D_Model_Object *object);
	void RemoveChild(IStorm3D_Helper *helper);

	void SetForceTransformation(bool useForceSettings, const VC3 &position, const QUAT &rotation);

	const MAT &GetMXG() const
	{
		return GetTM();
	}

	static void TransformBones(std::vector<Storm3D_Bone*> *bones);
	friend class Storm3D_Model;
};

#endif // STORM3D_BONE_H
