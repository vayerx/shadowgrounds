
#include "precompiled.h"

#include <assert.h>

#include "ObjectDurabilityParser.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../system/Logger.h"

using namespace frozenbyte;
using namespace frozenbyte::editor;

namespace util {
namespace {

	/*
	// sorting is not too good for this..
	struct DurabilitySorter
	{
		bool operator () (const ObjectDurabilityParser::ObjectDurability &a, const ObjectDurabilityParser::ObjectDurability &b) const
		{
			return a.name < b.name;
		}
	};
	*/

} // unnamed

ObjectDurabilityParser::ObjectDurabilityParser()
{
	// Empty group
	{
		ObjectDurability dur;
		dur.name = "no_durability";
		dur.requiredForce = 9999999.0f;

		objectDurabilities.push_back(dur);
	}

	EditorParser parser;
	filesystem::InputStream durabilitiesfile = filesystem::createInputFileStream("data/misc/object_durabilities.txt");
	durabilitiesfile >> parser;

	// TODO: parser sorts these alphabetically... I don't like that - would rather want them unsorted.
	// if parser cannot be fixed, should add some index value to each durability type...
	const ParserGroup &globals = parser.getGlobals();
	int groups = globals.getSubGroupAmount();

	minimumRequiredForce = 9999999.0f;

	for(int i = 0; i < groups; ++i)
	{
		ObjectDurability dur;
		dur.name = globals.getSubGroupName(i);
		if(dur.name.empty())
			continue;

		// Properties
		const ParserGroup &group = globals.getSubGroup(dur.name);

		const std::string &forceString = group.getValue("required_force");
		dur.requiredForce = convertFromString<float> (forceString, OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_FORCE);

		if (dur.requiredForce < minimumRequiredForce)
		{
			minimumRequiredForce = dur.requiredForce;
		}

		const std::string &accelString = group.getValue("required_acceleration");
		dur.requiredAcceleration = convertFromString<float> (accelString, OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_ACCELERATION);

		const std::string &angAccelString = group.getValue("required_angular_acceleration");
		dur.requiredAngularAcceleration = convertFromString<float> (angAccelString, OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_ANGULAR_ACCELERATION);

		if(dur.name == "base_durability")
			continue;

		objectDurabilities.push_back(dur);
	}

	// Leave no_durability material as first not depending on alphabetical order
	// sorting is not too good for this..
	/*
	std::sort(objectDurabilities.begin() + 1, objectDurabilities.end(), DurabilitySorter());
	*/
}

ObjectDurabilityParser::~ObjectDurabilityParser()
{
}

int ObjectDurabilityParser::getDurabilityTypeAmount() const
{
	return objectDurabilities.size();
}

const std::string &ObjectDurabilityParser::getDurabilityTypeName(int index) const
{
	assert(index >= 0 && index < getDurabilityTypeAmount());
	return objectDurabilities[index].name;
}

float ObjectDurabilityParser::getMinimumDurabilityRequiredForce() const
{
	return minimumRequiredForce;
}

const int ObjectDurabilityParser::getDurabilityTypeIndexByName(const std::string &name) const
{
	for (int i = 0; i < (int)objectDurabilities.size(); i++)
	{
		if (objectDurabilities[i].name == name)
		{
			return i;
		}
	}

	Logger::getInstance()->warning("ObjectDurabilityParser::getDurabilityTypeIndexByName - Given durability type name name not found.");
	Logger::getInstance()->debug(name.c_str());

	return OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX;
}

const ObjectDurabilityParser::ObjectDurabilityList &ObjectDurabilityParser::getObjectDurabilities() const
{
	return objectDurabilities;
}

} // util
