// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <vector>
#include <string>
#include <stdio.h>
#include <boost/static_assert.hpp>

#include <d3d9.h>
#include <d3dx9core.h>

#include "Storm3D_ShaderManager.h"
#include "storm3d_model.h"
#include "storm3d_model_object.h"
#include "Storm3D_Bone.h"
#include "storm3d_mesh.h"

#include "../../filesystem/input_stream_wrapper.h"
#include "../../util/Debug_MemoryManager.h"

using namespace frozenbyte;

namespace {
	static const int DEFAULT_SHADER = 1;
	static const int DEFAULT_PROJECTED_SHADER_DIRECTIONAL = 2;
	static const int DEFAULT_PROJECTED_SHADER_POINT = 3;
	static const int DEFAULT_PROJECTED_SHADER_FLAT = 4;
	static const int BONE_SHADER = 5;
	static const int BONE_PROJECTED_SHADER_DIRECTIONAL = 6;
	static const int BONE_PROJECTED_SHADER_POINT = 7;
	static const int BONE_PROJECTED_SHADER_FLAT = 8;

	static const int ATI_DEPTH_SHADER = 9;
	static const int ATI_DEPTH_BONE_SHADER = 10;
	static const int ATI_SHADOW_SHADER_DIRECTIONAL = 11;
	static const int ATI_SHADOW_SHADER_POINT = 12;
	static const int ATI_SHADOW_SHADER_FLAT = 13;
	static const int ATI_SHADOW_BONE_SHADER_DIRECTIONAL = 14;
	static const int ATI_SHADOW_BONE_SHADER_POINT = 15;
	static const int ATI_SHADOW_BONE_SHADER_FLAT = 16;

	static const int FAKE_DEPTH_SHADER = 17;
	static const int FAKE_SHADOW_SHADER = 18;
	static const int FAKE_DEPTH_BONE_SHADER = 19;
	static const int FAKE_SHADOW_BONE_SHADER = 20;

	static const int LIGHTING_SHADER = 21;
	static const int BONE_LIGHTING_SHADER = 22;

}

	//HAXHAX
	bool enableLocalReflection = false;
	float reflection_height = 0.f;
	D3DXMATRIX reflection_matrix;
	D3DXMATRIX clip_matrix;

// 106 = 21
// 107 = 22
// 108 = 23
// 109 = 24
// 110 = 25
// 111 = 26

// 21, 25
const int Storm3D_ShaderManager::BONE_INDEX_START = 27;
const int Storm3D_ShaderManager::BONE_INDICES = 23;

Storm3D_ShaderManager::Storm3D_ShaderManager(IDirect3DDevice9 &device)
:	ambient_color(.7f,.7f,.7f,0),
	ambient_force_color(0,0,0,0),
	fog(-20.f,1.f / 100.f,0.0f,1.f),

	object_ambient_color(0,0,0,0),
	object_diffuse_color(1.f,1.f,1.f,0),

	update_values(true),

	default_shader(device),
	lighting_shader_0light_noreflection(device),
	lighting_shader_0light_localreflection(device),
	lighting_shader_0light_reflection(device),
	lighting_shader_1light_noreflection(device),
	lighting_shader_1light_localreflection(device),
	lighting_shader_1light_reflection(device),
	lighting_shader_2light_noreflection(device),
	lighting_shader_2light_localreflection(device),
	lighting_shader_2light_reflection(device),
	lighting_shader_3light_noreflection(device),
	lighting_shader_3light_localreflection(device),
	lighting_shader_3light_reflection(device),
	lighting_shader_4light_noreflection(device),
	lighting_shader_4light_localreflection(device),
	lighting_shader_4light_reflection(device),
	lighting_shader_5light_noreflection(device),
	lighting_shader_5light_localreflection(device),
	lighting_shader_5light_reflection(device),

	skybox_shader(device),
	default_projected_shader_directional(device),
	default_projected_shader_point(device),
	default_projected_shader_flat(device),
	bone_shader(device),
	bone_lighting_shader_0light_noreflection(device),
	bone_lighting_shader_0light_reflection(device),
	bone_lighting_shader_1light_noreflection(device),
	bone_lighting_shader_1light_reflection(device),
	bone_lighting_shader_2light_noreflection(device),
	bone_lighting_shader_2light_reflection(device),
	bone_lighting_shader_3light_noreflection(device),
	bone_lighting_shader_3light_reflection(device),
	bone_lighting_shader_4light_noreflection(device),
	bone_lighting_shader_4light_reflection(device),
	bone_lighting_shader_5light_noreflection(device),
	bone_lighting_shader_5light_reflection(device),

	bone_projected_shader_directional(device),
	bone_projected_shader_point(device),
	bone_projected_shader_flat(device),

	ati_depth_default_shader(device),
	ati_depth_bone_shader(device),
	ati_shadow_default_shader_directional(device),
	ati_shadow_default_shader_point(device),
	ati_shadow_default_shader_flat(device),
	ati_shadow_bone_shader_directional(device),
	ati_shadow_bone_shader_point(device),
	ati_shadow_bone_shader_flat(device),

	fake_depth_shader(device),
	fake_shadow_shader(device),
	fake_depth_bone_shader(device),
	fake_shadow_bone_shader(device),

	current_shader(0),
	software_shaders(true),
	projected_shaders(false),
	ati_depth_shaders(false),
	ati_shadow_shaders(false),
	fake_depth_shaders(false),
	fake_shadow_shaders(false),
	model(0),

	transparency_factor(1.f),

	reflection(false),
	local_reflection(false),
	light_count(0),
	light_params_changed(true),
	spot_type(Directional)
{
	for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
	{
		light_position[i].x = 0;
		light_position[i].y = 25.f;
		light_position[i].z = 0;
		light_color[i].x = .5f;
		light_color[i].y = .5f;
		light_color[i].z = .5f;
	}

	sun_properties.x = 0.f;
	sun_properties.y = 0.f;
	sun_properties.z = 0.f;
	sun_properties.w = 0.f;

	D3DXMatrixIdentity(&reflection_matrix);
	reflection_matrix._22 = -1.f;
	reflection_matrix._42 = 2 * reflection_height;
}

