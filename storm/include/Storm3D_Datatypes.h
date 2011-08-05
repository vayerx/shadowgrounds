/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001 

  Datatypes
	
	- Basic Storm3D datatypes
	
*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "DatatypeDef.h"
#include "Storm3D_Common.h"
#include <c2_collisioninfo.h>


//------------------------------------------------------------------
// Class Prototypes
//------------------------------------------------------------------
class Storm3D_Face;
class Storm3D_Vertex;
struct Storm3D_CollisionInfo;
class Storm3D_SurfaceInfo;

class IStorm3D_Model;
class IStorm3D_Model_Object;
class IStorm3D_Material;



//------------------------------------------------------------------
// Face
//------------------------------------------------------------------
class Storm3D_Face
{

public:

	// Data (public for speed)
	//int vertex_index[3];
	//VC3 normal;
	unsigned int vertex_index[3];

	// Creation
	ST3D_EXP_DLLAPI Storm3D_Face(int v1,int v2,int v3,const VC3 &_normal=VC3(0,1,0));
	ST3D_EXP_DLLAPI Storm3D_Face(const int _vertex_index[3],const VC3 &_normal=VC3(0,1,0));
	ST3D_EXP_DLLAPI Storm3D_Face();
};



//------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------
class Storm3D_Vertex
{

public:

	// Data (public for speed)
	VC3 position;
	VC3 normal;
	VC2 texturecoordinates;
	VC2 texturecoordinates2;

	// Creation
	ST3D_EXP_DLLAPI Storm3D_Vertex();
	ST3D_EXP_DLLAPI Storm3D_Vertex(const VC3 &_position,
		const VC3 &_normal,
		const VC2 &_texturecoordinates=VC2(0,0),
		const VC2 &_texturecoordinates2=VC2(0,0),
		const WORD _smoothing_groups[2]=0);
};


//------------------------------------------------------------------
// ObstacleCollisionInfo
//------------------------------------------------------------------

// count max 2 obstacles
#define MAX_OBSTACLE_COLLISIONS 2

class ObstacleCollisionInfo
{

public:

	bool hit;

  // amount of obstacles in ray path
  int hitAmount;

  // ranges of MAX_OBSTACLE_COLLISIONS first collisions.
  float ranges[MAX_OBSTACLE_COLLISIONS];
  VC3 plane_normals[MAX_OBSTACLE_COLLISIONS];
  VC3 positions[MAX_OBSTACLE_COLLISIONS];

	// Information about collision
	VC3 position;					// Nearest collision position
	float range;						// Nearest range of collision (from position)
	//VC3 plane_normal;				// Collided face's normal vector (in global coordinates)
	//IStorm3D_Model *model;				// Model that was collided
	//IStorm3D_Model_Object *object;		// Object that was collided

	ObstacleCollisionInfo() :
		hit(false),
		hitAmount(0),
		position(0.0f),
		range(static_cast<float>(HUGE))
	{};
};


//------------------------------------------------------------------
// Storm3D_SurfaceInfo
//------------------------------------------------------------------
class Storm3D_SurfaceInfo
{

public:

	// Data (public for speed)
	int width,height;
	int bpp;

	// Creation
	Storm3D_SurfaceInfo(int _width,int _height,int _bpp=0) : width(_width),
		height(_height), bpp(_bpp) {};
	Storm3D_SurfaceInfo() : width(0), height(0), bpp(0) {};
};


