// Copyright 2002-2004 Frozenbyte Ltd.

#include "storm3d_material.h"
#include <IStorm3D_Helper.h>
#include <string>



/*
File structure
--------------
1. header
2. textures[]
3. materials[] (each material is followed by number of texturelayers)
	- texturelayers[] (for bump and reflection textures only)
4. objects[] (each object is followed by number of vertexes, LODs and animation keys)
    - vertexes[]
	[[[- LODs[] (each LOD is followed by number of faces) REMOVED!]]]
	- faces[]
	- poskeys[]
	- rotkeys[]
	- scalekeys[]
	- meshkeys[] (each meshkey is followed by number of vertexes)
		- vertexes[]
5. lights[] (each light is followed by number of animation keys)
	- lumkeys[]
	- poskeys[]
	- dirkeys[]
	- conekeys[]
6. helpers[] (each helper is followed by number of animation keys)
	- poskeys[]
	- o1keys[]
	- o2keys[]
7. bones[]

([] indicates, that it is an array.)
*/



struct S3D_HEADER
{
	char id[4];					// identifier ("S3D5")
	WORD num_textures;
	WORD num_materials;
	WORD num_objects;
	WORD num_lights;
	WORD num_helpers;	
	WORD num_bones;

	S3D_HEADER()
	:	num_textures(0),
		num_materials(0),
		num_objects(0),
		num_lights(0),
		num_helpers(0),
		num_bones(0)
	{
	}
};



struct S3D_TEXTURE
{
	std::string filename;
	DWORD identification;		// usually 0
	WORD start_frame;			// for videotextures
	WORD framechangetime;		// for videotextures
	BYTE dynamic;				// 0=not dynamic, 1=basic dynamic, 2=cube dynamic

	S3D_TEXTURE()
	:	identification(0),
		start_frame(0),
		framechangetime(0),
		dynamic(0)
	{
	}
};



struct S3D_MATERIAL
{
	std::string name;			// Name

	// Used textureleyers (texture indices, -1 if not used)
	// If Base2 or Reflection is used: material struct
	// is followed by texturelayer-structs (one for base2 and one for reflection)
	short int texture_base;
	short int texture_base2;
	short int texture_bump;
	short int texture_reflection;

	// COL and other lighting properties
	float color[3];
	float self_illum[3];
	float specular[3];
	float specular_sharpness;

	// Misc. properties
	bool doublesided;
	bool wireframe;

	// v2.5 new features
	IStorm3D_Material::TEX_GEN reflection_texgen;

	// Alphablending properties
	IStorm3D_Material::ATYPE alphablend_type;
	float transparency;		// %-transparency (0.0=opaque, 0.5=50% transparent, 1.0=invisible, etc...)
	float glow;

	S3D_MATERIAL()
	:	texture_base(0),
		texture_base2(0),
		texture_bump(0),
		texture_reflection(0),
		doublesided(false),
		wireframe(false),
		transparency(0),
		glow(0)
	{
	}
};



struct S3D_MATERIAL_TEXTURELAYER
{
	// Only for base2/reflection
	Storm3D_Material::MTL_BOP blend_op;
	float blend_factor;
};



struct S3D_OBJECT
{
	std::string name;			// Name
	std::string parent;		// Parent's name ("" if not)

	short int material_index;	// Object's material index (in this file, -1 if not used)
	
	float position[3];		// Position
	float rotation[4];		// QUAT (quaternion)
	float scale[3];			// VC3

	bool no_collision;		// No collision detection
	bool no_render;			// No rendering (this object is only for collisions)
	bool light_object;
	//bool is_volume_fog;		// This object is volume fog (color=material.color, density=material.transparency/1000)
	bool is_mirror;			// If this object is an mirror
	BYTE shadow_level;		// Realtime shadows (number of lightsources)

	int keyframe_endtime;	// Keyframe animation end time (loops at 0)

	BYTE lod_amount;		// number of following LOD's
	WORD poskey_amount;		// number of following POSITIONKEYs
	WORD rotkey_amount;		// number of following ROTATIONKEYs
	WORD scalekey_amount;	// number of following SCALEKEYs
	WORD meshkey_amount;	// number of following MESHKEYs

