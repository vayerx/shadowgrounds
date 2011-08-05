#include "precompiled.h"

#include <boost/lexical_cast.hpp>
#include <fstream>
#include <string.h>
#include <stdio.h>

#include <istorm3d_videostreamer.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "../util/fb_assert.h"
#include "../container/LinkedList.h"
#include "OguiException.h"
#include "IOguiDriver.h"
#include "OguiStormDriver.h"
#include "orvgui2.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../system/Logger.h"
#include "../util/TextureCache.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../util/UnicodeConverter.h"
#include "../util/Debug_MemoryManager.h"
#include "../game/DHLocaleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_locale.h"

using namespace frozenbyte;
#ifdef _MSC_VER
#pragma warning(disable : 4290)
#endif

extern int scr_size_x;
extern int scr_size_y;


OguiStormFont::OguiStormFont()
{
	parent = NULL;
	listNode = NULL;
	filename = NULL;
	texfname = NULL;
	tex = NULL;
	fnt = 0;
	cols = 0;
	rows = 0;
	chrwidth = 0;
	chrheight = 0;
	chrsize = NULL;
	chrdef = NULL;

	fontFace = 0;
	isUnicode = false;
	isBold = false;
	isItalic = false;
	fontWidth = 0;
	fontHeight = 0;
	COL color = COL(1.f, 1.f, 1.f);
}

OguiStormFont::~OguiStormFont()
{
	if (filename != NULL) delete [] filename;
	if (texfname != NULL) delete [] texfname;
	if (parent != NULL)
	{
		parent->removeFontByNode(listNode);
		if (fnt != NULL) fnt->Release();
		if (tex != NULL) tex->Release();
	}
	fnt = NULL;
	tex = NULL;
	if (chrdef != NULL)
		delete [] chrdef;
	if (chrsize != NULL)
		delete [] chrsize;
}

int OguiStormFont::getStringWidth(const wchar_t *text)
{
	if(fnt && text )
	{
		// BUGBUG: crashes here at start-up
		int length = fnt->GetCharacterWidth((wchar_t *) text, wcslen(text));
		return 1024 * length / scr_size_x;
	}

	return 0;
}

int OguiStormFont::getStringWidth(const char *text)
{
	if(!chrdef)
	{
		std::wstring uniText;
		util::convertToWide(text, uniText);
		
		if( uniText.empty() == false )
			return getStringWidth(uniText.c_str());
		else return 0;
	}

	float width = 0;
	int textlen = strlen(text);
	for (int i = 0; i < textlen; i++)
	{
		int j = 0;
		while (chrdef[j] != '\0')
		{
			if (chrdef[j] == text[i])
			{
				break;
			}
			j++;
		}
		if (chrdef[j] == '\0')
		{
			// storm makes missing fonts with empty space half of the font's width
			// or something... in this case we'll rather abort in such a case
			#ifdef _DEBUG
			// TODO: put the abort back!
//			if (text[i] != '\n' && text[i] != '\r')
//				assert(!"invalid font character");
			#endif
			// blaah... this looks like a nice number (missing font)
			width += chrwidth / 2.0f; 
		} else {
			// the magic number 64 and the static font size.
			width += (chrsize[j] * chrwidth / 64.0f );
		}
	}
	return (int)width;
}

int OguiStormFont::getStringHeight(const char *)
{
	return chrheight;
}

int OguiStormFont::getWidth()
{
	return chrwidth;
}

int OguiStormFont::getHeight()
{
	return chrheight;
}

OguiStormImage::OguiStormImage()
{
	parent = NULL;
	listNode = NULL;
	filename = NULL;
	mat = NULL;
	tex = NULL;
	video = NULL;
	renderTargetIndex = -1;
	deletedVideo = false;
	deleteVideoOnDestruction = false;
	streamBuilder = 0;
}

OguiStormImage::~OguiStormImage()
{
	if (filename != NULL) delete [] filename;

	if( video && deleteVideoOnDestruction )
	{
		delete video;
		video = NULL;

		mat = NULL;
	}

	if( deleteVideoOnDestruction == false && video )
	{
		video = NULL;
		mat = NULL;
	}


	if (parent != NULL)
	{
		parent->removeImageByNode(listNode);

		if (mat != NULL) delete mat;
//		if (tex != NULL) tex->Release();
	}


	mat = NULL;
	tex = NULL;
}


