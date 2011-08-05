/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

  Class: Storm3D_Mesh_Animation_?????

	- Keyframe animation stuff (mesh)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_animation.h"
#include "storm3d_mesh.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Mesh_Animation
//------------------------------------------------------------------
Storm3D_Mesh_Animation::Storm3D_Mesh_Animation(Storm3D_Mesh *_owner) :
	owner(_owner),
	first_meshkey(NULL)
{
}


Storm3D_Mesh_Animation::~Storm3D_Mesh_Animation()
{
	Clear();
}


void Storm3D_Mesh_Animation::Clear()
{
	// Delete everything (works recursively)
	SAFE_DELETE(first_meshkey);
}


void Storm3D_Mesh_Animation::AddNewMeshKeyFrame(int time,const Storm3D_Vertex *vertexes)
{
	// Search for right position in list
	Storm3D_KeyFrame_Mesh *kfp=first_meshkey;
	Storm3D_KeyFrame_Mesh *old_kfp=NULL;
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
		Storm3D_KeyFrame_Mesh *newkf=new Storm3D_KeyFrame_Mesh(time,owner,vertexes);
		old_kfp->next_key=newkf;
		if (kfp) newkf->next_key=kfp;
	}
	else	// This was first keyframe
	{
		if (first_meshkey) // If there is already first key, move it backwards
		{
			Storm3D_KeyFrame_Mesh *temp=first_meshkey;
			first_meshkey=new Storm3D_KeyFrame_Mesh(time,owner,vertexes);
			first_meshkey->next_key=temp;
		}
		else	// The list is empty
		{
			first_meshkey=new Storm3D_KeyFrame_Mesh(time,owner,vertexes);
		}
	}
}


void Storm3D_Mesh_Animation::Apply(Storm3D_Scene *scene,int time_now)
{
	// Calculate time difference
	/*static DWORD dlast_time=SDL_GetTicks();
	DWORD dtime_now=SDL_GetTicks();
	DWORD dtime_dif=dtime_now-dlast_time;
	dlast_time=dtime_now;*/

	// Is the mesh mesh animated?
	if (first_meshkey)
	{		
		// Search for mesh keyframes
		Storm3D_KeyFrame_Mesh *kfp=first_meshkey;
		Storm3D_KeyFrame_Mesh *old_kfp=NULL;
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
			if (td==0) td=1;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate mesh
			Storm3D_Vertex *vbuf=owner->GetVertexBuffer();
			int vam=owner->GetVertexCount();

			// Loop vertices
			for (int vx=0;vx<vam;vx++)
			{
				// Optimized lerp (BETA: should be copied to other stuff too)
				vbuf[vx].position=(kfp->vertexes[vx].position-old_kfp->vertexes[vx].position)*ipol+old_kfp->vertexes[vx].position;
				vbuf[vx].normal=(kfp->vertexes[vx].normal-old_kfp->vertexes[vx].normal)*ipol+old_kfp->vertexes[vx].normal;
			}
		}
		/*else if ((old_kfp)&&(old_kfp!=first_meshkey)) // Interpolate between last and first
		{
			//Calculate time delta (and interpolation)
			int td=end_time-old_kfp->keytime;
			if (td==0) td=1;
			float ipol=(float)(time_now-old_kfp->keytime)/(float)td;

			// Interpolate mesh
			Storm3D_Vertex *vbuf=owner->GetVertexBuffer();
			int vam=owner->GetVertexCount();

			// Loop vertices
			for (int vx=0;vx<vam;vx++)
			{
				vbuf[vx].position=(first_meshkey->vertexes[vx].position*ipol)+(old_kfp->vertexes[vx].position*(1.0f-ipol));
				vbuf[vx].normal=(first_meshkey->vertexes[vx].normal*ipol)+(old_kfp->vertexes[vx].normal*(1.0f-ipol));
			}
		}*/
	}
}



