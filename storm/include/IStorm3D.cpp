/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: IStorm3D

  Storm3D interface creation
*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"



//------------------------------------------------------------------
// IStorm3D::Create_Storm3D_Interface
//------------------------------------------------------------------
IStorm3D *IStorm3D::Create_Storm3D_Interface(bool no_info)
{
	return new Storm3D(no_info);
}


