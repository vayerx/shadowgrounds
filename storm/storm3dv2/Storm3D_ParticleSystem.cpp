// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"

#include <vector>

#include "storm3d_scene.h"
#include "storm3d_particle.h"
#include "Storm3D_ShaderManager.h"
#include "VertexFormats.h"
#include "storm3d_terrain_utils.h"
#include "..\..\util\Debug_MemoryManager.h"
#include <boost/lexical_cast.hpp>

using namespace frozenbyte::storm;


IDirect3DVertexBuffer9* Storm3D_ParticleSystem::PointArray::m_vb = NULL;
int Storm3D_ParticleSystem::PointArray::m_totalParticles = 0;
int Storm3D_ParticleSystem::PointArray::m_maxParticles = 0;

IDirect3DVertexBuffer9* Storm3D_ParticleSystem::QuadArray::m_vb = NULL;
IDirect3DIndexBuffer9* Storm3D_ParticleSystem::QuadArray::m_ib = NULL;
int Storm3D_ParticleSystem::QuadArray::m_totalParticles = 0;
int Storm3D_ParticleSystem::QuadArray::m_maxParticles = 0;

IDirect3DVertexBuffer9* Storm3D_ParticleSystem::LineArray::m_vb = NULL;
IDirect3DIndexBuffer9* Storm3D_ParticleSystem::LineArray::m_ib = NULL;
int Storm3D_ParticleSystem::LineArray::m_totalParticles = 0;
int Storm3D_ParticleSystem::LineArray::m_maxParticles = 0;

namespace {

	frozenbyte::storm::IndexBuffer indexBuffer;
	frozenbyte::storm::VertexBuffer vertexBuffer[2];
	char *temporaryPointer[2] = { 0, 0};
	//int allocateIndexAmount = 4096 * 6; // 4096 particles in one call
	int allocateIndexAmount = 22000 * 6; // 4096 particles in one call
	int allocateVertexAmount[2] = { 0, 0 };
	int vertexBufferOffset[2] = { 0, 0 };

	void initBuffer(IDirect3DDevice9 &device, int particleBufferSize, int index)
	{
		int vertexAmount = particleBufferSize * 4;
		if(vertexAmount < 2048)
			vertexAmount = 2048;

		if(vertexAmount > allocateVertexAmount[index])
		{
			vertexBufferOffset[index] = 0;
			vertexBuffer[index].create(device, vertexAmount, sizeof(VXFORMAT_PART), true);
			allocateVertexAmount[index] = vertexAmount;

			delete[] temporaryPointer[index];
			temporaryPointer[index] = new char[vertexAmount * sizeof(VXFORMAT_PART)];
		}

		if(!indexBuffer)
		{
			int particleAmount = allocateIndexAmount / 6;
			indexBuffer.create(device, particleAmount * 2, false);

			unsigned short int *buffer = indexBuffer.lock();
			int position = 0;

			for(int i = 0; i < particleAmount; ++i)
			{
				buffer[position + 0] = i*4 + 0;
				buffer[position + 1] = i*4 + 1;
				buffer[position + 2] = i*4 + 2;
				buffer[position + 3] = i*4 + 2;
				buffer[position + 4] = i*4 + 1;
				buffer[position + 5] = i*4 + 3;
				position += 6;
			}

			indexBuffer.unlock();
		}
	}

} // unnamed

void releaseDynamicBuffers()
{
	allocateVertexAmount[0] = 0;
	vertexBufferOffset[0] = 0;
	vertexBuffer[0].release();
	allocateVertexAmount[1] = 0;
	vertexBufferOffset[1] = 0;
	vertexBuffer[1].release();
}

void recreateDynamicBuffers()
{
}

void releaseBuffers()
{
	releaseDynamicBuffers();

	delete[] temporaryPointer[0];
	delete[] temporaryPointer[1];
	temporaryPointer[0] = 0;
	temporaryPointer[1] = 0;

	indexBuffer.release();
}

void Storm3D_ParticleSystem::Render(Storm3D_Scene* scene, bool distortion) 
{
	RenderImp(scene, distortion);
	Clear();
}

