// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <GL/glew.h>
#include "storm3d_common_imp.h"
#include "IStorm3D_Particle.h"
#include "VertexFormats.h"
#include "storm3d_terrain_utils.h"
#include "storm3d.h"

void releaseDynamicBuffers();
void recreateDynamicBuffers();
void releaseBuffers();

class Storm3D_ParticleSystem : public IStorm3D_ParticleSystem {
public:

	class IParticleArray 
	{
	public:
		virtual ~IParticleArray() {}
		
		virtual bool isDistorted() const = 0;
		virtual int getMaxParticleAmount() const = 0;
		virtual int lock(VXFORMAT_PART *pointer, int particleOffset, Storm3D_Scene *scene) = 0;
		virtual void setRender(int &vertexOffset, int &particleAmount) = 0;

		virtual void render(Storm3D* Storm3D2, Storm3D_Scene* scene)=0;
	};

	class PointArray : public IParticleArray 
	{
	public:

		friend class Storm3D_ParticleSystem;
		
		Storm3D_PointParticle* m_parts;
		int m_numParts;
		int m_partOffset;
		IStorm3D_Material* m_mtl;

		COL factor;
		bool distortion;

		static GLuint m_vb;
		static int m_totalParticles;
		static int m_maxParticles;

		static void init(Storm3D* storm);
		static void releaseDynamicBuffers();
		static void createDynamicBuffers(Storm3D* storm);
		static void release();

	public:
		PointArray(IStorm3D_Material* mtl, Storm3D_PointParticle* parts, int nParts, const COL &factor_, bool distortion_) 
		: m_parts(parts), m_numParts(nParts), m_partOffset(0), m_mtl(mtl), factor(factor_), distortion(distortion_)
		{
		}

		bool isDistorted() const { return distortion; };
		
		int getMaxParticleAmount() const;
		int lock(VXFORMAT_PART *pointer, int vertexOffset, Storm3D_Scene *scene);
		void setRender(int &vertexOffset, int &particleAmount);

		void render(Storm3D* Storm3D2, Storm3D_Scene* scene);
	};

	class QuadArray : public IParticleArray 
	{
	public:
		friend class Storm3D_ParticleSystem;

		Storm3D_PointParticle* m_parts;
		int m_numParts;
		int m_partOffset;
		IStorm3D_Material* m_mtl;
		Storm3D_ParticleTextureAnimationInfo m_animInfo;

		COL factor;
		bool distortion;
		bool faceUp;

		static GLuint m_vb;
		static GLuint m_ib;
		static int m_totalParticles;
		static int m_maxParticles;
		
		static void init(Storm3D* storm);
		static void releaseDynamicBuffers();
		static void createDynamicBuffers(Storm3D* storm);
		static void release();
	
	public:
		QuadArray(IStorm3D_Material* mtl, Storm3D_PointParticle* parts, int nParts, Storm3D_ParticleTextureAnimationInfo* info, const COL &factor_, bool distortion_, bool faceUp_) 
		: m_parts(parts), m_numParts(nParts), m_partOffset(0), m_mtl(mtl), factor(factor_), distortion(distortion_), faceUp(faceUp_)
		{
			if(info) {
				m_animInfo.numFrames = info->numFrames;
				m_animInfo.textureUSubDivs = info->textureUSubDivs;
				m_animInfo.textureVSubDivs = info->textureVSubDivs;

				if(m_animInfo.numFrames > 0)
				{
					if(!m_animInfo.textureUSubDivs || !m_animInfo.textureVSubDivs)
						m_animInfo.numFrames = 0;
				}

			} else {
				m_animInfo.numFrames = 0;
			}		
		}

		bool isDistorted() const { return distortion; };
	
		int getMaxParticleAmount() const;
		int lock(VXFORMAT_PART *pointer, int particleOffset, Storm3D_Scene *scene);
		void setRender(int &vertexOffset, int &particleAmount);

		void render(Storm3D* Storm3D2, Storm3D_Scene* scene);
	};
	
	class LineArray : public IParticleArray 
	{
	public:
		friend class Storm3D_ParticleSystem;

