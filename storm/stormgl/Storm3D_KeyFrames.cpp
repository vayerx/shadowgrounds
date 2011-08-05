// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_animation.h"
#include "storm3d_mesh.h"

#include "../../util/Debug_MemoryManager.h"


//! Constructor
Storm3D_KeyFrame_Vector::Storm3D_KeyFrame_Vector(int time,const VC3 &_position) :
	keytime(time),position(_position),next_key(NULL)
{
}

//! Destructor
Storm3D_KeyFrame_Vector::~Storm3D_KeyFrame_Vector()
{
	// Recursively delete "childs"
	SAFE_DELETE(next_key);
}

//! Constructor
Storm3D_KeyFrame_Rotation::Storm3D_KeyFrame_Rotation(int time,const QUAT &_rotation) :
	keytime(time),rotation(_rotation),next_key(NULL)
{
}

//! Destructor
Storm3D_KeyFrame_Rotation::~Storm3D_KeyFrame_Rotation()
{
	// Recursively delete "childs"
	SAFE_DELETE(next_key);
}

//! Constructor
Storm3D_KeyFrame_Scale::Storm3D_KeyFrame_Scale(int time,const VC3 &_scale) :
	keytime(time),scale(_scale),next_key(NULL)
{
}

//! Destructor
Storm3D_KeyFrame_Scale::~Storm3D_KeyFrame_Scale()
{
	// Recursively delete "childs"
	SAFE_DELETE(next_key);
}

//! Constructor
Storm3D_KeyFrame_Mesh::Storm3D_KeyFrame_Mesh(int time,Storm3D_Mesh *_owner,const Storm3D_Vertex *_vertexes) :
	owner(_owner),keytime(time),next_key(NULL)
{
	// Create/copy vertexes
	vertexes=new Storm3D_Vertex[owner->vertex_amount];
	memcpy(vertexes,_vertexes,sizeof(Storm3D_Vertex)*owner->vertex_amount);
}

//! Destructor
Storm3D_KeyFrame_Mesh::~Storm3D_KeyFrame_Mesh()
{
	// Delete vertexes
	delete[] vertexes;

	// Recursively delete "childs"
	SAFE_DELETE(next_key);
}

//! Constructor
Storm3D_KeyFrame_Luminance::Storm3D_KeyFrame_Luminance(int time,float _multiplier,float _decay,const COL &_color) :
	keytime(time),multiplier(_multiplier),decay(_decay),color(_color),next_key(NULL)
{
}

//! Destructor
Storm3D_KeyFrame_Luminance::~Storm3D_KeyFrame_Luminance()
{
	// Recursively delete "childs"
	SAFE_DELETE(next_key);
}

//! Constructor
Storm3D_KeyFrame_Cones::Storm3D_KeyFrame_Cones(int time,float _inner,float _outer) :
	keytime(time),inner(_inner),outer(_outer),next_key(NULL)
{
}

//! Destructor
Storm3D_KeyFrame_Cones::~Storm3D_KeyFrame_Cones()
{
	// Recursively delete "childs"
	SAFE_DELETE(next_key);
}