void Storm3D_ParticleSystem::RenderImp(Storm3D_Scene *scene, bool distortion) 
{
	//if(distortion && !offsetShader.hasShader())
	//	offsetShader.createOffsetBlendShader();

	IDirect3DDevice9 &device = *Storm3D2->GetD3DDevice();
	//device.SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	frozenbyte::storm::setCulling(device, D3DCULL_NONE);
	device.SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	device.SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	device.SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	device.SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	device.SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	device.SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	device.SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	device.SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	device.SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
	device.SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	device.SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
	device.SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
	device.SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	device.SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	device.SetRenderState(D3DRS_ALPHAREF, 0x1);
	scene->camera.Apply();

	device.SetVertexShader(0);
	device.SetFVF(FVF_VXFORMAT_PART);

	int maxParticles = 0;
	int index = (distortion) ? 1 : 0;

	// Get particle amount
	{
		std::vector<IParticleArray *>::iterator it = m_particleArrays.begin();
		for(; it != m_particleArrays.end(); ++it)
		{
			if((*it)->isDistorted() == distortion)
				maxParticles += (*it)->getMaxParticleAmount();
		}
	}

	initBuffer(device, 2 * maxParticles, index);
	int oldVertexBufferOffset = 0;

	// Update buffer
	{
		VXFORMAT_PART *updatePointer = reinterpret_cast<VXFORMAT_PART *> (temporaryPointer[index]);
		int particleOffset = 0;

		if(updatePointer)
		{
			std::vector<IParticleArray *>::iterator it = m_particleArrays.begin();
			for(; it != m_particleArrays.end(); ++it) 
			{
				if((*it)->isDistorted() == distortion)
				{
					particleOffset += (*it)->lock(updatePointer, particleOffset, scene);
					assert(particleOffset <= maxParticles);
				}
			}

			VXFORMAT_PART *vertexPointer = 0;
			int neededVertexAmount = particleOffset * 4;
/*
if(neededVertexAmount)
{
std::string msg = "Buffer size, vertex offset, neededVertices: ";
msg += boost::lexical_cast<std::string> (allocateVertexAmount) + std::string(", ");
msg += boost::lexical_cast<std::string> (vertexBufferOffset) + std::string(", ");
msg += boost::lexical_cast<std::string> (neededVertexAmount);
msg += "\r\n";
OutputDebugString(msg.c_str());
}
*/
			if(neededVertexAmount)
			{
				// WTF we need this for -- DISCARD/NOOVERWRITE shoud do the trick already?
				// Bugs only with certain explosion effect - DX issue?
				if(vertexBufferOffset + neededVertexAmount >= allocateVertexAmount)
				{
					vertexBufferOffset[index] = 0;
					vertexPointer = static_cast<VXFORMAT_PART *> (vertexBuffer[index].lock());
					vertexBuffer[index].unlock();
				}

				if(vertexBufferOffset + neededVertexAmount < allocateVertexAmount)
				{
					vertexPointer = static_cast<VXFORMAT_PART *> (vertexBuffer[index].unsafeLock(vertexBufferOffset[index], neededVertexAmount));
					oldVertexBufferOffset = vertexBufferOffset[index];
					vertexBufferOffset[index] += neededVertexAmount;
				}
				else
				{
					vertexBufferOffset[index] = 0;
					vertexPointer = static_cast<VXFORMAT_PART *> (vertexBuffer[index].lock());
				}

				memcpy(vertexPointer, updatePointer, neededVertexAmount * sizeof(VXFORMAT_PART));
				vertexBuffer[index].unlock();
			}
		}
	}

	vertexBuffer[index].apply(device, 0);

	device.SetVertexShader(0);
	device.SetFVF(FVF_VXFORMAT_PART);

	// Render arrays
	{
		std::vector<IParticleArray *>::iterator it = m_particleArrays.begin();
		for(; it != m_particleArrays.end(); ++it) 
		{
			if((*it)->isDistorted() == distortion)
			{
				int vertexOffset = 0;
				int particleAmount = 0;

				scene->camera.Apply();
				(*it)->setRender(device, vertexOffset, particleAmount);

				if(particleAmount > 0)
				{
					// Do we have to something more intelligent - no need for this large amounts?
					if(particleAmount >= allocateIndexAmount / 6)
						particleAmount = allocateIndexAmount / 6 - 1;

/*
{
std::string msg = "\tRender offset, amount: ";
msg += boost::lexical_cast<std::string> (vertexOffset + oldVertexBufferOffset) + std::string(", ");
msg += boost::lexical_cast<std::string> (particleAmount * 4);
msg += "\r\n";
OutputDebugString(msg.c_str());
}
*/
//if(!oldVertexBufferOffset)
//continue;
					indexBuffer.render(device, particleAmount * 2, particleAmount * 4, vertexOffset + oldVertexBufferOffset);
				}

				scene->AddPolyCounter(particleAmount);
			}
		}
	}

	scene->camera.Apply();
}

