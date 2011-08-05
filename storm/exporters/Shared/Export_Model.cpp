// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Model.h"
#include "Export_Object_Chopper.h"
#include "Export_Lightobject.h"

#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <cstdio>
#include <windows.h>

namespace frozenbyte {
namespace exporter {
namespace {
	std::string getString(const Color &color)
	{
		std::string result = "(";
		result += boost::lexical_cast<std::string> (color.r);
		result += ", ";
		result += boost::lexical_cast<std::string> (color.g);
		result += ", ";
		result += boost::lexical_cast<std::string> (color.b);
		result += ")";

		return result;
	}

	std::string getString(const VC2 &v)
	{
		std::string result = "(";
		result += boost::lexical_cast<std::string> (v.x);
		result += ", ";
		result += boost::lexical_cast<std::string> (v.y);
		result += ")";

		return result;
	}

	std::string getString(float value)
	{
		std::string result = "(";
		result += boost::lexical_cast<std::string> (value);
		result += ")";

		return result;
	}
}

Model::Model()
{
}

Model::~Model()
{
}

std::string Model::getSummary() const
{
	std::string result;

	std::vector<std::string> objectNames;
	for(unsigned int j = 0; j < objects.size(); ++j)
		objectNames.push_back(objects[j]->getName());

	int objectAmount = objects.size();
	int faceAmount = 0;
	int vertexAmount = 0;
	int materialAmount = materials.size();
	int textureAmount = getTextures(objectNames, objects).size();

	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		faceAmount += objects[i]->getFaceAmount();
		vertexAmount += objects[i]->getVertexAmount();
	}

	char number[20] = { 0 };

	_itoa(objectAmount, number, 10);
	result += "Number of objects: " + std::string(number) + std::string("\r\n");
	_itoa(faceAmount, number, 10);
	result += "\tTotal face amount: " + std::string(number) + std::string("\r\n");
	_itoa(vertexAmount, number, 10);
	result += "\tTotal vertex amount " + std::string(number) + std::string("\r\n");
	_itoa(materialAmount, number, 10);
	result += "Number of materials: " + std::string(number) + std::string("\r\n");
	_itoa(textureAmount, number, 10);
	result += "\tNumber of textures: " + std::string(number) + std::string("\r\n");	
	
	return result;
}

std::string Model::getDetails() const
{
	std::string result;
	
	char number[20] = { 0 };
	_itoa(objects.size(), number, 10);
	std::string objectAmount = number;

	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		_itoa(i+1, number, 10);
		result += "Object (" + std::string(number) + "/" + objectAmount + ")\r\n";
		result += "\tName: " + objects[i]->getName() + "\r\n";
		result += "\tParent: " + objects[i]->getParentName() + "\r\n";
		
		_itoa(objects[i]->getFaceAmount(), number, 10);
		result += "\tFace amount: " + std::string(number) + "\r\n";
		_itoa(objects[i]->getVertexAmount(), number, 10);
		result += "\tVertex amount: " + std::string(number) + "\r\n";

		bool visibility = objects[i]->hasCameraVisibility();
		result += "\tRenderable: " + boost::lexical_cast<std::string> (visibility) + "\r\n";
		bool collisions = objects[i]->hasCollisionVisibility();
		result += "\tCollisions: " + boost::lexical_cast<std::string> (collisions) + "\r\n";
	}

	result += "\r\n";
	_itoa(materials.size(), number, 10);
	std::string materialAmount = number;

	for(unsigned int j = 0; j < materials.size(); ++j)
	{
		_itoa(j+1, number, 10);
		result += "Material (" + std::string(number) + "/" + materialAmount + ")\r\n";
		result += "\tName: " + materials[j].getName() + "\r\n";
		result += "\tBase texture: " + materials[j].getBaseTexture() + "\r\n";
		result += "\tLight map: " + materials[j].getLuminosityTexture() + "\r\n";
		//result += "\tBump texture: " + materials[j].getBumpTexture() + "\r\n";
		result += "\tReflection texture: " + materials[j].getReflectionTexture() + " : ";
		result += getString(materials[j].getReflectionFactor()) + "\r\n";
		result += "\tDistortion texture: " + materials[j].getDistortionTexture() + "\r\n";

		const Color &diffuseColor = materials[j].getDiffuseColor();
		const Color &luminosityColor = materials[j].getLuminosityColor();
		float transparency = materials[j].getTransparency();
		float glow = materials[j].getGlow();
		const VC2 &scrollSpeed = materials[j].getScrollSpeed();
		bool scrollAutoStart = materials[j].scrollAutoStartEnabled();
		bool alphaTest = materials[j].hasAlphaTexture();

		result += "\tAlpha test " + getString(alphaTest) + "\r\n";
		result += "\tDiffuse color " + getString(diffuseColor) + "\r\n";
		result += "\tSelf illumination " + getString(luminosityColor) + "\r\n";
		result += "\tScroll speed " + getString(scrollSpeed) + "\r\n";
		result += "\tScroll autostart " + getString(scrollAutoStart) + "\r\n";
		result += "\tGlow " + getString(glow) + "\r\n";
		result += "\tTransparency " + getString(transparency) + "\r\n";
	}