void ogui_storm_driver_error(const char *msg, const char *filename)
{
	if(filename)
	{
		char *emsg = new char[strlen(msg) + strlen(filename) + 8];
		strcpy(emsg, msg);
		strcat(emsg, " (");
		strcat(emsg, filename);
		strcat(emsg, ")");
		Logger::getInstance()->error(emsg);
		delete [] emsg;
	}
	else if(msg)
		Logger::getInstance()->error(msg);
}


OguiStormDriver::OguiStormDriver(IStorm3D *storm3d, IStorm3D_Scene *stormScene) 
	throw (OguiException *)
{
	if (got_orvgui) throw new OguiException("OguiStormDriver - Multiple instances not allowed.");
	og_setStorm3D(storm3d);
	og_setRendererScene(stormScene);
	init_orvgui();
 
	fonts = new LinkedList();
	images = new LinkedList();

	stormGeneration = 0;

	textureCache = new frozenbyte::TextureCache(*storm3d);
	textureCache->setLoadFlags(TEXLOADFLAGS_NOWRAP);
}

OguiStormDriver::~OguiStormDriver() 
{
	// should we delete all loaded images or just set their contents 
	// to null pointers... now we'll just modify the contents so
	// any pointers to them won't become invalid
	delete textureCache;

	while (!fonts->isEmpty())
	{
		OguiStormFont *f = (OguiStormFont *)fonts->popLast();
		//delete f;
		f->listNode = NULL;
		f->parent = NULL;
	}
	while (!images->isEmpty())
	{
		OguiStormImage *im = (OguiStormImage *)images->popLast();
		//delete im;
		im->listNode = NULL;
		im->parent = NULL;
	}

#ifdef WIN32
	for(unsigned int i = 0; i < fontResources.size(); i++)
	{
		RemoveFontResource(fontResources[i].c_str());
	}
#endif

	og_setRendererScene(NULL);
	og_setStorm3D(NULL);
	uninit_orvgui();

	delete images;
	delete fonts;
}


IOguiImage *OguiStormDriver::LoadOguiImage(const char *filename)
	throw (OguiException *)
{
	// hack fix
	if( filename == NULL || std::string( filename ).empty() )
	{
		return NULL;
	}

	OguiStormImage *tmp = new OguiStormImage(); 
	tmp->filename = new char[strlen(filename) + 1];
	strcpy(tmp->filename, filename);

/*
	IStorm3D_Texture *tex = 
		og_storm3d->CreateNewTexture(tmp->filename, 
			TEXLOADFLAGS_NOCOMPRESS | TEXLOADFLAGS_NOLOD);	
*/
	//this->textureCache->loadTexture(tmp->filename, false);
	IStorm3D_Texture *tex = this->textureCache->getTexture(tmp->filename, false);

	if (tex == NULL)
	{
		// just to pass the (deleted) filename to exception data pointer
		// helps debugging in case of exception, must not be used otherwise though
		//const char *invalidptr = filename; 
		delete tmp;
		ogui_storm_driver_error("OguiStormDriver::LoadOguiImage - Could not load image.", filename);
		//throw new OguiException(emsg, invalidptr);
		return NULL;
	}

	// TODO: possible problem with material naming convention
	// what happens if same image loaded twice -> 2 materials of same name!

	char *matname = new char[5 + strlen(tmp->filename) + 1];
	strcpy(matname, "ogui;");
	strcat(matname, tmp->filename);
	IStorm3D_Material *mat = og_storm3d->CreateNewMaterial(matname);
	delete [] matname;

	mat->SetBaseTexture(tex);
	mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
	//mat->SetAlphaType(IStorm3D_Material::ATYPE_NONE);

	tmp->mat = mat;
	tmp->tex = tex;

	images->append(tmp);
	tmp->parent = this;
	tmp->listNode = images->getLastNode();

	return tmp;
}

