// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_adapter.h"
#include "storm3d_material.h"
#include "storm3d_texture.h"
#include "storm3d_scene.h"

#include "../../util/Debug_MemoryManager.h"

int storm3d_material_allocs = 0;

namespace {

	// Contains EffectTexture(jokuname)
	std::string getEffectName(const std::string &name)
	{
		std::string result;

		std::string::size_type index = name.find("EffectTexture");
		if(index != name.npos)
		{
			std::string::size_type start = name.find_first_of("(", index);
			
			if(start != name.npos)
			{
				start += 1;
				std::string::size_type end = name.find_first_of(")", start);
				if(end != name.npos)
				{
					std::string result = name.substr(start, end - start);
					return result;
				}
			}
		}

		return result;
	}

	void clamp(float &value)
	{
		float intval = floorf(value);
		if(fabsf(intval) > 1.f)
			value = fmodf(value, intval);
	}

} // unnamed

//------------------------------------------------------------------
// Storm3D_Material::Storm3D_Material
//------------------------------------------------------------------
Storm3D_Material::Storm3D_Material(Storm3D *s2,const char *_name) :
	Storm3D2(s2),
	texture_base(NULL),
	texture_base2(NULL),
	texture_bump(NULL),
	texture_reflection(NULL),
	texture_distortion(NULL),
	color(1,1,1),
	self_illum(0,0,0),
	specular(1,1,1),
	specular_sharpness(25),
	doublesided(false),
	wireframe(false),
	bumpheight(1.0f),
	alphablend_type(ATYPE_NONE),
	transparency(0),
	glow(0),
	glow_factor(1.f),
	multitexture_type(MTYPE_COLORONLY),
	shader_handle(0),
	scrollEnabled(false),
	scrollUpdateFrame(-1),
	local_reflection(false),
	reflection_blend_factor(0.f)
{
	// Copy name
	name=new char[strlen(_name)+1];
	strcpy(name,_name);

	storm3d_material_allocs++;

	effectTextureName = getEffectName(name);
}

void Storm3D_Material::updateScroll(float time_delta, int frame_id)
{
	if(!scrollEnabled)
		return;
	if(scrollUpdateFrame == frame_id)
		return;

	VC2 delta1 = scrollSpeed1;
	delta1 *= time_delta;
	VC2 delta2 = scrollSpeed2;
	delta2 *= time_delta;

	scrollOffset1 += delta1;
	scrollOffset2 += delta2;
	scrollUpdateFrame = frame_id;

	clamp(scrollOffset1.x);
	clamp(scrollOffset1.y);
	clamp(scrollOffset2.x);
	clamp(scrollOffset2.y);
	/*
	while(scrollOffset1.x > 1.f)
		scrollOffset1.x -= 1.f;
	while(scrollOffset1.y > 1.f)
		scrollOffset1.y -= 1.f;
	while(scrollOffset1.x < -1.f)
		scrollOffset1.x += 1.f;
	while(scrollOffset1.y < -1.f)
		scrollOffset1.y += 1.f;
	while(scrollOffset2.x > 1.f)
		scrollOffset2.x -= 1.f;
	while(scrollOffset2.y > 1.f)
		scrollOffset2.y -= 1.f;
	while(scrollOffset2.x < -1.f)
		scrollOffset2.x += 1.f;
	while(scrollOffset2.y < -1.f)
		scrollOffset2.y += 1.f;
	*/
}


//------------------------------------------------------------------
// Storm3D_Material::~Storm3D_Material
//------------------------------------------------------------------
Storm3D_Material::~Storm3D_Material()
{
	storm3d_material_allocs--;

	// Remove from Storm3D's list
	Storm3D2->Remove(this, 0);

	if (texture_base) delete texture_base;
	if (texture_base2) delete texture_base2;
	if (texture_bump) delete texture_bump;
	if (texture_reflection) delete texture_reflection;
	if (name) delete[] name;
}



//------------------------------------------------------------------
// Storm3D_Material::CreateNewClone
//------------------------------------------------------------------
IStorm3D_Material *Storm3D_Material::CreateNewClone()
{
	Storm3D_Material *ret = (Storm3D_Material *)Storm3D2->CreateNewMaterial(name);

	// need to preserve the name buffer...
	char *namebuf = ret->name;
	// ...as this would set it to point to this material's name buffer
	*ret = *this;
	ret->name = namebuf;

	if (texture_base) 
		ret->texture_base = texture_base->CreateNewClone();
	if (texture_base2) 
		ret->texture_base2 = texture_base2->CreateNewClone();
	if (texture_bump) 
		ret->texture_bump = texture_bump->CreateNewClone();
	if (texture_reflection) 
		ret->texture_reflection = texture_reflection->CreateNewClone();

	return ret;
}






//------------------------------------------------------------------
// Storm3D_Material::SetName
//------------------------------------------------------------------
void Storm3D_Material::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}



//------------------------------------------------------------------
// Storm3D_Material::GetName
//------------------------------------------------------------------
const char *Storm3D_Material::GetName()
{
	return name;
}



//------------------------------------------------------------------
// Storm3D_Material::SetColor
//------------------------------------------------------------------
void Storm3D_Material::SetColor(const COL &_color)
{
	color=_color;
}



