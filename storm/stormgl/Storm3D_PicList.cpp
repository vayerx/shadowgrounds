/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_PicList

	- Particlesystem_PMH (ParticleMaterialHandler)
	- Contains particles with same material

*/

/*

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_PicList::Storm3D_PicList
//------------------------------------------------------------------
Storm3D_PicList::Storm3D_PicList(Storm3D_Material *mat) :
	material(mat)
{
}



//------------------------------------------------------------------
// Storm3D_PicList::~Storm3D_PicList
//------------------------------------------------------------------
Storm3D_PicList::~Storm3D_PicList()
{
}



//------------------------------------------------------------------
// Storm3D_PicList::Render
//------------------------------------------------------------------
void Storm3D_PicList::Render(Storm3D_Scene &scene,int timedif)
{
	// Set material active...
	// Use only base texture, and no other parameters
	if (material)
	{	
		material->ApplyBaseTextureOnly();
	}
	else
	{
		Storm3D2.D3DDevice->SetTexture(0,NULL);
	}

	// Render the list
	particlelist.Render(scene,timedif);
}
*/

