// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"



//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Texture;



//------------------------------------------------------------------
// IStorm3D_Texture (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Texture
{

public:

	// Dynamic texture types
	enum TEXTYPE
	{
		TEXTYPE_BASIC				=0,
		TEXTYPE_CUBE				=1,
		TEXTYPE_BASIC_RENDER		=2,		// 3d-rendertarget
		TEXTYPE_CUBE_RENDER		=3,		// 3d-rendertarget
		TEXTYPE_BASIC_RPOOL		=4,			// This texture is created to same texturepool as rendertarget (so copying from rendertarget is much faster than with basic textures)
		TEXTYPE_DYNAMIC,
		TEXTYPE_DYNAMIC_LOCKABLE,
		TEXTYPE_DYNAMIC_LOCKABLE_32,
		TEXTYPE_RAM
	};

	// Get parameters
	virtual const char *GetFilename()=0;		// Returns NULL, if dynamic texture
	virtual DWORD GetTexIdentity()=0;

	// Texture edit etc
	virtual Storm3D_SurfaceInfo GetSurfaceInfo()=0;
	virtual void CopyTextureToAnother(IStorm3D_Texture *other)=0;
	virtual void Copy32BitSysMembufferToTexture(DWORD *sysbuffer)=0;
	virtual void CopyTextureTo32BitSysMembuffer(DWORD *sysbuffer)=0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	// Video looping parameters (for videotextures)
	enum VIDEOLOOP
	{
		VIDEOLOOP_DEFAULT				=0,		// Default looping (repeat from start when finished)
		VIDEOLOOP_PINGPONG				=1,		// Set framechangespeed to negative when finished (video is shown backwards), back to positive when at beginning (etc.etc...)
		VIDEOLOOP_STOP_AT_END			=2,		// Show video one time and stop at end (leaving last frame visible)
		VIDEOLOOP_PP_STOP_AT_BEGINNING	=3		// Show video one time (to end and back to beginning) and stop (leaving first frame visible)
	};

	// Video texture special (only for videotextures)
	// You can check if a texture is a videotexture by calling VideoGetFrameAmount()
	// If the result other than 1, the texture is a videotexture.
	virtual void VideoSetFrame(int num)=0;
	virtual void VideoSetFrameChangeSpeed(int millisecs)=0;
	virtual void VideoSetLoopingParameters(VIDEOLOOP params)=0;
	virtual int VideoGetFrameAmount()=0;
	virtual int VideoGetCurrentFrame()=0;

	virtual void AddHighPriority() = 0;
	virtual void RemoveHighPriority() = 0;

	// Delete with this (v3)
	virtual bool Release()=0;
	virtual void swapTexture(IStorm3D_Texture *other) = 0;

	virtual void saveToFile( const char * filename ) = 0;
	virtual void loadFromFile( const char * filename ) = 0;

	// Virtual destructor (DO NOT USE: Reference counting does not work!)
	virtual ~IStorm3D_Texture() {};
};