//------------------------------------------------------------------
// Storm3D_Material::SetSelfIllumination
//------------------------------------------------------------------
void Storm3D_Material::SetSelfIllumination(const COL &_self_illum)
{
	self_illum=_self_illum;
}



//------------------------------------------------------------------
// Storm3D_Material::SetSpecular
//------------------------------------------------------------------
void Storm3D_Material::SetSpecular(const COL &_specular,float _specular_sharpness)
{
	specular=_specular;
	specular_sharpness=_specular_sharpness;
}



//------------------------------------------------------------------
// Storm3D_Material::SetSpecial
//------------------------------------------------------------------
void Storm3D_Material::SetSpecial(bool _doublesided,bool _wireframe)
{
	doublesided=_doublesided;
	wireframe=_wireframe;
}



//------------------------------------------------------------------
// Storm3D_Material::SetAlphaType
//------------------------------------------------------------------
void Storm3D_Material::SetAlphaType(ATYPE _alphablend_type)
{
	alphablend_type=_alphablend_type;
	UpdateAlphaType();
}



//------------------------------------------------------------------
// Storm3D_Material::SetTransparency
//------------------------------------------------------------------
void Storm3D_Material::SetTransparency(float _transparency)
{
	transparency=_transparency;
}

void Storm3D_Material::SetGlow(float glow_)
{
	glow = glow_;
}

void Storm3D_Material::SetGlowFactor(float glow_factor_)
{
	glow_factor = glow_factor_;
}

void Storm3D_Material::SetScrollSpeed(const VC2 &speed)
{
	scrollSpeed1 = speed;
	scrollSpeed2 = speed;
}

void Storm3D_Material::SetScrollSpeed(const VC2 &speed1, const VC2 &speed2)
{
	scrollSpeed1 = speed1;
	scrollSpeed2 = speed2;
}

void Storm3D_Material::EnableScroll(bool enable)
{
	scrollEnabled = enable;
}

void Storm3D_Material::ResetScrollPosition()
{
	scrollOffset1 = VC2();
	scrollOffset2 = VC2();
}

//------------------------------------------------------------------
// Storm3D_Material::GetColor
//------------------------------------------------------------------
COL &Storm3D_Material::GetColor()
{
	return color;
}



//------------------------------------------------------------------
// Storm3D_Material::GetSelfIllumination
//------------------------------------------------------------------
COL &Storm3D_Material::GetSelfIllumination()
{
	return self_illum;
}



//------------------------------------------------------------------
// Storm3D_Material::GetSpecular
//------------------------------------------------------------------
void Storm3D_Material::GetSpecular(COL &_specular,float &_specular_sharpness)
{
	_specular=specular;
	_specular_sharpness=specular_sharpness;
}

float Storm3D_Material::GetReflectionFactor()
{
	if(texture_reflection)
		return texture_reflection->blend_factor;

	return 0.f;
}


//------------------------------------------------------------------
// Storm3D_Material::GetSpecial
//------------------------------------------------------------------
void Storm3D_Material::GetSpecial(bool &_doublesided,bool &_wireframe)
{
	_doublesided=doublesided;
	_wireframe=wireframe;
}



//------------------------------------------------------------------
// Storm3D_Material::GetAlphaType
//------------------------------------------------------------------
Storm3D_Material::ATYPE Storm3D_Material::GetAlphaType()
{
	return alphablend_type;
}



//------------------------------------------------------------------
// Storm3D_Material::GetTransparency
//------------------------------------------------------------------
float Storm3D_Material::GetTransparency()
{
	return transparency;
}

float Storm3D_Material::GetGlow() const
{
	return glow;
}

float Storm3D_Material::GetGlowFactor() const
{
	return glow_factor;
}

