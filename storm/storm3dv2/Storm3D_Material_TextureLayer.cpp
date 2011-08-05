// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_material.h"
#include "storm3d_texture.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Material_TextureLayer::Storm3D_Material_TextureLayer
//------------------------------------------------------------------
Storm3D_Material_TextureLayer::Storm3D_Material_TextureLayer(Storm3D *s2,Storm3D_Texture *_texture,
		Storm3D_Material::MTL_BOP _blend_op,float _blend_factor,Storm3D_Material::TEX_GEN texgen) :
	Storm3D2(s2),
	texture(_texture),
	blend_op(_blend_op),
	blend_factor(_blend_factor),
	texcoord_gen(texgen)
{
}



//------------------------------------------------------------------
// Storm3D_Material_TextureLayer::~Storm3D_Material_TextureLayer
//------------------------------------------------------------------
Storm3D_Material_TextureLayer::~Storm3D_Material_TextureLayer()
{
	// Delete the texture (actually decreases ref.count)
	if (texture)
		texture->Release();
}



//------------------------------------------------------------------
// Storm3D_Material_TextureLayer::CreateNewClone - clone the texlayer
// -jpk
//------------------------------------------------------------------
Storm3D_Material_TextureLayer *Storm3D_Material_TextureLayer::CreateNewClone()
{
	Storm3D_Material_TextureLayer *ret = new Storm3D_Material_TextureLayer(
		Storm3D2, texture, blend_op, blend_factor, texcoord_gen);

	if (texture)
		texture->AddRef();

	return ret;
}



//------------------------------------------------------------------
// Compare (used when new material is loaded -> saves memory and processing power)
//------------------------------------------------------------------
bool Storm3D_Material_TextureLayer::IsIdenticalWith(const Storm3D_Material_TextureLayer *other) const
{
	if (texture!=other->texture) return false;
	if (blend_op!=other->blend_op) return false;
	if (texcoord_gen!=other->texcoord_gen) return false;
	
	if (fabsf(blend_factor-other->blend_factor)>0.001f) return false;

	// Its the same (at last)
	return true;
}


//------------------------------------------------------------------
// Storm3D_Material_TextureLayer::GetDX8MultitexBlendingOp
//------------------------------------------------------------------
D3DTEXTUREOP Storm3D_Material_TextureLayer::GetDX8MultitexBlendingOp()
{
	switch(blend_op)
	{
		case Storm3D_Material::MTL_BOP_MUL: return D3DTOP_MODULATE;
		case Storm3D_Material::MTL_BOP_MUL2X: return D3DTOP_MODULATE2X;
		case Storm3D_Material::MTL_BOP_MUL4X: return D3DTOP_MODULATE4X;
		case Storm3D_Material::MTL_BOP_ADD: return D3DTOP_ADD;
		case Storm3D_Material::MTL_BOP_SUB: return D3DTOP_SUBTRACT;
		case Storm3D_Material::MTL_BOP_ADDSUB: return D3DTOP_ADDSIGNED;
		case Storm3D_Material::MTL_BOP_ADDSUB2X: return D3DTOP_ADDSIGNED2X;
		case Storm3D_Material::MTL_BOP_BLENDFACTOR: return D3DTOP_BLENDFACTORALPHA;
	}
	return D3DTOP_MODULATE;
}


