// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_scene.h"
#include "storm3d_particle.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_material.h"
#include "VertexFormats.h"

#include "../../util/Debug_MemoryManager.h"
#include <d3dx9.h>

extern int storm3d_dip_calls;

void Storm3D_ParticleSystem::PointArray::init(Storm3D* Storm3D2) 
{
}

// PointArray

int Storm3D_ParticleSystem::PointArray::getMaxParticleAmount() const
{
	return 0;
}

int Storm3D_ParticleSystem::PointArray::lock(VXFORMAT_PART *pointer, int particleOffset, Storm3D_Scene *scene)
{
	return 0;
}

void Storm3D_ParticleSystem::PointArray::setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount)
{
}

void Storm3D_ParticleSystem::PointArray::render(Storm3D* Storm3D2, Storm3D_Scene* scene) 
{
	// TODO: implementation using point sprites
}

void Storm3D_ParticleSystem::PointArray::release() 
{
}

void Storm3D_ParticleSystem::PointArray::createDynamicBuffers(Storm3D* Storm3D2) 
{
}

void Storm3D_ParticleSystem::PointArray::releaseDynamicBuffers() 
{
}

// QuadArray


void Storm3D_ParticleSystem::QuadArray::releaseDynamicBuffers() {
	SAFE_RELEASE(m_vb);
}

void Storm3D_ParticleSystem::QuadArray::release() {
	SAFE_RELEASE(m_vb);
	SAFE_RELEASE(m_ib);
}

void Storm3D_ParticleSystem::QuadArray::createDynamicBuffers(Storm3D* Storm3D2) 
{
/*
	SAFE_RELEASE(m_vb);
	if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
	{
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(m_maxParticles*4*sizeof(VXFORMAT_PART),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
			D3DPOOL_DEFAULT,&m_vb, 0);
	}
	else
	{
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(m_maxParticles*4*sizeof(VXFORMAT_PART),
			D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
			D3DPOOL_DEFAULT,&m_vb, 0);
	}
*/	
}

void Storm3D_ParticleSystem::QuadArray::init(Storm3D* Storm3D2) {
	
	if((m_totalParticles > m_maxParticles) || (m_vb == NULL)) {
		
		m_maxParticles = m_totalParticles + 10;
		
		/*
		createDynamicBuffers(Storm3D2);
		
		SAFE_RELEASE(m_ib);
		
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
		{
			Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*m_maxParticles*6,
				D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_ib, 0);
		}
		else
		{
			Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*m_maxParticles*6,
				D3DUSAGE_WRITEONLY|D3DUSAGE_SOFTWAREPROCESSING,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_ib, 0);
		}
		
		// prefill ib
		WORD *ip=NULL;
		m_ib->Lock(0,sizeof(WORD)*m_maxParticles*6,(void**)&ip,0);
		if (ip)
		{
			int pos=0;
			for (int i=0;i<m_maxParticles;i++)
			{
				ip[pos+0]=i * 4 + 0;
				ip[pos+1]=i * 4 + 1;
				ip[pos+2]=i * 4 + 2;
				ip[pos+3]=i * 4 + 2;
				ip[pos+4]=i * 4 + 1;
				ip[pos+5]=i * 4 + 3;
				pos+=6;
			}
		}
		m_ib->Unlock();		
		*/
	}
	
}

namespace quad_util
{

inline void  rotatePointFast(float& x1, float& y1, float tx, float ty, float ca, float sa) {
	x1 = ca * tx - sa * ty;
	y1 = sa * tx + ca * ty;
}


}

int Storm3D_ParticleSystem::QuadArray::getMaxParticleAmount() const
{
	return m_numParts;
}

namespace {
	void rotateToward(const VC3 &a, const VC3 &b, QUAT &result)
	{
		VC3 axis = a.GetCrossWith(b);
		float dot = a.GetDotWith(b);

		if(dot < -0.99f)
		{
			result = QUAT();
			return;
		}

		result.x = axis.x;
		result.y = axis.y;
		result.z = axis.z;
		result.w = (dot + 1.0f);
		result.Normalize();
	}
}

