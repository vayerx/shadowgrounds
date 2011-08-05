/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  Class: Storm3D_SystemSurface

  System memory surface and software rendering

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <windows.h>

// Common datatype includes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Datatypes.h"



//------------------------------------------------------------------
// Storm3D_SystemSurface
//------------------------------------------------------------------
class Storm3D_SystemSurface
{
	BYTE *data;
	int size,sizesq;
	int shift;

public:

	void Clear();
	void Fix();	// Fills clipping holes...
	void RenderFaceList(Storm3D_Scene *scene,D3DMATRIX worldmx,Storm3D_Face *faces,Storm3D_Vertex *vertexes,int num_faces,int num_vertexes);

	BYTE *GetDataStart();

	Storm3D_SystemSurface(int size);	// size must be: 16 - 2048
	~Storm3D_SystemSurface();
};


