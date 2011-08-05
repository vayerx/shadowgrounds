/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_ParticleList

	- Particlelist
	- Contains particles with same material

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>
#include <VertexFormats.h>
#include "clipper.h"



//------------------------------------------------------------------
// Storm3D_ParticleList::Storm3D_ParticleList
//------------------------------------------------------------------
Storm3D_ParticleList::Storm3D_ParticleList(Storm3D *s2) :
	Storm3D2(s2),
	list_size(10),	// Start with a list of 10 particles
	first_free(0)
{
	particlelist=new Storm3D_Particle[list_size];

	// Create new vertexbuffer
	Storm3D2->D3DDevice->CreateVertexBuffer(list_size*4*sizeof(VXFORMAT_2D),
		D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_2D,
		D3DPOOL_DEFAULT,&dx8_vbuf);

	// Create new indexbuffer
	Storm3D2->D3DDevice->CreateIndexBuffer(sizeof(WORD)*list_size*6,
		D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);

	// (Pre)Fill indexbuffer (optimization)
	WORD *ip=NULL;
	dx8_ibuf->Lock(0,sizeof(WORD)*list_size*6,(BYTE**)&ip,0);
	if (ip)
	{
		int pos=0,pc=0;
		for (int i=0;i<list_size;i++)
		{
			ip[pos+0]=0+pc;
			ip[pos+1]=1+pc;
			ip[pos+2]=2+pc;
			ip[pos+3]=0+pc;
			ip[pos+4]=2+pc;
			ip[pos+5]=3+pc;
			pos+=6;
			pc+=4;
		}
	}
	dx8_ibuf->Unlock();
}



//------------------------------------------------------------------
// Storm3D_ParticleList::~Storm3D_ParticleList
//------------------------------------------------------------------
Storm3D_ParticleList::~Storm3D_ParticleList()
{
	// Delete buffers
	delete[] particlelist;
	SAFE_RELEASE(dx8_vbuf);
	SAFE_RELEASE(dx8_ibuf);
}