//------------------------------------------------------------------
// Storm3D_Material::ReCalculateMultiTextureType
//------------------------------------------------------------------
void Storm3D_Material::ReCalculateMultiTextureType()
{
	// Change multitexturingtype
	/*
		MTYPE_COLORONLY,		// 0 texcoords, 1 layers, NO TEXTURES
		MTYPE_TEXTURE,			// 1 texcoords, 1 layers, base
		MTYPE_DUALTEX,			// 2 texcoords, 2 layers, base+base2
		MTYPE_REF,				// 0 texcoords, 1 layers, ref
		MTYPE_TEX_REF,			// 1 texcoords, 2 layers, base+ref
		MTYPE_DUALTEX_REF,		// 2 texcoords, 3 layers, base+base2+ref
		
		MTYPE_EMBM_REF,			// 1 texcoords, 2 layers, bump+ref
		MTYPE_TEX_EMBM_REF,		// 2 texcoords, 3 layers, base+bump+ref
		MTYPE_DUALTEX_EMBM_REF,	// 3 texcoords, 4 layers, base+base2+bump+ref
		
		MTYPE_DOT3,				// 1 texcoords, 1 layers, bump
		MTYPE_DOT3_TEX,			// 2 texcoords, 2 layers, base+bump
		MTYPE_DOT3_REF,			// 1 texcoords, 2 layers, bump+ref
		MTYPE_DOT3_TEX_REF,		// 2 texcoords, 3 layers, base+bump+ref
		MTYPE_DOT3_DUALTEX,		// 3 texcoords, 3 layers, base+base2+bump
		MTYPE_DOT3_DUALTEX_REF,	// 3 texcoords, 4 layers, base+base2+bump+ref

		MTYPE_DIFF_EMBM_TEX,	// 2 texcoords, 2 layers, bump diffracts base texture
	*/

	bool supports_dot3=(Storm3D2->adapters[Storm3D2->active_adapter].caps&Storm3D_Adapter::CAPS_DOT3)!=0;
	bool supports_embm=(Storm3D2->adapters[Storm3D2->active_adapter].caps&Storm3D_Adapter::CAPS_EMBM)!=0;

	// BETA: No DOT3 support yet (remove comments to enable support;)

	if (texture_base)
	{
		if (texture_base2)
		{
			if (texture_bump)
			{
				if (texture_reflection)
				{
					// base+base2+bump+ref
					// EMBM is best choice for this one
					if (supports_embm) multitexture_type=MTYPE_DUALTEX_EMBM_REF;
					else if (supports_dot3) multitexture_type=MTYPE_DOT3_DUALTEX_REF;
					else multitexture_type=MTYPE_DUALTEX_REF;
				}
				else
				{
					// base+base2+bump
					// DOT3 is best choice for this one
					if (supports_dot3) multitexture_type=MTYPE_DOT3_DUALTEX;
					else multitexture_type=MTYPE_DUALTEX;
				}
			}
			else
			{
				if (texture_reflection)
				{
					// base+base2+ref
					multitexture_type=MTYPE_DUALTEX_REF;
				}
				else
				{
					// base+base2
					multitexture_type=MTYPE_DUALTEX;
				}
			}
		}
		else
		{
			if (texture_bump)
			{
				if (texture_reflection)
				{
					// base+bump+ref
					// EMBM is best choice for this one
					if (supports_embm) multitexture_type=MTYPE_TEX_EMBM_REF;
					//else if (supports_dot3) multitexture_type=MTYPE_DOT3_TEX_REF;
					else multitexture_type=MTYPE_TEX_REF;
				}
				else
				{
					// base+bump
					// DOT3 is best choice for this one
					if (supports_dot3) multitexture_type=MTYPE_DOT3_TEX;
					else multitexture_type=MTYPE_TEXTURE;
				}
			}
			else
			{
				if (texture_reflection)
				{
					// base+ref
					multitexture_type=MTYPE_TEX_REF;
				}
				else
				{
					// base
					multitexture_type=MTYPE_TEXTURE;
				}
			}
		}
	}
	else	// NO BASE(1) TEXTURE (Base2 is ignored if set)
	{
		if (texture_bump)
		{
			if (texture_reflection)
			{
				// bump+ref
				// EMBM is best choice for this one
				if (supports_embm) multitexture_type=MTYPE_EMBM_REF;
				else if (supports_dot3) multitexture_type=MTYPE_DOT3_REF;
				else multitexture_type=MTYPE_REF;
			}
			else
			{
				// bump
				// DOT3 is best choice for this one
				if (supports_dot3) multitexture_type=MTYPE_DOT3;
				else multitexture_type=MTYPE_COLORONLY;
			}
		}
		else
		{
			if (texture_reflection)
			{
				// ref
				multitexture_type=MTYPE_REF;
			}
			else
			{
				// color only!
				multitexture_type=MTYPE_COLORONLY;
			}
		}		
	}
}



//------------------------------------------------------------------
// Storm3D_Material::SetBaseTexture
//------------------------------------------------------------------
void Storm3D_Material::SetBaseTexture(IStorm3D_Texture *itexture)
{
	Storm3D_Texture *texture=(Storm3D_Texture*)itexture;
	
	// Add texture reference count
	if (texture) texture->AddRef();

	// Delete old texturelayer if exists
	if (texture_base) delete texture_base;
	texture_base=NULL;

	// Create new texturelayer
	if (texture) texture_base=new Storm3D_Material_TextureLayer(Storm3D2,texture);

	// Recalc.multitextype
	ReCalculateMultiTextureType();
	UpdateAlphaType();
}

void Storm3D_Material::SetDistortionTexture(IStorm3D_Texture *itexture)
{
	Storm3D_Texture *texture=(Storm3D_Texture*)itexture;
	
	// Add texture reference count
	if (texture) texture->AddRef();

	// Delete old texturelayer if exists
	if (texture_distortion) 
		delete texture_distortion;
	texture_distortion = NULL;

	// Create new texturelayer
	if (texture) 
		texture_distortion=new Storm3D_Material_TextureLayer(Storm3D2,texture);
}


//------------------------------------------------------------------
// Storm3D_Material::SetBaseTexture2
//------------------------------------------------------------------
void Storm3D_Material::SetBaseTexture2(IStorm3D_Texture *itexture,Storm3D_Material::MTL_BOP blend_op,float blend_factor)
{
	Storm3D_Texture *texture=(Storm3D_Texture*)itexture;
	
	// Add texture reference count
	if (texture) texture->AddRef();

	// Delete old texturelayer if exists
	if (texture_base2) delete texture_base2;
	texture_base2=NULL;

	// Create new texturelayer
	if (texture) texture_base2=new Storm3D_Material_TextureLayer(Storm3D2,texture,blend_op,blend_factor);

	// Recalc.multitextype
	ReCalculateMultiTextureType();
}



