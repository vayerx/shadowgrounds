// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatypes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Texture.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Font;



//------------------------------------------------------------------
// IStorm3D_Font
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Font
{

public:

	// Add texture to font
	virtual void AddTexture(IStorm3D_Texture *tex)=0;

	// Set textures' letter rows/columns
	virtual void SetTextureRowsAndColums(int rows,int columns)=0;
	
	// Set characters and letter widths.
	// Use this after you have loaded all needed textures
	// and set texture rows and columns. Otherwise font will
	// not get all letters needed.
	virtual void SetCharacters(const char *characters,BYTE *letter_width)=0;

	// COL functions
	virtual void SetColor(const COL &color)=0;
	virtual COL &GetColor()=0;

	virtual void SetFont(const char *face, int width, int height, bool bold, bool italic) = 0;
	virtual int GetCharacterWidth(wchar_t *string, int length) const = 0; 

	virtual bool isUnicode() const = 0;
	virtual const char *GetFace() const = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual bool IsBold() const = 0;
	virtual bool IsItalic() const = 0;

	virtual IStorm3D_Font *Clone() = 0;
	virtual void Release() = 0;

	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_Font() {};
};


