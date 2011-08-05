/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Light_Spot

  Spot light

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
// Storm3D_Light_Spot::Storm3D_Light_Spot
//------------------------------------------------------------------
Storm3D_Light_Spot::Storm3D_Light_Spot(Storm3D *s2,const char *name,Storm3D_Model *_parent_model,
	COL &_color,float multiplier,float decay,VC3 &_position,VC3 &_direction,
	float _cone_inner,float _cone_outer,Storm3D_LensFlare *_lflare) :
	Storm3D_Light_AInterface(s2,name,_parent_model,_color,multiplier,decay),
	position(_position),
	lflare(_lflare),
	direction(_direction),
	cone_inner(_cone_inner),
	cone_outer(_cone_outer),
	flarevis(0),animation(this)
{
}


	
//------------------------------------------------------------------
// Storm3D_Light_Spot::~Storm3D_Light_Spot
//------------------------------------------------------------------
Storm3D_Light_Spot::~Storm3D_Light_Spot()
{
	SAFE_DELETE_ARRAY(name);
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::Apply
//------------------------------------------------------------------
void Storm3D_Light_Spot::Apply(int num)
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

	// Spotlight specials
	lgt.Type=D3DLIGHT_SPOT;
	lgt.Position=D3DXVECTOR3(position_global.x,
		position_global.y,position_global.z);
	lgt.Direction=D3DXVECTOR3(direction_global.x,
		direction_global.y,direction_global.z);
	//lgt.Direction=D3DXVECTOR3(direction.x,
	//	direction.y,direction.z);
	//lgt.Direction=D3DXVECTOR3(0,1,0);
	//lgt.Range=radius*2.0f;
	lgt.Range=10*decay;
	lgt.Attenuation0=1;//1;
	lgt.Attenuation1=1.0f/decay;
	lgt.Attenuation2=0;
	lgt.Theta=cone_inner;
	lgt.Phi=cone_outer;
	lgt.Falloff=1;
	
	// Set light (to 3d-card)
	Storm3D2->GetD3DDevice()->SetLight(num,&lgt);
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetColor
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetColor(COL &_color)
{
	color=_color;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetPosition
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetPosition(VC3 &_position)
{
	position=_position;
	update_globals=true;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetMultiplier
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetMultiplier(float _multiplier)
{
	multiplier=_multiplier;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetDecay
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetDecay(float _decay)
{
	decay=_decay;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetDecay
//------------------------------------------------------------------
float Storm3D_Light_Spot::GetDecay()
{
	return decay;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetLensFlare
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetLensFlare(IStorm3D_LensFlare *_lflare)
{
	lflare=(Storm3D_LensFlare*)_lflare;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetColor
//------------------------------------------------------------------
COL &Storm3D_Light_Spot::GetColor()
{
	return color;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetPosition
//------------------------------------------------------------------
VC3 &Storm3D_Light_Spot::GetPosition()
{
	return position;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetGlobalPosition
//------------------------------------------------------------------
VC3 &Storm3D_Light_Spot::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetMultiplier
//------------------------------------------------------------------
float Storm3D_Light_Spot::GetMultiplier()
{
	return multiplier;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetLensFlare
//------------------------------------------------------------------
IStorm3D_LensFlare *Storm3D_Light_Spot::GetLensFlare()
{
	return lflare;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetDirection
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetDirection(VC3 &_direction)
{
	direction=_direction;
	update_globals=true;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetCones
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetCones(float inner,float outer)
{
	cone_inner=inner;
	cone_outer=outer;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetDirection
//------------------------------------------------------------------
VC3 &Storm3D_Light_Spot::GetDirection()
{
	return direction;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetGlobalDirection
//------------------------------------------------------------------
VC3 &Storm3D_Light_Spot::GetGlobalDirection()
{
	// Update global values if needed
	UpdateGlobals();

	return direction_global;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetCones
//------------------------------------------------------------------
void Storm3D_Light_Spot::GetCones(float &inner,float &outer)
{
	inner=cone_inner;
	outer=cone_outer;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetLightType
//------------------------------------------------------------------
IStorm3D_Light::LTYPE Storm3D_Light_Spot::GetLightType()
{
	return LTYPE_SPOT;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::SetName
//------------------------------------------------------------------
void Storm3D_Light_Spot::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}



//------------------------------------------------------------------
// Storm3D_Light_Spot::GetName
//------------------------------------------------------------------
const char *Storm3D_Light_Spot::GetName()
{
	return name;
}



//------------------------------------------------------------------
// Storm3D_Light_Spot - Keyframe animation
//------------------------------------------------------------------
void Storm3D_Light_Spot::Animation_Clear()
{
	animation.Clear();
}


void Storm3D_Light_Spot::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}


void Storm3D_Light_Spot::Animation_AddNewDirectionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewDirectionKeyFrame(time,position);
}


void Storm3D_Light_Spot::Animation_AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color)
{
	animation.AddNewLuminanceKeyFrame(time,multiplier,decay,color);
}


void Storm3D_Light_Spot::Animation_AddNewConeKeyFrame(int time,float inner,float outer)
{
	animation.AddNewConeKeyFrame(time,inner,outer);
}


void Storm3D_Light_Spot::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}


void Storm3D_Light_Spot::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}


void Storm3D_Light_Spot::UpdateGlobals()
{
	if (update_globals)
	{
		update_globals=false;
		position_global=position;
		direction_global=direction;
		if (parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
			parent_object->GetMXG().GetWithoutTranslation().TransformVector(direction_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
			parent_model->GetMX().GetWithoutTranslation().TransformVector(direction_global);
		}
	}
}
