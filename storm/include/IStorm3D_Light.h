/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  Interfaces:

	IStorm3D_Light (base class)
	IStorm3D_Light_Direction
	IStorm3D_Light_Point
	IStorm3D_Light_Spot
	IStorm3D_LensFlare

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatypes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Texture.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Light;
class IStorm3D_Light_Directional;
class IStorm3D_Light_Point;
class IStorm3D_Light_Spot;
class IStorm3D_LensFlare;



//------------------------------------------------------------------
// IStorm3D_Light - base class (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Light
{

public:

	enum LTYPE 
	{
		LTYPE_POINT			=0,
		LTYPE_DIRECTIONAL	=1,
		LTYPE_SPOT			=2
	};

	// Can be used instead of RTTI (dynamic_cast)
	virtual LTYPE GetLightType()=0;

	// Name stuff (used on model's SearchLight)
	virtual void SetName(const char *name)=0;
	virtual const char *GetName()=0;

	// Change parameters
	virtual void SetColor(const COL &color)=0;
	virtual void SetMultiplier(float multiplier)=0;
	virtual void SetDecay(float decay)=0;

	// Get parameters
	virtual COL &GetColor()=0;
	virtual float GetMultiplier()=0;
	virtual float GetDecay()=0;
	
	// Keyframe animation (all times in milliseconds)
	virtual void Animation_Clear()=0;	// Removes all animation keys
	virtual void Animation_SetLoop(int end_time)=0;	// Set end_time to 0, if you do not want looping
	virtual void Animation_SetCurrentTime(int time)=0;
	virtual void Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color)=0;

	// Virtual destructor (DO NOT USE THIS... Use Model's delete instead)
	virtual ~IStorm3D_Light() {};
};



//------------------------------------------------------------------
// IStorm3D_Light_Directional (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Light_Directional : public IStorm3D_Light
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
// IStorm3D_Light_Point (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Light_Point : public IStorm3D_Light
{

public:

	// Change params
	virtual void SetPosition(const VC3 &position)=0;
	virtual void SetLensFlare(IStorm3D_LensFlare *lflare)=0;

	// Get params
	virtual VC3 &GetPosition()=0;
	virtual VC3 &GetGlobalPosition()=0;
	virtual IStorm3D_LensFlare *GetLensFlare()=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewPositionKeyFrame(int time,const VC3 &position)=0;
};



//------------------------------------------------------------------
// IStorm3D_Light_Spot (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Light_Spot : public IStorm3D_Light_Point
{

public:

	// Change params
	virtual void SetDirection(const VC3 &direction)=0;
	virtual void SetCones(float inner,float outer)=0;

	// Get params
	virtual VC3 &GetDirection()=0;
	virtual VC3 &GetGlobalDirection()=0;
	virtual void GetCones(float &inner,float &outer)=0;

	// Keyframe animation (all times in milliseconds)
	virtual void Animation_AddNewDirectionKeyFrame(int time,const VC3 &dir)=0;
	virtual void Animation_AddNewConeKeyFrame(int time,float inner,float outer)=0;
};



//------------------------------------------------------------------
// IStorm3D_LensFlare
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_LensFlare
{

public:

	// Set textures (set NULL if you don't want glow/rings)
	virtual void SetGlowTexture(IStorm3D_Texture *texture)=0;
	virtual void SetRingTexture(IStorm3D_Texture *texture)=0;

	// Set other properties
	virtual void SetColor(const COL &color)=0;
	virtual void SetGlowSize(float size)=0;
	virtual void SetRingSize(float size)=0;

	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_LensFlare() {};
};


