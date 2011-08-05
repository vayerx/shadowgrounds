// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatypes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Helper;
class IStorm3D_Helper_Point;
class IStorm3D_Helper_Vector;
class IStorm3D_Helper_Box;
class IStorm3D_Helper_Camera;
class IStorm3D_Bone;


//------------------------------------------------------------------
// IStorm3D_Helper - base class (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Helper
{

public:

	enum HTYPE 
	{
		HTYPE_POINT		=0,
		HTYPE_VECTOR	=1,
		HTYPE_BOX		=2,
		HTYPE_CAMERA	=3,
		HTYPE_SPHERE	=4
	};

	// Can be used instead of RTTI (dynamic_cast)
	virtual HTYPE GetHelperType()=0;

	// Name stuff (used on model's SearchHelper)
	virtual void SetName(const char *name)=0;
	virtual const char *GetName()=0;
	virtual IStorm3D_Bone *GetParentBone()=0;
	
	// Get helpers (local) TM
	virtual Matrix GetTM() = 0;
	virtual VC3 &GetGlobalPosition()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_Clear()=0;	// Removes all animation keys
	virtual void Animation_SetLoop(int end_time)=0;	// Set end_time to 0, if you do not want looping
	virtual void Animation_SetCurrentTime(int time)=0;

	// Virtual destructor (DO NOT USE THIS... Use Model's delete instead)
	virtual ~IStorm3D_Helper() {};
};



//------------------------------------------------------------------
// IStorm3D_Helper_Point (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Helper_Point : public IStorm3D_Helper
{

public:

	// Change params
	virtual void SetPosition(const VC3 &position)=0;

	// Get params
	virtual VC3 &GetPosition()=0;
	virtual VC3 &GetGlobalPosition()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewPositionKeyFrame(int time,const VC3 &position)=0;
};



//------------------------------------------------------------------
// IStorm3D_Helper_Vector (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Helper_Vector : public IStorm3D_Helper_Point
{

public:

	// Change params
	virtual void SetDirection(const VC3 &direction)=0;

	// Get params
	virtual VC3 &GetDirection()=0;
	virtual VC3 &GetGlobalDirection()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewDirectionKeyFrame(int time,const VC3 &dir)=0;
};



//------------------------------------------------------------------
// IStorm3D_Helper_Box (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Helper_Box : public IStorm3D_Helper_Point
{

public:

	// Change params
	virtual void SetSize(VC3 &size)=0;

	// Get params
	virtual VC3 &GetSize()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewSizeKeyFrame(int time,const VC3 &size)=0;
};



//------------------------------------------------------------------
// IStorm3D_Helper_Sphere (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Helper_Sphere : public IStorm3D_Helper_Point
{

public:

	// Change params
	virtual void SetRadius(float radius)=0;

	// Get params
	virtual float GetRadius()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewRadiusKeyFrame(int time,float radius)=0;
};



//------------------------------------------------------------------
// IStorm3D_Helper_Camera (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Helper_Camera : public IStorm3D_Helper_Vector
{

public:

	// Change params
	virtual void SetUpVector(const VC3 &upvec)=0;

	// Get params
	virtual VC3 &GetUpVector()=0;
	virtual VC3 &GetGlobalUpVector()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewUpVectorKeyFrame(int time,const VC3 &upvec)=0;
};


