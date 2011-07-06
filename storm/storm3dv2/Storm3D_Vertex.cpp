// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_datatypes.h"
#include <memory.h>

#include "..\..\util\Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Vertex::Storm3D_Vertex
//------------------------------------------------------------------
Storm3D_Vertex::Storm3D_Vertex(const VC3 &_position,const VC3 &_normal,
		const VC2 &_texturecoordinates,const VC2 &_texturecoordinates2,
		const WORD smoothing_groups[2]) :
	position(_position),
	normal(_normal),
	texturecoordinates(_texturecoordinates),
	texturecoordinates2(_texturecoordinates2)
{
}



//------------------------------------------------------------------
// Storm3D_Vertex::Storm3D_Vertex
//------------------------------------------------------------------
Storm3D_Vertex::Storm3D_Vertex()
{
}


