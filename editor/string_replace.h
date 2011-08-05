// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_STRING_REPLACE_H
#define INCLUDED_EDITOR_STRING_REPLACE_H

#include <string>
#include <map>

namespace frozenbyte {
namespace editor {

typedef std::map<std::string, std::string> StringMap;
std::string replaceString(const std::string &source, const StringMap &strings, const StringMap &defaults);

} // editor
} // frozenbyte

#endif
