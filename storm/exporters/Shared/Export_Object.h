// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EXPORT_OBJECT_H
#define INCLUDED_EXPORT_OBJECT_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#pragma warning(disable: 4710) // function not inlined
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

#ifndef INCLUDED_EXPORT_FACE_H
#include "Export_Face.h"
#endif
#ifndef INCLUDED_EXPORT_VERTEX_H
#include "Export_Vertex.h"
#endif
#ifndef INCLUDED_EXPORT_MATERIALS_H
#include "Export_Material.h"
#endif

#include "Export_Types.h"

#pragma pack(push, export_object_h)
#pragma pack(1)

namespace frozenbyte {
namespace exporter {

class Object
{
	std::vector<Vertex> vertices;
	std::vector<Face> faces;

	std::string name;
	std::string parentName;

	FBMatrix transform;
	FBMatrix pivotTransform;

	bool cameraVisibility;
	bool collisionVisibility;
	bool lightObject;

public:
	enum { LOD_AMOUNT = 3 };

	Object(const std::string &name, const std::string &parent);
	Object();
	~Object();

	void setStrings(const std::string &name, const std::string &parent);
	void setLightObject();

	int getVertexAmount() const;
	int getFaceAmount() const;
	FBMatrix getTm() const;
	int getMaterialAmount() const;

	const std::string &getName() const;
	const std::string &getParentName() const;
	const std::vector<Face> &getFaces() const;
	const std::vector<Vertex> &getVertices() const;
	std::vector<Face> &getFaces();
	std::vector<Vertex> &getVertices();

	std::vector<int> getMaterialIndices(bool includeNoMaterial = false) const;
	const FBMatrix &getTransform() const;
	const FBMatrix &getPivotTransform() const;

	void setTransform(const FBMatrix &tm);
	void setPivotTransform(const FBMatrix &tm);
	void addVertex(const Vertex &vertex);
	void addFace(const Face &face);

	void setProperties(bool cameraVisibility, bool collisionVisibility);
	bool hasCollisionFlag() const;
	bool hasNoCollisionFlag() const;
	bool hasNoVisibilityFlag() const;
	bool hasBuildingRoofFlag() const;
	bool hasBuildingFloorFlag() const;
	bool hasBuildingOuterWallFlag() const;
	bool hasDecalFlag() const;
	bool hasEditorOnlyFlag() const;
	bool hasCameraVisibility() const;
	bool hasCollisionVisibility() const;
	bool isLightObject() const;

	// Correct data
	void correctBoneIndices(const std::vector<int> &newIndices);
	void correctMaterialIndices(const std::vector<int> &newIndices);
	void remapIndices();

	void writeToFile(FILE *fp, const FBMatrix &parentTm,const FBMatrix &pivotTm,  const std::vector<int> &usedMaterials, bool optimizeVcache, bool factorPivot) const;
};

// For hierarchy sorting. Parent's before their childs
bool operator < (const Object &lhs, const Object &rhs);

} // end of namespace export
} // end of namespace frozenbyte

#pragma pack(pop, export_object_h)

#endif
