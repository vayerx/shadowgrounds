// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "group_list.h"
#include "FindFileWrapper.h"
#include "parser.h"
#include "string_conversions.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include <fstream>
#include <algorithm>

using namespace boost;
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

typedef vector<GroupList::ObjectGroup> ObjectGroupList;

struct SubgroupImp
{
	string name;
	ObjectGroupList groups;
};

typedef vector<SubgroupImp> SubgroupList;

struct GroupImp
{
	string name;
	SubgroupList subgroups;
};

typedef vector<GroupImp> GroupListImp;

struct SubgroupImpSorter
{
	bool operator() (const SubgroupImp &a, const SubgroupImp &b)
	{
		return a.name < b.name;
	}
};

struct GroupImpSorter
{
	bool operator() (const GroupImp &a, const GroupImp &b)
	{
		return a.name < b.name;
	}
};

} // unnamed

struct GroupList::Data
{
	GroupListImp groups;

	void load(const string &name)
	{
		Parser parser(true, false);
		//ifstream(name.c_str()) >> parser;
		filesystem::FilePackageManager::getInstance().getFile(name) >> parser;

		const ParserGroup &root = parser.getGlobals();

		std::string group = root.getValue("group");
		std::string subGroup = root.getValue("subgroup");
		ObjectGroup objectGroup;
		objectGroup.name = root.getValue("name");
		objectGroup.original.x = convertFromString<float> (root.getValue("original_x"), 0.f);
		objectGroup.original.y = convertFromString<float> (root.getValue("original_y"), 0.f);
		objectGroup.original.z = convertFromString<float> (root.getValue("original_z"), 0.f);

		for(int i = 0; ; ++i)
		{
			string name = "Object";
			name += convertToString<int> (i + 1);

			const ParserGroup &ogroup = root.getSubGroup(name);
			Instance instance;
			instance.model = ogroup.getValue("model");
			if(instance.model.empty())
				break;

			instance.color.r = convertFromString<float> (ogroup.getValue("color_r"), 0.f);
			instance.color.g = convertFromString<float> (ogroup.getValue("color_g"), 0.f);
			instance.color.b = convertFromString<float> (ogroup.getValue("color_b"), 0.f);
			instance.position.x = convertFromString<float> (ogroup.getValue("position_x"), 0.f);
			instance.position.y = convertFromString<float> (ogroup.getValue("position_y"), 0.f);
			instance.position.z = convertFromString<float> (ogroup.getValue("position_z"), 0.f);
			instance.rotation.x = convertFromString<float> (ogroup.getValue("rotation_x"), 0.f);
			instance.rotation.y = convertFromString<float> (ogroup.getValue("rotation_y"), 0.f);
			instance.rotation.z = convertFromString<float> (ogroup.getValue("rotation_z"), 0.f);

			objectGroup.instances.push_back(instance);
		}

		add(group, subGroup, objectGroup);
	}

	void add(const string &group, const string &subgroup, const ObjectGroup &objectGroup)
	{
		int groupIndex = -1;
		int subgroupIndex = -1;
		if(!findIndices(group, subgroup, groupIndex, subgroupIndex))
		{
			if(groupIndex == -1)
			{
				groupIndex = groups.size();
				
				GroupImp groupImp;
				groupImp.name = group;
				groups.push_back(groupImp);
			}

			if(subgroupIndex == -1)
			{
				subgroupIndex = groups[groupIndex].subgroups.size();
				
				SubgroupImp groupImp;
				groupImp.name = subgroup;
				groups[groupIndex].subgroups.push_back(groupImp);
			}
		}

		groups[groupIndex].subgroups[subgroupIndex].groups.push_back(objectGroup);
	}

	void sortAll()
	{
		sort(groups.begin(), groups.end(), GroupImpSorter());

		for(unsigned int i = 0; i < groups.size(); ++i)
			sort(groups[i].subgroups.begin(), groups[i].subgroups.end(), SubgroupImpSorter());
	}

	void load()
	{
		string dir = "Editor/Prefabs";
		string findString = dir;
		findString += "/*.fbt";

		FindFileWrapper files(findString.c_str(), FindFileWrapper::File);
		for(; !files.end(); files.next())
		{
			std::string fileName = dir + std::string("/") + files.getName();
			load(fileName);
		}

		sortAll();
	}

	bool findIndices(const string &group, const string &subgroup, int &groupIndex, int &subgroupIndex) const
	{
		groupIndex = -1;
		subgroupIndex = -1;

		for(unsigned int i = 0; i < groups.size(); ++i)
		{
			if(groups[i].name == group)
			{
				groupIndex = i;

				for(unsigned int j = 0; j < groups[i].subgroups.size(); ++j)
				{
					if(groups[i].subgroups[j].name == subgroup)
					{
						subgroupIndex = j;
						return true;
					}

				}

				break;
			}
		}

		return false;
	}

