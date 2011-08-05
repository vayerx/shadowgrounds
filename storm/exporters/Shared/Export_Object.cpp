// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Object.h"
#include "Export_Lod.h"
#include <algorithm>
#include <cstdio>
#include <hash_set>

#include "nvtristrip.h"

namespace frozenbyte {
namespace exporter {

Object::Object(const std::string &name_, const std::string &parent_)
:	name(name_),
	parentName(parent_),

	cameraVisibility(true),
	collisionVisibility(true),
	lightObject(false)
{
}

Object::Object()
:	cameraVisibility(true),
	collisionVisibility(true),
	lightObject(false)
{
}

Object::~Object()
{
}

void Object::setStrings(const std::string &name_, const std::string &parent_)
{
	name = name_;
	parentName = parent_;
}

void Object::setLightObject()
{
	lightObject = true;
}

int Object::getVertexAmount() const
{
	return vertices.size();
}

int Object::getFaceAmount() const
{
	return faces.size();
}

FBMatrix Object::getTm() const
{
	//return transform * pivotTransform.GetInverse();
	//return pivotTransform * transform;
	return transform.GetInverse();
}

int Object::getMaterialAmount() const
{
	std::vector<int> materials;
	int materialAmount = 0;

	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		if(std::find(materials.begin(), materials.end(), faces[i].getMaterialId()) == materials.end())
		{
			materials.push_back(faces[i].getMaterialId());
			++materialAmount;
		}
	}

	return materialAmount;
}

const std::string &Object::getName() const
{
	return name;
}

const std::string &Object::getParentName() const
{
	return parentName;
}

const std::vector<Face> &Object::getFaces() const
{
	return faces;
}

const std::vector<Vertex> &Object::getVertices() const
{
	return vertices;
}

std::vector<Face> &Object::getFaces()
{
	return faces;
}

std::vector<Vertex> &Object::getVertices()
{
	return vertices;
}

std::vector<int> Object::getMaterialIndices(bool includeNoMaterial) const
{
	std::vector<int> usedMaterials;

	// Loop all(tm)
	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		if(std::find(usedMaterials.begin(), usedMaterials.end(), faces[i].getMaterialId()) == usedMaterials.end())
		{
			int id = faces[i].getMaterialId();
			{
				if((includeNoMaterial == true) && (id == -1))
					continue;
			
				usedMaterials.push_back(id);
			}
		}
	}

	return usedMaterials;
}

const FBMatrix &Object::getTransform() const
{
	return transform;
}

const FBMatrix &Object::getPivotTransform() const
{
	return pivotTransform;
}

void Object::setTransform(const FBMatrix &tm)
{
	transform = tm;
}

void Object::setPivotTransform(const FBMatrix &tm)
{
	pivotTransform = tm;
}

void Object::addVertex(const Vertex &vertex)
{
	vertices.push_back(vertex);
}

void Object::addFace(const Face &face)
{
	faces.push_back(face);
}

void Object::setProperties(bool cameraVisibility_, bool collisionVisibility_)
{
	cameraVisibility = cameraVisibility_;
	collisionVisibility = collisionVisibility_;
}

bool Object::hasCollisionFlag() const
{
	if(hasNoCollisionFlag())
		return false;

	std::string collisionString("Collision");

	if(name.size() > collisionString.size())
	for(unsigned int i = 0; i < name.size() - collisionString.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString.size());
		if(chunk == collisionString)
			return true;
	}

	return false;
}

bool Object::hasNoCollisionFlag() const
{
	std::string collisionString("NoCollision");

	if(name.size() > collisionString.size())
	for(unsigned int i = 0; i < name.size() - collisionString.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString.size());
		if(chunk == collisionString)
			return true;
	}

	return false;
}

bool Object::hasNoVisibilityFlag() const
{
	std::string collisionString1("BuildingFloor");
	std::string collisionString2("NoVisibility");

	if(name.size() > collisionString1.size())
	for(unsigned int i = 0; i < name.size() - collisionString1.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString1.size());
		if(chunk == collisionString1)
			return true;
	}

	if(name.size() > collisionString2.size())
	for(unsigned int i = 0; i < name.size() - collisionString2.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString2.size());
		if(chunk == collisionString2)
			return true;
	}

	return false;
}

