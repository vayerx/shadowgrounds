
#ifndef IOGUIFONT_H
#define IOGUIFONT_H

class IOguiFont
{
public:
  virtual ~IOguiFont() {};

  // should return the pixel width of the given text when using this font
  virtual int getStringWidth(const char *text) = 0;
  virtual int getStringWidth(const wchar_t *text) = 0;

  // should return the pixel height of the given text when using this font
  virtual int getStringHeight(const char *text) = 0;

  // should return the pixel width of a given font character
  // with variable width fonts, this is not the final width seen on screen!
  virtual int getWidth() = 0;

  // should return the pixel height of a given font character
  virtual int getHeight() = 0;
};

#endif
