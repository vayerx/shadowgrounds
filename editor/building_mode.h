// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_BUILDING_MODE_H
#define INCLUDED_EDITOR_BUILDING_MODE_H

#ifndef INCLUDED_EDITOR_IMODE_H
#include "imode.h"
#endif

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

namespace frozenbyte {
namespace editor {

class Gui;
class IEditorState;
struct HeightmapData;
struct Storm;
struct BuildingModeData;

class BuildingMode: public IMode
{
	boost::scoped_ptr<BuildingModeData> data;

public:
	BuildingMode(Gui &gui, Storm &storm, IEditorState &editorState);
	~BuildingMode();

	void tick();
	void update();
	void reset();

	void updateLighting();
	void roofCollision(bool collision);

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
