// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Object.h"
#include "LWExport_Manager.h"
#include "LWExport_Transform.h"

#include "..\Shared\Export_Exporter.h"
#include "..\Shared\Export_Vertex.h"
#include "..\Shared\Export_Face.h"
#include "..\Shared\Export_Object_Chopper.h"

#include <map>

//#define NEW_SDK

namespace frozenbyte {
namespace exporter {

#ifdef _MSC_VER
#pragma warning(disable: 4512) // Assignment operator could no be generated
#endif

namespace {

	int findMaterialIndex(const std::vector<LWMaterial> &materials, const std::string &name)
	{
		if(materials.empty())
			return -1;

		int lowLimit = 0;
		int highLimit = materials.size();
		int pos = highLimit / 2;

		for(;;)
		{
			const LWMaterial &cur = materials[pos];
			if(cur.getName() == name)
				break;
			else if(name < cur.getName())
			{
				if(pos <= lowLimit)
				{
					assert(!"whoops");
					return -1;
				}

				highLimit = pos;
				int delta = (pos - lowLimit) / 2;
				if(!delta)
					delta = 1;

				pos -= delta;
			}
			else if(name > cur.getName())
			{
				if(pos >= highLimit - 1)
				{
					assert(!"whoops");
					return -1;
				}

				lowLimit = pos;
				int delta = (highLimit - pos) / 2;
				if(!delta)
					delta = 1;

				pos += delta;
			}
		}

		assert(pos >= lowLimit && pos < highLimit);

		// Ensure that we found the first occurence
		while(pos > 0)
		{
			const LWMaterial &cur = materials[pos - 1];
			if(cur.getName() == name)
				--pos;
			else
				break;
		}

		return pos;
	}

	bool equal(const FBVector &a, const FBVector &b)
	{
		if(fabs(a.x - b.x) > 0.001)
			return false;
		if(fabs(a.y - b.y) > 0.001)
			return false;
		if(fabs(a.z - b.z) > 0.001)
			return false;

		return true;
	}

	bool equal(const FBVector2 &a, const FBVector2 &b)
	{
		if(fabs(a.x - b.x) > 0.001)
			return false;
		if(fabs(a.y - b.y) > 0.001)
			return false;

		return true;
	}

	bool onSameEdge(const FBVector &p1, const FBVector &p2, const FBVector &q, double epsilonSq)
	{
		FBVector qMinusP1 = q;
		qMinusP1 -= p1;
		FBVector p2MinusP1 = p2;
		p2MinusP1 -= p1;

		// Point q on edge line
		double qProj = qMinusP1.GetDotWith(p2MinusP1);
		double dSq = qMinusP1.GetSquareLength() - (qProj * qProj) / p2MinusP1.GetSquareLength();
		if(dSq < -epsilonSq || dSq > epsilonSq)
			return false;

		return true;
	}

	// Combines our callback parameters
	struct ScanStruct
	{
		ScanStruct(LWMeshInfo *meshInfo_, std::vector<LWVertex> *vertices_, std::vector<LWFace> *faces_, std::vector<LWMaterial> &materials_, const std::string fileName_)
		:	meshInfo(meshInfo_),
			vertices(vertices_),
			faces(faces_),
			maxOriginalVertex(0),
			materials(materials_),
			fileName(fileName_)
		{
		}

		int findNewIndex(const LWVertex &vo) const
		{
			int index = -1;
			for(unsigned int i = maxOriginalVertex; i < vertices->size(); ++i)
			{
				const LWVertex &v = (*vertices)[i];
				
				if(v.id != vo.id)
					continue;
				if(!equal(v.position, vo.position))
					continue;
				if(!equal(v.uv, vo.uv))
					continue;
				if(!equal(v.uv2, vo.uv2))
					continue;

				index = i;
				break;
			}

			if(index == -1)
			{
				index = vertices->size();
				vertices->push_back(vo);
			}

			return index;
		}

		LWMeshInfo *meshInfo;
		std::vector<LWVertex> *vertices;
		std::vector<LWFace> *faces;
		int maxOriginalVertex;

		// Stores pointId <-> vertexIndex pairs
		std::map<LWPntID, int> vertexIndex;

