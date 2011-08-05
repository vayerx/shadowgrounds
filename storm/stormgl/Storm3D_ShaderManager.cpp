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
#include <GL/glew.h>

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

//! Constructor
Storm3D_ShaderManager::Storm3D_ShaderManager()
:	ambient_color(.7f,.7f,.7f,0),
	ambient_force_color(0,0,0,0),
	fog(-20.f,1.f / 100.f,0.0f,1.f),

	object_ambient_color(0,0,0,0),
	object_diffuse_color(1.f,1.f,1.f,0),

	update_values(true),

	default_shader(),
	lighting_shader_0light_noreflection(),
	lighting_shader_0light_localreflection(),
	lighting_shader_0light_reflection(),
	lighting_shader_1light_noreflection(),
	lighting_shader_1light_localreflection(),
	lighting_shader_1light_reflection(),
	lighting_shader_2light_noreflection(),
	lighting_shader_2light_localreflection(),
	lighting_shader_2light_reflection(),
	lighting_shader_3light_noreflection(),
	lighting_shader_3light_localreflection(),
	lighting_shader_3light_reflection(),
	lighting_shader_4light_noreflection(),
	lighting_shader_4light_localreflection(),
	lighting_shader_4light_reflection(),
	lighting_shader_5light_noreflection(),
	lighting_shader_5light_localreflection(),
	lighting_shader_5light_reflection(),

	skybox_shader(),
	default_projected_shader_directional(),
	default_projected_shader_point(),
	default_projected_shader_flat(),
	bone_shader(),
	bone_lighting_shader_0light_noreflection(),
	bone_lighting_shader_0light_reflection(),
	bone_lighting_shader_1light_noreflection(),
	bone_lighting_shader_1light_reflection(),
	bone_lighting_shader_2light_noreflection(),
	bone_lighting_shader_2light_reflection(),
	bone_lighting_shader_3light_noreflection(),
	bone_lighting_shader_3light_reflection(),
	bone_lighting_shader_4light_noreflection(),
	bone_lighting_shader_4light_reflection(),
	bone_lighting_shader_5light_noreflection(),
	bone_lighting_shader_5light_reflection(),

	bone_projected_shader_directional(),
	bone_projected_shader_point(),
	bone_projected_shader_flat(),

	fake_depth_shader(),
	fake_shadow_shader(),
	fake_depth_bone_shader(),
	fake_shadow_bone_shader(),

	current_shader(0),
	projected_shaders(false),
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

	D3DXMatrixIdentity(reflection_matrix);
	reflection_matrix._22 = -1.f;
	reflection_matrix._42 = 2 * reflection_height;
}

//! Destructor
Storm3D_ShaderManager::~Storm3D_ShaderManager()
{
}

//! Create shaders
void Storm3D_ShaderManager::CreateShaders()
{
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
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}


//! Set lighting parameters
/*!
	\param reflection_ use reflection
	\param local_reflection_ use local reflection
	\param light_count_ number of lights
*/
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

//! Set transparency factor
/*!
	\param factor factor
*/
void Storm3D_ShaderManager::SetTransparencyFactor(float factor)
{
	transparency_factor = factor;
}

//! Set view matrix
/*!
	\param view view matrix
*/
void Storm3D_ShaderManager::SetViewMatrix(const D3DXMATRIX &view) {
	view_tm = view;
	updatematrices();
}

//! Set projection matrix
/*!
	\param proj projection matrix
*/
void Storm3D_ShaderManager::SetProjectionMatrix(const D3DXMATRIX &proj) {
	projection_tm = proj;
	updatematrices();
}

