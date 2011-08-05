//#include "precompiled.h"

#include "physics_mass.h"
#include "parser.h"
#include "string_conversions.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../system/Logger.h"
#include <vector>

namespace frozenbyte {
namespace editor {
namespace {
	std::string fileName = "Editor\\Masses.fbt";
}

struct PhysicsMass::Data
{
	struct Mass
	{
		std::string name;
		float value;

		Mass()
		:	value()
		{
		}
	};

	struct MassSorter
	{
		bool operator () (const Mass &a, const Mass &b) const
		{
			return a.value < b.value;
		}
	};

	typedef std::vector<Mass> MassList;
	MassList masses;

	Data()
	{
		parseData();
	}

	void parseData()
	{
		Parser parser;
		filesystem::FilePackageManager::getInstance().getFile(fileName) >> parser;

		const ParserGroup &globals = parser.getGlobals();
		int groups = globals.getSubGroupAmount();

		for(int i = 0; i < groups; ++i)
		{
			Mass mass;
			mass.name = globals.getSubGroupName(i);
			
			if(mass.name.empty())
				continue;

			const ParserGroup &group = globals.getSubGroup(mass.name);
			const std::string &massString = group.getValue("value");
			mass.value = convertFromString<float> (massString, 20.f);

			masses.push_back(mass);
		}

		std::sort(masses.begin(), masses.end(), MassSorter());
	}
};

PhysicsMass::PhysicsMass()
:	data(new Data())
{
}

PhysicsMass::~PhysicsMass()
{
}

int PhysicsMass::getMassAmount() const
{
	return data->masses.size();
}

std::string PhysicsMass::getMassName(int index) const
{
	assert(index >= 0 && index <= getMassAmount());
	return data->masses[index].name;
}

int PhysicsMass::getMassIndex(const std::string &name) const
{
	for(int i = 0; i < getMassAmount(); ++i)
	{
		if(data->masses[i].name == name)
			return i;
	}

	//assert(!"Name not found! -- conflicts?");
	return 0;
}

float PhysicsMass::getMass(const std::string &name) const
{
	for(int i = 0; i < getMassAmount(); ++i)
	{
		if(data->masses[i].name == name)
			return data->masses[i].value;
	}

	//assert(!"Name not found! -- conflicts?");
	Logger::getInstance()->warning("PhysicsMass::getMass - Physics mass name not found.");
	Logger::getInstance()->debug(name.c_str());

	return 20.f;
}

} // editor
} // frozenbyte
