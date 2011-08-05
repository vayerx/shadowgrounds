// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_RESOURCEMANAGER_H
#define INCLUDED_STORM3D_RESOURCEMANAGER_H

class IStorm3D_Material;
class IStorm3D_Mesh;
class IStorm3D_Model_Object;

#include <map>

class Storm3D_ResourceManager
{
	std::map<IStorm3D_Material *, int> materials;
	std::map<IStorm3D_Mesh *, int> meshes;

public:
	Storm3D_ResourceManager();
	~Storm3D_ResourceManager();

	// Should only be called right before delete!
	// Does not need to be called, but can be called in order to 
	// clean up sooner (because otherwise this won't be deleted by
	// storm3d before d3d is released) -jpk
	void uninitialize();

	void addResource(IStorm3D_Material *material);
	void addResource(IStorm3D_Mesh *mesh);
	
	void deleteResource(IStorm3D_Material *material);
	void deleteResource(IStorm3D_Mesh *mesh);

	void addUser(IStorm3D_Material *material, IStorm3D_Mesh *mesh);
	void addUser(IStorm3D_Mesh *mesh, IStorm3D_Model_Object *object);

	void removeUser(IStorm3D_Material *material, IStorm3D_Mesh *mesh);
	void removeUser(IStorm3D_Mesh *mesh, IStorm3D_Model_Object *object);
};

#endif