bool Object::hasBuildingRoofFlag() const
{
	std::string collisionString("BuildingRoof");

	if(name.size() > collisionString.size())
	for(unsigned int i = 0; i < name.size() - collisionString.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString.size());
		if(chunk == collisionString)
			return true;
	}

	return false;
}

bool Object::hasBuildingOuterWallFlag() const
{
	std::string collisionString("BuildingOuterWall");

	if(name.size() > collisionString.size())
	for(unsigned int i = 0; i < name.size() - collisionString.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString.size());
		if(chunk == collisionString)
			return true;
	}

	return false;
}

bool Object::hasDecalFlag() const
{
	std::string collisionString("Decal");

	if(name.size() > collisionString.size())
	for(unsigned int i = 0; i < name.size() - collisionString.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString.size());
		if(chunk == collisionString)
			return true;
	}

	return false;
}

bool Object::hasEditorOnlyFlag() const
{
	std::string collisionString("EditorOnly");

	if(name.size() > collisionString.size())
	for(unsigned int i = 0; i < name.size() - collisionString.size() + 1; ++i)
	{
		std::string chunk = name.substr(i, collisionString.size());
		if(chunk == collisionString)
			return true;
	}

	return false;
}

bool Object::hasCameraVisibility() const
{
	return cameraVisibility;
}

bool Object::hasCollisionVisibility() const
{
	return collisionVisibility;
}

bool Object::isLightObject() const
{
	return lightObject;
}

void Object::correctBoneIndices(const std::vector<int> &newIndices)
{
	for(unsigned int i = 0; i < vertices.size(); ++i)
		vertices[i].correctBoneIndices(newIndices);
}

void Object::correctMaterialIndices(const std::vector<int> &newIndices)
{
	for(unsigned int i = 0; i < faces.size(); ++i)
		faces[i].correctMaterialIndices(newIndices);
}

void Object::remapIndices()
{
/*
	enum PrimType
	{
		PT_LIST,
		PT_STRIP,
		PT_FAN
	};

	struct PrimitiveGroup
	{
		PrimType type;
		unsigned int numIndices;
		unsigned short* indices;

	////////////////////////////////////////////////////////////////////////////////////////

		PrimitiveGroup() : type(PT_STRIP), numIndices(0), indices(NULL) {}
		~PrimitiveGroup()
		{
			if(indices)
				delete[] indices;
			indices = NULL;
		}
	};
*/
	//void RemapIndices(const PrimitiveGroup* in_primGroups, const unsigned short numGroups, const unsigned short numVerts, PrimitiveGroup** remappedGroups);
/*	
	//unsigned int *indices = new unsigned int[faces.size() * 3];
	PrimitiveGroup oldPrimitives;

	// Fill old stuff
	oldPrimitives.type = PT_LIST;
	oldPrimitives.numIndices = faces[0].size() * 3;
	oldPrimitives.indices = new unsigned short[faces[0].size() * 3];

	for(unsigned int i = 0; i < faces[0].size(); ++i)
	for(unsigned int j = 0; j < 3; ++j)
		oldPrimitives.indices[i*3 + j] = static_cast<unsigned short> (faces[0][i].getVertexIndex(j));

	// Nvidia remap 
	PrimitiveGroup *newPrimitives = 0;
	RemapIndices(&oldPrimitives, 1, static_cast<unsigned short> (vertices.size()), &newPrimitives);

	// Remap our stuff
	std::vector<Vertex> newVertices;
	newVertices.reserve(vertices.size());

	int *indexCache = new int[vertices.size()];
	for(i = 0; i < vertices.size(); ++i)
		indexCache[i] = -1;

	// What's happening? ;-)
	for(i = 0; i < faces[0].size(); ++i)
	for(unsigned int j = 0; j < 3; ++j)
	{
		int vertexIndex = faces[0][i].getVertexIndex(j);
		int cacheIndex = indexCache[vertexIndex];
		if(cacheIndex == -1)
		{
			newVertices.push_back(vertices[vertexIndex]);
			faces[0][i].setVertexIndex(j, newVertices.size() - 1);
			indexCache[vertexIndex] = newVertices.size() - 1;
		}
		else
			faces[0][i].setVertexIndex(j, cacheIndex);
	}

	vertices = newVertices;
	delete[] newPrimitives;
	delete[] indexCache;

	// Now, generate new face indices
	{
		unsigned short *oldIndices = new unsigned short[faces[0].size() * 3];
		for(unsigned int i = 0; i < faces[0].size(); ++i)
		for(int j = 0; j < 3; ++j)
			oldIndices[i *  3 + j] = faces[0][i].getVertexIndex(j);

		PrimitiveGroup *primitiveGroup = 0;
		unsigned short groupAmount = 0;

		SetCacheSize(CACHESIZE_GEFORCE3);
		SetListsOnly(true);
		
		GenerateStrips(oldIndices, faces[0].size() * 3, &primitiveGroup, &groupAmount);

		for(i = 0; i < faces[0].size(); ++i)
		for(int j = 0; j < 3; ++j)
			faces[0][i].setVertexIndex(j, primitiveGroup->indices[i * 3 + j]);

		delete[] oldIndices;
		delete[] primitiveGroup;
	}
*/
}

