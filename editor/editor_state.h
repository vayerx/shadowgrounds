// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EDITOR_STATE_H
#define INCLUDED_EDITOR_EDITOR_STATE_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_EDITOR_IEDITOR_STATE_H
#include "ieditor_state.h"
#endif

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

class Gui;
class Dialog;
class Camera;
struct Storm;
struct ExportOptions;
struct EditorStateData;

class LightMode;

class EditorState: public IEditorState
{
	boost::scoped_ptr<EditorStateData> data;

public:
	EditorState(Gui &gui, Storm &storm, Camera &camera);
	~EditorState();

	void tick();
	void update();
	void reset();

	void updateHeightmap();
	void updateTexturing();
	void updateShadows();
	void updateObjects();
	void updateLighting();

	VC3 getSunDirection() const;
	void getEditorObjectStates(EditorObjectState &states) const;
	void setEditorObjectStates(const EditorObjectState &states);

	TerrainColorMap &getColorMap();
	TerrainLightMap &getLightMap();
	Camera &getCamera();
	void hideObjects();
	void showObjects();
	void roofCollision(bool collision);
	void getLights(std::vector<TerrainLightMap::PointLight> &lights);
	void getBuildingLights(std::vector<TerrainLightMap::PointLight> &lights);

	void visualizeCompletion();
	void endCompletionVisualization();
	void exportData(const ExportOptions &options) const;

	void updateGrid(bool force=false);
	void updateFolds(bool force=false);
	void updateTriggersVisibility(bool force=false);
	void updateUnitsVisibility(bool force=false);

	void updateHelpersVisibility(bool force=false);
	void updateIncompleteVisibility(bool force=false);
	void updateStaticVisibility(bool force=false);
	void updateDynamicVisibility(bool force=false);
	void updateNoCollisionVisibility(bool force=false);

	// (crap. i want direct access to this!)
	LightMode &EditorState::getLightMode();


	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const EditorState &state) 
{ 
	return state.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, EditorState &state) 
{ 
	return state.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
