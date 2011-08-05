// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "VertexFormats.h"

#include "../../util/Debug_MemoryManager.h"

//------------------------------------------------------------------
// Clip2DRectangle
// Returns false if rectancle is completely outside screen
//------------------------------------------------------------------
bool Clip2DRectangle(Storm3D *st,VXFORMAT_2D &ul,VXFORMAT_2D &dr)
{
	// Get Screen coordinates
	Storm3D_SurfaceInfo ss=st->GetScreenSize();
	int sxm=0;
	int sym=0;

	// Cull
	if (ul.position.x>dr.position.x) return false;
	if (ul.position.y>dr.position.y) return false;

	// Check if completely outside screen
	if (ul.position.x>(ss.width-1)) return false;
	if (ul.position.y>(ss.height-1)) return false;
	if (dr.position.x<sxm) return false;
	if (dr.position.y<sym) return false;

	// Test up-left X (ul)
	if (ul.position.x<sxm)
	{
		float tcp=(dr.texcoords.x-ul.texcoords.x)/(dr.position.x-ul.position.x);
		ul.texcoords.x += tcp*(sxm-ul.position.x);
		ul.position.x = float(sxm);
	}

	// Test up-left Y (ul)
	if (ul.position.y<sym)
	{
		float tcp=(dr.texcoords.y-ul.texcoords.y)/(dr.position.y-ul.position.y);
		ul.texcoords.y += tcp*(sym-ul.position.y);
		ul.position.y = float(sym);
	}

	// Test down-right X (dr)
	if (dr.position.x>(ss.width-1))
	{
		float tcp=(dr.texcoords.x-ul.texcoords.x)/(dr.position.x-ul.position.x);
		dr.texcoords.x += tcp*((ss.width-1)-dr.position.x);
		dr.position.x = float(ss.width-1);
	}

	// Test down-right Y (dr)
	if (dr.position.y>(ss.height-1))
	{
		float tcp=(dr.texcoords.y-ul.texcoords.y)/(dr.position.y-ul.position.y);
		dr.texcoords.y += tcp*((ss.height-1)-dr.position.y);
		dr.position.y = float(ss.height-1);
	}

	// Visible
	return true;
}
