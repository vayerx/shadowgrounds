// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <string>
#include "storm3d_common_imp.h"
#include "IStorm3D_Material.h"



//------------------------------------------------------------------
// Storm3D_Material
//------------------------------------------------------------------
class Storm3D_Material : public IStorm3D_Material
{

public:

	// Multitexturing types
	enum MTYPE
	{
		MTYPE_COLORONLY			=0,		// 0 texcoords, 1 layers, NO TEXTURES
		MTYPE_TEXTURE			=1,		// 1 texcoords, 1 layers, base
		MTYPE_DUALTEX			=2,		// 2 texcoords, 2 layers, base+base2
		MTYPE_REF				=3,		// 0 texcoords, 1 layers, ref
		MTYPE_TEX_REF			=4,		// 1 texcoords, 2 layers, base+ref
		MTYPE_DUALTEX_REF		=5,		// 2 texcoords, 3 layers, base+base2+ref
		
		MTYPE_EMBM_REF			=6,		// 1 texcoords, 2 layers, bump+ref
		MTYPE_TEX_EMBM_REF		=7,		// 2 texcoords, 3 layers, base+bump+ref
		MTYPE_DUALTEX_EMBM_REF	=8,		// 3 texcoords, 4 layers, base+base2+bump+ref
		
		MTYPE_DOT3				=9,		// 1 texcoords, 1 layers, bump
		MTYPE_DOT3_TEX			=10,	// 2 texcoords, 2 layers, base+bump
		MTYPE_DOT3_REF			=11,	// 1 texcoords, 2 layers, bump+ref
		MTYPE_DOT3_TEX_REF		=12,	// 2 texcoords, 3 layers, base+bump+ref
		MTYPE_DOT3_DUALTEX		=13,	// 3 texcoords, 3 layers, base+base2+bump
		MTYPE_DOT3_DUALTEX_REF	=14		// 3 texcoords, 4 layers, base+base2+bump+ref
	};

	enum BUMPTYPE
	{
		BUMPTYPE_NONE	=0,
		BUMPTYPE_EMBM	=1,
		BUMPTYPE_DOT3	=2
	};

	Storm3D_Material(Storm3D *Storm3D2,const char *name);
	~Storm3D_Material();

	IStorm3D_Material *CreateNewClone();

private:

	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// Name
	char *name;
	std::string effectTextureName;

	// Textureleyers (NULL if not used)
	Storm3D_Material_TextureLayer *texture_base;
	Storm3D_Material_TextureLayer *texture_base2;
	Storm3D_Material_TextureLayer *texture_bump;
	Storm3D_Material_TextureLayer *texture_reflection;
	Storm3D_Material_TextureLayer *texture_distortion;

	// COL and other lighting properties
	COL color;
	COL self_illum;
	COL specular;
	float specular_sharpness;

	// Misc. properties
	bool doublesided;
	bool wireframe;

	// Bumpheight
	float bumpheight;

	// Alphablending properties
	ATYPE alphablend_type;
	float transparency;		// %-transparency (0.0=opaque, 0.5=50% transparent, 1.0=invisible, etc...)
	float glow;
	float glow_factor;

	// Multitextype (used on: rendering, buffer formats)
	void ReCalculateMultiTextureType();
	MTYPE multitexture_type;

	// Shader handle (v3)
	DWORD shader_handle;

	VC2 scrollSpeed1;
	VC2 scrollOffset1;
	VC2 scrollSpeed2;
	VC2 scrollOffset2;
	bool scrollEnabled;
	int scrollUpdateFrame;

	bool local_reflection;
	float reflection_blend_factor;

public:

	void updateScroll(float time_delta, int frame_id);
	const std::string &getEffectTextureName() const { return effectTextureName; }
	bool hasLocalReflection() const { return local_reflection; }
	float getReflectionBlendFactor() const { return reflection_blend_factor; }

	// Compare (used when new material is loaded -> saves memory and processing power)
	bool IsIdenticalWith(const Storm3D_Material *other) const;
	void UpdateAlphaType();

	// Apply
	bool Apply(Storm3D_Scene *scene,int pass,DWORD fvf,D3DMATRIX *mat);
	void ApplyBaseTextureOnly();
	void ApplyBase2TextureOnly();
	void ApplyBaseTextureExtOnly();
	void ApplyBaseTextureExtOnly_NoAlphaSort(Storm3D_Scene *scene,DWORD fvf,D3DMATRIX *mat);

