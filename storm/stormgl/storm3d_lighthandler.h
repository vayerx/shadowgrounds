/*

  Storm3D v3 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"



//------------------------------------------------------------------
// Storm3D_LightHandler
//------------------------------------------------------------------
class Storm3D_LightHandler
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	int active_lights;

public:

	void SetActiveLightAmount(int light_amount);
	Storm3D_LightHandler(Storm3D *Storm3D2);
};



