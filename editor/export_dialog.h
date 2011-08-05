// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORT_DIALOG_H
#define INCLUDED_EDITOR_EXPORT_DIALOG_H

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

namespace frozenbyte {
namespace editor {

struct ExportOptions;
struct ExportDialogData;

class ExportDialog
{
	boost::scoped_ptr<ExportDialogData> data;

public:
	ExportDialog();
	~ExportDialog();

	ExportOptions show();
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