//! Set view projection matrix
/*!
	\param proj projection matrix (not viewProjection !!!)
	\param view view matrix
	this was killed. it might be necessary for local reflections though...
*/
void Storm3D_ShaderManager::SetViewProjectionMatrix(const D3DXMATRIX &proj, const D3DXMATRIX &view)
{
	//view_projection_tm = vp;
	D3DXMatrixMultiply(view_projection_tm, view, proj);

	D3DXMatrixIdentity(clip_matrix);
	D3DXMatrixIdentity(reflection_matrix);

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

		D3DXMatrixMultiply(view_projection_tm, reflection_matrix, view_projection_tm);

		// Oblique depth projection
		{
			// Wanted plane
			D3DXPLANE plane;
			VC3 point(0.f, reflection_height, 0.f);
			VC3 normal(0.f, 1.f, 0.f);
			D3DXPlaneFromPointNormal(plane, point, normal);
			VC4 clipPlane((float)plane.a, (float)plane.b, (float)plane.c, (float)plane.d);

			// Transform plane
			D3DXMATRIX normalizedViewProjection;
			D3DXMatrixInverse(normalizedViewProjection, 0, view_projection_tm);
			D3DXMatrixTranspose(normalizedViewProjection, normalizedViewProjection);

			VC4 projectedPlane;
			D3DXVec4Transform(projectedPlane, clipPlane, normalizedViewProjection);

			if(projectedPlane.w > 0)
			{
				VC4 tempPlane = -clipPlane;
				D3DXVec4Transform(projectedPlane, tempPlane, normalizedViewProjection);
			}

			// Create skew matrix
			D3DXMatrixIdentity(clip_matrix);
			clip_matrix(0, 2) = projectedPlane.x;
			clip_matrix(1, 2) = projectedPlane.y;
			clip_matrix(2, 2) = projectedPlane.z;
			clip_matrix(3, 2) = projectedPlane.w;
			projection_tm = projection_tm * clip_matrix;
			updatematrices();
		}
	}
}

//! Set view position
/*
	\param p position
*/
void Storm3D_ShaderManager::SetViewPosition(const VC4 &p)
{
	view_position = p;
}

//! Set ambient color
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetAmbient(const Color &color)
{
	ambient_color.x = color.r;
	ambient_color.y = color.g;
	ambient_color.z = color.b;

	object_ambient_color = ambient_color;
	update_values = true;
}

//! Set ambient force color
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetForceAmbient(const Color &color)
{
	ambient_force_color.x = color.r;
	ambient_force_color.y = color.g;
	ambient_force_color.z = color.b;
}

//! Set light properties
/*!
	\param index light index
	\param direction light direction vector
	\param color light color
	\param range light range
*/
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

//! Set sun properties
/*!
	\param direction sun light direction
	\param strength light strength
*/
void Storm3D_ShaderManager::SetSun(const Vector &direction, float strength)
{
	sun_properties.x = direction.x;
	sun_properties.y = direction.y;
	sun_properties.z = direction.z;
	sun_properties.w = strength;

	update_values = true;
}

//! Set fog properties
/*!
	\param start fog start
	\param range fog range
*/
void Storm3D_ShaderManager::SetFog(float start, float range, const COL &color)
{
	fog.x = start - range;
	fog.y = 1.f / range;
	fog.z = 0.f;
	fog.w = 1.f;
	fogColor = color;

	update_values = true;
}

//! Set texture offset
/*!
	\param offset offset
*/
void Storm3D_ShaderManager::SetTextureOffset(const VC2 &offset)
{
	textureOffset.x = offset.x;
	textureOffset.y = offset.y;
}

//! Set fake spotlight properties
/*!
	\param plane
	\param factor
	\param add
*/
void Storm3D_ShaderManager::setFakeProperties(float plane, float factor, float add)
{
	fake_properties.x = plane;
	fake_properties.y = factor;
	fake_properties.z = add;
}

//! Set model ambient color properties
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetModelAmbient(const Color &color)
{
	model_ambient_color.x = color.r;
	model_ambient_color.y = color.g;
	model_ambient_color.z = color.b;

	update_values = true;
}

//! Set object ambient color properties
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetObjectAmbient(const Color &color)
{
	object_ambient_color.x = color.r;
	object_ambient_color.y = color.g;
	object_ambient_color.z = color.b;

	update_values = true;
}

//! Set object diffuse color properties
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetObjectDiffuse(const Color &color)
{
	object_diffuse_color.x = color.r;
	object_diffuse_color.y = color.g;
	object_diffuse_color.z = color.b;

	update_values = true;
}

//! Set projected shaders
void Storm3D_ShaderManager::setProjectedShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = true;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

//! Set lighting shaders
void Storm3D_ShaderManager::setLightingShaders()
{
	lighting_shaders = true;
	projected_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

//! Set normal shaders
void Storm3D_ShaderManager::setNormalShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = false;
}

//! Set fake depth shaders
void Storm3D_ShaderManager::setFakeDepthShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	fake_depth_shaders = true;
	fake_shadow_shaders = false;
}

