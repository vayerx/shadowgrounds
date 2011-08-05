// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#ifdef NVPERFSDK
#include "NVPerfSDK.h"
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#ifdef _MSC_VER
#include <windows.h>
#endif

#include <GL/glew.h>
#include "SDL_ttf.h"
#include "storm3d.h"
#include "Iterator.h"
#include "storm3d_adapter.h"
#include "storm3d_texture.h"
#include "storm3d_material.h"
#include "storm3d_model.h"
#include "storm3d_mesh.h"
#include "storm3d_scene.h"
#include "storm3d_font.h"
#include "storm3d_terrain.h"
#include "storm3d_terrain_renderer.h"
#include "storm3d_spotlight.h"
#include "storm3d_fakespotlight.h"

#include "Storm3D_Bone.h"
#include "Storm3D_Line.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_videostreamer.h"
#include "IStorm3D_Logger.h"
#include "../../filesystem/file_package_manager.h"
#include "../../filesystem/ifile_package.h"
#include "../../filesystem/standard_package.h"
#include "../../filesystem/input_stream_wrapper.h"

using namespace frozenbyte;

// HACK: for statistics
extern int storm3d_model_boneanimation_allocs;
extern int storm3d_model_loads;
extern int storm3d_model_bone_loads;
extern int storm3d_model_objects_created;
extern int storm3d_model_allocs;
extern int storm3d_mesh_allocs;
extern int storm3d_material_allocs;
extern int storm3d_bone_allocs;
extern int storm3d_texture_allocs;
extern int storm3d_dip_calls;
extern int storm3d_model_objects_tested;
extern int storm3d_model_objects_rough_passed;
extern int storm3d_model_objects_passed;

extern bool gl_initialized;

class IStorm3D_BoneAnimation;

// http://graphics.stanford.edu/~seander/bithacks.html
	void toNearestPow(int &v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
	}

namespace {
	struct RenderTargetSorter
	{
		bool operator() (const Storm3D::RenderTarget &a, const Storm3D::RenderTarget &b) const
		{
			int ax = int(2048.f * a.size.x);
			int bx = int(2048.f * b.size.x);
			if(ax != bx)
				return ax < bx;

			int ay = int(2048.f * a.size.y);
			int by = int(2048.f * b.size.y);
			return ay < by;
		}
	};

	bool equals(const VC2 &a, const VC2 &b, float epsilon)
	{
		if(fabsf(a.x - b.x) > epsilon)
			return false;
		if(fabsf(a.y - b.y) > epsilon)
			return false;

		return true;
	}
} // unnamed

//! Check that necessary render targets exist
/*!
	\return true if targets exist
*/
bool Storm3D::hasNeededBuffers()
{
	std::string e = "Failed creating needed render targets - out of video memory?";

	if(!colorTarget)
	{
		error_string = e;
		return false;
	}
	if(!colorSecondaryTarget)
	{
		error_string = e;
		return false;
	}
	if(!depthTarget)
	{
		error_string = e;
		return false;
	}

	if(!Storm3D_TerrainRenderer::hasNeededBuffers())
	{
		error_string = e;
		return false;
	}

	return true;
}

//! Create texture targets
void Storm3D::createTargets() {
	glErrors();
	if (colorTarget) {
		colorTarget.reset();
	}

	if (colorSecondaryTarget) {
		colorSecondaryTarget.reset();
	}

	if (depthTarget) {
		depthTarget.reset();
	}

	if (colorTargetSize.x > 0 && colorTargetSize.y > 0)
	{
		toNearestPow(colorTargetSize.x);
		toNearestPow(colorTargetSize.y);

		colorTarget = glTexWrapper::rgbaTexture(colorTargetSize.x, colorTargetSize.y);
	}

	if (colorSecondaryTargetSize.x > 0 && colorSecondaryTargetSize.y > 0)
	{
		toNearestPow(colorSecondaryTargetSize.x);
		toNearestPow(colorSecondaryTargetSize.y);

		colorSecondaryTarget = glTexWrapper::rgbaTexture(colorSecondaryTargetSize.x, colorSecondaryTargetSize.y);
	}

	depthTargetSize.x = max(depthTargetSize.x, colorTargetSize.x);
	depthTargetSize.x = max(depthTargetSize.x, colorSecondaryTargetSize.x);
	depthTargetSize.y = max(depthTargetSize.y, colorTargetSize.y);
	depthTargetSize.y = max(depthTargetSize.y, colorSecondaryTargetSize.y);

	if (depthTargetSize.x > 0 && depthTargetSize.y > 0) {
		depthTarget = glTexWrapper::depthStencilTexture(depthTargetSize.x, depthTargetSize.y);
	}

	if(allocate_procedural_target && proceduralTargetSize.x && proceduralTargetSize.y)
	{
		if(proceduralTarget) {
			proceduralTarget.reset();
		}

		int xs = proceduralTargetSize.x;
		int ys = proceduralTargetSize.y;
		if(textureLODLevel)
		{
			xs >>= textureLODLevel;
			ys >>= textureLODLevel;
		}

		proceduralTarget = glTexWrapper::rgbaTexture(xs, ys);

		if(proceduralOffsetTarget) {
			proceduralOffsetTarget.reset();
		}

		proceduralOffsetTarget = glTexWrapper::rgbaTexture(xs / 2, ys / 2);

		if(proceduralTarget)
			proceduralManager.setTarget(proceduralTarget, proceduralOffsetTarget);
	}

	if(needValueTargets)
	{
		if(valueTarget) {
			valueTarget.reset();
		}

		if(systemTarget) {
			systemTarget.reset();
		}

		valueTarget = glTexWrapper::rgbaTexture(1, 1);
		systemTarget = glTexWrapper::rgbaTexture(1, 1);
	}

	glErrors();
}