Storm3D_ShaderManager::~Storm3D_ShaderManager()
{
}

void Storm3D_ShaderManager::CreateShaders(IDirect3DDevice9 *device, bool hw_shader)
{
	if(hw_shader == true)
		software_shaders = false;
	else
		software_shaders = true;

	default_shader.createDefaultShader();
	lighting_shader_0light_noreflection.createLightingShader_0light_noreflection();
	lighting_shader_0light_localreflection.createLightingShader_0light_localreflection();
	lighting_shader_0light_reflection.createLightingShader_0light_reflection();
	lighting_shader_1light_noreflection.createLightingShader_1light_noreflection();
	lighting_shader_1light_localreflection.createLightingShader_1light_localreflection();
	lighting_shader_1light_reflection.createLightingShader_1light_reflection();
	lighting_shader_2light_noreflection.createLightingShader_2light_noreflection();
	lighting_shader_2light_localreflection.createLightingShader_2light_localreflection();
	lighting_shader_2light_reflection.createLightingShader_2light_reflection();
	lighting_shader_3light_noreflection.createLightingShader_3light_noreflection();
	lighting_shader_3light_localreflection.createLightingShader_3light_localreflection();
	lighting_shader_3light_reflection.createLightingShader_3light_reflection();
	lighting_shader_4light_noreflection.createLightingShader_4light_noreflection();
	lighting_shader_4light_localreflection.createLightingShader_4light_localreflection();
	lighting_shader_4light_reflection.createLightingShader_4light_reflection();
	lighting_shader_5light_noreflection.createLightingShader_5light_noreflection();
	lighting_shader_5light_localreflection.createLightingShader_5light_localreflection();
	lighting_shader_5light_reflection.createLightingShader_5light_reflection();

	skybox_shader.createSkyboxShader();
	default_projected_shader_directional.createDefaultProjectionShaderDirectional();
	default_projected_shader_point.createDefaultProjectionShaderPoint();
	default_projected_shader_flat.createDefaultProjectionShaderFlat();
	bone_shader.createBoneShader();

	bone_lighting_shader_0light_noreflection.createBoneLightingShader_0light_noreflection();
	bone_lighting_shader_0light_reflection.createBoneLightingShader_0light_reflection();
	bone_lighting_shader_1light_noreflection.createBoneLightingShader_1light_noreflection();
	bone_lighting_shader_1light_reflection.createBoneLightingShader_1light_reflection();
	bone_lighting_shader_2light_noreflection.createBoneLightingShader_2light_noreflection();
	bone_lighting_shader_2light_reflection.createBoneLightingShader_2light_reflection();
	bone_lighting_shader_3light_noreflection.createBoneLightingShader_3light_noreflection();
	bone_lighting_shader_3light_reflection.createBoneLightingShader_3light_reflection();
	bone_lighting_shader_4light_noreflection.createBoneLightingShader_4light_noreflection();
	bone_lighting_shader_4light_reflection.createBoneLightingShader_4light_reflection();
	bone_lighting_shader_5light_noreflection.createBoneLightingShader_5light_noreflection();
	bone_lighting_shader_5light_reflection.createBoneLightingShader_5light_reflection();

	bone_projected_shader_directional.createBoneProjectionShaderDirectional();
	bone_projected_shader_point.createBoneProjectionShaderPoint();
	bone_projected_shader_flat.createBoneProjectionShaderFlat();

	fake_depth_shader.createFakeDepthShader();
	fake_shadow_shader.createFakeShadowShader();
	fake_depth_bone_shader.createFakeDepthBoneShader();
	fake_shadow_bone_shader.createFakeShadowBoneShader();

	// Set identity matrix on card
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	device->SetVertexShaderConstantF(BONE_INDEX_START, identity, 3);
}