//! Set fake shadow shaders
void Storm3D_ShaderManager::setFakeShadowShaders()
{
	current_shader = 0;
	update_values = true;
	model = 0;

	lighting_shaders = false;
	projected_shaders = false;
	fake_depth_shaders = false;
	fake_shadow_shaders = true;
}

//! Set texture matrix to transpose
/*!
	\param matrix from world space -> light projection space
*/
void Storm3D_ShaderManager::setTextureTm(D3DXMATRIX &matrix)
{
	texture_matrix = matrix;
}

//! Set spot light properties
/*!
	\param color spot color
	\param position spot position
	\param direction spot direction
	\param range spot light range
	\param fadeFactor spot fade factor
*/
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

//! Set spot target matrix
/*!
	\param matrix matrix
*/
void Storm3D_ShaderManager::setSpotTarget(const D3DXMATRIX &matrix)
{
	target_matrix = matrix;
}

//! Set spot type
/*!
	\param type spot type
*/
void Storm3D_ShaderManager::setSpotType(SpotType type)
{
	spot_type = type;
}

//! Is shader a bone shader
/*!
	\return true if bone shader
*/
bool Storm3D_ShaderManager::BoneShader()
{
	switch(current_shader)
	{
		case BONE_SHADER:
		case BONE_LIGHTING_SHADER:
		case BONE_PROJECTED_SHADER_DIRECTIONAL:
		case BONE_PROJECTED_SHADER_POINT:
		case BONE_PROJECTED_SHADER_FLAT:
		case FAKE_DEPTH_BONE_SHADER:
		case FAKE_SHADOW_BONE_SHADER:
			return true;
	}

	return false;
}

//! Set shader for object
/*!
	\param object model object
*/
void Storm3D_ShaderManager::SetShader(Storm3D_Model_Object *object)
{
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

	D3DXMATRIX object_tm;
	object->GetMXG().GetAsD3DCompatible4x4((float *) &object_tm);
	SetWorldTransform(object_tm);

	IStorm3D_Material *m = object->GetMesh()->GetMaterial();
	float alpha = 1.f;

	float force_alpha = object->force_alpha;
	if(projected_shaders && object->force_lighting_alpha_enable)
		force_alpha = object->force_lighting_alpha;

	IStorm3D_Material::ATYPE a = m->GetAlphaType();
	if(a == IStorm3D_Material::ATYPE_USE_TRANSPARENCY)
		alpha = 1.f - m->GetTransparency() - force_alpha;
	else if(a == IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY || force_alpha > 0.0001f)
		alpha = 1.f - m->GetTransparency() - force_alpha;
	else if(a == IStorm3D_Material::ATYPE_USE_ALPHATEST)
		alpha = 1.f - m->GetTransparency();

	if(alpha < 0)
		alpha = 0;

	if(!projected_shaders && !fake_shadow_shaders && !fake_depth_shaders)
	{
		D3DXMatrixTranspose(object_tm, object_tm);
		for (int i = 0; i < 3; i++) {
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 4 + i, object_tm.raw + (4 * i));
		}

		{
			// Constants
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, object_ambient_color.GetAsFloat());
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, object_diffuse_color.GetAsFloat());
			update_values = false;
		}

		// Set transparency?
		float ambient_temp[] = { sun_properties.w * ambient_color.x, sun_properties.w * ambient_color.y, sun_properties.w * ambient_color.z, sun_properties.w * ambient_color.w };
		float ambient[] = { ambient_temp[0] + model_ambient_color.x + ambient_force_color.x, ambient_temp[1] + model_ambient_color.y + ambient_force_color.y,
			ambient_temp[2] + model_ambient_color.z + ambient_force_color.z, ambient_temp[3] + model_ambient_color.w + ambient_force_color.w };

		ambient[0] = max(ambient[0], object_ambient_color.x);
		ambient[1] = max(ambient[1], object_ambient_color.y);
		ambient[2] = max(ambient[2], object_ambient_color.z);

