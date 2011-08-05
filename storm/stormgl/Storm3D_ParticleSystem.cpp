// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"

#include <vector>

#include <GL/glew.h>
#include "storm3d_scene.h"
#include "storm3d_particle.h"
#include "VertexFormats.h"
#include "storm3d_terrain_utils.h"
#include "../../util/Debug_MemoryManager.h"
#include "igios3D.h"
#include <boost/lexical_cast.hpp>

using namespace frozenbyte::storm;


//GLuint Storm3D_ParticleSystem::PointArray::m_vb = 0;
int Storm3D_ParticleSystem::PointArray::m_totalParticles = 0;
//int Storm3D_ParticleSystem::PointArray::m_maxParticles = 0;

GLuint Storm3D_ParticleSystem::QuadArray::m_vb = 0;
GLuint Storm3D_ParticleSystem::QuadArray::m_ib = 0;
int Storm3D_ParticleSystem::QuadArray::m_totalParticles = 0;
int Storm3D_ParticleSystem::QuadArray::m_maxParticles = 0;

GLuint Storm3D_ParticleSystem::LineArray::m_vb = 0;
GLuint Storm3D_ParticleSystem::LineArray::m_ib = 0;
int Storm3D_ParticleSystem::LineArray::m_totalParticles = 0;
int Storm3D_ParticleSystem::LineArray::m_maxParticles = 0;

namespace {

	frozenbyte::storm::IndexBuffer indexBuffer;
	frozenbyte::storm::VertexBuffer vertexBuffer[2];
	char *temporaryPointer[2] = { 0, 0};
	int allocateIndexAmount = 22000 * 6; // 4096 particles in one call
	int allocateVertexAmount[2] = { 0, 0 };
	int vertexBufferOffset[2] = { 0, 0 };

	void initBuffer(int particleBufferSize, int index)
	{
		int vertexAmount = particleBufferSize * 4;
		if(vertexAmount < 2048)
			vertexAmount = 2048;

		if(vertexAmount > allocateVertexAmount[index])
		{
			vertexBufferOffset[index] = 0;
			vertexBuffer[index].create(vertexAmount, sizeof(VXFORMAT_PART), true);
			allocateVertexAmount[index] = vertexAmount;

			delete[] temporaryPointer[index];
			temporaryPointer[index] = new char[vertexAmount * sizeof(VXFORMAT_PART)];
		}

		if(!indexBuffer)
		{
			int particleAmount = allocateIndexAmount / 6;
			indexBuffer.create(particleAmount * 2, false);

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

//! Release dynamic buffers
void releaseDynamicBuffers()
{
	allocateVertexAmount[0] = 0;
	vertexBufferOffset[0] = 0;
	vertexBuffer[0].release();
	allocateVertexAmount[1] = 0;
	vertexBufferOffset[1] = 0;
	vertexBuffer[1].release();
}

//! Recreate dynamic buffers
void recreateDynamicBuffers()
{
}

//! Release buffers
void releaseBuffers()
{
	releaseDynamicBuffers();

	delete[] temporaryPointer[0];
	delete[] temporaryPointer[1];
	temporaryPointer[0] = 0;
	temporaryPointer[1] = 0;

	indexBuffer.release();
}

//! Render
/*!
	\param scene scene
	\param distortion use distortion
*/
void Storm3D_ParticleSystem::Render(Storm3D_Scene* scene, bool distortion) 
{
	RenderImp(scene, distortion);
	Clear();
}

//! Render
/*!
	\param scene scene
	\param distortion use distortion
*/
void Storm3D_ParticleSystem::RenderImp(Storm3D_Scene *scene, bool distortion) 
{
	frozenbyte::storm::setCulling(CULL_NONE);
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glActiveTexture(GL_TEXTURE1);
	glClientActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 1.0f/255.0f);
	*/
	scene->camera.Apply();

	for (int i = 1; i < 8; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	frozenbyte::storm::VertexShader::disable();

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

	initBuffer(2 * maxParticles, index);
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

	applyFVF(FVF_VXFORMAT_PART, sizeof(VXFORMAT_PART));
	vertexBuffer[index].apply(0);
	// ugly HACK!
	int prevBindOffs = 0;

	frozenbyte::storm::VertexShader::disable();

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
				(*it)->setRender(vertexOffset, particleAmount);

				if(particleAmount > 0)
				{
					// Do we have to something more intelligent - no need for this large amounts?
					if(particleAmount >= allocateIndexAmount / 6)
						particleAmount = allocateIndexAmount / 6 - 1;
					if (prevBindOffs != vertexOffset + oldVertexBufferOffset) {
						vertexBuffer[index].apply(0, (vertexOffset + oldVertexBufferOffset) * sizeof(VXFORMAT_PART));
						prevBindOffs = vertexOffset + oldVertexBufferOffset;
					}
					// FIXME: this is broken

					indexBuffer.render(particleAmount * 2, particleAmount * 4);
				}

				scene->AddPolyCounter(particleAmount);
			}
		}
	}

	scene->camera.Apply();
}

//! Clear
void Storm3D_ParticleSystem::Clear() 
{
	std::vector<IParticleArray *>::iterator it = m_particleArrays.begin();
	for(; it != m_particleArrays.end(); ++it) 
			delete (*it);

	m_particleArrays.clear();
}