IOguiImage *OguiStormDriver::LoadOguiImage(int width, int height)
	throw (OguiException *)
{
	OguiStormImage *tmp = new OguiStormImage(); 

	IStorm3D_Texture *tex = 
		og_storm3d->CreateNewTexture(width, height, 
		IStorm3D_Texture::TEXTYPE_BASIC);  

	if (tex == NULL)
	{
		ogui_storm_driver_error("OguiStormDriver::LoadOguiImage - Could not create image.", 0);
		//throw new OguiException(emsg, invalidptr);
		return NULL;
	}

	// TODO: possible problem with material naming convention
	// what happens if same image loaded twice -> 2 materials of same name!

	IStorm3D_Material *mat = og_storm3d->CreateNewMaterial("user_defined");
	mat->SetBaseTexture(tex);
	mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);

	tmp->mat = mat;
	tmp->tex = tex;

	images->append(tmp);
	tmp->parent = this;
	tmp->listNode = images->getLastNode();

	return tmp;
}


IOguiImage *OguiStormDriver::GetOguiRenderTarget(int index)
{
	OguiStormImage *tmp = new OguiStormImage(); 
	IStorm3D_Texture *tex = og_storm3d->getRenderTarget(index);

	if (tex == NULL)
	{
		ogui_storm_driver_error("OguiStormDriver::GetOguiRenderTarget - Could not get render target.", 0);
		//throw new OguiException(emsg, invalidptr);
		return NULL;
	}

	// TODO: possible problem with material naming convention
	// what happens if same image loaded twice -> 2 materials of same name!

	IStorm3D_Material *mat = og_storm3d->CreateNewMaterial("user_defined");
	mat->SetBaseTexture(tex);
	mat->SetAlphaType(IStorm3D_Material::ATYPE_NONE);
	//mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);

	tmp->mat = mat;
	tmp->tex = tex;

	images->append(tmp);
	tmp->parent = this;
	tmp->listNode = images->getLastNode();

	tmp->renderTargetIndex = index;

	return tmp;
}

// TODO: leaks memory if exception thrown (tmp is not deleted)