	// Name stuff
	const char *GetName();
	void SetName(const char *name);

	// Set Parameters
	void SetColor(const COL &color=COL(1,1,1));
	void SetSelfIllumination(const COL &self_illum=COL(0,0,0));
	void SetSpecular(const COL &specular=COL(1,1,1),float specular_sharpness=25.0f);
	void SetSpecial(bool doublesided=false,bool wireframe=false);
	void SetAlphaType(ATYPE alphablend_type=ATYPE_NONE);
	void SetTransparency(float transparency=0.0f);
	void SetGlow(float value);
	void SetGlowFactor(float value);

	void SetScrollSpeed(const VC2 &speed);
	void SetScrollSpeed(const VC2 &speed1, const VC2 &speed2);
	void EnableScroll(bool enable);
	void ResetScrollPosition();

	// Get Parameters
	COL &GetColor();
	COL &GetSelfIllumination();
	void GetSpecular(COL &specular,float &specular_sharpness);
	float GetReflectionFactor();
	void GetSpecial(bool &doublesided,bool &wireframe);
	IStorm3D_Material::ATYPE GetAlphaType();
	float GetTransparency();
	float GetGlow() const;
	float GetGlowFactor() const;
	const VC2 &getScrollOffset1() const { return scrollOffset1; }
	const VC2 &getScrollOffset2() const { return scrollOffset2; }

	// Get layer's textures
	IStorm3D_Texture *GetBaseTexture();
	IStorm3D_Texture *GetBaseTexture2();
	IStorm3D_Texture *GetBumpTexture();
	IStorm3D_Texture *GetReflectionTexture();
	IStorm3D_Texture *GetDistortionTexture();

	// Texturing formula:   (bop stands for any blending operation: +,-,*,mul2x,mul4x,etc...)
	// (base bop base2) * bump bop reflection
	void SetBaseTexture(IStorm3D_Texture *texture);
	void SetDistortionTexture(IStorm3D_Texture *texture);
	void SetBaseTexture2(IStorm3D_Texture *texture,MTL_BOP blend_op=MTL_BOP_MUL,float blend_factor=0.5f);
	void SetBumpTexture(IStorm3D_Texture *texture,float bumpheight=1.0f);
	void SetReflectionTexture(IStorm3D_Texture *texture,TEX_GEN texgen=TEX_GEN_REFLECTION,MTL_BOP blend_op=MTL_BOP_ADD,float blend_factor=0.5f);
	void SetLocalReflection(bool enable, float blend_factor);

	// Special texturelayer operations
	void ChangeBaseTexture2Parameters(MTL_BOP op=MTL_BOP_MUL,float blend_factor=0.5f);
	void ChangeReflectionTextureParameters(MTL_BOP op=MTL_BOP_ADD,float blend_factor=0.5f,TEX_GEN texgen=TEX_GEN_REFLECTION);
	void ChangeBumpHeight(float bumpheight=1.0f);

	// Get parameters
	int GetTextureCoordinateSetCount();
	BUMPTYPE GetBumpType();
	MTYPE GetMultiTextureType();

	friend class Storm3D;
};



//------------------------------------------------------------------
// Storm3D_Material_TextureLayer
//------------------------------------------------------------------
class Storm3D_Material_TextureLayer
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// Texture
	Storm3D_Texture *texture;

	// Only for base2/reflection
	IStorm3D_Material::MTL_BOP blend_op;
	float blend_factor;

	// Only for reflection
	IStorm3D_Material::TEX_GEN texcoord_gen;

	// Blending operation conversion to DX8
	D3DTEXTUREOP GetDX8MultitexBlendingOp();
	D3DBLEND GetDX8MultipassSourceAlpha();
	D3DBLEND GetDX8MultipassDestinationAlpha();

	// Compare (used when new material is loaded -> saves memory and processing power)
	bool IsIdenticalWith(const Storm3D_Material_TextureLayer *other) const;

	Storm3D_Material_TextureLayer(Storm3D *Storm3D2,Storm3D_Texture *_texture,
		Storm3D_Material::MTL_BOP blend_op=Storm3D_Material::MTL_BOP_MUL,float blend_factor=0.5f,
		Storm3D_Material::TEX_GEN texgen=Storm3D_Material::TEX_GEN_REFLECTION);
	~Storm3D_Material_TextureLayer();

	Storm3D_Material_TextureLayer *CreateNewClone();

	friend class Storm3D_Material;
};



