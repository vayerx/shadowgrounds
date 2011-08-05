// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_VERTEX_H
#define INCLUDED_VERTEX_H

#ifdef _MSC_VER
#pragma warning(disable: 4514) // removed unreferenced inline function (stl)
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

#include "Export_Types.h"

#pragma pack(push, export_vertex_h)
#pragma pack(1)

namespace frozenbyte {
namespace exporter {

/**  @class Vertex
  *  @author psd
  *  @version 1.0
  *  @date    2001
  */
class Vertex
{
	FBVector position;
	FBVector normal;
	FBVector2 uv;
	FBVector2 uv2;

	enum { MAX_WEIGHTS = 2 };
	char weights;

	char boneIndices[MAX_WEIGHTS];
	float boneWeights[MAX_WEIGHTS];

public:
	Vertex();
	~Vertex();

	const FBVector &getPosition() const;
	const FBVector &getNormal() const;
	const FBVector2 &getUv() const;
	const FBVector2 &getUv2() const;
	
	bool hasBoneWeights() const;
	float getBoneWeight(int index) const;
	int getBoneIndex(int index) const;
	
	void setPosition(const FBVector &position);
	void setNormal(const FBVector &normal);
	void setUv(const FBVector2 &uv);
	void setUv2(const FBVector2 &uv);
	void addWeight(int boneIndex, float weight);

	void correctBoneIndices(const std::vector<int> &newIndices);
};

} // end of namespace export
} // end of namespace frozenbyte

#pragma pack(pop, export_vertex_h)

#endif