//------------------------------------------------------------------
// Storm3D_ParticleList::Render
//------------------------------------------------------------------
void Storm3D_ParticleList::Render(Storm3D_Scene *scene,bool ismain,int time_dif)
{
	// Optimization: calculate gravity here
	VC3 grav=scene->particlesystem->gravity*((float)time_dif/1000.0f);

	// Mesh (fill) place index
	int mpi=0;

	// Lock vertex buffer
	BYTE *vp;
	dx8_vbuf->Lock(0,0,(BYTE**)&vp,D3DLOCK_DISCARD);
	if (vp==NULL) return;

	// Typecast (to simplify code)
	VXFORMAT_2D *mesh=(VXFORMAT_2D*)vp;

	// Optimization mesh
	VXFORMAT_2D mopt[4];

	mopt[0].rhw=1;
	mopt[0].texcoords.x=1;
	mopt[0].texcoords.y=1;

	mopt[1].rhw=1;
	mopt[1].texcoords.x=0;
	mopt[1].texcoords.y=1;

	mopt[2].rhw=1;
	mopt[2].texcoords.x=0;
	mopt[2].texcoords.y=0;

	mopt[3].rhw=1;
	mopt[3].texcoords.x=1;
	mopt[3].texcoords.y=0;

	// Animation flag (optimization)
	bool prev_was_animated=false;

	// Do particles tic / Render
	for (int pt=0;pt<list_size-1;pt++)
	if (particlelist[pt].lifetime>0)	// Only living particles
	{
		if (ismain)
		{
			// Add speed / gravity
			particlelist[pt].speed+=grav;

			// Test collision (BETA)
			/*Storm3D_CollisionInfo rti;
			scene.RayTrace(particlelist[pt].position,particlelist[pt].speed.GetNormalized(),particlelist[pt].speed.GetLength()*((float)time_dif/1000.0f),rti);
			if (rti.hit)
			{
				particlelist[pt].speed*=-1;
			}*/

			// Add position
			particlelist[pt].position+=particlelist[pt].speed*((float)time_dif/1000.0f);

			// Decrease lifetime
			particlelist[pt].lifetime-=time_dif;

			// Add animation frame
			if (particlelist[pt].framechangetime)
			{
				particlelist[pt].framechangecounter+=time_dif;
				if (particlelist[pt].framechangecounter>particlelist[pt].framechangetime)
				{
					particlelist[pt].framechangecounter=0;
					particlelist[pt].frame++;
					if (particlelist[pt].frame>=(particlelist[pt].columns*particlelist[pt].rows))
						particlelist[pt].frame=0;
				}
			}

			// Test particle's lifetime
			if (particlelist[pt].lifetime<1)
			{
				// Remove particle
				Remove(pt);

				// Do not render this particle
				continue;
			}
		}

		// Calculate particle's position on screen
		VC3 scpos;
		float w,rz;

		// Calculate position, and put particle into list only if it's visible
		if (particlelist[pt].position.GetTransformedToScreen((float*)&scene->camera.GetVP(),scpos,w,rz))
		if (rz<scene->camera.vis_range)
		if (rz>1)
		// BETA: here should be x/y visibility test
		{
			// Get viewport size
			Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();

			// VC3 values to actual display coordinates
			scpos.x*=ss.width;
			scpos.y*=ss.height;

			// Calculate size
			float hsizees=particlelist[pt].size/rz;
			float hsizex=(particlelist[pt].size/rz)*ss.width;
			float hsizey=(particlelist[pt].size/rz)*ss.height;

			// Aspect (BETA!... not yet)

			// Calculate color
			DWORD col;
			if (particlelist[pt].lifetime>particlelist[pt].fadestart)
			{
				col=particlelist[pt].material->GetColor().GetAsD3DCompatibleARGB();
			}
			else
			{
				float f=(float)particlelist[pt].lifetime/(float)particlelist[pt].fadestart;
				col=(particlelist[pt].material->GetColor()*f).GetAsD3DCompatibleARGB();
			}

			// Animated particles:
			// Texturecoordinates must be set
			if (particlelist[pt].framechangetime>0)
			{
				// Calculate texture coordinates
				float tx1=1/(float)particlelist[pt].columns;
				float ty1=1/(float)particlelist[pt].rows;
				/*float fx=(float)((int)particlelist[pt].frame%particlelist[pt].columns)*tx1;
				float fy=(float)((int)particlelist[pt].frame/particlelist[pt].columns)*ty1;*/
				float fx=(float)((int)particlelist[pt].frame/particlelist[pt].rows)*tx1;
				float fy=(float)((int)particlelist[pt].frame%particlelist[pt].rows)*ty1;

				// Set texturecoordinates
				mopt[0].rhw=1;
				mopt[0].texcoords.x=fx+tx1;
				mopt[0].texcoords.y=fy+ty1;

				mopt[1].rhw=1;
				mopt[1].texcoords.x=fx;
				mopt[1].texcoords.y=fy+ty1;
		
				mopt[2].rhw=1;
				mopt[2].texcoords.x=fx;
				mopt[2].texcoords.y=fy;

				mopt[3].rhw=1;
				mopt[3].texcoords.x=fx+tx1;
				mopt[3].texcoords.y=fy;

				prev_was_animated=true;
			}
			else if (prev_was_animated)
			{
				// If previous was animated, and this is not,
				// return basic un-animated texturecoordinates
				mopt[0].rhw=1;
				mopt[0].texcoords.x=1;
				mopt[0].texcoords.y=1;

				mopt[1].rhw=1;
				mopt[1].texcoords.x=0;
				mopt[1].texcoords.y=1;
		
				mopt[2].rhw=1;
				mopt[2].texcoords.x=0;
				mopt[2].texcoords.y=0;

				mopt[3].rhw=1;
				mopt[3].texcoords.x=1;
				mopt[3].texcoords.y=0;

				prev_was_animated=false;
			}

			// Test if the particle must be stretched
			if (particlelist[pt].stretch<0.01f)
			{
				// Add particle in to the vertexbuffer... (optimized version)
				float sxp=scpos.x+hsizex;
				float sxm=scpos.x-hsizex;
				float syp=scpos.y+hsizey;
				float sym=scpos.y-hsizey;

				// Faces (quad) (optimized stuff)
				mopt[0].position.x=sxp;
				mopt[0].position.y=syp;
				mopt[0].position.z=scpos.z;

				mopt[1].position.x=sxm;
				mopt[1].position.y=syp;
				mopt[1].position.z=scpos.z;

				mopt[2].position.x=sxm;
				mopt[2].position.y=sym;
				mopt[2].position.z=scpos.z;

				mopt[3].position.x=sxp;
				mopt[3].position.y=sym;
				mopt[3].position.z=scpos.z;

				/*mopt[0].texcoords.x=1;
				mopt[0].texcoords.y=1;
				mopt[1].texcoords.x=0;
				mopt[1].texcoords.y=1;
				mopt[2].texcoords.x=0;
				mopt[2].texcoords.y=0;
				mopt[3].texcoords.x=1;
				mopt[3].texcoords.y=0;*/

				// Clip
				/*if (Clip2DRectangle(Storm3D2,mopt[2],mopt[0])) 
				{
					// Copy clipping
					mopt[3].position.x=mopt[0].position.x;
					mopt[3].position.y=mopt[2].position.y;
					mopt[3].texcoords.x=mopt[0].texcoords.x;
					mopt[3].texcoords.y=mopt[2].texcoords.y;
					mopt[1].position.y=mopt[0].position.y;
					mopt[1].position.x=mopt[2].position.x;
					mopt[1].texcoords.y=mopt[0].texcoords.y;
					mopt[1].texcoords.x=mopt[2].texcoords.x;
*/
					// Optimized copy
					mopt[0].color=mopt[1].color=mopt[2].color=mopt[3].color=col;
					memcpy(&mesh[mpi],mopt,sizeof(VXFORMAT_2D)*4);
					mpi+=4;
				//}
			}
			else
			{	
				// BETA: Render particle only if has last position
				if (ismain)		// BETA: FIX THIS!!
				if (particlelist[pt].has_last_position)
				{
					// Calculate vector between current and last position
					VC2 v1(scpos.x-particlelist[pt].last_position.x,scpos.y-particlelist[pt].last_position.y);
					const float length=sqrtf(v1.x*v1.x+v1.y*v1.y);
					v1.x/=length;
					v1.y/=length;

					// Create other vector (at 90deg angle)
					VC2 v2(v1.y*hsizees,-v1.x*hsizees);

					// Modify vector 1
					v1.x*=length+(hsizees*ss.width)*particlelist[pt].stretch;
					v1.y*=length+(hsizees*ss.width)*particlelist[pt].stretch;

					// Add particle in to the vertexbuffer... (optimized version)

					// Faces (quad) (optimized stuff)
					mopt[0].position.x=scpos.x+(-v1.x+v2.x*ss.width);
					mopt[0].position.y=scpos.y+(-v1.y+v2.y*ss.height);
					mopt[0].position.z=scpos.z;

					mopt[1].position.x=scpos.x+(-v1.x-v2.x*ss.width);
					mopt[1].position.y=scpos.y+(-v1.y-v2.y*ss.height);
					mopt[1].position.z=scpos.z;

					mopt[2].position.x=scpos.x+(v1.x-v2.x*ss.width);
					mopt[2].position.y=scpos.y+(v1.y-v2.y*ss.height);
					mopt[2].position.z=scpos.z;

					mopt[3].position.x=scpos.x+(v1.x+v2.x*ss.width);
					mopt[3].position.y=scpos.y+(v1.y+v2.y*ss.height);
					mopt[3].position.z=scpos.z;


					// Optimized copy
					mopt[0].color=mopt[1].color=mopt[2].color=mopt[3].color=col;
					memcpy(&mesh[mpi],mopt,sizeof(VXFORMAT_2D)*4);
					mpi+=4;					
				}

				// Save current position as last position
				if (ismain)
				{
					particlelist[pt].last_position=scpos;
					particlelist[pt].has_last_position=true;
				}
			}
		} 
		else
		{
			if (ismain)
				particlelist[pt].has_last_position=false;
		}
	}

	// Unlock vertex buffer
	dx8_vbuf->Unlock();

	if (mpi>0)
	{
		// Render the buffer
		Storm3D2->D3DDevice->SetVertexShader(FVF_VXFORMAT_2D);
		Storm3D2->D3DDevice->SetStreamSource(0,dx8_vbuf,sizeof(VXFORMAT_2D));
		Storm3D2->D3DDevice->SetIndices(dx8_ibuf,0);

		// Render as indexed primitive
		++storm3d_dip_calls;
		Storm3D2->D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,mpi,0,mpi/2);
		scene->AddPolyCounter(mpi/2);
	}
}