extern int actualObjectAmount;

void Object::writeToFile(FILE *fp, const FBMatrix &tm, const FBMatrix &pivotTm, const std::vector<int> &usedMaterials, bool optimizeVcache, bool factorPivot) const
{
	std::vector<int> materials = getMaterialIndices();

	/*
	// Common data
	FBMatrix relativeTransform = transform;// * parentTm.GetInverse();
	Vector position = convert(relativeTransform.GetTranslation());
	Rotation rotation = convert(relativeTransform.GetRotation());
	Vector scale = convert(relativeTransform.GetScale());
	FBMatrix inversePivot = pivotTm; //pivotTm.GetInverse();
	*/

	Vector position = convert(tm.GetTranslation());
	Rotation rotation = convert(tm.GetRotation());
	Vector scale = convert(tm.GetScale());

	for(unsigned int i = 0; i < materials.size(); ++i)
	{
		++actualObjectAmount;

		// ToDo: generate some fancy names for splitted objects
		fwrite(name.c_str(), 1, name.size() + 1, fp);
		fwrite(parentName.c_str(), 1, parentName.size() + 1, fp);

		short int materialIndex = -1;
		for(unsigned int j = 0; j < usedMaterials.size(); ++j)
		{
			if(usedMaterials[j] == materials[i])
			{
				materialIndex = static_cast<short int> (j);
				break;
			}
		}
		
		fwrite(&materialIndex, sizeof(short int), 1, fp); // Material index
		fwrite(&position, sizeof(float), 3, fp); // position
		fwrite(&rotation, sizeof(float), 4, fp); // rotation
		fwrite(&scale, sizeof(float), 3, fp); // scale

		char noCollision = 0;
		char noRender = 0;
		char isLightObject = 0;

		if(!cameraVisibility)
			noRender = 1;
		if(!collisionVisibility)
			noCollision = 1;
		if(lightObject)
			isLightObject = 1;

		fwrite(&noCollision, sizeof(char), 1, fp); // no_collision
		fwrite(&noRender, sizeof(char), 1, fp); // no_render
		fwrite(&isLightObject, sizeof(char), 1, fp); // lightobject

		// For this material
		std::vector<Face> tempFaces;
		std::vector<Vertex> tempVertices;
		char hasWeights = 0;

		// Build faces
		for(unsigned int j = 0; j < faces.size(); ++j)
		{
			if(faces[j].getMaterialId() == materials[i])
				tempFaces.push_back(faces[j]);
		}

		std::vector<int> newVertexIndex(vertices.size());
		std::hash_set<int> tempVertexIndices;

		for(unsigned int j = 0; j < tempFaces.size(); ++j)
		for(unsigned int k = 0; k < 3; ++k)
		{
			int index = tempFaces[j].getVertexIndex(k);
			assert(index >= 0 && index < int(vertices.size()));

			if(tempVertexIndices.find(index) == tempVertexIndices.end())
			{
				tempVertexIndices.insert(index);

				if(vertices[index].hasBoneWeights() == true)
					hasWeights = 1;

				newVertexIndex[index] = tempVertices.size();
				tempVertices.push_back(vertices[index]);
			}
		}

		for(unsigned int j = 0; j < tempFaces.size(); ++j)
		for(unsigned int k = 0; k < 3; ++k)
		{
			// Grab index and convert it to temp mesh vertex buffer
			int vertexIndex = tempFaces[j].getVertexIndex(k);
			int newIndex = newVertexIndex[vertexIndex];

			assert(newIndex >= 0 && newIndex < int(tempVertices.size()));
			tempFaces[j].setVertexIndex(k, newIndex);
		}

		Lod lod(tempVertices, tempFaces, optimizeVcache);
		//if(generateLods)
		//	lod.generateLods(lodDetail);

		const std::vector<Vertex> &lodVertices = lod.getVertices();
		const std::vector<Face> &lodFaces = lod.getFaceBuffer(0);

		int vertexAmountInt = lodVertices.size();
		WORD vertexAmount = static_cast<WORD> (lodVertices.size());
		WORD faceAmount = static_cast<WORD> (lodFaces.size());

		char hasLods = 0; //generateLods;

		fwrite(&vertexAmount, sizeof(WORD), 1, fp); // vertex count
		fwrite(&faceAmount, sizeof(WORD), 1, fp); // face count
		fwrite(&hasLods, sizeof(char), 1, fp); // lod info
		fwrite(&hasWeights, sizeof(char), 1, fp); // vertex weights

		// Vertex data
		for(int j = 0; j < vertexAmount; ++j)
		{
			FBVector positionTmp = lodVertices[j].getPosition();
			Vector normal = convert(lodVertices[j].getNormal());
			Vector2D uv = convert(lodVertices[j].getUv());
			Vector2D uv2 = convert(lodVertices[j].getUv2());

			if(factorPivot)
				pivotTm.TransformVector(positionTmp);

			Vector position = convert(positionTmp);

			fwrite(&position, sizeof(float), 3, fp); // position
			fwrite(&normal, sizeof(float), 3, fp); // normal
			fwrite(&uv, sizeof(float), 2, fp); // texture coordinates
			fwrite(&uv2, sizeof(float), 2, fp); // texture coordinates
		}

		// Face data
		for(int j = 0; int(j) < lod.getFaceBufferCount(); ++j)
		{
			const std::vector<Face> &lodFaces = lod.getFaceBuffer(j);
			WORD faceCount = lodFaces.size();

			if(j > 0)
			{
				fwrite(&faceCount, sizeof(WORD), 1, fp);
			}

			for(unsigned int i = 0; i < faceAmount; ++i)
			for(unsigned int k = 0; k < 3; ++k)
			{
				const Face &face = lodFaces[i];
				WORD vertexIndex = WORD(face.getVertexIndex(k));

				int a = face.getVertexIndex(k);
				int b = int(lodVertices.size());
				assert(face.getVertexIndex(k) < int(lodVertices.size()));
				assert(vertexIndex >= 0 && vertexIndex < vertexAmount);

				WORD index = vertexIndex;
				fwrite(&index, sizeof(WORD), 1, fp);
			}
		}

		// Weight data (if has)
		if(hasWeights == 1)
		{
			static const int MAX_WEIGHTS = 2;

			for(unsigned int j = 0; j < lodVertices.size(); ++j)
			{
				int indices[MAX_WEIGHTS] = { -1 };
				signed char weights[MAX_WEIGHTS] = { 0 };

				for(int k = 0; k < 2; ++k)
				{
					indices[k] = lodVertices[j].getBoneIndex(k);

					float weight = lodVertices[j].getBoneWeight(k);
					weights[k] = static_cast<signed char> (100.f * weight + .5f);
				}

				// Indices
				for(int k = 0; k < 2; ++k)
					fwrite(&indices[k], sizeof(int), 1, fp);

				// Weights
				for(int k = 0; k < 2; ++k)
					fwrite(&weights[k], sizeof(signed char), 1, fp);
			}
		}
	}
}

bool operator < (const Object &lhs, const Object &rhs)
{
	if(lhs.getName() == rhs.getParentName())
		return false;

	return true;
}	

} // end of namespace export
} // end of namespace frozenbyte