#ifdef HACKY_SG_AMBIENT_LIGHT_FIX
		// EVIL HAX around too dark characters etc.
		const float MIN_AMBIENT_LIGHT = 0.05f;
		ambient[0] = max(ambient[0], MIN_AMBIENT_LIGHT);
		ambient[1] = max(ambient[0], MIN_AMBIENT_LIGHT);
		ambient[2] = max(ambient[0], MIN_AMBIENT_LIGHT);

		ambient[0] = min(ambient[0], 1.0f);
		ambient[1] = min(ambient[0], 1.0f);
		ambient[2] = min(ambient[0], 1.0f);
#else
		if(ambient[0] > 1.f)
			ambient[0] = 1.f;
		if(ambient[1] > 1.f)
			ambient[1] = 1.f;
		if(ambient[2] > 1.f)
			ambient[2] = 1.f;
#endif

		ambient[3] = alpha;

		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, ambient);
	}

	if(projected_shaders)
	{
		float dif[4];
		dif[0] = object_diffuse_color.x * spot_color.x;
		dif[1] = object_diffuse_color.y * spot_color.y;
		dif[2] = object_diffuse_color.z * spot_color.z;

		dif[3] = alpha * transparency_factor;
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 17, dif);
	}

	if(fake_depth_shaders)
	{
		D3DXMatrixTranspose(object_tm, object_tm);
		for (int i = 0; i < 2; i++)
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 13 + i, object_tm.raw + (4 * i));
	}
}

//! Set shader
/*!
	\param bone_indices
*/
void Storm3D_ShaderManager::SetShader(const std::vector<int> &bone_indices)
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
			int shader_index = i;

			if(index >= bone_amount)
				continue;

			const MAT &vertexTm = model->bones[index]->GetVertexTransform();
			int arrayIndex = shader_index * 3 * 4;

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

		for (unsigned int i = 0; i < 3 * bone_indices.size(); i++) {
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, BONE_INDEX_START + i, array + (4 * i));
		}
	}
}

//! Reset shader
void Storm3D_ShaderManager::ResetShader()
{
	light_count = 1000000000;
	current_shader = 0;
	model = 0;
	update_values = true;
	projected_shaders = false;
	lighting_shaders = false;
	// ...
	fake_depth_shaders = false;
	fake_shadow_shaders = false;

	object_ambient_color = ambient_color;
	transparency_factor = 1.f;
}

//! Clear shader cache
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

//! Apply background shader
void Storm3D_ShaderManager::BackgroundShader()
{
	current_shader = 0;
	skybox_shader.apply();
}

//! Set shader default values
void Storm3D_ShaderManager::SetShaderDefaultValues()
{
	float foo[] = { 1, 1, 1, 1 };
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, foo);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, foo);
}

//! Set shader ambient color
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetShaderAmbient(const COL &color)
{
	float ambient[] = { color.r, color.g, color.b, 0.f };
	float diffuse[] = { 1, 1, 1, 0 };

	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, ambient);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, diffuse);
}

//! Set shader diffuse color
/*!
	\param color color
*/
void Storm3D_ShaderManager::SetShaderDiffuse(const COL &color)
{
	float diffuse[] = { color.r, color.g, color.b, 0.f };
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, diffuse);
}

//! Set lightmap factor
/*!
	\param xf
	\param yf
*/
void Storm3D_ShaderManager::SetLightmapFactor(float xf, float yf)
{
	lightmap_factor.x = xf;
	lightmap_factor.y = yf;
	lightmap_factor.z = 0;
	lightmap_factor.w = 0;
}

//! Apply declaration
void Storm3D_ShaderManager::ApplyDeclaration()
{
	default_shader.applyDeclaration();
}


void Storm3D_ShaderManager::updatematrices() {
	// MATRIX0_ARB = world matrix
	glMatrixMode(GL_MATRIX0_ARB);
	glLoadMatrixf(world_tm.raw);

	D3DXMATRIX result;
	D3DXMatrixMultiply(result, world_tm, view_tm);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(result.raw);

	//D3DXMatrixMultiply(view_projection_tm, result, projection_tm);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection_tm.raw);

	// ViewProj matrix
    // old and busted
	/*D3DXMatrixTranspose(result, view_projection_tm);
	for (int i = 0; i < 4; i++) {
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, i, ((const float *) result) + (4 * i));
	}*/

}

