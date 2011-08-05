// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatype includes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Material.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Mesh;



//------------------------------------------------------------------
// IStorm3D_Mesh (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Mesh
{

public:
	enum { LOD_AMOUNT = 3 };

	// Get radius (recreates if needed)
	virtual float GetRadius()=0;
	virtual float GetSquareRadius()=0;

	// Material functions
	virtual void UseMaterial(IStorm3D_Material *material)=0;
	virtual IStorm3D_Material *GetMaterial()=0;	// Returns NULL if no material used

	// Clear (faces/vertices)
	virtual void DeleteAllFaces()=0;
	virtual void DeleteAllVertexes()=0;

	// Create a clone
	// -jpk
	virtual IStorm3D_Mesh *CreateNewClone()=0;

	// Dynamic geometry edit
	// How to use:
	// (1. Change face/vertex counts if needed)
	// 2. Get face/vertex count
	// 3. Get face/vertex buffers
	// 4. Edit buffers
	// (5. Update CollisionTable)
	virtual Storm3D_Face *GetFaceBuffer()=0;			// Causes Storm3D to rebuild videomem buffers
	virtual Storm3D_Vertex *GetVertexBuffer()=0;		// Causes Storm3D to rebuild videomem buffers
	virtual const Storm3D_Face *GetFaceBufferReadOnly()=0;		// No rebuild (read only)
	virtual const Storm3D_Vertex *GetVertexBufferReadOnly()=0;	// No rebuild (read only)
	virtual int GetFaceCount()=0;
	virtual int GetVertexCount()=0;
	virtual void ChangeFaceCount(int new_face_count)=0;		// Old faces are lost
	virtual void ChangeVertexCount(int new_vertex_count)=0;	// Old vertices are lost
	virtual void UpdateCollisionTable()=0;	// Use this after editing, if you want collisions to this object

	// Test collision (these return true if collided)
	virtual bool RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate = true)=0;
	virtual bool SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate = true)=0;

	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_Mesh() {};
};