void Storm3D_ShaderManager::CreateAtiShaders(IDirect3DDevice9 *device)
{
	ati_depth_default_shader.createAtiDepthShader();
	ati_depth_bone_shader.createAtiBoneDepthShader();	
	ati_shadow_default_shader_directional.createAtiShadowShaderDirectional();
	ati_shadow_default_shader_point.createAtiShadowShaderPoint();
	ati_shadow_default_shader_flat.createAtiShadowShaderFlat();
	ati_shadow_bone_shader_directional.createAtiBoneShadowShaderDirectional();
	ati_shadow_bone_shader_point.createAtiBoneShadowShaderPoint();
	ati_shadow_bone_shader_flat.createAtiBoneShadowShaderFlat();
}

void Storm3D_ShaderManager::setLightingParameters(bool reflection_, bool local_reflection_, int light_count_)
{
	if(light_count_ > LIGHT_MAX_AMOUNT)
		light_count_ = LIGHT_MAX_AMOUNT;
	if(light_count_ < 0)
		light_count_ = 0;

	if(reflection != reflection_ || light_count != light_count_ || local_reflection != local_reflection_)
	{
		reflection = reflection_;
		local_reflection = local_reflection_;
		light_count = light_count_;
		light_params_changed = true;
	}
}

void Storm3D_ShaderManager::SetTransparencyFactor(float factor)
{
	transparency_factor = factor;
}

void Storm3D_ShaderManager::SetViewProjectionMatrix(const D3DXMATRIX &vp, const D3DXMATRIX &view)
{
	view_projection_tm = vp;

	D3DXMatrixIdentity(&clip_matrix);
	D3DXMatrixIdentity(&reflection_matrix);

	// HAXHAX
	if(enableLocalReflection)
	{
#ifdef PROJECT_CLAW_PROTO
		//reflection_height = 17.8f;
#endif
		{
			reflection_matrix._22 = -1.f;
			reflection_matrix._42 = 2 * reflection_height;
		}

		D3DXMatrixMultiply(&view_projection_tm, &reflection_matrix, &view_projection_tm);

		// Oblique depth projection
		{
			// Wanted plane
			D3DXPLANE plane;
			D3DXVECTOR3 point(0.f, reflection_height, 0.f);
			D3DXVECTOR3 normal(0.f, 1.f, 0.f);
			D3DXPlaneFromPointNormal(&plane, &point, &normal);
			D3DXVECTOR4 clipPlane(plane.a, plane.b, plane.c, plane.d);

			// Transform plane
			D3DXMATRIX normalizedViewProjection;
			D3DXMatrixInverse(&normalizedViewProjection, 0, &view_projection_tm);
			D3DXMatrixTranspose(&normalizedViewProjection, &normalizedViewProjection);

			D3DXVECTOR4 projectedPlane;
			D3DXVec4Transform(&projectedPlane, &clipPlane, &normalizedViewProjection);

			if(projectedPlane.w > 0)
			{
				D3DXVECTOR4 tempPlane = -clipPlane;
				D3DXVec4Transform(&projectedPlane, &tempPlane, &normalizedViewProjection);
			}

			// Create skew matrix
			D3DXMatrixIdentity(&clip_matrix);
			clip_matrix(0, 2) = projectedPlane.x;
			clip_matrix(1, 2) = projectedPlane.y;
			clip_matrix(2, 2) = projectedPlane.z;
			clip_matrix(3, 2) = projectedPlane.w;
			view_projection_tm = view_projection_tm * clip_matrix;
		}
	}
}

void Storm3D_ShaderManager::SetViewPosition(const D3DXVECTOR4 &p)
{
	view_position = p;
}

void Storm3D_ShaderManager::SetAmbient(const Color &color)
{
	ambient_color.x = color.r;
	ambient_color.y = color.g;
	ambient_color.z = color.b;

	object_ambient_color = ambient_color;
	update_values = true;
}

void Storm3D_ShaderManager::SetForceAmbient(const Color &color)
{
	ambient_force_color.x = color.r;
	ambient_force_color.y = color.g;
	ambient_force_color.z = color.b;
}

void Storm3D_ShaderManager::SetLight(int index, const Vector &direction, const Color &color, float range)
{
	if(index >= 0 && index < LIGHT_MAX_AMOUNT)
	{
		light_position[index].x = direction.x;
		light_position[index].y = direction.y;
		light_position[index].z = direction.z;
		if(index == 0)
			light_position[index].w = 0.f;

		light_color[index].x = color.r;
		light_color[index].y = color.g;
		light_color[index].z = color.b;
		light_color[index].w = 1.f / range;
	}
}

void Storm3D_ShaderManager::SetSun(const Vector &direction, float strength)
{
	sun_properties.x = direction.x;
	sun_properties.y = direction.y;
	sun_properties.z = direction.z;
	sun_properties.w = strength;

	update_values = true;
}

void Storm3D_ShaderManager::SetFog(float start, float range)
{
	fog.x = start - range;
	fog.y = 1.f / range;
	fog.z = 0.f;
	fog.w = 1.f;

	update_values = true;
}

