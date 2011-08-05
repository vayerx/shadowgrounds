// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Exporter.h"

#include <algorithm>
#include <cstdio>

namespace frozenbyte {
namespace exporter {

const int Exporter::frameLength = 40; // fps 25

Exporter::Exporter()
:	boneId(0),
	animationLength(0)
{
}

Exporter::~Exporter()
{
}

const Model &Exporter::getModel() const
{
	return model;
}

const std::vector<Bone> &Exporter::getBones() const
{
	return bones;
}

Model &Exporter::getModel()
{
	return model;
}


const std::string &Exporter::getExporterInfo() const
{
	return info;
}

std::string Exporter::getSummaryInfo() const
{
	std::string result = model.getSummary();
	
	char number[20] = { 0 };
	_itoa(bones.size(), number, 10);

	result += "Number of bones: " + std::string(number) + std::string("\r\n");
	return result;
}

std::string Exporter::getModelInfo() const
{
	return model.getDetails();
}

std::string Exporter::getBoneInfo() const
{
	std::string result;
	
	char number[20] = { 0 };
	_itoa(bones.size(), number, 10);
	std::string boneAmount = number;

	for(unsigned int i = 0; i < bones.size(); ++i)
	{
		_itoa(i+1, number, 10);
		result += "Bone (" + std::string(number) + "/" + boneAmount + ")\r\n";
		result += "\tName: " + bones[i].getName() + "\r\n";
		result += "\tParent: " + bones[i].getParentName() + "\r\n";
		_gcvt(bones[i].getRestLength(), 7, number);
		result += "\tRest length: " + std::string(number) + "\r\n";

		_gcvt(bones[i].getRadius(), 7, number);
		result += "\tBone radius: " + std::string(number) + "\r\n";
	}

	return result;
}

std::string Exporter::getAnimationInfo() const
{
	std::string result;
	
	char number[20] = { 0 };
	_itoa(animationLength, number, 10);

	result += "Animation length: " + std::string(number) + " milliseconds \r\n";
	_itoa(1000 / frameLength, number, 10);
	result += "Internal frame rate: " + std::string(number) + "\r\n";
	_itoa(animationLength / frameLength, number, 10);
	result += "Samples per bone: " + std::string(number) + "\r\n";

	return result;
}

void Exporter::printWarning(const std::string &message)
{
	info += "Warning:\t    " + message + "\r\n";
}

void Exporter::printInfo(const std::string &message)
{
	info += "Info: \t    " + message + "\r\n";
}

int Exporter::addBone(const Bone &bone)
{
	bones.push_back(bone);
	boneKeys.resize(bones.size());
	
	return bones.size() - 1;
}

int Exporter::addBoneHelper(const Helper &helper)
{
	boneHelpers.push_back(helper);
	return boneHelpers.size() - 1;
}

void Exporter::addBoneKey(int index, int time, const FBMatrix &transform)
{
	assert(static_cast<unsigned int> (index) <= boneKeys.size());
	boneKeys[index].push_back(AnimationKey(time, transform));
}

int Exporter::getBoneId() const
{
	return boneId;
}

void Exporter::saveBonesToFile(const std::string &fileName)
{
	FILE *fp = fopen(fileName.c_str(), "wb");
	
	// Header
	fwrite("B3D11", 1, 5, fp); // Version 1
	fwrite(&boneId, 1, sizeof(int), fp); // Bone id

	int boneAmount = static_cast<int> (bones.size());
	fwrite(&boneAmount, 1, sizeof(int), fp); // Bone amount

	for(unsigned int i = 0; i < bones.size(); ++i)
	{
		// Get parents transform (if has)
		FBMatrix parentTm;
		int parentIndex = -1;

		for(unsigned int j = 0; j < i; ++j)
		{
			if(bones[i].getParentName() == bones[j].getName())
			{
				parentTm = bones[j].getRestTransform();
				parentIndex = j;
				break;
			}
		}

		// Relative tm
		FBMatrix tm = bones[i].getRestTransform();
		FBMatrix relativeTm = tm * parentTm.GetInverse();

		float length = float(bones[i].getRestLength());
		float radius = float(bones[i].getRadius());

		Vector position = convert(relativeTm.GetTranslation());
		Rotation rotation = convert(relativeTm.GetRotation());
		Vector originalPosition = convert(tm.GetTranslation());
		Rotation originalRotation = convert(tm.GetRotation());
		Vector minAngles = convert(bones[i].getMinAngles());
		Vector maxAngles = convert(bones[i].getMaxAngles());
		
		// Write to file

		const std::string &name = bones[i].getName();
		fwrite(name.c_str(), 1, name.size() + 1, fp); // name

		fwrite(&position, sizeof(float), 3, fp); // Position
		fwrite(&rotation, sizeof(float), 4, fp); // Rotation
		fwrite(&originalPosition, sizeof(float), 3, fp); // Original position
		fwrite(&originalRotation, sizeof(float), 4, fp); // Original rotation
		fwrite(&minAngles, sizeof(float), 3, fp); // Min angles
		fwrite(&maxAngles, sizeof(float), 3, fp); // Max angles
	
		fwrite(&length, sizeof(float), 1, fp); // Length
		fwrite(&radius, sizeof(float), 1, fp); // Thickness
		fwrite(&parentIndex, sizeof(int), 1, fp); // Parent
	}

	// Bone helpers (here for compatibility)
	int helperAmount = static_cast<int> (boneHelpers.size());
	fwrite(&helperAmount, 1, sizeof(int), fp); 

	for(unsigned int i = 0; i < boneHelpers.size(); ++i)
		boneHelpers[i].writeToFile(fp);

	fclose(fp);
}

void Exporter::saveBoneAnimationToFile(const std::string &fileName, const std::vector<std::string> &boneNames)
{
	FILE *fp = fopen(fileName.c_str(), "wb");
	assert(boneKeys.size() == bones.size());

	// Header
	fwrite("ANM11", 1, 5, fp); // Version 1

	fwrite(&boneId, sizeof(int), 1, fp); // Bone id
	fwrite(&animationLength, sizeof(int), 1, fp); // Bone id

	int boneAmount = static_cast<int> (boneKeys.size());
	fwrite(&boneAmount, sizeof(int), 1, fp); // Bone amount

	for(unsigned int i = 0; i < static_cast<unsigned int> (boneAmount); ++i)
	{
		int parentIndex = -1;

		// Find parent
		for(unsigned int j = 0; j < i; ++j)
		{
			if(bones[i].getParentName() == bones[j].getName())
				parentIndex = j;
		}

		int rotationKeyAmount = static_cast<int> (boneKeys[i].size());
		int positionKeyAmount = 0;
		if(parentIndex == -1)
			positionKeyAmount = rotationKeyAmount; // Only parents can move

		// If this bone is not on the list, ignore
		bool boneFound = false;
		for(unsigned int j = 0; j < boneNames.size(); ++j)
		{
			if(boneNames[j] == bones[i].getName())
			{
				boneFound = true;
				break;
			}
		}

		if(boneFound == false)
		{
			rotationKeyAmount = 0;
			positionKeyAmount = 0;
		}

		// Key amount
		fwrite(&rotationKeyAmount, sizeof(int), 1, fp);
		fwrite(&positionKeyAmount, sizeof(int), 1, fp);

		if(rotationKeyAmount == 0)
			continue;

		for(unsigned int j = 0; j < static_cast<unsigned int> (rotationKeyAmount); ++j)
		{
			const AnimationKey &key = boneKeys[i][j];
			FBMatrix tm = key.transform;

			// If has parent, make transform relative
			if(parentIndex >= 0)
			{
				assert(key.time == boneKeys[parentIndex][j].time);
				tm = tm * boneKeys[parentIndex][j].transform.GetInverse();
			}

			// Final values
			Rotation keyRotation = convert(tm.GetRotation());
			Vector keyPosition = convert(tm.GetTranslation());

			// Save
			fwrite(&key.time, 1, sizeof(int), fp); // time
			fwrite(&keyRotation, sizeof(float), 4, fp); // rotation
			if(parentIndex == -1)
				fwrite(&keyPosition, sizeof(float), 3, fp); // position
		}
	}

	fclose(fp);
}

void Exporter::setAnimationLength(int length)
{
	animationLength = length;
}

int Exporter::getFrameLength()
{
	return frameLength;
}

void Exporter::validateData()
{
	info += "\r\n";

	removeRedundantMaterials();
	buildBoneHierarchy();

	// Calculate bone id (prevents mixing wrong types)
	boneId = 0;
	for(unsigned int i = 0; i < bones.size(); ++i)
	{
		// Sum of child index * parent index
		for(unsigned int j = 0; j < i; ++j)
		if(bones[i].getParentName() == bones[j].getName())
			boneId += i*j;
	}

	if(model.getObjects().size() > 1)
	{
		model.buildObjectHierarchy();
		printInfo("Resorted object hierarchy");
	}

	if(model.getHelpers().size() > 1)
	{
		model.buildHelperHierarchy();
		printInfo("Resorted helper hierarchy");
	}

	if(boneId >= 0)
	{
		char number[20] = { 0 };
		_itoa(boneId, number, 10);

		info += "\r\n";
		printInfo("Calculated bone id: " + std::string(number));
	}

	model.setCollisionFlags();
	// !!!!!!!!!!!!!!!!!!
	//if(boneId <= 0)
	//	model.chopObjects();
}

bool BoneAlphaSort(const Bone &lhs, const Bone &rhs)
{
	return (lhs.getName() < rhs.getName());
}

void Exporter::buildBoneHierarchy()
{
	if(bones.size() == 0)
		return;

	// Hierarchy level for each bone (starting from 0)
	std::vector<int> boneLevels(bones.size());
	std::vector<Bone> newBones = bones;
	
	// Parents before their childs
	{
		for(unsigned int k = 0; k < newBones.size(); ++k)
		for(unsigned int i = 1; i < newBones.size(); ++i)
		{
			for(unsigned int j = 0; j < i; ++j)
			{
				if(newBones[i].getName() == newBones[j].getParentName())
				{
					std::swap(newBones[i], newBones[j]);
				}
			}
		}
	}

	// Calculate hierarchy levels
	for(unsigned int i = 0; i < bones.size(); ++i)
	{
		Bone &bone = newBones[i];

		std::string parent = bone.getParentName();
		int level = 0;

		while(!parent.empty())
		{
			// Find parent
			for(unsigned int j = 0; j < i; ++j)
			{
				Bone &b = newBones[j];

				if(newBones[j].getName() == parent)
				{
					++level;
					parent = newBones[j].getParentName();

					break;
				}
			}

			boneLevels[i] = level;
			if(parent == newBones[i].getParentName())
				break;
		} 
	}

	// Sort levels
	for(unsigned int i = 0; i < newBones.size(); ++i) // Max amount of levels
	{
		// Search for level i
		for(unsigned int j = 0; j < newBones.size(); ++j)
		{
			if(boneLevels[j] == static_cast<int> (i))
			{
				// Can we move this?
				for(unsigned int k = 0; k < j; ++k)
				{
					if(boneLevels[k] > static_cast<int> (i))
					{
						std::swap(boneLevels[k], boneLevels[j]);
						std::swap(newBones[k], newBones[j]);

						break;
					}
				}
			}
		}
	}

	unsigned int sortStart = 0;
	unsigned int sortEnd = bones.size();
	int sortLevel = 0;

	// Sort levels alphabetically
	for(;;)
	{
		// Find the range for sorting
		for(unsigned int i = sortStart; i < bones.size(); ++i)
		{
			// This gives us the end
			if(boneLevels[i] != sortLevel)
			{
				sortEnd = i;
				break;
			}
		}

		// End of list
		if(sortStart == sortEnd)
			sortEnd = bones.size();

		// Sort
		std::sort(newBones.begin() + sortStart, newBones.begin() + sortEnd, &BoneAlphaSort);
		
		// All done, exit
		if(sortEnd == bones.size())
			break;

		// Update values
		sortStart = sortEnd;
		++sortLevel;

		assert(sortLevel < int(bones.size()));
	}

	// [old index] =  new index
	std::vector<int> newIndices(bones.size());

	for(unsigned int i = 0; i < bones.size(); ++i) // old index
	for(unsigned int j = 0; j < bones.size(); ++j) // new index
	{
		if(bones[i].getName() == newBones[j].getName())
		{
			newIndices[i] = j;
			break;
		}
	}

	// Correct bone indices from meshes
	std::vector<boost::shared_ptr<Object> > &objects = model.getObjects();
	for(unsigned int j = 0; j < objects.size(); ++j)
		objects[j]->correctBoneIndices(newIndices);

	// Correct bone indices from animation
	std::vector<std::vector<AnimationKey> > newBoneKeys(boneKeys.size());
	for(unsigned int k = 0; k < boneKeys.size(); ++k)
		newBoneKeys[newIndices[k]] = boneKeys[k];

	// Set
	bones = newBones;
	boneKeys = newBoneKeys;

	printInfo("Resorted bone hierarchy");	
}

void Exporter::removeRedundantMaterials()
{
	int foo = model.removeRedundantMaterials();
	int bar = model.collapseMaterials();

	if(foo > 0)
	{
		char number[20] = { 0 };
		_itoa(foo, number, 10);

		printInfo("Removed " + std::string(number) + " redundant material(s)");
	}
	if(bar > 0)
	{
		char number[20] = { 0 };
		_itoa(bar, number, 10);

		printInfo("Removed " + std::string(number) + " collapsed material(s)");
	}
}

} // end of namespace export
} // end of namespace frozenbyte
