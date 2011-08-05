// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

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
#include "../../util/Debug_MemoryManager.h"
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