//------------------------------------------------------------------
// Storm3D_Material::SetBumpTexture
//------------------------------------------------------------------
void Storm3D_Material::SetBumpTexture(IStorm3D_Texture *itexture,float _bumpheight)
{
	Storm3D_Texture *texture=(Storm3D_Texture*)itexture;
	
	// Add texture reference count
	if (texture) texture->AddRef();

	// Delete old texturelayer if exists
	if (texture_bump) delete texture_bump;
	texture_bump=NULL;

	// Create new texturelayer
	if (texture) texture_bump=new Storm3D_Material_TextureLayer(Storm3D2,texture,Storm3D_Material::MTL_BOP_MUL);

	// Set material bumpheight
	bumpheight=_bumpheight;

	// Recalc.multitextype
	ReCalculateMultiTextureType();
}



//------------------------------------------------------------------
// Storm3D_Material::SetReflectionTexture
//------------------------------------------------------------------
void Storm3D_Material::SetReflectionTexture(IStorm3D_Texture *itexture,TEX_GEN texgen,Storm3D_Material::MTL_BOP blend_op,float blend_factor)
{
	Storm3D_Texture *texture=(Storm3D_Texture*)itexture;
	
	// Add texture reference count
	if (texture) texture->AddRef();

	// Delete old texturelayer if exists
	if (texture_reflection) delete texture_reflection;
	texture_reflection=NULL;

	// Create new texturelayer
	if (texture) texture_reflection=new Storm3D_Material_TextureLayer(Storm3D2,texture,blend_op,blend_factor,texgen);

	// Recalc.multitextype
	ReCalculateMultiTextureType();
}



//------------------------------------------------------------------
// Storm3D_Material::GetTextureCoordinateSetCount
//------------------------------------------------------------------
int Storm3D_Material::GetTextureCoordinateSetCount()
{
	/*switch (multitexture_type)
	{
		MTYPE_COLORONLY,		// 0 texcoords, 1 layers, NO TEXTURES
		MTYPE_TEXTURE,			// 1 texcoords, 1 layers, base
		MTYPE_DUALTEX,			// 2 texcoords, 2 layers, base+base2
		MTYPE_REF,				// 0 texcoords, 1 layers, ref
		MTYPE_TEX_REF,			// 1 texcoords, 2 layers, base+ref
		MTYPE_DUALTEX_REF,		// 2 texcoords, 3 layers, base+base2+ref
		
		MTYPE_EMBM_REF,			// 1 texcoords, 2 layers, bump+ref
		MTYPE_TEX_EMBM_REF,		// 2 texcoords, 3 layers, base+bump+ref
		MTYPE_DUALTEX_EMBM_REF,	// 3 texcoords, 4 layers, base+base2+bump+ref
		
		MTYPE_DOT3,				// 1 texcoords, 1 layers, bump
		MTYPE_DOT3_TEX,			// 2 texcoords, 2 layers, base+bump
		MTYPE_DOT3_REF,			// 1 texcoords, 2 layers, bump+ref
		MTYPE_DOT3_TEX_REF,		// 2 texcoords, 3 layers, base+bump+ref
		MTYPE_DOT3_DUALTEX,		// 3 texcoords, 3 layers, base+base2+bump
		MTYPE_DOT3_DUALTEX_REF,	// 3 texcoords, 4 layers, base+base2+bump+ref

		MTYPE_DIFF_EMBM_TEX,	// 2 texcoords, 2 layers, bump diffracts base texture

		case MTYPE_COLORONLY: return 0;
		case MTYPE_TEXTURE: return 1;
		case MTYPE_DUALTEX: return 2;
		case MTYPE_REF: return 0;
		case MTYPE_TEX_REF: return 1;
		case MTYPE_DUALTEX_REF: return 2;

		case MTYPE_EMBM_REF: return 1;
		case MTYPE_TEX_EMBM_REF: return 2;
		case MTYPE_DUALTEX_EMBM_REF: return 3;

		case MTYPE_DOT3: return 1;
		case MTYPE_DOT3_TEX: return 2;
		case MTYPE_DOT3_SREF: return 1;
		case MTYPE_DOT3_CREF: return 1;
	}
	return 0;*/

	if ((multitexture_type==MTYPE_COLORONLY)||(multitexture_type==MTYPE_REF))
		return 0; else return 1;
}



//------------------------------------------------------------------
// Storm3D_Material::GetBumpType
//------------------------------------------------------------------
Storm3D_Material::BUMPTYPE Storm3D_Material::GetBumpType()
{
	switch (multitexture_type)
	{
		case MTYPE_EMBM_REF: return BUMPTYPE_EMBM;
		case MTYPE_TEX_EMBM_REF: return BUMPTYPE_EMBM;
		case MTYPE_DUALTEX_EMBM_REF: return BUMPTYPE_EMBM;

		case MTYPE_DOT3: return BUMPTYPE_DOT3;
		case MTYPE_DOT3_TEX: return BUMPTYPE_DOT3;
		case MTYPE_DOT3_REF: return BUMPTYPE_DOT3;
		case MTYPE_DOT3_TEX_REF: return BUMPTYPE_DOT3;
		case MTYPE_DOT3_DUALTEX: return BUMPTYPE_DOT3;
		case MTYPE_DOT3_DUALTEX_REF: return BUMPTYPE_DOT3;
		default: break;
	}
	return BUMPTYPE_NONE;
}

