// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <boost/scoped_array.hpp>
#include "storm3d.h"
#include "storm3d_scene_piclist.h"
#include "storm3d_texture.h"
#include "storm3d_font.h"
#include "Clipper.h"
#include "igios3D.h"

#include "../../util/Debug_MemoryManager.h"


//! Constructor
Storm3D_Scene_PicList_Font::Storm3D_Scene_PicList_Font(Storm3D *s2,
		Storm3D_Scene *scene,Storm3D_Font *_font,VC2 _position,VC2 _size,const char *_text,float alpha_,const COL &colorFactor_) :
	Storm3D_Scene_PicList(s2,scene,_position,_size),
	font(_font),
	text(new char[strlen(_text)+1]),
	uniText(0),
	alpha(alpha_),
	colorFactor(colorFactor_),
	width(0),
	height(0),
	wd(0),
	hd(0)
{
	// Copy text
	strcpy(text,_text);
}

//! Constructor
Storm3D_Scene_PicList_Font::Storm3D_Scene_PicList_Font(Storm3D *s2,
		Storm3D_Scene *scene,Storm3D_Font *_font,VC2 _position,VC2 _size,const wchar_t *_text,float alpha_,const COL &colorFactor_) :
	Storm3D_Scene_PicList(s2,scene,_position,_size),
	font(_font),
	text(0),
	uniText(new wchar_t[wcslen(_text)+1]),
	alpha(alpha_),
	colorFactor(colorFactor_),
	width(0),
	height(0),
	wd(0),
	hd(0)
{
	// Copy text
	wcscpy(uniText, _text);
}

//! Destructor
Storm3D_Scene_PicList_Font::~Storm3D_Scene_PicList_Font()
{
	delete[] text;
	delete[] uniText;
}

//! Render font
void Storm3D_Scene_PicList_Font::Render()
{
	// Calculate complete letter amount and letters per texture
	int letters_per_texture=font->tex_letter_rows*font->tex_letter_columns;
	int letter_amt=font->texture_amount*letters_per_texture;

	// Create 3d-vector
	VC3 pos(position.x,position.y,0);

	// Create color (color+alpha)
	COL color = font->GetColor();
	color *= colorFactor;
	color.Clamp();

	DWORD col = COLOR_RGBA(int(color.r * 255), int(color.g * 255), int(color.b * 255), int(alpha * 255));
	glEnable(GL_BLEND);

	if(font->font)
	{
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		if (!tex)
		{
			SDL_Surface *buffer = NULL;
			SDL_Color col2 = { 255, 255, 255, 0 };
			if(uniText)
			{
				buffer = TTF_RenderUNICODE_Blended(font->GetFont(), (Uint16*)uniText, col2);
			}
			else if(text)
			{
				buffer = TTF_RenderUTF8_Blended(font->GetFont(), text, col2);
			}
			if (buffer)
			{
				int w2, h2;
				width  = w2 = buffer->w;
				height = h2 = buffer->h;

				toNearestPow(w2);
				toNearestPow(h2);

				wd = float(width )/float(w2);
				hd = float(height)/float(h2);

				boost::scoped_array<char> data(new char[w2*h2*4]);
				memset(data.get(), 0, w2 * h2 * 4);
				char *srcdata = (char *) buffer->pixels;
				GLenum format = (buffer->format->BytesPerPixel==4)?GL_RGBA:GL_RGB;
				for(unsigned int y=0;y<height;++y)
					memcpy(data.get()+y*w2*4,srcdata+y*buffer->pitch,buffer->pitch);
				SDL_FreeSurface(buffer);
				// upload to gl
				tex = glTexWrapper::rgbaTexture(w2, h2);
				glTexImage2D(GL_TEXTURE_2D, 0, tex->getFmt(), w2, h2, 0, format, GL_UNSIGNED_BYTE, data.get());
			}
			else
			{
				return;
			}
		}

		tex->bind();

		// render quad
		frozenbyte::storm::PixelShader::disable();
		frozenbyte::storm::VertexShader::disable();
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);

		// Create a quad
		VXFORMAT_2D vx[4];
		vx[0]=VXFORMAT_2D(pos+VC3(0           , (float)height, 0), 1, col, VC2(0,  hd));
		vx[1]=VXFORMAT_2D(pos                                    , 1, col, VC2(0,  0 ));
		vx[2]=VXFORMAT_2D(pos+VC3((float)width, (float)height, 0), 1, col, VC2(wd, hd));
		vx[3]=VXFORMAT_2D(pos+VC3((float)width, 0            , 0), 1, col, VC2(wd, 0 ));

		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), vx);
		glTexCoordPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), &vx[0].texcoords);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VXFORMAT_2D), &vx[0].color);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		scene->AddPolyCounter(2);
	}
	else if(text)
	{
		for(int l=0;l<int(strlen(text));l++)
		{
			// Search for letter
			int let=-1;
			for (int i=0;i<letter_amt;i++) 
			{
				if (font->letter_characters[i]==text[l]) 
				{
					let=i;
					// doh, why not break now when we found it and save time!
					break; 
				} 
				else 
				{
					// if we find the null terminator, just stop there, because
					// otherwise we'll go past the character array size if it
					// does not contain character definitions for total of letter_amt
					// characters. In my opininion requiring such a thing is not nice.
					if (font->letter_characters[i] == '\0') 
						break;
				}
			}

			// Is this letter in font
			if (let>=0)
			{
				// Apply the correct texture
				font->textures[let/letters_per_texture]->Apply(0);

				// Calculate x/y
				int x=let%font->tex_letter_columns;
				int y=(let%letters_per_texture)/font->tex_letter_columns;

				// Calculate texture coordinates
				float tx1=1/(float)font->tex_letter_columns;
				float ty1=1/(float)font->tex_letter_rows;
				float fx=(float)x*tx1;
				float fy=(float)y*ty1;

				VC2 p[4];
				p[0] = VC2(fx,fy+ty1);
				p[1] = VC2(fx,fy);
				p[2] = VC2(fx+tx1,fy+ty1);
				p[3] = VC2(fx+tx1,fy);

				// Create a quad
				VXFORMAT_2D vx[4];
				vx[0]=VXFORMAT_2D(pos+VC3(0,size.y,0),1,col,p[0]);
				vx[1]=VXFORMAT_2D(pos,1,col,p[1]);
				vx[2]=VXFORMAT_2D(pos+VC3(size.x,size.y,0),1,col,p[2]);
				vx[3]=VXFORMAT_2D(pos+VC3(size.x,0,0),1,col,p[3]);

				// Clip
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

					for(int i = 0; i < 4; ++i)
					{
						vx[i].position.x -= .5f;
						vx[i].position.y -= .5f;
					}

					// Render it
					frozenbyte::storm::PixelShader::disable();
					frozenbyte::storm::VertexShader::disable();

					glClientActiveTexture(GL_TEXTURE0);
					glEnableClientState(GL_COLOR_ARRAY);
					glEnableClientState(GL_VERTEX_ARRAY);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glVertexPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), vx);
					glTexCoordPointer(2, GL_FLOAT, sizeof(VXFORMAT_2D), &vx[0].texcoords);
					glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VXFORMAT_2D), &vx[0].color);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					scene->AddPolyCounter(2);
				}
			}

			// Add x-koordinate
			if (let>=0) pos.x+=((float)font->letter_width[let]/64.0f)*size.x;
				else pos.x+=size.x/2.0f;
		}
	} else {
		igiosWarning("no font and no text\n");
	}
}
