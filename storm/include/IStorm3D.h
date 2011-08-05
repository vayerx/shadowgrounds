// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <vector>
#include <string>

// Common datatypes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Scene.h"
#include "IStorm3D_Model.h"
#include "IStorm3D_Texture.h"
#include "IStorm3D_Material.h"
#include "IStorm3D_Font.h"
#include "IStorm3D_Particle.h"
#include "IStorm3D_Terrain.h"



//------------------------------------------------------------------
// Defines
//------------------------------------------------------------------

// Texture load frags
#define	TEXLOADFLAGS_NOCOMPRESS		0x00000001
#define	TEXLOADFLAGS_NOLOD		0x00000002
#define	TEXLOADFLAGS_NOWRAP		0x00000004 // when resizing non-power of two textures, don't wrap edges


namespace frozenbyte {
namespace filesystem {
	class FilePackageManager;
} // filesystem
} // frozenbyte


//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D;
class IStorm3D_BoneAnimation;
class IStorm3D_ProceduralManager;
class IStorm3D_VideoStreamer;
class IStorm3D_Line;
class IStorm3D_Logger;
class IStorm3D_StreamBuilder;

class IStorm3D_ScreenBuffer
{
public:
	virtual ~IStorm3D_ScreenBuffer() {};

	virtual const VC2I &getSize() const = 0;
	virtual const std::vector<DWORD> &getBuffer() const = 0;
};

//------------------------------------------------------------------
// IStorm3D (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D
{

public:

	// ScreenModes
	virtual bool SetFullScreenMode(int width=640,int height=480,int bpp=16)=0;
	virtual bool SetWindowedMode(int width=640,int height=480,bool titlebar=false)=0;
	virtual bool SetWindowedMode(bool disableBuffers = false)=0;
	virtual std::string GetErrorString() = 0;

	// 0 - 100
	virtual void SetShadowQuality(int quality) = 0;
	virtual void SetFakeShadowQuality(int quality) = 0;
	virtual void SetLightingQuality(int quality) = 0;
	virtual void EnableGlow(bool enable) = 0;
	virtual void EnableDistortion(bool enable) = 0;
	virtual void UseVSync(bool use_vsync) = 0;
	virtual void EnableHighQualityTextures(bool enable) = 0;
	virtual void DownscaleVideos(bool enable) = 0;
	virtual void HigherColorRangeVideos(bool enable) = 0;
	virtual void SetAntiAliasing(int quality) = 0;
	virtual void AllocateProceduralTarget(bool enable) = 0;
	virtual void SetReflectionQuality(int quality) = 0;
	virtual void UseReferenceDriver(bool refdriver) = 0;
	virtual void forceReset() = 0;

	virtual void addAdditionalRenderTargets(const VC2 &size, int amount) = 0;
	virtual void setProceduralTextureSize(const VC2I &size) = 0;
	virtual IStorm3D_Texture *getRenderTarget(int index) = 0;
	virtual IStorm3D_ProceduralManager &getProceduralManager() = 0;

	// Gamma and other color settings
	virtual void RestoreGammaRamp()=0;
	virtual void SetGammaRamp(float gamma, float brightness, float contrast,
		float red, float green, float blue, bool calibrate)=0;

	virtual void SetGammaPeak(bool peakEnabled, float peakPosition, 
    float peakLowShape, float peakHighShape, 
    float peakRed, float peakGreen, float peakBlue)=0;

	// Texture LOD (mipmap) level
	virtual void SetTextureLODLevel(int lodlevel)=0;
	virtual int GetTextureLODLevel()=0;

	// Returns status information in printable format
	// Returned char* is newly allocated, delete it once done with it.
	// (preferrably, use the below delete method to delete the buffer)
	virtual char *GetPrintableStatusInfo()=0;
	virtual void DeletePrintableStatusInfo(char *buf)=0;

	// Set the time factor used for animations / video textures
	virtual void setGlobalTimeFactor(float timeFactor)=0;

	// Set application name (window title)
	virtual void SetApplicationName(const char *shortName, const char *applicationName)=0;

	virtual Storm3D_SurfaceInfo GetCurrentDisplayMode()=0; 
	virtual Storm3D_SurfaceInfo GetScreenSize()=0;	// backbuffer dimension (normally same as displaymode, but not in windowed mode)

	// Screenshot
	virtual void TakeScreenshot(const char *file_name) = 0;
	virtual IStorm3D_ScreenBuffer *TakeScreenshot(const VC2 &area) = 0;
	virtual DWORD getScreenColorValue(const VC2 &area) = 0;

	virtual bool RenderSceneToArray( IStorm3D_Scene * stormScene, unsigned char * destination, int width, int height ) = 0;

	// Texture handling
	virtual IStorm3D_Texture *CreateNewTexture(const char *filename,DWORD tex_load_flags=0,DWORD tex_identity=0, const void *data=0, size_t data_size=0)=0;	// Use tex_identity if you do not want to share this texture resource. (reasonable for videotextures only)
	virtual IStorm3D_Texture *CreateNewTexture(int width,int height,IStorm3D_Texture::TEXTYPE textype)=0; // For dynamic textures
	//virtual void DeleteTexture(IStorm3D_Texture *tex)=0;

	// Material handling
	virtual IStorm3D_Material *CreateNewMaterial(const char *name)=0;
	//virtual void DeleteMaterial(IStorm3D_Material *mat)=0;

	// Mesh handling
	virtual IStorm3D_Mesh *CreateNewMesh()=0;
	//virtual void DeleteMesh(IStorm3D_Mesh *mesh)=0;

	// Model handling
	virtual IStorm3D_Model *CreateNewModel()=0;
	//virtual void DeleteModel(IStorm3D_Model *mod)=0;

	// Scene handling
	virtual IStorm3D_Scene *CreateNewScene()=0;
	//virtual void DeleteScene(IStorm3D_Scene *sce)=0;

	// Font handling
	virtual IStorm3D_Font *CreateNewFont()=0;
	//virtual void DeleteFont(IStorm3D_Font *font)=0;

	// Terrain handling
	virtual IStorm3D_Terrain *CreateNewTerrain( int block_size )=0;
	//virtual void DeleteTerrain(IStorm3D_Terrain *terrain)=0;

	// Bone handling
	virtual IStorm3D_BoneAnimation *CreateNewBoneAnimation(const char *file_name) = 0;

	// Lines
	virtual IStorm3D_Line *CreateNewLine() = 0;

	virtual IStorm3D_VideoStreamer *CreateVideoStreamer(const char *file_name, IStorm3D_StreamBuilder *streamBuilder, bool loop) = 0;

	// Delete everything (textures,materials,models,scenes)
	// (All user's stored pointers become illegal)
	virtual void Empty()=0;

	// Iterators
	ICreate<IStorm3D_Texture*> *ITTexture;
	ICreate<IStorm3D_Material*> *ITMaterial;
	ICreate<IStorm3D_Model*> *ITModel;
	ICreate<IStorm3D_Scene*> *ITScene;
	ICreate<IStorm3D_Font*> *ITFont;
	ICreate<IStorm3D_Mesh*> *ITMesh;
	ICreate<IStorm3D_Terrain*> *ITTerrain;

	// Use this function to create Storm3D interface
	static IStorm3D *Create_Storm3D_Interface(bool no_info = false, frozenbyte::filesystem::FilePackageManager *fileManager = 0, IStorm3D_Logger *logger = 0);

	// Virtual destructor
	virtual ~IStorm3D() {};
};