IOguiFont *OguiStormDriver::LoadFont(const char *filename)
	throw (OguiException *)
{
	IStorm3D_Font *fnt = 0;
	OguiStormFont *tmp = 0;

	if(filename && strlen(filename) > 4 && strcmp(&filename[strlen(filename) - 4], ".fbf") == 0)
	{
		//filesystem::InputStream stream = filesystem::createInputFileStream(filename);
		filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(filename);
		editor::EditorParser parser;
		parser.readStream(stream);
		const editor::ParserGroup &group = parser.getGlobals();

		std::string fontFace = group.getValue("name");
		int width = editor::convertFromString<int> (group.getValue("width"), 0);
		int height = editor::convertFromString<int> (group.getValue("height"), 0);
		int lineHeight = editor::convertFromString<int> (group.getValue("line_height"), 0);
		bool bold = editor::convertFromString<bool> (group.getValue("bold"), false);
		bool italic = editor::convertFromString<bool> (group.getValue("italic"), false);


#ifdef WIN32
		// / IGIOS TL: FIXME: whut?
		// load unicode font from file hax
		std::string fontFile = group.getValue("font_file");

		// get locale based settings
		std::string subgroup_name = game::DHLocaleManager::getInstance()->getLocaleIdString(game::SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE));
		if(group.hasSubGroup(subgroup_name))
		{
			const editor::ParserGroup &subgroup = parser.getGlobals().getSubGroup(subgroup_name);
			fontFace = subgroup.getValue("name", fontFace);
			fontFile = group.getValue("font_file", fontFile);
			width = editor::convertFromString<int> (subgroup.getValue("width"), width);
			height = editor::convertFromString<int> (subgroup.getValue("height"), height);
			lineHeight = editor::convertFromString<int> (subgroup.getValue("line_height"), lineHeight);
		}

		if( fontFile.empty() == false )
		{
			if( AddFontResource( fontFile.c_str() ) == 0 )
			{
				Logger::getInstance()->error( "OguiStromDriver::LoadFont() - Couldn't load font file" );
			} else {
				fontResources.push_back(fontFile); // IGIOS TL: this should be kept even if everything else goes
			}
		}
#endif

		// Convert
		width = scr_size_x * width / 1024;
		height = scr_size_y * height / 768;
		//lineHeight = scr_size_y * lineHeight / 768;

		int color_r = editor::convertFromString<int> (group.getValue("color_r"), 255);
		int color_g = editor::convertFromString<int> (group.getValue("color_g"), 255);
		int color_b = editor::convertFromString<int> (group.getValue("color_b"), 255);
		COL color(color_r / 255.f, color_g / 255.f, color_b / 255.f);

		// HAAAX -- color should be for per instance

		{
			fonts->resetIterate();
			while (fonts->iterateAvailable())
			{
				OguiStormFont *oguiFont = (OguiStormFont *)fonts->iterateNext();
				if(!fnt)
					continue;

				IStorm3D_Font *font = oguiFont->fnt;
				if(fontFace == font->GetFace() && width == font->GetWidth() && height == font->GetHeight() && 
					bold == font->IsBold() && italic == font->IsItalic())
				{
					fnt = font->Clone();
				}
			}

			if(!fnt)
			{
				fnt = og_storm3d->CreateNewFont();
				fnt->SetFont(fontFace.c_str(), width, height, bold, italic);
				fnt->SetColor(COL(1.f, 1.f, 1.f));
				//fnt->SetColor(color);
			}

			tmp = new OguiStormFont(); 
			tmp->fontFace = new char[fontFace.size() + 1];
			strcpy(tmp->fontFace, fontFace.c_str());
			tmp->chrheight = lineHeight;
			tmp->isUnicode = true;
			tmp->isBold = bold;
			tmp->isItalic = italic;
			tmp->fontWidth = width;
			tmp->fontHeight = height;
			tmp->color = color;
		}
	}
	else
	{
		filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		if (f == NULL)
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - Could not open font file.", filename);

			/*
			std::fstream file( "missing_fonts.txt", std::ios::out | std::ios::app );
			file << filename << std::endl;
			file.close();
			*/
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - Could not open font file.", filename);
			//
		}

		//fseek(f, 0, SEEK_END);
		//int flen = ftell(f);
		//fseek(f, 0, SEEK_SET);
		int flen = filesystem::fb_fsize(f);

		if (flen < 4)
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - File size too small.", filename);
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - File size too small.", filename);
		}

		char *buf = new char[flen]; 

		if ((int)filesystem::fb_fread(buf, sizeof(char), flen, f) != flen) 
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - Error while reading file.", filename);
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - Error while reading file.", filename);
		}

		filesystem::fb_fclose(f);

		// the quick hack parser
		// OGF;XX;YY;xx;yy;AAA;CCC;1...;SSSS...;texturename;ignored...
		// or 
		// OG2;XX;YY;xx;yy;AAA;CCC;1...;SS,SS...,;texturename;ignored...
		//
		// XX,YY = 0-99 (columns, rows)
		// xx,yy = 0-99 (letter width, height)
		// AAA = 0-999 (amount)
		// CCC = 0-8,0-8,0-8 (colors rgb)
		// 1... = (letters)
		// SS = 0-99 (size of letter)
		
		if ((strncmp(buf, "OGF;", 4) != 0 && strncmp(buf, "OG2;", 4) != 0) || buf[6] != ';' || buf[9] != ';' 
			|| buf[12] != ';' || buf[15] != ';'
			|| buf[19] != ';' || buf[23] != ';')
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - Bad file format.", filename);
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - Bad file format.", filename);
		}
		
		bool ver2 = false;
		if (strncmp(buf, "OG2;", 4) == 0) 
			ver2 = true;

		int cols = 0;
		int rows = 0;
		int amount = 0;
		int chrwidth = 0;
		int chrheight = 0;
		float col_r = 0;
		float col_g = 0;
		float col_b = 0;

		cols = buf[5] - 48 + (buf[4] - 48) * 10;
		rows = buf[8] - 48 + (buf[7] - 48) * 10;
		chrwidth = buf[11] - 48 + (buf[10] - 48) * 10;
		chrheight = buf[14] - 48 + (buf[13] - 48) * 10;
		amount = buf[18] - 48 + (buf[17] - 48) * 10 + (buf[16] - 48) * 100;
		col_r = (float)(buf[20] - 48) / 8.0f;
		col_g = (float)(buf[21] - 48) / 8.0f;
		col_b = (float)(buf[22] - 48) / 8.0f;
		COL col = COL(col_r, col_g, col_b);

		int minflen;
		if (ver2)
			minflen = 26 + amount * 4;
		else
			minflen = 26 + amount * 3;

		if (flen < minflen)
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - File size too small for font amount.", filename);
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - File size too small for font amount.", tmp->filename);
		}

		int bufsize = amount + 8;

		// Allow one incomplete row.
		//if (amount != rows * cols)
		if(amount <= (rows-1) * cols)
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - Amount does not match rows and columns.", filename);
			//fb_assert(!"OguiStormDriver::LoadFont - Amount does not match rows and columns.");
			if (amount < rows * cols)
			{
				bufsize = rows * cols + 8;
			}
		}

		char *chrdef = new char[bufsize];
		BYTE *chrsize = new BYTE[bufsize];
		for (int i = 0; i < bufsize; i++)
		{
			chrdef[i] = '\0';
			chrsize[i] = 0;
		}
		for (int i = 0; i < amount; i++)
		{
			chrdef[i] = buf[24 + i];
			if (ver2)
				chrsize[i] = (unsigned char)((buf[25 + amount + i * 3] - 48) * 10 + buf[26 + amount + i * 3] - 48);
			else
				chrsize[i] = (unsigned char)((buf[25 + amount + i * 2] - 48) * 10 + buf[26 + amount + i * 2] - 48);
		}
		chrdef[amount] = '\0';

		char *fname = NULL;
		int fnamestart;
		if (ver2)
			fnamestart = 26 + amount * 4;
		else
			fnamestart = 26 + amount * 3;
		for (int j = fnamestart; j < flen; j++)
		{
			if (buf[j] == ';')
			{
				fname = new char[j - fnamestart + 1];
				strncpy(fname, &buf[fnamestart], j - fnamestart);
				fname[j - fnamestart] = '\0';
				break;
			}
		}
		if (fname == NULL)
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - Unable to resolve texture name.", filename);
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - Unable to resolve texture name.", filename);
		}

		delete [] buf;

		IStorm3D_Texture *tex = 
			og_storm3d->CreateNewTexture(fname, 
				TEXLOADFLAGS_NOCOMPRESS | TEXLOADFLAGS_NOLOD);

		if (tex == NULL)
		{
			ogui_storm_driver_error("OguiStormDriver::LoadFont - Could not load texture.", fname);
			return NULL;
			//throw new OguiException("OguiStormDriver::LoadFont - Could not load texture.", fname);
		}

		fnt = og_storm3d->CreateNewFont();
		fnt->AddTexture(tex);
		fnt->SetTextureRowsAndColums(rows, cols);
		//fnt->SetColor(col);
		fnt->SetColor(COL(1.f, 1.f, 1.f));

		fnt->SetCharacters(chrdef, chrsize);
	
		tmp = new OguiStormFont(); 
		tmp->texfname = fname;
		tmp->cols = cols;
		tmp->rows = rows;
		tmp->tex = tex;
		tmp->col = col;
		tmp->chrwidth = chrwidth;
		tmp->chrheight = chrheight;
		tmp->amount = amount;
		tmp->chrdef = chrdef;
		tmp->chrsize = chrsize;
		tmp->color = col;
	}

	tmp->filename = new char[strlen(filename) + 1];
	strcpy(tmp->filename, filename);

	fonts->append(tmp);
	tmp->fnt = fnt;
	tmp->parent = this;
	tmp->listNode = fonts->getLastNode();

	return tmp;
}

