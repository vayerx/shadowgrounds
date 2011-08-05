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
#include "storm3d_animation.h"



//------------------------------------------------------------------
// Storm3D_Light_AInterface
//------------------------------------------------------------------
class Storm3D_Light_AInterface
{

public:

	// Light's name (for model's name search)
	char *name;

	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	COL color;
	float multiplier;
	float decay;

	Storm3D_Model *parent_model;			// the model that owns this light
	Storm3D_Model_Object *parent_object;	// link (NULL if parent is model)

	bool update_globals;

	// Destructor/Constructor
	Storm3D_Light_AInterface(Storm3D *Storm3D2,const char *name,Storm3D_Model *parent_model,COL &color,float multiplier,float decay);
	virtual ~Storm3D_Light_AInterface();
};



//------------------------------------------------------------------
// Storm3D_Light_Directional
//------------------------------------------------------------------
class Storm3D_Light_Directional : public IStorm3D_Light_Directional, public Storm3D_Light_AInterface
{
	// Animation keyframes
	Storm3D_Light_Animation animation;

	VC3 direction;			// Direction in model
	VC3 direction_global;	// Direction (global)

	// (Virtual) apply
	void Apply(int num);

	// Update globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Light::LTYPE GetLightType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();

	// Create
	Storm3D_Light_Directional(Storm3D *Storm3D2,const char *name,Storm3D_Model *_parent_model,
		COL &_color,float multiplier,VC3 &_direction);
	
	~Storm3D_Light_Directional();

	// Change params
	void SetColor(COL &_color);
	void SetMultiplier(float _multiplier);
	void SetDecay(float _decay);
	void SetDirection(VC3 &_direction);

	// Get params
	COL &GetColor();
	float GetMultiplier();
	float GetDecay();
	VC3 &GetDirection();
	VC3 &GetGlobalDirection();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color);
	void Animation_AddNewDirectionKeyFrame(int time,const VC3 &dir);

	friend class Storm3D_Scene_LightHandler;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



//------------------------------------------------------------------
// Storm3D_Light_Point
//------------------------------------------------------------------
class Storm3D_Light_Point : public IStorm3D_Light_Point, public Storm3D_Light_AInterface
{
	// Animation keyframes
	Storm3D_Light_Animation animation;

	VC3 position;		// Position in model
	VC3 position_global;	// Position (global)

	//float radius;					// Visibility range

	Storm3D_LensFlare *lflare;		// Lensflare (NULL=none)
	float flarevis;					// Flare visibility "counter" (1.0=100% visible, 0.0=invisible)

	// (Virtual) apply
	void Apply(int num);

	// Update globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	LTYPE GetLightType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();

	// Create
	Storm3D_Light_Point(Storm3D *Storm3D2,const char *name,Storm3D_Model *_parent_model,COL &_color,
		float multiplier,float decay,VC3 &_position,Storm3D_LensFlare *_lflare=NULL);

	~Storm3D_Light_Point();

	// Change params
	void SetColor(COL &_color);
	void SetMultiplier(float _multiplier);
	void SetDecay(float _decay);
	void SetPosition(VC3 &_position);
	void SetLensFlare(IStorm3D_LensFlare *_lflare);

	// Get params
	COL &GetColor();
	float GetMultiplier();
	float GetDecay();
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();
	IStorm3D_LensFlare *GetLensFlare();

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);

	friend class Storm3D_Scene_LightHandler;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



//------------------------------------------------------------------
// Storm3D_Light_Spot
//------------------------------------------------------------------
class Storm3D_Light_Spot : public IStorm3D_Light_Spot, public Storm3D_Light_AInterface
{
	// Animation keyframes
	Storm3D_Light_Animation animation;

	VC3 position;		// Position in model
	VC3 position_global;	// Position (global)

	VC3 direction;			// Direction in model
	VC3 direction_global;	// Direction (global)

	//float radius;					// Visibility range

	Storm3D_LensFlare *lflare;		// Lensflare (NULL=none)
	float flarevis;					// Flare visibility "counter" (1.0=100% visible, 0.0=invisible)

	float cone_inner;		// Light cone inner angle (in radians)
	float cone_outer;		// Light cone outer angle (in radians)

	// (Virtual) apply
	void Apply(int num);

	// Update globals
	void UpdateGlobals();

public:

	// Is used instead of RTTI (dynamic_cast)
	IStorm3D_Light::LTYPE GetLightType();

	// Name stuff
	void SetName(const char *name);
	const char *GetName();

	// Create
	Storm3D_Light_Spot(Storm3D *Storm3D2,const char *name,Storm3D_Model *_parent_model,COL &_color,
		float multiplier,float decay,VC3 &_position,VC3 &_direction,
		float _cone_inner=PI/2,float _cone_outer=PI,
		Storm3D_LensFlare *_lflare=NULL);

	~Storm3D_Light_Spot();

	// Change params
	void SetColor(COL &_color);
	void SetMultiplier(float _multiplier);
	void SetDecay(float _decay);
	void SetPosition(VC3 &_position);
	void SetLensFlare(IStorm3D_LensFlare *_lflare);
	void SetDirection(VC3 &_direction);
	void SetCones(float inner,float outer);

	// Get params
	COL &GetColor();
	float GetMultiplier();
	float GetDecay();
	VC3 &GetPosition();
	VC3 &GetGlobalPosition();
	IStorm3D_LensFlare *GetLensFlare();
	VC3 &GetDirection();
	VC3 &GetGlobalDirection();
	void GetCones(float &inner,float &outer);

	// Keyframe animation (all times in milliseconds)
	void Animation_Clear();	// Removes all animation keys
	void Animation_SetLoop(int end_time);	// Set end_time to 0, if you do not want looping
	void Animation_SetCurrentTime(int time);
	void Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color);
	void Animation_AddNewPositionKeyFrame(int time,const VC3 &position);
	void Animation_AddNewDirectionKeyFrame(int time,const VC3 &dir);
	void Animation_AddNewConeKeyFrame(int time,float inner,float outer);

	friend class Storm3D_Scene_LightHandler;
	friend class Storm3D_Model;
	friend class Storm3D_Scene;
};