int Storm3D_ParticleSystem::QuadArray::lock(VXFORMAT_PART *pointer, int particleOffset, Storm3D_Scene *scene)
{
	m_partOffset = particleOffset;
	pointer += particleOffset * 4;
	VXFORMAT_PART *vp = pointer;

	float frameWidth = 0.0f;
	float frameHeight = 0.0f;
	if(m_animInfo.numFrames > 1) 
	{
		frameWidth = 1.0f / m_animInfo.textureUSubDivs;
		frameHeight = 1.0f / m_animInfo.textureVSubDivs;
	}
	
	BYTE *verts = (BYTE*)&vp[0].position;
	BYTE *uvs = (BYTE*)&vp[0].texcoords;
	BYTE *colors = (BYTE*)&vp[0].color;
	
	DWORD stride = sizeof(VXFORMAT_PART);
	D3DXMATRIX mv = scene->camera.GetViewMatrix();
	DWORD c0 = 0;

	// Up rotation
	MAT view(scene->camera.GetViewMatrix());
	VC3 worldUp(0, 1.f, 0);
	view.RotateVector(worldUp);
	QUAT q;
	rotateToward(worldUp, VC3(0, 0, 1.f), q);
	MAT g;
	g.CreateRotationMatrix(q);

	for(int i = 0; i < m_numParts; i++) 
	{
		Storm3D_PointParticle& p = m_parts[i];

		float sa = sinf(p.angle);
		float ca = cosf(p.angle);
		float hsize = 0.5f*p.size;
			
		float x1,y1,x2,y2,x3,y3,x4,y4;

		quad_util::rotatePointFast(x1, y1, -hsize, hsize, ca, sa);
		quad_util::rotatePointFast(x2, y2,  hsize, hsize, ca, sa);
		quad_util::rotatePointFast(x3, y3, -hsize, -hsize, ca, sa);
		quad_util::rotatePointFast(x4, y4,  hsize, -hsize, ca, sa);

		VC3 v;
			
		v.x = p.position.x * mv.m[0][0] + 
			p.position.y * mv.m[1][0] + 
			p.position.z * mv.m[2][0] + mv.m[3][0];
		
		v.y = p.position.x * mv.m[0][1] + 
			p.position.y * mv.m[1][1] + 
			p.position.z * mv.m[2][1] + mv.m[3][1];

		v.z = p.position.x * mv.m[0][2] + 
			p.position.y * mv.m[1][2] + 
			p.position.z * mv.m[2][2] + mv.m[3][2];

		VC3 v1(x1, y1, 0);
		VC3 v2(x2, y2, 0);
		VC3 v3(x3, y3, 0);
		VC3 v4(x4, y4, 0);

		if(faceUp)
		{
			g.RotateVector(v1);
			g.RotateVector(v2);
			g.RotateVector(v3);
			g.RotateVector(v4);
		}

		v1 += v;
		v2 += v;
		v3 += v;
		v4 += v;

		/*
		VC3 v1(v.x + x1, v.y + y1, v.z);
		VC3 v2(v.x + x2, v.y + y2, v.z);
		VC3 v3(v.x + x3, v.y + y3, v.z);
		VC3 v4(v.x + x4, v.y + y4, v.z);

		{
			v1 -= v;
			g.RotateVector(v1);
			v1 += v;

			v2 -= v;
			g.RotateVector(v2);
			v2 += v;

			v3 -= v;
			g.RotateVector(v3);
			v3 += v;

			v4 -= v;
			g.RotateVector(v4);
			v4 += v;
		}
		*/

		*((Vector*)verts) = v1; verts += stride;
		*((Vector*)verts) = v2; verts += stride;
		*((Vector*)verts) = v3; verts += stride;
		*((Vector*)verts) = v4; verts += stride;

		// Fill texturecoords
		if(m_animInfo.numFrames > 1) {
			
			int frame = (int)p.frame % m_animInfo.numFrames;
			
			int col = frame % m_animInfo.textureUSubDivs;
			int row = frame / m_animInfo.textureUSubDivs;
			
			float tx = frameWidth * (float)col;
			float ty = frameHeight * (float)row;				
			
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);

		} else {

			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			
		}

		c0 = (((DWORD)(p.alpha * 255.0f) &0xff) << 24) |
			(((DWORD)(factor.r * p.color.r * 255.0f) &0xff) << 16) |
			(((DWORD)(factor.g * p.color.g * 255.0f) &0xff) << 8) |
			(((DWORD)(factor.b * p.color.b * 255.0f) &0xff) );
	
		*((DWORD*)colors) = c0; colors += stride;
		*((DWORD*)colors) = c0; colors += stride;
		*((DWORD*)colors) = c0; colors += stride;
		*((DWORD*)colors) = c0; colors += stride;

	}

	return m_numParts;
}

