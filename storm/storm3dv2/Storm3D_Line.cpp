// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include "Storm3D_Line.h"
#include "storm3d.h"
#include <cassert>

#include "Storm3D_ShaderManager.h"
#include "VertexFormats.h"
#include "../../util/Debug_MemoryManager.h"

extern int storm3d_dip_calls;

Storm3D_Line::Storm3D_Line(Storm3D *storm_)
:	storm(storm_)
{
	vertex_buffer = 0;
	vertex_buffer2 = 0;
	index_buffer = 0;

	thickness = 0.5f;
	color = RGB(255,255,255);

	rebuild_indices = true;
	rebuild_vertices = true;
	pixel_line = false;
}

Storm3D_Line::~Storm3D_Line()
{
	if(storm)
		storm->lines.erase(this);

	if(index_buffer)
		index_buffer->Release();
	if(vertex_buffer)
		vertex_buffer->Release();
	if(vertex_buffer2)
		vertex_buffer2->Release();
}

void Storm3D_Line::AddPoint(const Vector &position)
{
	points.push_back(position);

	rebuild_indices = true;
	rebuild_vertices = true;
}

int Storm3D_Line::GetPointCount()
{
	return points.size();
}

void Storm3D_Line::RemovePoint(int index)
{
	assert((index >= 0) && (index < static_cast<int> (points.size()) ));
	points.erase(points.begin() + index);

	rebuild_indices = true;
	rebuild_vertices = true;
}
	
void Storm3D_Line::SetThickness(float thickness)
{
	this->thickness = thickness;
	if(thickness < 0)
		pixel_line = true;
	else
		pixel_line = false;

	rebuild_indices = true;
	rebuild_vertices = true;
}

void Storm3D_Line::SetColor(int color)
{
	this->color = color;
	rebuild_indices = true;
	rebuild_vertices = true;
}

void Storm3D_Line::Render(IDirect3DDevice9 *device)
{
	if(points.size() < 2)
		return;
	
	int faces = (points.size() - 1) * 2;
	int vertices = (points.size() - 1) * 4;

	// Create buffers when needed
	if(vertex_buffer == 0)
	{
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == true)
			device->CreateVertexBuffer( vertices*sizeof(VXFORMAT_PSD), D3DUSAGE_SOFTWAREPROCESSING| D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, FVF_VXFORMAT_PSD, D3DPOOL_DEFAULT, &vertex_buffer, 0);
		else
			device->CreateVertexBuffer( vertices*sizeof(VXFORMAT_PSD), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PSD, D3DPOOL_DEFAULT, &vertex_buffer, 0);
	}
	if(index_buffer == 0)
	{
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == true)
			device->CreateIndexBuffer(sizeof(WORD)*faces*3, D3DUSAGE_WRITEONLY|D3DUSAGE_SOFTWAREPROCESSING|D3DUSAGE_DYNAMIC,D3DFMT_INDEX16,D3DPOOL_DEFAULT, &index_buffer, 0);
		else
			device->CreateIndexBuffer(sizeof(WORD)*faces*3, D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,D3DFMT_INDEX16,D3DPOOL_DEFAULT, &index_buffer, 0);
	}
	if(vertex_buffer2 == 0)
	{
		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == true)
			device->CreateVertexBuffer( points.size()*sizeof(VXFORMAT_PSD), D3DUSAGE_SOFTWAREPROCESSING| D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, FVF_VXFORMAT_PSD, D3DPOOL_DEFAULT, &vertex_buffer2, 0);
		else
			device->CreateVertexBuffer( points.size()*sizeof(VXFORMAT_PSD), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,FVF_VXFORMAT_PSD, D3DPOOL_DEFAULT, &vertex_buffer2, 0);
	}

	// Update as needed
	if(rebuild_vertices == true)
	{
		void *vp = 0;
		vertex_buffer->Lock(0,0,&vp,D3DLOCK_DISCARD);

		VXFORMAT_PSD *p= (VXFORMAT_PSD*) vp;

		// last corner points (next part will be connected to these)
		// -jpk
		VXFORMAT_PSD *lastp1 = NULL;
		VXFORMAT_PSD *lastp2 = NULL;

		for(unsigned int i = 0; i < points.size() - 1; ++i)
		{
			Vector direction = points[i];
			direction -= points[i + 1];

			if (fabs(direction.x) < 0.0001f 
				&& fabs(direction.y) < 0.0001f 
				&& fabs(direction.z) < 0.0001f)
			{
				// to avoid division by zero
				direction.x = 0.1f;
			}

			direction.Normalize();
			Vector side = direction.GetCrossWith(Vector(0,1,0));

			p->color = color;
			if (i > 1)
				p->position = lastp1->position;
			else
				p->position = points[i] + (side * .5f * thickness);				
			++p;

			p->color = color;
			if (i > 1)
				p->position = lastp2->position;
			else
				p->position = points[i] - (side * .5f * thickness);
			++p;

			p->color = color;
			p->position = points[i+1] + (side * .5f * thickness) + (direction * .5f * -thickness);
			lastp1 = p;
			++p;

			p->color = color;
			p->position = points[i+1] - (side * .5f * thickness) + (direction * .5f * -thickness);
			lastp2 = p;
			++p;
		}
					
		vertex_buffer->Unlock();
	}
	if(rebuild_indices == true)
	{
		WORD *ip = 0;
		index_buffer->Lock(0, sizeof(WORD)*faces*3, (void**)& ip, D3DLOCK_DISCARD);

		for(int i = 0; i < static_cast<int> (points.size() - 1); ++i)
		{
			// First face
			*ip++ = i*4 + 0;
			*ip++ = i*4 + 1;
			*ip++ = i*4 + 2;

			// Second face
			*ip++ = i*4 + 1;
			*ip++ = i*4 + 3;
			*ip++ = i*4 + 2;
		}
		
		index_buffer->Unlock();
	}
	if(rebuild_vertices == true)
	{
		void *vp = 0;
		vertex_buffer2->Lock(0,0,&vp,D3DLOCK_DISCARD);

		VXFORMAT_PSD *p=(VXFORMAT_PSD*)vp;
		for(unsigned int i=0;i<points.size();i++)
		{
			p[i].color = color;
			p[i].position = points[i];
		}
					
		vertex_buffer2->Unlock();
	}

	rebuild_vertices = false;
	rebuild_indices = false;

	device->SetVertexShader(0);
	device->SetFVF(FVF_VXFORMAT_PSD);

	// Render
	if(!pixel_line)
	{
		device->SetStreamSource(0, vertex_buffer, 0, sizeof(VXFORMAT_PSD));
		device->SetIndices(index_buffer);

		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertices, 0, faces);
		++storm3d_dip_calls;
	}
	else
	{
		device->SetStreamSource(0, vertex_buffer2, 0, sizeof(VXFORMAT_PSD));
		device->DrawPrimitive(D3DPT_LINELIST, 0, points.size() / 2);
		++storm3d_dip_calls;
	}
}

void Storm3D_Line::releaseDynamicResources()
{
	if(index_buffer)
		index_buffer->Release();
	if(vertex_buffer)
		vertex_buffer->Release();
	if(vertex_buffer2)
		vertex_buffer2->Release();
}

void Storm3D_Line::recreateDynamicResources()
{
}
