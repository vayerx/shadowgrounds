// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include "Storm3D_Line.h"
#include "storm3d.h"
#include <cassert>

#include "Storm3D_ShaderManager.h"
#include "VertexFormats.h"
#include "igios3D.h"
#include "../../util/Debug_MemoryManager.h"

extern int storm3d_dip_calls;

//! Constructor
Storm3D_Line::Storm3D_Line(Storm3D *storm_)
:	storm(storm_),
	index_buffer(0),
	vertex_buffer(0),
	vertex_buffer2(0),
	pixel_line(false),
	points(),
	thickness(0.5f),
	color(RGBI(255,255,255)),
	rebuild_indices(true),
	rebuild_vertices(true)
{
}

//! Destructor
Storm3D_Line::~Storm3D_Line()
{
	if(storm)
		storm->lines.erase(this);

	if(glIsBuffer(index_buffer))
		glDeleteBuffers(1, &index_buffer);
	if(glIsBuffer(vertex_buffer))
		glDeleteBuffers(1, &vertex_buffer);
	if(glIsBuffer(vertex_buffer2))
		glDeleteBuffers(1, &vertex_buffer2);
}

//! Add new point to line
/*!
	\param position point position
*/
void Storm3D_Line::AddPoint(const Vector &position)
{
	points.push_back(position);

	rebuild_indices = true;
	rebuild_vertices = true;
}

//! Get number of points in line
/*!
	\return number of points
*/
int Storm3D_Line::GetPointCount()
{
	return points.size();
}

//! Remove point from line
/*!
	\param index point to remove
*/
void Storm3D_Line::RemovePoint(int index)
{
	assert((index >= 0) && (index < static_cast<int> (points.size()) ));
	points.erase(points.begin() + index);

	rebuild_indices = true;
	rebuild_vertices = true;
}

//! Set thickness of line
/*!
	\param thickness thickness value
*/
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

//! Set color of line
/*!
	\param color color
*/
void Storm3D_Line::SetColor(int color)
{
	this->color = color;
	rebuild_indices = true;
	rebuild_vertices = true;
}

//! Render line
void Storm3D_Line::Render()
{
	if(points.size() < 2)
		return;
	
	int faces = (points.size() - 1) * 2;
	int vertices = (points.size() - 1) * 4;

	// Create buffers when needed
	if(!glIsBuffer(vertex_buffer))
	{
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices*sizeof(VXFORMAT_PSD), NULL, GL_DYNAMIC_DRAW);
	}
	if(!glIsBuffer(index_buffer))
	{
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*faces*3, NULL, GL_DYNAMIC_DRAW);
	}
	if(!glIsBuffer(vertex_buffer2))
	{
		glGenBuffers(1, &vertex_buffer2);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);
		glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(VXFORMAT_PSD), NULL, GL_DYNAMIC_DRAW);
	}

	// Update as needed
	if(rebuild_vertices == true)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		void *vp = reinterpret_cast<void *> (glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

		VXFORMAT_PSD *p=(VXFORMAT_PSD*)vp;

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

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	if(rebuild_indices == true)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		GLushort *ip = reinterpret_cast<unsigned short *> (glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

		for(int i = 0; i < static_cast<int> (points.size() - 1); ++i)
		{
			// First face
			*ip++ = i*4 + 0;
			*ip++ = i*4 + 2;
			*ip++ = i*4 + 1;

			// Second face
			*ip++ = i*4 + 1;
			*ip++ = i*4 + 2;
			*ip++ = i*4 + 3;
		}

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}
	if(rebuild_vertices == true)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);
		void *vp = reinterpret_cast<void *> (glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

		VXFORMAT_PSD *p=(VXFORMAT_PSD*)vp;
		for(unsigned int i=0;i<points.size();i++)
		{
			p[i].color = color;
			p[i].position = points[i];
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	rebuild_vertices = false;
	rebuild_indices = false;

	frozenbyte::storm::VertexShader::disable();

	// Render
	if(!pixel_line)
	{
		applyFVF(FVF_VXFORMAT_PSD, sizeof(VXFORMAT_PSD));
		setStreamSource(0, vertex_buffer, 0, sizeof(VXFORMAT_PSD));
		glDrawRangeElements(GL_TRIANGLES, 0, vertices, faces*3, GL_UNSIGNED_SHORT, NULL);

		++storm3d_dip_calls;
	}
	else
	{
		applyFVF(FVF_VXFORMAT_PSD, sizeof(VXFORMAT_PSD));
		setStreamSource(0, vertex_buffer2, 0, sizeof(VXFORMAT_PSD));
		glDrawArrays(GL_LINES, 0, points.size());

		++storm3d_dip_calls;
	}
}

//! Release dynamic resources
void Storm3D_Line::releaseDynamicResources()
{
	if(glIsBuffer(index_buffer))
		glDeleteBuffers(1, &index_buffer);
	if(glIsBuffer(vertex_buffer))
		glDeleteBuffers(1, &vertex_buffer);
	if(glIsBuffer(vertex_buffer2))
		glDeleteBuffers(1, &vertex_buffer2);
}

//! Recreate dynamic resources
void Storm3D_Line::recreateDynamicResources()
{
}
