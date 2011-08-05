
#ifndef OGUIBUTTONSTYLE_H
#define OGUIBUTTONSTYLE_H

//
// Container class for style definitions to be used in a button
//

#include "IOguiImage.h"
#include "IOguiFont.h"


class OguiButton;
class OguiSelectList;
class Ogui;


struct OguiButtonStyle
{
//public:
  OguiButtonStyle(IOguiImage *image, IOguiImage *imageDown,
    IOguiImage *imageDisabled, IOguiImage *imageHighlighted, 
    IOguiFont *textFont, int sizeX, int sizeY);

// private:
  IOguiImage *image;
  IOguiImage *imageDown;
  IOguiImage *imageDisabled;
  IOguiImage *imageHighlighted;
  IOguiFont *textFont;
  IOguiFont *textFontDown;
  IOguiFont *textFontHighlighted;
  IOguiFont *textFontDisabled;
  int sizeX;
  int sizeY;

//  friend class OguiButton;
//  friend class OguiSelectList;
//  friend class Ogui;
};

#endif