IOguiImage* OguiStormDriver::LoadOguiVideo( const char* filename, IStorm3D_StreamBuilder *streamBuilder)
{
	IStorm3D_VideoStreamer* video_str = og_storm3d->CreateVideoStreamer( filename, streamBuilder, true );
	OguiStormImage *tmp = new OguiStormImage(); 
	tmp->filename = new char[strlen(filename) + 1];
	strcpy(tmp->filename, filename);

	if( video_str == NULL )
	{
		// just to pass the (deleted) filename to exception data pointer
		// helps debugging in case of exception, must not be used otherwise though
		// const char *invalidptr = filename; 
		delete tmp;
		ogui_storm_driver_error("OguiStormDriver::ConvertVideoToImage - Could not load video.", "" );
		//throw new OguiException(emsg, invalidptr);
		return NULL;
	}

	tmp->mat = video_str->getMaterial();
	tmp->tex = NULL;
	tmp->video = video_str;
	tmp->streamBuilder = streamBuilder;
	tmp->deleteVideoOnDestruction = true;

	images->append(tmp);
	tmp->parent = this;
	tmp->listNode = images->getLastNode();

	return tmp;
}

IOguiImage* OguiStormDriver::ConvertVideoToImage( IStorm3D_VideoStreamer* video_str, IStorm3D_StreamBuilder *streamBuilder )
{
	OguiStormImage *tmp = new OguiStormImage(); 
	tmp->filename = NULL; // new char[strlen(filename) + 1];
	// strcpy(tmp->filename, filename);

	if( video_str == NULL )
	{
		// just to pass the (deleted) filename to exception data pointer
		// helps debugging in case of exception, must not be used otherwise though
		// const char *invalidptr = filename; 
		delete tmp;
		ogui_storm_driver_error("OguiStormDriver::ConvertVideoToImage - Could not load video.", "" );
		//throw new OguiException(emsg, invalidptr);
		return NULL;
	}

	tmp->mat = video_str->getMaterial();
	tmp->tex = NULL;
	tmp->video = video_str;
	tmp->streamBuilder = streamBuilder;
	tmp->deleteVideoOnDestruction = false;

	images->append(tmp);
	tmp->parent = this;
	tmp->listNode = images->getLastNode();

	return tmp;
}