		Storm3D_LineParticle* m_parts;
		int m_numParts;
		int m_partOffset;
		IStorm3D_Material* m_mtl;
		Storm3D_ParticleTextureAnimationInfo m_animInfo;

		COL factor;
		bool distortion;

		static GLuint m_vb;
		static GLuint m_ib;
		static int m_totalParticles;
		static int m_maxParticles;
		
		static void init(Storm3D* storm);
		static void releaseDynamicBuffers();
		static void createDynamicBuffers(Storm3D* storm);
		static void release();
	
	public:
		LineArray(IStorm3D_Material* mtl, Storm3D_LineParticle* parts, int nParts, Storm3D_ParticleTextureAnimationInfo* info, const COL &factor_, bool distortion_) 
		: m_parts(parts), m_numParts(nParts), m_partOffset(0), m_mtl(mtl), factor(factor_), distortion(distortion_) 
		{
			if(info) {
				m_animInfo.numFrames = info->numFrames;
				m_animInfo.textureUSubDivs = info->textureUSubDivs;
				m_animInfo.textureVSubDivs = info->textureVSubDivs;

				if(m_animInfo.numFrames > 0)
				{
					if(!m_animInfo.textureUSubDivs || !m_animInfo.textureVSubDivs)
						m_animInfo.numFrames = 0;
				}
			} else {
				m_animInfo.numFrames = 0;
			}
		}
		
		bool isDistorted() const { return distortion; };

		int getMaxParticleAmount() const;
		int lock(VXFORMAT_PART *pointer, int particleOffset, Storm3D_Scene *scene);
		void setRender(int &vertexOffset, int &particleAmount);

		void render(Storm3D* Storm3D2, Storm3D_Scene* scene);
	};

	std::vector<IParticleArray*> m_particleArrays;
	Storm3D* Storm3D2;
	
	frozenbyte::storm::PixelShader offsetShader;

	void renderPoints(IStorm3D_Material* mtl, Storm3D_PointParticle* parts, int nParts, const COL &factor, bool distortion) {
		
		if(PointArray::m_totalParticles < nParts)
			PointArray::m_totalParticles = nParts;
		
		m_particleArrays.push_back(new PointArray(mtl, parts, nParts, factor, distortion));
	}
	
	void renderQuads(IStorm3D_Material* mtl, Storm3D_PointParticle* parts, int nParts,
		Storm3D_ParticleTextureAnimationInfo* info, const COL &factor, bool distortion, bool faceUp) {

		if(QuadArray::m_totalParticles < nParts)
			QuadArray::m_totalParticles = nParts;
		
		m_particleArrays.push_back(new QuadArray(mtl, parts, nParts, info, factor, distortion, faceUp));
	}
	
	void renderLines(IStorm3D_Material* mtl, Storm3D_LineParticle* parts, int nParts,
		Storm3D_ParticleTextureAnimationInfo* info, const COL &factor, bool distortion) {

		if(LineArray::m_totalParticles < nParts)
			LineArray::m_totalParticles = nParts;
		
		m_particleArrays.push_back(new LineArray(mtl, parts, nParts, info, factor, distortion));
	}
		
	void Render(Storm3D_Scene *scene, bool distortion = false);
	void RenderImp(Storm3D_Scene *scene, bool distortion);
	void Clear();

	void ReleaseDynamicBuffers() {
		PointArray::releaseDynamicBuffers();
		QuadArray::releaseDynamicBuffers();
		LineArray::releaseDynamicBuffers();

		releaseDynamicBuffers();
	}
	
	void ReCreateDynamicBuffers() {
		
		PointArray::createDynamicBuffers(Storm3D2);	
		QuadArray::createDynamicBuffers(Storm3D2);	
		LineArray::createDynamicBuffers(Storm3D2);	
	
		recreateDynamicBuffers();
	}

	Storm3D_ParticleSystem(Storm3D *storm) 
	:	Storm3D2(storm),
		offsetShader()
	{
		
		PointArray::m_totalParticles = 500;
		QuadArray::m_totalParticles = 500;
		LineArray::m_totalParticles = 500;

	}
	~Storm3D_ParticleSystem() {
	
		PointArray::release();
		QuadArray::release();
		LineArray::release();
	
		releaseBuffers();
	}

};
