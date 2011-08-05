// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
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
		virtual void setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount) = 0;

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

		static IDirect3DVertexBuffer9* m_vb;
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
		void setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount);

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

		static IDirect3DVertexBuffer9* m_vb;
		static IDirect3DIndexBuffer9* m_ib;
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
		void setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount);

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

		static IDirect3DVertexBuffer9* m_vb;
		static IDirect3DIndexBuffer9* m_ib;
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
		void setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount);

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

	void ReleaseDynamicDXBuffers() {
		PointArray::releaseDynamicBuffers();
		QuadArray::releaseDynamicBuffers();
		LineArray::releaseDynamicBuffers();

		releaseDynamicBuffers();
	}
	
	void ReCreateDynamicDXBuffers() {
		
		PointArray::createDynamicBuffers(Storm3D2);	
		QuadArray::createDynamicBuffers(Storm3D2);	
		LineArray::createDynamicBuffers(Storm3D2);	
	
		recreateDynamicBuffers();
	}

	Storm3D_ParticleSystem(Storm3D *storm) 
	:	Storm3D2(storm),
		offsetShader(*storm->GetD3DDevice())
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

/*

//------------------------------------------------------------------
// Storm3D_ParticleSystem_PMH (ParticleMaterialHandler)
//------------------------------------------------------------------
class Storm3D_ParticleSystem_PMH
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	Storm3D_Material *material;
	Storm3D_PointParticle *partptr_points;
	Storm3D_LineParticle *partptr_lines;
	int num_parts;
	Storm3D_ParticleAnimationInfo animInfo;

	// Renders all particles with this material
	void Render(Storm3D_Scene *scene,Storm3D_ParticleSystem *pts);

	Storm3D_ParticleSystem_PMH(Storm3D *Storm3D2,Storm3D_Material *material,Storm3D_PointParticle *parts,int array_size, Storm3D_ParticleAnimationInfo* ainfo);
	Storm3D_ParticleSystem_PMH(Storm3D *Storm3D2,Storm3D_Material *material,Storm3D_LineParticle *parts,int array_size, Storm3D_ParticleAnimationInfo* ainfo);
	~Storm3D_ParticleSystem_PMH();

	friend class Storm3D_ParticleSystem;
};



//------------------------------------------------------------------
// Storm3D_ParticleSystem
//------------------------------------------------------------------
class Storm3D_ParticleSystem : public IStorm3D_ParticleSystem
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// Particles "in air" grouped by material
	set<Storm3D_ParticleSystem_PMH*> particle_groups;
	
	// v2.3 buffers
	LPDIRECT3DVERTEXBUFFER8 dx8_vbuf;	// Vertexbuffer (in videomemory)
	LPDIRECT3DINDEXBUFFER8 dx8_ibuf;	// Indexbuffer (in videomemory
	int buffer_size;
	int biggest_group_size;

public:

	// DX buffer handling (for lost devices)
	// For particlesystem vbuffer
	void ReleaseDynamicDXBuffers();
	void ReCreateDynamicDXBuffers();

	// Renders all particles in system
	//void Render(Storm3D_Scene *scene,bool ismain);
	void Render(Storm3D_Scene *scene);

	// New v2.3 particle system
	// Use these to render array(s) of particles (actually particles are stored into list
	// and rendered at the time when scene is rendered, and then are removed from the list)
	void RenderParticles(IStorm3D_Material *mat,Storm3D_PointParticle *part,int array_size, Storm3D_ParticleAnimationInfo* ainfo);
	void RenderParticles(IStorm3D_Material *mat,Storm3D_LineParticle *part,int array_size, Storm3D_ParticleAnimationInfo* ainfo);

	Storm3D_ParticleSystem(Storm3D *Storm3D2);
	~Storm3D_ParticleSystem();

	friend class Storm3D_ParticleSystem_PMH;
};



*/
