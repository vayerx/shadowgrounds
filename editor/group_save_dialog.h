// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_GROUP_SAVE_DIALOG_H
#define INCLUDED_GROUP_SAVE_DIALOG_H

#include <boost/scoped_ptr.hpp>
#include <string>

namespace frozenbyte {
namespace editor {

class GroupSaveDialog
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	GroupSaveDialog();
	~GroupSaveDialog();

	const std::string &getGroup();
	const std::string &getSubgroup();
	const std::string &getName();

	bool show();
};

} // editor
} // frozenbyte

#endif
