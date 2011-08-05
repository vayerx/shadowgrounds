// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "Storm3D_Datatypes.h"



//------------------------------------------------------------------
// ClipPlane
//------------------------------------------------------------------
struct ClipPlane
{
	PLANE plane;
	bool active;

	ClipPlane() : active(false) {};
	ClipPlane(const PLANE &pl) : plane(pl),active(true) {};
};



//------------------------------------------------------------------
// LodFace
//------------------------------------------------------------------
struct LodFace
{
	int vertex_index[3];
	int adjacency[3];
	VC3 normal;

	// Creation
	LodFace(int v1,int v2,int v3,int a1,int a2,int a3,const VC3 &n) :
		normal(n)
	{
		vertex_index[0]=v1;
		vertex_index[1]=v2;
		vertex_index[2]=v3;
		adjacency[0]=a1;
		adjacency[1]=a2;
		adjacency[2]=a3;
	}
	LodFace(int v1,int v2,int v3)
	{
		vertex_index[0]=v1;
		vertex_index[1]=v2;
		vertex_index[2]=v3;
		adjacency[0]=-1;
		adjacency[1]=-1;
		adjacency[2]=-1;
	}
	LodFace()
	{
		adjacency[0]=-1;
		adjacency[1]=-1;
		adjacency[2]=-1;
	}
};



//------------------------------------------------------------------
// IntLiList
//------------------------------------------------------------------
struct IntLiList
{
	int data;
	IntLiList *next;

	// Creation
	IntLiList() : next(NULL) {}
	IntLiList(int _data) : data(_data),next(NULL) {}

	// Destructor (recursive)
	~IntLiList()
	{
		if (next) delete next;
	}
};



//------------------------------------------------------------------
// Mirror
//------------------------------------------------------------------
struct Mirror
{
	PLANE plane;
	Storm3D_Model_Object *object;

	Mirror() : object(NULL) {}
	Mirror(PLANE pla,Storm3D_Model_Object *obj) :
		plane(pla),object(obj) {}
};


