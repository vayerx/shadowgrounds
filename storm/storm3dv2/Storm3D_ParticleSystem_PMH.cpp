// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_scene.h"
#include "storm3d_particle.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_material.h"
#include "VertexFormats.h"

#include "..\..\util\Debug_MemoryManager.h"
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









/*
//------------------------------------------------------------------
// Storm3D_ParticleSystem_PMH::Storm3D_ParticleSystem_PMH
//------------------------------------------------------------------
//Storm3D_ParticleSystem_PMH::Storm3D_ParticleSystem_PMH(Storm3D *s2,Storm3D_Material *mat) :
Storm3D_ParticleSystem_PMH::Storm3D_ParticleSystem_PMH(Storm3D *s2,Storm3D_Material *mat,Storm3D_PointParticle *parts,int array_size,
													   Storm3D_ParticleAnimationInfo* ai) :
	Storm3D2(s2),
	material(mat),
	partptr_points(parts),
	partptr_lines(NULL),
	//num_points(array_size),
	//num_lines(0)
	num_parts(array_size)
{
		animInfo.frames = 0;
		if(ai) {
			memcpy(&animInfo, ai, sizeof(animInfo));
		}
}


Storm3D_ParticleSystem_PMH::Storm3D_ParticleSystem_PMH(Storm3D *s2,Storm3D_Material *mat,Storm3D_LineParticle *parts,int array_size,
													   Storm3D_ParticleAnimationInfo* ai) :
	Storm3D2(s2),
	material(mat),
	partptr_points(NULL),
	partptr_lines(parts),
	//num_points(0),
	//num_lines(array_size)
	num_parts(array_size)
{
		animInfo.frames = 0;
		if(ai) {
			memcpy(&animInfo, ai, sizeof(animInfo));
		}
}



//------------------------------------------------------------------
// Storm3D_ParticleSystem_PMH::~Storm3D_ParticleSystem_PMH
//------------------------------------------------------------------
Storm3D_ParticleSystem_PMH::~Storm3D_ParticleSystem_PMH()
{
}




//------------------------------------------------------------------
// Storm3D_ParticleSystem_PMH::Render
// Test version that uses vertex shader, the overhead by
// particle systems is so small atlead compared to collision detection and
// physics that i think there is no need to use this HACK!!!!
// - absu
//------------------------------------------------------------------

/*
void Storm3D_ParticleSystem_PMH::Render(Storm3D_Scene *scene,Storm3D_ParticleSystem *pts)
{
	// Set material active...
	// Use only base texture, and no other parameters
	if (material)
	{	
		material->ApplyBaseTextureExtOnly();
	}
	else
	{
		Storm3D2->GetD3DDevice()->SetTexture(0,NULL);
	}

	// Render the list
	//particlelist.Render(scene,ismain,timedif);

	// [v2.3 new stuff]
	
	// Lock vertex buffer
	BYTE *vp;
	int c = 0;
	pts->dx8_vbuf->Lock(0,0,(BYTE**)&vp,D3DLOCK_DISCARD);
	if (vp==NULL) return;

	QuadParticleVertex *mesh=(QuadParticleVertex*)vp;

	if (partptr_points)
	{
		for(int i = 0; i < num_parts; i++) {
			
			// this unrolled copy may look like a monster but I think 
			// it should be a lot faster than actually doing the math here.
			
			mesh[c+0].px = partptr_points[i].center.position.x;
			mesh[c+0].py = partptr_points[i].center.position.y;
			mesh[c+0].pz = partptr_points[i].center.position.z;
			mesh[c+1].px = partptr_points[i].center.position.x;
			mesh[c+1].py = partptr_points[i].center.position.y;
			mesh[c+1].pz = partptr_points[i].center.position.z;
			mesh[c+2].px = partptr_points[i].center.position.x;
			mesh[c+2].py = partptr_points[i].center.position.y;
			mesh[c+2].pz = partptr_points[i].center.position.z;
			mesh[c+3].px = partptr_points[i].center.position.x;
			mesh[c+3].py = partptr_points[i].center.position.y;
			mesh[c+3].pz = partptr_points[i].center.position.z;
			float ca = cos(partptr_points[i].angle);
			float sa = sin(partptr_points[i].angle);
			mesh[c+0].cosAngle = ca;
			mesh[c+0].sinAngle = sa;
			mesh[c+1].cosAngle = ca;
			mesh[c+1].sinAngle = sa;
			mesh[c+2].cosAngle = ca;
			mesh[c+2].sinAngle = sa;
			mesh[c+3].cosAngle = ca;
			mesh[c+3].sinAngle = sa;			
			mesh[c+0].size = partptr_points[i].center.size;
			mesh[c+1].size = partptr_points[i].center.size;
			mesh[c+2].size = partptr_points[i].center.size;
			mesh[c+3].size = partptr_points[i].center.size;
			mesh[c+0].zero = 0.0f;
			mesh[c+1].zero = 0.0f;
			mesh[c+2].zero = 0.0f;
			mesh[c+3].zero = 0.0f;
			mesh[c+0].frame = (float)(partptr_points[i].frame % 32);
			mesh[c+1].frame = (float)(partptr_points[i].frame % 32);
			mesh[c+2].frame = (float)(partptr_points[i].frame % 32);
			mesh[c+3].frame = (float)(partptr_points[i].frame % 32);
			
			mesh[c+0].r = partptr_points[i].center.color.r;
			mesh[c+0].g = partptr_points[i].center.color.g;
			mesh[c+0].b = partptr_points[i].center.color.b;
			mesh[c+0].a = partptr_points[i].center.alpha;
			mesh[c+1].r = partptr_points[i].center.color.r;
			mesh[c+1].g = partptr_points[i].center.color.g;
			mesh[c+1].b = partptr_points[i].center.color.b;
			mesh[c+1].a = partptr_points[i].center.alpha;			
			mesh[c+2].r = partptr_points[i].center.color.r;
			mesh[c+2].g = partptr_points[i].center.color.g;
			mesh[c+2].b = partptr_points[i].center.color.b;
			mesh[c+2].a = partptr_points[i].center.alpha;
			mesh[c+3].r = partptr_points[i].center.color.r;
			mesh[c+3].g = partptr_points[i].center.color.g;
			mesh[c+3].b = partptr_points[i].center.color.b;
			mesh[c+3].a = partptr_points[i].center.alpha;
			
/*			mesh[c+0].r = 1.0f;
			mesh[c+0].g = 1.0f;
			mesh[c+0].b = 1.0f;
			mesh[c+0].a = 1.0f;

			mesh[c+1].r = 1.0f;
			mesh[c+1].g = 1.0f;
			mesh[c+1].b = 1.0f;
			mesh[c+1].a = 1.0f;
			
			mesh[c+2].r = 1.0f;
			mesh[c+2].g = 1.0f;
			mesh[c+2].b = 1.0f;
			mesh[c+2].a = 1.0f;

			mesh[c+3].r = 1.0f;
			mesh[c+3].g = 1.0f;
			mesh[c+3].b = 1.0f;
			mesh[c+3].a = 1.0f;
*/
			// these offsets are actually only thing that change per vertex
