/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  Class: Storm3D_Light_Animation_?????

	- Light keyframe animation stuff (position/direction/luminance/cones)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_animation.h"
#include "storm3d_scene.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Light_Animation
//------------------------------------------------------------------
Storm3D_Light_Animation::Storm3D_Light_Animation(IStorm3D_Light *_owner) :
	owner(_owner),
	first_poskey(NULL),
	first_dirkey(NULL),
	first_lumkey(NULL),
	first_conekey(NULL),
	end_time(0),
	time_now(0)
{
}


Storm3D_Light_Animation::~Storm3D_Light_Animation()
{
	Clear();
}


void Storm3D_Light_Animation::Clear()
{
	// Delete everything (works recursively)
	SAFE_DELETE(first_poskey);
	SAFE_DELETE(first_dirkey);
	SAFE_DELETE(first_lumkey);
	SAFE_DELETE(first_conekey);
}


void Storm3D_Light_Animation::AddNewPositionKeyFrame(int time,const VC3 &position)
{
	// Search for right position in list
	Storm3D_KeyFrame_Vector *kfp=first_poskey;
	Storm3D_KeyFrame_Vector *old_kfp=NULL;
	while(kfp)
	{
		// Test if we found a right spot
		if (kfp->keytime>time) break;
	
		// Next
		old_kfp=kfp;
		kfp=kfp->next_key;
	}

	if (old_kfp)	// There was keyframes before this
	{
		Storm3D_KeyFrame_Vector *newkf=new Storm3D_KeyFrame_Vector(time,position);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_poskey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Vector *temp=first_poskey;
			first_poskey=new Storm3D_KeyFrame_Vector(time,position);
			first_poskey->next_key=temp;
		}
		else	// The list is empty
		{
			first_poskey=new Storm3D_KeyFrame_Vector(time,position);
		}
	}
}


void Storm3D_Light_Animation::AddNewDirectionKeyFrame(int time,const VC3 &position)
{
	// Search for right position in list
	Storm3D_KeyFrame_Vector *kfp=first_dirkey;
	Storm3D_KeyFrame_Vector *old_kfp=NULL;
	while(kfp)
	{
		// Test if we found a right spot
		if (kfp->keytime>time) break;
	
		// Next
		old_kfp=kfp;
		kfp=kfp->next_key;
	}

	if (old_kfp)	// There was keyframes before this
	{
		Storm3D_KeyFrame_Vector *newkf=new Storm3D_KeyFrame_Vector(time,position);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_dirkey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Vector *temp=first_dirkey;
			first_dirkey=new Storm3D_KeyFrame_Vector(time,position);
			first_dirkey->next_key=temp;
		}
		else	// The list is empty
		{
			first_dirkey=new Storm3D_KeyFrame_Vector(time,position);
		}
	}
}


void Storm3D_Light_Animation::AddNewLuminanceKeyFrame(int time,float multiplier,float decay,const COL &color)
{
	// Search for right position in list
	Storm3D_KeyFrame_Luminance *kfp=first_lumkey;
	Storm3D_KeyFrame_Luminance *old_kfp=NULL;
	while(kfp)
	{
		// Test if we found a right spot
		if (kfp->keytime>time) break;
	
		// Next
		old_kfp=kfp;
		kfp=kfp->next_key;
	}

	if (old_kfp)	// There was keyframes before this
	{
		Storm3D_KeyFrame_Luminance *newkf=new Storm3D_KeyFrame_Luminance(time,multiplier,decay,color);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_lumkey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Luminance *temp=first_lumkey;
			first_lumkey=new Storm3D_KeyFrame_Luminance(time,multiplier,decay,color);
			first_lumkey->next_key=temp;
		}
		else	// The list is empty
		{
			first_lumkey=new Storm3D_KeyFrame_Luminance(time,multiplier,decay,color);
		}
	}
}


void Storm3D_Light_Animation::AddNewConeKeyFrame(int time,float inner,float outer)
{
	// Search for right position in list
	Storm3D_KeyFrame_Cones *kfp=first_conekey;
	Storm3D_KeyFrame_Cones *old_kfp=NULL;
	while(kfp)
	{
		// Test if we found a right spot
		if (kfp->keytime>time) break;
	
		// Next
		old_kfp=kfp;
		kfp=kfp->next_key;
	}

	if (old_kfp)	// There was keyframes before this
	{
		Storm3D_KeyFrame_Cones *newkf=new Storm3D_KeyFrame_Cones(time,inner,outer);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_conekey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Cones *temp=first_conekey;
			first_conekey=new Storm3D_KeyFrame_Cones(time,inner,outer);
			first_conekey->next_key=temp;
		}
		else	// The list is empty
		{
			first_conekey=new Storm3D_KeyFrame_Cones(time,inner,outer);
		}
	}
}


void Storm3D_Light_Animation::SetLoop(int _end_time)
{
	end_time=_end_time;
}


void Storm3D_Light_Animation::SetCurrentTime(int time)
{
	time_now=time;
}


