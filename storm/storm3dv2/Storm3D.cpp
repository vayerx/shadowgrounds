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
#include <SDL.h>

#include "storm3d.h"
#include "RenderWindow.h"
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
#include <stdio.h>
#include <fstream>
#include "IStorm3D_Logger.h"
#include "../../filesystem/file_package_manager.h"
#include "../../filesystem/ifile_package.h"
#include "../../filesystem/standard_package.h"
#include "../../filesystem/input_stream_wrapper.h"
#include "../../util/Debug_MemoryManager.h"

// HACK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include "../../system/Logger.h"

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

class IStorm3D_BoneAnimation;

	void toNearestPow(int &v)
	{
		if(v <= 32)
			v = 32;
		else if(v <= 64)
			v = 64;
		else if(v <= 128)
			v = 128;
		else if(v <= 256)
			v = 256;
		else if(v <= 512)
			v = 512;
		else if(v <= 1024)
			v = 1024;
		else if(v <= 2048)
			v = 2048;
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

	D3DMULTISAMPLE_TYPE findMultiSampleType(IDirect3D9 &d3d, int active_adapter, D3DFORMAT format, int antialiasing_level, BOOL windowed)
	{
		if(antialiasing_level <= 1 || antialiasing_level > 16)
			return D3DMULTISAMPLE_NONE;

		for(int i = antialiasing_level; i > 1; --i)
		{
			D3DMULTISAMPLE_TYPE current = (D3DMULTISAMPLE_TYPE) i;
			if(SUCCEEDED(d3d.CheckDeviceMultiSampleType(active_adapter, D3DDEVTYPE_HAL, format, windowed, current, 0)))
				return current;
		}

		return D3DMULTISAMPLE_NONE;
	}

} // unnamed

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

void Storm3D::createTargets()
{
	if(colorTarget)
		colorTarget.Release();
	if(colorSecondaryTarget)
		colorSecondaryTarget.Release();
	if(depthTarget)
		depthTarget.Release();

	if(colorTargetSize.x > 0 && colorTargetSize.y > 0)
	{
		toNearestPow(colorTargetSize.x);
		toNearestPow(colorTargetSize.y);
		D3DDevice->CreateTexture(colorTargetSize.x, colorTargetSize.y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &colorTarget, 0);
	}

	if(colorSecondaryTargetSize.x > 0 && colorSecondaryTargetSize.y > 0)
	{
		toNearestPow(colorSecondaryTargetSize.x);
		toNearestPow(colorSecondaryTargetSize.y);
		D3DDevice->CreateTexture(colorSecondaryTargetSize.x, colorSecondaryTargetSize.y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &colorSecondaryTarget, 0);
	}

	depthTargetSize.x = max(depthTargetSize.x, colorTargetSize.x);
	depthTargetSize.x = max(depthTargetSize.x, colorSecondaryTargetSize.x);
	depthTargetSize.y = max(depthTargetSize.y, colorTargetSize.y);
	depthTargetSize.y = max(depthTargetSize.y, colorSecondaryTargetSize.y);

	if(depthTargetSize.x > 0 && depthTargetSize.y > 0)
		D3DDevice->CreateDepthStencilSurface(depthTargetSize.x, depthTargetSize.y, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, D3DMULTISAMPLE_NONE, FALSE, &depthTarget, 0);

	if(allocate_procedural_target && proceduralTargetSize.x && proceduralTargetSize.y)
	{
		if(proceduralTarget)
			proceduralTarget.Release();

		int xs = proceduralTargetSize.x;
		int ys = proceduralTargetSize.y;
		if(textureLODLevel)
		{
			xs >>= textureLODLevel;
			ys >>= textureLODLevel;
		}

		//D3DDevice->CreateTexture(xs, ys, 1, D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &proceduralTarget, 0);
		D3DDevice->CreateTexture(xs, ys, 1, D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &proceduralTarget, 0);

		if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_PS20)
		{
			if(proceduralOffsetTarget)
				proceduralOffsetTarget.Release();
			
			D3DDevice->CreateTexture(xs / 2, ys / 2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &proceduralOffsetTarget, 0);
			//D3DDevice->CreateTexture(xs / 2, ys / 2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &proceduralOffsetTarget, 0);
		}

		if(proceduralTarget)
			proceduralManager.setTarget(proceduralTarget, proceduralOffsetTarget);
	}

	if(needValueTargets)
	{
		if(valueTarget)
			valueTarget.Release();
		if(systemTarget)
			systemTarget.Release();

		D3DDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &valueTarget, 0);
		D3DDevice->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &systemTarget, 0);
	}
}

//------------------------------------------------------------------
// Storm3D::TestActiveAdapterFeatures
//------------------------------------------------------------------
bool Storm3D::TestAdapterFeatures(int adanum)
{
	// Test features
	if (adapters[adanum].multitex_layers<2)
	{
		MessageBox(NULL,"Multitexturing not supported. Cannot continue program.","Error: Needed adapter feature missing",0);
		return false;
	}

	// All supported that needed
	return true;
}



//------------------------------------------------------------------
// Storm3D::SetApplicationName
//------------------------------------------------------------------
void Storm3D::SetApplicationName(const char *shortName, const char *applicationName)
{
	/*
	if (this->application_name != NULL)
	{
		delete [] this->application_name;
	}
	if (this->application_shortname != NULL)
	{
		delete [] this->application_shortname;
	}

  this->application_name = new char[strlen(applicationName) + 1];
	strcpy(this->application_name, applicationName);
  this->application_shortname = new char[strlen(shortName) + 1];
	strcpy(this->application_shortname, shortName);
	*/
	this->application_name = applicationName;
	this->application_shortname = shortName;
}


//------------------------------------------------------------------
// Storm3D::SetFullScreenMode
//------------------------------------------------------------------
bool Storm3D::SetFullScreenMode(int width,int height,int bpp)
{
	destroy_window = true;

	// Test if adapter has all the needed features
	if (!TestAdapterFeatures(active_adapter)) return false;
	
	// Search for closest display mode match
	D3DDISPLAYMODE *mode=NULL;
	int amodes=adapters[active_adapter].display_mode_amount;
	int bestscore=99999999;
	for (int md=0;md<amodes;md++)
	{
		D3DDISPLAYMODE *mode_nm=&adapters[active_adapter].display_modes[md];

		int score=0;
		score+=abs(int(mode_nm->Width-width));
		score+=abs(int(mode_nm->Height-height));
		if (GetDisplayModeBPP(*mode_nm)>=16)
		{
			if ((bpp>16)&&(GetDisplayModeBPP(*mode_nm)>16))
			{
				if (GetDisplayModeBPP(*mode_nm)!=bpp) score+=1;
			} else score+=2;

			if ((bpp<=16)&&(GetDisplayModeBPP(*mode_nm)==16))
			{
				if (GetDisplayModeBPP(*mode_nm)!=bpp) score+=1;
			} else score+=2;

		} else score+=99999999;

		// Set the founded mode active if its better(=lower) that (last) best
		if (score<bestscore)
		{
			bestscore=score;
			adapters[active_adapter].active_display_mode=md;
			mode=mode_nm;
		}
	}

	// Did we find a good display mode?
	if (mode==NULL)
	{
		//MessageBox(NULL,"No display modes","Storm3D Error",0);
		//assert(!"Whoops");
		error_string = "No display modes available";
		return false;
	}

	// Find a compatible DS-buffer (dept/stencil)
	D3DFORMAT DSFormat=GetDSBufferModeForDisplayMode(active_adapter,*mode);
	support_stencil=GetDSModeStencilSupport(DSFormat);

	if (!no_info)
	{
		// Info box
		char s[255];
		sprintf(s,
			"Width:%d Height:%d\r\n"
			"COL: %d bits\r\n"
			"Z: %d bits\r\n"
			"Stencil: %s\r\n"
			,mode->Width,mode->Height,
			GetDisplayModeBPP(*mode),
			GetDSModeBPP(DSFormat),
			support_stencil? "Supported":"NOT supported");
		MessageBox(NULL,s,"Selected mode",0);
	}

	SDL_SetVideoMode(mode->Width, mode->Height, GetDisplayModeBPP(*mode), SDL_FULLSCREEN);
	window_handle = GetActiveWindow();

    // Set up the presentation parameters
    //D3DPRESENT_PARAMETERS pp; 
    ZeroMemory(&present_params,sizeof(present_params));
    present_params.Windowed               = 0;
    present_params.BackBufferCount        = 1;
    present_params.MultiSampleType        = D3DMULTISAMPLE_NONE;
	present_params.MultiSampleType = findMultiSampleType(*D3D, active_adapter, DSFormat, antialiasing_level, FALSE);

    present_params.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    present_params.EnableAutoDepthStencil = 1;
    present_params.AutoDepthStencilFormat = DSFormat;
    present_params.hDeviceWindow          = window_handle;
	if(vsync)
		present_params.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
	else
		present_params.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
	present_params.BackBufferWidth=mode->Width;
	present_params.BackBufferHeight=mode->Height;
	present_params.BackBufferFormat=mode->Format;

	screen_size.width=present_params.BackBufferWidth;
	screen_size.height=present_params.BackBufferHeight;
	viewport_size=screen_size;

	DWORD ptype=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	present_params.Flags = 0;//D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HW_TL)
		ptype=D3DCREATE_MIXED_VERTEXPROCESSING;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		ptype=D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		ptype=D3DCREATE_HARDWARE_VERTEXPROCESSING;


	if (use_reference_driver)
	{
		// --- REFERENCE DRIVER ---
		if (FAILED(D3D->CreateDevice(D3D->GetAdapterCount() - 1,D3DDEVTYPE_REF,window_handle,
		ptype,&present_params,&D3DDevice)))
		{
			MessageBox(NULL,"Error creating windowed device","Storm3D Error",0);
			return false;
		}
		// --- END REFERENCE DRIVER ---
	} else {
		// Create the device
		if (FAILED(D3D->CreateDevice(active_adapter,D3DDEVTYPE_HAL,window_handle,
		ptype,&present_params,&D3DDevice)))
		{
			//MessageBox(NULL,"Error creating fullscreen device","Storm3D Error",0);
			error_string = "Failed to create device";
			return false;
		}
	}

	// Get original gamma ramp
	// --jpk
	D3DDevice->GetGammaRamp(0, &originalGammaRamp);
	for (int i = 0; i < 256; i++)
	{
	  currentGammaRamp.red[i] = originalGammaRamp.red[i];
	  currentGammaRamp.green[i] = originalGammaRamp.green[i];
	  currentGammaRamp.blue[i] = originalGammaRamp.blue[i];
	}
	gammaPeakEnabled = false;

	bool ps14 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS14)
		ps14 = true;
	bool ps20 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS20)
		ps20 = true;

	setNeededDepthTarget(VC2I(width, height));

	Storm3D_TerrainRenderer::querySizes(*this, true);
	Storm3D_Spotlight::querySizes(*this, ps14, shadow_quality);
	Storm3D_FakeSpotlight::querySizes(*this, fake_shadow_quality);
	createTargets();
	createRenderTargets();
	Storm3D_TerrainRenderer::createRenderBuffers(*this, lighting_quality);
	Storm3D_Spotlight::createShadowBuffers(*this, *D3D, *D3DDevice, ps14, ps20, shadow_quality);
	Storm3D_FakeSpotlight::createBuffers(*this, *D3D, *D3DDevice, fake_shadow_quality);
	Storm3D_TerrainRenderer::createSecondaryRenderBuffers(*this, enable_glow);

	// Create shader manager
	new Storm3D_ShaderManager(*D3DDevice);

	if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, true);
	else
	{
		D3DDevice->SetSoftwareVertexProcessing(TRUE);
		Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, false);
	}

	//D3DDevice->SetDepthStencilSurface(depthTarget);

	if(!hasNeededBuffers())
		return false;

	// Everything OK
	return true;
}