/*			
			mesh[c+0].scrX = 0.5f; 
			mesh[c+0].scrY = -0.5f;

			mesh[c+1].scrX = -0.5f; 
			mesh[c+1].scrY = -0.5f;

			mesh[c+2].scrX = -0.5f; 
			mesh[c+2].scrY = 0.5f;

			mesh[c+3].scrX = 0.5f; 
			mesh[c+3].scrY = 0.5f;

/*			mesh[c+0].scrX = 1.0f; 
			mesh[c+0].scrY = 1.0f;

			mesh[c+1].scrX = 0.0f; 
			mesh[c+1].scrY = 1.0f;

			mesh[c+2].scrX = 0.0f; 
			mesh[c+2].scrY = 0.0f;

			mesh[c+3].scrX = 1.0f; 
			mesh[c+3].scrY = 0.0f;
*/
/*			
			c += 4;
		}
	}

	pts->dx8_vbuf->Unlock();

	if (num_parts>0)
	{		
		// Render the buffer		
		Storm3D2->GetD3DDevice()->SetStreamSource(0,pts->dx8_vbuf,sizeof(QuadParticleVertex));
		Storm3D2->GetD3DDevice()->SetIndices(pts->dx8_ibuf,0);
//		Storm3D_ShaderManager::GetSingleton()->SetParticleShader(Storm3D2->GetD3DDevice(), 
//			16, 2, 0);

		// Render as indexed primitive
		Storm3D2->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,c,0,c/2);
		scene->AddPolyCounter(c/2);
	}

}
*/

