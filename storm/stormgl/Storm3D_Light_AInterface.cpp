/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  Class: Storm3D_Light_AInterface

  Light interface for Storm3D inside use.

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_light.h"
#include "../../util/Debug_MemoryManager.h"



//------------------------------------------------------------------
// Storm3D_Light_AInterface::Storm3D_Light_AInterface
//------------------------------------------------------------------
Storm3D_Light_AInterface::Storm3D_Light_AInterface(Storm3D *s2,const char *_name,
		Storm3D_Model *_parent_model,COL &_color,float _multiplier,float _decay) :
	Storm3D2(s2),
	color(_color),
	multiplier(_multiplier),
	decay(_decay),
	parent_model(_parent_model),
	parent_object(NULL),
	update_globals(true)
{
	// Create name
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}


	
//------------------------------------------------------------------
// Storm3D_Light_AInterface::~Storm3D_Light_AInterface
//------------------------------------------------------------------
Storm3D_Light_AInterface::~Storm3D_Light_AInterface()
{
	SAFE_DELETE_ARRAY(name);
}


