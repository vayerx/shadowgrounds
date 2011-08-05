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
#include "IStorm3D_Texture.h"
#include "IStorm3D_Material.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Texture;
class IStorm3D_Material;



//------------------------------------------------------------------
// IStorm3D_Material (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Material
{

public:

	// Material layer blend operations
	// It's recommended that you use mainly ADD, MUL or MUL2X, because old 3d-cards
	// support only them. Also if a material has more texturelayers than a 3d-card
	// can draw, multipass blending must be used, and multipass blending does
	// not support all of these blending operations.
	enum MTL_BOP
	{
		MTL_BOP_MUL				=0,		
		MTL_BOP_MUL2X			=1,		// if no support: ->MUL
		MTL_BOP_MUL4X			=2,		// if no support: ->MUL2X
		MTL_BOP_ADD				=3,
		MTL_BOP_SUB				=4,
		MTL_BOP_ADDSUB			=5,		// if no support: ->ADD
		MTL_BOP_ADDSUB2X		=6,		// if no support: ->ADDSUB
		MTL_BOP_BLENDFACTOR		=7		// if no support: ->MUL
	};

	// Alphablending types
	enum ATYPE
	{
		ATYPE_NONE					=0,
		ATYPE_USE_TRANSPARENCY		=1,		// %-transparency
		ATYPE_USE_TEXTRANSPARENCY	=2,		// transparency from texture's alphachannel multiplied by %-transparency
		ATYPE_ADD					=3,		// additive
		ATYPE_MUL					=4,		// multiple
		ATYPE_MUL2X					=5,		// multiple*2
		ATYPE_USE_ALPHATEST			=6		// transparency from texture's alphachannel multiplied by %-transparency
	};

	// Reflection texturecoordinate generation (texgen) types
	enum TEX_GEN
	{
		TEX_GEN_REFLECTION			=0,		// Generate texturecoordinates as reflection (spherical or cubic depending of texture)
		TEX_GEN_PROJECTED			=1,		// Project texture (to screen)
		TEX_GEN_PROJECTED_MIRROR	=2		// Project texture (to screen) and mirror it
	};

	// Name stuff
	virtual const char *GetName()=0;
	virtual void SetName(const char *name)=0;

	// Set Parameters
	virtual void SetColor(const COL &color=COL(1,1,1))=0;
	virtual void SetSelfIllumination(const COL &self_illum=COL(0,0,0))=0;
	virtual void SetSpecular(const COL &specular=COL(1,1,1),float specular_sharpness=25.0f)=0;
	virtual void SetSpecial(bool doublesided=false,bool wireframe=false)=0;
	virtual void SetAlphaType(ATYPE alphablend_type=ATYPE_NONE)=0;
	virtual void SetTransparency(float transparency=0.0f)=0;
	virtual void SetGlow(float value) = 0;
	virtual void SetGlowFactor(float value) = 0;
	virtual void SetScrollSpeed(const VC2 &speed) = 0;
	virtual void SetScrollSpeed(const VC2 &speed1, const VC2 &speed2) = 0;
	virtual void EnableScroll(bool enable) = 0;
	virtual void ResetScrollPosition() = 0;

	// Get Parameters
	virtual COL &GetColor()=0;
	virtual COL &GetSelfIllumination()=0;
	virtual void GetSpecular(COL &specular,float &specular_sharpness)=0;
	virtual void GetSpecial(bool &doublesided,bool &wireframe)=0;
	virtual ATYPE GetAlphaType()=0;
	virtual float GetTransparency()=0;
	virtual float GetGlow() const = 0;
	virtual float GetGlowFactor() const = 0;

	// Get layer's textures
	virtual IStorm3D_Texture *GetBaseTexture()=0;
	virtual IStorm3D_Texture *GetBaseTexture2()=0;
	virtual IStorm3D_Texture *GetBumpTexture()=0;
	virtual IStorm3D_Texture *GetReflectionTexture()=0;
	virtual IStorm3D_Texture *GetDistortionTexture()=0;

	// Texturelayers
	// Texturing formula:   (bop stands for any blending operation: +,-,*,mul2x,mul4x,etc...)
	// (base bop base2) * bump bop reflection
	virtual void SetBaseTexture(IStorm3D_Texture *texture)=0;
	virtual void SetBaseTexture2(IStorm3D_Texture *texture,MTL_BOP blend_op=MTL_BOP_MUL,float blend_factor=0.5f)=0;
	virtual void SetBumpTexture(IStorm3D_Texture *texture,float bumpheight=1.0f)=0;
	virtual void SetReflectionTexture(IStorm3D_Texture *texture,TEX_GEN texgen=TEX_GEN_REFLECTION,MTL_BOP blend_op=MTL_BOP_ADD,float blend_factor=0.5f)=0;
	virtual void SetLocalReflection(bool enable, float blend_factor)=0;
	virtual void SetDistortionTexture(IStorm3D_Texture *texture)=0;
	
	// Special texturelayer operations
	virtual void ChangeBaseTexture2Parameters(MTL_BOP op=MTL_BOP_MUL,float blend_factor=0.5f)=0;
	virtual void ChangeReflectionTextureParameters(MTL_BOP op=MTL_BOP_ADD,float blend_factor=0.5f,TEX_GEN texgen=TEX_GEN_REFLECTION)=0;
	virtual void ChangeBumpHeight(float bumpheight=1.0f)=0;
	
	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_Material() {};
};