//! Constructor
Storm3D::Storm3D(bool _no_info, filesystem::FilePackageManager *fileManager, IStorm3D_Logger *logger_) : 
	logger(logger_),
	active_material((Storm3D_Material*)1),	// NULL is not right!
	active_mesh(NULL),
	display_modes(NULL),
	textureLODLevel(0),
	no_info(_no_info),
	application_name(NULL),
	application_shortname(NULL),
	allocated_models(0),
	allocated_meshes(0),
	shadow_quality(100),
	fake_shadow_quality(100),
	lighting_quality(100),
	vsync(false),
	enable_glow(false),
	enable_distortion(false),
	high_quality_textures(true),
	downscale_videos(false),
	colorTarget(),
	colorSecondaryTarget(),
	depthTarget(),
	proceduralTarget(),
	proceduralOffsetTarget(),
	enableReflection(false),
	halfReflection(false),
	reflectionQuality(0),
	proceduralManager(*this),
	needValueTargets(false),
	valueTarget(),
	systemTarget(),
	antialiasing_level(0),
	allocate_procedural_target(true),
	force_reset(false),
	glew_initialized(false),
	fbo(NULL) // this can only be created after glewInit()
{
	if(fileManager)
		filesystem::FilePackageManager::setInstancePtr(fileManager);
	else
	{
		boost::shared_ptr<filesystem::IFilePackage> standardPackage(new filesystem::StandardPackage());
		filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
		manager.addPackage(standardPackage, 0);
	}

	initTextureBank(logger);

	// Create iterators
	ITTexture=new ICreateIM_Set<IStorm3D_Texture*>(&(textures));
	ITMaterial=new ICreateIM_Set<IStorm3D_Material*>(&(materials));
	ITModel=new ICreateIM_Set<IStorm3D_Model*>(&(models));
	ITScene=new ICreateIM_Set<IStorm3D_Scene*>(&(scenes));
	ITFont=new ICreateIM_Set<IStorm3D_Font*>(&(fonts));
	ITMesh=new ICreateIM_Set<IStorm3D_Mesh*>(&(meshes));
	ITTerrain=new ICreateIM_Set<IStorm3D_Terrain*>(&(terrains));

	// Enumerate all modes
	EnumModes();

	timeFactor = 1.0f;
	gammaPeakEnabled = false;
	SetApplicationName("Storm3D", "Storm3D v2.0 - Render Window");

	currentGammaRampRed = new Uint16[256];
	currentGammaRampGreen = new Uint16[256];
	currentGammaRampBlue = new Uint16[256];
	originalGammaRampRed = new Uint16[256];
	originalGammaRampGreen = new Uint16[256];
	originalGammaRampBlue = new Uint16[256];

#ifdef NVPERFSDK
	NVPMInit();
	NVPMAddCounterByName("GPU Bottleneck");
#endif
}

//! Destructor
Storm3D::~Storm3D()
{
#ifdef NVPERFSDK
	NVPMShutdown();
#endif

	// Delete iterators
	delete ITTexture;
	delete ITMaterial;
	delete ITModel;
	delete ITScene;
	delete ITFont;
	delete ITMesh;
	delete ITTerrain;

	// Empty (models, textures, materials)
	Empty();

	Storm3D_TerrainRenderer::freeSecondaryRenderBuffers();
	Storm3D_Spotlight::freeShadowBuffers();
	Storm3D_FakeSpotlight::freeBuffers();
	Storm3D_TerrainRenderer::freeRenderBuffers();

	// Delete shader manager
	delete Storm3D_ShaderManager::GetSingleton();

	proceduralManager.reset();

	// May now leak memory, but otherwise crashes on Windows
#ifndef _MSC_VER
	while(textures.begin()!=textures.end())
	{
		delete (*textures.begin());
	}
#else
	igios_unimplemented();
#endif

	// Delete modelist
	if (display_modes) delete[] display_modes;

	freeTextureBank();

	// Delete gamma ramp tables
	delete[] currentGammaRampRed;
	delete[] currentGammaRampGreen;
	delete[] currentGammaRampBlue;
	delete[] originalGammaRampRed;
	delete[] originalGammaRampGreen;
	delete[] originalGammaRampBlue;

	if (fbo != NULL) {
		delete fbo; fbo = NULL;
	}
}

