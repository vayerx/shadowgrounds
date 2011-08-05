// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#ifdef _MSC_VER
#include <windows.h>
#endif

#include "storm3d.h"
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

} // unnamed

//! Constructor
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

//! Update scroll
/*
	\param time_delta
	\param frame_id
*/
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
}

//! Destructor
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

//! Get the name of material
/*
	\return name
*/
const char *Storm3D_Material::GetName()
{
	return name;
}

//! Creates new clone of material
/*
	\return clone
*/
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

//! Set name of material
/*
	\param _name name of material
*/
void Storm3D_Material::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

//! Set color of material
/*
	\param _color color
*/
void Storm3D_Material::SetColor(const COL &_color)
{
	color=_color;
}

//! Set self illumination color of material
/*
	\param _self_illum color
*/
void Storm3D_Material::SetSelfIllumination(const COL &_self_illum)
{
	self_illum=_self_illum;
}

//! Set specular color of material
/*
	\param _specular color
	\param _specular_sharpness specular sharpness
*/
void Storm3D_Material::SetSpecular(const COL &_specular,float _specular_sharpness)
{
	specular=_specular;
	specular_sharpness=_specular_sharpness;
}

//! Set special properties of material
/*
	\param _doublesided is the material double sided
	\param _wireframe is material a wireframe
*/
void Storm3D_Material::SetSpecial(bool _doublesided,bool _wireframe)
{
	doublesided=_doublesided;
	wireframe=_wireframe;
}

//! Set alpha blending type of material
/*
	\param _alphablend_type alpha blending type
*/
void Storm3D_Material::SetAlphaType(ATYPE _alphablend_type)
{
	alphablend_type=_alphablend_type;
	UpdateAlphaType();
}

//! Set transparency of material
/*
	\param _transparency transparency
*/
void Storm3D_Material::SetTransparency(float _transparency)
{
	transparency=_transparency;
}

//! Set glow of material
/*
	\param glow_ glow
*/
void Storm3D_Material::SetGlow(float glow_)
{
	glow = glow_;
}

//! Set glow factor of material
/*
	\param glow_factor_ glow factor
*/
void Storm3D_Material::SetGlowFactor(float glow_factor_)
{
	glow_factor = glow_factor_;
}

//! Set scroll speed of material
/*
	\param speed scroll speed
*/
void Storm3D_Material::SetScrollSpeed(const VC2 &speed)
{
	scrollSpeed1 = speed;
	scrollSpeed2 = speed;
}

//! Set scroll speed of material
/*
	\param speed1
	\param speed2
*/
void Storm3D_Material::SetScrollSpeed(const VC2 &speed1, const VC2 &speed2)
{
	scrollSpeed1 = speed1;
	scrollSpeed2 = speed2;
}

//! Enable or disable scroll
/*
	\param enable
*/
void Storm3D_Material::EnableScroll(bool enable)
{
	scrollEnabled = enable;
}

//! Reset scroll position
void Storm3D_Material::ResetScrollPosition()
{
	scrollOffset1 = VC2();
	scrollOffset2 = VC2();
}

//! Get color of material
/*
	\return color
*/
COL &Storm3D_Material::GetColor()
{
	return color;
}

//! Get self illumination color of material
/*
	\return color
*/
COL &Storm3D_Material::GetSelfIllumination()
{
	return self_illum;
}

//! Get specular color and sharpness of material
/*
	\param _specular specular color
	\param _specular_sharpness specular sharpness
*/
void Storm3D_Material::GetSpecular(COL &_specular,float &_specular_sharpness)
{
	_specular=specular;
	_specular_sharpness=specular_sharpness;
}

//! Get reflection factor of material
/*
	\return reflection factor
*/
float Storm3D_Material::GetReflectionFactor()
{
	if(texture_reflection)
		return texture_reflection->blend_factor;

	return 0.f;
}

//! Get special properties of material
/*
	\param _doublesided
	\param _wireframe
*/
void Storm3D_Material::GetSpecial(bool &_doublesided,bool &_wireframe)
{
	_doublesided=doublesided;
	_wireframe=wireframe;
}

//! Get alpha blending type of material
/*
	\return alpha blending type
*/
Storm3D_Material::ATYPE Storm3D_Material::GetAlphaType()
{
	return alphablend_type;
}

