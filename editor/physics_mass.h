#ifndef EDITOR_PHYSICS_MASS_H
#define EDITOR_PHYSICS_MASS_H

#include <string>
#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
namespace editor {

class PhysicsMass
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	PhysicsMass();
	~PhysicsMass();

	// Name mapping
	int getMassAmount() const;
	std::string getMassName(int index) const;
	int getMassIndex(const std::string &name) const;

	// Value
	float getMass(const std::string &name) const;
};

} // editor
} // frozenbyte

#endif