	return result;
}

const std::vector<boost::shared_ptr<Object> > &Model::getObjects() const
{
	return objects;
}

const std::vector<Helper> &Model::getHelpers() const
{
	return helpers;
}

std::vector<boost::shared_ptr<Object> > &Model::getObjects()
{
	return objects;
}

int actualObjectAmount = 0;

void Model::saveToFile(const std::string &fileName, std::vector<std::string> objectNames, int boneId, bool copyTextures, int transformType, bool chop, bool optimizeVcache, bool hasBones) const
{
	FILE *fp = fopen(fileName.c_str(), "wb");
	if(fp == 0)
		return;

	WORD objectAmount = 0;
	std::vector<std::vector<boost::shared_ptr<Object> > > saveObjects(objects.size());
	std::vector<boost::shared_ptr<Object> > objectList = objects;

	LightObjects lightObjects;
	LightObjects roofObjects;

	for(unsigned int i = 0; i < objectNames.size(); ++i)
	{
		int index = 0;
		for(unsigned int j = 0; j < objects.size(); ++j)
		{
			if(objects[j]->getName() == objectNames[i])
			{
				index = j;
				break;
			}
		}

		const Object &object = *objects[index].get();
		boost::shared_ptr<Object> newObject(new Object(object));
		saveObjects[index].push_back(newObject);

		if(chop)
		{
			//insertLightObjects(lightObjects, object, materials);
			::frozenbyte::exporter::chopObjects(saveObjects[index]);
		}

		//for(j = 0; j < saveObjects[index].size(); ++j)
		//	objectAmount += saveObjects[index][j]->getMaterialIndices().size();
	}
	
	lightObjects.combine();

	if(chop)
	{
		adjustMaterialIndices(lightObjects, materials.size());
		::frozenbyte::exporter::chopObjects(lightObjects.objects);

		saveObjects.resize(objects.size() + lightObjects.objects.size());
		//objectAmount += lightObjects.objects.size();

		for(unsigned int i = 0; i < lightObjects.objects.size(); ++i)
		{
			Object &o = *lightObjects.objects[i];
			objectNames.push_back(o.getName());

			boost::shared_ptr<Object> newObject1(new Object(o));
			//boost::shared_ptr<Object> newObject1(new Object(o.getName(), o.getParentName()));
			objectList.push_back(newObject1);
	
			boost::shared_ptr<Object> newObject2(new Object(o));
			saveObjects[objects.size() + i].push_back(newObject2);
			o = Object();
		}
	}

	// Meet limits
	{
		for(unsigned int i = 0; i < saveObjects.size(); ++i)
			::frozenbyte::exporter::chopObjectToLimits(saveObjects[i]);
	}

	{
		for(unsigned int i = 0; i < saveObjects.size(); ++i)
		{
			for(unsigned int j = 0; j < saveObjects[i].size(); ++j)
				objectAmount += saveObjects[i][j]->getMaterialIndices().size();
		}
	}

	std::vector<std::string> textures = getTextures(objectNames, objectList, lightObjects.materials);
	std::vector<int> usedMaterials;
	getMaterials(usedMaterials, objectNames, objectList);

	// Saves as version x format
	int version = 14;
	fwrite("S3D0", 1, 4, fp);
	fwrite(&version, 1, sizeof(int), fp);

	WORD textureAmount = static_cast<WORD> (textures.size());
	fwrite(&textureAmount, 1, sizeof(WORD), fp); // Texture amount
	WORD materialAmount = static_cast<WORD> (usedMaterials.size());
	fwrite(&materialAmount, 1, sizeof(WORD), fp); // Material amount

	fwrite(&objectAmount, 1, sizeof(WORD), fp); // Object amount
	WORD lightAmount = 0;
	fwrite(&lightAmount, 1, sizeof(WORD), fp); // Light amount
	WORD helperAmount = static_cast<WORD> (helpers.size());
	fwrite(&helperAmount, 1, sizeof(WORD), fp); // Helper amount
	fwrite(&boneId, 1, sizeof(int), fp); // Bone id

	// Write textures
	std::vector<std::string> saveTextures = textures;
	for(unsigned int i = 0; i < usedMaterials.size(); ++i)
	{
		int index = usedMaterials[i];

		if(index < int(materials.size()))
			materials[index].writeTexturesToFile(fp, &saveTextures);
		else
			lightObjects.materials[index - materials.size()].writeTexturesToFile(fp, &saveTextures);
	}

	// Write materials
	for(unsigned int i = 0; i < usedMaterials.size(); ++i)
	{
		int index = usedMaterials[i];
		if(index < int(materials.size()))
			materials[index].writeToFile(fp, textures);
		else
			lightObjects.materials[index - materials.size()].writeToFile(fp, textures);
	}

	std::vector<int> savedIndices;

	// Write objects
	for(unsigned int i = 0; i < objectNames.size(); ++i)
	{
		int index = -1;
		for(unsigned int j = 0; j < objectList.size(); ++j)
		{
			if(objectList[j]->getName() == objectNames[i])
			{
				index = j;
				break;
			}
		}

		const std::string &name = objectNames[i];
		assert(std::find(savedIndices.begin(), savedIndices.end(), index) == savedIndices.end());
		savedIndices.push_back(index);
		assert(index >= 0 && index < int(objectList.size()));

		bool found = false;
		/*
		// We do not support object hierarchies!
		for(j = 0; j < i; ++j)
		{
			if(objectList[index]->getParentName() == objectList[j]->getName())
			{
				for(unsigned int k = 0; k < saveObjects[index].size(); ++k)
					saveObjects[index][k]->writeToFile(fp, objectList[j]->getTransform(), usedMaterials, optimizeVcache, true);
				
				found = true;
				break;
			}
		}
		*/

		if(!found)
		{
			FBMatrix tm;
			FBMatrix pivotTm;

			//bool factorPivot = false;
			//if(transformType == 0 || transformType == 1)
			//	factorPivot = true;
			bool factorPivot = true;

			FBMatrix w = objectList[index]->getTransform();
			FBMatrix p = objectList[index]->getPivotTransform();

			if(transformType == 0)
			{
				pivotTm = w.GetInverse();
			}
			else if(transformType == 1)
			{
				//tm = w;
				//pivotTm = p;
				//tm = w * p;

				tm = p;
				pivotTm = w.GetInverse();
			}
			else
			{
				//pivotTm = w * p;
				pivotTm = p * w.GetInverse();
			}

			if(hasBones)
			{
				tm = FBMatrix();
				pivotTm = FBMatrix();
			}

			for(unsigned int k = 0; k < saveObjects[index].size(); ++k)
				saveObjects[index][k]->writeToFile(fp, tm, pivotTm, usedMaterials, optimizeVcache, factorPivot);
		}
	}

	int foofoo = actualObjectAmount;
	//assert(foofoo == objectAmount);

	// Write helpers
	for(unsigned int i = 0; i < helpers.size(); ++i)
		helpers[i].writeToFile(fp);

	// Copy texture
	if(copyTextures == true)
	{
		std::string directory;

		for(int i = fileName.size() - 1; i > 0; --i)
		{
			if(fileName[i] == '\\')
			{
				directory = fileName.substr(0, i);
				break;
			}
		}

		for(unsigned int i = 0; i < textures.size(); ++i)
		{
			for(unsigned int j = textures[i].size() - 1; j > 0; --j)
			{
				if(textures[i][j] == '\\')
				{
					std::string foo = directory;
					foo += '\\';
					foo += textures[i].substr(j + 1, textures[i].size() - 1);
					
					CopyFile(textures[i].c_str(), foo.c_str(), FALSE);
					break;
				}
			}
		}
	}

	fclose(fp);
}

