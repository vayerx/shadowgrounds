/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Light_Point

  Point light

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_light.h"
#include "storm3d_lensflare.h"
#include "storm3d_model.h"
#include "storm3d_model_object.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Light_Point::Storm3D_Light_Point
//------------------------------------------------------------------
Storm3D_Light_Point::Storm3D_Light_Point(Storm3D *s2,const char *name,
			Storm3D_Model *_parent_model,COL &_color,float multiplier,float decay,
			VC3 &_position,Storm3D_LensFlare *_lflare) :
	Storm3D_Light_AInterface(s2,name,_parent_model,_color,multiplier,decay),
	position(_position),
	lflare(_lflare),
	flarevis(0),animation(this)
{
}


	
//------------------------------------------------------------------
// Storm3D_Light_Point::~Storm3D_Light_Point
//------------------------------------------------------------------
Storm3D_Light_Point::~Storm3D_Light_Point()
{
	SAFE_DELETE_ARRAY(name);
}



//------------------------------------------------------------------
// Storm3D_Light_Point::Apply
//------------------------------------------------------------------
void Storm3D_Light_Point::Apply(int num)
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
	lgt.Ambient.r=0;//color.r*0.1f*multiplier;
	lgt.Ambient.g=0;//color.g*0.1f*multiplier;
	lgt.Ambient.b=0;//color.b*0.1f*multiplier;

	// Update global values if needed
	UpdateGlobals();

	// Pointlight specials
	lgt.Type=D3DLIGHT_POINT;
	lgt.Position=D3DXVECTOR3(position_global.x,
		position_global.y,position_global.z);
	lgt.Range=10*decay;
	lgt.Attenuation0=1;//1;
	lgt.Attenuation1=1/decay;
	lgt.Attenuation2=0;

	// Set light (to 3d-card)
	Storm3D2->GetD3DDevice()->SetLight(num,&lgt);
}



//------------------------------------------------------------------
// Storm3D_Light_Point::SetColor
//------------------------------------------------------------------
void Storm3D_Light_Point::SetColor(COL &_color)
{
	color=_color;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::SetPosition
//------------------------------------------------------------------
void Storm3D_Light_Point::SetPosition(VC3 &_position)
{
	position=_position;
	update_globals=true;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::SetMultiplier
//------------------------------------------------------------------
void Storm3D_Light_Point::SetMultiplier(float _multiplier)
{
	multiplier=_multiplier;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::SetDecay
//------------------------------------------------------------------
void Storm3D_Light_Point::SetDecay(float _decay)
{
	decay=_decay;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetDecay
//------------------------------------------------------------------
float Storm3D_Light_Point::GetDecay()
{
	return decay;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::SetLensFlare
//------------------------------------------------------------------
void Storm3D_Light_Point::SetLensFlare(IStorm3D_LensFlare *_lflare)
{
	lflare=(Storm3D_LensFlare*)_lflare;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetColor
//------------------------------------------------------------------
COL &Storm3D_Light_Point::GetColor()
{
	return color;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetPosition
//------------------------------------------------------------------
VC3 &Storm3D_Light_Point::GetPosition()
{
	return position;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetGlobalPosition
//------------------------------------------------------------------
VC3 &Storm3D_Light_Point::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetMultiplier
//------------------------------------------------------------------
float Storm3D_Light_Point::GetMultiplier()
{
	return multiplier;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetLensFlare
//------------------------------------------------------------------
IStorm3D_LensFlare *Storm3D_Light_Point::GetLensFlare()
{
	return lflare;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetLightType
//------------------------------------------------------------------
IStorm3D_Light::LTYPE Storm3D_Light_Point::GetLightType()
{
	return LTYPE_POINT;
}



//------------------------------------------------------------------
// Storm3D_Light_Point::SetName
//------------------------------------------------------------------
void Storm3D_Light_Point::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}



//------------------------------------------------------------------
// Storm3D_Light_Point::GetName
//------------------------------------------------------------------
const char *Storm3D_Light_Point::GetName()
{
	return name;
}



//------------------------------------------------------------------
// Storm3D_Light_Point - Keyframe animation
//------------------------------------------------------------------
void Storm3D_Light_Point::Animation_Clear()
{
	animation.Clear();
}


void Storm3D_Light_Point::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}


void Storm3D_Light_Point::Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color)
{
	animation.AddNewLuminanceKeyFrame(time,multiplier,decay,color);
}


void Storm3D_Light_Point::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}


void Storm3D_Light_Point::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}



void Storm3D_Light_Point::UpdateGlobals()
{
	if (update_globals)
	{
		update_globals=false;
		position_global=position;
		if (parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
		}
	}
}


