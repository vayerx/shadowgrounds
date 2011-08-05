// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_HELPER_H
#define INCLUDED_HELPER_H

#ifdef _MSC_VER
#pragma warning(disable: 4514) // removed unreferenced inline function (stl)
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

class Helper
{
public:
	enum Type { point = 0, vector = 1, box = 2, camera = 3, sphere = 4 };

private:	
	Type type;

	std::string name;
	std::string parentName;

	FBVector position;
	FBVector other[2];

public:
	Helper(Type type, const std::string &name, const std::string &parent);
	~Helper();

	const std::string &getName() const;
	const std::string &getParentName() const;

	// Common
	Type getType() const;
	const FBVector &getPosition() const;
	void setPosition(const FBVector &position);

	// Vector & Camera (upvector for camera only)
	const FBVector &getDirection() const;
	const FBVector &getUpVector() const;
	void setDirection(const FBVector &direction);
	void setUpVector(const FBVector &upVector);

	// Sphere
	double getRadius() const;
	void setRadius(float radius);

	void writeToFile(FILE *fp) const;
};

// For hierarchy sorting. Parent's before their childs
bool operator < (const Helper &lhs, const Helper &rhs);

} // end of namespace export
} // end of namespace frozenbyte

#endif