int Model::addObject(const boost::shared_ptr<Object> &object)
{
	int index = objects.size();
	bool hasSameName = false;

	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		if(object->getName() == objects[i]->getName())
		{
			hasSameName = true;
			break;
		}
	}

	if(hasSameName)
	{
		std::string name = object->getName();
		for(int i = 0; i < index; ++i)
			name += "*";

		object->setStrings(name, object->getParentName());
	}

	objects.push_back(object);

	return index;
}

int Model::addMaterial(const Material &material)
{
	materials.push_back(material);
	return materials.size() - 1;
}

int Model::addHelper(const Helper &helper)
{
	helpers.push_back(helper);
	return helpers.size() - 1;
}

void Model::buildObjectHierarchy()
{
	std::stable_sort(objects.begin(), objects.end());
	//std::sort(objects.begin(), objects.end() - 7);
	//std::sort(objects.begin() + 19, objects.end());
}

void Model::buildHelperHierarchy()
{
	std::stable_sort(helpers.begin(), helpers.end());
}

int Model::removeRedundantMaterials()
{
	std::vector<std::string> objectNames;
	for(unsigned int j = 0; j < objects.size(); ++j)
		objectNames.push_back(objects[j]->getName());

	// Contains used indices
	std::vector<int> usedMaterials;
	getMaterials(usedMaterials, objectNames, objects);

	// All in use
	if(usedMaterials.size() == materials.size())
		return 0;

	std::sort(usedMaterials.begin(), usedMaterials.end());

	// New materials
	std::vector<Material> newMaterials;
	// New indices ([oldIndex] = newIndex)
	std::vector<int> newIndices(materials.size());

	for(unsigned int i = 0; i < usedMaterials.size(); ++i)
	{
		int oldIndex = usedMaterials[i];
		
		newMaterials.push_back(materials[oldIndex]);
		newIndices[oldIndex] = i;
	}

	// Corrent indices from meshes
	for(unsigned int i = 0; i < objects.size(); ++i)
		objects[i]->correctMaterialIndices(newIndices);

	materials = newMaterials;
	return newIndices.size() - materials.size();
}