//------------------------------------------------------------------
// Storm3D::SetWindowedMode
//------------------------------------------------------------------
bool Storm3D::SetWindowedMode(int width,int height,bool titlebar)
{
	destroy_window = true;

	// Get the current desktop display mode (for backbuffer and z)
    D3DDISPLAYMODE desktopmode;
    if(FAILED(D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&desktopmode)))
	{
		error_string = "Failed to get adapter display mode";
        return false;
	}

	// Find a compatible DS-buffer (dept/stencil)
	D3DFORMAT DSFormat=GetDSBufferModeForDisplayMode(active_adapter,desktopmode);
	support_stencil=GetDSModeStencilSupport(DSFormat);

	if (!no_info)
	{
		// Info box
		char s[255];
		sprintf(s,
			"Width:%d Height:%d\r\n"
			"COL: %d bits\r\n"
			"Z: %d bits\r\n"
			"Stencil: %s\r\n"
			,width,height,
			GetDisplayModeBPP(desktopmode),
			GetDSModeBPP(DSFormat),
			support_stencil? "Supported":"NOT supported");
		MessageBox(NULL,s,"Selected mode",0);
	}

	SDL_SetVideoMode(width, height, GetDisplayModeBPP(desktopmode), 0);
	SDL_WM_SetCaption(application_name, NULL);
	window_handle = GetActiveWindow();

    // Set up the presentation parameters
    //D3DPRESENT_PARAMETERS pp; 
    ZeroMemory(&present_params,sizeof(present_params));
    present_params.Windowed               = 1;
    present_params.BackBufferCount        = 1;
    present_params.MultiSampleType        = D3DMULTISAMPLE_NONE;
	present_params.MultiSampleType = findMultiSampleType(*D3D, active_adapter, DSFormat, antialiasing_level, TRUE);

    present_params.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    present_params.EnableAutoDepthStencil = 1;
    present_params.AutoDepthStencilFormat = DSFormat;
    present_params.hDeviceWindow          = window_handle;
	if(vsync)
		present_params.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
	else
		present_params.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
	present_params.BackBufferWidth=width;
	present_params.BackBufferHeight=height;
	present_params.BackBufferFormat=desktopmode.Format;

	screen_size.width=present_params.BackBufferWidth;
	screen_size.height=present_params.BackBufferHeight;
	viewport_size=screen_size;
	present_params.Flags = 0; //D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	DWORD ptype=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HW_TL)
		ptype=D3DCREATE_MIXED_VERTEXPROCESSING;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		ptype=D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		ptype=D3DCREATE_HARDWARE_VERTEXPROCESSING;

    // Create the device
	if (use_reference_driver)
	{
		// --- REFERENCE DRIVER ---
		// TODO: this relies on the last adapter being the reference driver (is that always true?)
		if (FAILED(D3D->CreateDevice(D3D->GetAdapterCount() - 1,D3DDEVTYPE_REF,window_handle,
			ptype,&present_params,&D3DDevice)))
		{
			MessageBox(NULL,"Error creating windowed device","Storm3D Error",0);
			return false;
		}
		// --- END REFERENCE DRIVER ---
	} else {
		if (FAILED(D3D->CreateDevice(active_adapter,D3DDEVTYPE_HAL,window_handle,
			ptype,&present_params,&D3DDevice)))
		{
			//MessageBox(NULL,"Error creating windowed device","Storm3D Error",0);
			error_string = "Failed to create device";
			return false;
		}
	}

	// Get original gamma ramp
	// --jpk
	D3DDevice->GetGammaRamp(0, &originalGammaRamp);
	for (int i = 0; i < 256; i++)
	{
	  currentGammaRamp.red[i] = originalGammaRamp.red[i];
	  currentGammaRamp.green[i] = originalGammaRamp.green[i];
	  currentGammaRamp.blue[i] = originalGammaRamp.blue[i];
	}

	gammaPeakEnabled = false;

	bool ps14 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS14)
		ps14 = true;
	bool ps20 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS20)
		ps20 = true;

	setNeededDepthTarget(VC2I(width, height));
	Storm3D_TerrainRenderer::querySizes(*this, true);
	Storm3D_Spotlight::querySizes(*this, ps14, shadow_quality);
	Storm3D_FakeSpotlight::querySizes(*this, fake_shadow_quality);
	createTargets();
	createRenderTargets();
	Storm3D_TerrainRenderer::createRenderBuffers(*this, lighting_quality);
	Storm3D_Spotlight::createShadowBuffers(*this, *D3D, *D3DDevice, ps14, ps20, shadow_quality);
	Storm3D_FakeSpotlight::createBuffers(*this, *D3D, *D3DDevice, fake_shadow_quality);
	Storm3D_TerrainRenderer::createSecondaryRenderBuffers(*this, enable_glow);

	// Create shader manager
	new Storm3D_ShaderManager(*D3DDevice);

	if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, true);
	else
	{
		D3DDevice->SetSoftwareVertexProcessing(TRUE);
		Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, false);
	}

	//D3DDevice->SetDepthStencilSurface(depthTarget);

	if(!hasNeededBuffers())
		return false;

	// Everything OK
	return true;
}

