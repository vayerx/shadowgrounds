// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <cassert>
#include "Storm3D_Datatypes.h"
#include <vector>
#include "storm3d_terrain_utils.h"

//------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------
class Storm3D_Model_Object;
class Storm3D_Model;

template <class T>
class Singleton
{
	static T *instance;

public:
	Singleton() 
	{
		assert(instance == 0);

		// Some pointer magic from 'Gems
		//	-> Cast dump pointer to both types and store relative address
		intptr_t pointer_super = (intptr_t) (T*) 1;
		intptr_t pointer_derived = (intptr_t) (Singleton<T>*)(T*) 1;
		intptr_t offset = pointer_super - pointer_derived;
		
		// Use offset to get our instance address
		instance = (T*) (intptr_t)(this) + offset;
	}
	
	~Singleton()
	{
		assert(instance);
		instance = 0;
	}

	static T* GetSingleton()
	{
		assert(instance);
		return instance;
	}
};

// Initialize static instance to 0
template<class T> T* Singleton<T>::instance = 0;


//------------------------------------------------------------------
// Storm3D_ShaderManager
//	-> Keeps track of related properties (lightning, ...)
//	-> Manages shaders with requested properties
//	-> 
//------------------------------------------------------------------
class Storm3D_ShaderManager: public Singleton<Storm3D_ShaderManager>
{
 public:
	// HACK:
	COL fogColor;
 private:
	// View matrix
	D3DXMATRIX view_projection_tm;
	D3DXMATRIX view_tm;
	D3DXMATRIX texture_matrix;
	D3DXMATRIX projection_tm;
	D3DXMATRIX world_tm;

	VC4 view_position;

	// Lighting
	VC4 ambient_color;
	VC4 ambient_force_color;
	VC4 fog;
	VC4 textureOffset;
	VC4 fake_properties;

	VC4 light_position[LIGHT_MAX_AMOUNT];
	VC4 light_color[LIGHT_MAX_AMOUNT];

	VC4 sun_properties;
	VC4 spot_position;
	VC4 spot_properties;
	VC4 spot_color;
	D3DXMATRIX target_matrix;

	VC4 model_ambient_color;	
	VC4 object_ambient_color;	
	VC4 object_diffuse_color;
	VC4 lightmap_factor;

	// For lazy updating
	bool update_values;

	// Shader id's
	frozenbyte::storm::VertexShader default_shader;
	//frozenbyte::storm::VertexShader lighting_shader;
	frozenbyte::storm::VertexShader lighting_shader_0light_noreflection;
	frozenbyte::storm::VertexShader lighting_shader_0light_localreflection;
	frozenbyte::storm::VertexShader lighting_shader_0light_reflection;
	frozenbyte::storm::VertexShader lighting_shader_1light_noreflection;
	frozenbyte::storm::VertexShader lighting_shader_1light_localreflection;
	frozenbyte::storm::VertexShader lighting_shader_1light_reflection;
	frozenbyte::storm::VertexShader lighting_shader_2light_noreflection;
	frozenbyte::storm::VertexShader lighting_shader_2light_localreflection;
	frozenbyte::storm::VertexShader lighting_shader_2light_reflection;
	frozenbyte::storm::VertexShader lighting_shader_3light_noreflection;
	frozenbyte::storm::VertexShader lighting_shader_3light_localreflection;
	frozenbyte::storm::VertexShader lighting_shader_3light_reflection;
	frozenbyte::storm::VertexShader lighting_shader_4light_noreflection;
	frozenbyte::storm::VertexShader lighting_shader_4light_localreflection;
	frozenbyte::storm::VertexShader lighting_shader_4light_reflection;
	frozenbyte::storm::VertexShader lighting_shader_5light_noreflection;
	frozenbyte::storm::VertexShader lighting_shader_5light_localreflection;
	frozenbyte::storm::VertexShader lighting_shader_5light_reflection;

