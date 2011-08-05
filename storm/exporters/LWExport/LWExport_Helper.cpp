// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Helper.h"
#include "LWExport_Manager.h"
#include "LWExport_Transform.h"

#include "..\Shared\Export_Helper.h"
#include "..\Shared\Export_Exporter.h"
#include <sstream>

namespace frozenbyte {
namespace exporter {

LWHelper::LWHelper(LWItemID id)
:	lwId(id),
	parentId(0)
{
}

LWHelper::~LWHelper()
{
}

bool LWHelper::collectData()
{
	// Currently, we are only interested about null objects
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return false;

	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();
	if(objectInfo == 0)
		return false;

	// Null objects don't have geometry
	LWMeshInfo *meshInfo = objectInfo->meshInfo(lwId, TRUE);
	if(meshInfo != 0)
	{
		if(meshInfo->destroy)
			meshInfo->destroy(meshInfo);

		return false;
	}

	// Name
	name = itemInfo->name(lwId);
	if(name.substr(0, 6) != "HELPER")
		return false;

	// Parent info
	parentId = itemInfo->parent(lwId);
	std::string parentName;
	if(parentId)
		parentName = itemInfo->name(parentId);

	// Make transform relative
	/*
	Matrix transform = LWTransforms::GetTransform(lwId);
	Matrix parentTransform = LWTransforms::GetTransform(parentId);
	if(itemInfo->type(parentId) == LWI_BONE)
		transform = LWTransforms::GetLocalTransform(lwId) * parentTransform;
	*/
	FBMatrix transform = LWTransforms::GetBoneTransform(lwId, 0);
	FBMatrix parentTransform = LWTransforms::GetBoneTransform(parentId, 0);
	if(itemInfo->type(parentId) == LWI_BONE)
		transform = LWTransforms::GetLocalTransform(lwId) * parentTransform;

	FBQuaternion tq = transform.GetRotation();
	FBQuaternion pq = parentTransform.GetRotation();
	FBQuaternion lq = tq * pq.GetInverse();
/*
	if(itemInfo->type(parentId) == LWI_BONE)
	{
		std::stringstream stream;
		stream << name << "\r\n";
		stream << "World rotation: " << tq.x << "," << tq.y << "," << tq.z << "," << tq.w << "\r\n";
		stream << "Parent rotation: " << pq.x << "," << pq.y << "," << pq.z << "," << pq.w << "\r\n";
		stream << "Local rotation: " << lq.x << "," << lq.y << "," << lq.z << "," << lq.w << "\r\n";
		MessageBox(0, stream.str().c_str(), "Bone parent", MB_OK);
	}
	else
	{
		std::stringstream stream;
		stream << name << "\r\n";
		stream << "World rotation: " << tq.x << "," << tq.y << "," << tq.z << "," << tq.w << "\r\n";
		stream << "Parent rotation: " << pq.x << "," << pq.y << "," << pq.z << "," << pq.w << "\r\n";
		stream << "Local rotation: " << lq.x << "," << lq.y << "," << lq.z << "," << lq.w << "\r\n";
		MessageBox(0, stream.str().c_str(), "Normal parent", MB_OK);
	}
*/
	transform = transform * parentTransform.GetInverse();
	FBVector position;
	position.x = transform.Get(12);
	position.y = transform.Get(13);
	position.z = transform.Get(14);

	FBVector direction;
	direction.x = transform.Get(8);
	direction.y = transform.Get(9);
	direction.z = transform.Get(10);

	FBVector up;
	up.x = transform.Get(4);
	up.y = transform.Get(5);
	up.z = transform.Get(6);

	// Camera
	if(name.substr(0, 11) == "HELPER_BONE")
	{
		Helper helper(Helper::camera, name, parentName);
		
		helper.setPosition(position);
		helper.setDirection(direction);
		helper.setUpVector(up);

		// Store
		Manager::getSingleton()->getExporter()->addBoneHelper(helper);			
	}
	// Direction
	else
	{
		Helper helper(Helper::vector, name, parentName);
		
		helper.setPosition(position);
		helper.setDirection(direction);

		Manager::getSingleton()->getExporter()->getModel().addHelper(helper);
	}

	return false;
}

} // end of namespace export
} // end of namespace frozenbyte