bool Storm3D::SetWindowedMode(bool disableBuffers = false)
{
	destroy_window = false;

	// Get the current desktop display mode (for backbuffer and z)
    D3DDISPLAYMODE desktopmode;
    if(FAILED(D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&desktopmode)))
	{
		error_string = "Failed to get adapter display mode";
        return false;
	}

	// Find a compatible DS-buffer (dept/stencil)
	D3DFORMAT DSFormat=GetDSBufferModeForDisplayMode(active_adapter,desktopmode);
	support_stencil=GetDSModeStencilSupport(DSFormat);

	window_handle = GetActiveWindow();
	RECT window_rect = { 0 };
	GetClientRect(window_handle, &window_rect);
	int width = window_rect.right;
	int height = window_rect.bottom;

    // Set up the presentation parameters
    //D3DPRESENT_PARAMETERS pp; 
    ZeroMemory(&present_params,sizeof(present_params));
    present_params.Windowed               = 1;
    present_params.BackBufferCount        = 1;
	present_params.MultiSampleType        = D3DMULTISAMPLE_NONE;
	present_params.MultiSampleType = findMultiSampleType(*D3D, active_adapter, DSFormat, antialiasing_level, TRUE);

    present_params.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    present_params.EnableAutoDepthStencil = 1;
    present_params.AutoDepthStencilFormat = DSFormat;
    present_params.hDeviceWindow          = window_handle;
	if(vsync)
		present_params.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
	else
		present_params.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
	present_params.BackBufferWidth=width;
	present_params.BackBufferHeight=height;
	present_params.BackBufferFormat=desktopmode.Format;

	screen_size.width=present_params.BackBufferWidth;
	screen_size.height=present_params.BackBufferHeight;
	viewport_size=screen_size;

	DWORD ptype=D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//present_params.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	//present_params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HW_TL)
		ptype=D3DCREATE_MIXED_VERTEXPROCESSING;
	if (adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		ptype=D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;

/*
#pragma message(" ********************* ")
#pragma message(" !!!!!!!!!!!!!!!!!!!!! ")
#pragma message(" ********************* ")
ptype=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
*/

    // Create the device
    if (FAILED(D3D->CreateDevice(active_adapter,D3DDEVTYPE_HAL,window_handle,
		ptype,&present_params,&D3DDevice)))
	{
		//MessageBox(NULL,"Error creating windowed device","Storm3D Error",0);
		error_string = "Failed creating device";
		return false;
	}

/*
if (FAILED(D3D->CreateDevice(D3D->GetAdapterCount() - 1,D3DDEVTYPE_REF,window_handle,
	ptype,&present_params,&D3DDevice)))
{
	MessageBox(NULL,"Error creating windowed device","Storm3D Error",0);
	return false;
}
*/

	// Get original gamma ramp
	// --jpk
	D3DDevice->GetGammaRamp(0, &originalGammaRamp);
	for (int i = 0; i < 256; i++)
	{
	  currentGammaRamp.red[i] = originalGammaRamp.red[i];
	  currentGammaRamp.green[i] = originalGammaRamp.green[i];
	  currentGammaRamp.blue[i] = originalGammaRamp.blue[i];
	}
	gammaPeakEnabled = false;

	bool ps14 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS14)
		ps14 = true;
	bool ps20 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS20)
		ps20 = true;

	needValueTargets = true;
	setNeededDepthTarget(VC2I(width, height));

	//if(!disableBuffers)
	{
		Storm3D_TerrainRenderer::querySizes(*this, true);
		Storm3D_Spotlight::querySizes(*this, ps14, shadow_quality);
		Storm3D_FakeSpotlight::querySizes(*this, fake_shadow_quality);
	}

	createTargets();
	createRenderTargets();

	//if(!disableBuffers)
	{
		Storm3D_TerrainRenderer::createRenderBuffers(*this, lighting_quality);
		Storm3D_Spotlight::createShadowBuffers(*this, *D3D, *D3DDevice, ps14, ps20, shadow_quality);
		Storm3D_FakeSpotlight::createBuffers(*this, *D3D, *D3DDevice, fake_shadow_quality);
		Storm3D_TerrainRenderer::createSecondaryRenderBuffers(*this, enable_glow);
	}

	// Create shader manager
	new Storm3D_ShaderManager(*D3DDevice);

	if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
		Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, true);
	else
	{
		D3DDevice->SetSoftwareVertexProcessing(TRUE);
		Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, false);
	}

	//D3DDevice->SetDepthStencilSurface(depthTarget);

	if(!hasNeededBuffers())
		return false;

	// Everything OK
	return true;
}

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

//------------------------------------------------------------------
// Storm3D::Storm3D
//------------------------------------------------------------------
Storm3D::Storm3D(bool _no_info, filesystem::FilePackageManager *fileManager, IStorm3D_Logger *logger_) : 
	logger(logger_),
	active_material((Storm3D_Material*)1),	// NULL is not right!
	active_mesh(NULL),
	adapters(NULL),
	active_adapter(D3DADAPTER_DEFAULT),
	destroy_window(true),
	D3D(NULL),
	D3DDevice(NULL),
	textureLODLevel(0),
	bbuf_orig(NULL),
	zbuf_orig(NULL),
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
	highcolorrange_videos(true),
	use_reference_driver(false),
	enableReflection(false),
	halfReflection(false),
	reflectionQuality(0),
	proceduralManager(*this),
	needValueTargets(false),
	antialiasing_level(0),
	allocate_procedural_target(true),
	force_reset(false)
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

	// Create the D3D object
	if((D3D=Direct3DCreate9(D3D_SDK_VERSION))==NULL)
	{
		MessageBox(NULL,"DirectX9 is needed to run this program.","Storm3D Error",0);
		return;
	}

	// Enumerate all adapters and modes
	EnumAdaptersAndModes();

	timeFactor = 1.0f;
	gammaPeakEnabled = false;
	SetApplicationName("Storm3D", "Storm3D v2.0 - Render Window");

#ifdef NVPERFSDK
	NVPMInit();
	NVPMAddCounterByName("GPU Bottleneck");
#endif
}


//------------------------------------------------------------------
// Storm3D::~Storm3D
//------------------------------------------------------------------
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

	// Is there left references to backbuffer?
	SAFE_RELEASE(bbuf_orig);
	SAFE_RELEASE(zbuf_orig);

	proceduralManager.reset();

	// Empty (models, textures, materials)
	Empty();

	assert(allocated_models == 0);
	//assert(allocated_meshes == 0);

	Storm3D_TerrainRenderer::freeSecondaryRenderBuffers();
	Storm3D_Spotlight::freeShadowBuffers();
	Storm3D_FakeSpotlight::freeBuffers();
	Storm3D_TerrainRenderer::freeRenderBuffers();

	if(colorTarget)
		colorTarget.Release();
	if(colorSecondaryTarget)
		colorSecondaryTarget.Release();
	if(depthTarget)
		depthTarget.Release();
	if(valueTarget)
		valueTarget.Release();
	if(systemTarget)
		systemTarget.Release();
	if(proceduralTarget)
		proceduralTarget.Release();
	if(proceduralOffsetTarget)
		proceduralOffsetTarget.Release();

	freeRenderTargets();

	// Delete shader manager
	delete Storm3D_ShaderManager::GetSingleton();

	proceduralManager.reset();

	// May now leak memory, but otherwise crashes on Windows
	while(textures.begin()!=textures.end())
	{
		delete (*textures.begin());
	}

	// Release Direct3D stuff
    if(D3DDevice) D3DDevice->Release();
    if(D3D) D3D->Release();

	// Delete adapterlist
	if (adapters) delete[] adapters;

	// Delete techniquehandler
	//if (techniquehandler) delete techniquehandler;

	/*
	if (application_shortname != NULL)
	{
		delete [] application_shortname;
		application_shortname = NULL;
	}

	if (application_name != NULL)
	{
		delete [] application_name;
		application_name = NULL;
	}
	*/
	freeTextureBank();

	if(window_handle && destroy_window)
		DestroyWindow(window_handle);
}


// Few usefull macros
#ifndef D3DSHADER_VERSION_MAJOR
#define D3DSHADER_VERSION_MAJOR(_Version) (((_Version)>>8)&0xFF)
#define D3DSHADER_VERSION_MINOR(_Version) (((_Version)>>0)&0xFF)
#endif


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

void Storm3D::addAdditionalRenderTargets(const VC2 &size, int amount)
{
	RenderTarget target;
	target.size = size;

	for(int i = 0; i < amount; ++i)
		renderTargets.push_back(target);

}

void Storm3D::setProceduralTextureSize(const VC2I &size)
{
	proceduralTargetSize = size;
}

IStorm3D_Texture *Storm3D::getRenderTarget(int index)
{
	/*
	int i = 0;
	for(RenderTargetList::iterator it = renderTargets.begin(); it != renderTargets.end(); ++it)
	{
		assert(it->texture);
		if(!equals(size, it->size, 0.001f))
			continue;

		if(i++ == index)
			return it->texture;
	}
	*/

	if(index >= 0 && index < int(renderTargets.size()))
		return renderTargets[index].texture;

	return 0;
}

void Storm3D::SetGammaPeak(bool peakEnabled, float peakPosition, 
  float peakLowShape, float peakHighShape, 
  float peakRed, float peakGreen, float peakBlue)
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

