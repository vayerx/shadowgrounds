// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Bone.h"

namespace frozenbyte {
namespace exporter {

Bone::Bone(const std::string &name_, const std::string &parent)
:	name(name_),
	parentName(parent),
	radius(0.f),
	maxAngles(3.14f, 3.14f, 3.14f)
{
}

Bone::~Bone()
{
}

const FBMatrix &Bone::getRestTransform() const
{
	return restTransform;
}

double Bone::getRestLength() const
{
	return restLength;
}

double Bone::getRadius() const
{
	return radius;
}

const FBVector &Bone::getMaxAngles() const
{
	return maxAngles;
}

const FBVector &Bone::getMinAngles() const
{
	return minAngles;
}

const std::string &Bone::getName() const
{
	return name;
}

const std::string &Bone::getParentName() const
{
	return parentName;
}

void Bone::setRestTransform(const FBMatrix &transform)
{
	restTransform = transform;
}

void Bone::setRestLength(double length)
{
	restLength = length;
}

void Bone::setRadius(float radius_)
{
	radius = radius_;
}

bool operator < (const Bone &lhs, const Bone &rhs)
{
	if(lhs.getName() == rhs.getParentName())
		return true;

	return false;
}	


} // end of namespace export
} // end of namespace frozenbyte
