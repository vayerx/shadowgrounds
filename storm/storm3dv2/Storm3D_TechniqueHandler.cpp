/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_TechniqueHandler

  Technique handler

  Handles texturing techniques. (basic textures, (cube-)reflections, bumpmaps etc...)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>
#include "Techniques.h"



//------------------------------------------------------------------
// Storm3D_TechniqueHandler::Storm3D_TechniqueHandler
//------------------------------------------------------------------
Storm3D_TechniqueHandler::Storm3D_TechniqueHandler() :
	coloronly("1color.sha"),
	tex("tex.sha"),
	dualtex("dualtex.sha"),
	ref("ref.sha")
/*	ref(NULL),
	texref(NULL),
	bumpref(NULL),
	dualtexref(NULL),
	bump(NULL),
	texbump(NULL),
	texbumpref(NULL),
	dualtexbump(NULL),
	dualtexbumpref(NULL)*/
{
}



//------------------------------------------------------------------
// Storm3D_TecniqueHandler::~Storm3D_TecniqueHandler
//------------------------------------------------------------------
Storm3D_TechniqueHandler::~Storm3D_TechniqueHandler()
{
}


