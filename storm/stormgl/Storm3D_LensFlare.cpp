/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_LensFlare

	- Lensflare routines

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_lensflare.h"
#include "storm3d_texture.h"

#include "../../util/Debug_MemoryManager.h"



//------------------------------------------------------------------
// Storm3D_LensFlare::Storm3D_LensFlare
//------------------------------------------------------------------
Storm3D_LensFlare::Storm3D_LensFlare(Storm3D *s2) :
	Storm3D2(s2),
	tex_glow(NULL),
	tex_ring(NULL),
	glow_size(10000),
	ring_size(1000),
	color(1,1,1)
{
}



//------------------------------------------------------------------
// Storm3D_LensFlare::~Storm3D_LensFlare
//------------------------------------------------------------------
Storm3D_LensFlare::~Storm3D_LensFlare()
{
	// Remove from Storm3D's list
	Storm3D2->Remove(this);

	// Delete textures (actually decreases ref.count)
	if (tex_glow) tex_glow->Release();
	if (tex_ring) tex_ring->Release();
}



//------------------------------------------------------------------
// Storm3D_LensFlare - Texture stuff
//------------------------------------------------------------------
void Storm3D_LensFlare::SetGlowTexture(IStorm3D_Texture *itex)
{
	if (itex==NULL)
	{
		// Delete old texture (actually decreases ref.count)
		if (tex_glow) tex_glow->Release();
	}
	else
	{	
		// Typecast
		Storm3D_Texture *tex=(Storm3D_Texture*)itex;

		// Add texture reference count
		tex->AddRef();

		// Delete old texture (actually decreases ref.count)
		if (tex_glow) tex_glow->Release();

		// Set new texture
		tex_glow=tex;
	}
}


void Storm3D_LensFlare::SetRingTexture(IStorm3D_Texture *itex)
{
	if (itex==NULL)
	{
		// Delete old texture (actually decreases ref.count)
		if (tex_ring) tex_ring->Release();
	}
	else
	{	
		// Typecast
		Storm3D_Texture *tex=(Storm3D_Texture*)itex;

		// Add texture reference count
		tex->AddRef();

		// Delete old texture (actually decreases ref.count)
		if (tex_ring) tex_ring->Release();

		// Set new texture
		tex_ring=tex;
	}
}



//------------------------------------------------------------------
// Storm3D_LensFlare - Set other properties
//------------------------------------------------------------------
void Storm3D_LensFlare::SetColor(const COL &_color)
{
	color=_color;
}


void Storm3D_LensFlare::SetGlowSize(float size)
{
	glow_size=size;
}


void Storm3D_LensFlare::SetRingSize(float size)
{
	ring_size=size;
}