//! Get transparency of material
/*
	\return transparency
*/
float Storm3D_Material::GetTransparency()
{
	return transparency;
}

//! Get glow of material
/*
	\return glow
*/
float Storm3D_Material::GetGlow() const
{
	return glow;
}

//! Get glow factor of material
/*
	\return glow factor
*/
float Storm3D_Material::GetGlowFactor() const
{
	return glow_factor;
}

//! Recalculate multi texture type
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

	bool supports_dot3 = true; // always supported on opengl
	// igios_unimplemented();  // FIXME: implement envbumpmap if time
	bool supports_embm = false; //(Storm3D2->adapters[Storm3D2->active_adapter].caps&Storm3D_Adapter::CAPS_EMBM)!=0;/

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

//! Get base texture of material
/*
	\return base texture
*/
IStorm3D_Texture *Storm3D_Material::GetBaseTexture()
{
	if ((texture_base)&&(texture_base->texture)) return texture_base->texture;
	return NULL;
}

//! Get base texture2 of material
/*
	\return base texture2
*/
IStorm3D_Texture *Storm3D_Material::GetBaseTexture2()
{
	if ((texture_base2)&&(texture_base2->texture)) return texture_base2->texture;
	return NULL;
}

//! Get bump texture of material
/*
	\return bump texture
*/
IStorm3D_Texture *Storm3D_Material::GetBumpTexture()
{
	if ((texture_bump)&&(texture_bump->texture)) return texture_bump->texture;
	return NULL;
}

//! Get reflection texture of material
/*
	\return reflection texture
*/
IStorm3D_Texture *Storm3D_Material::GetReflectionTexture()
{
	if ((texture_reflection)&&(texture_reflection->texture)) return texture_reflection->texture;
	return NULL;
}

//! Get distortion texture of material
/*
	\return distortion texture
*/
IStorm3D_Texture *Storm3D_Material::GetDistortionTexture()
{
	if ((texture_distortion)&&(texture_distortion->texture)) 
		return texture_distortion->texture;

	return NULL;
}

//! Set base texture of material
/*
	\param itexture texture
*/
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

//! Set distortion texture of material
/*
	\param itexture texture
*/
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

//! Set base texture2 of material
/*
	\param itexture texture
	\param blend_op blending operation
	\param blend_factor blending factor
*/
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

//! Set bump texture of material
/*
	\param itexture texture
	\param _bumpheight bump height
*/
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

//! Set reflection texture of material
/*
	\param itexture texture
	\param blend_op blending operation
	\param blend_factor blending factor
*/
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

//! Set local reflection of material
/*
	\param enable enable local reflection
	\param blend_factor blend factor
*/
void Storm3D_Material::SetLocalReflection(bool enable, float blend_factor)
{
	local_reflection = enable;
	reflection_blend_factor = blend_factor;
}

//! Change parameters of base texture2 of material
/*
	\param blend_op blending operation
	\param blend_factor blending factor
*/
void Storm3D_Material::ChangeBaseTexture2Parameters(MTL_BOP op,float blend_factor)
{
	if(texture_base2)
	{
		texture_base2->blend_op=op;
		texture_base2->blend_factor=blend_factor;
	}
}

//! Change parameters of reflection texture of material
/*
	\param blend_op blending operation
	\param blend_factor blending factor
	\param texgen
*/
void Storm3D_Material::ChangeReflectionTextureParameters(MTL_BOP op,float blend_factor,TEX_GEN texgen)
{
	if(texture_reflection)
	{
		texture_reflection->blend_op=op;
		texture_reflection->blend_factor=blend_factor;
		texture_reflection->texcoord_gen=texgen;
	}
}

//! Change bump height of material
/*
	\param _bumpheight bump height
*/
void Storm3D_Material::ChangeBumpHeight(float _bumpheight)
{
	bumpheight=_bumpheight;
}