void Storm3D_ParticleSystem::QuadArray::setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount)
{
	if(m_numParts <= 0)
		return;

	if (m_mtl)
	{	
		static_cast<Storm3D_Material*>(m_mtl)->ApplyBaseTextureExtOnly();
	}
	else
	{
		device.SetTexture(0,NULL);
	}

	D3DMATRIX dm;
	dm._12=dm._13=dm._14=0;
	dm._21=dm._23=dm._24=0;
	dm._31=dm._32=dm._34=0;
	dm._41=dm._42=dm._43=0;
	dm._11=dm._22=dm._33=dm._44=1;

	device.SetTransform(D3DTS_WORLD,&dm);
	device.SetTransform(D3DTS_VIEW,&dm);

	vertexOffset = m_partOffset * 4;
	particleAmount = m_numParts;
}

void Storm3D_ParticleSystem::QuadArray::render(Storm3D* Storm3D2, Storm3D_Scene* scene) {
	// Not used anywhere

	if(m_numParts <= 0)
		return;

	if (m_mtl)
	{	
		static_cast<Storm3D_Material*>(m_mtl)->ApplyBaseTextureExtOnly();
	}
	else
	{
		Storm3D2->GetD3DDevice()->SetTexture(0,NULL);
	}

	// lock vb
	
	VXFORMAT_PART *vp;
	m_vb->Lock(0,0,(void**)&vp,D3DLOCK_DISCARD);
	if (vp==NULL) return;
	
	float frameWidth=0.0f;
	float frameHeight=0.0f;
	if(m_animInfo.numFrames > 1) {
		frameWidth = 1.0f / m_animInfo.textureUSubDivs;
		frameHeight = 1.0f / m_animInfo.textureVSubDivs;
	}
	
	BYTE* verts = (BYTE*)&vp[0].position;
	BYTE* uvs = (BYTE*)&vp[0].texcoords;
	BYTE* colors = (BYTE*)&vp[0].color;
	
	DWORD stride = sizeof(VXFORMAT_PART);

	D3DXMATRIX mv = scene->camera.GetViewMatrix();
		
	DWORD c0;
	
	for(int i = 0; i < m_numParts; i++) {
	
		Storm3D_PointParticle& p = m_parts[i];

		c0 = (((DWORD)(p.alpha * 255.0f) &0xff) << 24) |
			(((DWORD)(p.color.r * 255.0f) &0xff) << 16) |
			(((DWORD)(p.color.g * 255.0f) &0xff) << 8) |
			(((DWORD)(p.color.b * 255.0f) &0xff) );


		float sa = sinf(p.angle);
		float ca = cosf(p.angle);
		float hsize = 0.5f*p.size;
			
		float x1,y1,x2,y2,x3,y3,x4,y4;

		quad_util::rotatePointFast(x1, y1, -hsize, hsize, ca, sa);
		quad_util::rotatePointFast(x2, y2,  hsize, hsize, ca, sa);
		quad_util::rotatePointFast(x3, y3, -hsize, -hsize, ca, sa);
		quad_util::rotatePointFast(x4, y4,  hsize, -hsize, ca, sa);

		VC3 v;
			
		v.x = p.position.x * mv.m[0][0] + 
			p.position.y * mv.m[1][0] + 
			p.position.z * mv.m[2][0] + mv.m[3][0];
		
		v.y = p.position.x * mv.m[0][1] + 
			p.position.y * mv.m[1][1] + 
			p.position.z * mv.m[2][1] + mv.m[3][1];

		v.z = p.position.x * mv.m[0][2] + 
			p.position.y * mv.m[1][2] + 
			p.position.z * mv.m[2][2] + mv.m[3][2];

		VC3 v1(v.x + x1, v.y + y1, v.z);
		VC3 v2(v.x + x2, v.y + y2, v.z);
		VC3 v3(v.x + x3, v.y + y3, v.z);
		VC3 v4(v.x + x4, v.y + y4, v.z);
				
		*((Vector*)verts) = v1; verts += stride;
		*((Vector*)verts) = v2; verts += stride;
		*((Vector*)verts) = v3; verts += stride;
		*((Vector*)verts) = v4; verts += stride;

		// Fill texturecoords
		if(m_animInfo.numFrames > 1) {
			
			int frame = (int)p.frame % m_animInfo.numFrames;
			
			int col = frame % m_animInfo.textureUSubDivs;
			int row = frame / m_animInfo.textureUSubDivs;
			
			float tx = frameWidth * (float)col;
			float ty = frameHeight * (float)row;				
			
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);

		} else {

			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			
		}
		
		*((DWORD*)colors) = c0; colors += stride;
		*((DWORD*)colors) = c0; colors += stride;
		*((DWORD*)colors) = c0; colors += stride;
		*((DWORD*)colors) = c0; colors += stride;

	}

	// unlock vb

	m_vb->Unlock();

	D3DMATRIX dm;
	dm._12=dm._13=dm._14=0;
	dm._21=dm._23=dm._24=0;
	dm._31=dm._32=dm._34=0;
	dm._41=dm._42=dm._43=0;
	dm._11=dm._22=dm._33=dm._44=1;

	// Set world transform to identity
	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_WORLD,&dm);
	
	// Set view transform to identity
	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&dm);
	
	// Render the buffer
	Storm3D2->GetD3DDevice()->SetVertexShader(0);
	Storm3D2->GetD3DDevice()->SetFVF(FVF_VXFORMAT_PART);
	
	Storm3D2->GetD3DDevice()->SetStreamSource(0,m_vb,0,stride);
	Storm3D2->GetD3DDevice()->SetIndices(m_ib);
	
	// Render as indexed primitive
	frozenbyte::storm::validateDevice(*Storm3D2->GetD3DDevice(), Storm3D2->getLogger());
	Storm3D2->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,m_numParts * 4,0,m_numParts * 2);
	scene->AddPolyCounter(m_numParts * 2);

	++storm3d_dip_calls;
	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&mv);
}

