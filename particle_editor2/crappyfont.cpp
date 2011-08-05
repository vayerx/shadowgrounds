// Copyright 2002-2004 Frozenbyte Ltd.

#include <Storm3D_Ui.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include "../editor/storm.h"

#include "crappyfont.h"

namespace frozenbyte
{
namespace particle
{

struct CrappyFontData {

	editor::Storm& storm;

	IStorm3D_Texture* texture;
	IStorm3D_Font* font;
	
	float sizeX, sizeY;
	
	CrappyFontData(editor::Storm& storm_) : storm(storm_) {
		
		sizeX = 10.0f;
		sizeY = 16.0f;
		init();
	}

	void init() 
	{
		font = storm.storm->CreateNewFont();
		font->SetFont("Times New Roman", 0, 17, false, false);
		font->SetColor(COL(1.0f, 1.0f, 1.0f));	

		/*
		font = storm.storm->CreateNewFont();
		
		std::string buff;
		buff += '\t';
		buff += '!';
		buff += '"';
		buff += "#$%@'()*+.-,/0123456789:;<=>?\nabcdefghijklmnopqrstuvwxyz[\\]\t_";
		
		std::vector<unsigned char> widths;
		for(unsigned int i = 0; i < buff.size(); i++) {
			widths.push_back(64);
		}
		
		//IStorm3D_Texture* ftex = storm.storm->CreateNewTexture("particle_editor/font2.dds");
#ifdef LEGACY_FILES
		IStorm3D_Texture* ftex = storm.storm->CreateNewTexture("Data/Fonts/font2.dds");
#else
		IStorm3D_Texture* ftex = storm.storm->CreateNewTexture("data/gui/font/font2.dds");
#endif
		font->AddTexture(ftex);
		font->SetTextureRowsAndColums(8, 8);
		font->SetCharacters(buff.c_str(), &widths[0]);
		font->SetColor(COL(1.0f, 1.0f, 1.0f));	
		*/
	}

	void renderText(const VC2& position, const std::string& text) {
					
		storm.scene->Render2D_Text(font, position, VC2(sizeX, sizeY), const_cast<char*>(text.c_str()));
		
	}


};


CrappyFont::CrappyFont(editor::Storm& storm) {
	boost::scoped_ptr<CrappyFontData> p(new CrappyFontData(storm));
	m.swap(p);
}

CrappyFont::~CrappyFont() {
}

void CrappyFont::recreate() {

	m->init();
}

void CrappyFont::setLetterSize(float sizeX, float sizeY) {
	m->sizeX = sizeX;
	m->sizeY = sizeY;
}

void CrappyFont::renderText(float x, float y, const std::string& text) {
	m->renderText(VC2(x,y), text);
}

} // particle
} // frozenbyte