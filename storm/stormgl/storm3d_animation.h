/*

  Storm3D v3 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Light.h"
#include "IStorm3D_Helper.h"



//------------------------------------------------------------------
// Storm3D_Model_Object_Animation
//------------------------------------------------------------------
class Storm3D_Model_Object_Animation
{
	// Owner object
	Storm3D_Model_Object *owner;

	// Link to first keyframes of position, rotation and scale
	// (NULL if not animated)
	Storm3D_KeyFrame_Vector *first_poskey;
	Storm3D_KeyFrame_Rotation *first_rotkey;
	Storm3D_KeyFrame_Scale *first_scalekey;

	int end_time;			// Time to loop to beginning (0 if no looping at all)
	int time_now;			// Time counter	(set to 0, when end_time is reached)

public:

	// Set's objects values according to animation position (and adds animation
	// position)
	void Apply(Storm3D_Scene *scene);

	void Clear();	// Removes all animation keys
	void AddNewPositionKeyFrame(int time,const VC3 &position);
	void AddNewRotationKeyFrame(int time,const QUAT &rotation);
	void AddNewScaleKeyFrame(int time,const VC3 &scale);
	void AddNewMeshKeyFrame(int time,const Storm3D_Vertex *vertexes);

	void SetLoop(int end_time);
	void SetCurrentTime(int time);

	// Creation/deletion
	Storm3D_Model_Object_Animation(Storm3D_Model_Object *owner);
	~Storm3D_Model_Object_Animation();

	friend class Storm3D_Mesh;
};



//------------------------------------------------------------------
// Storm3D_Mesh_Animation
//------------------------------------------------------------------
class Storm3D_Mesh_Animation
{
	// Owner mesh
	Storm3D_Mesh *owner;

	// Link to first keyframes (NULL if not animated)
	Storm3D_KeyFrame_Mesh *first_meshkey;

public:

	// Set's meshes values according to animation position
	void Apply(Storm3D_Scene *scene,int time);

	void Clear();	// Removes all animation keys
	void AddNewMeshKeyFrame(int time,const Storm3D_Vertex *vertexes);

	// Creation/deletion
	Storm3D_Mesh_Animation(Storm3D_Mesh *owner);
	~Storm3D_Mesh_Animation();
};



//------------------------------------------------------------------
// Storm3D_Light_Animation
//------------------------------------------------------------------
class Storm3D_Light_Animation
{
	// Owner object
	IStorm3D_Light *owner;

	// Link to first keyframes of position, rotation and scale
	// (NULL if not animated)
	Storm3D_KeyFrame_Vector *first_poskey;
	Storm3D_KeyFrame_Vector *first_dirkey;
	Storm3D_KeyFrame_Luminance *first_lumkey;
	Storm3D_KeyFrame_Cones *first_conekey;

	int end_time;			// Time to loop to beginning (0 if no looping at all)
	int time_now;			// Time counter	(set to 0, when end_time is reached)

public:

	// Set's objects values according to animation position (and adds animation
	// position)
	void Apply(Storm3D_Scene *scene);

	void Clear();	// Removes all animation keys
	void AddNewPositionKeyFrame(int time,const VC3 &position);
	void AddNewDirectionKeyFrame(int time,const VC3 &dir);
	void AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color);
	void AddNewConeKeyFrame(int time,float inner,float outer);

	void SetLoop(int end_time);
	void SetCurrentTime(int time);

	// Creation/deletion
	Storm3D_Light_Animation(IStorm3D_Light *owner);
	~Storm3D_Light_Animation();
};



//------------------------------------------------------------------
// Storm3D_Helper_Animation
//------------------------------------------------------------------
class Storm3D_Helper_Animation
{
	// Owner object
	IStorm3D_Helper *owner;

	// Link to first keyframes of position, rotation and scale
	// (NULL if not animated)
	Storm3D_KeyFrame_Vector *first_poskey;
	Storm3D_KeyFrame_Vector *first_dirkey;	// and size and radius(x)
	Storm3D_KeyFrame_Vector *first_upveckey;

	int end_time;			// Time to loop to beginning (0 if no looping at all)
	int time_now;			// Time counter	(set to 0, when end_time is reached)

public:

	// Set's objects values according to animation position (and adds animation
	// position)
	void Apply(Storm3D_Scene *scene);

	void Clear();	// Removes all animation keys
	void AddNewPositionKeyFrame(int time,const VC3 &position);
	void AddNewDirectionKeyFrame(int time,const VC3 &dir);
	void AddNewUpVectorKeyFrame(int time,const VC3 &upvec);

	void SetLoop(int end_time);
	void SetCurrentTime(int time);

	// Creation/deletion
	Storm3D_Helper_Animation(IStorm3D_Helper *owner);
	~Storm3D_Helper_Animation();
};



//------------------------------------------------------------------
// Storm3D_KeyFrame_Vector (position, direction, up)
//------------------------------------------------------------------
struct Storm3D_KeyFrame_Vector
{
	int keytime;					// Time (in millisecs)
	VC3 position;		// New position

	// Link to next keyframe (NULL if this is the last one)
	Storm3D_KeyFrame_Vector *next_key;

	// Creation
	Storm3D_KeyFrame_Vector(int time,const VC3 &position);
	~Storm3D_KeyFrame_Vector();
};



//------------------------------------------------------------------
// Storm3D_KeyFrame_Rotation
//------------------------------------------------------------------
struct Storm3D_KeyFrame_Rotation
{
	int keytime;					// Time (in millisecs)
	QUAT rotation;		// New position

	// Link to next keyframe (NULL if this is the last one)
	Storm3D_KeyFrame_Rotation *next_key;

	// Creation
	Storm3D_KeyFrame_Rotation(int time,const QUAT &rotation);
	~Storm3D_KeyFrame_Rotation();
};



//------------------------------------------------------------------
// Storm3D_KeyFrame_Scale
//------------------------------------------------------------------
struct Storm3D_KeyFrame_Scale
{
	int keytime;					// Time (in millisecs)
	VC3 scale;			// New scale

	// Link to next keyframe (NULL if this is the last one)
	Storm3D_KeyFrame_Scale *next_key;

	// Creation
	Storm3D_KeyFrame_Scale(int time,const VC3 &scale);
	~Storm3D_KeyFrame_Scale();
};



//------------------------------------------------------------------
// Storm3D_KeyFrame_Mesh
//------------------------------------------------------------------
struct Storm3D_KeyFrame_Mesh
{
	// Owner object
	Storm3D_Mesh *owner;

	int keytime;					// Time (in millisecs)
	Storm3D_Vertex *vertexes;		// New vertexes (same faces)

	// Link to next keyframe (NULL if this is the last one)
	Storm3D_KeyFrame_Mesh *next_key;

	// Creation
	Storm3D_KeyFrame_Mesh(int time,Storm3D_Mesh *owner,const Storm3D_Vertex *vertexes);
	~Storm3D_KeyFrame_Mesh();
};



//------------------------------------------------------------------
// Storm3D_KeyFrame_Luminance
//------------------------------------------------------------------
struct Storm3D_KeyFrame_Luminance
{
	int keytime;					// Time (in millisecs)
	float multiplier;
	float decay;
	COL color;

	// Link to next keyframe (NULL if this is the last one)
	Storm3D_KeyFrame_Luminance *next_key;

	// Creation
	Storm3D_KeyFrame_Luminance(int time,float multiplier,float decay,const COL &_color);
	~Storm3D_KeyFrame_Luminance();
};



//------------------------------------------------------------------
// Storm3D_KeyFrame_Cones
//------------------------------------------------------------------
struct Storm3D_KeyFrame_Cones
{
	int keytime;					// Time (in millisecs)
	float inner;
	float outer;

	// Link to next keyframe (NULL if this is the last one)
	Storm3D_KeyFrame_Cones *next_key;

	// Creation
	Storm3D_KeyFrame_Cones(int time,float inner,float outer);
	~Storm3D_KeyFrame_Cones();
};



