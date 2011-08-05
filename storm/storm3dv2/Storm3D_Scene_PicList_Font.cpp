// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_scene_piclist.h"
#include "storm3d_texture.h"
#include "storm3d_font.h"
#include "Clipper.h"

#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Scene_PicList_Font::Storm3D_Scene_PicList_Font
//------------------------------------------------------------------
Storm3D_Scene_PicList_Font::Storm3D_Scene_PicList_Font(Storm3D *s2,
		Storm3D_Scene *scene,Storm3D_Font *_font,VC2 _position,VC2 _size,const char *_text,float alpha_,const COL &colorFactor_) :
	Storm3D_Scene_PicList(s2,scene,_position,_size),
	font(_font),
	text(0),
	uniText(0),
	alpha(alpha_),
	colorFactor(colorFactor_)
{
	if (!font->isUnicode())
	{
		// Copy text
		text=new char[strlen(_text)+1];
		strcpy(text,_text);
	}
	else
	{
		// Hack: convert to unicode for non-english languages
		int length = MultiByteToWideChar(CP_ACP | CP_UTF8, 0, _text, strlen(_text) + 1, 0, 0);
		uniText = new wchar_t[length];
		MultiByteToWideChar(CP_ACP | CP_UTF8, 0, _text, strlen(_text) + 1, uniText, length);
	}
}

Storm3D_Scene_PicList_Font::Storm3D_Scene_PicList_Font(Storm3D *s2,
		Storm3D_Scene *scene,Storm3D_Font *_font,VC2 _position,VC2 _size,const wchar_t *_text,float alpha_,const COL &colorFactor_) :
	Storm3D_Scene_PicList(s2,scene,_position,_size),
	font(_font),
	text(0),
	uniText(0),
	alpha(alpha_),
	colorFactor(colorFactor_)
{
	// Copy text
	uniText = new wchar_t[wcslen(_text)+1];
	wcscpy(uniText, _text);
}


//------------------------------------------------------------------
// Storm3D_Scene_PicList_Font::~Storm3D_Scene_PicList_Font
//------------------------------------------------------------------
Storm3D_Scene_PicList_Font::~Storm3D_Scene_PicList_Font()
{
	delete[] text;
	delete[] uniText;
}



//------------------------------------------------------------------
// Storm3D_Scene_PicList_Font::Render
//------------------------------------------------------------------
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

	//DWORD col=font->GetColor().GetAsD3DCompatibleARGB();
	//DWORD col=color.GetAsD3DCompatibleARGB();
	DWORD col = D3DCOLOR_ARGB((int)((alpha)*255.0f),(int)(color.r*255.0f),(int)(color.g*255.0f),(int)(color.b*255.0f));
	Storm3D2->GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

	if(font->font && font->sprite)
	{
		#ifdef _MSC_VER
		#pragma message("**                                             **")
		#pragma message("** Size to screen boundary and enable clipping **")
		#pragma message("**                                             **")
		#endif

		//VC2 _position,VC2 _size
		RECT rc = { int(position.x), int(position.y), int(position.x + size.x + 100), int(position.y + size.y + 1000) };
		//if(uniText)
		//	font->font->DrawTextW(0, uniText, wcslen(uniText), &rc, DT_SINGLELINE | DT_LEFT | DT_NOCLIP, col);
		//else if(text)
		//	font->font->DrawText(0, text, strlen(text), &rc, DT_SINGLELINE | DT_LEFT | DT_NOCLIP, col);

		DWORD flags = D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_ALPHABLEND;
		font->sprite->Begin(flags);

		if(uniText)
			font->font->DrawTextW(font->sprite, uniText, wcslen(uniText), &rc, DT_LEFT | DT_NOCLIP, col);
		else if(text)
			font->font->DrawText(font->sprite, text, strlen(text), &rc, DT_LEFT | DT_NOCLIP, col);

		font->sprite->End();
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

				// Create a quad
				VXFORMAT_2D vx[4];
				vx[0]=VXFORMAT_2D(pos+VC3(0,size.y,0),1,
					col,VC2(fx,fy+ty1));
		
				vx[1]=VXFORMAT_2D(pos,1,
					col,VC2(fx,fy));
		
				vx[2]=VXFORMAT_2D(pos+VC3(size.x,size.y,0),1,
					col,VC2(fx+tx1,fy+ty1));

				vx[3]=VXFORMAT_2D(pos+VC3(size.x,0,0),1,
					col,VC2(fx+tx1,fy));

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
					Storm3D2->GetD3DDevice()->SetVertexShader(0);
					Storm3D2->GetD3DDevice()->SetFVF(FVF_VXFORMAT_2D);

					frozenbyte::storm::validateDevice(*Storm3D2->GetD3DDevice(), Storm3D2->getLogger());
					Storm3D2->GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vx,sizeof(VXFORMAT_2D));
					scene->AddPolyCounter(2);
				}
			}

			// Add x-koordinate
			if (let>=0) pos.x+=((float)font->letter_width[let]/64.0f)*size.x;
				else pos.x+=size.x/2.0f;
		}
	}
}


