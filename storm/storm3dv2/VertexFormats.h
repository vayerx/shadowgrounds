// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


// Basic - 0 texcoords
#define FVF_VXFORMAT_TC0 (D3DFVF_XYZ|D3DFVF_NORMAL)
struct VXFORMAT_TC0
{
    VC3 position;
    VC3 normal;

	// Constructor
	VXFORMAT_TC0(VC3 _position,VC3 _normal) :
		position(_position),normal(_normal)
	{}

	VXFORMAT_TC0() {}
};



// Basic - 1 texcoords
#define FVF_VXFORMAT_TC1 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
struct VXFORMAT_TC1
{
	VC3 position;
	VC3 normal;
	VC2 texcoords;

	// Constructor
	VXFORMAT_TC1(VC3 _position,VC3 _normal,VC2 _texcoords) :
		position(_position),normal(_normal),texcoords(_texcoords)
	{}

	VXFORMAT_TC1() {}
};



// Basic - 2 texcoords
#define FVF_VXFORMAT_TC2 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2)
struct VXFORMAT_TC2
{
    VC3 position;
    VC3 normal;
	VC2 texcoords;
	VC2 texcoords2;

	// Constructor
	VXFORMAT_TC2(VC3 _position,VC3 _normal,VC2 _texcoords,VC2 _texcoords2) :
		position(_position),normal(_normal),texcoords(_texcoords),texcoords2(_texcoords2)
	{}

	VXFORMAT_TC2() {}
};



// Dot3 - 1 texcoords (no normals, lightvector(diffuse))
#define FVF_VXFORMAT_DOT3_TC1 (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct VXFORMAT_DOT3_TC1
{
    VC3 position;
	DWORD lightvector;	// Light direction vector
	VC2 texcoords;

	// Constructor
	VXFORMAT_DOT3_TC1(VC3 _position,DWORD _lightvector,VC2 _texcoords) :
		position(_position),lightvector(_lightvector),texcoords(_texcoords)
	{}

	VXFORMAT_DOT3_TC1() {}
};



// Dot3 - 2 texcoords (no normals, lightvector(diffuse))
/*#define FVF_VXFORMAT_DOT3_TC2 (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX2)
struct VXFORMAT_DOT3_TC2
{
    VC3 position;
	DWORD lightvector;	// Light direction vector
	VC2 texcoords;
	VC2 texcoords2;

	// Constructor
	VXFORMAT_DOT3_TC2(VC3 _position,DWORD _lightvector,
		VC2 _texcoords,VC2 _texcoords2) :
		position(_position),lightvector(_lightvector),
		texcoords(_texcoords),texcoords2(_texcoords2)
	{}
};*/


// "2d" particle/sprite (manually lighted)
#define FVF_VXFORMAT_2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct VXFORMAT_2D
{
    VC3 position;
	float rhw;
	DWORD color;	// 32 bit color
	VC2 texcoords;

	// Constructor
	VXFORMAT_2D(VC3 _position,float _rhw,DWORD _color,VC2 _texcoords) :
		position(_position),rhw(_rhw),color(_color),texcoords(_texcoords)
	{}

	VXFORMAT_2D() {}
};


// Particle [v2.3] (manually lighted)
#define FVF_VXFORMAT_PART (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX2)
struct VXFORMAT_PART
{
	VC3 position;
	DWORD color;	// 32 bit color
	VC2 texcoords;

	float pad[2];

	// Constructor
	VXFORMAT_PART(VC3 _position,DWORD _color,VC2 _texcoords) :
		position(_position),color(_color),texcoords(_texcoords)
	{}

	VXFORMAT_PART() {}
};



// Position only (for shadow volumes etc. no lighting or textures)
#define FVF_VXFORMAT_POSONLY (D3DFVF_XYZ)
struct VXFORMAT_POSONLY
{
    VC3 position;

	// Constructor
	VXFORMAT_POSONLY(VC3 _position) :
		position(_position)
	{}

	VXFORMAT_POSONLY() {}
};