extern float reflection_height;
void Storm3D::enableLocalReflection(bool enable, float height)
{
	enableReflection = enable;
	reflection_height = height;
}

// Getting/Setting gamma and stuff...
// --jpk
void Storm3D::RestoreGammaRamp()
{
	D3DDevice->SetGammaRamp(0, D3DSGR_CALIBRATE, &originalGammaRamp);	
}


void Storm3D::SetGammaRamp(float gamma, float brightness, float contrast,
  float red, float green, float blue, bool calibrate)
{
	if(gamma <= 0.01f)
		gamma = 0.01f;

	for (int i = 0; i < 256; i++)
	{
		float gammaVal = powf(i / 255.f, 1.f / gamma);
		//float gammaVal = powf(i / 255.f, gamma);
		float val = 32767.5f + 65535.0f * ((gammaVal - 0.5f) * contrast + (brightness - 1.0f));

	//float val2 = 32767.5f + 65535.0f * ((((float)i / 255.0f) - 0.5f) * contrast + (brightness - 1.0f));
	//val = (val + val2) * 0.5f;
	
		// TODO: effect of gamma on value
		//float val = 32767.5f + 65535.0f * ((((float)i / 255.0f) - 0.5f) * contrast + (brightness - 1.0f));
		
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

		currentGammaRamp.red[i] = (WORD)r;
		currentGammaRamp.green[i] = (WORD)g;
		currentGammaRamp.blue[i] = (WORD)b;
	}

	DWORD flags;
	if (calibrate)
		flags = D3DSGR_CALIBRATE;
	else
		flags = D3DSGR_NO_CALIBRATION;

	D3DDevice->SetGammaRamp(0, flags, &currentGammaRamp);
}


void Storm3D::SetTextureLODLevel(int lodlevel)
{
	textureLODLevel = lodlevel;
}


int Storm3D::GetTextureLODLevel()
{
	return textureLODLevel;
}

char *Storm3D::GetPrintableStatusInfo()
{
	char *buf = new char[2048 + 1];

	if (this->allocated_models != storm3d_model_allocs)
	{
	  sprintf(buf, "Storm3D status info follows:\nModel allocation amounts mismatch, internal error.\n");
	} else {
		sprintf(buf, "Models allocated: %d\nModel meshes allocated: %d\nModel loads so far: %d\nBone loads so far: %d\nBone animations allocated: %d\nModel objects allocated: %d\nMeshes allocated: %d\nMaterials allocated: %d\nBones allocated: %d\nTextures allocated: %d\nDIP calls %d\nObjects tested %d\nObjects rough passed %d\nObjects passed %d\n\n",
			allocated_models, allocated_meshes, storm3d_model_loads, storm3d_model_bone_loads, storm3d_model_boneanimation_allocs, storm3d_model_objects_created, storm3d_mesh_allocs, storm3d_material_allocs, storm3d_bone_allocs, storm3d_texture_allocs, storm3d_dip_calls, storm3d_model_objects_tested, storm3d_model_objects_rough_passed, storm3d_model_objects_passed);
	}

  return buf;
}


// need this just to easily detour the problems caused by memorymanager usage.
// also, seems like a more "correct" way to delete resources...
// what storm3d allocates, it also deletes.
void Storm3D::DeletePrintableStatusInfo(char *buf)
{
  delete [] buf;
}


void Storm3D::setGlobalTimeFactor(float timeFactor)
{
  this->timeFactor = timeFactor;
}


//------------------------------------------------------------------
// Storm3D::EnumAdaptersAndModes
// Enumerates all 3d-adapters attached to computer and saves
// each adapter's videomodes to a list.
//------------------------------------------------------------------
void Storm3D::EnumAdaptersAndModes()
{
	// Get adapter count
	adapter_amount=D3D->GetAdapterCount();

	// Create array to store adapter data
    adapters=new Storm3D_Adapter[adapter_amount];

	// Get all graphics-adapters on system
    for(int ada=0;ada<adapter_amount;ada++)
    {
		// Get adapter identifier
		D3DADAPTER_IDENTIFIER9 aid;
        D3D->GetAdapterIdentifier(ada,0,&aid);

        // Enumerate all display modes on this adapter
		int amount_modes=0;

		// Get (adapter) display mode count
        int amodes=D3D->GetAdapterModeCount(ada, D3DFMT_X8R8G8B8);

		// Create array to store display modes (to adapter)
		adapters[ada].display_modes=new D3DDISPLAYMODE[amodes];

		// Get info on each mode
        for(int md=0;md<amodes;md++)
        {
            // Get the display mode attributes
            D3DDISPLAYMODE mode;
            D3D->EnumAdapterModes(ada,D3DFMT_X8R8G8B8,md,&mode);

            // Check if the mode already exists (to filter out refresh rates)
			int i = 0;
            for(;i<amount_modes;i++)
                if((adapters[ada].display_modes[i].Width==mode.Width)&&
                   (adapters[ada].display_modes[i].Height==mode.Height)&&
                   (adapters[ada].display_modes[i].Format==mode.Format)) break;

            // If we found a new mode, add it to the list of modes
            if(i==amount_modes)
            {
                adapters[ada].display_modes[amount_modes].Width=mode.Width;
                adapters[ada].display_modes[amount_modes].Height=mode.Height;
                adapters[ada].display_modes[amount_modes].Format=mode.Format;
                adapters[ada].display_modes[amount_modes].RefreshRate=0;

				amount_modes++;
            }
		}

		// Save the mode amount
		adapters[ada].display_mode_amount=amount_modes;

		// Get HAL-device caps (no REF or SW, because they are too slow!)
		D3DCAPS9 devcaps;
		D3D->GetDeviceCaps(ada,D3DDEVTYPE_HAL,&devcaps);

		// Store intresting caps...

		// Get texture max size
		adapters[ada].maxtexsize=Storm3D_SurfaceInfo(devcaps.MaxTextureWidth,devcaps.MaxTextureHeight);

		// Get mapping
		if (devcaps.TextureCaps&D3DPTEXTURECAPS_PROJECTED) adapters[ada].caps|=Storm3D_Adapter::CAPS_TEX_PROJECTED;
		if (devcaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP) adapters[ada].caps|=Storm3D_Adapter::CAPS_TEX_CUBE;
		if (devcaps.TextureCaps&D3DPTEXTURECAPS_VOLUMEMAP) adapters[ada].caps|=Storm3D_Adapter::CAPS_TEX_VOLUME;

		// Get bumpmapping
		if (devcaps.TextureOpCaps&D3DTEXOPCAPS_BUMPENVMAP) adapters[ada].caps|=Storm3D_Adapter::CAPS_EMBM;
		if (devcaps.TextureOpCaps&D3DTEXOPCAPS_DOTPRODUCT3) adapters[ada].caps|=Storm3D_Adapter::CAPS_DOT3;

		// Get T&L stuff
		if (devcaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT) adapters[ada].caps|=Storm3D_Adapter::CAPS_HW_TL;
		if (devcaps.VertexProcessingCaps&D3DVTXPCAPS_TWEENING) adapters[ada].caps|=Storm3D_Adapter::CAPS_HW_MESH_INTERPOLATION;

		// Hw shaders
		int vshaderMa = D3DSHADER_VERSION_MAJOR(devcaps.VertexShaderVersion);
		int vshaderMi = D3DSHADER_VERSION_MINOR(devcaps.VertexShaderVersion);
		int pshaderMa = D3DSHADER_VERSION_MAJOR(devcaps.PixelShaderVersion);
		int pshaderMi = D3DSHADER_VERSION_MINOR(devcaps.PixelShaderVersion);

		if((vshaderMa == 1 && vshaderMi >= 1) || vshaderMa > 1)
			adapters[ada].caps |= Storm3D_Adapter::CAPS_HWSHADER;
		if((pshaderMa == 1 && pshaderMi >= 1) || vshaderMa > 1)
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS11;
		if((pshaderMa == 1 && pshaderMi >= 3) || vshaderMa > 1)
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS13;
		if((pshaderMa == 1 && pshaderMi >= 4) || vshaderMa > 1)
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS14;
		if(pshaderMa >= 2)
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS20;

		/*
		if((D3DSHADER_VERSION_MAJOR(devcaps.VertexShaderVersion) >= 1) && (D3DSHADER_VERSION_MINOR(devcaps.VertexShaderVersion) >= 1))
		{
			adapters[ada].caps |= Storm3D_Adapter::CAPS_HWSHADER;
		}
		if((D3DSHADER_VERSION_MAJOR(devcaps.PixelShaderVersion) >= 1) && (D3DSHADER_VERSION_MINOR(devcaps.PixelShaderVersion) >= 1))
		{
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS11;
		}
		if((D3DSHADER_VERSION_MAJOR(devcaps.PixelShaderVersion) >= 1) && (D3DSHADER_VERSION_MINOR(devcaps.PixelShaderVersion) >= 3))
		{
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS13;
		}
		if((D3DSHADER_VERSION_MAJOR(devcaps.PixelShaderVersion) >= 1) && (D3DSHADER_VERSION_MINOR(devcaps.PixelShaderVersion) >= 4))
		{
			adapters[ada].caps |= Storm3D_Adapter::CAPS_PS14;
		}
		*/

		adapters[ada].maxPixelShaderValue = 1.f; //devcaps.MaxPixelShaderValue;
		adapters[ada].user_clip_planes = devcaps.MaxUserClipPlanes;

		// Get HOP
		if ((devcaps.DevCaps&D3DDEVCAPS_RTPATCHES)&&
			(devcaps.DevCaps&D3DDEVCAPS_QUINTICRTPATCHES)) adapters[ada].caps|=Storm3D_Adapter::CAPS_BEZ_SPL_PATCHES;
		if (devcaps.DevCaps&D3DDEVCAPS_NPATCHES) adapters[ada].caps|=Storm3D_Adapter::CAPS_NPATCHES;

		// Get anisotropy
		adapters[ada].max_anisotropy=devcaps.MaxAnisotropy;
		if (devcaps.TextureFilterCaps&D3DPTFILTERCAPS_MINFANISOTROPIC)		
			adapters[ada].caps|=Storm3D_Adapter::CAPS_ANISOTROPIC;
		else 
			adapters[ada].max_anisotropy=0;

		if(devcaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR && devcaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
			adapters[ada].stretchFilter = true;
		else
			adapters[ada].stretchFilter = false;

		// Get multitexturing layers
		adapters[ada].multitex_layers=devcaps.MaxSimultaneousTextures;

		// Get pixelfog
		if (devcaps.RasterCaps&D3DPRASTERCAPS_FOGTABLE) adapters[ada].caps|=Storm3D_Adapter::CAPS_PIXELFOG;

		// Test for multisample antialias
		if (SUCCEEDED(D3D->CheckDeviceMultiSampleType(ada,D3DDEVTYPE_HAL,D3DFMT_R8G8B8,FALSE,D3DMULTISAMPLE_2_SAMPLES,0))) 
			adapters[ada].multisample=2;
	}
}



//------------------------------------------------------------------
// Storm3D::GetDSBufferModeForDisplayMode
// Search best compatible depth/scencil combination to be used
// with a displaymode.
//------------------------------------------------------------------
D3DFORMAT Storm3D::GetDSBufferModeForDisplayMode(int adapter,D3DDISPLAYMODE &mode)
{
	D3DFORMAT moud=D3DFMT_D16;
	switch (mode.Format)
	{
		case D3DFMT_R8G8B8: moud=D3DFMT_D32;break;
		case D3DFMT_A8R8G8B8: moud=D3DFMT_D32;break;
		case D3DFMT_X8R8G8B8: moud=D3DFMT_D32;break;
		case D3DFMT_R5G6B5: moud=D3DFMT_D16;break;
		case D3DFMT_X1R5G5B5: moud=D3DFMT_D16;break;
		default: break;
	}

	switch (moud)
	{
		case D3DFMT_D16: // 16 bits

			// 16 bit Z
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D16)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D16))) return D3DFMT_D16;
			}

			// 16 bit Z (lockable)
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D16_LOCKABLE)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D16_LOCKABLE))) return D3DFMT_D16_LOCKABLE;
			}

			// 15 bit Z, 1 bit stencil
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D15S1)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D15S1))) return D3DFMT_D15S1;
			}

			// 32 bit Z
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D32)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D32))) return D3DFMT_D32;
			}

			// 24 bit Z (24 Z-bits, 8 unused)
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24X8)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D24X8))) return D3DFMT_D24X8;            
			}
	
			// 24 bit Z, 8 bit stencil
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24S8)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D24S8))) return D3DFMT_D24S8;
			}

			// 24 bit Z, 4 bit stencil (4 bits unused!!)
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24X4S4)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D24X4S4))) return D3DFMT_D24X4S4;
			}

			break;

		case D3DFMT_D32: // 32 bits

			// 32 bit Z
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D32)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D32))) return D3DFMT_D32;
			}

			// 24 bit Z (24 Z-bits, 8 unused)
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24X8)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D24X8))) return D3DFMT_D24X8;            
			}
	
			// 24 bit Z, 8 bit stencil
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24S8)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D24S8))) return D3DFMT_D24S8;
			}

			// 24 bit Z, 4 bit stencil (4 bits unused!!)
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24X4S4)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D24X4S4))) return D3DFMT_D24X4S4;
			}

			// 16 bit Z
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D16)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D16))) return D3DFMT_D16;
			}

			// 16 bit Z (lockable)
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D16_LOCKABLE)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D16_LOCKABLE))) return D3DFMT_D16_LOCKABLE;
			}

			// 15 bit Z, 1 bit stencil
			if(SUCCEEDED(D3D->CheckDeviceFormat(adapter,D3DDEVTYPE_HAL,
				mode.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D15S1)))
			{
				if(SUCCEEDED(D3D->CheckDepthStencilMatch(adapter,D3DDEVTYPE_HAL,
					mode.Format,mode.Format,D3DFMT_D15S1))) return D3DFMT_D15S1;
			}
		default: break;
	}

	// No compatible modes found
	// If this happens we can only quess right format...;)
	return moud;
}



