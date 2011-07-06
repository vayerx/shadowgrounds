// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_helper.h"
#include "storm3d_bone.h"
#include "storm3d_model_object.h"
#include "..\..\util\Debug_MemoryManager.h"



//------------------------------------------------------------------
// Storm3D_Helper_AInterface::Storm3D_Helper_AInterface
//------------------------------------------------------------------
Storm3D_Helper_AInterface::Storm3D_Helper_AInterface(const char *_name,
		Storm3D_Model *_parent_model,VC3 &_position, IStorm3D_Helper *helper_) :
	update_globals(true),
	parent_model(_parent_model),
	parent_object(NULL),
	parent_bone(NULL),
	position(_position),
	helper(helper_)
{
	
	// Create name
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}


	
//------------------------------------------------------------------
// Storm3D_Helper_AInterface::~Storm3D_Helper_AInterface
//------------------------------------------------------------------
Storm3D_Helper_AInterface::~Storm3D_Helper_AInterface()
{
	if(parent_object)
		parent_object->RemoveChild(helper);
	if(parent_bone)
		parent_bone->RemoveChild(helper);

	delete[] name;
}