//! Create render targets
void Storm3D::createRenderTargets()
{
	const Storm3D_SurfaceInfo &info = GetScreenSize();

	for(RenderTargetList::iterator it = renderTargets.begin(); it != renderTargets.end(); ++it)
	{
		assert(!it->texture);

		const VC2 &s = it->size;
		int xs = int(s.x * info.width + .5f);
		int ys = int(s.y * info.height + .5f);

		if(textureLODLevel)
		{
			xs >>= textureLODLevel;
			ys >>= textureLODLevel;
		}

		it->texture = CreateNewTexture(xs, ys, IStorm3D_Texture::TEXTYPE_BASIC_RENDER);
		static_cast<Storm3D_Texture *> (it->texture)->AddRef();
	}

	if(enableReflection)
	{
		int xs = info.width;
		int ys = info.height;

		if(halfReflection)
		{
			xs /= 2;
			ys /= 2;
		}

		reflectionTarget.texture = CreateNewTexture(xs, ys, IStorm3D_Texture::TEXTYPE_BASIC_RENDER);
		static_cast<Storm3D_Texture *> (reflectionTarget.texture)->AddRef();
	}
}

//! Free render targets
void Storm3D::freeRenderTargets()
{
	for(RenderTargetList::iterator it = renderTargets.begin(); it != renderTargets.end(); ++it)
	{
		if(it->texture)
		{
			it->texture->Release();
			it->texture->Release();
			it->texture = 0;
		}
	}

	if(reflectionTarget.texture)
	{
		reflectionTarget.texture->Release();
		reflectionTarget.texture->Release();
		reflectionTarget.texture = 0;
	}
}

//! Set texture as render target
/*!
	\param newtarget new texture target
	\param map
	\return true if succeeded
*/
bool Storm3D::SetRenderTarget(Storm3D_Texture *newtarget,int map)
{
	// NULL means, returning to original target
	if (newtarget == NULL)
	{
		fbo->disable();
		// Set viewport
		viewport_size=screen_size;
		glViewport(0, 0, screen_size.width, screen_size.height);
		return true;
		// Everything went ok
	}

	// Is this texture a render target?
	if (!newtarget->IsRenderTarget()) return false;

	// Set the target
	if (!newtarget->IsCube())
	{
		if(newtarget->texhandle)
		{
            fbo->setRenderTarget(newtarget->texhandle, newtarget->width, newtarget->height, GL_TEXTURE_2D);
			if (!fbo->validate()) {
				igiosWarning("setRenderTarget failed, texture format: %x\n", newtarget->texfmt);
				igiosWarning("size: %dx%d type:%d\n", newtarget->width, newtarget->height, newtarget->textype);
				fbo->disable();
				return false;
			}

			// Set viewport
			viewport_size.width=newtarget->width;
			viewport_size.height=newtarget->height;
			glViewport(0, 0, newtarget->width,newtarget->height);
		}
	}
	else
	{
		// Test
		if (map<0) return false;
		if (map>5) return false;

		igios_unimplemented();
		return false; // due to igios_unimplemented()
		// Get cube face
	}
	return true;
}

//! Render scene to array
/*!
	\param stormScene scene
	\param dest destination array
	\param width width
	\param height height
	\return true if successful
*/
bool Storm3D::RenderSceneToArray( IStorm3D_Scene * stormScene, unsigned char * dest, int width, int height )
{
	Storm3D_Texture * target = CreateNewTextureInstance( width, height, IStorm3D_Texture::TEXTYPE_BASIC_RENDER );

	if( target )
	{
		//stormScene->RenderSceneToDynamicTexture( target );
		SetRenderTarget ( target, 0 );
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClearDepth(1.0);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		stormScene->RenderScene();
		SetRenderTarget ( NULL, 0 );

		target->CopyTextureTo32BitSysMembuffer((DWORD *) dest);
		Remove( target );
		return true;
	}
	else
	{
        igiosErrorMessage("Storm: Couldn't create render target.");
	}

	return false;
}