	WORD vertex_amount;		// number of following VERTEXes (new in v2.1)
	WORD face_amount;		// number of following FACEs (new in v3. NO LODS SAVED!)

	// Number of affecting bones. bone_amount * vertex_amount BONEWEIGHTS follows
	WORD bone_amount;

	S3D_OBJECT()
	:	material_index(0),
		no_collision(false),
		no_render(false),
		light_object(false),
		is_mirror(false),
		shadow_level(0),
		lod_amount(0),
		poskey_amount(0),
		rotkey_amount(0),
		scalekey_amount(0),
		meshkey_amount(0),
		bone_amount(0)
	{
	}
};



/*struct S3D_OBJECT_LOD
{
	// Removed vertexes in v2.1. Now LOD's contain only faces
	WORD face_amount;		// number of following FACEs
};*/



struct S3D_FACE
{
	WORD vertex[3];
	//short int material_index;	// Face's material index (in this file, -1 if not used)	
};



struct S3D_VERTEX
{	
	// psd: smoothing & texcoords 2 removed
	float position[3];
	float normal[3];
//	WORD smoothing_groups[2];	// for sharp edges (0=not grouped -> always smooth)
	float texturecoords[2];
	float texturecoords2[2];
};



struct S3D_V3KEY	// POSION, SCALE, DIRECTION, UPVECTOR
{
	int keytime;			// in millisecs	
	float x,y,z;
};



struct S3D_ROTKEY
{
	int keytime;			// in millisecs	
	float x,y,z,w;
};



struct S3D_MESHKEY
{
	int keytime;			// in millisecs	

	// ... Meshkey is always followed by a vertex-array (same size as objects original vertexarray)
};



struct S3D_LUMKEY			// For lights
{
	int keytime;			// in millisecs	
	float r,g,b;			// color settings
	float multiplier;		// multiplier setting
	float decay;			// decay setting
};



struct S3D_CONEKEY			// For spotlights
{
	int keytime;			// in millisecs	
	float inner,outer;		// cone settings
};




struct S3D_LIGHT
{
	std::string name;			// Name
	std::string parent;		// Parent's name ("" if not)

	//IStorm3D_Light::LTYPE light_type;		// Light type (point/spot/directional)
	int light_type;
	int lensflare_index;	// Light's lensflare index (in this file, -1 if not used)
	float color[3];
	float position[3];		// for points/spots
	float direction[3];		// for directionals/spots
	//float radius;			// for points/spots (REMOVED)
	float cone_inner;		// for spots
	float cone_outer;		// for spots

	// v2.5 new features
	float multiplier;
	float decay;

	int keyframe_endtime;	// Keyframe animation end time (loops at 0)

	WORD poskey_amount;		// number of following POSITION KEYs
	WORD dirkey_amount;		// number of following DIRECTION KEYs
	WORD lumkey_amount;		// number of following LIGHTLUM KEYs
	WORD conekey_amount;	// number of following LIGHTCONE KEYs

	S3D_LIGHT()
	:	poskey_amount(0),
		dirkey_amount(0),
		lumkey_amount(0),
		conekey_amount(0)
	{
	}

};



struct S3D_HELPER
{
	std::string name;			// Name
	std::string parent;		// Parent's name ("" if not)

	IStorm3D_Helper::HTYPE helper_type;		// Helper type (point/vector/box/camera)
	float position[3];
	float other[3];			// VC3 direction or box size (or sphere radius)
	float other2[3];		// Camerahelper's upvector

	int keyframe_endtime;	// Keyframe animation end time (loops at 0)

	WORD poskey_amount;		// number of following POSITIONKEYs
	WORD o1key_amount;		// number of following other KEYs	(direction, size, radius)
	WORD o2key_amount;		// number of following other2 KEYs	(upvector)

	S3D_HELPER()
	:	poskey_amount(0),
		o1key_amount(0),
		o2key_amount(0)
	{
	}
};


/* 
  Bone data 
*/

struct S3D_BONE 
{
	std::string name;

	float position[3]; // local (animations relative to this)
	float rotation[4]; // local (animations relative to this)

	float original_position[3]; // model space
	float original_rotation[4]; // model space

	// Not used
	float min_angles[3];
	float max_angles[3];

	int parent_index; // -1 for root bone
};
