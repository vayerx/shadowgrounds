/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Light

  Light (base class)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_Light::Storm3D_Light
//------------------------------------------------------------------
Storm3D_Light::Storm3D_Light(char *_name,Storm3D_Model *_parent_model,
							 COL &_color,Storm3D_Light::LTYPE _light_type) :
	update_globals(true),
	parent_model(_parent_model),
	parent_object(NULL),
	color(_color),
	light_type(_light_type)
{
	// Create name
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}


	
//------------------------------------------------------------------
// Storm3D_Light::~Storm3D_Light
//------------------------------------------------------------------
Storm3D_Light::~Storm3D_Light()
{
	// Delete name
	delete[] name;
}



//------------------------------------------------------------------
// Storm3D_Light::Apply
//------------------------------------------------------------------
void Storm3D_Light::Apply(int num)
{
}



//------------------------------------------------------------------
// Storm3D_Light::GetLightType
//------------------------------------------------------------------
Storm3D_Light::LTYPE Storm3D_Light::GetLightType()
{
	return light_type;
}



//------------------------------------------------------------------
// Storm3D_Light::SetColor
//------------------------------------------------------------------
void Storm3D_Light::SetColor(COL &_color)
{
	color=_color;
}



//------------------------------------------------------------------
// Storm3D_Light::SetUpdateGlobals
//------------------------------------------------------------------
void Storm3D_Light::SetUpdateGlobals()
{
	update_globals=true;
}