//! Set up fullscreen mode
/*!
	\param width screen width
	\param height screen height
	\param bpp bits per pixel
	\return true if successful
*/
bool Storm3D::SetFullScreenMode(int width, int height, int bpp)
{
  static const unsigned int numModes = 10;
  static const int modes[numModes][2] = {
    { 2560,1600 },
    { 1920,1200 },
    { 1600,1050 },
    { 1600,1200 },
    { 1440,900  },
    { 1280,800  },
    { 1280,1024 },
    { 1024,768  },
    { 800 ,600  },
    { 640 ,480  },
  };
	// Test if adapter has all the needed features
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	// Try to set the requested video mode
	if (SDL_SetVideoMode(width, height, 0, SDL_OPENGL | SDL_FULLSCREEN)) {
		return SetDisplayMode(width, height);
	} else {
		// FALLBACK: Search for closest display mode match
		for(unsigned int i=0;i<numModes;++i)
			if (width >= modes[i][0] && height >= modes[i][1]) {
				if (SDL_SetVideoMode(modes[i][0], modes[i][1], 0, SDL_OPENGL | SDL_FULLSCREEN))
					return SetDisplayMode(modes[i][0],modes[i][1]);
			}
		// Still didnt manage to set a display mode, signal error
		error_string = "Failed to set adapter display mode";
		return false;
	}
}

//! Set up windowed mode
/*!
	\param width window width
	\param height window height
	\param titlebar
	\return true if successful
*/
bool Storm3D::SetWindowedMode(int width, int height, bool titlebar)
{
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	if (!SDL_SetVideoMode(width, height, 0, SDL_OPENGL))
	{
		error_string = "Failed to set adapter display mode";
		return false;
	}

	return SetDisplayMode(width, height);
}

//! Set up windowed mode
/*!
	\param disableBuffers
	\return true if successful
*/
bool Storm3D::SetWindowedMode(bool disableBuffers = false)
{
	igios_unimplemented();
	return true;
}

//! Set up display mode
/*!
	\param width width
	\param height height
	\return true if successful
*/
bool Storm3D::SetDisplayMode(int width, int height)
{
	int temp;
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &temp);
	if (temp > 0)
		support_stencil = true;

	screen_size.width = width;
	screen_size.height = height;
	viewport_size = screen_size;

	if (!glew_initialized) {
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			error_string = "GLEW init failed: ";
			error_string.append((const char *) glewGetErrorString(err));
			return false;
		}
		glew_initialized = true;
		gl_initialized = true;

		if (!GLEW_ARB_vertex_program) {
			error_string = "ARB_vertex_program not supported";
			return false;
		}

		if (!GLEW_ARB_fragment_program) {
			error_string = "ARB_fragment_program not supported";
			return false;
		}

		if (!GLEW_EXT_framebuffer_object) {
			error_string = "EXT_framebuffer_object not supported";
			return false;
		}

		if (!GLEW_NV_texgen_reflection) {
			error_string = "NV_texgen_reflection not supported";
			return false;
		}

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
		if (GLEW_EXT_texture_filter_anisotropic) {
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxanisotropy);
		} else {
			maxanisotropy = 1;
		}
	}

	SDL_WM_SetCaption(application_name, NULL);
	glViewport(0, 0, width, height);

	if (!TTF_WasInit())
		TTF_Init();

	// Get original gamma ramp
	SDL_GetGammaRamp(originalGammaRampRed, originalGammaRampGreen, originalGammaRampBlue);

	gammaPeakEnabled = false;

	setNeededDepthTarget(VC2I(width, height));

	Storm3D_TerrainRenderer::querySizes(*this, true);
	Storm3D_Spotlight::querySizes(*this, shadow_quality);
	Storm3D_FakeSpotlight::querySizes(*this, fake_shadow_quality);
	createTargets();
	createRenderTargets();
	Storm3D_TerrainRenderer::createRenderBuffers(*this, lighting_quality);
	Storm3D_Spotlight::createShadowBuffers(*this, shadow_quality);
	Storm3D_FakeSpotlight::createBuffers(*this, fake_shadow_quality);
	Storm3D_TerrainRenderer::createSecondaryRenderBuffers(*this, enable_glow);

	// Create shader manager
	new Storm3D_ShaderManager();
	Storm3D_ShaderManager::GetSingleton()->CreateShaders();

	if(!hasNeededBuffers())
		return false;

	// GLEW must be initialized before this
	fbo = new Framebuffer();

	// Everything OK
	return true;
}

