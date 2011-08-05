// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <SDL.h>

#include "storm3d_common_imp.h"
#include "Storm3D_Datatypes.h"



//------------------------------------------------------------------
// Storm3D_Adapter
//------------------------------------------------------------------
class Storm3D_Adapter
{
	enum CAPS
	{
		CAPS_TEX_PROJECTED=0x00000001,
		CAPS_TEX_CUBE=0x00000002,
		CAPS_TEX_VOLUME=0x00000004,

		CAPS_EMBM=0x00000008,
		CAPS_DOT3=0x00000010,

		CAPS_HW_TL=0x00000020,
		CAPS_HW_MESH_INTERPOLATION=0x00000040,
		
		CAPS_BEZ_SPL_PATCHES=0x00000080,
		CAPS_NPATCHES=0x00000100,

		CAPS_ANISOTROPIC=0x00000200,

		CAPS_PIXELFOG=0x00000400,
		CAPS_HWSHADER=0x00000800,
		CAPS_PS11    =0x00001000,
		CAPS_PS13    =0x00002000,
		CAPS_PS14    =0x00004000,
		CAPS_PS20    =0x00008000
	};

	DWORD caps;					// Adapter caps

	int multisample;			// Multisample antialiasing level
	int max_anisotropy;		// Max anisotropy level
	int multitex_layers;		// Simultaneous texturelayers (multitexturing)

	SDL_Rect *display_modes;
	int display_mode_amount;
	int active_display_mode;

	float maxPixelShaderValue;
	int user_clip_planes;
	Storm3D_SurfaceInfo maxtexsize;	// Max texture size!

	bool stretchFilter;

public:

	// Creation/delete
	Storm3D_Adapter();
	~Storm3D_Adapter();

	friend class Storm3D;
	friend class Storm3D_Texture;
	friend class Storm3D_Material;
	friend class Storm3D_Scene;
	friend struct Storm3D_TerrainRendererData;
	friend class Storm3D_Spotlight;
};



