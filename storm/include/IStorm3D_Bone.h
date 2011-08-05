#pragma once

// Storm3D includes 
#include "Storm3D_Common.h"
#include <DatatypeDef.h>

/*
Bone stuff

Animations are created through IStorm3D interface
	-> IStorm3D::CreateBoneAnimation("file_name")
Set to model using IStorm3D_Model::SetAnimation(animation, looping).

Animation class is reference counted so use Release() instead of
deleting it directly.

ToDo:
	- Methods for accessing bones
*/

class IStorm3D_Helper;
class IStorm3D_Model_Object;

//------------------------------------------------------------------
// ISnowStorm_BoneAnimation
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_BoneAnimation
{
protected:
	virtual ~IStorm3D_BoneAnimation() {}

public:
	virtual void Release() = 0; // Use this for deleting
};

//------------------------------------------------------------------
// ISnowStorm_Bone
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Bone
{
public:
	virtual void AddChild(IStorm3D_Model_Object *object) = 0;
	virtual void AddChild(IStorm3D_Helper *helper) = 0;
	virtual void RemoveChild(IStorm3D_Model_Object *object) = 0;
	virtual void RemoveChild(IStorm3D_Helper *helper) = 0;

	virtual void SetForceTransformation(bool useForceSettings, const VC3 &position, const QUAT &rotation) = 0;
	virtual const MAT &GetMXG() const = 0;

	virtual ~IStorm3D_Bone() {}
};
