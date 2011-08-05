// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_font.h"
#include "storm3d.h"
#include "storm3d_texture.h"
#include "../../util/Debug_MemoryManager.h"


// Typedef...
typedef Storm3D_Texture *PS3DTEX;



//------------------------------------------------------------------
// Storm3D_Font::Storm3D_Font
//------------------------------------------------------------------
Storm3D_Font::Storm3D_Font(Storm3D *s2) :
	Storm3D2(s2),
	textures(NULL),
	texture_amount(0),
	tex_letter_rows(1),
	tex_letter_columns(1),
	letter_characters(NULL),
	letter_width(NULL),
	font(0),
	sprite(0),
	face(0),
	width(0),
	height(0),
	bold(false),
	italic(false),
	ref_count(1)
{
}



//------------------------------------------------------------------
// Storm3D_Font::~Storm3D_Font
//------------------------------------------------------------------
Storm3D_Font::~Storm3D_Font()
{
	// Remove from Storm3D's list
	Storm3D2->Remove(this);

	// Delete textures (actually decreases ref.count)
	for (int i=0;i<texture_amount;i++) textures[i]->Release();
	delete[] textures;	// and array too

	// Delete letter properties
	if (letter_characters) SAFE_DELETE_ARRAY(letter_characters);
	if (letter_width) SAFE_DELETE_ARRAY(letter_width);

	if(font)
		font->Release();
	if(sprite)
		sprite->Release();
}



//------------------------------------------------------------------
// Storm3D_Font::AddTexture
//------------------------------------------------------------------
void Storm3D_Font::AddTexture(IStorm3D_Texture *itex)
{
	Storm3D_Texture *tex=(Storm3D_Texture*)itex;
	
	// Add texture reference count
	tex->AddRef();

	// Add texture amount
	texture_amount++;

	// Allocate new array / free old
	Storm3D_Texture **newtexs=new PS3DTEX[texture_amount];
	if (textures)
	{
		memcpy(newtexs,textures,sizeof(PS3DTEX)*texture_amount-1);
		delete[] textures;
	}
	textures=newtexs;

	// Put the new one into the list
	textures[texture_amount-1]=tex;

	// Free letter characters/widths
	if (letter_characters) SAFE_DELETE_ARRAY(letter_characters);
	if (letter_width) SAFE_DELETE_ARRAY(letter_width);

	// Calculate complete letter amount
	int letter_amt=texture_amount*tex_letter_rows*tex_letter_columns;

	// Alloc new
	if (letter_amt>0)
	{
		letter_characters=new char[letter_amt];
		letter_width=new BYTE[letter_amt];
	}
}



//------------------------------------------------------------------
// Storm3D_Font::SetTextureRowsAndColums
//------------------------------------------------------------------
void Storm3D_Font::SetTextureRowsAndColums(int rows,int columns)
{
	// Test first
	if (rows<1) rows=1;
	if (columns<1) columns=1;

	tex_letter_rows=rows;
	tex_letter_columns=columns;

	// Free letter characters/widths
	if (letter_characters) SAFE_DELETE_ARRAY(letter_characters);
	if (letter_width) SAFE_DELETE_ARRAY(letter_width);

	// Calculate complete letter amount
	int letter_amt=texture_amount*tex_letter_rows*tex_letter_columns;

	// Alloc new
	if (letter_amt>0)
	{
    // CHANGED: size + 1, so we can fit null term. char there
		letter_characters=new char[letter_amt + 1];
		letter_width=new BYTE[letter_amt + 1];
	}
}



