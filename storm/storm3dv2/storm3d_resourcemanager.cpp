// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <stack>
#include <vector>

#include "storm3d_resourcemanager.h"
#include "storm3d_material.h"
#include "storm3d_mesh.h"
#include <cassert>

#include "../../util/Debug_MemoryManager.h"

Storm3D_ResourceManager::Storm3D_ResourceManager()
{
}

// (look for comment in the header) -jpk
void Storm3D_ResourceManager::uninitialize()
{
	/*
	std::map<IStorm3D_Mesh *, int>::iterator it = meshes.begin();
	for(; it != meshes.end(); ++it)
		delete (*it).first;

	std::map<IStorm3D_Material *, int>::iterator im = materials.begin();
	for(; im != materials.end(); ++im)
	{
		++foo;
		Storm3D_Material *const m = static_cast<Storm3D_Material *const> (im->first);
		delete (*im).first;
	}
	*/
	while(meshes.begin() != meshes.end())
		delete meshes.begin()->first;

	while(materials.begin() != materials.end())
		delete materials.begin()->first;

	meshes.clear();
	materials.clear();
}

Storm3D_ResourceManager::~Storm3D_ResourceManager()
{
	uninitialize();
}

void Storm3D_ResourceManager::addResource(IStorm3D_Material *material)
{
	if(!material)
		return;

	assert(materials[material] == 0);
	materials[material] = 0;
}

void Storm3D_ResourceManager::addResource(IStorm3D_Mesh *mesh)
{
	if(!mesh)
		return;

	assert(meshes[mesh] == 0);
	meshes[mesh] = 0;
}

void Storm3D_ResourceManager::deleteResource(IStorm3D_Material *material)
{
	assert(materials[material] == 0);
	materials.erase(material);
}

void Storm3D_ResourceManager::deleteResource(IStorm3D_Mesh *mesh)
{
	assert(meshes[mesh] == 0);
	meshes.erase(mesh);
}

void Storm3D_ResourceManager::addUser(IStorm3D_Material *material, IStorm3D_Mesh *mesh)
{
	if((!material) || (!mesh))
	{
		assert(!"Whoops");
		return;
	}

	++materials[material];
}

void Storm3D_ResourceManager::addUser(IStorm3D_Mesh *mesh, IStorm3D_Model_Object *object)
{
	if((!mesh) || (!object))
	{
		assert(!"Whoops");
		return;
	}

	++meshes[mesh];
}

void Storm3D_ResourceManager::removeUser(IStorm3D_Material *material, IStorm3D_Mesh *mesh)
{
	if((!material) || (!mesh))
	{
		assert(!"Whoops");
		return;
	}

	if(--materials[material] == 0)
	{
		materials.erase(material);
		delete material;
	}
}

void Storm3D_ResourceManager::removeUser(IStorm3D_Mesh *mesh, IStorm3D_Model_Object *object)
{
	if((!mesh) || (!object))
	{
		assert(!"Whoops");
		return;
	}

	if(--meshes[mesh] == 0)
	{
		meshes.erase(mesh);
		delete mesh;
	}
}
