// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef GROUP_LIST_UTILS_H
#define GROUP_LIST_UTILS_H

#include "group_list.h"
#include <istorm3d_model.h>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace frozenbyte {
namespace editor {

struct Storm;
class ObjectSettings;

struct ModelGroup
{
	std::vector<boost::shared_ptr<IStorm3D_Model> > models;
	GroupList::ObjectGroup objectGroup;
	VC3 rotationEul;
	float heightOffset;

	ModelGroup();

	VC3 getPosition(Storm &storm, int index) const;
	VC3 getRotation(Storm &storm, int index) const;

	void create(Storm &storm, const GroupList::ObjectGroup &group, ObjectSettings &objectSettings);
	void update(Storm &storm, const VC3 &position);
};

} // editor
} // frozenbyte

#endif