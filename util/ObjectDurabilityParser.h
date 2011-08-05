#ifndef INCLUDED_OBJECT_DURABILITY_PARSER_H
#define INCLUDED_OBJECT_DURABILITY_PARSER_H

#include <vector>
#include <string>

// (0 index is assumed to be "no_durability" material, use this define instead of value 0 to refer to that) --jpk
#define OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX 0

#define OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_FORCE 100.0f
#define OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_ACCELERATION 0.0f
#define OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_ANGULAR_ACCELERATION 0.0f


namespace util {

class ObjectDurabilityParser
{
public:
	ObjectDurabilityParser();
	~ObjectDurabilityParser();

	int getDurabilityTypeAmount() const;
	const std::string &getDurabilityTypeName(int index) const;

	const int getDurabilityTypeIndexByName(const std::string &name) const;

	float getMinimumDurabilityRequiredForce() const;

	struct ObjectDurability
	{
		std::string name;

		float requiredForce;
		float requiredAcceleration;
		float requiredAngularAcceleration;

		//std::vector<std::string> breakObjects;
		
		ObjectDurability()
		:	requiredForce(OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_FORCE),
			requiredAcceleration(OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_ACCELERATION),
			requiredAngularAcceleration(OBJECTDURABILITYPARSER_DEFAULT_REQUIRED_ANGULAR_ACCELERATION)
		{
		}
	};

	typedef std::vector<ObjectDurability> ObjectDurabilityList;
private:
	ObjectDurabilityList objectDurabilities;
	float minimumRequiredForce;

public:
	const ObjectDurabilityList &getObjectDurabilities() const;

};

} // util

#endif