		// There can be multiple materials with same name. Use file name to separate them
		std::vector<LWMaterial> &materials;
		std::string fileName;
		std::string name;
	};
}

// Callback for getting id's

#if defined(NEW_SDK)
	static size_t pointScanCallback(void *info, LWPntID id);
	static size_t polygonScanCallback(void *info, LWPolID id);
#else
	static int pointScanCallback(void *info, LWPntID id);
	static int polygonScanCallback(void *info, LWPolID id);
#endif

static int getDominantAxis(const FBVector &v);

LWObject::LWObject(LWItemID id)
:	lwId(id),
	parentId(0),
	exporterId(-1)

{
}

LWObject::~LWObject()
{
}

bool LWObject::collectGeometry(std::vector<LWMaterial> &materials, const std::vector<LWBone> &bones)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return false;
	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();
	if(objectInfo == 0)
		return false;
	LWMeshInfo *meshInfo = objectInfo->meshInfo(lwId, TRUE);
	if(meshInfo == 0)
		return false;

	parentId = itemInfo->parent(lwId);
	fileName = objectInfo->filename(lwId);

	// Name
	name = itemInfo->name(lwId);
	if(name.find("DoNotExport") != name.npos)
		return false;

	std::string parentName;
	if(parentId)
		parentName = itemInfo->name(parentId);

	// Geometry callbacks
	ScanStruct scanInfo(meshInfo, &vertices, &faces, materials, fileName);
	scanInfo.name = name;
	meshInfo->scanPoints(meshInfo, &pointScanCallback, &scanInfo);
	scanInfo.maxOriginalVertex = scanInfo.vertices->size();
	meshInfo->scanPolys(meshInfo, &polygonScanCallback, &scanInfo);
	if(meshInfo->destroy)
		meshInfo->destroy(meshInfo);

	// We don't want empty meshes
	if((vertices.empty()) || (faces.empty()))
		return false;

	// Transform
	FBMatrix objectTm = LWTransforms::GetTransform(lwId);
	FBMatrix pivotTm = LWTransforms::GetPivotTransform(lwId);
/*	
	// If linked to bone helper, make tm relative
	if(parentName.substr(0, 11) == "HELPER_BONE")
	{
		Matrix parentTm = LWTransforms::GetTransform(parentId);
		objectTm = objectTm * parentTm.GetInverse();
	}
*/
	// Normals and texturing
	
	if(collectWeights(bones) == true)
		objectTm = FBMatrix();

	collectNormals(materials);
	collectTextureCoordinates(materials, objectTm);
	mapVertices();

	// Scaling
	double scale = LWTransforms::GetScale(lwId);

	// Exporter instance
	boost::shared_ptr<Object> object(new Object(name, parentName));
	object->setTransform(objectTm);
	object->setPivotTransform(pivotTm);

	// Exporter vertices
	for(unsigned int j = 0; j < vertices.size(); ++j)
	{
		Vertex v;
		v.setPosition(vertices[j].position * scale);
		v.setNormal(vertices[j].normal);
		v.setUv(vertices[j].uv);
		v.setUv2(vertices[j].uv2);

		const LWVertex &vo = vertices[j];

		for(int k = 0; k < LWVertex::MAX_WEIGHTS; ++k)
			v.addWeight(vertices[j].boneIndices[k], vertices[j].boneWeights[k]);

		object->addVertex(v);
	}

	// Exporter faces
	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		Face f;
		f.setNormal(faces[i].normal);

		int foo = faces[i].materialId;
		if(foo >= 0)
			f.setMaterialId(materials[foo].getExporterId());

		for(int j = 0; j < 3; ++j)
		{
			assert(faces[i].indices[j] < object->getVertexAmount());
			f.setVertexIndex(j, faces[i].indices[j]);
		}

		object->addFace(f);
	}
/*
	double size = calculateSize(object);
	if(size > 500.f)
	{
		std::string message = name;
		message += ": object over 500m wide, skipping it";
		Manager::getSingleton()->getExporter()->printWarning(message);
		return false;
	}
*/
	// Store
	exporterId = Manager::getSingleton()->getExporter()->getModel().addObject(object);
	return true;
}

