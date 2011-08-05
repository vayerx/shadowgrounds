
#ifndef OGUISTORMDRIVER_H
#define OGUISTORMDRIVER_H

#ifdef _MSC_VER
#pragma warning(disable : 4290)
#endif

#include <Storm3D_UI.h>
#include "../container/LinkedList.h"
#include "IOguiDriver.h"
#include "IOguiImage.h"
#include "IOguiFont.h"
#include <boost/shared_ptr.hpp>

//
// Does not actually implement a driver, but just gives ogui user a dummy
// class to hide the real hard coded implmentation. Making the ogui api 
// less underlaying implementation specific (allowing implentation changes
// without changes in api or program using ogui).
//
// Notice: this may seem like a proper oo-style implementation, but the 
// underlaying code is procedural C using global variables. Because of that, 
// ONLY ONE INSTANCE OF THIS DRIVER IS ALLOWED!
//

class IStorm3D_StreamBuilder;
class IStorm3D_Streamer;

namespace frozenbyte
{
	class TextureCache;
}

class OguiStormDriver;

class OguiStormImage : public IOguiImage
{
public:
  char *filename;
  IStorm3D_Material *mat;
  IStorm3D_Texture *tex;
  IStorm3D_VideoStreamer* video;
  //int stormGeneration;
  OguiStormDriver *parent;
  const ListNode *listNode;
	int renderTargetIndex;
	
	bool deleteVideoOnDestruction;
	bool deletedVideo;
	IStorm3D_StreamBuilder *streamBuilder;

  OguiStormImage();
  virtual ~OguiStormImage();

  IStorm3D_Texture *getTexture()
  {
	  return tex;
  }

	IStorm3D_Material *getMaterial()
	{
		return mat;
	}
};


class OguiStormFont : public IOguiFont
{
public:
  char *filename;
  char *texfname;
  IStorm3D_Font *fnt;
  IStorm3D_Texture *tex;
  //int stormGeneration;
  OguiStormDriver *parent;
  const ListNode *listNode;

  int cols;
  int rows;
  COL col;
  int amount;
  char *chrdef;
  BYTE *chrsize;  // x/64 part of chrwidth
  
  int chrwidth;
  int chrheight;

  bool isUnicode;
  char *fontFace;
  bool isBold;
  bool isItalic;
  int fontWidth;
  int fontHeight;
  COL color;

  OguiStormFont();
  virtual ~OguiStormFont();
  virtual int getStringWidth(const char *text);
  virtual int getStringWidth(const wchar_t *text);
  virtual int getStringHeight(const char *text);
  virtual int getWidth();
  virtual int getHeight();
};

class OguiStormDriver : public IOguiDriver
{
public:
  OguiStormDriver(IStorm3D *storm3d, IStorm3D_Scene *stormScene) throw (OguiException *);
  ~OguiStormDriver();
  virtual IOguiImage *LoadOguiImage(const char *filename) throw (OguiException *);
  virtual IOguiImage *LoadOguiImage(int width, int height) throw (OguiException *);
  virtual IOguiImage *GetOguiRenderTarget(int index);
  virtual IOguiFont *LoadFont(const char *filename) throw (OguiException *);
  virtual IOguiImage *LoadOguiVideo( const char* filename, IStorm3D_StreamBuilder *streamBuilder );
  virtual IOguiImage *ConvertVideoToImage( IStorm3D_VideoStreamer* stream, IStorm3D_StreamBuilder *streamBuilder );

	// call before storm empty (or video textures will crash)
	void prepareForNextStormGeneration(IStorm3D_Scene *stormScene) throw (OguiException *);

  // call this when storm does something nasty, such as Empty() method call
  // in general, should be called if any storm texture(, etc.) pointers
  // get invalidated.
  void nextStormGeneration(IStorm3D_Scene *stormScene) throw (OguiException *);

	// get rid of texture cache, cos nextStormGeneration is called when storm has already
	// been destroyed. (so cannot do this in there)
	void deleteTextureCache();
	frozenbyte::TextureCache *getTextureCache();

	void updateVideos();

  void removeImageByNode(const ListNode *node);
  void removeFontByNode(const ListNode *node);

private:
  int stormGeneration;
  LinkedList *fonts;
  LinkedList *images;
  LinkedList *videos;
	std::vector<std::string> fontResources;

	frozenbyte::TextureCache *textureCache;

};


#endif
