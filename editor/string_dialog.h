#ifndef INCLUDED_STRING_DIALOG_H
#define INCLUDED_STRING_DIALOG_H

#include <boost/scoped_ptr.hpp>
#include <string>

namespace frozenbyte {
namespace editor {

class StringDialog
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	StringDialog();
	~StringDialog();

	std::string show(const std::string &title);
};

} // editor
} // frozenbyte

#endif