//------------------------------------------------------------------
// Storm3D::GetDisplayModeBPP
// Returns: bit depth or 0 if failed.
//------------------------------------------------------------------
int Storm3D::GetDisplayModeBPP(D3DDISPLAYMODE &mode)
{
	switch(mode.Format)
	{
		case D3DFMT_R8G8B8: return 24;
		case D3DFMT_A8R8G8B8: return 32;
		case D3DFMT_X8R8G8B8: return 32;
		case D3DFMT_R5G6B5: return 16;
		case D3DFMT_X1R5G5B5: return 16;
		case D3DFMT_A1R5G5B5: return 16;
		default: break;
	}

	// fail (mode cannot be identified)
	return 0;
}



//------------------------------------------------------------------
// Storm3D::GetDSModeBPP
// Returns: bit depth or 0 if failed.
//------------------------------------------------------------------
int Storm3D::GetDSModeBPP(D3DFORMAT &form)
{
	switch(form)
	{
		case D3DFMT_D16_LOCKABLE: return 16;
		case D3DFMT_D32: return 32;
		case D3DFMT_D15S1: return 15;
		case D3DFMT_D24S8: return 24;
		case D3DFMT_D16: return 16;
		case D3DFMT_D24X8: return 24;
		case D3DFMT_D24X4S4: return 24;
		default: break;
	}

	// fail (mode cannot be identified)
	return 0;
}



//------------------------------------------------------------------
// Storm3D::GetDSModeStencilSupport
// Returns: stencil supported or not
//------------------------------------------------------------------
bool Storm3D::GetDSModeStencilSupport(D3DFORMAT &form)
{
	switch(form)
	{
		case D3DFMT_D16_LOCKABLE: return false;
		case D3DFMT_D32: return false;
		case D3DFMT_D15S1: return true;
		case D3DFMT_D24S8: return true;
		case D3DFMT_D16: return false;
		case D3DFMT_D24X8: return false;
		case D3DFMT_D24X4S4: return true;
		default: break;
	}

	// fail (mode cannot be identified)
	return false;
}



//------------------------------------------------------------------
// Texture handling
//------------------------------------------------------------------
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

//logger->debug("Trying to find before conversion");
//logger->debug(originalString.c_str());

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

