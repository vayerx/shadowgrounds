/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Clases: Storm3D_Helper, Storm3D_Helper_Point, Storm3D_Helper_Vector,
			Storm3D_Helper_Box

  Helpers
  

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_Helper::Storm3D_Helper
//------------------------------------------------------------------
Storm3D_Helper::Storm3D_Helper(char *_name,Storm3D_Model *_parent_model,
							 Storm3D_Helper::HTYPE _helper_type) :
	parent_model(_parent_model),
	parent_object(NULL),
	update_globals(true),
	helper_type(_helper_type)
{
	// Create name
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}


	
//------------------------------------------------------------------
// Storm3D_Helper::~Storm3D_Helper
//------------------------------------------------------------------
Storm3D_Helper::~Storm3D_Helper()
{
	// Delete name
	delete[] name;
}



//------------------------------------------------------------------
// Storm3D_Helper::GetHelperType
//------------------------------------------------------------------
Storm3D_Helper::HTYPE Storm3D_Helper::GetHelperType()
{
	return helper_type;
}



//------------------------------------------------------------------
// Storm3D_Helper::SetUpdateGlobals
//------------------------------------------------------------------
void Storm3D_Helper::SetUpdateGlobals()
{
	update_globals=true;
}



//------------------------------------------------------------------
// Storm3D_Helper_Point::Storm3D_Helper_Point
//------------------------------------------------------------------
Storm3D_Helper_Point::Storm3D_Helper_Point(char *name,Storm3D_Model *_parent_model,
		Storm3D_Vector _position) :
	Storm3D_Helper(name,_parent_model,HTYPE_POINT),
	position(_position)
{
}



//------------------------------------------------------------------
// Storm3D_Helper_Vector::Storm3D_Helper_Vector
//------------------------------------------------------------------
Storm3D_Helper_Vector::Storm3D_Helper_Vector(char *name,Storm3D_Model *_parent_model,
		Storm3D_Vector _position,Storm3D_Vector _direction) :
	Storm3D_Helper_Point(name,_parent_model,_position),
	direction(_direction)
{
	helper_type=HTYPE_VECTOR;
}



//------------------------------------------------------------------
// Storm3D_Helper_Box::Storm3D_Helper_Box
//------------------------------------------------------------------
Storm3D_Helper_Box::Storm3D_Helper_Box(char *name,Storm3D_Model *_parent_model,
		Storm3D_Vector _position,Storm3D_Vector _size) :
	Storm3D_Helper_Point(name,_parent_model,_position),
	size(_size)
{
	helper_type=HTYPE_BOX;
}



