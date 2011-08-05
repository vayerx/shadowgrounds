// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Helper.h"
#include "storm3d_animation.h"

class Storm3D_Bone;

//------------------------------------------------------------------
// Storm3D_Helper_AInterface
//------------------------------------------------------------------
class Storm3D_Helper_AInterface
{

public:

	// Helper's name (for model's name search)
	char *name;

	VC3 position;
	VC3 position_global;

	Storm3D_Model *parent_model;			// the model that owns this helper
	Storm3D_Model_Object *parent_object;	// link (NULL if parent is model)
	Storm3D_Bone *parent_bone;

	bool update_globals;
	IStorm3D_Helper *helper;

	// Destructor/Constructor
	virtual ~Storm3D_Helper_AInterface();
	Storm3D_Helper_AInterface(const char *name,Storm3D_Model *parent_model,VC3 &position, IStorm3D_Helper *helper);
};



//------------------------------------------------------------------
// Storm3D_Helper_Point
//------------------------------------------------------------------
class Storm3D_Helper_Point : public IStorm3D_Helper_Point, public Storm3D_Helper_AInterface
{

	// Animation keyframes
	Storm3D_Helper_Animation animation;

	// Updates globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Helper::HTYPE GetHelperType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();
	IStorm3D_Bone *GetParentBone();

	// Create
	Storm3D_Helper_Point(const char *name,Storm3D_Model *_parent_model,VC3 _position);

	// Change params
	void SetPosition(const VC3 &_position);

	// Get params
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();

	// Get helpers TM
	Matrix GetTM();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);

	friend struct Storm3D_TerrainModelsData;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



//------------------------------------------------------------------
// Storm3D_Helper_Vector
//------------------------------------------------------------------
class Storm3D_Helper_Vector : public IStorm3D_Helper_Vector, public Storm3D_Helper_AInterface
{
	// Animation keyframes
	Storm3D_Helper_Animation animation;

	VC3 direction;
	VC3 direction_global;	// Direction (global)

	// Updates globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Helper::HTYPE GetHelperType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();
	IStorm3D_Bone *GetParentBone();

	// Create
	Storm3D_Helper_Vector(const char *name,Storm3D_Model *_parent_model,VC3 _position,VC3 _direction);

	// Change params
	void SetPosition(const VC3 &_position);
	void SetDirection(const VC3 &_direction);

	// Get params
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();
	VC3 &GetDirection();
	VC3 &GetGlobalDirection();

	// Get helpers TM
	Matrix GetTM();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);
	void Animation_AddNewDirectionKeyFrame(int time,const VC3 &dir);

	friend struct Storm3D_TerrainModelsData;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



//------------------------------------------------------------------
// Storm3D_Helper_Camera
//------------------------------------------------------------------
class Storm3D_Helper_Camera : public IStorm3D_Helper_Camera, public Storm3D_Helper_AInterface
{
	// Animation keyframes
	Storm3D_Helper_Animation animation;

	VC3 direction;
	VC3 direction_global;	// Direction (global)
	VC3 upvec;
	VC3 upvec_global;		// Upvector (global)

	// Updates globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Helper::HTYPE GetHelperType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();
	IStorm3D_Bone *GetParentBone();

	// Create
	Storm3D_Helper_Camera(const char *name,Storm3D_Model *_parent_model,VC3 _position,VC3 _direction,VC3 _upvec);

	// Change params
	void SetPosition(const VC3 &_position);
	void SetDirection(const VC3 &_direction);
	void SetUpVector(const VC3 &upvec);

	// Get params
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();
	VC3 &GetDirection();
	VC3 &GetGlobalDirection();
	VC3 &GetUpVector();
	VC3 &GetGlobalUpVector();

	// Get helpers TM
	Matrix GetTM();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);
	void Animation_AddNewDirectionKeyFrame(int time,const VC3 &dir);
	void Animation_AddNewUpVectorKeyFrame(int time,const VC3 &up);

	friend struct Storm3D_TerrainModelsData;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



//------------------------------------------------------------------
// Storm3D_Helper_Box
//------------------------------------------------------------------
class Storm3D_Helper_Box : public IStorm3D_Helper_Box, public Storm3D_Helper_AInterface
{
	// Animation keyframes
	Storm3D_Helper_Animation animation;

	VC3 size;

	// Updates globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Helper::HTYPE GetHelperType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();
	IStorm3D_Bone *GetParentBone();

	// Create
	Storm3D_Helper_Box(const char *name,Storm3D_Model *_parent_model,VC3 _position,VC3 _size);

	// Change params
	void SetPosition(const VC3 &_position);
	void SetSize(VC3 &_size);

	// Get params
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();
	VC3 &GetSize();

	// Get helpers TM
	Matrix GetTM();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);
	void Animation_AddNewSizeKeyFrame(int time,const VC3 &size);

	friend struct Storm3D_TerrainModelsData;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



//------------------------------------------------------------------
// Storm3D_Helper_Sphere
//------------------------------------------------------------------
class Storm3D_Helper_Sphere : public IStorm3D_Helper_Sphere, public Storm3D_Helper_AInterface
{
	// Animation keyframes
	Storm3D_Helper_Animation animation;

	float radius;

	// Updates globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Helper::HTYPE GetHelperType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();
	IStorm3D_Bone *GetParentBone();

	// Create
	Storm3D_Helper_Sphere(const char *name,Storm3D_Model *_parent_model,VC3 _position,float _radius);

	// Change params
	void SetPosition(const VC3 &_position);
	void SetRadius(float _radius);

	// Get params
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();
	float GetRadius();

	// Get helpers TM
	Matrix GetTM();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);
	void Animation_AddNewRadiusKeyFrame(int time,float radius);

	friend struct Storm3D_TerrainModelsData;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