void Storm3D_ShaderManager::SetTextureOffset(const VC2 &offset)
{
	textureOffset.x = offset.x;
	textureOffset.y = offset.y;
}

void Storm3D_ShaderManager::setFakeProperties(float plane, float factor, float add)
{
	fake_properties.x = plane;
	fake_properties.y = factor;
	fake_properties.z = add;
}

void Storm3D_ShaderManager::SetModelAmbient(const Color &color)
{
	model_ambient_color.x = color.r;
	model_ambient_color.y = color.g;
	model_ambient_color.z = color.b;

	update_values = true;
}

void Storm3D_ShaderManager::SetObjectAmbient(const Color &color)
{
	object_ambient_color.x = color.r;
	object_ambient_color.y = color.g;
	object_ambient_color.z = color.b;

	update_values = true;
}

void Storm3D_ShaderManager::SetObjectDiffuse(const Color &color)
{
	object_diffuse_color.x = color.r;
	object_diffuse_color.y = color.g;
	object_diffuse_color.z = color.b;

	update_values = true;
}

void Storm3D_ShaderManager::setProjectedShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = true;
	ati_depth_shaders = false;
	ati_shadow_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

void Storm3D_ShaderManager::setAtiDepthShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	ati_depth_shaders = true;
	ati_shadow_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

void Storm3D_ShaderManager::setAtiShadowShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	ati_depth_shaders = false;
	ati_shadow_shaders = true;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

void Storm3D_ShaderManager::setLightingShaders()
{
	lighting_shaders = true;
	projected_shaders = false;
	ati_depth_shaders = false;
	ati_shadow_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

void Storm3D_ShaderManager::setNormalShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	ati_depth_shaders = false;
	ati_shadow_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

void Storm3D_ShaderManager::setFakeDepthShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	ati_depth_shaders = false;
	ati_shadow_shaders = false;
	fake_depth_shaders = true;
	fake_shadow_shaders = false;
}

void Storm3D_ShaderManager::setFakeShadowShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	ati_depth_shaders = false;
	ati_shadow_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = true;
}

void Storm3D_ShaderManager::setTextureTm(D3DXMATRIX &matrix)
{
	D3DXMatrixTranspose(&texture_matrix, &matrix);
}

void Storm3D_ShaderManager::setSpot(const COL &color, const VC3 &position, const VC3 &direction, float range, float fadeFactor)
{
	spot_color.x = color.r;
	spot_color.y = color.g;
	spot_color.z = color.b;
	spot_color.w = 1.f;

	spot_position.x = position.x;
	spot_position.y = position.y;
	spot_position.z = position.z;
	spot_position.w = 1.f / range;

	spot_properties.x = -direction.x;
	spot_properties.y = -direction.y;
	spot_properties.z = -direction.z;
	spot_properties.w = 1.f / range;
}

void Storm3D_ShaderManager::setSpotTarget(const D3DXMATRIX &matrix)
{
	D3DXMatrixTranspose(&target_matrix, &matrix);
}

void Storm3D_ShaderManager::setSpotType(SpotType type)
{
	spot_type = type;
}

bool Storm3D_ShaderManager::SoftwareShaders()
{
	return software_shaders;
}

bool Storm3D_ShaderManager::BoneShader()
{
	switch(current_shader)
	{
		case BONE_SHADER:
		case BONE_LIGHTING_SHADER:
		case BONE_PROJECTED_SHADER_DIRECTIONAL:
		case BONE_PROJECTED_SHADER_POINT:
		case BONE_PROJECTED_SHADER_FLAT:
		case ATI_DEPTH_BONE_SHADER:
		case ATI_SHADOW_BONE_SHADER_DIRECTIONAL:
		case ATI_SHADOW_BONE_SHADER_POINT:
		case ATI_SHADOW_BONE_SHADER_FLAT:
		case FAKE_DEPTH_BONE_SHADER:
		case FAKE_SHADOW_BONE_SHADER:
			return true;
	}

	return false;
}

