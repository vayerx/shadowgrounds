// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORT_OPTIONS_H
#define INCLUDED_EDITOR_EXPORT_OPTIONS_H

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace editor {

struct ExportOptions
{
	std::string fileName;
	std::string id;

	bool onlyScripts;

	ExportOptions()
	:	onlyScripts(false)
	{
	}
};

inline std::string makeMissionIdFromFileName(const std::string &fileName)
{
	int index = fileName.find_last_of("/\\");
	assert(index < int(fileName.size()));

	std::string id = fileName.substr(index + 1, fileName.size() - index - 1);

	for(unsigned int i = 0; i < id.size(); ++i)
		id[i] = tolower(id[i]);

	return id;
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
