// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatype includes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Material.h"

struct Storm3D_PointParticle {
	VC3 position;
	COL color;
	float alpha;
	float size;
	float frame;
	float angle;
};

struct Storm3D_LineParticle {
	VC3 position[2];
	COL color[2];
	float alpha[2];
	float size[2];
	float frame;
};

struct Storm3D_ParticleTextureAnimationInfo {
	int numFrames;
	int textureUSubDivs;
	int textureVSubDivs;
};


class ST3D_EXP_DLLAPI IStorm3D_ParticleSystem {
public:
	
	virtual ~IStorm3D_ParticleSystem() {}
	
	virtual void renderPoints(IStorm3D_Material* mtl, Storm3D_PointParticle* parts, int nParts, const COL &factor = COL(), bool distortion = false)=0;
	virtual void renderQuads(IStorm3D_Material* mtl, Storm3D_PointParticle* parts, int nParts,
		Storm3D_ParticleTextureAnimationInfo* info = NULL, const COL &factor = COL(), bool distortion = false, bool faceUp = false)=0;
	virtual void renderLines(IStorm3D_Material* mtl, Storm3D_LineParticle* parts, int nParts,
		Storm3D_ParticleTextureAnimationInfo* info = NULL, const COL &factor = COL(), bool distortion = false)=0;


};

// maybe could be better like this ?
/*

	Storm3D_PointParticle* renderPoints(IStorm3D_Material* mtl, int nParts);

*/

