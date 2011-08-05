// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_BONE_H
#define INCLUDED_BONE_H

#ifdef _MSC_VER
#pragma warning(disable: 4786) // identifier truncate
#pragma warning(disable: 4514) // removed unreferenced inline function (stl)
#pragma warning(disable: 4710) // function not inlined
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <DataTypeDef.h>
#endif

#include "Export_Types.h"

namespace frozenbyte {
namespace exporter {

class Bone
{
	std::string name;
	std::string parentName;
	
	FBMatrix restTransform;
	
	double restLength;
	double radius;

	FBVector maxAngles;
	FBVector minAngles;

public:
	Bone(const std::string &name, const std::string &parent);
	~Bone();

	const FBMatrix &getRestTransform() const;
	const FBVector &getMaxAngles() const;
	const FBVector &getMinAngles() const;
	
	double getRestLength() const;	
	double getRadius() const;
	
	const std::string &getName() const;
	const std::string &getParentName() const;

	void setRestTransform(const FBMatrix &transform);
	void setRestLength(double length);
	void setRadius(float radius);
};

// For hierarchy sorting. Parent's before their childs
bool operator < (const Bone &lhs, const Bone &rhs);

} // end of namespace export
} // end of namespace frozenbyte

#endif
