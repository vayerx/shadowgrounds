// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "Storm3D_Datatypes.h"

// Common datatype includes
#include "DatatypeDef.h"
//#include "c2_qtree.h"
#include "c2_qtree_static.h"

//------------------------------------------------------------------
// CollisionFace
//------------------------------------------------------------------
struct CollisionFace;

//------------------------------------------------------------------
// Storm3D_Mesh_CollisionTable
//------------------------------------------------------------------
class Storm3D_Mesh_CollisionTable
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	Storm3D_Mesh *owner;
	CollisionFace *faces;
	int face_amount;
	
	//Storm3D_Face *tfaces;
	//Quadtree<CollisionFace> *tree;
	StaticQuadtree<CollisionFace> *tree;
public:

	// Rebuild table (fill with mesh's faces)
	void ReBuild(Storm3D_Mesh *mesh);

	// Test collision (these return true if collision happened)
	bool RayTrace(const VC3 &position, const VC3 &direction_normalized, float ray_length, Storm3D_CollisionInfo &rti, bool accurate = true);
	bool SphereCollision(const VC3 &position, float radius, Storm3D_CollisionInfo &rti, bool accurate = true);

	Storm3D_Mesh_CollisionTable();
	~Storm3D_Mesh_CollisionTable();

	void reset();

	friend class Storm3D_Scene;
};