	frozenbyte::storm::VertexShader skybox_shader;
	frozenbyte::storm::VertexShader default_projected_shader_directional;
	frozenbyte::storm::VertexShader default_projected_shader_point;
	frozenbyte::storm::VertexShader default_projected_shader_flat;
	frozenbyte::storm::VertexShader bone_shader;
	//frozenbyte::storm::VertexShader bone_lighting_shader;
	frozenbyte::storm::VertexShader bone_lighting_shader_0light_noreflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_0light_reflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_1light_noreflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_1light_reflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_2light_noreflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_2light_reflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_3light_noreflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_3light_reflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_4light_noreflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_4light_reflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_5light_noreflection;
	frozenbyte::storm::VertexShader bone_lighting_shader_5light_reflection;

	frozenbyte::storm::VertexShader bone_projected_shader_directional;
	frozenbyte::storm::VertexShader bone_projected_shader_point;
	frozenbyte::storm::VertexShader bone_projected_shader_flat;

	frozenbyte::storm::VertexShader fake_depth_shader;
	frozenbyte::storm::VertexShader fake_shadow_shader;	
	frozenbyte::storm::VertexShader fake_depth_bone_shader;
	frozenbyte::storm::VertexShader fake_shadow_bone_shader;	

	int current_shader;

	bool lighting_shaders;
	bool projected_shaders;
	bool fake_depth_shaders;
	bool fake_shadow_shaders;

	// Stored model pointer (on bone meshes)
	Storm3D_Model *model;

	VC2I targetPos;
	VC2I targetSize;

	float transparency_factor;

	bool reflection;
	bool local_reflection;
	int light_count;
	bool light_params_changed;

	void updatematrices();
public:
	Storm3D_ShaderManager();
	~Storm3D_ShaderManager();
	
	// Create shaders
	void CreateShaders();

	void setLightingParameters(bool reflection_, bool local_reflection_, int light_count_);

	// Set properties
	void SetViewMatrix(const D3DXMATRIX &view);
	void SetProjectionMatrix(const D3DXMATRIX &proj);
	void SetViewProjectionMatrix(const D3DXMATRIX &proj, const D3DXMATRIX &view);
	void SetTransparencyFactor(float factor);
	void SetViewPosition(const VC4 &p);
	void SetAmbient(const Color &color);
	void SetForceAmbient(const Color &color);
	void SetLight(int index, const Vector &position, const Color &color, float range);
	void SetSun(const VC3 &direction, float strength);
	void SetFog(float start, float range, const COL &color);
	void SetTextureOffset(const VC2 &offset);
	void setFakeProperties(float plane, float factor, float add);

	void SetModelAmbient(const Color &color);
	void SetObjectAmbient(const Color &color);
	void SetObjectDiffuse(const Color &color);

	void setProjectedShaders();
	void setFakeDepthShaders();
	void setFakeShadowShaders();
	void setLightingShaders();
	void setNormalShaders();

	void setTextureTm(D3DXMATRIX &matrix);
	void setSpot(const COL &color, const VC3 &position, const VC3 &direction, float range, float fadeFactor);
	void setSpotTarget(const D3DXMATRIX &matrix);

	enum SpotType
	{
		Flat = 0,
		Point = 1,
		Directional = 2
	};

	void setSpotType(SpotType type);

	bool BoneShader();

	// Do the magic ;-)
	void SetShader(Storm3D_Model_Object *object);
	void SetShader(const std::vector<int> &bone_indices); // not including first identity
	void ResetShader();
	void ClearCache();
	void BackgroundShader();

	void SetShaderDefaultValues();
	void SetShaderAmbient(const COL &color);
	void SetShaderDiffuse(const COL &color);
	void SetLightmapFactor(float xf, float yf);

	void ApplyDeclaration();
	void ApplyForceAmbient();
	void SetWorldTransform(const D3DXMATRIX &tm, bool forceTextureTm = false, bool terrain = false);

	// Public constants
	static const int BONE_INDEX_START; // First is identity
	static const int BONE_INDICES;

private:
	SpotType spot_type;
};