//! Sets quality of reflections
/*!
	\param quality quality of reflections, 0 = no reflections, >= 50 no halfReflections
*/
void Storm3D::SetReflectionQuality(int quality)
{
	if(quality == 0)
		enableReflection = false;
	else if(quality >= 50)
	{
		halfReflection = false;
		enableReflection = true;
	}
	else
	{
		halfReflection = true;
		enableReflection = true;
	}
}

//! Adds render targets
/*!
	\param size 
	\param amount
*/
void Storm3D::addAdditionalRenderTargets(const VC2 &size, int amount)
{
	RenderTarget target;
	target.size = size;

	for(int i = 0; i < amount; ++i)
		renderTargets.push_back(target);
}

//! Sets the size of procedural textures
/*!
	\param size size of textures
*/
void Storm3D::setProceduralTextureSize(const VC2I &size)
{
	proceduralTargetSize = size;
}

//! Get render target
/*!
	\param index index of render target
	\return render target
*/
IStorm3D_Texture *Storm3D::getRenderTarget(int index)
{
	if(index >= 0 && index < int(renderTargets.size()))
		return renderTargets[index].texture;

	return 0;
}

extern float reflection_height;

//! Enable local reflection
/*!
	\param enable
	\param height
*/
void Storm3D::enableLocalReflection(bool enable, float height)
{
	enableReflection = enable;
	reflection_height = height;
}

//! Restores gamma ramp to original value
void Storm3D::RestoreGammaRamp()
{
	SDL_SetGammaRamp(originalGammaRampRed, originalGammaRampGreen, originalGammaRampBlue);
}

//! Sets gamma ramp
/*!
	\param gamma
	\param brightness
 */
void Storm3D::SetGammaRamp(float gamma, float brightness, float contrast, float red, float green, float blue, bool calibrate)
{
	if(gamma <= 0.01f)
		gamma = 0.01f;

	for (int i = 0; i < 256; i++)
	{
		float gammaVal = powf(i / 255.f, 1.f / gamma);
		float val = 32767.5f + 65535.0f * ((gammaVal - 0.5f) * contrast + (brightness - 1.0f));		
		float r = (val * red);
		float g = (val * green);
		float b = (val * blue);

		if (gammaPeakEnabled)
		{
			float ifl = float(i) / 256.0f;
			if (ifl < gammaPeakPosition)
			{
				if (ifl > gammaPeakPosition - gammaPeakLowShape)
				{
					float fact = 1.0f - ((gammaPeakPosition - ifl) / gammaPeakLowShape);
					r += gammaPeakRed * fact * 65536.0f;
					g += gammaPeakGreen * fact * 65536.0f;
					b += gammaPeakBlue * fact * 65536.0f;
				}
			} 
			else 
			{
				if (ifl < gammaPeakPosition + gammaPeakHighShape)
				{
					float fact = 1.0f - ((ifl - gammaPeakPosition) / gammaPeakHighShape);
					r += gammaPeakRed * fact * 65536.0f;
					g += gammaPeakGreen * fact * 65536.0f;
					b += gammaPeakBlue * fact * 65536.0f;
				}
			}
		}

		if (r < 0) 
			r = 0;
		if (r > 65535) 
			r = 65535;
		if (g < 0) 
			g = 0;
		if (g > 65535) 
			g = 65535;
		if (b < 0) 
			b = 0;
		if (b > 65535) 
			b = 65535;

		currentGammaRampRed[i] = (Uint16)r;
		currentGammaRampGreen[i] = (Uint16)g;
		currentGammaRampBlue[i] = (Uint16)b;
	}

	/*
	DWORD flags;
	if (calibrate)
		flags = D3DSGR_CALIBRATE;
	else
		flags = D3DSGR_NO_CALIBRATION;
	*/

	SDL_SetGammaRamp(currentGammaRampRed, currentGammaRampGreen, currentGammaRampBlue);
}

//! Sets gamma peak
/*!
	\param peakEnabled
	\param peakPosition
	\param peakLowShape
	\param peakHighShape
	\param peakRed
	\param peakGreen
	\param peakBlue
*/
void Storm3D::SetGammaPeak(bool peakEnabled, float peakPosition, float peakLowShape, float peakHighShape, float peakRed, float peakGreen, float peakBlue)
{
	if (peakEnabled)
	{
		this->gammaPeakEnabled = true;
		this->gammaPeakPosition = peakPosition;
		this->gammaPeakLowShape = peakLowShape;
		this->gammaPeakHighShape = peakHighShape;
		this->gammaPeakRed = peakRed;
		this->gammaPeakGreen = peakGreen;
		this->gammaPeakBlue = peakBlue;

		if (gammaPeakLowShape < 0.001f)
			gammaPeakLowShape = 0.001f;
		if (gammaPeakHighShape < 0.001f)
			gammaPeakHighShape = 0.001f;
	} else {
		this->gammaPeakEnabled = false;
	}
}