//logger->debug("Trying to find after conversion");
//logger->debug(filename);

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
		/*
		if (logger != NULL)
		{
			logger->debug("Storm3D::CreateNewTexture - Texture was not found in cache, loading it.");
			logger->debug(filename);
		}
		*/
		if(data == NULL)
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
			if (((filename[sl-3]=='a')||(filename[sl-3]=='A'))&&
				((filename[sl-2]=='v')||(filename[sl-2]=='V'))&&
				((filename[sl-1]=='i')||(filename[sl-1]=='I'))) is_avi=true;
		}

		// Load the new texture
		Storm3D_Texture *tex = 0;
		if (is_avi)
		{
			assert(data == NULL); // preloaded data not supported for video
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


Storm3D_Texture *Storm3D::CreateNewTextureInstance(int width,int height,IStorm3D_Texture::TEXTYPE textype)
{
	// Returns class implementation instead of just interface.
	Storm3D_Texture *tex=new Storm3D_Texture(this,width,height,textype);
	textures.insert(tex);
	return tex;
}

IStorm3D_Texture *Storm3D::CreateNewTexture(int width,int height,IStorm3D_Texture::TEXTYPE textype)
{
	return CreateNewTextureInstance( width, height, textype);
}


void Storm3D::Remove(IStorm3D_Texture *itex)
{
	textures.erase(itex);

	/*Storm3D_Texture *tex=(Storm3D_Texture*)itex;

	// First check if this texture exists in Storm3D
	set<IStorm3D_Texture*>::iterator it;
	it=textures.find(itex);
	if (it!=textures.end())
	{
		// Delete texture and remove it from the set
		// If texture if still referenced, do not remove it from
		// the list...
		if (tex->Release())
		{
			textures.erase(it);
		}
	}*/
}


// Log an error message
void Storm3D::LogError(const char *logMessage)
{
	if (this->logger == NULL)
	{
		Logger::getInstance()->error(logMessage);
	}

	if (this->logger != NULL)
	{
		this->logger->error(logMessage);
	}
}


//------------------------------------------------------------------
// Material handling
//------------------------------------------------------------------
IStorm3D_Material *Storm3D::CreateNewMaterial(const char *name)
{
	// Create new material and add it into the set
	Storm3D_Material *mat=new Storm3D_Material(this,name);
	//materials.insert(mat);

	// Return material's pointer for user modifications
	resourceManager.addResource(mat);
	return mat;
}


void Storm3D::Remove(IStorm3D_Material *imat, IStorm3D_Mesh *mesh)
{
	//std::set<IStorm3D_Material *>::iterator it = materials.find(imat);
	//if(it == materials.end())
	//	return;

	//materials.erase(it);
	if(mesh == 0)
		resourceManager.deleteResource(imat);
	else
		resourceManager.removeUser(imat, mesh);

	/*Storm3D_Material *mat=(Storm3D_Material*)imat;

	// First check if this material exists in Storm3D
	set<IStorm3D_Material*>::iterator it;
	it=materials.find(imat);
	if (it!=materials.end())
	{
		// Delete material and remove it from the set
		delete imat;
		materials.erase(it);
	}*/
}



//------------------------------------------------------------------
// Model handling
//------------------------------------------------------------------
IStorm3D_Model *Storm3D::CreateNewModel()
{
	// Create new model and add it into the set
	Storm3D_Model *mod=new Storm3D_Model(this);
	models.insert(mod);

	allocated_models++;

	// Return model's pointer for user modifications
	return mod;
}


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

	/*Storm3D_Model *mod=(Storm3D_Model*)imod;

	// First check if this model exists in Storm3D
	set<IStorm3D_Model*>::iterator it;
	it=models.find(imod);
	if (it!=models.end())
	{
		// Delete model and remove it from the set
		delete imod;
		models.erase(it);
	}*/
}



//------------------------------------------------------------------
// Mesh handling
//------------------------------------------------------------------
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


void Storm3D::Remove(IStorm3D_Mesh *imod, IStorm3D_Model_Object *object)
{
	allocated_meshes--;
	assert(allocated_meshes >= 0);
/*
	std::set<IStorm3D_Mesh *>::iterator it = meshes.find(imod);
	if(it == meshes.end())
	{
		assert(!"Allocated mesh does not exist in storm3d?");
		return;
	}

	meshes.erase(it);
*/	
	if(object == 0)
		resourceManager.deleteResource(imod);
	else
		resourceManager.removeUser(imod, object);

	//meshes.erase(imod);
	/*Storm3D_Mesh *mod=(Storm3D_Mesh*)imod;

	// First check if this mesh exists in Storm3D
	set<IStorm3D_Mesh*>::iterator it;
	it=meshes.find(imod);
	if (it!=meshes.end())
	{
		// Delete mesh and remove it from the set
		delete imod;
		meshes.erase(it);
	}*/
}



//------------------------------------------------------------------
// Scene handling
//------------------------------------------------------------------
IStorm3D_Scene *Storm3D::CreateNewScene()
{
	// Create new scene and add it into the set
	Storm3D_Scene *sce=new Storm3D_Scene(this);
	scenes.insert(sce);

	// Return scene's pointer for user modifications
	return sce;
}


void Storm3D::Remove(IStorm3D_Scene *isce)
{
	scenes.erase(isce);
	/*Storm3D_Scene *sce=(Storm3D_Scene*)isce;

	// First check if this scene exists in Storm3D
	set<IStorm3D_Scene*>::iterator it;
	it=scenes.find(isce);
	if (it!=scenes.end())
	{
		// Delete scene and remove it from the set
		delete isce;
		scenes.erase(it);
	}*/
}



//------------------------------------------------------------------
// Storm3D::CreateNewParticle
//------------------------------------------------------------------
/*IStorm3D_Particle *Storm3D::CreateNewParticle()
{
	// Create new particle and add it into the set
	Storm3D_Particle *part=new Storm3D_Particle(NULL,0,0,0,0,1,1,0);
	particles.insert(part);

	// Return particle's pointer for user modifications
	return part;
}



//------------------------------------------------------------------
// Storm3D::DeleteParticle
//------------------------------------------------------------------
void Storm3D::DeleteParticle(IStorm3D_Particle *ipart)
{
	Storm3D_Particle *part=(Storm3D_Particle*)ipart;

	// First check if this particle exists in Storm3D
	set<IStorm3D_Particle*>::iterator it;
	it=particles.find(ipart);
	if (it!=particles.end())
	{
		// Delete particle and remove it from the set
		delete ipart;
		particles.erase(it);
	}
}*/



//------------------------------------------------------------------
// Font handling
//------------------------------------------------------------------
IStorm3D_Font *Storm3D::CreateNewFont()
{
	// Create new font and add it into the set
	Storm3D_Font *font=new Storm3D_Font(this);
	fonts.insert(font);

	// Return font's pointer for user modifications
	return font;
}


void Storm3D::Remove(IStorm3D_Font *ifont)
{
	fonts.erase(ifont);
}


//------------------------------------------------------------------
// Terrain handling
//------------------------------------------------------------------
IStorm3D_Terrain *Storm3D::CreateNewTerrain( int block_size )
{
	bool ps13 = false;
	bool ps14 = false;
	bool ps20 = false;

	if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_PS13)
		ps13 = true;
	if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_PS14)
		ps14 = true;
	if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_PS20)
		ps20 = true;

	Storm3D_Terrain *terrain = new Storm3D_Terrain(*this, ps13, ps14, ps20);
	terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, enable_glow);
	terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, enable_distortion);
	terrains.insert(terrain);

	return terrain;
}

void Storm3D::Remove(IStorm3D_Terrain *terrain)
{
	terrains.erase(terrain);
}

//------------------------------------------------------------------
// Bone handling
//------------------------------------------------------------------
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
		//delete ret;

		return NULL;
	}
}

// Lines
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

//------------------------------------------------------------------
// Storm3D::Empty
//------------------------------------------------------------------
void Storm3D::Empty()
{
	/*for(set<IStorm3D_Scene*>::iterator is=scenes.begin();is!=scenes.end();is++)
	{
		delete (*is);
	}*/

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

	/*for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();itr++)
	{
		delete (*itr);
	}*/

	/*for(set<IStorm3D_Model*>::iterator io=models.begin();io!=models.end();io++)
	{
		delete (*io);
	}*/
	// Delete meshes
	/*
	while(meshes.begin()!=meshes.end())
	{
		delete (*meshes.begin());
	}
	*/
	/*for(set<IStorm3D_Mesh*>::iterator imh=meshes.begin();imh!=meshes.end();imh++)
	{
		delete (*imh);
	}*/
	
	// Delete fonts
	while(fonts.begin()!=fonts.end())
	{
		delete (*fonts.begin());
	}
	/*for(set<IStorm3D_Font*>::iterator ifo=fonts.begin();ifo!=fonts.end();ifo++)
	{
		delete (*ifo);
	}*/

	// Delete lensflares
	while(lflares.begin()!=lflares.end())
	{
		delete (*lflares.begin());
	}
	/*for(set<IStorm3D_LensFlare*>::iterator ilf=lflares.begin();ilf!=lflares.end();ilf++)
	{
		delete (*ilf);
	}*/

	// Delete particles
	/*for(set<IStorm3D_Particle*>::iterator ip=particles.begin();ip!=particles.end();ip++)
	{
		delete (*ip);
	}*/

	// Delete materials
	/*
	while(materials.begin()!=materials.end())
	{
		delete (*materials.begin());
	}
	*/

	/*for(set<IStorm3D_Material*>::iterator im=materials.begin();im!=materials.end();im++)
	{
		delete (*im);
	}*/

	resourceManager.uninitialize();

	/*
	while(textures.begin()!=textures.end())
	{
		delete (*textures.begin());
	}
	*/
	/*
	for(set<IStorm3D_Texture*>::iterator it=textures.begin();it!=textures.end();++it)
	{
		Storm3D_Texture *t = static_cast<Storm3D_Texture *> (*it);
		if(t->IsRenderTarget())
			continue;

		delete (*it);
		//textures.erase(it);
	}
	*/
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
	
	// Clear sets
	/*scenes.clear();
	models.clear();
	meshes.clear();
	fonts.clear();
	lflares.clear();
	//particles.clear();
	materials.clear();
	textures.clear();
	terrains.clear();*/
}