/*
inline void  __buaha_rotate(float& x1, float& y1, float tx, float ty, float ca, float sa) {
	x1 = ca * tx - sa * ty;
	y1 = sa * tx + ca * ty;
}

	

void Storm3D_ParticleSystem_PMH::Render(Storm3D_Scene *scene,Storm3D_ParticleSystem *pts)
{
	// Set material active...
	// Use only base texture, and no other parameters
	if (material)
	{	
		material->ApplyBaseTextureExtOnly();
	}
	else
	{
		Storm3D2->GetD3DDevice()->SetTexture(0,NULL);
	}

	// Render the list
	//particlelist.Render(scene,ismain,timedif);

	// [v2.3 new stuff]

//	Matrix viewMatrix;
	D3DXMATRIX mv;

	// Lock vertex buffer
	BYTE *vp;
	pts->dx8_vbuf->Lock(0,0,(BYTE**)&vp,D3DLOCK_DISCARD);
	if (vp==NULL) return;

	// Typecast (to simplify code)
	VXFORMAT_PART *mesh=(VXFORMAT_PART*)vp;
	int mp=0;	// Vertex counter

	float frameWidth=0.0f;
	float frameHeight=0.0f;
	if(animInfo.frames > 0) {
		frameWidth = 1.0f / animInfo.columns;
		frameHeight = 1.0f / animInfo.rows;
	}



	// line particles didn't even work anyway...
	
	if (partptr_lines)
	{
		//num_parts=num_lines;

		// Fill buffer
		VC3 campos=scene->GetCamera()->GetPosition();
		for (int i=0;i<num_parts;i++)
		if (partptr_lines[i].alive)
		{
			// Precalc
			DWORD col1=partptr_lines[i].start.color.GetAsD3DCompatibleARGB();
			DWORD col2=partptr_lines[i].end.color.GetAsD3DCompatibleARGB();
			col1=(DWORD)(col1&0x00ffffff+((((DWORD)(partptr_lines[i].start.alpha*255))&0xff)<<24));
			col2=(DWORD)(col2&0x00ffffff+((((DWORD)(partptr_lines[i].end.alpha*255))&0xff)<<24));

			// Fill colors
			mesh[mp+0].color=col1;
			mesh[mp+1].color=col2;
			mesh[mp+2].color=col2;
			mesh[mp+3].color=col1;
			
			// Fill texturecoords
			float tx = 0.0f;
			float ty = 0.0f;
			if(animInfo.frames > 0) {
								
				int frame = partptr_lines[i].frame % animInfo.frames;
				
				int col = frame % animInfo.columns;
				int row = frame / animInfo.columns;
				
				float tx = frameWidth * (float)col;
				float ty = frameHeight * (float)row;				

				mesh[mp+0].texcoords.x=tx+frameWidth;
				mesh[mp+0].texcoords.y=ty+frameHeight;
				mesh[mp+1].texcoords.x=tx;
				mesh[mp+1].texcoords.y=ty+frameHeight;
				mesh[mp+2].texcoords.x=tx;
				mesh[mp+2].texcoords.y=ty;
				mesh[mp+3].texcoords.x=tx+frameWidth;
				mesh[mp+3].texcoords.y=ty;

			} else {

				mesh[mp+0].texcoords.x=1;
				mesh[mp+0].texcoords.y=1;
				mesh[mp+1].texcoords.x=0;
				mesh[mp+1].texcoords.y=1;
				mesh[mp+2].texcoords.x=0;
				mesh[mp+2].texcoords.y=0;
				mesh[mp+3].texcoords.x=1;
				mesh[mp+3].texcoords.y=0;

			}
			
			// Calc updown
			VC3 updown=(partptr_lines[i].end.position-
				partptr_lines[i].start.position).GetCrossWith(
				partptr_lines[i].start.position-campos);
			updown.Normalize();

			
			// Fill positions
			float hsize1=partptr_lines[i].start.size*0.5f;
			float hsize2=partptr_lines[i].end.size*0.5f;
			VC3 ud1=updown*hsize1;
			VC3 ud2=updown*hsize2;
			mesh[mp+3].position=partptr_lines[i].start.position+ud1;
			mesh[mp+0].position=partptr_lines[i].end.position+ud2;
			mesh[mp+1].position=partptr_lines[i].end.position-ud2;
			mesh[mp+2].position=partptr_lines[i].start.position-ud1;

			// Add position
			mp+=4;
		}
	}
		
	else if (partptr_points)
	{
		
		//num_parts=num_points;
/*
		// Calc vectors
		VC3 updown;
		VC3 side;
		VC3 dir = scene->GetCamera()->GetDirection().GetNormalized();
		updown=scene->GetCamera()->GetUpVecReal();
		side=updown.GetCrossWith(dir);
*/
/*		
		
		//viewMatrix.CreateCameraMatrix(scene->GetCamera()->GetPosition(),
		//	scene->GetCamera()->GetTarget(), scene->GetCamera()->GetUpVec());
		

		// get transform doesn't work on pure device... 
		Storm3D2->GetD3DDevice()->GetTransform(D3DTS_VIEW, &mv);
		
	    // so lets contruct our matrix from scratch ;)
		//D3DXMatrixLookAtLH(&mv,(D3DXVECTOR3*)&scene->GetCamera()->GetPosition(),
		//	(D3DXVECTOR3*)&scene->GetCamera()->GetTarget(),(D3DXVECTOR3*)&scene->GetCamera()->GetUpVec());

		// Fill buffer
		for (int i=0;i<num_parts;i++)
		if (partptr_points[i].alive)
		{
			// Precalc
			DWORD col=partptr_points[i].center.color.GetAsD3DCompatibleARGB();
			col=(DWORD)(col&0x00ffffff+((((DWORD)(partptr_points[i].center.alpha*255))&0xff)<<24));

			// Fill colors
			mesh[mp+0].color=col;
			mesh[mp+1].color=col;
			mesh[mp+2].color=col;
			mesh[mp+3].color=col;
			
			// Fill texturecoords
			float tx = 0.0f;
			float ty = 0.0f;
			if(animInfo.frames > 0) {
				
				int frame = partptr_points[i].frame % animInfo.frames;
				
				int col = frame % animInfo.columns;
				int row = frame / animInfo.columns;
				
				float tx = frameWidth * (float)col;
				float ty = frameHeight * (float)row;				

				mesh[mp+0].texcoords.x=tx+frameWidth;
				mesh[mp+0].texcoords.y=ty+frameHeight;
				mesh[mp+1].texcoords.x=tx;
				mesh[mp+1].texcoords.y=ty+frameHeight;
				mesh[mp+2].texcoords.x=tx;
				mesh[mp+2].texcoords.y=ty;
				mesh[mp+3].texcoords.x=tx+frameWidth;
				mesh[mp+3].texcoords.y=ty;
			} else {
				mesh[mp+0].texcoords.x=1;
				mesh[mp+0].texcoords.y=1;
				mesh[mp+1].texcoords.x=0;
				mesh[mp+1].texcoords.y=1;
				mesh[mp+2].texcoords.x=0;
				mesh[mp+2].texcoords.y=0;
				mesh[mp+3].texcoords.x=1;
				mesh[mp+3].texcoords.y=0;
			}

			// Fill positions
			float sa = sin(partptr_points[i].angle);
			float ca = cos(partptr_points[i].angle);
			float hsize = 0.5f*partptr_points[i].center.size;
			
			float x1,y1,x2,y2,x3,y3,x4,y4;

			__buaha_rotate(x1, y1,  hsize,-hsize, ca, sa);
			__buaha_rotate(x2, y2, -hsize,-hsize, ca, sa);
			__buaha_rotate(x3, y3, -hsize, hsize, ca, sa);
			__buaha_rotate(x4, y4,  hsize, hsize, ca, sa);

			VC3 p = partptr_points[i].center.position;
			
			p.x = partptr_points[i].center.position.x * mv.m[0][0] + 
				partptr_points[i].center.position.y * mv.m[1][0] + 
				partptr_points[i].center.position.z * mv.m[2][0] + mv.m[3][0];
		

			p.y = partptr_points[i].center.position.x * mv.m[0][1] + 
				partptr_points[i].center.position.y * mv.m[1][1] + 
				partptr_points[i].center.position.z * mv.m[2][1] + mv.m[3][1];

			p.z = partptr_points[i].center.position.x * mv.m[0][2] + 
				partptr_points[i].center.position.y * mv.m[1][2] + 
				partptr_points[i].center.position.z * mv.m[2][2] + mv.m[3][2];

			VC3 v1(x1, y1, 0.0f);
			VC3 v2(x2, y2, 0.0f);
			VC3 v3(x3, y3, 0.0f);
			VC3 v4(x4, y4, 0.0f);

			mesh[mp+0].position=p + v1;
			mesh[mp+1].position=p + v2;
			mesh[mp+2].position=p + v3;
			mesh[mp+3].position=p + v4;

			// Add position
			mp+=4;
		}
	}

	// Unlock vertex buffer
	pts->dx8_vbuf->Unlock();

	if (num_parts>0)
	{
		// Set world transform (identity)
		D3DMATRIX dm;
        dm._12=dm._13=dm._14=0;
        dm._21=dm._23=dm._24=0;
        dm._31=dm._32=dm._34=0;
        dm._41=dm._42=dm._43=0;
        dm._11=dm._22=dm._33=dm._44=1;
		Storm3D2->GetD3DDevice()->SetTransform(D3DTS_WORLD,&dm);
		
		if(partptr_points)
			Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&dm);
		
		// Render the buffer
		Storm3D2->GetD3DDevice()->SetVertexShader(FVF_VXFORMAT_PART);
		Storm3D2->GetD3DDevice()->SetStreamSource(0,pts->dx8_vbuf,sizeof(VXFORMAT_PART));
		Storm3D2->GetD3DDevice()->SetIndices(pts->dx8_ibuf,0);

		// Render as indexed primitive
		Storm3D2->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,mp,0,mp/2);
		scene->AddPolyCounter(mp/2);

		if(partptr_points)
			Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&mv);	
	}
}


*/