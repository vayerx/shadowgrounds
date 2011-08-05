// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_SCENE_MODE_H
#define INCLUDED_EDITOR_SCENE_MODE_H

#ifndef INCLUDED_EDITOR_IMODE_H
#include "imode.h"
#endif
#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

#include <datatypedef.h>

namespace frozenbyte {
namespace editor {

class Dialog;
class Gui;
class IEditorState;
struct Storm;
struct SceneModeData;

class SceneMode: public IMode
{
	boost::scoped_ptr<SceneModeData> data;

public:
	SceneMode(Gui &gui, Storm &storm, IEditorState &editorState);
	~SceneMode();

	void tick();
	void reset();

	void setLightning();
	void update();
	VC3 getSunDirection() const;

	bool doesShowFolds() const; 
	bool doesShowGrid() const;
	bool doesShowTriggers() const;
	bool doesShowUnits() const;

	bool doesShowHelpers() const;
	bool doesShowStatic() const;
	bool doesShowDynamic() const;
	bool doesShowNoCollision() const;
	bool doesShowIncomplete() const;

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
