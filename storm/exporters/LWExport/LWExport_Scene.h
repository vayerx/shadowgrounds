// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LWEXPORT_SCENE_H
#define INCLUDED_LWEXPORT_SCENE_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#endif

#ifndef INCLUDED_LWEXPORT_OBJECT_H
#include "LWExport_Object.h"
#endif
#ifndef INCLUDED_LWEXPORT_MATERIAL_H
#include "LWExport_Material.h"
#endif
#ifndef INCLUDED_LWEXPORT_BONE_H
#include "LWExport_Bone.h"
#endif
#ifndef INCLUDED_LWEXPORT_HELPER_H
#include "LWExport_Helper.h"
#endif

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

namespace frozenbyte {
namespace exporter {

/**  @class LWScene
  *  @brief Collects relevant data from scene
  *  @author Juha Hiekkamäki
  *  @version 1.0
  *  @date 2001
  */
class LWScene
{
	std::vector<LWObject> objects;
	std::vector<LWMaterial> materials;
	std::vector<LWBone> bones;
	std::vector<LWHelper> helpers;

	void collectMaterials();
	void collectBones();
	void collectObjects();
	void collectHelpers();
	void fixLightmaps();

public:
	LWScene();
	~LWScene();

	void collectData();
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