// TODO: leaks memory in case of exception, but who cares because 
// exception here is very likely to be fatal anyway.

void OguiStormDriver::prepareForNextStormGeneration(IStorm3D_Scene *stormScene)
	throw (OguiException *)
{
	images->resetIterate();
	while (images->iterateAvailable())
	{
		OguiStormImage *img = (OguiStormImage *)images->iterateNext();
		if (img->video)
		{
			delete img->video;
			img->video = NULL;
			img->deletedVideo = true;
		}
	}

}

void OguiStormDriver::nextStormGeneration(IStorm3D_Scene *stormScene)
	throw (OguiException *)
{
	// reload all storm textures, fonts and materials in all 
	// ogui images and fonts because they are invalid now

	// Clear font pointers
	fonts->resetIterate();
	while (fonts->iterateAvailable())
	{
		OguiStormFont *fnt = (OguiStormFont *)fonts->iterateNext();
		fnt->fnt = 0;
	}

	// reload all fonts
	//fonts->resetIterate();
	//while (fonts->iterateAvailable())
	LinkedListIterator it(fonts);
	while(it.iterateAvailable())
	{
		OguiStormFont *fnt = (OguiStormFont *)it.iterateNext();
		if(!fnt)
			continue;

		if(fnt->isUnicode)
		{
			IStorm3D_Font *stormfnt = 0;

			LinkedListIterator it2(fonts);
			while(it2.iterateAvailable())
			{
				OguiStormFont *fnt2 = (OguiStormFont *)it2.iterateNext();
				if(!fnt2 || !fnt2->isUnicode || !fnt2->fnt)
					continue;

				if(fnt->fontWidth == fnt2->fontWidth && fnt->fontHeight == fnt2->fontHeight 
					&& fnt->isBold == fnt2->isBold && fnt->isItalic == fnt2->isItalic &&
					strcmp(fnt->fontFace, fnt2->fontFace) == 1)
				{
					stormfnt = fnt2->fnt->Clone();
				}
			}

			if(!stormfnt)
			{
				stormfnt = og_storm3d->CreateNewFont();
				stormfnt->SetFont(fnt->fontFace, fnt->fontWidth, fnt->fontHeight, fnt->isBold, fnt->isItalic);
				stormfnt->SetColor(COL(1.f, 1.f, 1.f));
				//stormfnt->SetColor(fnt->color);
			}

			fnt->fnt = stormfnt;
		}
		else
		{
			fnt->tex = 
				og_storm3d->CreateNewTexture(fnt->texfname, 
					TEXLOADFLAGS_NOCOMPRESS | TEXLOADFLAGS_NOLOD | TEXLOADFLAGS_NOWRAP);

			if (fnt->tex == NULL)
			{
				throw new OguiException("OguiStormDriver::nextStormGeneration - Could not reload font texture.", fnt->texfname);
			}

			IStorm3D_Font *stormfnt = og_storm3d->CreateNewFont();
			stormfnt->AddTexture(fnt->tex);
			stormfnt->SetTextureRowsAndColums(fnt->rows, fnt->cols);
			//stormfnt->SetColor(fnt->col);
			stormfnt->SetColor(COL(1.f, 1.f, 1.f));
			stormfnt->SetCharacters(fnt->chrdef, fnt->chrsize);

			fnt->fnt = stormfnt;
		}
	}

	// assuming texture cache has been earlier specifically destroyed
	assert(textureCache == NULL);

	textureCache = new frozenbyte::TextureCache(*og_storm3d);
	textureCache->setLoadFlags(TEXLOADFLAGS_NOWRAP);

	images->resetIterate();
	while (images->iterateAvailable())
	{
		OguiStormImage *img = (OguiStormImage *)images->iterateNext();

/*
		img->tex = 
			og_storm3d->CreateNewTexture(img->filename, 
				TEXLOADFLAGS_NOCOMPRESS | TEXLOADFLAGS_NOLOD);	
*/
		if (img->filename != NULL)
		{
			if(img->deletedVideo)
			{
				//delete img->video;
				IStorm3D_VideoStreamer* video_str = og_storm3d->CreateVideoStreamer( img->filename, img->streamBuilder, true );
				if(video_str)
				{
					img->mat = video_str->getMaterial();
					img->tex = NULL;
					img->video = video_str;

					continue;
				}
				else
				{
					img->mat = 0;
					img->tex = 0;
					img->video = 0;
				}
			}
			else
			{
				//this->textureCache->loadTexture(img->filename, false);
				img->tex = this->textureCache->getTexture(img->filename, false);
			}
		} 
		else 
		{
			if (img->renderTargetIndex >= 0)
			{
				img->tex = og_storm3d->getRenderTarget(img->renderTargetIndex);
			}
			else
				img->tex = 0;
		}
/*
		if (img->tex == NULL)
		{
			throw new OguiException("OguiStormDriver::nextStormGeneration - Could not reload image.", img->filename);
		}
*/

		// TODO: possible problem with material naming convention
		// what happens if same image loaded twice -> 2 materials of same name!

	IStorm3D_Material *mat = 0;
	if(img->filename)
	{
		char *matname = new char[5 + strlen(img->filename) + 1];
		strcpy(matname, "ogui;");
		strcat(matname, img->filename);
		mat = og_storm3d->CreateNewMaterial(matname);
		delete [] matname;
	}
	else
	{
		mat = og_storm3d->CreateNewMaterial("ogui: null filename");
	}

		mat->SetBaseTexture(img->tex);

		if(img->filename || img->renderTargetIndex < 0)
			mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
		else
			mat->SetAlphaType(IStorm3D_Material::ATYPE_NONE);

		img->mat = mat;
	}

	og_setRendererScene(stormScene);
	stormGeneration++;
}

void OguiStormDriver::deleteTextureCache()
{
	assert(textureCache != NULL);

	delete textureCache;
	textureCache = NULL;
}

void OguiStormDriver::updateVideos()
{
	images->resetIterate();
	while (images->iterateAvailable())
	{
		OguiStormImage *img = (OguiStormImage *)images->iterateNext();

		if( img->video != NULL )
		{
			img->video->update();
		}
	}
}


void OguiStormDriver::removeImageByNode(const ListNode *node)
{
	images->removeNode(node);
	// videos->removeNode(node);
}

void OguiStormDriver::removeFontByNode(const ListNode *node)
{
	fonts->removeNode(node);
}

frozenbyte::TextureCache *OguiStormDriver::getTextureCache()
{
	return textureCache;
}