//! Applies material
/*! Use pass parameter to tell with pass to render. Start with pass=0, and increase it until this routine
	returns false. (=all passes rendered).
	\return false = this was last pass; true = new pass must be rendered
*/
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

	glDisable(GL_ALPHA_TEST);
	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);

	for (int i=0;i<2;i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
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
		// psd: assume lightmap if base2 defined
		if(texture_base2)
		{
			texture_base2->texture->Apply(0);
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, texture_base2->GetMultitexBlendingOp());
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		}
	}
	else if (multitexture_type==MTYPE_TEXTURE)		// Base
	{
		// Set stage (base)
		texture_base->texture->Apply(0);

		// Disable last stage
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
	else if (multitexture_type==MTYPE_DUALTEX)		// Base+Base2
	{
		// Set stage (base)
		texture_base->texture->Apply(0);
		
		// Set stage (base2)
		texture_base2->texture->Apply(1);
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, texture_base2->GetMultitexBlendingOp());

		// Disable last stage
		glActiveTexture(GL_TEXTURE2);
		glClientActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
	else if (multitexture_type==MTYPE_REF)			// Reflection
	{		
		// Set stage (reflection)
		texture_reflection->texture->Apply(0);

		// Reflection texture (or projective?)
		if (texture_reflection->texcoord_gen==TEX_GEN_REFLECTION)
		{
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
			glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
			glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);

			// Correct number of texturecoordinates
			igios_unimplemented(); // set coord count
            /*if (texture_reflection->texture->IsCube())
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3);
			else
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);*/

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
			D3DXMATRIX dxmat(mat);
			setTextureMatrix(0, dxmat);
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

			igios_unimplemented(); // COUNT3
			setTextureMatrix(0, mat);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		}

		// Disable last stage
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
	else if (multitexture_type==MTYPE_TEX_REF)		// Base+Reflection
	{
		// Set stage (base)
		texture_base->texture->Apply(0);

		// Set stage (reflection)
		texture_reflection->texture->Apply(1);
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, texture_reflection->GetMultitexBlendingOp());

		// Reflection texture (or projective?)
		if (texture_reflection->texcoord_gen==TEX_GEN_REFLECTION)
		{
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
			glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
			glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);

			igios_unimplemented();
			/*
			// Correct number of texturecoordinates
			if (texture_reflection->texture->IsCube())
				Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3);
			else
				Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
			*/

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
			D3DXMATRIX dxmat(mat);
			setTextureMatrix(1, dxmat);
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

			setTextureMatrix(1, mat);
			igios_unimplemented(); // count
			//Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3|D3DTTFF_PROJECTED);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		}

		// Disable last stage
		glActiveTexture(GL_TEXTURE2);
		glClientActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}

	// Set diffuse
	GLfloat diff[4];
	GLfloat amb[4];
	diff[0] = amb[0] = color.r;
	diff[1] = amb[1] = color.g;
	diff[2] = amb[2] = color.b;
	diff[3] = amb[3] = 0;

	// Set self.illum
	GLfloat self[4];
	self[0] = self_illum.r;
	self[1] = self_illum.g;
	self[2] = self_illum.b;
	self[3] = 0;

	// Set specular
	GLfloat spec[4];
	spec[0] = specular.r;
	spec[1] = specular.g;
	spec[2] = specular.b;
	spec[3] = 0;

	// Set wireframe
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set alphablending
	if (alphablend_type==ATYPE_NONE)
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	else if (alphablend_type==ATYPE_USE_TRANSPARENCY)
	{
		if (transparency>0.001f)
		{
			diff[3] = 1.0f - transparency;
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glDepthMask(GL_FALSE);
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDepthMask(GL_TRUE);
			glEnable(GL_BLEND);
		}
	}
	else if (alphablend_type==ATYPE_USE_TEXTRANSPARENCY || alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
	{
		diff[3] = 1.0f - transparency;

		glEnable(GL_BLEND);

		if(alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);

		if ((multitexture_type==MTYPE_DOT3_TEX)||(multitexture_type==MTYPE_DOT3_REF))
		{
			// Stage 1 alphamap (no diffuse)
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		}
		else	
		{
			// Stage 0 alphamap * diffuse
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glAlphaFunc(GL_GEQUAL, 1.0f/255.0f);
		glEnable(GL_ALPHA_TEST);
	}
	else if (alphablend_type==ATYPE_ADD)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);
	}
	else if (alphablend_type==ATYPE_MUL)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
	}
	else if (alphablend_type==ATYPE_MUL2X)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
	}

	Storm3D2->active_material=this;

	return false;
}

