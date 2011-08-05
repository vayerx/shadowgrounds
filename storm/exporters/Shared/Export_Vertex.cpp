// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Vertex.h"
#include <algorithm>

namespace frozenbyte {
namespace exporter {

Vertex::Vertex()
:	weights(0)
{
	for(int i = 0; i < MAX_WEIGHTS; ++i)
	{
		boneIndices[i] = -1;
		boneWeights[i] = 0.f;
	}
}

Vertex::~Vertex()
{
}

const FBVector &Vertex::getPosition() const
{
	return position;
}

const FBVector &Vertex::getNormal() const
{
	return normal;
}

const FBVector2 &Vertex::getUv() const
{
	return uv;
}

const FBVector2 &Vertex::getUv2() const
{
	return uv2;
}

bool Vertex::hasBoneWeights() const
{
	if(weights > 0)
		return true;
	return false;
}

float Vertex::getBoneWeight(int index) const
{
	assert(index < MAX_WEIGHTS);
	return boneWeights[index];
}

int Vertex::getBoneIndex(int index) const
{
	assert(index < MAX_WEIGHTS);
	return boneIndices[index];
}

void Vertex::setPosition(const FBVector &position_)
{
	position = position_;
}

void Vertex::setNormal(const FBVector &normal_)
{
	normal = normal_;
}

void Vertex::setUv(const FBVector2 &uv_)
{
	uv = uv_;
}

void Vertex::setUv2(const FBVector2 &uv_)
{
	uv2 = uv_;
}

void Vertex::addWeight(int boneIndex, float weight)
{
	if(boneIndex < 0)
		return;

	if(weights == MAX_WEIGHTS)
		return;

	boneIndices[weights] = boneIndex;
	boneWeights[weights] = weight;
	++weights;
}

void Vertex::correctBoneIndices(const std::vector<int> &newIndices)
{
	for(int i = 0; i < weights; ++i)
	{
		//if(std::find(newIndices.begin(), newIndices.end(), boneIndices[i]) != newIndices.end())
		boneIndices[i] = newIndices[boneIndices[i]];
	}
}


} // end of namespace export
} // end of namespace frozenbyte
