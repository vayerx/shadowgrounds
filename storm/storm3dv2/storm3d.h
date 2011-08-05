// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D.h"
#include "storm3d_resourcemanager.h"
#include "Storm3D_ProceduralManager.h"
#include <atlbase.h>

// Forward declarations
class IStorm3D_BoneAnimation;
class IStorm3D_Line;

//------------------------------------------------------------------
// Storm3D
//------------------------------------------------------------------
class Storm3D : public IStorm3D
{
public:
	struct RenderTarget
	{
		VC2 size;
		IStorm3D_Texture *texture;

		RenderTarget()
		:	texture(0)
		{
		}
	};

private:	

	Storm3D_Texture *CreateNewTextureInstance(int width,int height,IStorm3D_Texture::TEXTYPE textype);

	// Loaded resources
	set<IStorm3D_Texture*> textures;
	set<IStorm3D_Model*> models;
	set<IStorm3D_Scene*> scenes;
	set<IStorm3D_Font*> fonts;
	set<IStorm3D_LensFlare*> lflares;
	set<IStorm3D_Terrain*> terrains;
	set<IStorm3D_Material*> materials;
	set<IStorm3D_Mesh*> meshes;
	set<IStorm3D_Line*> lines;
	
	Storm3D_ResourceManager resourceManager;
	IStorm3D_Logger *logger;

	// Active material and mesh (optimization)
	Storm3D_Material *active_material;
	Storm3D_Mesh *active_mesh;

	// Adapters
	Storm3D_Adapter *adapters;
	int adapter_amount;
	int active_adapter;
	bool TestAdapterFeatures(int adanum);

	HWND window_handle;			// Renderwindow-handle
	bool destroy_window;

	void EnumAdaptersAndModes();
	int GetDisplayModeBPP(D3DDISPLAYMODE &mode);
	int GetDSModeBPP(D3DFORMAT &form);
	bool GetDSModeStencilSupport(D3DFORMAT &form);
	D3DFORMAT GetDSBufferModeForDisplayMode(int adapter,D3DDISPLAYMODE &mode);

	LPDIRECT3D9 D3D;									// Direct3D device
	LPDIRECT3DDEVICE9 D3DDevice;					// 3d-device
	D3DPRESENT_PARAMETERS present_params;		// Present parameters

	// gamma ramps
	D3DGAMMARAMP currentGammaRamp;
	D3DGAMMARAMP originalGammaRamp;

	bool gammaPeakEnabled;
	float gammaPeakPosition;
	float gammaPeakLowShape;
	float gammaPeakHighShape;
	float gammaPeakRed;
	float gammaPeakGreen;
	float gammaPeakBlue;

	// texture lod level
	int textureLODLevel;

	// Original backbuffer (for setrendertarget)
	LPDIRECT3DSURFACE9 bbuf_orig;
	LPDIRECT3DSURFACE9 zbuf_orig;

	// Screen size (actually backbuffer size)
	// Active displaymode can be read from adapter
	Storm3D_SurfaceInfo screen_size;

	// Rendertarget real width/height (if rendering in textures etc.)
	Storm3D_SurfaceInfo viewport_size;

	// Show mode/device info or not
	bool no_info;

	// application name and short version of it
	const char *application_name;
	const char *application_shortname;

	int allocated_models;
	int allocated_meshes;

	float timeFactor;

	int shadow_quality;
	int fake_shadow_quality;
	int lighting_quality;
	bool vsync;
	bool enable_glow;
	bool enable_distortion;
	bool high_quality_textures;
	bool downscale_videos;
	bool highcolorrange_videos;	
	bool use_reference_driver;

	VC2I colorTargetSize;
	VC2I colorSecondaryTargetSize;
	VC2I depthTargetSize;
	CComPtr<IDirect3DTexture9> colorTarget;
	CComPtr<IDirect3DTexture9> colorSecondaryTarget;
	CComPtr<IDirect3DSurface9> depthTarget;
	VC2I proceduralTargetSize;
	CComPtr<IDirect3DTexture9> proceduralTarget;
	CComPtr<IDirect3DTexture9> proceduralOffsetTarget;

	typedef std::vector<RenderTarget> RenderTargetList;
	RenderTargetList renderTargets;

	bool enableReflection;
	bool halfReflection;
	int reflectionQuality;
	RenderTarget reflectionTarget;

