/*

  Storm3D v3 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000-2001

*/


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Light.h"



//------------------------------------------------------------------
// Storm3D_LensFlare
//------------------------------------------------------------------
class Storm3D_LensFlare : public IStorm3D_LensFlare
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	Storm3D_Texture *tex_glow;		// NULL if not
	Storm3D_Texture *tex_ring;		// NULL if not
	float glow_size;
	float ring_size;
	COL color;

public:

	// Constructor/destructor
	Storm3D_LensFlare(Storm3D *Storm3D2);
	~Storm3D_LensFlare();

	// Set textures (set NULL if you don't want glow/rings)
	void SetGlowTexture(IStorm3D_Texture *texture);
	void SetRingTexture(IStorm3D_Texture *texture);

	// Set other properties
	void SetColor(const COL &color);
	void SetGlowSize(float size);
	void SetRingSize(float size);

	friend class Storm3D_Scene_LightHandler;
};



