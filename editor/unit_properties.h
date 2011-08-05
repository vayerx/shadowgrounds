// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_UNIT_PROPERTIES_H
#define INCLUDED_EDITOR_UNIT_PROPERTIES_H

#include <map>
#include <string>

namespace frozenbyte {
namespace filesystem {
	class OutputStream;
	class InputStream;
}

namespace editor {

struct UnitProperties
{
	enum Difficulty
	{
		All = 0,
		EasyOnly = 1,
		HardOnly = 2
	};

	std::map<std::string, std::string> strings;
	
	Difficulty difficulty;
	int layout;

	UnitProperties()
	:	difficulty(All),
		layout(0)
	{
	}

	void writeStream(filesystem::OutputStream &stream) const;
	void readStream(filesystem::InputStream &stream);
};

} // editor
} // frozenbyte

#endif