// psd .. this was declared but not defined
bool Storm3D_Material::IsIdenticalWith(const Storm3D_Material *other) const
{
	if(this == other)
		return true;

	// this is enough for terrain, I guess
	bool identical = true;

	// ToDo: operator == for different stuff
	
	// Texture layers
	if(texture_base && other->texture_base)
	{
		if(texture_base->texture != other->texture_base->texture)
			identical = false;
	}
	else if(texture_base || other->texture_base)
		identical = false;
	if(texture_base2 && other->texture_base2)
	{
		if(texture_base2->texture != other->texture_base2->texture)
			identical = false;
	}
	else if(texture_base2 || other->texture_base2)
		identical = false;

	// Diffuse
	if((fabsf(color.r - other->color.r) > 0.05f) ||
	 (fabsf(color.g - other->color.g) > 0.05f) ||
	 (fabsf(color.b - other->color.b) > 0.05f))
		identical = false;

	return identical;
}

void Storm3D_Material::UpdateAlphaType()
{
	if(alphablend_type == ATYPE_USE_TEXTRANSPARENCY || alphablend_type == ATYPE_USE_ALPHATEST)
	{
		if(texture_base && texture_base->texture)
		{
			if(!effectTextureName.empty() || texture_base->texture->hasDecentAlpha())
				alphablend_type = ATYPE_USE_TEXTRANSPARENCY;
			else
				alphablend_type = ATYPE_USE_ALPHATEST;
		}
	}
}


