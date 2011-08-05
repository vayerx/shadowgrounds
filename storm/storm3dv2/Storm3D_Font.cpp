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

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

// Typedef...
typedef Storm3D_Texture *PS3DTEX;

//! Constructor
Storm3D_Font::Storm3D_Font(Storm3D *s2) :
	Storm3D2(s2),
	textures(NULL),
	texture_amount(0),
	tex_letter_rows(1),
	tex_letter_columns(1),
	letter_characters(NULL),
	letter_width(NULL),
	font(0),
	face(0),
	width(0),
	height(0),
	bold(false),
	italic(false),
	ref_count(1)
{
}

//! Destructor
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
		TTF_CloseFont(font);
}

//! Add new texture
/*
	\param itex texture
*/
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

//! Set texture rows and columns
/*
	\param rows rows
	\param columns columns
*/
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

//! Set characters
/*
	\param characters
	\param _letter_width
*/
void Storm3D_Font::SetCharacters(const char *characters,BYTE *_letter_width)
{
	// Calculate complete letter amount
	int letter_amt=texture_amount*tex_letter_rows*tex_letter_columns;

	// Copy
	if (letter_amt>0)
	{
		// CHANGED: characters now treated as a null terminated string
		// however, keeping max. compatibility with non-null-terminated arrays
		int slen = 0;
		for (int i = 0; i < letter_amt; i++)
		{
			if (characters[i] != '\0') 
				slen++;
		}
				
		memcpy(letter_characters,characters,sizeof(char)*slen);
		memcpy(letter_width,_letter_width,sizeof(BYTE)*slen);
		letter_characters[slen] = '\0';
		letter_width[slen] = 0;
	}
}

//! Set the font to use
/*!
	\param face_ font face
	\param width_ font width
	\param height_ font height
	\param bold_ is font bold
	\param italic_ is font italic
*/
void Storm3D_Font::SetFont(const char *face_, int width_, int height_, bool bold_, bool italic_)
{
	if (font)
	{
		TTF_CloseFont(font);
		font = NULL;
	}

	// FIXME: hack below
	std::string font_file;

#ifdef WIN32

	if(strcasecmp(face_, "Zero Threes") == 0)
	{
		font_file = "./data/fonts/unicode/";
		font_file.append("zerothre");
	}
	// HACK HACK HACK
	// Someone is trying to load a font with null string as filename and zero height, and yet it is used!
	else if(strcasecmp(face_, "") == 0)
	{
		font_file = igios_getFontDirectory();
		font_file.append("verdana");
		height_ = 14;
	}
	else
	{
		font_file = igios_getFontDirectory();
		font_file.append(face_);
	}

#else

	font_file = "./data/fonts/unicode/";
	// HACK HACK HACK
	// Someone is trying to load a font with null string as filename and zero height, and yet it is used!
	if (*face_ == '\0') {
		font_file.append("FreeSans");
		height_ = 14;
	} else if(strcasecmp(face_, "Zero Threes") == 0) {
		font_file.append("zerothre");
	} else if(strcasecmp(face_, "Verdana") == 0) {
		// Verdana is microsoft font and is not available on linux
		font_file.append("FreeSans");
	} else {
		font_file.append(face_);
	}

#endif

	font_file.append(".ttf");
	int style = 0x0;
	if (bold_)
		style = style | TTF_STYLE_BOLD;
	if (italic_)
		style = style | TTF_STYLE_ITALIC;
	// FIXME: font width
	font = TTF_OpenFont(font_file.c_str(), height_);
	if (font)
		TTF_SetFontStyle(font, style);
	else {
		igiosWarning("Font (%s) load failed: %s\n", font_file.c_str(), TTF_GetError());
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
	if (font)
		TTF_CloseFont(font);
	igios_unimplemented();
}

void Storm3D_Font::CreateDynamicBuffers()
{
	igios_unimplemented();
}

//! Set font color
/*
	\param _color color
*/
void Storm3D_Font::SetColor(const COL &_color)
{
	color=_color;
}

//! Get font color
/*
	\return color
*/
COL &Storm3D_Font::GetColor()
{
	return color;
}

//! Get character width of string
/*!
	\param string text string
	\param length length of string
	\return character width
*/
int Storm3D_Font::GetCharacterWidth(wchar_t *string, int length) const
{
	int w, h;
	if(font)
	{
		if (sizeof(wchar_t) == 2) {
			TTF_SizeUNICODE(font, (Uint16*)string, &w, &h);
			return w;
		} else {
			Uint16 *tmp = new Uint16[length+1];
			for (unsigned int i = 0; i < (unsigned int)(length + 1); ++i)
				tmp[i] = string[i] & 0xffff;
			TTF_SizeUNICODE(font, tmp, &w, &h);
			delete [] tmp;
			return w;
		}
	}

	return 0;
}

//! Is font unicode?
/*!
	\return
*/
bool Storm3D_Font::isUnicode() const
{
	return (font) ? true : false;
}

//! Get font face
/*
	\return face
*/
const char *Storm3D_Font::GetFace() const
{
	return face;
}

//! Get font width
/*
	\return width
*/
int Storm3D_Font::GetWidth() const
{
	return width;
}

//! Get font height
/*
	\return height
*/
int Storm3D_Font::GetHeight() const
{
	return height;
}

//! Is font bold?
/*!
	\return true if bold
*/
bool Storm3D_Font::IsBold() const
{
	return bold;
}

//! Is font italic?
/*!
	\return true if italic
*/
bool Storm3D_Font::IsItalic() const
{
	return italic;
}

//! Clone font
/*!
	\return clone
*/
IStorm3D_Font *Storm3D_Font::Clone()
{
	++ref_count;
	return this;
}

//! Release font
void Storm3D_Font::Release()
{
	if(--ref_count < 1)
		delete this;
}

//! Get TTF font
/*
	\return TTF_Font pointer
*/
TTF_Font *Storm3D_Font::GetFont()
{
	return font;
}