//! Set LOD level of textures
/*!
	\param lodlevel LOD level
 */
void Storm3D::SetTextureLODLevel(int lodlevel)
{
	textureLODLevel = lodlevel;
}

//! Get LOD level of textures
/*!
	\return LOD level
 */
int Storm3D::GetTextureLODLevel()
{
	return textureLODLevel;
}

//! Get the printable status information buffer
/*!
	\return status information
 */
char *Storm3D::GetPrintableStatusInfo()
{
	char *buf = new char[2048 + 1];

	if (this->allocated_models != storm3d_model_allocs)
	{
		sprintf(buf, "Storm3D status info follows:\nModel allocation amounts mismatch, internal error.\n");
	} else
	{
		sprintf(buf, "Models allocated: %d\nModel meshes allocated: %d\nModel loads so far: %d\nBone loads so far: %d\nBone animations allocated: %d\nModel objects allocated: %d\nMeshes allocated: %d\nMaterials allocated: %d\nBones allocated: %d\nTextures allocated: %d\nDIP calls %d\nObjects tested %d\nObjects rough passed %d\nObjects passed %d\n\n",
			allocated_models, allocated_meshes, storm3d_model_loads, storm3d_model_bone_loads, storm3d_model_boneanimation_allocs, storm3d_model_objects_created, storm3d_mesh_allocs, storm3d_material_allocs, storm3d_bone_allocs, storm3d_texture_allocs, storm3d_dip_calls, storm3d_model_objects_tested, storm3d_model_objects_rough_passed, storm3d_model_objects_passed);
	}

	return buf;
}

//! Deletes the printable status information buffer
/*!
   need this just to easily detour the problems caused by memorymanager usage.
   also, seems like a more "correct" way to delete resources...
   what storm3d allocates, it also deletes.
*/
void Storm3D::DeletePrintableStatusInfo(char *buf)
{
	delete [] buf;
}

//! Sets the global time factor
/*!
	\param timeFactor time factor
*/
void Storm3D::setGlobalTimeFactor(float timeFactor)
{
	this->timeFactor = timeFactor;
}

//! Enumerate all available video modes
void Storm3D::EnumModes(void)
{
	SDL_Rect **modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);

	if (modes != NULL) {
		int i;
		for (i = 0; modes[i] != NULL; i++) { }

		display_modes = new SDL_Rect[i];
		for (i = 0; modes[i] != NULL; i++) {
			display_modes[i].w = modes[i]->w;
			display_modes[i].h = modes[i]->h;
		}
	}
}

//! Set application name
/*!
	\param shortName short name of application
	\param applicationName name of application
 */
void Storm3D::SetApplicationName(const char *shortName, const char *applicationName)
{
	this->application_name = applicationName;
	this->application_shortname = shortName;
}

//! Get display mode
/*!
	\return screen size
 */
Storm3D_SurfaceInfo Storm3D::GetCurrentDisplayMode()
{
	SDL_Surface *screen = SDL_GetVideoSurface();
	return Storm3D_SurfaceInfo(screen->w,screen->h,32); // always 32 bit mode with opengl
	// return *new Storm3D_SurfaceInfo(screen->w, screen->h, 32); // always 32 bit mode with opengl
}

//! Get the size of the screen
/*!
	\return screen size
 */
Storm3D_SurfaceInfo Storm3D::GetScreenSize()
{
	return viewport_size;
}


void Storm3D::TakeScreenshot(const char *file_name)
{
	igios_unimplemented();
}


IStorm3D_ScreenBuffer *Storm3D::TakeScreenshot(const VC2 &area)
{
	igios_unimplemented();
	return NULL;
}


DWORD Storm3D::getScreenColorValue(const VC2 &area)
{
	igios_unimplemented();
	return 0;
}

