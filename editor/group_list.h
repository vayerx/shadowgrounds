// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_GROUP_LIST_H
#define INCLUDED_GROUP_LIST_H

#include <boost/scoped_ptr.hpp>
#include <string>
#include <datatypedef.h>
#include <vector>

namespace frozenbyte {
namespace editor {

class GroupList
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	GroupList();
	~GroupList();

	struct Instance
	{
		std::string model;
		COL color;
		VC3 rotation;
		VC3 position;
	};

	struct ObjectGroup
	{
		std::string name;
		std::vector<Instance> instances;
		VC3 original;
	};

	int getGroupAmount() const;
	int getSubgroupAmount(int group) const;
	int getObjectGroupAmount(int group, int subgroup) const;
	const std::string &getGroupName(int group) const;
	const std::string &getSubgroupName(int group, int subgroup) const;
	const ObjectGroup &getObjectGroup(int group, int subgroup, int object) const;

	void reload();
	void addObjectGroup(const std::string &group, const std::string &subgroup, const ObjectGroup &objectGroup);
};

} // editor
} // frozenbyte

#endif