void Storm3D_ShaderManager::SetShader(IDirect3DDevice9 *device, Storm3D_Model_Object *object)
{
	assert(device);

	D3DXMATRIX object_tm;
	object->GetMXG().GetAsD3DCompatible4x4((float *) &object_tm);
	SetWorldTransform(*device, object_tm);

	IStorm3D_Material *m = object->GetMesh()->GetMaterial();
	float alpha = 1.f;

	float force_alpha = object->force_alpha;
	if((projected_shaders || ati_shadow_shaders) && object->force_lighting_alpha_enable)
		force_alpha = object->force_lighting_alpha;

	IStorm3D_Material::ATYPE a = m->GetAlphaType();
	if(a == IStorm3D_Material::ATYPE_USE_TRANSPARENCY)
		alpha = 1.f - m->GetTransparency() - force_alpha;
	else if(a == IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY || force_alpha > 0.0001f)
		alpha = 1.f - m->GetTransparency() - force_alpha;
	else if(a == IStorm3D_Material::ATYPE_USE_ALPHATEST)
		alpha = 1.f - m->GetTransparency();
	//else if(a == IStorm3D_Material::ATYPE_MUL)
	//	alpha = 1.f - m->GetTransparency() - force_alpha;

	if(alpha < 0)
		alpha = 0;

//if(!lighting_shaders && !projected_shaders && !ati_depth_shaders && !ati_shadow_shaders && !fake_depth_shaders && !fake_shadow_shaders)
//	alpha = 0.8f;

	if(!projected_shaders && !ati_shadow_shaders && !fake_shadow_shaders && !fake_depth_shaders)
	{
		D3DXMatrixTranspose(&object_tm, &object_tm);
		device->SetVertexShaderConstantF(4, object_tm, 3);

		//if(update_values == true)
		{
			// Constants
			device->SetVertexShaderConstantF(7, object_ambient_color, 1);
			device->SetVertexShaderConstantF(8, object_diffuse_color, 1);
			update_values = false;	
		}

		// Set transparency?
		D3DXVECTOR4 ambient = ambient_color;
		ambient *= sun_properties.w;
		ambient += model_ambient_color + ambient_force_color;
		//ambient += object_ambient_color;

		ambient.x = max(ambient.x, object_ambient_color.x);
		ambient.y = max(ambient.y, object_ambient_color.y);
		ambient.z = max(ambient.z, object_ambient_color.z);

#ifdef HACKY_SG_AMBIENT_LIGHT_FIX
		// EVIL HAX around too dark characters etc.
		const float MIN_AMBIENT_LIGHT = 0.05f;
		ambient.x = max(ambient.x, MIN_AMBIENT_LIGHT);
		ambient.y = max(ambient.y, MIN_AMBIENT_LIGHT);
		ambient.z = max(ambient.z, MIN_AMBIENT_LIGHT);

		ambient.x = min(ambient.x, 1.0f);
		ambient.y = min(ambient.y, 1.0f);
		ambient.z = min(ambient.z, 1.0f);
#else
		if(ambient.x > 1.f)
			ambient.x = 1.f;
		if(ambient.y > 1.f)
			ambient.y = 1.f;
		if(ambient.z > 1.f)
			ambient.z = 1.f;
#endif

		ambient.w = alpha; //1.f;

		device->SetVertexShaderConstantF(7, ambient, 1);
	}

	if(projected_shaders || ati_shadow_shaders)
	{
		D3DXVECTOR4 dif = object_diffuse_color;
		dif.x *= spot_color.x;
		dif.y *= spot_color.y;
		dif.z *= spot_color.z;

		dif.w = alpha * transparency_factor;
		device->SetVertexShaderConstantF(17, dif, 1);
	}

	if(fake_depth_shaders)
	{
		D3DXMatrixTranspose(&object_tm, &object_tm);
		device->SetVertexShaderConstantF(13, object_tm, 3);
	}

	// Set actual shader
	if(object->parent_model->bones.empty() || (static_cast<Storm3D_Mesh *> (object->GetMesh())->HasWeights() == false))
	{
		model = NULL;

		if(lighting_shaders)
		{
			if(current_shader != LIGHTING_SHADER || this->light_params_changed)
			{
				current_shader = LIGHTING_SHADER;

				if(reflection)
				{
					if(light_count == 0)
						lighting_shader_0light_reflection.apply();
					else if(light_count == 1)
						lighting_shader_1light_reflection.apply();
					else if(light_count == 2)
						lighting_shader_2light_reflection.apply();
					else if(light_count == 3)
						lighting_shader_3light_reflection.apply();
					else if(light_count == 4)
						lighting_shader_4light_reflection.apply();
					else
						lighting_shader_5light_reflection.apply();
				}
				else if(local_reflection)
				{
					if(light_count == 0)
						lighting_shader_0light_localreflection.apply();
					else if(light_count == 1)
						lighting_shader_1light_localreflection.apply();
					else if(light_count == 2)
						lighting_shader_2light_localreflection.apply();
					else if(light_count == 3)
						lighting_shader_3light_localreflection.apply();
					else if(light_count == 4)
						lighting_shader_4light_localreflection.apply();
					else
						lighting_shader_5light_localreflection.apply();
				}
				else
				{
					if(light_count == 0)
						lighting_shader_0light_noreflection.apply();
					else if(light_count == 1)
						lighting_shader_1light_noreflection.apply();
					else if(light_count == 2)
						lighting_shader_2light_noreflection.apply();
					else if(light_count == 3)
						lighting_shader_3light_noreflection.apply();
					else if(light_count == 4)
						lighting_shader_4light_noreflection.apply();
					else
						lighting_shader_5light_noreflection.apply();
				}

				light_params_changed = false;
			}
		}
		else if(projected_shaders)
		{
			if(spot_type == Directional)
			if(current_shader != DEFAULT_PROJECTED_SHADER_DIRECTIONAL)
			{
				default_projected_shader_directional.apply();
				current_shader = DEFAULT_PROJECTED_SHADER_DIRECTIONAL;
			}

			if(spot_type == Point)
			if(current_shader != DEFAULT_PROJECTED_SHADER_POINT)
			{
				default_projected_shader_point.apply();
				current_shader = DEFAULT_PROJECTED_SHADER_POINT;
			}

			if(spot_type == Flat)
			if(current_shader != DEFAULT_PROJECTED_SHADER_FLAT)
			{
				default_projected_shader_flat.apply();
				current_shader = DEFAULT_PROJECTED_SHADER_FLAT;
			}
		}
		else if(ati_depth_shaders)
		{
			if(current_shader != ATI_DEPTH_SHADER)
			{
				ati_depth_default_shader.apply();
				current_shader = ATI_DEPTH_SHADER;
			}
		}
		else if(ati_shadow_shaders)
		{
			if(spot_type == Directional)
			if(current_shader != ATI_SHADOW_SHADER_DIRECTIONAL)
			{
				ati_shadow_default_shader_directional.apply();
				current_shader = ATI_SHADOW_SHADER_DIRECTIONAL;
			}

			if(spot_type == Point)
			if(current_shader != ATI_SHADOW_SHADER_POINT)
			{
				ati_shadow_default_shader_point.apply();
				current_shader = ATI_SHADOW_SHADER_POINT;
			}

			if(spot_type == Flat)
			if(current_shader != ATI_SHADOW_SHADER_FLAT)
			{
				ati_shadow_default_shader_flat.apply();
				current_shader = ATI_SHADOW_SHADER_FLAT;
			}
		}
		else if(fake_depth_shaders)
		{
			if(current_shader != FAKE_DEPTH_SHADER)
			{
				fake_depth_shader.apply();
				current_shader = FAKE_DEPTH_SHADER;
			}
		}
		else if(fake_shadow_shaders)
		{
			if(current_shader != FAKE_SHADOW_SHADER)
			{
				fake_shadow_shader.apply();
				current_shader = FAKE_SHADOW_SHADER;
			}
		}
		else
		{
			if(current_shader != DEFAULT_SHADER)
			{
				default_shader.apply();
				current_shader = DEFAULT_SHADER;
			}
		}
	}
	else
	{
		model = object->parent_model;

		if(lighting_shaders)
		{
			if(current_shader != BONE_LIGHTING_SHADER || this->light_params_changed)
			{
				current_shader = BONE_LIGHTING_SHADER;

				if(reflection)
				{
					if(light_count == 0)
						bone_lighting_shader_0light_reflection.apply();
					else if(light_count == 1)
						bone_lighting_shader_1light_reflection.apply();
					else if(light_count == 2)
						bone_lighting_shader_2light_reflection.apply();
					else if(light_count == 3)
						bone_lighting_shader_3light_reflection.apply();
					else if(light_count == 4)
						bone_lighting_shader_4light_reflection.apply();
					else
						bone_lighting_shader_5light_reflection.apply();
				}
				else
				{
					if(light_count == 0)
						bone_lighting_shader_0light_noreflection.apply();
					else if(light_count == 1)
						bone_lighting_shader_1light_noreflection.apply();
					else if(light_count == 2)
						bone_lighting_shader_2light_noreflection.apply();
					else if(light_count == 3)
						bone_lighting_shader_3light_noreflection.apply();
					else if(light_count == 4)
						bone_lighting_shader_4light_noreflection.apply();
					else
						bone_lighting_shader_5light_noreflection.apply();
				}
			}
		}
		else if(projected_shaders)
		{
			if(spot_type == Directional)
			if(current_shader != BONE_PROJECTED_SHADER_DIRECTIONAL)
			{
				bone_projected_shader_directional.apply();
				current_shader = BONE_PROJECTED_SHADER_DIRECTIONAL;
			}

			if(spot_type == Point)
			if(current_shader != BONE_PROJECTED_SHADER_POINT)
			{
				bone_projected_shader_point.apply();
				current_shader = BONE_PROJECTED_SHADER_POINT;
			}

			if(spot_type == Flat)
			if(current_shader != BONE_PROJECTED_SHADER_FLAT)
			{
				bone_projected_shader_flat.apply();
				current_shader = BONE_PROJECTED_SHADER_FLAT;
			}

		}
		else if(ati_depth_shaders)
		{
			if(current_shader != ATI_DEPTH_BONE_SHADER)
			{
				ati_depth_bone_shader.apply();
				current_shader = ATI_DEPTH_BONE_SHADER;
			}
		}
		else if(ati_shadow_shaders)
		{
			if(spot_type == Directional)
			if(current_shader != ATI_SHADOW_BONE_SHADER_DIRECTIONAL)
			{
				ati_shadow_bone_shader_directional.apply();
				current_shader = ATI_SHADOW_BONE_SHADER_DIRECTIONAL;
			}

			if(spot_type == Point)
			if(current_shader != ATI_SHADOW_BONE_SHADER_POINT)
			{
				ati_shadow_bone_shader_point.apply();
				current_shader = ATI_SHADOW_BONE_SHADER_POINT;
			}

			if(spot_type == Flat)
			if(current_shader != ATI_SHADOW_BONE_SHADER_FLAT)
			{
				ati_shadow_bone_shader_flat.apply();
				current_shader = ATI_SHADOW_BONE_SHADER_FLAT;
			}

		}
		else if(fake_depth_shaders)
		{
			if(current_shader != FAKE_DEPTH_BONE_SHADER)
			{
				fake_depth_bone_shader.apply();
				current_shader = FAKE_DEPTH_BONE_SHADER;
			}
		}
		else if(fake_shadow_shaders)
		{
			if(current_shader != FAKE_SHADOW_BONE_SHADER)
			{
				fake_shadow_bone_shader.apply();
				current_shader = FAKE_SHADOW_BONE_SHADER;
			}
		}
		else
		{
			if(current_shader != BONE_SHADER)
			{
				bone_shader.apply();
				current_shader = BONE_SHADER;
			}
		}
	}
}

