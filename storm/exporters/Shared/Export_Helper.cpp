// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Helper.h"
#include <cstdio>

namespace frozenbyte {
namespace exporter {

Helper::Helper(Type type_, const std::string &name_, const std::string &parent)
:	name(name_),
	type(type_),
	parentName(parent)
{
}

Helper::~Helper()
{
}

const std::string &Helper::getName() const
{
	return name;
}

const std::string &Helper::getParentName() const
{
	return parentName;
}

// Common
Helper::Type Helper::getType() const
{
	return type;
}

const FBVector &Helper::getPosition() const
{
	return position;
}

void Helper::setPosition(const FBVector &position_)
{
	position = position_;
}

// Camera
const FBVector &Helper::getDirection() const
{
	return other[0];
}

const FBVector &Helper::getUpVector() const
{
	return other[1];
}

void Helper::setDirection(const FBVector &direction)
{
	other[0] = direction;
}

void Helper::setUpVector(const FBVector &upVector)
{
	other[1] = upVector;
}

// Sphere
double Helper::getRadius() const
{
	return other[0].x;
}

void Helper::setRadius(float)
{
}

void Helper::writeToFile(FILE *fp) const
{
	// Names
	fwrite(name.c_str(), 1, name.size() + 1, fp);
	fwrite(parentName.c_str(), 1, parentName.size() + 1, fp);

	// Type
	fwrite(&type, sizeof(int), 1, fp);

	VC3 pos = convert(position);
	VC3 o0 = convert(other[0]);
	VC3 o1 = convert(other[1]);

	fwrite(&pos, sizeof(float), 3, fp);
	fwrite(&o0, sizeof(float), 3, fp);
	fwrite(&o1, sizeof(float), 3, fp);

	int keyframeEnd = 0;
	WORD keyAmount = 0;

	fwrite(&keyframeEnd, sizeof(int), 1, fp);

	// Position/rotation/scale keys
	fwrite(&keyAmount, sizeof(WORD), 1, fp);
	fwrite(&keyAmount, sizeof(WORD), 1, fp);
	fwrite(&keyAmount, sizeof(WORD), 1, fp);
}

bool operator < (const Helper &lhs, const Helper &rhs)
{
	if(lhs.getName() == rhs.getParentName())
		return true;

	return false;
}	

} // end of namespace export
} // end of namespace frozenbyte
