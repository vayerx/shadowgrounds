// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_animation.h"
#include "storm3d_scene.h"
#include "../../util/Debug_MemoryManager.h"



//------------------------------------------------------------------
// Storm3D_Helper_Animation
//------------------------------------------------------------------
Storm3D_Helper_Animation::Storm3D_Helper_Animation(IStorm3D_Helper *_owner) :
	owner(_owner),
	first_poskey(NULL),
	first_dirkey(NULL),
	first_upveckey(NULL),
	end_time(0),
	time_now(0)
{
}


Storm3D_Helper_Animation::~Storm3D_Helper_Animation()
{
	Clear();
}


void Storm3D_Helper_Animation::Clear()
{
	// Delete everything (works recursively)
	SAFE_DELETE(first_poskey);
	SAFE_DELETE(first_dirkey);
	SAFE_DELETE(first_upveckey);
}


void Storm3D_Helper_Animation::AddNewPositionKeyFrame(int time,const VC3 &position)
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


void Storm3D_Helper_Animation::AddNewDirectionKeyFrame(int time,const VC3 &position)
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
			first_dirkey=new Storm3D_KeyFrame_Vector(time,position.GetNormalized());
			first_dirkey->next_key=temp;
		}
		else	// The list is empty
		{
			first_dirkey=new Storm3D_KeyFrame_Vector(time,position.GetNormalized());
		}
	}
}


void Storm3D_Helper_Animation::AddNewUpVectorKeyFrame(int time,const VC3 &position)
{
	// Search for right position in list
	Storm3D_KeyFrame_Vector *kfp=first_upveckey;
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
		if (first_upveckey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Vector *temp=first_upveckey;
			first_upveckey=new Storm3D_KeyFrame_Vector(time,position.GetNormalized());
			first_upveckey->next_key=temp;
		}
		else	// The list is empty
		{
			first_upveckey=new Storm3D_KeyFrame_Vector(time,position.GetNormalized());
		}
	}
}


void Storm3D_Helper_Animation::SetLoop(int _end_time)
{
	end_time=_end_time;
}


void Storm3D_Helper_Animation::SetCurrentTime(int time)
{
	time_now=time;
}


void Storm3D_Helper_Animation::Apply(Storm3D_Scene *scene)
{
	// Add time
	time_now+=scene->time_dif;
	if (end_time>0) while (time_now>end_time) time_now-=end_time;

	// Get helper type
	IStorm3D_Helper::HTYPE htype=owner->GetHelperType();

	// Is the helper position animated?
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
			// Each helper can be typecasted to point helper
			((IStorm3D_Helper_Point*)owner)->SetPosition((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
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

	// Is the helper direction animated?
	if ((htype==IStorm3D_Helper::HTYPE_VECTOR)||
		(htype==IStorm3D_Helper::HTYPE_CAMERA))
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

			// Interpolate position+
			// Each camera/vector helper can be typecasted to vector helper
			((IStorm3D_Helper_Vector*)owner)->SetDirection((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
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

	// Is the helper up-vector animated?
	if (htype==IStorm3D_Helper::HTYPE_CAMERA)
	if (first_upveckey)
	{
		// Search for position keyframes
		Storm3D_KeyFrame_Vector *kfp=first_upveckey;
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

			// Interpolate position+
			// This is a camera...
			((IStorm3D_Helper_Camera*)owner)->SetUpVector((kfp->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}
		/*else if ((old_kfp)&&(old_kfp!=first_poskey)) // Interpolate between last and first
		{
			//Calculate time delta (and interpolation)
			int td=end_time-old_kfp->keytime;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate position
			owner->SetPosition((first_upveckey->position*ipol)+(old_kfp->position*(1.0f-ipol)));
		}*/
	}
}


