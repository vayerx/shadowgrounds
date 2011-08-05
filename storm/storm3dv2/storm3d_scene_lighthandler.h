// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "storm3d_light.h"



//------------------------------------------------------------------
// Storm3D_Scene_LightHandler
//------------------------------------------------------------------
class Storm3D_Scene_LightHandler
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// light-set
	set<IStorm3D_Light*> lights;

	// UpdateLights (called when scene is rendered)
	void UpdateLights(Storm3D_Scene *scene);

public:

	// ApplyLights (called for each rendered object)
	void ApplyLights(Storm3D_Model_Object *obj);

	// Render lights lensflares
	void RenderLensFlares(Storm3D_Scene *scene);

	// Creation/delete (automatically updates lights when created)
	Storm3D_Scene_LightHandler(Storm3D *Storm3D2,Storm3D_Scene *scene);
	~Storm3D_Scene_LightHandler();
};