//! Set world transform
/*!
	\param tm matrix
	\param forceTextureTm
	\param terrain
*/
void Storm3D_ShaderManager::SetWorldTransform(const D3DXMATRIX &tm, bool forceTextureTm, bool terrain)
{
	update_values = true;

	world_tm = tm;

	updatematrices();

	if(projected_shaders || fake_shadow_shaders || fake_depth_shaders || forceTextureTm)
	{
		/*
		D3DXMATRIX foo = tm;
		D3DXMatrixTranspose(foo, foo);
		D3DXMatrixMultiply(foo, texture_matrix, foo);
		for (int i = 0; i < 3; i++)
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 4 + i, ((const float *) foo) + (4 * i));

		foo = tm;
		D3DXMatrixTranspose(foo, foo);

		if(!fake_depth_shaders)
		{
			for (int i = 0; i < 3; i++)
				glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8 + i, ((const float *) foo) + (4 * i));
		}
		*/

		// matrix1_arb is model -> shadowmap space
		D3DXMATRIX foo;
		D3DXMatrixMultiply(foo, tm, target_matrix);
		glMatrixMode(GL_MATRIX1_ARB);
		glLoadMatrixf(foo.raw);

		// matrix2_arb is model -> light projection space
		D3DXMatrixMultiply(foo, tm, texture_matrix);
		glMatrixMode(GL_MATRIX2_ARB);
		glLoadMatrixf(foo.raw);
	}

	if(lighting_shaders)
	{
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 9, light_position[0].GetAsFloat());
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 10, light_color[0].GetAsFloat());

		float sun_temp[] = { sun_properties.x, sun_properties.y, sun_properties.z, textureOffset.x };
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 11, sun_temp);

		if(LIGHT_MAX_AMOUNT > 1)
		{
			float light_position2_temp[] = { light_position[1].x, light_position[1].y, light_position[1].z, textureOffset.y };
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 12, light_position2_temp);
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 13, light_color[1].GetAsFloat());
		}
		else
		{
			float position_temp[] = { 0, 0, 0, textureOffset.y };
			float color_temp[] = { 0, 0, 0, 0 };
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 12, position_temp);
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 13, color_temp);
		}

		if(terrain)
		{
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 25, fog.GetAsFloat());
		}
		else
		{
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 18, view_position.GetAsFloat());
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 19, fog.GetAsFloat());
		}

		if(local_reflection)
		{
			D3DXMATRIX reflection_matrix(	0.5f,	0.0f,	0.0f,	0.5f,
											0.0f,	-0.5f,	0.0f,	0.5f,
											0.0f,	0.0f,	0.0f,	0.0f,
											0.0f,   0.0f,	0.0f,	1.0f);

			for (int i = 0; i < 4; i++) {
				glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 27 + i, reflection_matrix.raw + (4 * i));
			}
		}

		if(!terrain)
		{
			BOOST_STATIC_ASSERT(LIGHT_MAX_AMOUNT >= 2 && LIGHT_MAX_AMOUNT <= 5);
			for(int i = 2; i < LIGHT_MAX_AMOUNT; ++i)
			{
				glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 21 + ((i - 2) * 2), light_position[i].GetAsFloat());
				glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 22 + ((i - 2) * 2), light_color[i].GetAsFloat());
			}
		}
	}
	else
	{
		if(spot_type == Point || fake_shadow_shaders)
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 11, spot_position.GetAsFloat());
		else
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 11, spot_properties.GetAsFloat());

		if(!terrain)
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 19, fog.GetAsFloat());
	}

	if(projected_shaders || fake_depth_shaders || fake_shadow_shaders)
	{
		D3DXMATRIX foo = tm;
		D3DXMatrixTranspose(foo, foo);
		D3DXMatrixMultiply(foo, target_matrix, foo);
		for (int i = 0; i < 4; i++) {
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 12 + i, foo.raw + (4 * i));
		}

		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 16, textureOffset.GetAsFloat());
	}

	if(!projected_shaders && !fake_depth_shaders && !fake_shadow_shaders)
	{
		if(!lighting_shaders)
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 12, textureOffset.GetAsFloat());
	}

	if(fake_depth_shaders)
	{
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 12, fake_properties.GetAsFloat());
	}
}

//! Apply ambient force color
void Storm3D_ShaderManager::ApplyForceAmbient()
{
	float v[] = { ambient_force_color.x + ambient_color.x, ambient_force_color.y + ambient_color.y, ambient_force_color.z + ambient_color.z, ambient_force_color.w + ambient_color.w };
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, v);
}