bool LWObject::collectWeights(const std::vector<LWBone> &bones)
{
	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();
	if(objectInfo == 0)
		return false;
			
	LWMeshInfo *meshInfo = objectInfo->meshInfo(lwId, TRUE);
	if(meshInfo == 0)
		return false;

	bool hasWeights = false;

	// Each bone has it's own weightmap
	for(unsigned int i = 0; i < bones.size(); ++i)
	{
		const std::string &weightMap = bones[i].getWeightName();

		// Select vertex map to read
		void *map = 0;
		if((map = meshInfo->pntVLookup(meshInfo, LWVMAP_WGHT, weightMap.c_str())) == 0)
			continue;

		if(meshInfo->pntVSelect(meshInfo, map) != 1)
		{
			Manager::getSingleton()->getExporter()->printWarning(name + ": weightmap has more than 1 dimension");
			continue;
		}

		// Try for each vertex
		for(unsigned int j = 0; j < vertices.size(); ++j)
		{
			float weight = 0.f;
			if(meshInfo->pntVGet(meshInfo, vertices[j].id, &weight))
			{
				// 0-weight is valid
				//if(fabs(weight) < .001f)
				//	continue;


				// Set if greater than what's already in
				for(int k = 0; k < LWVertex::MAX_WEIGHTS; ++k)
				{
					if(vertices[j].boneIndices[k] == -1 || fabs(vertices[j].boneWeights[k]) < fabsf(weight)) 
					{	
						// Store old value forward
						if(k + 1 < LWVertex::MAX_WEIGHTS)
						{
							vertices[j].boneIndices[k + 1] = vertices[j].boneIndices[k];
							vertices[j].boneWeights[k + 1] = vertices[j].boneWeights[k];
						}

						vertices[j].boneIndices[k] = i;
						vertices[j].boneWeights[k] = weight;
						break;
					}
				}

				/*
				// Set if has no weight
				bool set = false;
				for(int k = 0; k < LWVertex::MAX_WEIGHTS; ++k)
				{
					if(vertices[j].boneIndices[k] == -1) 
					{
						vertices[j].boneIndices[k] = i;
						vertices[j].boneWeights[k] = weight;
						set = true;

						hasWeights = true;
						break;
					}
				}

				if(set == true)
					continue;

				// Set if greater than what's already in
				for(k = LWVertex::MAX_WEIGHTS - 1; k >= 0; --k)
				{
					if(fabs(vertices[j].boneWeights[k]) < fabsf(weight)) 
					{	
						// Store old value forward
						if(k + 1 < LWVertex::MAX_WEIGHTS)
						{
							vertices[j].boneIndices[k + 1] = vertices[j].boneIndices[k];
							vertices[j].boneWeights[k + 1] = vertices[j].boneWeights[k];
						}

						vertices[j].boneIndices[k] = i;
						vertices[j].boneWeights[k] = weight;
						break;
					}
				}
				*/
			}

		}
	}

	if(meshInfo->destroy)
		meshInfo->destroy(meshInfo);

	return hasWeights;
}