int Model::collapseMaterials()
{
	std::vector<std::vector<int> > duplicates;
	duplicates.resize(materials.size());

	// Find duplicates
	for(unsigned int i = 1; i < materials.size(); ++i)
	{
		for(unsigned int j = 0; j < i; ++j)
			if(materials[i] == materials[j])
			{
				duplicates[j].push_back(i);
				break;
			}
	}

	std::vector<int> removedMaterials;
	for(unsigned int j = 0; j < duplicates.size(); ++j)
	{
		for(unsigned int k = 0; k < duplicates[j].size(); ++k)
			removedMaterials.push_back(duplicates[j][k]);
	}

	std::sort(removedMaterials.begin(), removedMaterials.end());

	{
		std::vector<Material> newMaterials;
		std::vector<int> newIndices(materials.size());

		// Remove references to duplicates
		for(unsigned int i = 0; i < materials.size(); ++i)
		{
			newIndices[i] = i;

			for(unsigned int j = 0; j < i; ++j)
			{
				std::vector<int> &duplicate = duplicates[j];
				bool found = false;

				for(unsigned int k = 0; k < duplicate.size(); ++k)
				{
					if(duplicate[k] == int(i))
					{
						newIndices[i] = j;
						found = true;
						break;
					}
				}

				if(found)
					break;
			}
		}

		// Remove duplicates
		for(unsigned int j = 0; j < materials.size(); ++j)
		{
			int removedIndices = j - newMaterials.size();

			if(!std::binary_search(removedMaterials.begin(), removedMaterials.end(), j))
				newMaterials.push_back(materials[j]);

			for(unsigned int k = 0; k < newIndices.size(); ++k)
			{
				if(newIndices[k] == int(j))
				{
					newIndices[k] -= removedIndices;
					assert(newIndices[k] < int(newMaterials.size()));
				}
			}
		}

		assert(newMaterials.size() == materials.size() - removedMaterials.size());

		// Corrent indices from meshes
		for(unsigned int i = 0; i < objects.size(); ++i)
			objects[i]->correctMaterialIndices(newIndices);

		materials = newMaterials;
	}

	return removedMaterials.size();
}

void Model::chopObjects()
{
	::frozenbyte::exporter::chopObjects(objects);
}