//! Applies material base texture and alpha. Used on sprites.
void Storm3D_Material::ApplyBaseTextureExtOnly()
{
	if (texture_base)
	{
		texture_base->texture->AnimateVideo();
		texture_base->texture->Apply(0);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_3D, 0);
		glDisable(GL_TEXTURE_3D);
	}

	// Set diffuse
	GLfloat diff[4];
	GLfloat amb[4];
	diff[0] = amb[0] = color.r;
	diff[1] = amb[1] = color.g;
	diff[2] = amb[2] = color.b;
	diff[3] = amb[3] = 0;

	// Set self.illum
	GLfloat self[4];
	self[0] = self_illum.r;
	self[1] = self_illum.g;
	self[2] = self_illum.b;
	self[3] = 0;

	// Set specular
	GLfloat spec[4];
	spec[0] = specular.r;
	spec[1] = specular.g;
	spec[2] = specular.b;
	spec[3] = 0;

	// Set alphablending
	if (alphablend_type==ATYPE_NONE)
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	else if (alphablend_type==ATYPE_USE_TRANSPARENCY)
	{
		if (transparency>0.001f)
		{
			diff[3] = 1.0f - transparency;
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glDepthMask(GL_FALSE);
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}
	}
	else if (alphablend_type==ATYPE_USE_TEXTRANSPARENCY || alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
	{
		diff[3] = 1.0f - transparency;

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);

		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glAlphaFunc(GL_GEQUAL, 1.0f/255.0f);
		glEnable(GL_ALPHA_TEST);
	}
	else if (alphablend_type==ATYPE_ADD)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);
	}
	else if (alphablend_type==ATYPE_MUL)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
	}
	else if (alphablend_type==ATYPE_MUL2X)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
	}

	// Set active material
	Storm3D2->active_material=this;
}

//! Apply only base texture. Used on particle systems.
void Storm3D_Material::ApplyBaseTextureOnly()
{
	if (texture_base)
	{
		texture_base->texture->AnimateVideo();
		texture_base->texture->Apply(0);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

//! Apply only base2texture
void Storm3D_Material::ApplyBase2TextureOnly()
{
	if(texture_base2)
		texture_base2->texture->Apply(1);
	else
	{
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

//! Applies only baseTextureExt without alpha sorting
/*!
	\param scene scene
	\param fvf vertex format
	\param mat matrix
*/
void Storm3D_Material::ApplyBaseTextureExtOnly_NoAlphaSort(Storm3D_Scene *scene,DWORD fvf,D3DMATRIX *mat)
{
	if (texture_base)
	{
		texture_base->texture->AnimateVideo();
		texture_base->texture->Apply(0);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Set diffuse
	GLfloat diff[4];
	GLfloat amb[4];
	diff[0] = amb[0] = color.r;
	diff[1] = amb[1] = color.g;
	diff[2] = amb[2] = color.b;
	diff[3] = amb[3] = 0;

	// Set self.illum
	GLfloat self[4];
	self[0] = self_illum.r;
	self[1] = self_illum.g;
	self[2] = self_illum.b;
	self[3] = 0;

	// Set specular
	GLfloat spec[4];
	spec[0] = specular.r;
	spec[1] = specular.g;
	spec[2] = specular.b;
	spec[3] = 0;

	if (alphablend_type==ATYPE_USE_TEXTRANSPARENCY || alphablend_type==IStorm3D_Material::ATYPE_USE_ALPHATEST)
	{
		diff[3] = 1.0f - transparency;

		// Only alphatest
		glEnable(GL_ALPHA_TEST);
	}

	// Set active material
	Storm3D2->active_material=this;
}

//! Update alpha type
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

//! Compares two materials
/*
	\param other the other material
*/
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

//! Get Texture Coordinate Set Count
/*
	\return
*/
int Storm3D_Material::GetTextureCoordinateSetCount()
{
	if ((multitexture_type==MTYPE_COLORONLY)||(multitexture_type==MTYPE_REF))
		return 0; else return 1;
}

//! Get bump type of material
/*
	\return bump type
*/
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

//! Get multitexture type of material
/*
	\return multitexture type
*/
Storm3D_Material::MTYPE Storm3D_Material::GetMultiTextureType()
{
	return multitexture_type;
}