//------------------------------------------------------------------
// Storm3D::GetCurrentDisplayMode
//------------------------------------------------------------------
Storm3D_SurfaceInfo Storm3D::GetCurrentDisplayMode()
{
	Storm3D_Adapter &ada=adapters[active_adapter];
	D3DDISPLAYMODE &dm=ada.display_modes[ada.active_display_mode];
	return Storm3D_SurfaceInfo(dm.Width,dm.Height,GetDisplayModeBPP(dm));
}



//------------------------------------------------------------------
// Storm3D::GetScreenSize
//------------------------------------------------------------------
Storm3D_SurfaceInfo Storm3D::GetScreenSize()
{
	return viewport_size;
}

void Storm3D::TakeScreenshot(const char *file_name)
{
	D3DDISPLAYMODE mode ;
	D3DDevice->GetDisplayMode(0, &mode);
	
	CComPtr<IDirect3DSurface9> front_buffer;
	D3DDevice->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &front_buffer, 0);
	HRESULT hr = D3DDevice->GetFrontBufferData(0, front_buffer);
	if(FAILED(hr))
		return;

	// In case we are in windowed mode, grab only the rendered part
	RECT rect = { 0 };
	GetClientRect(window_handle, &rect);

	POINT a = { rect.left, rect.top };
	POINT b = { rect.right, rect.bottom };

	ClientToScreen(window_handle, &a);
	ClientToScreen(window_handle, &b);

	rect.left = a.x;
	rect.top = a.y;
	rect.right = b.x;
	rect.bottom = b.y;

	if(file_name)
		D3DXSaveSurfaceToFile(file_name, D3DXIFF_BMP, front_buffer, NULL, 0 /*&rect*/);
}

class Storm3D_ScreenBuffer:
	public IStorm3D_ScreenBuffer
{
public:
	VC2I size;
	std::vector<DWORD> buffer;

	const VC2I &getSize() const
	{
		return size;
	}

	const std::vector<DWORD> &getBuffer() const
	{
		return buffer;
	}
};

IStorm3D_ScreenBuffer *Storm3D::TakeScreenshot(const VC2 &area)
{
	CComPtr<IDirect3DSurface9> back_buffer;
	HRESULT hr = D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	if(FAILED(hr))
		return 0;

	D3DSURFACE_DESC desc;
	back_buffer->GetDesc(&desc);

	RECT rect = { 0 };
	rect.right = desc.Width;
	rect.bottom = desc.Height;

	int xSize = rect.right - rect.left;
	int ySize = rect.bottom - rect.top;

	// Calculate area
	{
		int cx = xSize / 2;
		int cy = ySize / 2;
		xSize = int(float(xSize) * area.x);
		ySize = int(float(ySize) * area.y);

		rect.left = cx - xSize/2;
		rect.right = cx + xSize/2;
		rect.top = cy - ySize/2;
		rect.bottom = cy + ySize/2;
	}

	D3DLOCKED_RECT rc;
	hr = back_buffer->LockRect(&rc, 0 /*&rect*/, D3DLOCK_READONLY);
	if(FAILED(hr))
		return 0;

	Storm3D_ScreenBuffer *buffer = new Storm3D_ScreenBuffer();

	int pitch = rc.Pitch / 4;
	DWORD *cBuffer = reinterpret_cast<DWORD *> (rc.pBits);

	buffer->size.x = xSize;
	buffer->size.y = ySize;
	buffer->buffer.resize(xSize * ySize);

	for(int y = 0; y < ySize; ++y)
	for(int x = 0; x < xSize; ++x)
	{
		int yy = rect.top + y;
		int xx = rect.left + x;
		buffer->buffer[y * xSize + x] = cBuffer[yy * pitch + xx];
	}

	back_buffer->UnlockRect();
	return buffer;
}

DWORD Storm3D::getScreenColorValue(const VC2 &area)
{
	CComPtr<IDirect3DSurface9> back_buffer;
	HRESULT hr = D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	if(FAILED(hr))
		return 0;

	D3DSURFACE_DESC desc;
	back_buffer->GetDesc(&desc); // desc.Width, desc.Height

	CComPtr<IDirect3DSurface9> dest_buffer;
	hr = colorSecondaryTarget->GetSurfaceLevel(0, &dest_buffer);
	if(FAILED(hr))
		return 0;
	CComPtr<IDirect3DSurface9> source_buffer;
	hr = colorTarget->GetSurfaceLevel(0, &source_buffer);
	if(FAILED(hr))
		return 0;
	CComPtr<IDirect3DSurface9> value_buffer;
	hr = valueTarget->GetSurfaceLevel(0, &value_buffer);
	if(FAILED(hr))
		return 0;
	CComPtr<IDirect3DSurface9> system_buffer;
	hr = systemTarget->GetSurfaceLevel(0, &system_buffer);
	if(FAILED(hr))
		return 0;

	int sizeX = int(desc.Width * area.x);
	int sizeY = int(desc.Height * area.y);
	RECT rect1 = { 0 };
	rect1.left = (desc.Width - sizeX) / 2;
	rect1.top = (desc.Height - sizeY) / 2;
	rect1.right = (desc.Width + sizeX) / 2;
	rect1.bottom = (desc.Height + sizeY) / 2;
	RECT rect2 = { 0 };
	rect2.right = sizeX;
	rect2.bottom = sizeY;
	D3DDevice->StretchRect(back_buffer, &rect1, source_buffer, &rect2, D3DTEXF_POINT);

	while(sizeX > 1 || sizeY > 1)
	{
		RECT sourceRect = { 0 };
		sourceRect.right = sizeX;
		sourceRect.bottom = sizeY;

		sizeX /= 4;
		sizeY /= 4;
		if(sizeX < 1)
			sizeX = 1;
		if(sizeY < 1)
			sizeY = 1;

		RECT destRect = { 0 };
		destRect.right = sizeX;
		destRect.bottom = sizeY;

		D3DDevice->StretchRect(source_buffer, &sourceRect, dest_buffer, &destRect, D3DTEXF_POINT);
		CComPtr<IDirect3DSurface9> temp_buffer = source_buffer;
		source_buffer = dest_buffer;
		dest_buffer = temp_buffer;
	}

	RECT rc1 = { 0 };
	rc1.right = 1;
	rc1.bottom = 1;
	D3DDevice->StretchRect(source_buffer, &rc1, value_buffer, &rc1, D3DTEXF_LINEAR);
	D3DDevice->GetRenderTargetData(value_buffer, system_buffer);

	D3DLOCKED_RECT rc;
	hr = system_buffer->LockRect(&rc, 0, D3DLOCK_READONLY);
	if(FAILED(hr))
		return 0;

	DWORD *cBuffer = reinterpret_cast<DWORD *> (rc.pBits);
	DWORD result = (cBuffer) ? cBuffer[0] : 0;

	system_buffer->UnlockRect();
	
/*
	RECT rc1 = { 0 };
	rc1.right = 1;
	rc1.bottom = 1;
	D3DDevice->StretchRect(source_buffer, &rc1, back_buffer, &rc1, D3DTEXF_POINT);

	D3DLOCKED_RECT rc;
	hr = back_buffer->LockRect(&rc, 0, D3DLOCK_READONLY);
	if(FAILED(hr))
		return 0;

	DWORD *cBuffer = reinterpret_cast<DWORD *> (rc.pBits);
	DWORD result = (cBuffer) ? cBuffer[0] : 0;

	back_buffer->UnlockRect();
*/
	return result;
}