void LWObject::collectTextureCoordinates(const std::vector<LWMaterial> &materials, const FBMatrix &transform)
{
	// This part is a bit messy. 
	// Can't help it since LW's gotta do what LW's gotta do etc.
	LWTextureFuncs *textureFunctions = Manager::getSingleton()->getTextureFunctions();
	LWObjectInfo *objectInfo = Manager::getSingleton()->getObjectInfo();

	if((textureFunctions == 0) || (objectInfo == 0))
		return;

	LWMeshInfo *meshInfo = objectInfo->meshInfo(lwId, TRUE);
	if(meshInfo == 0)
		return;

	// Get them per face (looks ugly otherwise)
	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		// If no material, no texture
		if(faces[i].materialId == -1)
			continue;

		// Each face have different material
		LWTextureID textureId = materials[faces[i].materialId].getTextureId();
		if(textureId == 0)
			continue;

		LWTLayerID layerId = textureFunctions->firstLayer(textureId);
		if(layerId == 0)
			continue;

		int projection = 0;
		textureFunctions->getParam(layerId, TXTAG_PROJ, &projection);

		for(int j = 0; j < 3; ++j)
		{
			if(projection == TXPRJ_UVMAP) // UV-map
			{
				void *map = NULL;
				textureFunctions->getParam(layerId, TXTAG_VMAP, &map);

				if(map == 0)
					continue;

				if(meshInfo->pntVSelect(meshInfo, map) != 2)
				{
					Manager::getSingleton()->getExporter()->printWarning(name + ": UV-map isn't 2-dimensional. Discarding.");
					continue;
				}

				float uv[2] = { 0 };
				LWPntID vertexId = vertices[faces[i].indices[j]].id;

				// Finally query texture coordinates
				if(meshInfo->pntVPGet(meshInfo, vertexId, faces[i].id, uv))
				{
					faces[i].uvs[j].x = uv[0];
					faces[i].uvs[j].y = -uv[1];
					faces[i].hasUvs[j] = true;
				}
				else if(meshInfo->pntVGet(meshInfo, vertexId, uv))
				{
					faces[i].uvs[j].x = uv[0];
					faces[i].uvs[j].y = -uv[1];
					faces[i].hasUvs[j] = true;
				}
			}
			else // Projected mapping
			{
				// Normals largest absolute value defines dominant axis (in world and object space)
				FBVector worldNormal = faces[i].normal;
				transform.RotateVector(worldNormal);

				int wAxis = getDominantAxis(worldNormal);
				//int wAxis = getDominantAxis(transform.GetTransformedVector(faces[i].normal));
				int oAxis = getDominantAxis(faces[i].normal);

				// Position in world and object space
				FBVector &vO = vertices[faces[i].indices[j]].position;
				FBVector vW = transform.GetTransformedVector(vO);

				double oPosition[3] = { vO.x, vO.y, vO.z };
				double wPosition[3] = { vW.x, vW.y, vW.z };
				double uv[2] = { 0 };

				textureFunctions->evaluateUV(layerId, wAxis, oAxis, oPosition, wPosition, uv);
				faces[i].uvs[j].x = static_cast<double> (uv[0]);
				faces[i].uvs[j].y = static_cast<double> (-uv[1]);
				faces[i].hasUvs[j] = true;
			}

			//faces[i].uvs2[j] = faces[i].uvs[j];
		}
	}

	// In the name of holy spaghetti. Should clean this up a bit
	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		// If no material, no texture
		if(faces[i].materialId == -1)
			continue;

		// Each face have different material
		LWTextureID textureId = materials[faces[i].materialId].getLightmapId();
		if(textureId == 0)
			continue;
/*
int a = 0;
if(materials[faces[i].materialId].getName() == "Lattia")
	a = 1;
*/
		LWTLayerID layerId = textureFunctions->firstLayer(textureId);
		if(layerId == 0)
		{
			//materials[faces[i].materialId].removeLightmap();
			continue;
		}

		int projection = 0;
		textureFunctions->getParam(layerId, TXTAG_PROJ, &projection);

		for(int j = 0; j < 3; ++j)
		{
			if(projection == TXPRJ_UVMAP) // UV-map
			{
				void *map = 0;
				textureFunctions->getParam(layerId, TXTAG_VMAP, &map);

				if(meshInfo->pntVSelect(meshInfo, map) != 2)
				{
					Manager::getSingleton()->getExporter()->printWarning(name + ": No actual UV-map defined. Discarding.");
					//materials[faces[i].materialId].removeLightmap();
					continue;
				}

				if(map == 0)
				{
					//materials[faces[i].materialId].removeLightmap();
					continue;
				}

				if(meshInfo->pntVSelect(meshInfo, map) != 2)
				{
					Manager::getSingleton()->getExporter()->printWarning(name + ": UV-map isn't 2-dimensional. Discarding.");
					//materials[faces[i].materialId].removeLightmap();
					continue;
				}

				float uv[2] = { 0 };
				LWPntID vertexId = vertices[faces[i].indices[j]].id;

				// Finally query texture coordinates
				if(meshInfo->pntVPGet(meshInfo, vertexId, faces[i].id, uv))
				{
					faces[i].uvs2[j].x = uv[0];
					faces[i].uvs2[j].y = -uv[1];

					if(faces[i].hasUvs[j] == false)
					{
						faces[i].uvs[j].x = faces[i].uvs2[j].x;
						faces[i].uvs[j].y = faces[i].uvs2[j].y;
					}

					faces[i].hasUvs[j] = true;
				}
				else if(meshInfo->pntVGet(meshInfo, vertexId, uv))
				{
					faces[i].uvs2[j].x = uv[0];
					faces[i].uvs2[j].y = -uv[1];

					if(faces[i].hasUvs[j] == false)
					{
						faces[i].uvs[j].x = faces[i].uvs2[j].x;
						faces[i].uvs[j].y = faces[i].uvs2[j].y;
					}
					
					faces[i].hasUvs[j] = true;
				}
				//else
				//	materials[faces[i].materialId].removeLightmap();
			}
			//else
			//	materials[faces[i].materialId].removeLightmap();



/*
			else // Projected mapping
			{
				// Normals largest absolute value defines dominant axis (in world and object space)
				FBVector worldNormal = faces[i].normal;
				transform.RotateVector(worldNormal);

				int wAxis = getDominantAxis(worldNormal);
				int oAxis = getDominantAxis(faces[i].normal);

				// Position in world and object space
				Vector &vO = vertices[faces[i].indices[j]].position;
				Vector vW = transform.GetTransformedVector(vO);

				double oPosition[3] = { vO.x, vO.y, vO.z };
				double wPosition[3] = { vW.x, vW.y, vW.z };
				double uv[2] = { 0 };
				
				textureFunctions->evaluateUV(layerId, wAxis, oAxis, oPosition, wPosition, uv);
				faces[i].uvs2[j].x = static_cast<double> (uv[0]);
				faces[i].uvs2[j].y = static_cast<double> (-uv[1]);

				if(faces[i].hasUvs[j] == false)
				{
					faces[i].uvs[j].x = faces[i].uvs2[j].x;
					faces[i].uvs[j].y = faces[i].uvs2[j].y;
				}
				
				faces[i].hasUvs[j] = true;
			}
*/
		}
	}

	if(meshInfo->destroy)
		meshInfo->destroy(meshInfo);
}

