
#ifndef OGUIDRIVER_H
#define OGUIDRIVER_H

#ifdef _MSC_VER
#pragma warning(disable : 4290)
#endif
#include "OguiException.h"
#include "IOguiImage.h"
#include "IOguiFont.h"

class IStorm3D_StreamBuilder;
class IStorm3D_VideoStreamer;

class IOguiDriver
{
public:
	virtual ~IOguiDriver() {}
	virtual IOguiImage *LoadOguiImage(const char *filename) throw (OguiException *) = 0;
	virtual IOguiImage *LoadOguiImage(int width, int height) throw (OguiException *) = 0;
	virtual IOguiImage *GetOguiRenderTarget(int index) = 0;
	virtual IOguiImage *LoadOguiVideo( const char *filename, IStorm3D_StreamBuilder *streamBuilder ) = 0;
	virtual IOguiImage *ConvertVideoToImage( IStorm3D_VideoStreamer* stream, IStorm3D_StreamBuilder *streamBuilder ) = 0;


	virtual void updateVideos() = 0;
	virtual IOguiFont *LoadFont(const char *filename) throw (OguiException *) = 0;
};

#endif

