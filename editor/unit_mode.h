// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_UNIT_MODE_H
#define INCLUDED_EDITOR_UNIT_MODE_H

#ifndef INCLUDED_EDITOR_IMODE_H
#include "imode.h"
#endif
#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

namespace frozenbyte {
namespace editor {

class Dialog;
class IEditorState;
class Gui;
struct Storm;
struct UnitModeData;

class UnitMode: public IMode
{
	boost::scoped_ptr<UnitModeData> data;

public:
	UnitMode(Gui &gui, Storm &storm, IEditorState &editorState);
	~UnitMode();

	void tick();
	void reset();
	void update();

	void hideObjects();
	void showObjects();
	void updateLighting();

	void setGridUnitVisibility(bool gridVisible);
	void setUnitsVisibility(bool unitsVisible);
	void setTriggersVisibility(bool unitsVisible);

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