void LWObject::collectNormals(const std::vector<LWMaterial> &materials)
{
	// Stores all faces for each vertex
	std::map<int, std::vector<int> > faceIndices;
	
	// Face normals
	for(unsigned int i = 0; i < faces.size(); ++i)
	{
		LWFace &face = faces[i];

		LWVertex &v0 = vertices[face.indices[0]];
		LWVertex &v1 = vertices[face.indices[1]];
		LWVertex &v2 = vertices[face.indices[2]];

		FBVector a = v1.position - v0.position;
		FBVector b = v2.position - v0.position;
		face.normal = a.GetCrossWith(b);

		// Store face indices
		for(int j = 0; j < 3; ++j)
			faceIndices[face.indices[j]].push_back(i);
	}

	// Calculate vertex normals (for each face)
	for(unsigned int j = 0; j < faces.size(); ++j)
	{
		LWFace &face = faces[j];
		double smoothingAngle = 0.f;
		
		int index = face.materialId;
		if(index != -1)
			smoothingAngle = materials[index].getSmoothingAngle();

		FBVector n = face.normal.GetNormalized();

		// Loop it's vertices
		for(unsigned int k = 0; k < 3; ++k)
		{
			int vertexIndex = face.indices[k];
			std::vector<int> &indices = faceIndices[vertexIndex];

			// Loop through all other faces this vertex belongs to
			for(unsigned int i = 0; i < indices.size(); ++i)
			{
				LWFace &otherFace = faces[indices[i]];
				FBVector n2 = otherFace.normal.GetNormalized();
				
				//double angle = acosf(face.normal.GetDotWith(otherFace.normal));
				double dot = n.GetDotWith(n2);
				double angle = acos(dot);

				// If this is current face or angle < smoothing angle or normals are practically equal
				if((dot > 0.99) || (angle < smoothingAngle) || (indices[i] == static_cast<int>(j)))
					face.normals[k] += otherFace.normal;
			}

			// Finally normalize resulting normal

			if(face.normals[k].GetSquareLength() > 0.0000000001f)
				face.normals[k].Normalize();
			else
				Manager::getSingleton()->getExporter()->printWarning("0 length normal");
		}
	}
}