void Storm3D_ParticleSystem::Clear() 
{
	std::vector<IParticleArray *>::iterator it = m_particleArrays.begin();
	for(; it != m_particleArrays.end(); ++it) 
			delete (*it);

	m_particleArrays.clear();
}


/*

//------------------------------------------------------------------
// Storm3D_ParticleSystem::Storm3D_ParticleSystem
//------------------------------------------------------------------
Storm3D_ParticleSystem::Storm3D_ParticleSystem(Storm3D *s2) :
	Storm3D2(s2),
	//gravity(VC3(0,0,0))
	dx8_vbuf(NULL),
	dx8_ibuf(NULL),
	buffer_size(0),
	biggest_group_size(0)
{
}



//------------------------------------------------------------------
// Storm3D_ParticleSystem::~Storm3D_ParticleSystem
//------------------------------------------------------------------
Storm3D_ParticleSystem::~Storm3D_ParticleSystem()
{
	// Delete particlegroups
	for(set<Storm3D_ParticleSystem_PMH*>::iterator ip=particle_groups.begin();
		ip!=particle_groups.end();ip++)
	{
		delete (*ip);
	}
	particle_groups.clear();
}



//------------------------------------------------------------------
// Storm3D_ParticleSystem::Render
//------------------------------------------------------------------
void Storm3D_ParticleSystem::Render(Storm3D_Scene *scene/*,bool ismain*///)
/*{

	// Calculate time difference
	/*static DWORD last_time=timeGetTime();
	DWORD time_now=timeGetTime();
	DWORD time_dif=time_now-last_time;
	last_time=time_now;*/

	// Set renderstates
/*	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGENABLE,FALSE);
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	Storm3D2->GetD3DDevice()->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	Storm3D2->GetD3DDevice()->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);

	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);

	
	// BETA!!!!!!!!!
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,FALSE);

	// [v2.3 stuff]
	// Make bigger buffers if needed
	if (buffer_size<biggest_group_size)
	{
		// Set new size
		buffer_size=biggest_group_size;

		// Create new vertexbuffer (and delete old)
		SAFE_RELEASE(dx8_vbuf);
		
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
		{
			Storm3D2->GetD3DDevice()->CreateVertexBuffer(buffer_size*4*sizeof(VXFORMAT_PART),
				D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
				D3DPOOL_DEFAULT,&dx8_vbuf);
		}
		else
		{
			Storm3D2->GetD3DDevice()->CreateVertexBuffer(buffer_size*4*sizeof(VXFORMAT_PART),
				D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
				D3DPOOL_DEFAULT,&dx8_vbuf);
		}
/*
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
		{
			Storm3D2->GetD3DDevice()->CreateVertexBuffer(buffer_size*4*sizeof(QuadParticleVertex),
				D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,0,
				D3DPOOL_DEFAULT,&dx8_vbuf);
		}
		else
		{
			Storm3D2->GetD3DDevice()->CreateVertexBuffer(buffer_size*4*sizeof(QuadParticleVertex),
				D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,0,
				D3DPOOL_DEFAULT,&dx8_vbuf);
		}
*/
/*		// Create new indexbuffer (and delete old)
		SAFE_RELEASE(dx8_ibuf);
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
		{
			Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*buffer_size*6,
				D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);
		}
		else
		{
			Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*buffer_size*6,
				D3DUSAGE_WRITEONLY|D3DUSAGE_SOFTWAREPROCESSING,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);
		}

		// (Pre)Fill indexbuffer (optimization)
		WORD *ip=NULL;
		dx8_ibuf->Lock(0,sizeof(WORD)*buffer_size*6,(BYTE**)&ip,0);
		if (ip)
		{
			int pos=0,pc=0;
			for (int i=0;i<buffer_size;i++)
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

	// Render particlegroups (if any)
	if (buffer_size>0)
	{
		//Storm3D_ShaderManager::GetSingleton()->SetParticleShader(Storm3D2->GetD3DDevice(), 
		//	16, 16, 0);
		
		for(set<Storm3D_ParticleSystem_PMH*>::iterator ip=particle_groups.begin();
			ip!=particle_groups.end();ip++)
		{
			// Typecast to simplify code
			Storm3D_ParticleSystem_PMH *pmh=*ip;

			// Render it
			//pmh->Render((Storm3D_Scene*)scene,ismain,scene->time_dif);
			pmh->Render((Storm3D_Scene*)scene,this);
		}
	}

	// Return renderstates
	/*Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);*/