// LineArray

void Storm3D_ParticleSystem::LineArray::releaseDynamicBuffers() {
	SAFE_RELEASE(m_vb);
}

void Storm3D_ParticleSystem::LineArray::release() {
	SAFE_RELEASE(m_vb);
	SAFE_RELEASE(m_ib);
}

void Storm3D_ParticleSystem::LineArray::createDynamicBuffers(Storm3D* Storm3D2) {

	SAFE_RELEASE(m_vb);
/*
	if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
	{
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(m_maxParticles*4*sizeof(VXFORMAT_PART),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
			D3DPOOL_DEFAULT,&m_vb, 0);
	}
	else
	{
		Storm3D2->GetD3DDevice()->CreateVertexBuffer(m_maxParticles*4*sizeof(VXFORMAT_PART),
			D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PART,
			D3DPOOL_DEFAULT,&m_vb, 0);
	}
*/	
}

void Storm3D_ParticleSystem::LineArray::init(Storm3D* Storm3D2) {
	
	if((m_totalParticles > m_maxParticles) || (m_vb == NULL)) {
		
		m_maxParticles = m_totalParticles + 10;
		
		/*
		createDynamicBuffers(Storm3D2);
		
		SAFE_RELEASE(m_ib);
		
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
		{
			Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*m_maxParticles*6,
				D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_ib, 0);
		}
		else
		{
			Storm3D2->GetD3DDevice()->CreateIndexBuffer(sizeof(WORD)*m_maxParticles*6,
				D3DUSAGE_WRITEONLY|D3DUSAGE_SOFTWAREPROCESSING,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_ib, 0);
		}
		
		// prefill ib
		WORD *ip=NULL;
		m_ib->Lock(0,sizeof(WORD)*m_maxParticles*6,(void**)&ip,0);
		if (ip)
		{
			int pos=0;
			for (int i=0;i<m_maxParticles;i++)
			{
				ip[pos+0]=i * 4 + 0;
				ip[pos+1]=i * 4 + 1;
				ip[pos+2]=i * 4 + 2;
				ip[pos+3]=i * 4 + 2;
				ip[pos+4]=i * 4 + 1;
				ip[pos+5]=i * 4 + 3;
				pos+=6;
			}
		}
		m_ib->Unlock();		
		*/
	}
	
}


//-----------------------------------------------------------------------------------

// this code is a rip off from flipcode cotd, originally
// posted by Pierre Terdiman, remember to greet!!!!!!!!!
// needed because storm3d:s line particle implementation
// didn't really work at all.

