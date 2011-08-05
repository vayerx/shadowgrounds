// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Texture.h"
#include <string>

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

protected:

	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// Temp system buffers for dynamic textures
	// Videomemory buffers are copied to system memory, before reading,
	// because DX8 can only lock sysmem surfaces
	LPDIRECT3DSURFACE9 dx_tempbuf_sm;

	LPDIRECT3DTEXTURE9 dx_handle;			// DX8-texture
	LPDIRECT3DCUBETEXTURE9 dx_handle_cube;	// DX8-texture (for cubemaps)
	TEXTYPE textype;						// Cube/Basic/Rendersurface

	char *filename;					// Filename
	DWORD tex_identity;				// Used if no sharing wanted
	DWORD texloadflags;				// Onfy for loaded textures (NoCompress etc.)
	
	int refcount;					// Reference count (for delete)
	int width,height;				// Texture size

	int prioritycount;
	bool auto_release;

public:

	static void *classId();
	virtual void *getId() const;

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

	// DX buffer handling (for lost devices)
	void ReleaseDynamicDXBuffers();
	void ReCreateDynamicDXBuffers();

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

	bool lock(D3DLOCKED_RECT &rect);
	void unlock();

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
	LPDIRECT3DTEXTURE9 *frames;	// DX8 textures for frames
	D3DFORMAT dx_texformat;		// Bitformat for DX8 textures

	int frame_amount;		// Number of frames
	int frame;				// Current frame

	int framechangetime;	// Automatic frame change time (in milliseconds)
	int framechangecounter;	// Counter (milliseconds)

	VIDEOLOOP loop_params;	// Loop parameters

	// AVI file info
	int width,height,bpp;
	DWORD last_time;

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

	//friend Storm3D;
};
