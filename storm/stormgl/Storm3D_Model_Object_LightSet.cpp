/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Model_Object_LightSet

  Lightset (part of object)


*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_Model_Object_LightSet::Storm3D_Model_Object_LightSet
//------------------------------------------------------------------
Storm3D_Model_Object_LightSet::Storm3D_Model_Object_LightSet(Storm3D_Model_Object *owner) :
	light_amount(0),
	full_update_needed(true)
{
}



//------------------------------------------------------------------
// Storm3D_Model_Object_LightSet::FullUpdate
//------------------------------------------------------------------
void Storm3D_Model_Object_LightSet::FullUpdate()
{
	// Clear the marker
	full_update_needed=false;

	// Give points to each light
	//int points[];
}



//------------------------------------------------------------------
// Storm3D_Model_Object_LightSet::Apply
//------------------------------------------------------------------
void Storm3D_Model_Object_LightSet::Apply()
{
	// BETA!
	full_update_needed=true;

	// Do we need to rebuild lightset (object was changed)
	if (full_update_needed) FullUpdate();

	// Apply all lights
	for (int ln=0;ln<light_amount;ln++) lights[ln]->Apply(ln);

	// Tell Storm3D to use correct number of lights
	Storm3D2.lighthandler.SetActiveLightAmount(light_amount);
}