void Storm3D_Light_Animation::Apply(Storm3D_Scene *scene)
{
	// Add time
	time_now+=scene->time_dif;
	if (end_time>0) while (time_now>end_time) time_now-=end_time;

	// Get helper type
	IStorm3D_Light::LTYPE ltype=owner->GetLightType();

	// Is the light position animated?
	if (ltype!=IStorm3D_Light::LTYPE_DIRECTIONAL)
	if (first_poskey)
	{
		// Search for position keyframes
		Storm3D_KeyFrame_Vector *kfp=first_poskey;
		Storm3D_KeyFrame_Vector *old_kfp=NULL;
		while(kfp)
		{
			// Test if we found a right spot
			if (kfp->keytime>time_now) break;
	
			// Next
			old_kfp=kfp;
			kfp=kfp->next_key;
		}

		// Animate
		if ((old_kfp)&&(kfp))	// Between 2 keyframes
		{
			//Calculate time delta (and interpolation)
			int td=kfp->keytime-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate position
			// Each nondirectional light can be typecasted to point light
			((IStorm3D_Light_Point*)owner)->SetPosition((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}
		/*else if ((old_kfp)&&(old_kfp!=first_poskey)) // Interpolate between last and first
		{
			//Calculate time delta (and interpolation)
			int td=end_time-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate position
			owner->SetPosition((first_poskey->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}*/
	}

	// Is the light direction animated?
	if ((ltype==IStorm3D_Light::LTYPE_SPOT)||
		(ltype==IStorm3D_Light::LTYPE_DIRECTIONAL))
	if (first_dirkey)
	{
		// Search for position keyframes
		Storm3D_KeyFrame_Vector *kfp=first_dirkey;
		Storm3D_KeyFrame_Vector *old_kfp=NULL;
		while(kfp)
		{
			// Test if we found a right spot
			if (kfp->keytime>time_now) break;
	
			// Next
			old_kfp=kfp;
			kfp=kfp->next_key;
		}

		// Animate
		if ((old_kfp)&&(kfp))	// Between 2 keyframes
		{
			//Calculate time delta (and interpolation)
			int td=kfp->keytime-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate direction+			
			if (ltype==IStorm3D_Light::LTYPE_SPOT) ((IStorm3D_Light_Spot*)owner)->SetDirection((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
			else ((IStorm3D_Light_Directional*)owner)->SetDirection((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}
		/*else if ((old_kfp)&&(old_kfp!=first_poskey)) // Interpolate between last and first
		{
			//Calculate time delta (and interpolation)
			int td=end_time-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate position
			owner->SetPosition((first_dirkey->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}*/
	}

	// Is the light luminance animated?
	if (first_lumkey)
	{
		// Search for position keyframes
		Storm3D_KeyFrame_Luminance *kfp=first_lumkey;
		Storm3D_KeyFrame_Luminance *old_kfp=NULL;
		while(kfp)
		{
			// Test if we found a right spot
			if (kfp->keytime>time_now) break;
	
			// Next
			old_kfp=kfp;
			kfp=kfp->next_key;
		}

		// Animate
		if ((old_kfp)&&(kfp))	// Between 2 keyframes
		{
			//Calculate time delta (and interpolation)
			int td=kfp->keytime-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate color+
			owner->SetColor((kfp->color*ipol)+(old_kfp->color*(1.0f-ipol)));
			
			// Interpolate range if it's not directional light
			//if (ltype!=IStorm3D_Light::LTYPE_DIRECTIONAL)
			//	((IStorm3D_Light_Point*)owner)->SetRadius((kfp->radius*ipol)+(old_kfp->radius*(1.0f-ipol)));
			owner->SetMultiplier((kfp->multiplier*ipol)+(old_kfp->multiplier*(1.0f-ipol)));
			owner->SetDecay((kfp->decay*ipol)+(old_kfp->decay*(1.0f-ipol)));
		}
		/*else if ((old_kfp)&&(old_kfp!=first_poskey)) // Interpolate between last and first
		{
			//Calculate time delta (and interpolation)
			int td=end_time-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate position
			owner->SetPosition((first_lumkey->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}*/
	}

	// Is the light cone animated?
	if (ltype==IStorm3D_Light::LTYPE_SPOT)
	if (first_conekey)
	{
		// Search for position keyframes
		Storm3D_KeyFrame_Cones *kfp=first_conekey;
		Storm3D_KeyFrame_Cones *old_kfp=NULL;
		while(kfp)
		{
			// Test if we found a right spot
			if (kfp->keytime>time_now) break;
	
			// Next
			old_kfp=kfp;
			kfp=kfp->next_key;
		}

		// Animate
		if ((old_kfp)&&(kfp))	// Between 2 keyframes
		{
			//Calculate time delta (and interpolation)
			int td=kfp->keytime-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate cones
			((IStorm3D_Light_Spot*)owner)->SetCones(
				(kfp->inner*ipol)+(old_kfp->inner*(1.0f-ipol)),
				(kfp->outer*ipol)+(old_kfp->outer*(1.0f-ipol)));
		}
		/*else if ((old_kfp)&&(old_kfp!=first_poskey)) // Interpolate between last and first
		{
			//Calculate time delta (and interpolation)
			int td=end_time-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate position
			owner->SetPosition((first_conekey->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}*/
	}
}