// Position only 2D (for shadow volumes etc. no lighting or textures)
#define FVF_VXFORMAT_POSCOLONLY_2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
struct VXFORMAT_POSCOLONLY_2D
{
    VC3 position;
	float rhw;
	DWORD color;	// 32 bit color

	// Constructor
	VXFORMAT_POSCOLONLY_2D(VC3 _position,float _rhw,DWORD _color) :
		position(_position),rhw(_rhw),color(_color)
	{}

	VXFORMAT_POSCOLONLY_2D() {}
};



// Terrain (3d, no lighting, 1 texture)
#define FVF_VXFORMAT_TERRAIN (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX2)
struct VXFORMAT_TERRAIN		// Only colored detail texture (no basetexture)
{
	VC3 position;
	DWORD color;			// 32 bit color
	VC2 texcoords1;			// Texturecoordinates for 2nd detail texture
	VC2 texcoords2;			// Texturecoordinates for detail texture

	// Constructor
	VXFORMAT_TERRAIN(VC3 _position,DWORD _color,VC2 _texcoords1,VC2 _texcoords2) :
		position(_position),color(_color),texcoords1(_texcoords1),
		texcoords2(_texcoords2)
	{}

	VXFORMAT_TERRAIN() {}
};
/*#define FVF_VXFORMAT_TERRAIN (D3DFVF_XYZ|D3DFVF_TEX2)
struct VXFORMAT_TERRAIN
{
	VC3 position;
	VC2 texcoords;
	VC2 texcoords_detail;	// Texturecoordinates for detail texture

	// Constructor
	VXFORMAT_TERRAIN(VC3 _position,VC2 _texcoords,VC2 _texcoords_detail) :
		position(_position),texcoords(_texcoords),
		texcoords_detail(_texcoords_detail)
	{}

	VXFORMAT_TERRAIN() {}
};*/



// Terrain textureblend (2D, no lighting, texture)
/*#define FVF_VXFORMAT_BLEND (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct VXFORMAT_BLEND
{
	VC3 position;
	float rhw;
	DWORD color;	// 32 bit color
	VC2 texcoords;

	// Constructor
	VXFORMAT_BLEND(VC3 _position,float _rhw,DWORD _color,VC2 _texcoords) :
		position(_position),rhw(_rhw),color(_color),texcoords(_texcoords)
	{}

	VXFORMAT_BLEND() {}
};*/


// Terrainobject (manually lighted)
#define FVF_VXFORMAT_TOBJ (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct VXFORMAT_TOBJ
{
    VC3 position;
	DWORD color;
	VC2 texcoords;

	// Constructor
	VXFORMAT_TOBJ(VC3 _position,DWORD _color,VC2 _texcoords) :
		position(_position),color(_color),texcoords(_texcoords)
	{}

	VXFORMAT_TOBJ() {}
};

// psd .. for bone drawing
#define FVF_VXFORMAT_PSD (D3DFVF_XYZ|D3DFVF_DIFFUSE)
struct VXFORMAT_PSD
{
    Vector position;
    DWORD color;

	// Constructor
	VXFORMAT_PSD(Vector _position,DWORD _color) :
		position(_position),color(_color)
	{}
	VXFORMAT_PSD() :
		position(0,0,0), color(0)
	{}
};

// For blending
//#define FVF_VXFORMAT_BLEND (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_XYZB4)
#define FVF_VXFORMAT_BLEND (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX4)
struct VXFORMAT_BLEND
{
	VC3 position;
	VC3 normal;
	VC2 texcoords;
	VC2 texcoords2;

	// Bone 1 index, bone 1 weight, bone 2 index, bone 2 weight
	float blend[4]; 

	// Constructor
	VXFORMAT_BLEND(VC3 _position,VC3 _normal,VC2 _texcoords, VC2 _texcoords2, float *blend_) :
		position(_position),normal(_normal),texcoords(_texcoords), texcoords2(_texcoords2)
	{
		blend[0] = blend_[0];
		blend[1] = blend_[1];
		blend[2] = blend_[2];
		blend[3] = blend_[3];
	}

	//VXFORMAT_BLEND() {}
};
