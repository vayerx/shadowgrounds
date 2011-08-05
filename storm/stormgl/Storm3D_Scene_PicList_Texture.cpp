// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#ifdef _MSC_VER
#include <windows.h>
#endif

#include <GL/glew.h>
#include "storm3d.h"
#include "storm3d_scene_piclist.h"
#include "storm3d_texture.h"
#include "storm3d_material.h"
#include "storm3d_scene.h"
#include "Clipper.h"
#include "igios3D.h"

#include "../../util/Debug_MemoryManager.h"

//! Constructor
Storm3D_Scene_PicList_Picture::Storm3D_Scene_PicList_Picture(Storm3D *s2, Storm3D_Scene *scene,Storm3D_Material *_mat,VC2 _position,VC2 _size,float alpha_, float rotation_, float x1_,float y1_,float x2_,float y2_, bool wrap_) 
:	Storm3D_Scene_PicList(s2,scene,_position,_size),material(_mat),alpha(alpha_),rotation(rotation_),x1(x1_),y1(y1_),x2(x2_),y2(y2_),wrap(wrap_),customShape(NULL)
{
}

//! Destructor
Storm3D_Scene_PicList_Picture::~Storm3D_Scene_PicList_Picture()
{
	if(customShape)
	{
		delete[] customShape->vertices;
		customShape->vertices = NULL;
		delete customShape;
		customShape = NULL;
	}
}

//! Create custom shape
/*!
	\param vertices vertices
	\param numVertices number of vertices
*/
void Storm3D_Scene_PicList_Picture::createCustomShape(struct VXFORMAT_2D *vertices, int numVertices)
{
	customShape = new CustomShape();
	customShape->numVertices = numVertices;
	customShape->vertices = new VXFORMAT_2D[numVertices];
	memcpy(customShape->vertices, vertices, numVertices * sizeof(VXFORMAT_2D));
}

//! Render piclist texture
void Storm3D_Scene_PicList_Picture::Render()
{
	IStorm3D_Material::ATYPE alphaType = IStorm3D_Material::ATYPE_NONE;
	if(material)
	{
		alphaType = material->GetAlphaType();
		if(alpha < 0.99f && alphaType == IStorm3D_Material::ATYPE_NONE)
			material->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);

		// Apply the texture
		material->ApplyBaseTextureExtOnly();

		if(wrap)
		{
 			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		} else {
 			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		// Animate
		{
			IStorm3D_Texture *ti = material->GetBaseTexture();
			// FIXED: crashed to null pointer here. -jpk
			if (ti != NULL)
			{
				Storm3D_Texture *t = static_cast<Storm3D_Texture *> (ti);
				t->AnimateVideo();
			}
		}
	}

	// FIXME: ugly hack to work around disappearing grenade throw meter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Create 3d-vector
	VC3 pos(position.x,position.y,0);

	// Create color (color+alpha)
	DWORD col = 0xFFFFFFFF;
	if(material)
	{
		COL c(1.f, 1.f, 1.f);
		float newAlpha = 1.f;
		c = material->GetColor();
		newAlpha = alpha * (1-material->GetTransparency());
		col=COLOR_RGBA((int)(c.r*255.0f),(int)(c.g*255.0f),(int)(c.b*255.0f),(int)(newAlpha*255.0f));
		//col = COLOR_RGBA(c.r, c.g, c.b, newAlpha);
	}

	// Render it
	frozenbyte::storm::VertexShader::disable();

	// render with custom shape
	if(customShape && customShape->vertices)
	{
		// use combined alpha
		float alpha_mul = 1.0f;
		if(material)
		{
			alpha_mul = alpha * (1.0f - material->GetTransparency());
		}
		for(int i = 0; i < customShape->numVertices; i++)
		{
			DWORD c = customShape->vertices[i].color;
			int newAlpha = (int)((c >> 24) * alpha_mul);
			c &= 0x00FFFFFF;
			c |= (newAlpha & 0xFF) << 24;
			customShape->vertices[i].color = c;
			customShape->vertices[i].position.x -= .5f;
			customShape->vertices[i].position.y -= .5f;
		}
		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(VXFORMAT_2D), &customShape->vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VXFORMAT_2D), &customShape->vertices[0].color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), &customShape->vertices[0].texcoords);

		glDrawArrays(GL_TRIANGLES, 0, customShape->numVertices);
		scene->AddPolyCounter(customShape->numVertices/3);
	}
	// render quad
	else
	{
		VC2 p[4];
		p[0] = VC2(x1, y2);
		p[1] = VC2(x1, y1);
		p[2] = VC2(x2, y2);
		p[3] = VC2(x2, y1);

		float xc = .5f; //(x2 - x1) * .5f;
		float yc = .5f; //(y2 - y1) * .5f;

		if(fabsf(rotation) > 0.001f)
		{
			for(unsigned int i = 0; i < 4; ++i)
			{
				float x = p[i].x - xc;
				float y = p[i].y - yc;

				p[i].x =  x * cosf(rotation) + y * sinf(rotation);
				p[i].y = -x * sinf(rotation) + y * cosf(rotation);

				p[i].x += xc;
				p[i].y += yc;
			}
		}

		// Create a quad
		VXFORMAT_2D vx[4];
		vx[0]=VXFORMAT_2D(pos + VC3(0, size.y, 0),      1, col, p[0]);

		vx[1]=VXFORMAT_2D(pos,                          1, col, p[1]);
		vx[2]=VXFORMAT_2D(pos + VC3(size.x, size.y, 0), 1, col, p[2]);

		vx[3]=VXFORMAT_2D(pos + VC3(size.x, 0, 0),      1, col, p[3]);

		for(int i = 0; i < 4; ++i)
		{
			vx[i].position.x -= .5f;
			vx[i].position.y -= .5f;
		}

		glDisable(GL_CULL_FACE);

		// Render it
		frozenbyte::storm::PixelShader::disable();
		frozenbyte::storm::VertexShader::disable();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		for (unsigned int i = 1; i < 7; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(VXFORMAT_2D), &vx[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VXFORMAT_2D), &vx[0].color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), &vx[0].texcoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		scene->AddPolyCounter(2);

	}

	if(material)
		material->SetAlphaType(alphaType);

	if(wrap && material)
	{
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
}