	Storm3D_ProceduralManager proceduralManager;

	void createRenderTargets();
	void freeRenderTargets();

	bool needValueTargets;
	CComPtr<IDirect3DTexture9> valueTarget;
	CComPtr<IDirect3DTexture9> systemTarget;

	int antialiasing_level;
	std::string error_string;
	bool allocate_procedural_target;

	bool hasNeededBuffers();
	bool force_reset;

public:

	// 3d-supports (currect, mode depencent, not adapter depencent)
	bool support_stencil;

	void setNeededColorTarget(const VC2I &size)
	{
		if(size.x > colorTargetSize.x)
			colorTargetSize.x = size.x;
		if(size.y > colorTargetSize.y)
			colorTargetSize.y = size.y;
	}
	void setNeededSecondaryColorTarget(const VC2I &size)
	{
		if(size.x > colorSecondaryTargetSize.x)
			colorSecondaryTargetSize.x = size.x;
		if(size.y > colorSecondaryTargetSize.y)
			colorSecondaryTargetSize.y = size.y;
	}
	void setNeededDepthTarget(const VC2I &size)
	{
		if(size.x > depthTargetSize.x)
			depthTargetSize.x = size.x;
		if(size.y > depthTargetSize.y)
			depthTargetSize.y = size.y;
	}

	CComPtr<IDirect3DTexture9> getColorTarget() { return colorTarget; }
	CComPtr<IDirect3DTexture9> getColorSecondaryTarget() { return colorSecondaryTarget; }
	CComPtr<IDirect3DSurface9> getDepthTarget() { return depthTarget; }

	void createTargets();

	// Get D3D Device inline (v3)
	LPDIRECT3DDEVICE9 GetD3DDevice() const {return D3DDevice;} 
	LPDIRECT3D9 GetD3D() const {return D3D;} 

	IStorm3D_Logger *getLogger() {return logger; }

	// Active mesh inline (v3)
	const Storm3D_Mesh *GetActiveMesh() const {return active_mesh;}
	void SetActiveMesh(Storm3D_Mesh *_active_mesh) {active_mesh=_active_mesh;}

	Storm3D_ResourceManager &getResourceManager() {	return resourceManager; }

	// Resource handling (device lost)
	bool RestoreDeviceIfLost();		// returns false, if cannot be returned now
	void ReleaseDynamicResources();
	void RecreateDynamicResources();

	// Rendertarget change
	// - Newtarget must be a dynamic texture
	// - If newtarget==NULL backbuffer is returned as target.
	// - Map parameter is only used for cubemaps. 0-5 represents cubefaces in following order:
	//    0=pX, 1=nX, 2=pY, 3=nY, 4=pZ, 5=nZ (p=positive, n=negative)
	bool SetRenderTarget( Storm3D_Texture *newtarget,int map=0);	

	bool RenderSceneToArray( IStorm3D_Scene * stormScene, unsigned char * destination, int width, int height );

	// ScreenModes
	bool SetFullScreenMode(int width=640,int height=480,int bpp=16);
	bool SetWindowedMode(int width=640,int height=480,bool titlebar=false);
	bool SetWindowedMode(bool disableBuffers);
	std::string GetErrorString() { return error_string; }

	void SetShadowQuality(int quality) { shadow_quality = quality; }
	void SetFakeShadowQuality(int quality) { fake_shadow_quality = quality; }
	void SetLightingQuality(int quality) { lighting_quality = quality; }
	void EnableGlow(bool enable) { enable_glow = enable; }
	void EnableDistortion(bool enable)  { enable_distortion = enable; }
	void UseVSync(bool use_vsync) { vsync = use_vsync; }
	void EnableHighQualityTextures(bool enable) { high_quality_textures = enable; }
	void DownscaleVideos(bool enable) { downscale_videos = enable; }
	void HigherColorRangeVideos(bool enable) { highcolorrange_videos = enable; }
	void SetAntiAliasing(int quality) { antialiasing_level = quality; }
	void AllocateProceduralTarget(bool enable) { allocate_procedural_target = enable; }
	void SetReflectionQuality(int quality);
	void UseReferenceDriver(bool refdriver) { use_reference_driver = refdriver; }
	void forceReset() { force_reset = true; }

	bool hasHighQualityTextures() const { return high_quality_textures; }

