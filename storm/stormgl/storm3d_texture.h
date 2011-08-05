// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#ifdef _MSC_VER
#include <windows.h>
#endif

#include <string>
#include <GL/glew.h>
#include "IStorm3D_Texture.h"
#include "IStorm3D_Logger.h"
#include "storm3d_common_imp.h"
#include "treader.h"

class IStorm3D_Logger;

void initTextureBank(IStorm3D_Logger *logger);
void freeTextureBank();
std::string findTexture(const char *name);

//------------------------------------------------------------------
// Storm3D_Texture
//------------------------------------------------------------------
class Storm3D_Texture : public IStorm3D_Texture
{
	bool decentAlpha;

	bool loadDDS(const char *filename, const char *fileData);

protected:

	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	GLuint texhandle;
	GLenum glTextype;
	GLenum texfmt;
	TEXTYPE textype;						// Cube/Basic/Rendersurface

	char *filename;					// Filename
	DWORD tex_identity;				// Used if no sharing wanted
	DWORD texloadflags;				// Onfy for loaded textures (NoCompress etc.)
	
	int refcount;					// Reference count (for delete)
	int width,height;				// Texture size

	int prioritycount;
	bool auto_release;

public:

	int getWidth() const { return width; }
	int getHeight() const { return height; }
	bool hasDecentAlpha() const { return decentAlpha; }
	void setAutoRelease(bool auto_release_) { auto_release = auto_release_; }

	// Compare (used when new texture is created -> saves memory)
	bool IsIdenticalWith(const char *_filename,DWORD texloadflags,DWORD tex_identity) const;

	// Get parameters
	const char *GetFilename();		// Returns NULL, if dynamic texture
	DWORD GetTexIdentity();

	// Texture edit etc
	Storm3D_SurfaceInfo GetSurfaceInfo();
	void CopyTextureToAnother(IStorm3D_Texture *other);
	void Copy32BitSysMembufferToTexture(DWORD *sysbuffer);
	void CopyTextureTo32BitSysMembuffer(DWORD *sysbuffer);
	void CopyTextureTo8BitSysMembuffer(BYTE *sysbuffer);	// Creates grayscale color from green color component

	void AddHighPriority();
	void RemoveHighPriority();

	// Reference...
	// Used by Storm3D:
	// AddRef: Called when a new texture is created with CreateNewTexture(),
	//			and the new texture is identical with this.
	// Release: This function is used instead of delete(destructor), because
	//				it deletes the texture only when there are no references
	//				to it left. (Use delete only when deleting all materials
	//				also at the same time). Returns true, if texture is deleted.
	void AddRef();
	bool Release();

	// Get properties
	bool IsCube();
	bool IsRenderTarget();

	// Applies texture to a rendering stage
	void Apply(int stage);

	// Video texture special
	virtual void AnimateVideo();			// animates video (material calls, when used)
	virtual void VideoSetFrame(int num);
	virtual void VideoSetFrameChangeSpeed(int millisecs);
	virtual void VideoSetLoopingParameters(VIDEOLOOP params);
	virtual int VideoGetFrameAmount();
	virtual int VideoGetCurrentFrame();

	void swapTexture(IStorm3D_Texture *other);

	void saveToFile( const char * filename );
	void loadFromFile( const char * filename );

	// Creation/delete
	// Loaded textures
	Storm3D_Texture(Storm3D *Storm3D2,const char *_filename,
		DWORD texloadflags,DWORD tex_identity=0,
		const void *data=NULL, size_t data_size=0); // use data pointer if file is already loaded to memory
	// Dynamic textures
	Storm3D_Texture(Storm3D *Storm3D2,int width,int height,TEXTYPE ttype);
	// Videotextures
	Storm3D_Texture(Storm3D *Storm3D2);
	~Storm3D_Texture();

	friend class Storm3D;
	friend class Storm3D_Scene;
};


//------------------------------------------------------------------
// Storm3D_Texture_Video
//------------------------------------------------------------------
class Storm3D_Texture_Video : public Storm3D_Texture
{
	std::vector<GLuint> frames;	// textures for frames

	int frame_amount;		// Number of frames
	int frame;				// Current frame

	int framechangetime;	// Automatic frame change time (in milliseconds)
	int framechangecounter;	// Counter (milliseconds)

	VIDEOLOOP loop_params;	// Loop parameters

	// AVI file info
	int width,height,bpp;
	DWORD last_time;

	char *buf;
	GLuint handle;

	boost::shared_ptr<TReader> reader;

	// AVI load
	void ReadAVIVideoInfo();
	void LoadAVIVideoFrames();

public:

	static void *classId();
	void *getId() const;

	// Virtual functions
	void AnimateVideo();			// animates video (material calls, when used)
	void VideoSetFrame(int num);
	void VideoSetFrameChangeSpeed(int millisecs);
	void VideoSetLoopingParameters(VIDEOLOOP params);
	int VideoGetFrameAmount();
	int VideoGetCurrentFrame();

	void swapTexture(IStorm3D_Texture *other);

	// Creation/delete
	Storm3D_Texture_Video(Storm3D *Storm3D2,const char *_filename,DWORD tex_identity=0);
	~Storm3D_Texture_Video();
};
