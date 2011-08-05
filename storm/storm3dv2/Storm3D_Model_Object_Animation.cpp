// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_animation.h"
#include "storm3d_scene.h"
#include "storm3d_model_object.h"

#include "../../util/Debug_MemoryManager.h"

Storm3D_Model_Object_Animation::Storm3D_Model_Object_Animation(Storm3D_Model_Object *_owner) :
	owner(_owner),
	first_poskey(NULL),
	first_rotkey(NULL),
	first_scalekey(NULL),
	end_time(0),
	time_now(0)
{
}


Storm3D_Model_Object_Animation::~Storm3D_Model_Object_Animation()
{
	Clear();
}


void Storm3D_Model_Object_Animation::Clear()
{
	// Delete everything (works recursively)
	SAFE_DELETE(first_poskey);
	SAFE_DELETE(first_rotkey);
	SAFE_DELETE(first_scalekey);
}


void Storm3D_Model_Object_Animation::AddNewPositionKeyFrame(int time,const VC3 &position)
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


void Storm3D_Model_Object_Animation::AddNewRotationKeyFrame(int time,const QUAT &rotation)
{
	// Search for right position in list
	Storm3D_KeyFrame_Rotation *kfp=first_rotkey;
	Storm3D_KeyFrame_Rotation *old_kfp=NULL;
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
		Storm3D_KeyFrame_Rotation *newkf=new Storm3D_KeyFrame_Rotation(time,rotation);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_rotkey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Rotation *temp=first_rotkey;
			first_rotkey=new Storm3D_KeyFrame_Rotation(time,rotation);
			first_rotkey->next_key=temp;
		}
		else	// The list is empty
		{
			first_rotkey=new Storm3D_KeyFrame_Rotation(time,rotation);
		}
	}
}


void Storm3D_Model_Object_Animation::AddNewScaleKeyFrame(int time,const VC3 &scale)
{
	// Search for right position in list
	Storm3D_KeyFrame_Scale *kfp=first_scalekey;
	Storm3D_KeyFrame_Scale *old_kfp=NULL;
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
		Storm3D_KeyFrame_Scale *newkf=new Storm3D_KeyFrame_Scale(time,scale);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_scalekey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Scale *temp=first_scalekey;
			first_scalekey=new Storm3D_KeyFrame_Scale(time,scale);
			first_scalekey->next_key=temp;
		}
		else	// The list is empty
		{
			first_scalekey=new Storm3D_KeyFrame_Scale(time,scale);
		}
	}
}


void Storm3D_Model_Object_Animation::SetLoop(int _end_time)
{
	end_time=_end_time;
}


void Storm3D_Model_Object_Animation::SetCurrentTime(int time)
{
	time_now=time;
}


void Storm3D_Model_Object_Animation::Apply(Storm3D_Scene *scene)
{
	// Add time
	time_now+=scene->time_dif;
	if (end_time>0) while (time_now>end_time) time_now-=end_time;

	// Is the object position animated?
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
			owner->SetPosition((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}
	}

	// Is the object rotation animated?
	if (first_rotkey)
	{
		// Search for rotation keyframes
		Storm3D_KeyFrame_Rotation *kfp=first_rotkey;
		Storm3D_KeyFrame_Rotation *old_kfp=NULL;
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

			// Interpolate rotation
			owner->SetRotation(old_kfp->rotation.GetSLInterpolationWith(kfp->rotation,ipol));
		}
	}

	// Is the object scale animated?
	if (first_scalekey)
	{
		// Search for scale keyframes
		Storm3D_KeyFrame_Scale *kfp=first_scalekey;
		Storm3D_KeyFrame_Scale *old_kfp=NULL;
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

			// Interpolate scale
			owner->SetScale((kfp->scale*ipol)+(old_kfp->scale*(1.0f-ipol)));
		}
	}
}