	const string &get(int groupIndex) const
	{
		if(groupIndex < 0 || groupIndex >= int(groups.size()))
		{
			assert(!"Whoops");

			static string empty;
			return empty;
		}

		return groups[groupIndex].name;
	}

	const string &get(int groupIndex, int subgroupIndex) const
	{
		if(groupIndex < 0 || subgroupIndex < 0 || 
			groupIndex >= int(groups.size()) || subgroupIndex >= int(groups[groupIndex].subgroups.size()))
		{
			assert(!"Whoops");

			static string empty;
			return empty;
		}

		return groups[groupIndex].subgroups[subgroupIndex].name;
	}

	const ObjectGroup &get(int groupIndex, int subgroupIndex, int objectIndex) const
	{
		if(groupIndex < 0 || subgroupIndex < 0 || objectIndex < 0 ||
			groupIndex >= int(groups.size()) || subgroupIndex >= int(groups[groupIndex].subgroups.size()) ||
			objectIndex >= int(groups[groupIndex].subgroups[subgroupIndex].groups.size()))
		{
			assert(!"Whoops");

			static ObjectGroup empty;
			return empty;
		}

		return groups[groupIndex].subgroups[subgroupIndex].groups[objectIndex];
	}

};

GroupList::GroupList()
{
	scoped_ptr<Data> tempData(new Data());
	tempData->load();

	data.swap(tempData);
}

GroupList::~GroupList()
{
}

int GroupList::getGroupAmount() const
{
	return data->groups.size();
}

int GroupList::getSubgroupAmount(int group) const
{
	if(group < 0 || group >= int(data->groups.size()))
	{
		assert(!"Whoops");
		return 0;
	}

	return data->groups[group].subgroups.size();
}

int GroupList::getObjectGroupAmount(int group, int subgroup) const
{
	if(group < 0 || group >= int(data->groups.size()) ||
		subgroup < 0 || subgroup >= int(data->groups[group].subgroups.size()))
	{
		assert(!"Whoops");
		return 0;
	}

	return data->groups[group].subgroups[subgroup].groups.size();
}

const string &GroupList::getGroupName(int group) const
{
	return data->get(group);
}

const string &GroupList::getSubgroupName(int group, int subgroup) const
{
	return data->get(group, subgroup);
}

const GroupList::ObjectGroup &GroupList::getObjectGroup(int group, int subgroup, int object) const
{
	return data->get(group, subgroup, object);
}

void GroupList::reload()
{
	scoped_ptr<Data> tempData(new Data());
	tempData->load();

	data.swap(tempData);
}

void GroupList::addObjectGroup(const string &group, const string &subgroup, const ObjectGroup &objectGroup)
{
	Parser parser(true, false);
	ParserGroup &root = parser.getGlobals();
	root.setValue("group", group);
	root.setValue("subgroup", subgroup);
	root.setValue("name", objectGroup.name);
	root.setValue("original_x", convertToString<float> (objectGroup.original.x));
	root.setValue("original_y", convertToString<float> (objectGroup.original.y));
	root.setValue("original_z", convertToString<float> (objectGroup.original.z));

	for(unsigned int i = 0; i < objectGroup.instances.size(); ++i)
	{
		string name = "Object";
		name += convertToString<int> (i + 1);

		ParserGroup &ogroup = root.getSubGroup(name);
		const Instance &instance = objectGroup.instances[i];

		ogroup.setValue("model", instance.model);
		ogroup.setValue("color_r", convertToString<float> (instance.color.r));
		ogroup.setValue("color_g", convertToString<float> (instance.color.g));
		ogroup.setValue("color_b", convertToString<float> (instance.color.b));
		ogroup.setValue("position_x", convertToString<float> (instance.position.x));
		ogroup.setValue("position_y", convertToString<float> (instance.position.y));
		ogroup.setValue("position_z", convertToString<float> (instance.position.z));
		ogroup.setValue("rotation_x", convertToString<float> (instance.rotation.x));
		ogroup.setValue("rotation_y", convertToString<float> (instance.rotation.y));
		ogroup.setValue("rotation_z", convertToString<float> (instance.rotation.z));
	}

	string file = "Editor/Prefabs/";
	file += group;
	file += "_";
	file += subgroup;
	file += "_";
	file += objectGroup.name;
	file += ".fbt";
	ofstream(file.c_str()) << parser;
}

} // editor
} // frozenbyte
