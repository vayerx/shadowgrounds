// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Bone.h"
#include "LWExport_Manager.h"
#include "LWExport_Transform.h"

#include "..\Shared\Export_Bone.h"
#include "..\Shared\Export_Exporter.h"
#include <boost/lexical_cast.hpp>

namespace frozenbyte {
namespace exporter {
namespace {

bool validateBoneProperties(LWItemID id)
{
	LWBoneInfo *boneInfo = Manager::getSingleton()->getBoneInfo();
	if(!boneInfo)
		return false;

	if(!(boneInfo->flags(id) & LWBONEF_ACTIVE))
		return false;

	// No helper leafs
	//if(!boneInfo->weightMap(id))
	//	return false;
	
	return true;
}

} // unnamed

LWBone::LWBone(LWItemID id)
:	lwId(id),
	parentId(0),
	exporterId(-1)
{
}

LWBone::~LWBone()
{
}

const std::string &LWBone::getWeightName() const
{
	return weightMap;
}

bool LWBone::collectData()
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWBoneInfo *boneInfo = Manager::getSingleton()->getBoneInfo();

	if((itemInfo == 0) || (boneInfo == 0))
		return false;

	if(validateBone() == false)
		return false;

	name = itemInfo->name(lwId);
	parentId = itemInfo->parent(lwId);
	if(parentId && !validateBoneProperties(parentId))
		parentId = 0;

	if((parentId) && (itemInfo->type(parentId) == LWI_BONE))
		parentName = itemInfo->name(parentId);

	if(const char *cMap = boneInfo->weightMap(lwId))
		weightMap = cMap;

	// Transform
	FBMatrix boneTm = LWTransforms::GetBoneRestTransform(lwId);

	double inner = 0;
	double outer = 0;
	boneInfo->limits(lwId, &inner, &outer);
	float boneRadius = float(outer);

	double boneLength = boneInfo->restLength(lwId);
	double scale = LWTransforms::GetBoneScale(lwId);

	// Bone position
	boneTm.Set(12, boneTm.Get(12) * scale);
	boneTm.Set(13, boneTm.Get(13) * scale);
	boneTm.Set(14, boneTm.Get(14) * scale);

	// Exporter instance
	Bone bone(name, parentName);
	bone.setRestTransform(boneTm);
	bone.setRestLength(boneLength * scale);
	bone.setRadius(boneRadius);

	// Store
	exporterId = Manager::getSingleton()->getExporter()->addBone(bone);
	return true;
}

bool LWBone::validateBone()
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(!itemInfo)
		return false;

	// No helper leafs
	if(((parentId == 0) || (itemInfo->type(parentId) != LWI_BONE)) && !validateBoneProperties(lwId))
		return false;

	return true;
}

namespace {

	template<typename T>
	void lwCommand(LWLayoutGeneric *local, const std::string &name_, T param)
	{
		std::string name = name_;
		name += " ";

		try
		{
			name += boost::lexical_cast<std::string> (param);
		}
		catch(...)
		{
		}

		local->evaluate(local->data, name.c_str());
	}

	void setFrame(LWLayoutGeneric *local, double index)
	{
		lwCommand(local, "GoToFrame", index);
	}
}

void LWBone::getKey(float time, int ms)
{
	FBMatrix transform = LWTransforms::GetBoneTransform(lwId, time);
	Manager::getSingleton()->getExporter()->addBoneKey(exporterId, ms, transform);
}

/*
void LWBone::collectKeys()
{
	LWSceneInfo *sceneInfo = Manager::getSingleton()->getSceneInfo();
	LWInterfaceInfo *interfaceInfo = Manager::getSingleton()->getInterfaceInfo();
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWLayoutGeneric *local = Manager::getSingleton()->getLayoutGeneric();

	if(!sceneInfo || !interfaceInfo || !itemInfo || !local)
		return;

	// Times in ms
	int startTime = static_cast<int> ((1000 * interfaceInfo->previewStart) / sceneInfo->framesPerSecond);
	int endTime = static_cast<int> ((1000 * interfaceInfo->previewEnd) / sceneInfo->framesPerSecond);
	int animationLength = endTime - startTime - 1;
	if(animationLength == 0)
		return;

	Manager::getSingleton()->getExporter()->setAnimationLength(animationLength);
	for(int i = startTime; i < endTime; i += Manager::getSingleton()->getExporter()->getFrameLength())
	{
		float lwTime = i / 1000.f;
		setFrame(local, lwTime * sceneInfo->framesPerSecond);

		FBMatrix transform = LWTransforms::GetBoneTransform(lwId, lwTime);
		Manager::getSingleton()->getExporter()->addBoneKey(exporterId, i - startTime, transform);
	}

	float lwTime = (endTime - 1) / 1000.f;
	setFrame(local, lwTime * sceneInfo->framesPerSecond);

	FBMatrix transform = LWTransforms::GetBoneTransform(lwId, lwTime);
	Manager::getSingleton()->getExporter()->addBoneKey(exporterId, animationLength - 1, transform);
}
*/

} // end of namespace export
} // end of namespace frozenbyte