	void addAdditionalRenderTargets(const VC2 &size, int amount);
	void setProceduralTextureSize(const VC2I &size);
	IStorm3D_Texture *getRenderTarget(int index);
	Storm3D_ProceduralManager &getProceduralManagerImp() { return proceduralManager; };
	IStorm3D_ProceduralManager &getProceduralManager() { return proceduralManager; };
	
	void enableLocalReflection(bool enable, float height);
	IStorm3D_Texture *getReflectionTexture() { if(enableReflection) return reflectionTarget.texture; else return 0; };

	// Gamma and other color settings
	// --jpk
	void RestoreGammaRamp();
	void SetGammaRamp(float gamma, float brightness, float contrast,
		float red, float green, float blue, bool calibrate);

	void SetGammaPeak(bool peakEnabled, float peakPosition, 
    float peakLowShape, float peakHighShape, 
    float peakRed, float peakGreen, float peakBlue);

	// Set application name (window title)
	void SetApplicationName(const char *shortName, const char *applicationName);

	// Set texture LOD (mipmap) level
	void SetTextureLODLevel(int lodlevel);
	int GetTextureLODLevel();

	// Returns status information in printable format
	// Returned char* is newly allocated, delete it once done with it.
	char *GetPrintableStatusInfo();
	void DeletePrintableStatusInfo(char *buf);

	void setGlobalTimeFactor(float timeFactor);

	Storm3D_SurfaceInfo GetCurrentDisplayMode(); 
	Storm3D_SurfaceInfo GetScreenSize();	// backbuffer dimension (normally same as displaymode, but not in windowed mode)

	// Screenshot
	void TakeScreenshot(const char *file_name);
	IStorm3D_ScreenBuffer *TakeScreenshot(const VC2 &area);
	DWORD getScreenColorValue(const VC2 &area);

	// Texture handling
	IStorm3D_Texture *CreateNewTexture(const char *filename,DWORD tex_flags=0,DWORD tex_identity=0, const void *data=0, size_t data_size=0);
	IStorm3D_Texture *CreateNewTexture(int width,int height,IStorm3D_Texture::TEXTYPE textype); // For dynamic textures


	


	// Material handling
	IStorm3D_Material *CreateNewMaterial(const char *name);

	// Mesh handling
	IStorm3D_Mesh *CreateNewMesh();

	// Model handling
	IStorm3D_Model *CreateNewModel();

	// Scene handling
	IStorm3D_Scene *CreateNewScene();

	// Font handling
	IStorm3D_Font *CreateNewFont();

	// Lensflare handling
	IStorm3D_LensFlare *CreateNewLensFlare();

	// Terrain handling
	IStorm3D_Terrain *CreateNewTerrain( int block_size );

	// Bone handling
	IStorm3D_BoneAnimation *CreateNewBoneAnimation(const char *file_name);

	// Lines
	IStorm3D_Line *CreateNewLine();

	IStorm3D_VideoStreamer *CreateVideoStreamer(const char *file_name, IStorm3D_StreamBuilder *streamBuilder, bool loop);

	// Remove (new in v3)
	void Remove(IStorm3D_Texture *tex);
	void Remove(IStorm3D_Material *mat, IStorm3D_Mesh *mesh);
	void Remove(IStorm3D_Mesh *mesh, IStorm3D_Model_Object *object);
	void Remove(IStorm3D_Model *mod);
	void Remove(IStorm3D_Scene *sce);
	void Remove(IStorm3D_Font *font);
	void Remove(IStorm3D_LensFlare *lflare);
	void Remove(IStorm3D_Terrain *terrain);

	// Delete everything (textures,materials,models,scenes)
	void Empty();

	// Creation/delete
	Storm3D(bool no_info = false, frozenbyte::filesystem::FilePackageManager *fileManager = 0, IStorm3D_Logger *logger = 0);
	~Storm3D();

	// Log an error message
	void LogError(const char *logMessage);

	// Friends... (updated in v3)
	friend class Storm3D_Scene;
	friend class Storm3D_Mesh;
	friend class Storm3D_Line;
	friend class Storm3D_Texture;
	friend class Storm3D_Material;
	friend struct Storm3D_TerrainRendererData;
	friend class Storm3D_Spotlight;

	// added for getting timeFactor value..
	// -jpk
	friend class Storm3D_Texture_Video;
};