//------------------------------------------------------------------
// Storm3D_Material::Apply
// Applies material. Use pass parameter to tell with pass to
// render. Start with pass=0, and increase it until this routine
// returns false. (=all passes rendered)
// 
// Returns:
// false = this was last pass
// true = new pass must be rendered
//------------------------------------------------------------------
bool Storm3D_Material::Apply(Storm3D_Scene *scene,int pass,DWORD fvf,D3DMATRIX *mtx)
{
	/*
	Basic config:

	All textures = NULL

	Stage0:
	colorop = modulate
	colorarg1 = texture
	colorarg2 = diffuse
	alphaop = disable
	texturetransformflags = disable
	texcoordindex = 0

	Stage1-3:
	colorarg1 = texture
	colorarg2 = current
	alphaop = disable
	texturetransformflags = disable
	texcoordindex = 0
	
	After rendering material's UnApply() is called, and it
	returns these states, if it changes them.

	*/

	// BETA: UnApply() all (very slow!)
	Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	Storm3D2->D3DDevice->SetTexture(0,NULL);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);

	//	FixMe:
	//	Setting this causes debug runtime to halt (set only to supported stages)
		//for (int i=1;i<3;i++)
	for (int i=0;i<2;i++)
	{
		Storm3D2->D3DDevice->SetTexture(i,NULL);
		Storm3D2->D3DDevice->SetTextureStageState(i,D3DTSS_COLORARG1,D3DTA_TEXTURE);
		Storm3D2->D3DDevice->SetTextureStageState(i,D3DTSS_COLORARG2,D3DTA_CURRENT);
		Storm3D2->D3DDevice->SetTextureStageState(i,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
		Storm3D2->D3DDevice->SetTextureStageState(i,D3DTSS_TEXCOORDINDEX,i);
		Storm3D2->D3DDevice->SetTextureStageState(i,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	}

	// Animate textures
	if (texture_base) texture_base->texture->AnimateVideo();
	if (texture_base2) texture_base2->texture->AnimateVideo();
	if (texture_reflection) texture_reflection->texture->AnimateVideo();
	if (texture_bump) texture_bump->texture->AnimateVideo();

	// The superb if(TM) multitexturing (no effects/techniques needed anymore!)	
	// This routine is much faster than DX8-effect system (and bugfree also;)...
	if (multitexture_type==MTYPE_COLORONLY)			// COL only
	{
		// Set stages (color only)
		//Storm3D2->D3DDevice->SetTexture(0,NULL);

		// psd: assume lightmap if base2 defined
		if(texture_base2)
		{
			texture_base2->texture->Apply(0);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,texture_base2->GetDX8MultitexBlendingOp());
		}
		else
		{
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
		}
	}
	else if (multitexture_type==MTYPE_TEXTURE)		// Base
	{
		// Set stage (base)
		texture_base->texture->Apply(0);

		// Disable last stage
		Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
	}
	else if (multitexture_type==MTYPE_DUALTEX)		// Base+Base2
	{
		// Set stage (base)
		texture_base->texture->Apply(0);
		
		// Set stage (base2)
		texture_base2->texture->Apply(1);
		Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,texture_base2->GetDX8MultitexBlendingOp());

		// Disable last stage
		Storm3D2->D3DDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);
	}
	else if (multitexture_type==MTYPE_REF)			// Reflection
	{		
		// Set stage (reflection)
		texture_reflection->texture->Apply(0);
	
		// Reflection texture (or projective?)
		if (texture_reflection->texcoord_gen==TEX_GEN_REFLECTION)
		{
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

			// Correct number of texturecoordinates
			if (texture_reflection->texture->IsCube())
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3);
			else
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
		

			// Rotate the reflection
			VC3 camdir=scene->camera.GetDirection();
			float angle_y=VC2(camdir.x,camdir.z).CalculateAngle();
			float angle_x=VC2(VC2(camdir.x,camdir.z).GetLength(),camdir.y).CalculateAngle();
			MAT mat;
			mat.CreateRotationMatrix(QUAT(-angle_x,-angle_y,0));

			if (!texture_reflection->texture->IsCube())
			{	
				// Fix reflection (v2.3)
				float mt[16]=
				{
					0.5,	0,		0,	0,
					0,		-0.5,	0,	0,
					0,		0,		1,	0,
					0.5,	0.5,	0,	1
				};
				MAT mat2(mt);
				mat.Multiply(mat2);
			
				// Flip texture upside down
				//mat.Multiply(MAT(VC3(1,-1,1)));	
			}

			// Set texture rotation matrix
			D3DMATRIX dxmat;
			mat.GetAsD3DCompatible4x4((float*)&dxmat);
			Storm3D2->D3DDevice->SetTransform(D3DTS_TEXTURE0,&dxmat);
		}
		else	// Projective texture
		{
			// Create matrix, tu=(0.5+0.87*x)/z, tv=(0.5-0.87*y)/z 
		    D3DXMATRIX mat;
			mat._11=0.866f;mat._12=0.0f;mat._13=0.0f;
			mat._21=0.0f;mat._22=-0.866f;mat._23=0.0f;
			mat._31=0.5f;mat._32=0.5f;mat._33=1.0f;
			mat._41=0.0f;mat._42=0.0f;mat._43=0.0f;
			
			// If it's mirror negate _11
			if (texture_reflection->texcoord_gen==TEX_GEN_PROJECTED_MIRROR)
				mat._11*=-1;

		    Storm3D2->D3DDevice->SetTransform(D3DTS_TEXTURE0,&mat);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3|D3DTTFF_PROJECTED);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
		}

		// Disable last stage
		Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
	}
	else if (multitexture_type==MTYPE_TEX_REF)		// Base+Reflection
	{
		// Set stage (base)
		texture_base->texture->Apply(0);

		// Set stage (reflection)
		texture_reflection->texture->Apply(1);
		Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,texture_reflection->GetDX8MultitexBlendingOp());
		
		// Reflection texture (or projective?)
		if (texture_reflection->texcoord_gen==TEX_GEN_REFLECTION)
		{
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

			// Correct number of texturecoordinates
			if (texture_reflection->texture->IsCube())
				Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3);
			else
				Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);

			// Rotate the reflection
			VC3 camdir=scene->camera.GetDirection();
			float angle_y=VC2(camdir.x,camdir.z).CalculateAngle();
			float angle_x=VC2(VC2(camdir.x,camdir.z).GetLength(),camdir.y).CalculateAngle();
			MAT mat;
			mat.CreateRotationMatrix(QUAT(-angle_x,-angle_y,0));

			if (!texture_reflection->texture->IsCube())
			{	
				// Fix reflection (v2.3)
				float mt[16]=
				{
					0.5,	0,		0,	0,
					0,		-0.5,	0,	0,
					0,		0,		1,	0,
					0.5,	0.5,	0,	1
				};
				MAT mat2(mt);
				mat.Multiply(mat2);
			}

			// Set texture rotation matrix
			D3DMATRIX dxmat;
			mat.GetAsD3DCompatible4x4((float*)&dxmat);
			Storm3D2->D3DDevice->SetTransform(D3DTS_TEXTURE1,&dxmat);
		}
		else	// Projective mirror texture
		{
			// Create matrix, tu=(0.5+0.87*x)/z, tv=(0.5-0.87*y)/z 
			D3DXMATRIX mat;
			mat._11=0.866f;mat._12=0.0f;mat._13=0.0f;
			mat._21=0.0f;mat._22=-0.866f;mat._23=0.0f;
			mat._31=0.5f;mat._32=0.5f;mat._33=1.0f;
			mat._41=0.0f;mat._42=0.0f;mat._43=0.0f;

			// If it's mirror negate _11
			if (texture_reflection->texcoord_gen==TEX_GEN_PROJECTED_MIRROR)
				mat._11*=-1;

			Storm3D2->D3DDevice->SetTransform(D3DTS_TEXTURE1,&mat);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3|D3DTTFF_PROJECTED);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
		}

		// Disable last stage
		Storm3D2->D3DDevice->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE);
	}

	// Setup material
	D3DMATERIAL9 mat;

	// Set diffuse
	mat.Diffuse.r=mat.Ambient.r=color.r;
	mat.Diffuse.g=mat.Ambient.g=color.g;
	mat.Diffuse.b=mat.Ambient.b=color.b;
	mat.Diffuse.a=mat.Ambient.a=0;

	// Set self.illum
	mat.Emissive.r=self_illum.r;
	mat.Emissive.g=self_illum.g;
	mat.Emissive.b=self_illum.b;
	mat.Emissive.a=0;

	// Set specular
	mat.Specular.r=specular.r;
	mat.Specular.g=specular.g;
	mat.Specular.b=specular.b;
	mat.Specular.a=0;
	mat.Power=specular_sharpness;

	// Set specular on only if it's used
	if ((specular_sharpness>1)&&
		((specular.r>0.01f)||(specular.g>0.01f)||(specular.b>0.01f)))
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,TRUE);
	} else Storm3D2->D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,FALSE);

	// Set 2sided