void Model::setCollisionFlags()
{
	bool hasCollisionObjects = false;
	for(unsigned int i = 0; i < objects.size(); ++i)
		if(objects[i]->hasCollisionFlag())
			hasCollisionObjects = true;

	// setProperties(cameraVis, collisionVis)

	if(hasCollisionObjects)
	{
		for(unsigned int i = 0; i < objects.size(); ++i)
		{
			if(objects[i]->hasCollisionFlag())
			{
				//noCollision = 0;
				//noRender = 1;
				objects[i]->setProperties(false, true);
			}
			else
			{
				//noCollision = 1;
				//noRender = 0;
				objects[i]->setProperties(true, false);
			}
		}
	}
	else
	{
		for(unsigned int i = 0; i < objects.size(); ++i)
		{
			if(objects[i]->hasNoCollisionFlag())
			{
				//noCollision = 1;
				//noRender = 0;
				objects[i]->setProperties(true, false);
			}
			else
			{
				//noCollision = 0;
				//noRender = 0;
				objects[i]->setProperties(true, true);
			}
		}
	}

	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		Object &o = *objects[i];
		bool collision = o.hasCollisionVisibility();
		bool visibility = o.hasCameraVisibility();

		if(o.hasNoVisibilityFlag())
			visibility = false;
		if(o.hasEditorOnlyFlag())
			visibility = false;

		o.setProperties(visibility, collision);
	}
}

void Model::remapIndices()
{
	for(unsigned int i = 0; i < objects.size(); ++i)
		objects[i]->remapIndices();
}

void Model::removeJunctions()
{
//	::frozenbyte::exporter::snapVertices(objects);
	::frozenbyte::exporter::removeJunctions(objects);

	for(unsigned int i = 0; i < objects.size(); ++i)
		chopFaces(*objects[i]);
}

/*
std::vector<int> Model::getMaterials(const std::vector<std::string> &objectNames) const
{
	std::vector<int> usedMaterials;

	// Loop all(tm)
	for(unsigned int i = 0; i < objectNames.size(); ++i)
	{
		int index = -1;
		for(unsigned int j = 0; j < objects.size(); ++j)
		{
			if(objects[j].getName() == objectNames[i])
				index = j;
		}

		const std::vector<Face> &faces = objects[index].getFaces();
		
		for(j = 0; j < faces.size(); ++j)
		{
			if(std::find(usedMaterials.begin(), usedMaterials.end(), faces[j].getMaterialId()) == usedMaterials.end())
			{
				int id = faces[j].getMaterialId();
				if( id != -1)
					usedMaterials.push_back(id);
			}
		}
	}

	return usedMaterials;
}
*/
void Model::getMaterials(std::vector<int> &result, const std::vector<std::string> &objectNames, const std::vector<boost::shared_ptr<Object> > &objects) const
{
	// Loop all(tm)
	for(unsigned int i = 0; i < objectNames.size(); ++i)
	{
		int index = -1;
		for(unsigned int j = 0; j < objects.size(); ++j)
		{
			if(objects[j]->getName() == objectNames[i])
				index = j;
		}

		const std::vector<Face> &faces = objects[index]->getFaces();
		
		for(unsigned int j = 0; j < faces.size(); ++j)
		{
			if(std::find(result.begin(), result.end(), faces[j].getMaterialId()) == result.end())
			{
				int id = faces[j].getMaterialId();
				if( id != -1)
					result.push_back(id);
			}
		}
	}
}

std::vector<std::string> Model::getTextures(const std::vector<std::string> &objectNames, const std::vector<boost::shared_ptr<Object> > &objects, const std::vector<Material> &lightMaterials) const
{
	std::vector<int> usedMaterials;
	getMaterials(usedMaterials, objectNames, objects);

	std::vector<std::string> textures;
	std::string texture;

	for(unsigned int j = 0; j < usedMaterials.size(); ++j)
	{
		int index = usedMaterials[j];
		int materialAmount = materials.size();
		int lightMaterialAmount = lightMaterials.size();

		const Material &material = (index < int(materials.size())) ? materials[index] : lightMaterials[index - materials.size()];

		texture = material.getBaseTexture();
		if(std::find(textures.begin(), textures.end(), texture) == textures.end())
		{
			if(texture.size() > 0)
				textures.push_back(texture);
		}
		texture = material.getBumpTexture();
		if(std::find(textures.begin(), textures.end(), texture) == textures.end())
		{
			if(texture.size() > 0)
				textures.push_back(texture);
		}
		texture = material.getLuminosityTexture();
		if(std::find(textures.begin(), textures.end(), texture) == textures.end())
		{
			if(texture.size() > 0)
				textures.push_back(texture);
		}
		texture = material.getReflectionTexture();
		if(std::find(textures.begin(), textures.end(), texture) == textures.end())
		{
			if(texture.size() > 0)
				textures.push_back(texture);
		}
		texture = material.getDistortionTexture();
		if(std::find(textures.begin(), textures.end(), texture) == textures.end())
		{
			if(texture.size() > 0)
				textures.push_back(texture);
		}
	}

	return textures;
}


} // end of namespace export
} // end of namespace frozenbyte