void LWObject::mapVertices()
{
	// Fills in texture coordinates and normals
	// Creates new vertices as needed

	// Store all vertices for given index
	std::vector<std::vector<int> > handledVertices(vertices.size());

	for(unsigned int i = 0; i < faces.size(); ++i)
	for(int j = 0; j < 3; ++j)
	{
		int index = faces[i].indices[j];
		LWFace &f = faces[i];

		// If vertex has no values set, do it
		if(handledVertices[index].size() == 0)
		{
			LWVertex &v = vertices[index];
			v.normal = f.normals[j];
			v.uv = f.uvs[j];
			v.uv2 = f.uvs2[j];
			
			handledVertices[index].push_back(index);
			continue;
		}

		// Do we have to create new vertex
		bool createNew = false;

		// Loop all possibilities before creating new
		for(unsigned int k = 0; k < handledVertices[index].size(); ++k)
		{
			LWVertex &v = vertices[handledVertices[index][k]];

			// If normal or texture coordinates differ, create new vertex
			if(
				(fabs(v.normal.x - f.normals[j].x) > 0.01f) ||
				(fabs(v.normal.y - f.normals[j].y) > 0.01f) ||
				(fabs(v.normal.z - f.normals[j].z) > 0.01f)
			)
			{
				createNew = true;
			}

			if(
				(fabs(v.uv.x - f.uvs[j].x) > 0.001f) ||
				(fabs(v.uv.y - f.uvs[j].y) > 0.001f)
			)
			{
				createNew = true;
			}

			if(
				(fabs(v.uv2.x - f.uvs2[j].x) > 0.001f) ||
				(fabs(v.uv2.y - f.uvs2[j].y) > 0.001f)
			)
			{
				createNew = true;
			}

			if(createNew == false)
				break;
		}

		// Create
		if(createNew == true)
		{
			LWVertex newVertex = vertices[index];
			
			newVertex.normal = f.normals[j];
			newVertex.uv = f.uvs[j];
			newVertex.uv2 = f.uvs2[j];

			// Update face index
			faces[i].indices[j] = vertices.size();
			handledVertices[index].push_back(vertices.size());
			
			vertices.push_back(newVertex);
		}
	}
}

/*
  Helper functions
*/

// Callback for getting vertex data
#if defined(NEW_SDK)
	static size_t pointScanCallback(void *info, LWPntID id)
#else
	static int pointScanCallback(void *info, LWPntID id)
#endif
{
	ScanStruct *scanInfo = static_cast<ScanStruct *> (info);
	
	// Position
	LWFVector position;
	scanInfo->meshInfo->pntBasePos(scanInfo->meshInfo, id, position);

	LWVertex vertex;
	vertex.id = id;
	vertex.position = FBVector(position[0], position[1], position[2]);

	scanInfo->vertexIndex[id] = scanInfo->vertices->size();
	scanInfo->vertices->push_back(vertex);
	return 0;
}

static bool equal(const FBVector &a, const FBVector &b)
{
	if(
		fabs(a.x - b.x) < 0.001 &&
		fabs(a.x - b.x) < 0.001 &&
		fabs(a.x - b.x) < 0.001
		)
		return true;

	return false;
}

// Callback for getting face (polygon) data

#if defined(NEW_SDK)
	static size_t polygonScanCallback(void *info, LWPolID id)
#else
	static int polygonScanCallback(void *info, LWPolID id)
