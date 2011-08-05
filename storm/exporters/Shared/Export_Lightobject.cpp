// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Lightobject.h"
#include "Export_Material.h"
#include "Export_Object.h"
#include "Export_Object_Chopper.h"
#include <boost/lexical_cast.hpp>

namespace frozenbyte {
namespace exporter {
namespace {

	bool hasAlphaTextures(const Material &material)
	{
		if(!material.getLuminosityTexture().empty())
			return true;

		return false;
	}

	bool equal(float a, float b)
	{
		return fabsf(a - b) < 0.02f;
	}

	bool equalColor(const COL &a, const COL &b)
	{
		if(!equal(a.r, b.r))
			return false;
		if(!equal(a.g, b.g))
			return false;
		if(!equal(a.b, b.b))
			return false;

		return true;
	}

	int getMaterialIndex(const Material &material, LightObjects &lightObject, bool isOuterWall)
	{
		std::vector<Material> &materials = lightObject.materials;
		for(unsigned int i = 0; i < materials.size(); ++i)
		{
			const Material &m = materials[i];
			const Object &o = *lightObject.objects[i];
			if(o.hasBuildingOuterWallFlag() != isOuterWall)
				continue;

			bool alphaTextures = m.hasAlphaTexture() || material.hasAlphaTexture();
			bool sameAlphaTextures = m.hasAlphaTexture() == material.hasAlphaTexture();
			if(alphaTextures)
			{
				if(sameAlphaTextures)
				{
					if(m.getBaseTexture() != material.getBaseTexture())
						continue;
				}
				else
					continue;
			}

			bool alphas = m.hasAlpha() || material.hasAlpha();
			if(alphas)
			{
				if(m.hasAdditiveAlpha() != material.hasAdditiveAlpha())
					continue;

				if(!equal(m.getTransparency(), material.getTransparency()))
					continue;

				if(m.getBaseTexture() != material.getBaseTexture())
					continue;
			}

			if(m.getLuminosityTexture() != material.getLuminosityTexture())
				continue;

			if(!equalColor(m.getDiffuseColor(), material.getDiffuseColor()))
				continue;
			if(!equalColor(m.getLuminosityColor(), material.getLuminosityColor()))
				continue;

			if(m.isDoubleSided() != material.isDoubleSided())
				continue;

			return i;
		}

		return -1;
	}

	Material createNewMaterial(const Material &material, int index)
	{
		std::string name = "Light material";
		name += boost::lexical_cast<std::string> (index);

		Material result(name);
		result.copy(material);

		return result;
	}

	boost::shared_ptr<Object> createNewObject(int index, bool isRoof, bool isOuterWall)
	{
		std::string name = "Light object ";
		if(isOuterWall)
			name += "(BuildingOuterWall)";
		name += boost::lexical_cast<std::string> (index);

		if(isRoof)
			name += "_BuildingRoof";

		boost::shared_ptr<Object> result(new Object(name, ""));
		result->setLightObject();
		result->setProperties(true, false);

		return result;
	}

	int getObjectIndex(const Material &material, LightObjects &lightObject, bool isRoof, bool isOuterWall)
	{
		int index = getMaterialIndex(material, lightObject, isOuterWall);
		if(index == -1)
		{
			index = lightObject.objects.size();
			//if(isRoof)
			//	index = lightObject.roofObjects.size();
		
			//if(isRoof)
			//	lightObject.roofObjects.push_back(createNewObject(index, isRoof));
			//else
			//	lightObject.objects.push_back(createNewObject(index, isRoof));

			lightObject.roofObjects.push_back(createNewObject(index, true, isOuterWall));
			lightObject.objects.push_back(createNewObject(index, false, isOuterWall));

			lightObject.materials.push_back(createNewMaterial(material, index));
		}

		assert(lightObject.roofObjects.size() <= lightObject.materials.size());
		assert(lightObject.objects.size() <= lightObject.materials.size());

		return index;
	}

} // unnamed

void LightObjects::combine()
{
	for(unsigned i = 0; i < objects.size(); )
	{
		Object &object = *objects[i];
		if(!object.getVertexAmount() || !object.getFaceAmount())
		{
			objects.erase(objects.begin() + i);
			continue;
		}

		++i;
	}

	for(unsigned int i = 0; i < roofObjects.size(); ++i)
	{
		boost::shared_ptr<Object> roof = roofObjects[i];
		if(!roof->getVertexAmount() || !roof->getFaceAmount())
			continue;

		//boost::shared_ptr<Object> newObject(new Object(roof));
		//objects.push_back(newObject);
		objects.push_back(roof);
	}

	roofObjects.clear();
}

void insertLightObjects(LightObjects &results, const Object &source, const std::vector<Material> &materials)
{
	//if(source.hasCollisionVisibility())
	//	return;
	if(!source.hasCameraVisibility())
		return;
	if(source.hasDecalFlag())
		return;

	bool isRoof = source.hasBuildingRoofFlag();
	bool isOuterWall = source.hasBuildingOuterWallFlag();

	const std::vector<Face> &faces = source.getFaces();
	const std::vector<Vertex> &vertices = source.getVertices();
	std::vector<std::vector<int> > newVertexIndex;
	std::vector<int> newObjectIndex;

	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		const Face &face = faces[i];

		int materialIndex = face.getMaterialId();
		const Material &material = materials[materialIndex];

		if(materialIndex >= int(newObjectIndex.size()))
			newObjectIndex.resize(materialIndex + 1, -1);

		int objectIndex = newObjectIndex[materialIndex];
		if(objectIndex == -1)
		{
			objectIndex = getObjectIndex(material, results, isRoof, isOuterWall);
			newObjectIndex[materialIndex] = objectIndex;
		}

		Object &object = (isRoof) ? *results.roofObjects[objectIndex] : *results.objects[objectIndex];
		if(objectIndex >= int(newVertexIndex.size()))
			newVertexIndex.resize(objectIndex + 1, std::vector<int> (vertices.size(), -1));

		// Add only needed vertices
		for(int j = 0; j < 3; ++j)
		{
			int index = face.getVertexIndex(j);
			if(newVertexIndex[objectIndex][index] != -1)
				continue;

			newVertexIndex[objectIndex][index] = object.getVertexAmount();

			const Vertex &vertex = vertices[index];
			object.addVertex(vertex);
		}

		// Add face
		Face newFace = face;
		newFace.setMaterialId(objectIndex);

		for(int k = 0; k < 3; ++k)
		{
			int vertexIndex = face.getVertexIndex(k);
			int index = newVertexIndex[objectIndex][vertexIndex];

			assert(vertexIndex >= 0 && vertexIndex <= source.getVertexAmount());
			assert(index >= 0 && index < object.getVertexAmount());

			newFace.setVertexIndex(k, index);
		}

		object.addFace(newFace);
	}
}

void adjustMaterialIndices(LightObjects &result, int baseIndex)
{
	std::vector<boost::shared_ptr<Object> > &objects = result.objects;
	for(unsigned int i = 0; i < objects.size(); ++i)
	{
		Object &o = *objects[i];
		std::vector<Face> &faces = o.getFaces();

		for(unsigned int j = 0; j < faces.size(); ++j)
		{
			int id = faces[j].getMaterialId() + baseIndex;
			faces[j].setMaterialId(id);
		}
	}
}

} // export
} // frozenbyte
