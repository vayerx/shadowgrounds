// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "Storm3D_Datatypes.h"
#include <memory.h>

#include "../../util/Debug_MemoryManager.h"

//------------------------------------------------------------------
// Storm3D_Face::Storm3D_Face
//------------------------------------------------------------------
Storm3D_Face::Storm3D_Face(int v1,int v2,int v3,const VC3 &_normal) 
{
	vertex_index[0]=v1;
	vertex_index[1]=v2;
	vertex_index[2]=v3;
}

//------------------------------------------------------------------
// Storm3D_Face::Storm3D_Face
//------------------------------------------------------------------
Storm3D_Face::Storm3D_Face(const int _vertex_index[3],const VC3 &_normal) 
{
	memcpy(vertex_index,_vertex_index,sizeof(int)*3);
}

//------------------------------------------------------------------
// Storm3D_Face::Storm3D_Face
//------------------------------------------------------------------
Storm3D_Face::Storm3D_Face()// : material(NULL)
{
}
