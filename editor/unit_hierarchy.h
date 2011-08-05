#ifndef INCLUDED_UNIT_HIERARCHY_H
#define INCLUDED_UNIT_HIERARCHY_H

#include <boost/scoped_ptr.hpp>
#include <string>

namespace frozenbyte {
namespace editor {

class UnitScripts;

class UnitHierarchy
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	explicit UnitHierarchy(UnitScripts &scripts);
	~UnitHierarchy();

	void init();

	int getPrimaryAmount() const;
	int getSecondaryAmount(int primaryIndex) const;
	const std::string &getPrimaryName(int primaryIndex) const;
	const std::string &getSecondaryName(int primaryIndex, int secondaryIndex) const;

	bool setGroups(int primaryIndex, int secondaryIndex);
	int getUnitScriptAmount() const;
	int getUnitScriptIndex(int listIndex) const;
	int getUnitListIndex(int scriptIndex) const;
};

} // editor
} // frozenbyte

#endif
