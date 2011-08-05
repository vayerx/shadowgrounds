// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LWEXPORT_OBJECT_H
#define INCLUDED_LWEXPORT_OBJECT_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

#ifndef INCLUDED_LWMESHES_H
#define INCLUDED_LWMESHES_H
#include <lwmeshes.h>
#endif
#ifndef INCLUDED_LWRENDER_H
#define INCLUDED_LWRENDER_H
#include <lwrender.h>
#endif

#ifndef INCLUDED_LWEXPORT_MATERIAL_H
#include "LWExport_Material.h"
#endif
#ifndef INCLUDED_LWEXPORT_BONE_H
#include "LWExport_Bone.h"
#endif

#include "../Shared/Export_Types.h"

namespace frozenbyte {
namespace exporter {

namespace {
	struct LWFace
	{
		LWFace()
		:	id(0),
			materialId(-1)
		{
			indices[0] = -1;
			indices[1] = -1;
			indices[2] = -1;

			hasUvs[0] = false;
			hasUvs[1] = false;
			hasUvs[2] = false;
		}

		LWPolID id;

		// Face data
		int materialId;
		FBVector normal;

		// Per vertex data
		int indices[3];
		FBVector normals[3];
		FBVector2 uvs[3];
		FBVector2 uvs2[3];
		bool hasUvs[3];
	};

	struct LWVertex
	{
		LWVertex()
		:	id(0)
		{
			for(int i = 0; i < MAX_WEIGHTS; ++i)
			{
				boneWeights[i] = 0.f;
				boneIndices[i] = -1;
			}
		}

		LWPntID id;
		
		FBVector position;
		FBVector normal;
		FBVector2 uv;
		FBVector2 uv2;

		enum { MAX_WEIGHTS = 4 };
		float boneWeights[MAX_WEIGHTS];
		int boneIndices[MAX_WEIGHTS];
	};
}

/**  @class LWObject
  *  @brief Collects object data (transform/parent/geometry)
  *  @author Juha Hiekkamäki
  *  @version 1.0
  *  @date 2001
  */
class LWObject
{
	LWItemID lwId;
	LWItemID parentId;
	int exporterId;

	std::vector<LWVertex> vertices;
	std::vector<LWFace> faces;

	std::string name;
	std::string fileName;

public:
	explicit LWObject(LWItemID id);
	~LWObject();

	// Materials needed for uv's and normals
	bool collectGeometry(std::vector<LWMaterial> &materials, const std::vector<LWBone> &bones);

private:
	bool collectWeights(const std::vector<LWBone> &bones);
	void collectTextureCoordinates(const std::vector<LWMaterial> &materials, const FBMatrix &transform);
	void collectNormals(const std::vector<LWMaterial> &materials);
	void mapVertices();
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
