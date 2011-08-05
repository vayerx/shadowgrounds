
#include "precompiled.h"

#include <string>
#include "OguiButtonStyle.h"
#include "../util/Debug_MemoryManager.h"

OguiButtonStyle::OguiButtonStyle(
  IOguiImage *image, IOguiImage *imageDown,
  IOguiImage *imageDisabled, IOguiImage *imageHighlighted, 
  IOguiFont *textFont, int sizeX, int sizeY)
{
  this->image = image;
  this->imageDown = imageDown;
  this->imageDisabled = imageDisabled;
  this->imageHighlighted = imageHighlighted;
  this->textFont = textFont;
  this->sizeX = sizeX;
  this->sizeY = sizeY;
  this->textFontDisabled	= NULL;
  this->textFontDown		= NULL;
  this->textFontHighlighted = NULL;
}