#endif
{
	ScanStruct *scanInfo = static_cast<ScanStruct *> (info);

	LWID type = scanInfo->meshInfo->polType(scanInfo->meshInfo, id);
	int vertices = scanInfo->meshInfo->polSize(scanInfo->meshInfo, id);
	
	// We can only handle triangles (TODO: split if count > 3)
	if(type != LWPOLTYPE_FACE || vertices != 3)
	{
		Manager::getSingleton()->getExporter()->printWarning(scanInfo->fileName + ": Non-triangular polygon found. Discarding.");
		return 0;
	}

	LWFace face;
	face.id = id;

	bool needLightmap = scanInfo->name.find("Lightmapped") != scanInfo->name.npos;

	// If we have materials and surface's been assigned to one, try to find it
	if(const char *foo = scanInfo->meshInfo->polTag(scanInfo->meshInfo, id, LWPTAG_SURF))
	{
		std::string surfaceName = foo;
/*
if(surfaceName == "Lattia")
{
	if(!needLightmap)
		assert(!"!!");
	int a = 0;
}
*/
		int materialIndex = findMaterialIndex(scanInfo->materials, surfaceName);
		assert(materialIndex >= 0);

		for(unsigned int i = materialIndex; i < scanInfo->materials.size(); ++i)
		{
			LWMaterial &mi = scanInfo->materials[i];

			// There can be multiple materials with same name. Differentiate by objects filename
			if(scanInfo->fileName == mi.getObjectName())
			{
				if(surfaceName == mi.getName())
				{
					face.materialId = i;

					if(needLightmap && mi.getLightmapId())
						break;
					if(!needLightmap && !mi.getLightmapId())
						break;

					/*
					// If material is not lightmapped or object should contain lightmap, break
					if(mi.getLightmapId() == 0)
						break;
					if(scanInfo->name.find("Lightmapped") != scanInfo->name.npos)
						break;

					int id = -1;
					for(unsigned int j = materialIndex; j < scanInfo->materials.size(); ++j)
					{
						LWMaterial &mj = scanInfo->materials[j];
						if(scanInfo->fileName == mj.getObjectName())
						{
							if(surfaceName == mj.getName())
							{
								if(mj.getLightmapId() == 0)
								{
									id = j;
									break;
								}

							}
						}
					}

					if(id >= 0)
						face.materialId = id;

					break;
					*/
				}
			}
		}

		/*
		for(unsigned int i = 0; i < scanInfo->materials.size(); ++i)
		{
			LWMaterial &mi = scanInfo->materials[i];

			// There can be multiple materials with same name. Differentiate by objects filename
			if(scanInfo->fileName == mi.getObjectName())
			{
				if(surfaceName == mi.getName())
				{
					face.materialId = i;

					// If material is not lightmapped or object should contain lightmap, break
					if(mi.getLightmapId() == 0)
						break;
					if(scanInfo->name.find("Lightmapped") != scanInfo->name.npos)
						break;

					int id = -1;
					for(unsigned int j = scanInfo->materials.size() - 1; j > i; --j)
					{
						LWMaterial &mj = scanInfo->materials[j];
						if(scanInfo->fileName == mj.getObjectName())
						{
							if(surfaceName == mj.getName())
							{
								if(mj.getLightmapId() == 0)
								{
									id = j;
									break;
								}

							}
						}
					}

					if(id >= 0)
					{
						face.materialId = id;
						break;
					}


					face.materialId = scanInfo->materials.size();
					LWMaterial material = scanInfo->materials[i];
					material.removeLightmap();
					material.add();

					scanInfo->materials.push_back(material);

					break;
				}
			}
		}
		*/
	}

	// Vertex indices
	for(int i = 0; i < 3; ++i)
	{
		LWPntID pointId = scanInfo->meshInfo->polVertex(scanInfo->meshInfo, id, i);
		face.indices[i] = scanInfo->vertexIndex[pointId];
	}

	FBVector v1 = (*scanInfo->vertices)[face.indices[0]].position;
	FBVector v2 = (*scanInfo->vertices)[face.indices[1]].position;
	FBVector v3 = (*scanInfo->vertices)[face.indices[2]].position;

	if(onSameEdge(v1, v2, v3, 0.000001f))
	{
		Manager::getSingleton()->getExporter()->printWarning(scanInfo->fileName + ": Degenerate polygon found. Discarding.");
		return false;
	}

	FBVector a = v2 - v1;
	FBVector b = v3 - v1;
	FBVector normal = a.GetCrossWith(b);
	if(normal.GetLength() < 0.000001f)
	{
		Manager::getSingleton()->getExporter()->printWarning(scanInfo->fileName + ": Degenerate polygon found. Discarding.");
		return false;
	}

	scanInfo->faces->push_back(face);

	if(face.materialId >= 0 && scanInfo->materials[face.materialId].isDoubleSided())
	{
		LWVertex v1 = (*scanInfo->vertices)[face.indices[1]];
		LWVertex v2 = (*scanInfo->vertices)[face.indices[0]];
		LWVertex v3 = (*scanInfo->vertices)[face.indices[2]];

		face.indices[0] = scanInfo->findNewIndex(v1);
		face.indices[1] = scanInfo->findNewIndex(v2);
		face.indices[2] = scanInfo->findNewIndex(v3);

		//std::swap(face.indices[0], face.indices[1]);
		scanInfo->faces->push_back(face);
	}

	return 0;
}

static int getDominantAxis(const FBVector &v)
{
	double x = fabs(v.x);
	double y = fabs(v.y);
	double z = fabs(v.z);
	
	if((x > y) && (x > z))
		return 0;
	if((y > x) && (y > z))
		return 1;
	return 2;
}


} // end of namespace export
} // end of namespace frozenbyte