//! Constructor
Storm3D_Scene_PicList_Picture3D::Storm3D_Scene_PicList_Picture3D(Storm3D *s2,
		Storm3D_Scene *scene,Storm3D_Material *_mat,VC3 _position,VC2 _size) :
	Storm3D_Scene_PicList(s2,scene,_position,_size),material(_mat)
{
}

//! Render piclist 3D picture
void Storm3D_Scene_PicList_Picture3D::Render()
{
	// Calculate sprites position on screen
	VC3 scpos;
	float w,rz;

	// Calculate position, and render sprite only if it's visible
	if (scene->camera.GetTransformedToScreen(position,scpos,w,rz))
	if (rz<scene->camera.vis_range)
	{
		// VC3 size
		size/=rz;

		// Apply the texture
		material->ApplyBaseTextureExtOnly();

		// Get viewport size
		Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();

		// Create 3d-vector
		VC3 pos=VC3(scpos.x*ss.width,scpos.y*ss.height,scpos.z);
		size.x*=ss.width;
		size.y*=ss.height;

		// Create color (color+alpha)
		COL c=material->GetColor();
		float newAlpha = (1-material->GetTransparency());
		DWORD col=COLOR_RGBA((int)(c.r*255.0f),(int)(c.g*255.0f),(int)(c.b*255.0f),(int)(newAlpha*255.0f));
		//DWORD col=COLOR_RGBA(c.r,c.g,c.b,newAlpha);
		
		// Create a quad
		float hsx=size.x*0.5f;
		float hsy=size.y*0.5f;
		VXFORMAT_2D vx[4];
		vx[0]=VXFORMAT_2D(pos+VC3(-hsx,hsy,0),1,col,VC2(0,1));
		vx[1]=VXFORMAT_2D(pos+VC3(-hsx,-hsy,0),1,col,VC2(0,0));
		vx[2]=VXFORMAT_2D(pos+VC3(hsx,hsy,0),1,col,VC2(1,1));
		vx[3]=VXFORMAT_2D(pos+VC3(hsx,-hsy,0),1,col,VC2(1,0));

		if (Clip2DRectangle(Storm3D2,vx[1],vx[2])) 
		{
			// Copy clipping
			vx[0].position.x=vx[1].position.x;
			vx[0].texcoords.x=vx[1].texcoords.x;
			vx[3].position.y=vx[1].position.y;
			vx[3].texcoords.y=vx[1].texcoords.y;
			vx[0].position.y=vx[2].position.y;
			vx[0].texcoords.y=vx[2].texcoords.y;
			vx[3].position.x=vx[2].position.x;
			vx[3].texcoords.x=vx[2].texcoords.x;

			// Render it (with Z buffer read)
			glEnable(GL_DEPTH_TEST);
			frozenbyte::storm::VertexShader::disable();
			renderUP(FVF_VXFORMAT_2D, GL_TRIANGLE_STRIP, 2, sizeof(VXFORMAT_2D), (char *) vx);
			scene->AddPolyCounter(2);
			glDisable(GL_DEPTH_TEST);
		}
	}
}
