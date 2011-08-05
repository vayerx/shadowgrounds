// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef CRAPPY_FONT_H
#define CRAPPY_FONT_H

namespace frozenbyte {
namespace particle{

struct CrappyFontData;

class CrappyFont 
{
	boost::scoped_ptr<CrappyFontData> m;

public:
	CrappyFont(editor::Storm& storm);
	~CrappyFont();
	void recreate();
	void setLetterSize(float sizeX, float sizeY);
	void renderText(float x, float y, const std::string& text);
};

} // particle
} // frozenbyte

#endif
