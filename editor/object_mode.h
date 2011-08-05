// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_OBJECT_MODE_H
#define INCLUDED_EDITOR_OBEJCT_MODE_H

#ifndef INCLUDED_EDITOR_IMODE_H
#include "imode.h"
#endif

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

namespace frozenbyte {
namespace editor {

class Gui;
class IEditorState;
class EditorObjectState;
struct Storm;
struct ObjectModeData;

class ObjectMode: public IMode
{
	boost::scoped_ptr<ObjectModeData> data;

public:
	ObjectMode(Gui &gui, Storm &storm, IEditorState &editorState);
	~ObjectMode();

	void tick();
	void update();
	void reset();

	void resetTerrain();
	void restoreTerrain();

	void hideObjects();
	void showObjects();
	void updateLighting();

	void setHelpersVisibility(bool helpersVisible);
	void setIncompleteVisibility(bool incompleteVisible);
	void setStaticVisibility(bool staticVisible);
	void setDynamicVisibility(bool dynamicVisible);
	void setNoCollisionVisibility(bool noCollisionVisible);

	void getEditorObjectStates(EditorObjectState &states) const;
	void setEditorObjectStates(const EditorObjectState &states);

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