//------------------------------------------------------------------
// Storm3D::SetRenderTarget
//------------------------------------------------------------------
bool Storm3D::SetRenderTarget(Storm3D_Texture *newtarget,int map)
{
	// NULL means, returning to original target
	if (newtarget==NULL)
	{
		if (bbuf_orig)
		{
			D3DDevice->SetRenderTarget(0, bbuf_orig);
			D3DDevice->SetDepthStencilSurface(zbuf_orig);
			SAFE_RELEASE(bbuf_orig);	// Sets bbuf_orig to NULL
			SAFE_RELEASE(zbuf_orig);	// Sets zbuf_orig to NULL
			//if (FAILED(hr)) MessageBox(NULL,"Error in SetRenderTarget","Storm3D Error",0);
		}

		// Set viewport
		viewport_size=screen_size;
		D3DVIEWPORT9 viewData={0,0,screen_size.width,screen_size.height,0.0f,1.0f};
		D3DDevice->SetViewport(&viewData);

		// Everything went ok
		return true;
	}

	// Is this texture a render target?
	if (!newtarget->IsRenderTarget()) return false;

	// Save original (active) backbuffer
	if (!bbuf_orig)
	{
		D3DDevice->GetRenderTarget(0, &bbuf_orig);
		D3DDevice->GetDepthStencilSurface(&zbuf_orig);
	}

	// Set the target
	if (!newtarget->IsCube())
	{
		if(newtarget->dx_handle)
		{
			// Get surface
			LPDIRECT3DSURFACE9 surf;
			newtarget->dx_handle->GetSurfaceLevel(0,&surf);
			if (surf)
			{
				HRESULT hr=D3DDevice->SetRenderTarget(0, surf);
				D3DDevice->SetDepthStencilSurface(zbuf_orig);
				surf->Release();

				//if (FAILED(hr)) MessageBox(NULL,"Error in SetRenderTarget","Storm3D Error",0);

				// Set viewport
				viewport_size.width=newtarget->width;
				viewport_size.height=newtarget->height;
				D3DVIEWPORT9 viewData={0,0,newtarget->width,newtarget->height,0.0f,1.0f};
				hr=D3DDevice->SetViewport(&viewData);
			}
		}
	}
	else
	{
		// Test
		if (map<0) return false;
		if (map>5) return false;

		// Get cube face
        LPDIRECT3DSURFACE9 surf;
        newtarget->dx_handle_cube->GetCubeMapSurface((D3DCUBEMAP_FACES)map,0,&surf);
        if (surf)
		{
			HRESULT hr=D3DDevice->SetRenderTarget(0, surf);
			D3DDevice->SetDepthStencilSurface(zbuf_orig);
			surf->Release();

			//if (FAILED(hr)) MessageBox(NULL,"Error in SetRenderTarget","Storm3D Error",0);

			// Set viewport
			viewport_size.width=newtarget->width;
			viewport_size.height=newtarget->height;
			D3DVIEWPORT9 viewData={0,0,newtarget->width,newtarget->height,0.0f,1.0f};
			hr=D3DDevice->SetViewport(&viewData);
			if (FAILED(hr)) MessageBox(NULL,"Error in SetViewport","Storm3D Error",0);
		}
	}

	// Everything went ok
	return true;
}


bool Storm3D::RenderSceneToArray( IStorm3D_Scene * stormScene, unsigned char * dest, int width, int height )
{
	Storm3D_Texture * target = CreateNewTextureInstance( width, height, IStorm3D_Texture::TEXTYPE_BASIC_RENDER );

	if( target )
	{
		//stormScene->RenderSceneToDynamicTexture( target );
		SetRenderTarget ( target, 0 );
		D3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0xFFFFFFFF, 1.0f, 0 );
		D3DDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0 );
		stormScene->RenderScene();
		SetRenderTarget ( NULL, 0 );

		target->CopyTextureTo32BitSysMembuffer( (DWORD*)dest );
		Remove( target );
		return true;
	}
	else
	{
		MessageBox( NULL, "Storm: Couldn't create render target.", "Storm error", MB_OK);
	}

	return false;
}

//------------------------------------------------------------------
// Storm3D - Lost device handling
//------------------------------------------------------------------
bool Storm3D::RestoreDeviceIfLost()
{
    // Test the cooperative level to see if device is lost
    HRESULT hr = D3D_OK;
    if(force_reset || FAILED(hr=D3DDevice->TestCooperativeLevel()))
    {
		// If the device was lost, wait until we get it back
// FIXED: hr accessed without being initialized. --jpk
		if(hr==D3DERR_DEVICELOST) 
		{
			//if(logger)
			//	logger->info("Device lost");

			return false;
		}

		// Device is back, but needs to be reset.
		if(force_reset || hr==D3DERR_DEVICENOTRESET)
		{
			if(logger)
			{
				if (force_reset)
				{
					logger->debug("Storm3D::RestoreDeviceIfLost - forcing device reset");
				} else {
					logger->info("Storm3D::RestoreDeviceIfLost - device needs reset");
				}
			}

			// Release all dynamic resources
			ReleaseDynamicResources();

			// Reset the device
			if(FAILED(D3DDevice->Reset(&present_params))) return false;

			// Recreate dynamic resources
			RecreateDynamicResources();

			if(logger)
				logger->debug("Storm3D::RestoreDeviceIfLost - create shaders");

			// Recreate shaders
			if(adapters[active_adapter].caps&Storm3D_Adapter::CAPS_HWSHADER)
				Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, true);
			else
				Storm3D_ShaderManager::GetSingleton()->CreateShaders(D3DDevice, false);

			//MessageBox(0, "", "", MB_OK);
		}

		force_reset = false;

		// Test if it works now...
		if(FAILED(hr=D3DDevice->TestCooperativeLevel())) 
		{
			if(logger)
				logger->debug("Storm3D::RestoreDeviceIfLost - reset path failed");

			return false;
		}
		
		if(logger)
			logger->debug("Storm3D::RestoreDeviceIfLost - reset path succeeded");
	};

	// Everything OK
	return true;
}


void Storm3D::ReleaseDynamicResources()
{
	if(logger)
	{
		logger->debug("Storm3D::ReleaseDynamicResources - releasing dynamic resources");
	}

	Storm3D_TerrainRenderer::freeSecondaryRenderBuffers();
	Storm3D_Spotlight::freeShadowBuffers();
	Storm3D_FakeSpotlight::freeBuffers();
	Storm3D_TerrainRenderer::freeRenderBuffers();

	if(colorTarget)
		colorTarget.Release();
	if(colorSecondaryTarget)
		colorSecondaryTarget.Release();
	if(depthTarget)
		depthTarget.Release();
	if(proceduralTarget)
		proceduralTarget.Release();
	if(proceduralOffsetTarget)
		proceduralOffsetTarget.Release();
	if(valueTarget)
		valueTarget.Release();

	for(set<IStorm3D_Line*>::iterator il=lines.begin();il!=lines.end();++il)
	{
		((Storm3D_Line*)(*il))->releaseDynamicResources();
	}

	for(set<IStorm3D_Terrain*>::iterator itt=terrains.begin();itt!=terrains.end();++itt)
	{
		((Storm3D_Terrain*)(*itt))->releaseDynamicResources();
	}

	for(set<IStorm3D_Font *>::iterator itf = fonts.begin(); itf != fonts.end(); ++itf)
		((Storm3D_Font*)(*itf))->ReleaseDynamicBuffers();

	// Release scenes' particle-vbuffers
	for(set<IStorm3D_Scene*>::iterator is=scenes.begin();is!=scenes.end();++is)
	{
		((Storm3D_Scene*)(*is))->ReleaseDynamicDXBuffers();
	}

	// Release dynamic textures' dx-buffers
	for(set<IStorm3D_Texture*>::iterator it=textures.begin();it!=textures.end();++it)
	{
		((Storm3D_Texture*)(*it))->ReleaseDynamicDXBuffers();
	}	

	proceduralManager.releaseTarget();
}

void Storm3D::RecreateDynamicResources()
{
	if(logger)
	{
		logger->debug("Storm3D::RecreateDynamicResources - recreating dynamic resources");
	}

	bool ps14 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS14)
		ps14 = true;
	bool ps20 = false;
	if(adapters[active_adapter].caps & Storm3D_Adapter::CAPS_PS20)
		ps20 = true;

	createTargets();
	Storm3D_TerrainRenderer::createRenderBuffers(*this, lighting_quality);
	Storm3D_Spotlight::createShadowBuffers(*this, *D3D, *D3DDevice, ps14, ps20, shadow_quality);
	Storm3D_FakeSpotlight::createBuffers(*this, *D3D, *D3DDevice, fake_shadow_quality);

	for(set<IStorm3D_Line*>::iterator il=lines.begin();il!=lines.end();++il)
	{
		((Storm3D_Line*)(*il))->recreateDynamicResources();
	}

	for(set<IStorm3D_Terrain*>::iterator itt=terrains.begin();itt!=terrains.end();++itt)
	{
		((Storm3D_Terrain*)(*itt))->recreateDynamicResources();
	}

	for(set<IStorm3D_Font *>::iterator itf = fonts.begin(); itf != fonts.end(); ++itf)
		((Storm3D_Font*)(*itf))->CreateDynamicBuffers();

	// Release scenes' particle-vbuffers
	for(set<IStorm3D_Scene*>::iterator is=scenes.begin();is!=scenes.end();is++)
	{
		((Storm3D_Scene*)(*is))->ReCreateDynamicDXBuffers();
	}

	// Release dynamic textures' dx-buffers
	for(set<IStorm3D_Texture*>::iterator it=textures.begin();it!=textures.end();it++)
	{
		((Storm3D_Texture*)(*it))->ReCreateDynamicDXBuffers();
	}	

	Storm3D_TerrainRenderer::createSecondaryRenderBuffers(*this, enable_glow);
	//D3DDevice->SetDepthStencilSurface(depthTarget);
}