void Storm3D_ShaderManager::SetShader(IDirect3DDevice9 *device, const std::vector<int> &bone_indices)
{
	bool setIndices = false;

	if(BoneShader())
	{
		if(!bone_indices.empty() && model)
			setIndices = true;
	}

	if(setIndices)
	{
		float array[96 * 4];
		int bone_amount = model->bones.size();

		D3DXMATRIX foo;
		for(unsigned int i = 0; i < bone_indices.size(); ++i)
		{
			int index = bone_indices[i];
			int shader_index = i; //bone_indices[i].second;

			if(index >= bone_amount)
				continue;

			const MAT &vertexTm = model->bones[index]->GetVertexTransform();
			//vertexTm.GetAsD3DCompatible4x4((float *) foo);
			//D3DXMatrixTranspose(&foo, &foo);
			//device->SetVertexShaderConstantF(BONE_INDEX_START + ((shader_index)*3), foo, 3);

			int arrayIndex = shader_index * 3 * 4;
			//for(unsigned int j = 0; j < 12; ++j)
			//	array[arrayIndex++] = foo[j];

			array[arrayIndex++] = vertexTm.Get(0);
			array[arrayIndex++] = vertexTm.Get(4);
			array[arrayIndex++] = vertexTm.Get(8);
			array[arrayIndex++] = vertexTm.Get(12);
			array[arrayIndex++] = vertexTm.Get(1);
			array[arrayIndex++] = vertexTm.Get(5);
			array[arrayIndex++] = vertexTm.Get(9);
			array[arrayIndex++] = vertexTm.Get(13);
			array[arrayIndex++] = vertexTm.Get(2);
			array[arrayIndex++] = vertexTm.Get(6);
			array[arrayIndex++] = vertexTm.Get(10);
			array[arrayIndex++] = vertexTm.Get(14);
		}

		device->SetVertexShaderConstantF(BONE_INDEX_START, array, 3 * bone_indices.size());
	}
}

