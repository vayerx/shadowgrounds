// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EXPORT_EXPORTER_H
#define INCLUDED_EXPORT_EXPORTER_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#pragma warning(disable: 4710) // function not inlined
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

#ifndef INCLUDED_EXPORT_MODEL_H
#include "Export_Model.h"
#endif
#ifndef INCLUDED_EXPORT_OBJECT_H
#include "Export_Object.h"
#endif
#ifndef INCLUDED_EXPORT_MATERIAL_H
#include "Export_Material.h"
#endif
#ifndef INCLUDED_EXPORT_BONE_H
#include "Export_Bone.h"
#endif

#include "Export_Types.h"

namespace frozenbyte {
namespace exporter {

struct AnimationKey
{
	AnimationKey()
		:	time(0) {}
	AnimationKey(int time_, const FBMatrix &transform_)
		:	time(time_),
			transform(transform_) {}
				
	int time; // ms
	FBMatrix transform;
};

class Exporter
{
	Model model;

	std::vector<Bone> bones;
	std::vector<Helper> boneHelpers;
	std::string info;

	int animationLength;
	int boneId;
	static const int frameLength;

	// [boneIndex[keyIndex]] = Animation key
	std::vector<std::vector<AnimationKey> > boneKeys;

public:
	Exporter();
	~Exporter();

	const Model &getModel() const;
	const std::vector<Bone> &getBones() const;

	Model &getModel();

	// Just for kicks
	const std::string &getExporterInfo() const;
	std::string getSummaryInfo() const;
	std::string getModelInfo() const;
	std::string getBoneInfo() const;
	std::string getAnimationInfo() const;

	void printWarning(const std::string &message);
	void printInfo(const std::string &message);

	int addBone(const Bone &bone);
	int addBoneHelper(const Helper &helper);

	// Bone methods
	int getBoneId() const;
	void addBoneKey(int index, int time, const FBMatrix &transform);

	void saveBonesToFile(const std::string &fileName);
	void saveBoneAnimationToFile(const std::string &fileName, const std::vector<std::string> &boneNames);

	// Milliseconds
	void setAnimationLength(int length);
	int getFrameLength();

	void validateData();

private:
	void buildBoneHierarchy();
	void removeRedundantMaterials();
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