//! Texture handling
/*!
	\param originalFilename
	\param texloadcaps
	\param texidentity
*/
IStorm3D_Texture *Storm3D::CreateNewTexture(const char *originalFilename, DWORD texloadcaps, DWORD texidentity, const void *data, size_t data_size)
{
	Storm3D_Texture *ex_tex = 0;
	std::string originalString = originalFilename;

	{
		for(unsigned int i = 0; i < originalString.size(); ++i)
		{
			if(originalString[i] == '\\')
				originalString[i] = '/';

			originalString[i] = tolower(originalString[i]);
		}

		std::string::size_type aviPos = originalString.rfind(".avi");
		if (aviPos != std::string::npos) {
			originalString.replace(aviPos, 4, ".ogg");
		}

		for (set<IStorm3D_Texture*>::iterator it=textures.begin();it!=textures.end();it++)
		{
			// Typecast to simplify code
			Storm3D_Texture *tx=(Storm3D_Texture*)*it;

			// Ask the texture if it's identical as the new texture
			if (tx->IsIdenticalWith(originalString.c_str(),texloadcaps,texidentity))
			{
				ex_tex=tx;
				break;
			}
		}
	}

	std::string newFileName = originalString;
	const char *filename = newFileName.c_str();

	if(!ex_tex)
	{
		newFileName = findTexture(newFileName.c_str());
		filename = newFileName.c_str();

		for (set<IStorm3D_Texture*>::iterator it=textures.begin();it!=textures.end();it++)
		{
			// Typecast to simplify code
			Storm3D_Texture *tx=(Storm3D_Texture*)*it;

			// Ask the texture if it's identical as the new texture
			if (tx->IsIdenticalWith(filename,texloadcaps,texidentity))
			{
				ex_tex=tx;
				break;
			}

		}
	}

	// Load the texture if needed, otherwise return pointer to existing texture
	if (ex_tex)
	{
		// Add texture's reference count first
		ex_tex->AddRef();
		return ex_tex;
	}
	else
	{
		if (data == NULL)
		{
			// Test if texture exists
			filesystem::FB_FILE *f = filesystem::fb_fopen(filename,"rb");
			if(f)
			{
				filesystem::fb_fclose(f);
			} else {
				if (logger != NULL)
				{
					logger->error("Storm3D::CreateNewTexture - File does not exist or is zero length.");
					logger->debug(filename);
				}
				return NULL;
			}
		}

		// Test if it's AVI texture
		bool is_avi=false;
		int sl=strlen(filename);
		if (sl>4)
		{
			if (((filename[sl-3]=='a')||(filename[sl-3]=='o'))&&
				((filename[sl-2]=='v')||(filename[sl-2]=='g'))&&
				((filename[sl-1]=='i')||(filename[sl-1]=='g'))) is_avi=true;
		}

		// Load the new texture
		Storm3D_Texture *tex = 0;
		if (is_avi)
		{
			assert(data == NULL);
			tex=new Storm3D_Texture_Video(this,filename,texidentity);
		}
		else
		{
			tex=new Storm3D_Texture(this,filename,texloadcaps,texidentity, data, data_size);
		}

		textures.insert(tex);
		return tex;
	}
}

//! Creates a new dynamic texture
/*
	\param width width
	\param height height
	\param textype textype
*/
Storm3D_Texture *Storm3D::CreateNewTextureInstance(int width,int height,IStorm3D_Texture::TEXTYPE textype)
{
	// Returns class implementation instead of just interface.
	Storm3D_Texture *tex=new Storm3D_Texture(this,width,height,textype);
	textures.insert(tex);
	return tex;
}

//! Creates a new texture
/*
	\param width width
	\param height height
	\param textype textype
*/
IStorm3D_Texture *Storm3D::CreateNewTexture(int width,int height,IStorm3D_Texture::TEXTYPE textype)
{
	return CreateNewTextureInstance(width, height, textype);
}

//! Creates a new material
/*!
	\param name name of the material
	\return new material
 */
IStorm3D_Material *Storm3D::CreateNewMaterial(const char *name)
{
	// Create new material and add it into the set
	Storm3D_Material *mat=new Storm3D_Material(this,name);

	// Return material's pointer for user modifications
	resourceManager.addResource(mat);
	return mat;
}

//! Creates a new mesh
/*!
	\return new mesh
 */
IStorm3D_Mesh *Storm3D::CreateNewMesh()
{
	// Create new model and add it into the set
	Storm3D_Mesh *mod=new Storm3D_Mesh(this, resourceManager);
	meshes.insert(mod);

	allocated_meshes++;

	// Return model's pointer for user modifications
	resourceManager.addResource(mod);
	return mod;
}

//! Creates a new mesh
/*!
	\return new mesh
 */
IStorm3D_Model *Storm3D::CreateNewModel()
{
	// Create new model and add it into the set
	Storm3D_Model *mod=new Storm3D_Model(this);
	models.insert(mod);

	allocated_models++;

	// Return model's pointer for user modifications
	return mod;
}

//! Removes a model
/*
	\param imod model to remove
*/
void Storm3D::Remove(IStorm3D_Model *imod)
{
	models.erase(imod);

	allocated_models--;
	assert(allocated_models >= 0);

	std::set<IStorm3D_Scene *>::iterator it = scenes.begin();
	for(; it != scenes.end(); ++it)
	{
		IStorm3D_Scene *scene = *it;
		scene->RemoveModel(imod);
	}
}

//! Create new scene
/*!
	\return scene
 */
