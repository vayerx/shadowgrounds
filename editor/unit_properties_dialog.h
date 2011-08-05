// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_TERRAIN_UNIT_PROPERTIES_DIALOG_H
#define INCLUDED_EDITOR_TERRAIN_UNIT_PROPERTIES_DIALOG_H

#include <vector>
#include <string>
#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
namespace editor {

struct UnitProperties;
struct StringProperties;

class UnitPropertiesDialog
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	UnitPropertiesDialog(UnitProperties &properties, const std::vector<std::string> &usedStrings, const StringProperties &stringProperties);
	~UnitPropertiesDialog();

	void execute(int id);
};

} // editor
} // frozenbyte

#endif
