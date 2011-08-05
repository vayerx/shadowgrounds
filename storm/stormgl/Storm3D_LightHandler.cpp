/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_LightHandler

  Light handler

  Turns on/off direct3d lights

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_lighthandler.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_LightHandler::Storm3D_LightHandler
//------------------------------------------------------------------
Storm3D_LightHandler::Storm3D_LightHandler(Storm3D *s2) :
	Storm3D2(s2),
	active_lights(-1)
{
}



//------------------------------------------------------------------
// Storm3D_LightHandler::Storm3D_LightHandler
//------------------------------------------------------------------
void Storm3D_LightHandler::SetActiveLightAmount(int light_amount)
{
	if (light_amount!=active_lights)
	{
		for (int i=0;i<8;i++) Storm3D2->GetD3DDevice()->LightEnable(i,(i<light_amount));
	}
	active_lights=light_amount;
}