namespace line_utils
{

inline void transPoint3x3(Vector& dest, const Vector& src, const D3DXMATRIX& m) {
	
	dest.x = src.x * m.m[0][0] + src.y * m.m[1][0] + src.z * m.m[2][0];
	dest.y = src.x * m.m[0][1] + src.y * m.m[1][1] + src.z * m.m[2][1];
	dest.z = src.x * m.m[0][2] + src.y * m.m[1][2] + src.z * m.m[2][2];
	
}

inline void computePoint(Vector& dest, float x, float y, const D3DXMATRIX& rot, const Vector& trans)
{
	dest.x = trans.x + x * rot.m[0][0] + y * rot.m[1][0];
	dest.y = trans.y + x * rot.m[0][1] + y * rot.m[1][1];
	dest.z = trans.z + x * rot.m[0][2] + y * rot.m[1][2];
}

float computeConstantScale(const Vector& pos, const D3DXMATRIX& view, const D3DXMATRIX& proj)
{
	static const float mRenderWidth = 640.0f;
	
	Vector ppcam0;
	//D3DXVec3TransformCoord(&ppcam0, &pos, &view);
	transPoint3x3(ppcam0, pos, view);
	ppcam0.x += view.m[3][0];
	ppcam0.y += view.m[3][1];
	ppcam0.z += view.m[3][2];
	
	
	Vector ppcam1 = ppcam0;
	ppcam1.x += 1.0f;
	
	float l1 = 1.0f/(ppcam0.x*proj.m[0][3] + ppcam0.y*proj.m[1][3] + ppcam0.z*proj.m[2][3] + proj.m[3][3]);
	float c1 =  (ppcam0.x*proj.m[0][0] + ppcam0.y*proj.m[1][0] + ppcam0.z*proj.m[2][0] + proj.m[3][0])*l1;
	float l2 = 1.0f/(ppcam1.x*proj.m[0][3] + ppcam1.y*proj.m[1][3] + ppcam1.z*proj.m[2][3] + proj.m[3][3]);
	float c2 =  (ppcam1.x*proj.m[0][0] + ppcam1.y*proj.m[1][0] + ppcam1.z*proj.m[2][0] + proj.m[3][0])*l2;
	float CorrectScale = 1.0f/(c2 - c1);
	return (CorrectScale / mRenderWidth);
}


void computeScreenQuad(
					   const D3DXMATRIX& inverseview, const D3DXMATRIX& view, const D3DXMATRIX& proj,
					   BYTE* verts, BYTE* colors, DWORD stride, 
					   const Vector& p0, DWORD col0, float size0, 
					   const Vector& p1, DWORD col1, float size1, bool constantsize) 
{
	// Compute delta in camera space
	Vector Delta;
	transPoint3x3(Delta, p1-p0, view);
	
	// Compute size factors
	float SizeP0 = size0;
	float SizeP1 = size1;
/*	
	if(constantsize)
	{
		// Compute scales so that screen-size is constant
		SizeP0 *= computeConstantScale(p0, view, proj);
		SizeP1 *= computeConstantScale(p1, view, proj);
	}
*/	
	// Compute quad vertices
	float Theta0 = atan2f(-Delta.x, -Delta.y);
	float c0 = SizeP0 * cosf(Theta0);
	float s0 = SizeP0 * sinf(Theta0);
	computePoint(*((Vector*)verts),  c0, -s0, inverseview, p0); verts+=stride;
	computePoint(*((Vector*)verts),  -c0, s0, inverseview, p0); verts+=stride;
	
	float Theta1 = atan2f(Delta.x, Delta.y);
	float c1 = SizeP1 * cosf(Theta1);
	float s1 = SizeP1 * sinf(Theta1);
	computePoint(*((Vector*)verts),  -c1, s1, inverseview, p1); verts+=stride;
	computePoint(*((Vector*)verts),  c1, -s1, inverseview, p1); verts+=stride;

/*	
	// Output uvs if needed
	if(uvs)
	{
		*((float*)uvs) = 0.0f; *((float*)(uvs+4)) = 1.0f; uvs+=stride;
		*((float*)uvs) = 0.0f; *((float*)(uvs+4)) = 0.0f; uvs+=stride;
		*((float*)uvs) = 1.0f; *((float*)(uvs+4)) = 1.0f; uvs+=stride;
		*((float*)uvs) = 1.0f; *((float*)(uvs+4)) = 0.0f; uvs+=stride;
	}
*/
	
	// Output color if needed
	if(colors) {
		*((DWORD*)colors) = col0; colors+=stride;	
		*((DWORD*)colors) = col0; colors+=stride;	
		*((DWORD*)colors) = col1; colors+=stride;	
		*((DWORD*)colors) = col1; colors+=stride;	
	}
	
}

}


