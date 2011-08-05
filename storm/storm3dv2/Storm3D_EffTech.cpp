/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_EffTech

  Effect/Technique (pair)

  Handles a single texturing effect/technique.

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>
#include "Techniques.h"



//------------------------------------------------------------------
// Storm3D_EffTech::Storm3D_EffTech
//------------------------------------------------------------------
Storm3D_EffTech::Storm3D_EffTech() :
	effect(NULL),
	technique(NULL)
{
}



//------------------------------------------------------------------
// Storm3D_EffTech::Storm3D_EffTech
//------------------------------------------------------------------
Storm3D_EffTech::Storm3D_EffTech(char *filename) :
	effect(NULL),
	technique(NULL)
{
	InitEffect(filename);
}



//------------------------------------------------------------------
// Storm3D_EffTech::~Storm3D_EffTech
//------------------------------------------------------------------
Storm3D_EffTech::~Storm3D_EffTech()
{
	// Delete effect/technique
	SAFE_RELEASE(effect);
	SAFE_RELEASE(technique);
}



//------------------------------------------------------------------
// Storm3D_EffTech::GetBestTechniqueForEffect
//------------------------------------------------------------------
LPD3DXTECHNIQUE Storm3D_EffTech::GetBestTechniqueForEffect()
{
	// If there was an error in effect, no technique can be found.
	if (effect==NULL) return NULL;

	// Get best (=first) supported technique from effect
	LPD3DXTECHNIQUE tech=NULL;
	DWORD dw=0;
	do
	{
		// Release old
        SAFE_RELEASE(tech);

		// Get next technique from effect
		effect->GetTechnique(dw++,&tech);

		if (tech==NULL)
		{
			// All techniques tested, but none is supported
			return NULL;
		}

	} while (tech->Validate()!=D3D_OK);

	// Return the best (first found) technique
	return tech;
}



//------------------------------------------------------------------
// Storm3D_EffTech::InitEffect
//------------------------------------------------------------------
void Storm3D_EffTech::InitEffect(char *filename)
{
	// Compile effect
	LPD3DXBUFFER compeff;
	LPD3DXBUFFER errors;
	if (FAILED(D3DXCompileEffectFromFile(filename,&compeff,&errors)))
	{
		char s[500];
		memcpy(s,errors->GetBufferPointer(),errors->GetBufferSize());
		s[errors->GetBufferSize()]=0;
		MessageBox(NULL,s,"Storm3D Error",0);
		return;
	}

	// Create effect
	D3DXCreateEffect(Storm3D2.D3DDevice,compeff->GetBufferPointer(),
		compeff->GetBufferSize(),0,&effect);
	SAFE_RELEASE(compeff);

	// Initialize effect's variables
	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	effect->SetMatrix(mID,&id);
	effect->SetMatrix(mREF,&id);
	effect->SetDword(dBT,D3DTOP_MODULATE);
	//effect->SetDword(dBPS,D3DBLEND_ZERO);
	//effect->SetDword(dBPD,D3DBLEND_SRCCOLOR);
	effect->SetDword(dBPS,D3DBLEND_ONE);
	effect->SetDword(dBPD,D3DBLEND_ONE);

	// Get best technique for effect and store it to Storm3D
	technique=GetBestTechniqueForEffect();
}


	
