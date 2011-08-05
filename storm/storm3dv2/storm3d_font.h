// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "SDL_ttf.h"
#include "storm3d_common_imp.h"
#include "IStorm3D_Font.h"

//------------------------------------------------------------------
// Storm3D_Font
//------------------------------------------------------------------
class Storm3D_Font : public IStorm3D_Font
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	Storm3D_Texture **textures;		// Textures to be used to render font
	int texture_amount;				// How many textures loaded
	int tex_letter_rows;			// How many letters in a row (in texture)
	int tex_letter_columns;			// How many letters in a column (in texture)
	char *letter_characters;		// Letter characters in font (a,b,c,d... etc.)
	BYTE *letter_width;				// Letter width (for each letter in font)

	TTF_Font *font;
	char *face;
	int width;
	int height;
	bool bold;
	bool italic;
	int ref_count;

	COL color;			// Font's color

public:

	// Add texture to font
	void AddTexture(IStorm3D_Texture *tex);

	// Set textures letter rows/columns
	void SetTextureRowsAndColums(int rows,int columns);
	
	// Set characters and letter widths.
	// Use this after you have loaded all needed textures
	// and set texture rows and columns. Otherwise font will
	// not get all letters needed.
	void SetCharacters(const char *characters,BYTE *letter_width);

	void SetFont(const char *face, int width, int height, bool bold, bool italic);
	void ReleaseDynamicBuffers();
	void CreateDynamicBuffers();

	// COL functions
	void SetColor(const COL &color);
	COL &GetColor();

	int GetCharacterWidth(wchar_t *string, int length) const;
	bool isUnicode() const;

	TTF_Font *GetFont();
	const char *GetFace() const;
	int GetWidth() const;
	int GetHeight() const;
	bool IsBold() const;
	bool IsItalic() const;

	IStorm3D_Font *Clone();
	void Release();

	Storm3D_Font(Storm3D *Storm3D2);
	~Storm3D_Font();

	friend class Storm3D_Scene_PicList_Font;
};



