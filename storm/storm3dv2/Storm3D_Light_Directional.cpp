/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Light_Directional

  Directional light

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_light.h"
#include "storm3d_model.h"
#include "storm3d_model_object.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Light_Directional::Storm3D_Light_Directional
//------------------------------------------------------------------
Storm3D_Light_Directional::Storm3D_Light_Directional(Storm3D *s2,const char *name,
	Storm3D_Model *_parent_model,COL &_color,float multiplier,VC3 &_direction) :
	Storm3D_Light_AInterface(s2,name,_parent_model,_color,multiplier,0),
	direction(_direction),animation(this)
{
}


	
//------------------------------------------------------------------
// Storm3D_Light_Directional::~Storm3D_Light_Directional
//------------------------------------------------------------------
Storm3D_Light_Directional::~Storm3D_Light_Directional()
{
	SAFE_DELETE_ARRAY(name);
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::Apply
//------------------------------------------------------------------
void Storm3D_Light_Directional::Apply(int num)
{
	// Temp handle
	D3DLIGHT8 lgt;
	ZeroMemory(&lgt,sizeof(D3DLIGHT8));

	// Set colors
	lgt.Diffuse.r=color.r*multiplier;
	lgt.Diffuse.g=color.g*multiplier;
	lgt.Diffuse.b=color.b*multiplier;
	lgt.Specular.r=color.r*multiplier;
	lgt.Specular.g=color.g*multiplier;
	lgt.Specular.b=color.b*multiplier;
	lgt.Ambient.r=0;
	lgt.Ambient.g=0;
	lgt.Ambient.b=0;

	// Update global values if needed
	UpdateGlobals();

	// Directional light specials
	lgt.Type=D3DLIGHT_DIRECTIONAL;
	lgt.Direction=D3DXVECTOR3(direction_global.x,
		direction_global.y,direction_global.z);

	// Set light (to 3d-card)
	Storm3D2->GetD3DDevice()->SetLight(num,&lgt);
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::SetColor
//------------------------------------------------------------------
void Storm3D_Light_Directional::SetColor(COL &_color)
{
	color=_color;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetColor
//------------------------------------------------------------------
COL &Storm3D_Light_Directional::GetColor()
{
	return color;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::SetMultiplier
//------------------------------------------------------------------
void Storm3D_Light_Directional::SetMultiplier(float _multiplier)
{
	multiplier=_multiplier;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetMultiplier
//------------------------------------------------------------------
float Storm3D_Light_Directional::GetMultiplier()
{
	return multiplier;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::SetDecay
//------------------------------------------------------------------
void Storm3D_Light_Directional::SetDecay(float _decay)
{
	decay=_decay;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetDecay
//------------------------------------------------------------------
float Storm3D_Light_Directional::GetDecay()
{
	return decay;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::SetDirection
//------------------------------------------------------------------
void Storm3D_Light_Directional::SetDirection(VC3 &_direction)
{
	direction=_direction;
	update_globals=true;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetDirection
//------------------------------------------------------------------
VC3 &Storm3D_Light_Directional::GetDirection()
{
	return direction;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetGlobalDirection
//------------------------------------------------------------------
VC3 &Storm3D_Light_Directional::GetGlobalDirection()
{
	// Update global values if needed
	UpdateGlobals();
	return direction_global;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetLightType
//------------------------------------------------------------------
IStorm3D_Light::LTYPE Storm3D_Light_Directional::GetLightType()
{
	return LTYPE_DIRECTIONAL;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::SetName
//------------------------------------------------------------------
void Storm3D_Light_Directional::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}



//------------------------------------------------------------------
// Storm3D_Light_Directional::GetName
//------------------------------------------------------------------
const char *Storm3D_Light_Directional::GetName()
{
	return name;
}



//------------------------------------------------------------------
// Storm3D_Light_Directional - Keyframe animation
//------------------------------------------------------------------
void Storm3D_Light_Directional::Animation_Clear()
{
	animation.Clear();
}


void Storm3D_Light_Directional::Animation_AddNewDirectionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewDirectionKeyFrame(time,position);
}


void Storm3D_Light_Directional::Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color)
{
	animation.AddNewLuminanceKeyFrame(time,multiplier,decay,color);
}


void Storm3D_Light_Directional::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}


void Storm3D_Light_Directional::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}


void Storm3D_Light_Directional::UpdateGlobals()
{
	if (update_globals)
	{
		update_globals=false;
		direction_global=direction;
		if (parent_object)
		{
			parent_object->GetMXG().GetWithoutTranslation().TransformVector(direction_global);
		}
		else
		{
			parent_model->GetMX().GetWithoutTranslation().TransformVector(direction_global);
		}
	}
}