//-----------------------------------------------------------------------------------------

int Storm3D_ParticleSystem::LineArray::getMaxParticleAmount() const
{
	return m_numParts;
}

int Storm3D_ParticleSystem::LineArray::lock(VXFORMAT_PART *pointer, int particleOffset, Storm3D_Scene *scene)
{
	m_partOffset = particleOffset;
	pointer += particleOffset * 4;
	VXFORMAT_PART *vp = pointer;

	float frameWidth=0.0f;
	float frameHeight=0.0f;
	if(m_animInfo.numFrames > 1) 
	{
		frameWidth = 1.0f / m_animInfo.textureUSubDivs;
		frameHeight = 1.0f / m_animInfo.textureVSubDivs;
	}

	DWORD c0 = 0;
	DWORD c1 = 0;
	BYTE *verts = (BYTE*)&vp[0].position;
	BYTE *uvs = (BYTE*)&vp[0].texcoords;
	BYTE *colors = (BYTE*)&vp[0].color;

	DWORD stride = sizeof(VXFORMAT_PART);
	D3DXMATRIX inview, view, proj;
	view = scene->camera.GetViewMatrix();

	D3DXMatrixInverse(&inview, NULL, &view);
	proj = scene->camera.GetProjectionMatrix();

	for(int i = 0; i < m_numParts; i++) 
	{
		Storm3D_LineParticle& p = m_parts[i];
	
		c0 = (((DWORD)(p.alpha[0] * 255.0f) &0xff) << 24) |
			(((DWORD)(factor.r * p.color[0].r * 255.0f) &0xff) << 16) |
			(((DWORD)(factor.g * p.color[0].g * 255.0f) &0xff) << 8) |
			(((DWORD)(factor.b * p.color[0].b * 255.0f) &0xff) );

		c1 = (((DWORD)(p.alpha[1] * 255.0f) &0xff) << 24) |
			(((DWORD)(factor.r * p.color[1].r * 255.0f) &0xff) << 16) |
			(((DWORD)(factor.g * p.color[1].g * 255.0f) &0xff) << 8) |
			(((DWORD)(factor.b * p.color[1].b * 255.0f) &0xff) );

		
		line_utils::computeScreenQuad(inview, view, proj, verts, colors, stride,
			p.position[0], c0, p.size[0],
			p.position[1], c1, p.size[1], false);

		// Fill texturecoords
		if(m_animInfo.numFrames > 1) 
		{	
			int frame = (int)p.frame % m_animInfo.numFrames;	
			int col = frame % m_animInfo.textureUSubDivs;
			int row = frame / m_animInfo.textureUSubDivs;
			
			float tx = frameWidth * (float)col;
			float ty = frameHeight * (float)row;				
			
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);

		} 
		else 
		{
			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);	
		}
		
		verts += 4 * stride;
		colors += 4 * stride;
	}

	return m_numParts;
}

void Storm3D_ParticleSystem::LineArray::setRender(IDirect3DDevice9 &device, int &vertexOffset, int &particleAmount)
{
	if(m_numParts <= 0)
		return;

	if (m_mtl)
	{	
		static_cast<Storm3D_Material*>(m_mtl)->ApplyBaseTextureExtOnly();
	}
	else
	{
		device.SetTexture(0,NULL);
	}

	D3DMATRIX dm;
	dm._12=dm._13=dm._14=0;
	dm._21=dm._23=dm._24=0;
	dm._31=dm._32=dm._34=0;
	dm._41=dm._42=dm._43=0;
	dm._11=dm._22=dm._33=dm._44=1;

	device.SetTransform(D3DTS_WORLD,&dm);
	//device.SetTransform(D3DTS_VIEW,&dm);

	vertexOffset = m_partOffset * 4;
	particleAmount = m_numParts;
}