//	if (doublesided) Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
//		else Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

	// Set wireframe
	if (wireframe) Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
		else Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	
	// BETA!!!
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);

	// Set alphablending
	if (alphablend_type==ATYPE_NONE)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	}
	else if (alphablend_type==ATYPE_USE_TRANSPARENCY)
	{
		if (transparency>0.001f)
		{
			mat.Diffuse.a=1.0f-transparency;
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);				
		}
		else
		{
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		}
	}
	else if (alphablend_type==ATYPE_USE_TEXTRANSPARENCY || alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
	{
		mat.Diffuse.a=1.0f-transparency;

		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		if(alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		else
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

		if ((multitexture_type==MTYPE_DOT3_TEX)||(multitexture_type==MTYPE_DOT3_REF))
		{
			// Stage 1 alphamap (no diffuse)
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_ALPHAARG2,D3DTA_CURRENT);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
		}
		else	
		{
			// Stage 0 alphamap * diffuse
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
		}
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHAREF,(DWORD)0x00000001);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
	}
	else if (alphablend_type==ATYPE_ADD)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}
	else if (alphablend_type==ATYPE_MUL)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ZERO);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}
	else if (alphablend_type==ATYPE_MUL2X)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}

	// Use this material
	Storm3D2->D3DDevice->SetMaterial(&mat);
/* PSD
	// Apply shader
	if (!ApplyShaderIfAvailable(scene,mtx)) Storm3D2->D3DDevice->SetVertexShader(fvf);
*/
	// Set active material
	Storm3D2->active_material=this;

	return false;
}



//------------------------------------------------------------------
// Storm3D_Material::ApplyBaseTextureOnly
// Applies material base texture only. Used on particle systems.
//------------------------------------------------------------------
void Storm3D_Material::ApplyBaseTextureOnly()
{
	if (texture_base)
	{
		texture_base->texture->AnimateVideo();
		texture_base->texture->Apply(0);
	}
	else
	{
		Storm3D2->D3DDevice->SetTexture(0,NULL);
	}

	// Set active material
	// Storm3D2->active_material=this;
}

void Storm3D_Material::ApplyBase2TextureOnly()
{
	if(texture_base2)
		texture_base2->texture->Apply(1);
	else
		Storm3D2->D3DDevice->SetTexture(1, 0);
}


//------------------------------------------------------------------
// Storm3D_Material::ApplyBaseTextureExtOnly
// Applies material base texture and alpha. Used on sprites.
//------------------------------------------------------------------
void Storm3D_Material::ApplyBaseTextureExtOnly()
{
	if (texture_base)
	{
		texture_base->texture->AnimateVideo();
		texture_base->texture->Apply(0);
	}
	else
	{
		Storm3D2->D3DDevice->SetTexture(0,NULL);
	}

	// Setup material
	D3DMATERIAL9 mat;

	// Set diffuse
	mat.Diffuse.r=mat.Ambient.r=color.r;
	mat.Diffuse.g=mat.Ambient.g=color.g;
	mat.Diffuse.b=mat.Ambient.b=color.b;
	mat.Diffuse.a=mat.Ambient.a=0;

	// Set self.illum
	mat.Emissive.r=self_illum.r;
	mat.Emissive.g=self_illum.g;
	mat.Emissive.b=self_illum.b;
	mat.Emissive.a=0;

	// Set specular
	mat.Specular.r=specular.r;
	mat.Specular.g=specular.g;
	mat.Specular.b=specular.b;
	mat.Specular.a=0;
	mat.Power=specular_sharpness;

	// NO specular, wire or 2sided
	
	// Set alphablending
	if (alphablend_type==ATYPE_NONE)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	}
	else if (alphablend_type==ATYPE_USE_TRANSPARENCY)
	{
		if (transparency>0.001f)
		{
			mat.Diffuse.a=1.0f-transparency;
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);				
		}
		else
		{
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		}
	}
	else if (alphablend_type==ATYPE_USE_TEXTRANSPARENCY || alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
	{
		mat.Diffuse.a=1.0f-transparency;

		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		// wtf ?!
		// -- psd
		//Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHAREF,(DWORD)0x00000001);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
	}
	else if (alphablend_type==ATYPE_ADD)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}
	else if (alphablend_type==ATYPE_MUL)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ZERO);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
		//Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
		//Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);

		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}
	else if (alphablend_type==ATYPE_MUL2X)
	{
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}

	// Use this material
	Storm3D2->D3DDevice->SetMaterial(&mat);

	// Set active material
	Storm3D2->active_material=this;
}