void Storm3D_ShaderManager::ResetShader()
{
	light_count = 1000000000;
	current_shader = 0;
	model = 0;
	update_values = true;
	projected_shaders = false;
	lighting_shaders = false;
	// ...
	ati_depth_shaders = false;
	ati_shadow_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;

	object_ambient_color = ambient_color;
	transparency_factor = 1.f;
}

void Storm3D_ShaderManager::ClearCache()
{
	transparency_factor = 1.f;
	current_shader = 0;
	model = 0;
	update_values = true;

	object_ambient_color = ambient_color;
	object_diffuse_color.x = 1.f;
	object_diffuse_color.y = 1.f;
	object_diffuse_color.z = 1.f;
	object_diffuse_color.w = 1.f;
	sun_properties.w = 1.f;

	model_ambient_color.x = 0.f;
	model_ambient_color.y = 0.f;
	model_ambient_color.z = 0.f;
	model_ambient_color.w = 0.f;
}

void Storm3D_ShaderManager::BackgroundShader(IDirect3DDevice9 *device)
{
	//current_shader = 0; //DEFAULT_SHADER;
	//update_values = false;

	//SetShaderDefaultValues(device);
	//default_shader.apply();
	current_shader = 0;
	skybox_shader.apply();
}

void Storm3D_ShaderManager::SetShaderDefaultValues(IDirect3DDevice9 *device)
{
	// Set values
	D3DXVECTOR4 foo(1,1,1,1);
	device->SetVertexShaderConstantF(7, foo, 1);
	device->SetVertexShaderConstantF(8, foo, 1);
}

void Storm3D_ShaderManager::SetShaderAmbient(IDirect3DDevice9 *device, const COL &color)
{
	D3DXVECTOR4 ambient(color.r, color.g, color.b, 0.f);
	D3DXVECTOR4 diffuse(1, 1, 1, 0);

	device->SetVertexShaderConstantF(7, ambient, 1);
	device->SetVertexShaderConstantF(8, diffuse, 1);
}

void Storm3D_ShaderManager::SetShaderDiffuse(IDirect3DDevice9 *device, const COL &color)
{
	D3DXVECTOR4 diffuse(color.r, color.g, color.b, 0.f);
	device->SetVertexShaderConstantF(8, diffuse, 1);
}