void Storm3D_ParticleSystem::LineArray::render(Storm3D* Storm3D2, Storm3D_Scene* scene) {
	// Not used anywhere

	if(m_numParts <= 0)
		return;
	
	scene->camera.Apply();

	if (m_mtl)
	{	
		static_cast<Storm3D_Material*>(m_mtl)->ApplyBaseTextureExtOnly();
	}
	else
	{
		Storm3D2->GetD3DDevice()->SetTexture(0,NULL);
	}

	// lock vb
	
	VXFORMAT_PART *vp;
	m_vb->Lock(0,0,(void**)&vp,D3DLOCK_DISCARD);
	if (vp==NULL) return;
	
	float frameWidth=0.0f;
	float frameHeight=0.0f;
	if(m_animInfo.numFrames > 1) {
		frameWidth = 1.0f / m_animInfo.textureUSubDivs;
		frameHeight = 1.0f / m_animInfo.textureVSubDivs;
	}
	
	DWORD c0,c1;

	BYTE* verts = (BYTE*)&vp[0].position;
	BYTE* uvs = (BYTE*)&vp[0].texcoords;
	BYTE* colors = (BYTE*)&vp[0].color;
	
	DWORD stride = sizeof(VXFORMAT_PART);

	D3DXMATRIX inview, view, proj;
	view = scene->camera.GetViewMatrix();
	
	D3DXMatrixInverse(&inview, NULL, &view);
	
	proj = scene->camera.GetProjectionMatrix();
	
	for(int i = 0; i < m_numParts; i++) {
	
		Storm3D_LineParticle& p = m_parts[i];
		
		c0 = (((DWORD)(p.alpha[0] * 255.0f) &0xff) << 24) |
			(((DWORD)(p.color[0].r * 255.0f) &0xff) << 16) |
			(((DWORD)(p.color[0].g * 255.0f) &0xff) << 8) |
			(((DWORD)(p.color[0].b * 255.0f) &0xff) );

		c1 = (((DWORD)(p.alpha[1] * 255.0f) &0xff) << 24) |
			(((DWORD)(p.color[1].r * 255.0f) &0xff) << 16) |
			(((DWORD)(p.color[1].g * 255.0f) &0xff) << 8) |
			(((DWORD)(p.color[1].b * 255.0f) &0xff) );

		
		line_utils::computeScreenQuad(inview, view, proj, verts, colors, stride,
			p.position[0], c0, p.size[0],
			p.position[1], c1, p.size[1], false);

		// Fill texturecoords
		// Fill texturecoords
		if(m_animInfo.numFrames > 1) {
			
			int frame = (int)p.frame % m_animInfo.numFrames;
			
			int col = frame % m_animInfo.textureUSubDivs;
			int row = frame / m_animInfo.textureUSubDivs;
			
			float tx = frameWidth * (float)col;
			float ty = frameHeight * (float)row;				
			
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty; uvs += (stride - 4);
			*((float*)uvs) = tx; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);
			*((float*)uvs) = tx + frameWidth; uvs += 4; *((float*)uvs) = ty + frameHeight; uvs += (stride - 4);

		} else {

			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 0.0f; uvs += (stride - 4);
			*((float*)uvs) = 0.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			*((float*)uvs) = 1.0f; uvs += 4; *((float*)uvs) = 1.0f; uvs += (stride - 4);
			
		}
		
		verts += 4 * stride;
//		uvs += 4 * stride;
		colors += 4 * stride;
		
	}

	// unlock vb
	m_vb->Unlock();

	D3DMATRIX dm;
	dm._12=dm._13=dm._14=0;
	dm._21=dm._23=dm._24=0;
	dm._31=dm._32=dm._34=0;
	dm._41=dm._42=dm._43=0;
	dm._11=dm._22=dm._33=dm._44=1;

	// Set world transform to identity
	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_WORLD,&dm);
	
	// Set view transform to identity
//	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&dm);
	
	// Render the buffer
	Storm3D2->GetD3DDevice()->SetVertexShader(0);
	Storm3D2->GetD3DDevice()->SetFVF(FVF_VXFORMAT_PART);
	
	Storm3D2->GetD3DDevice()->SetStreamSource(0,m_vb,0,stride);
	Storm3D2->GetD3DDevice()->SetIndices(m_ib);
	
	// Render as indexed primitive
	frozenbyte::storm::validateDevice(*Storm3D2->GetD3DDevice(), Storm3D2->getLogger());
	Storm3D2->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,m_numParts * 4,0,m_numParts * 2);
	scene->AddPolyCounter(m_numParts * 2);
	++storm3d_dip_calls;
}