//------------------------------------------------------------------
// Storm3D_ParticleList::Add
//------------------------------------------------------------------
int Storm3D_ParticleList::Add(Storm3D_Particle *part)
{
	// Search for a free space
	while (particlelist[first_free].lifetime>0) first_free++;

	// Test if the array ended (if it's the terminator)
	if (first_free==list_size-1)
	{
		// So the particle array is FULL
		// We need to allocate more space to it.
		Storm3D_Particle *newlist;
		int new_size=list_size*2;	// double the size
		newlist=new Storm3D_Particle[new_size];

		// Copy stuff from the old array to the new bigger one
		memcpy(newlist,particlelist,sizeof(Storm3D_Particle)*list_size);
		
		// Delete old list
		delete[] particlelist;

		// Set new values
		particlelist=newlist;
		list_size=new_size;

		// Create new vertexbuffer (and delete old)
		SAFE_RELEASE(dx8_vbuf);
		Storm3D2->D3DDevice->CreateVertexBuffer(list_size*4*sizeof(VXFORMAT_2D),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_2D,
			D3DPOOL_DEFAULT,&dx8_vbuf);

		// Create new indexbuffer (and delete old)
		SAFE_RELEASE(dx8_ibuf);
		Storm3D2->D3DDevice->CreateIndexBuffer(sizeof(WORD)*list_size*6,
			D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);

		// (Pre)Fill indexbuffer (optimization)
		WORD *ip=NULL;
		dx8_ibuf->Lock(0,sizeof(WORD)*list_size*6,(BYTE**)&ip,0);
		if (ip)
		{
			int pos=0,pc=0;
			for (int i=0;i<list_size;i++)
			{
				ip[pos+0]=0+pc;
				ip[pos+1]=1+pc;
				ip[pos+2]=2+pc;
				ip[pos+3]=0+pc;
				ip[pos+4]=2+pc;
				ip[pos+5]=3+pc;
				pos+=6;
				pc+=4;
			}
		}
		dx8_ibuf->Unlock();

		// Search for a free space (second try)
		while (particlelist[first_free].lifetime>0) first_free++;

		// Test if the array ended (if it's the terminator)
		if (first_free==list_size-1)
		{
			// There is an error...
			// Do not create particles...
			// Return error...
			return -1;
		}
	}

	// Add the new particle to the list
	particlelist[first_free]=*part;
	first_free++;

	// Return particle's index
	return first_free-1;
}



//------------------------------------------------------------------
// Storm3D_ParticleList::Remove
//------------------------------------------------------------------
void Storm3D_ParticleList::Remove(int position)
{
	// Mark it removed
	particlelist[position].lifetime=0;
	particlelist[position].has_last_position=false;
	
	// Update first_free inxed if needed
	if (position<first_free) first_free=position;
}



//------------------------------------------------------------------
// Storm3D_ParticleList - Reset routines
//------------------------------------------------------------------
void Storm3D_ParticleList::ReleaseDynamicDXBuffers()
{
	SAFE_RELEASE(dx8_vbuf);
}


void Storm3D_ParticleList::ReCreateDynamicDXBuffers()
{
	// Create vertex buffer
	if (!dx8_vbuf)
		Storm3D2->D3DDevice->CreateVertexBuffer(list_size*4*sizeof(VXFORMAT_2D),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_2D,
			D3DPOOL_DEFAULT,&dx8_vbuf);
}