/*
	// Delete particlegroups always after rendering (v2.3)
	for(set<Storm3D_ParticleSystem_PMH*>::iterator ip=particle_groups.begin();ip!=particle_groups.end();ip++)
	{
		delete (*ip);
	}
	particle_groups.clear();
}



//------------------------------------------------------------------
// Storm3D_ParticleSystem::Spawn
//------------------------------------------------------------------
/*void Storm3D_ParticleSystem::Spawn(IStorm3D_Particle *iparticle,VC3 &position,VC3 &speed)
{
	Storm3D_Particle *particle=(Storm3D_Particle*)iparticle;

	// Set values
	particle->position=position;
	particle->speed=speed;

	// Loop through particle groups
	for (set<Storm3D_ParticleSystem_PMH*>::iterator pi=particle_groups.begin();
		pi!=particle_groups.end();pi++)
	{
		// Typecast to simplify code
		Storm3D_ParticleSystem_PMH *pmh=*pi;

		// Test if this groups material equals new particles material
		if (pmh->material==particle->material)
		{
			// Insert particle into the group's list
			pmh->particlelist.Add(particle);
			return;
		}

		// Continue searching until found (or list ends)...
	}

	// If we got here, it means that there is no particlegroup with
	// spawned particles material. So we need to create a new group.
	Storm3D_ParticleSystem_PMH *new_pmh=new Storm3D_ParticleSystem_PMH(Storm3D2,particle->material);
	particle_groups.insert(new_pmh);

	// Insert particle into the (new) group's list
	new_pmh->particlelist.Add(particle);
}*/



//------------------------------------------------------------------
// Storm3D_ParticleSystem - v2.3 particlerender
//------------------------------------------------------------------
/*void Storm3D_ParticleSystem::RenderParticles(IStorm3D_Material *mat,Storm3D_PointParticle *part,int array_size, Storm3D_ParticleAnimationInfo* ai)
{
	// v2.3 system: always create a new particlegroup
	Storm3D_ParticleSystem_PMH *new_pmh=new Storm3D_ParticleSystem_PMH(Storm3D2,(Storm3D_Material*)mat,part,array_size, ai);
	particle_groups.insert(new_pmh);

	// Set biggest group
	if (biggest_group_size<array_size) biggest_group_size=array_size;
}



//------------------------------------------------------------------
// Storm3D_ParticleSystem - v2.3 particlerender
//------------------------------------------------------------------
void Storm3D_ParticleSystem::RenderParticles(IStorm3D_Material *mat,Storm3D_LineParticle *part,int array_size, Storm3D_ParticleAnimationInfo* ai)
{
	// v2.3 system: always create a new particlegroup
	Storm3D_ParticleSystem_PMH *new_pmh=new Storm3D_ParticleSystem_PMH(Storm3D2,(Storm3D_Material*)mat,part,array_size, ai);
	particle_groups.insert(new_pmh);

	// Set biggest group
	if (biggest_group_size<array_size) biggest_group_size=array_size;
}



//------------------------------------------------------------------
// Storm3D_ParticleSystem::SetGravity
//------------------------------------------------------------------
/*void Storm3D_ParticleSystem::SetGravity(VC3 &gvec)
{
	gravity=gvec;
}*/



//------------------------------------------------------------------
// Storm3D_ParticleSystem - Reset routines
//------------------------------------------------------------------
/*void Storm3D_ParticleSystem::ReleaseDynamicDXBuffers()
{
	// Loop through particle groups
	/*for (set<Storm3D_ParticleSystem_PMH*>::iterator pi=particle_groups.begin();
		pi!=particle_groups.end();pi++)
	{
		// Typecast to simplify code
		Storm3D_ParticleSystem_PMH *pmh=*pi;
		pmh->particlelist.ReleaseDynamicDXBuffers();
	}*/

	// [v2.3]
/*	SAFE_RELEASE(dx8_vbuf);
}


void Storm3D_ParticleSystem::ReCreateDynamicDXBuffers()
{
	// Loop through particle groups
	/*for (set<Storm3D_ParticleSystem_PMH*>::iterator pi=particle_groups.begin();
		pi!=particle_groups.end();pi++)
	{
		// Typecast to simplify code
		Storm3D_ParticleSystem_PMH *pmh=*pi;
		pmh->particlelist.ReCreateDynamicDXBuffers();
	}*/

	// [v2.3]
	// Create vertex buffer
/*	if (!dx8_vbuf)
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(buffer_size*4*sizeof(VXFORMAT_PART),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
			D3DPOOL_DEFAULT,&dx8_vbuf);
}
*/