//------------------------------------------------------------------
// Storm3D_Material::ApplyBaseTextureExtOnly_NoAlphaSort
// Applies material base texture, alpha and shader. Used on terrain
// (without alpha sorting).
//------------------------------------------------------------------
void Storm3D_Material::ApplyBaseTextureExtOnly_NoAlphaSort(Storm3D_Scene *scene,DWORD fvf,D3DMATRIX *mtx)
{
	if (texture_base)
	{
		texture_base->texture->AnimateVideo();
		texture_base->texture->Apply(0);
	}
	else
	{
		Storm3D2->D3DDevice->SetTexture(0,NULL);
	}

	// Setup material
	D3DMATERIAL9 mat;

	// Set diffuse
	mat.Diffuse.r=mat.Ambient.r=color.r;
	mat.Diffuse.g=mat.Ambient.g=color.g;
	mat.Diffuse.b=mat.Ambient.b=color.b;
	mat.Diffuse.a=mat.Ambient.a=0;

	// Set self.illum
	mat.Emissive.r=self_illum.r;
	mat.Emissive.g=self_illum.g;
	mat.Emissive.b=self_illum.b;
	mat.Emissive.a=0;

	// Set specular
	mat.Specular.r=specular.r;
	mat.Specular.g=specular.g;
	mat.Specular.b=specular.b;
	mat.Specular.a=0;
	mat.Power=specular_sharpness;

	// NO specular, wire

	// Set 2sided
//	if (doublesided) Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	
	// Set alphablending
	/*if (alphablend_type==ATYPE_NONE)
	{
		// OK
	}
	else if (alphablend_type==ATYPE_USE_TRANSPARENCY)
	{
		// Not supported (every pixel must me 100% visible or 0% visible)
	}
	else*/ 
	if (alphablend_type==ATYPE_USE_TEXTRANSPARENCY || alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
	{
		mat.Diffuse.a=1.0f-transparency;

		// Only alphatest
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
	}
	/*else if (alphablend_type==ATYPE_ADD)
	{
		// Not supported (every pixel must me 100% visible or 0% visible)
	}
	else if (alphablend_type==ATYPE_MUL)
	{
		// Not supported (every pixel must me 100% visible or 0% visible)
	}
	else if (alphablend_type==ATYPE_MUL2X)
	{
		// Not supported (every pixel must me 100% visible or 0% visible)
	}*/

	// Use this material
	Storm3D2->D3DDevice->SetMaterial(&mat);
/*
	// Apply shader
	if (!ApplyShaderIfAvailable(scene,mtx)) Storm3D2->D3DDevice->SetVertexShader(fvf);
*/
	// Set active material
	Storm3D2->active_material=this;
}



//------------------------------------------------------------------
// Storm3D_Material::ChangeBaseTexture2Parameters
//------------------------------------------------------------------
void Storm3D_Material::ChangeBaseTexture2Parameters(MTL_BOP op,float blend_factor)
{
	if(texture_base2)
	{
		texture_base2->blend_op=op;
		texture_base2->blend_factor=blend_factor;
	}
}



//------------------------------------------------------------------
// Storm3D_Material::ChangeReflectionTextureParameters
//------------------------------------------------------------------
void Storm3D_Material::ChangeReflectionTextureParameters(MTL_BOP op,float blend_factor,TEX_GEN texgen)
{
	if(texture_reflection)
	{
		texture_reflection->blend_op=op;
		texture_reflection->blend_factor=blend_factor;
		texture_reflection->texcoord_gen=texgen;
	}
}



//------------------------------------------------------------------
// Storm3D_Material::GetBaseTexture
//------------------------------------------------------------------
IStorm3D_Texture *Storm3D_Material::GetBaseTexture()
{
	if ((texture_base)&&(texture_base->texture)) return texture_base->texture;
	return NULL;
}

IStorm3D_Texture *Storm3D_Material::GetDistortionTexture()
{
	if ((texture_distortion)&&(texture_distortion->texture)) 
		return texture_distortion->texture;

	return NULL;
}


//------------------------------------------------------------------
// Storm3D_Material::GetBaseTexture2
//------------------------------------------------------------------
IStorm3D_Texture *Storm3D_Material::GetBaseTexture2()
{
	if ((texture_base2)&&(texture_base2->texture)) return texture_base2->texture;
	return NULL;
}



//------------------------------------------------------------------
// Storm3D_Material::GetBumpTexture
//------------------------------------------------------------------
IStorm3D_Texture *Storm3D_Material::GetBumpTexture()
{
	if ((texture_bump)&&(texture_bump->texture)) return texture_bump->texture;
	return NULL;
}



//------------------------------------------------------------------
// Storm3D_Material::GetReflectionTexture
//------------------------------------------------------------------
IStorm3D_Texture *Storm3D_Material::GetReflectionTexture()
{
	if ((texture_reflection)&&(texture_reflection->texture)) return texture_reflection->texture;
	return NULL;
}


//------------------------------------------------------------------
// Storm3D_Material::GetMultiTextureType
//------------------------------------------------------------------
Storm3D_Material::MTYPE Storm3D_Material::GetMultiTextureType()
{
	return multitexture_type;
}



//------------------------------------------------------------------
// Storm3D_Material::ChangeBumpHeight
//------------------------------------------------------------------
void Storm3D_Material::ChangeBumpHeight(float _bumpheight)
{
	bumpheight=_bumpheight;
}

void Storm3D_Material::SetLocalReflection(bool enable, float blend_factor)
{
	local_reflection = enable;
	reflection_blend_factor = blend_factor;
}
