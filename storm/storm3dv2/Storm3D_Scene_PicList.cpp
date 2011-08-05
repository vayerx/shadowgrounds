// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_scene_piclist.h"
#include "../../util/Debug_MemoryManager.h"


//------------------------------------------------------------------
// Storm3D_Scene_PicList::Storm3D_Scene_PicList
//------------------------------------------------------------------
Storm3D_Scene_PicList::Storm3D_Scene_PicList(Storm3D *s2,Storm3D_Scene *_scene,
											 VC2 _position,VC2 _size) :
	scene(_scene),	
	Storm3D2(s2),
	position(VC3(_position.x,_position.y,0)),
	size(_size)
{
}

//------------------------------------------------------------------
// Storm3D_Scene_PicList::Storm3D_Scene_PicList
//------------------------------------------------------------------
Storm3D_Scene_PicList::Storm3D_Scene_PicList(Storm3D *s2,Storm3D_Scene *_scene,
											 VC3 _position,VC2 _size) :
	scene(_scene),
	Storm3D2(s2),
	position(_position),
	size(_size)
{
}

//------------------------------------------------------------------
// Storm3D_Scene_PicList::Render
//------------------------------------------------------------------
void Storm3D_Scene_PicList::Render()
{
	// Do nothing...
}