IStorm3D_Scene *Storm3D::CreateNewScene()
{
	// FIXME: Check ownership
	// Create new scene and add it into the set
	Storm3D_Scene *sce=new Storm3D_Scene(this);
	scenes.insert(sce);

	// Return scene's pointer for user modifications
	return sce;
}

//! Create new font
IStorm3D_Font *Storm3D::CreateNewFont()
{
	// Create new font and add it into the set
	Storm3D_Font *font=new Storm3D_Font(this);
	fonts.insert(font);

	// Return font's pointer for user modifications
	return font;
}

//! Remove font
/*
	\param ifont font to remove
*/
void Storm3D::Remove(IStorm3D_Font *ifont)
{
	fonts.erase(ifont);
}

//! Create new terrain
/*!
	\param block_size
	\return terrain
*/
IStorm3D_Terrain *Storm3D::CreateNewTerrain(int block_size)
{
	Storm3D_Terrain *terrain = new Storm3D_Terrain(*this);
	terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, enable_glow);
	terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, enable_distortion);
	terrains.insert(terrain);

	return terrain;
}

//! Create new bone animation
/*!
	\param file_name
	\return new animation
*/
IStorm3D_BoneAnimation *Storm3D::CreateNewBoneAnimation(const char *file_name)
{
	Storm3D_BoneAnimation *ret = new Storm3D_BoneAnimation(file_name);
	
	// Don't return bone animations that failed to load!
	// --jpk
	if (ret->WasSuccessfullyLoaded())
	{
		return ret;
	} else {
		if (this->logger != NULL)
		{
			this->logger->warning("Storm3D::CreateNewBoneAnimation - Bone animation was not successfully loaded.");
			this->logger->debug(file_name);
		}

		// FIXME: leaking resources now, need some way to delete the animation
		return NULL;
	}
}

//! Create new line
/*!
	\return line
*/
IStorm3D_Line *Storm3D::CreateNewLine()
{
	IStorm3D_Line *ptr = new Storm3D_Line(this);
	lines.insert(ptr);

	return ptr;
}


IStorm3D_VideoStreamer *Storm3D::CreateVideoStreamer(const char *file_name, IStorm3D_StreamBuilder *streamBuilder, bool loop)
{
	Storm3D_VideoStreamer *streamer = new Storm3D_VideoStreamer(*this, file_name, streamBuilder, loop, downscale_videos, highcolorrange_videos);
	if(streamer->hasVideo())
		return streamer;

	delete streamer;
	return 0;
}

//! Remove texture
/*!
	\param itex texture to remove
*/
void Storm3D::Remove(IStorm3D_Texture *itex)
{
	textures.erase(itex);
}

//! Remove material
/*!
	\param imat material to remove
	\param mesh mesh using material
*/
void Storm3D::Remove(IStorm3D_Material *imat, IStorm3D_Mesh *mesh)
{
	if(mesh == 0)
		resourceManager.deleteResource(imat);
	else
		resourceManager.removeUser(imat, mesh);
}

//! Remove scene
/*!
	\param isce scene to remove
*/
void Storm3D::Remove(IStorm3D_Scene *isce)
{
	scenes.erase(isce);
}

//! Remove terrain
/*!
	\param terrain terrain to remove
*/
void Storm3D::Remove(IStorm3D_Terrain *terrain)
{
	terrains.erase(terrain);
}

//! Remove mesh
/*!
	\param imod mesh to remove
	\param object object using mesh
*/
void Storm3D::Remove(IStorm3D_Mesh *imod, IStorm3D_Model_Object *object)
{
	allocated_meshes--;
	assert(allocated_meshes >= 0);

	if(object == 0)
		resourceManager.deleteResource(imod);
	else
		resourceManager.removeUser(imod, object);
}

//! Free resources
void Storm3D::Empty()
{
	// Delete scenes
	while(scenes.begin()!=scenes.end())
	{
		delete (*scenes.begin());
	}

	// Delete terrains
	while(terrains.begin()!=terrains.end())
	{
		delete (*terrains.begin());
	}

	// Delete models
	while(models.begin()!=models.end())
	{
		delete (*models.begin());
	}

	// Delete fonts
	while(fonts.begin()!=fonts.end())
	{
		delete (*fonts.begin());
	}

	resourceManager.uninitialize();

	for(;;)
	{
		bool found = false;

		set<IStorm3D_Texture *>::iterator it = textures.begin();
		for(; it != textures.end(); ++it)
		{
			Storm3D_Texture *t = static_cast<Storm3D_Texture *> (*it);
			if(t->IsRenderTarget() || !t->auto_release)
				continue;

			found = true;
			delete (*it);
			break;
		}

		if(!found)
			break;
	}
}
