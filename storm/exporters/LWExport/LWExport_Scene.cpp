// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Scene.h"
#include "LWExport_Manager.h"
#include "LWExport_Material.h"
#include "LWExport_Bone.h"
#include "LWExport_Object.h"
#include "..\Shared\Export_Exporter.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>

namespace frozenbyte {
namespace exporter {
namespace {

	struct MaterialSorter
	{
		bool operator () (const LWMaterial &a, const LWMaterial &b) const
		{
			return a.getName() < b.getName();
		}
	};

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

LWScene::LWScene()
{
}

LWScene::~LWScene()
{
}

void LWScene::collectMaterials()
{
	LWSurfaceFuncs *surfaceFunctions = Manager::getSingleton()->getSurfaceFunctions();
	if(surfaceFunctions == 0)
		return;

	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();
	if(!objectInfo || !itemInfo)
		return;

	LWItemID objectId = itemInfo->first(LWI_OBJECT, NULL);
	while(objectId != LWITEM_NULL)
	{
		const char *fname = objectInfo->filename(objectId);
		LWSurfaceID *surfaceId = surfaceFunctions->byObject(fname);
		if(!surfaceId)
		{
			objectId = itemInfo->next(objectId);
			continue;
		}

		for(int i = 0; surfaceId[i] != 0; ++i)
		{
			LWMaterial material(surfaceId[i]);
			if(material.collectProperties() == true)
				materials.push_back(material);
		}

		objectId = itemInfo->next(objectId);
	}

	unsigned int materialAmount = materials.size();
	unsigned int i = 0;

	for(i = 0; i < materialAmount; ++i)
	{
		if(!materials[i].getLightmapId())
			continue;

		LWMaterial copy = materials[i];
		copy.removeLightmap();
		materials.push_back(copy);
	}

	std::sort(materials.begin(), materials.end(), MaterialSorter());

	//unsigned int i = 0;
	for(i = 0; i < materials.size(); ++i)
		materials[i].add();
}

void LWScene::collectBones()
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();
	LWBoneInfo *boneInfo = Manager::getSingleton()->getBoneInfo();

	if((itemInfo == 0) || (objectInfo == 0) || (boneInfo == 0))
		return;

	// We have to loop all models and query bones from them
	LWItemID objectId = itemInfo->first(LWI_OBJECT, NULL);
	while(objectId != LWITEM_NULL)
	{
		LWItemID boneId = itemInfo->first(LWI_BONE, objectId);
		while(boneId != LWITEM_NULL)
		{
			// Add bone
			LWBone bone(boneId);
			if(bone.collectData() == true)
				bones.push_back(bone);

			boneId = itemInfo->next(boneId);
		}
		
		objectId = itemInfo->next(objectId);
	}

	if(!bones.empty())
	{
		LWSceneInfo *sceneInfo = Manager::getSingleton()->getSceneInfo();
		LWInterfaceInfo *interfaceInfo = Manager::getSingleton()->getInterfaceInfo();
		LWLayoutGeneric *local = Manager::getSingleton()->getLayoutGeneric();

		int startTime = static_cast<int> ((1000 * interfaceInfo->previewStart) / sceneInfo->framesPerSecond);
		int endTime = static_cast<int> ((1000 * interfaceInfo->previewEnd) / sceneInfo->framesPerSecond);
		double currentTime = interfaceInfo->curTime;
		int animationLength = endTime - startTime - 1;
		if(animationLength == 0)
			return;

		Manager::getSingleton()->getExporter()->setAnimationLength(animationLength);
		for(int i = startTime; i < endTime; i += Manager::getSingleton()->getExporter()->getFrameLength())
		{
			float lwTime = i / 1000.f;
			setFrame(local, lwTime * sceneInfo->framesPerSecond);

			for(unsigned int j = 0; j < bones.size(); ++j)
				bones[j].getKey(lwTime, i - startTime);
		}

		float lwTime = (endTime - 1) / 1000.f;
		setFrame(local, lwTime * sceneInfo->framesPerSecond);

		for(unsigned int j = 0; j < bones.size(); ++j)
			bones[j].getKey(lwTime, animationLength - 1);

		setFrame(local, currentTime * sceneInfo->framesPerSecond);
	}
}

void LWScene::collectObjects()
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();

	if((itemInfo == 0) || (objectInfo == 0))
		return;

	// Loop all objects
	LWItemID objectId = itemInfo->first(LWI_OBJECT, NULL);
	while(objectId != LWITEM_NULL)
	{
		// Add object
		LWObject object(objectId);
		if(object.collectGeometry(materials, bones) == true)
			objects.push_back(object);

		objectId = itemInfo->next(objectId);
	}
}

void LWScene::collectHelpers()
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return;

	// Loop all items. Helpers can be almost any type
	LWItemID itemId = itemInfo->first(LWI_OBJECT, NULL);
	while(itemId != LWITEM_NULL)
	{
		// Add item
		LWHelper helper(itemId);
		if(helper.collectData() == true)
			helpers.push_back(helper);

		itemId = itemInfo->next(itemId);
	}
}

void LWScene::fixLightmaps()
{
}

void LWScene::collectData()
{
	// Bones and animation
	collectBones();

	// Model
	collectMaterials();
	collectObjects();
	collectHelpers();

	fixLightmaps();
}

} // end of namespace export
} // end of namespace frozenbyte