//------------------------------------------------------------------
// Storm3D_Font::SetCharacters
//------------------------------------------------------------------
void Storm3D_Font::SetCharacters(const char *characters,BYTE *_letter_width)
{
	// Calculate complete letter amount
	int letter_amt=texture_amount*tex_letter_rows*tex_letter_columns;

	// Copy
	if (letter_amt>0)
	{
		// CHANGED: characters now treated as a null terminated string
		// however, keeping max. compatibility with non-null-terminated arrays
		int slen = 0; //strlen(characters);
		for (int i = 0; i < letter_amt; i++)
		{
			if (characters[i] != '\0') 
				slen++;
		}
				
		memcpy(letter_characters,characters,sizeof(char)*slen);
		memcpy(letter_width,_letter_width,sizeof(BYTE)*slen);
		letter_characters[slen] = '\0';
		letter_width[slen] = 0;
		//memcpy(letter_characters,characters,sizeof(char)*letter_amt);
		//memcpy(letter_width,_letter_width,sizeof(BYTE)*letter_amt);
	}
}

void Storm3D_Font::SetFont(const char *face_, int width_, int height_, bool bold_, bool italic_)
{
	if(font)
		font->Release();
	if(sprite)
		sprite->Release();

	D3DXFONT_DESC desc = { 0 };
	desc.Width = width_;
	desc.Height = height_;
	desc.MipLevels = 1;
	desc.CharSet = DEFAULT_CHARSET;
	desc.OutputPrecision = OUT_TT_ONLY_PRECIS;
	desc.Quality = DEFAULT_QUALITY;
	desc.Weight = (bold_) ? 700 : 400;
	desc.Italic = (italic_) ? TRUE : FALSE;
	desc.PitchAndFamily = DEFAULT_PITCH;
	strcpy(desc.FaceName, face_);

	HRESULT hr;
	hr = D3DXCreateFontIndirect(Storm3D2->GetD3DDevice(), &desc, &font);
	if(FAILED(hr))
	{
		//MessageBox(NULL,"Storm3D_Font::SetFont() - D3DXCreateFontIndirect failed!","Storm3D Error",0);
		font = NULL;
	}
	hr = D3DXCreateSprite(Storm3D2->GetD3DDevice(), &sprite);
	if(FAILED(hr))
	{
		//MessageBox(NULL,"Storm3D_Font::SetFont() - D3DXCreateSprite failed!","Storm3D Error",0);
		sprite = NULL;
	}

	// Store
	delete[] face;
	face = new char[strlen(face_) + 1];
	strcpy(face, face_);
	width = width_;
	height = height_;
	bold = bold_;
	italic = italic_;
}

void Storm3D_Font::ReleaseDynamicBuffers()
{
	if(font)
		font->OnLostDevice();
	if(sprite)
		sprite->OnLostDevice();
}

void Storm3D_Font::CreateDynamicBuffers()
{
	if(font)
		font->OnResetDevice();
	if(sprite)
		sprite->OnResetDevice();
}

//------------------------------------------------------------------
// Storm3D_Font::SetColor
//------------------------------------------------------------------
void Storm3D_Font::SetColor(const COL &_color)
{
	color=_color;
}



//------------------------------------------------------------------
// Storm3D_Font::GetColor
//------------------------------------------------------------------
COL &Storm3D_Font::GetColor()
{
	return color;
}

int Storm3D_Font::GetCharacterWidth(wchar_t *string, int length) const
{
	if(font && sprite)
	{
		RECT rc = { 0 };
		rc.right = 10;
		rc.bottom = 100;
		font->DrawTextW(0, (LPCWSTR) string, length, &rc, DT_SINGLELINE | DT_LEFT | DT_CALCRECT, 0);

		return rc.right;
	}

	return 0;
}

bool Storm3D_Font::isUnicode() const
{
	return (font && sprite) ? true : false;
}

const char *Storm3D_Font::GetFace() const
{
	return face;
}

int Storm3D_Font::GetWidth() const
{
	return width;
}

int Storm3D_Font::GetHeight() const
{
	return height;
}

bool Storm3D_Font::IsBold() const
{
	return bold;
}

bool Storm3D_Font::IsItalic() const
{
	return italic;
}

IStorm3D_Font *Storm3D_Font::Clone()
{
	++ref_count;
	return this;
}

void Storm3D_Font::Release()
{
	if(--ref_count < 1)
		delete this;
}