void Storm3D_ShaderManager::SetLightmapFactor(float xf, float yf)
{
	lightmap_factor.x = xf;
	lightmap_factor.y = yf;
	lightmap_factor.z = 0;
	lightmap_factor.w = 0;
}

void Storm3D_ShaderManager::ApplyDeclaration(IDirect3DDevice9 &device)
{
	default_shader.applyDeclaration();
}

void Storm3D_ShaderManager::SetWorldTransform(IDirect3DDevice9 &device, const D3DXMATRIX &tm, bool forceTextureTm, bool terrain)
{
	update_values = true;

	D3DXMATRIX result;
	D3DXMatrixMultiply(&result, &tm, &view_projection_tm);
	D3DXMatrixTranspose(&result, &result);

	// ViewProj matrix
	device.SetVertexShaderConstantF(0, result, 4);

	if(projected_shaders || ati_shadow_shaders || fake_shadow_shaders || fake_depth_shaders || forceTextureTm)
	{
		D3DXMATRIX foo = tm;
		D3DXMatrixTranspose(&foo, &foo);
		D3DXMatrixMultiply(&foo, &texture_matrix, &foo);
		device.SetVertexShaderConstantF(4, foo, 4);

		foo = tm;
		D3DXMatrixTranspose(&foo, &foo);

		if(!fake_depth_shaders)
			device.SetVertexShaderConstantF(8, foo, 3);
	}

	if(lighting_shaders)
	{
		device.SetVertexShaderConstantF(9, light_position[0], 1);
		device.SetVertexShaderConstantF(10, light_color[0], 1);

		D3DXVECTOR4 sun_temp = sun_properties;
		sun_temp.w = textureOffset.x;
		device.SetVertexShaderConstantF(11, sun_temp, 1);

		if(LIGHT_MAX_AMOUNT > 1)
		{
			D3DXVECTOR4 light_position2_temp = light_position[1];
			light_position2_temp.w = textureOffset.y;
			device.SetVertexShaderConstantF(12, light_position2_temp, 1);
			device.SetVertexShaderConstantF(13, light_color[1], 1);
		}
		else
		{
			D3DXVECTOR4 position_temp;
			D3DXVECTOR4 color_temp;
			position_temp.w = textureOffset.y;
			device.SetVertexShaderConstantF(12, position_temp, 1);
			device.SetVertexShaderConstantF(13, color_temp, 1);
		}

		if(terrain)
		{
			device.SetVertexShaderConstantF(25, fog, 1);
		}
		else
		{
			device.SetVertexShaderConstantF(18, view_position, 1);
			device.SetVertexShaderConstantF(19, fog, 1);
		}

		if(local_reflection)
		{
			D3DXMATRIX reflection_matrix(	0.5f,	0.0f,	0.0f,	0.5f,
											0.0f,	-0.5f,	0.0f,	0.5f,
											0.0f,	0.0f,	0.0f,	0.0f,
											0.0f,   0.0f,	0.0f,	1.0f);

			device.SetVertexShaderConstantF(27, reflection_matrix, 4);
		}

		if(!terrain)
		{
			BOOST_STATIC_ASSERT(LIGHT_MAX_AMOUNT >= 2 && LIGHT_MAX_AMOUNT <= 5);
			for(int i = 2; i < LIGHT_MAX_AMOUNT; ++i)
			{
				device.SetVertexShaderConstantF(21 + ((i - 2) * 2), light_position[i], 1);
				device.SetVertexShaderConstantF(22 + ((i - 2) * 2), light_color[i], 1);
			}
		}
	}	
	else
	{
		if(spot_type == Point || fake_shadow_shaders)
			device.SetVertexShaderConstantF(11, spot_position, 1);
		else
			device.SetVertexShaderConstantF(11, spot_properties, 1);

		if(!terrain)
			device.SetVertexShaderConstantF(19, fog, 1);
	}

	//if(projected_shaders || ati_shadow_shaders || fake_depth_shaders)
	if(projected_shaders || ati_shadow_shaders || fake_depth_shaders || fake_shadow_shaders)
	{
		D3DXMATRIX foo = tm;
		D3DXMatrixTranspose(&foo, &foo);
		D3DXMatrixMultiply(&foo, &target_matrix, &foo);
		device.SetVertexShaderConstantF(12, foo, 4);

		device.SetVertexShaderConstantF(16, textureOffset, 1);
	}

	if(!projected_shaders && !ati_depth_shaders && !ati_shadow_shaders && !fake_depth_shaders && !fake_shadow_shaders)
	{
		if(!lighting_shaders)
			device.SetVertexShaderConstantF(12, textureOffset, 1);
	}

	if(fake_depth_shaders)
	{
		device.SetVertexShaderConstantF(12, fake_properties, 1);
	}
}

void Storm3D_ShaderManager::ApplyForceAmbient(IDirect3DDevice9 &device)
{
	D3DXVECTOR4 v = ambient_force_color + ambient_color;
	device.SetVertexShaderConstantF(7, v, 1);
}
