// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_DECORATOR_MODE_H
#define INCLUDED_EDITOR_DECORATOR_MODE_H

#ifndef INCLUDED_EDITOR_IMODE_H
#include "imode.h"
#endif

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

namespace frozenbyte {
namespace ui {
	class TerrainLegacy;
}

namespace editor {

class Gui;
class IEditorState;
struct Storm;
struct DecoratorModeData;

class DecoratorMode: public IMode
{
	boost::scoped_ptr<DecoratorModeData> data;

public:
	DecoratorMode(Gui &gui, Storm &storm, IEditorState &editorState);
	~DecoratorMode();
	
	void tick();
	void update();
	void reset();

	void guiTick();

	void setTerrainLegacy();
	ui::TerrainLegacy &getTerrainLegacy();

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
