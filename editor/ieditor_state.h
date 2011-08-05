// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_IEDITOR_STATE_H
#define INCLUDED_EDITOR_IEDITOR_STATE_H

#include "terrain_lightmap.h"
#include <vector>
#include <datatypedef.h>

namespace frozenbyte {
namespace editor {

class Camera;
class TerrainColorMap;
class EditorObjectState;
class LightMode;

class IEditorState
{
public:
	virtual ~IEditorState() {}

	virtual TerrainColorMap &getColorMap() = 0;
	virtual TerrainLightMap &getLightMap() = 0;
	virtual Camera &getCamera() = 0;
	virtual void hideObjects() = 0;
	virtual void showObjects() = 0;
	virtual void roofCollision(bool collision) = 0;
	virtual void getLights(std::vector<TerrainLightMap::PointLight> &lights) = 0;
	virtual void getBuildingLights(std::vector<TerrainLightMap::PointLight> &lights) = 0;

	virtual void updateHeightmap() = 0;
	virtual void updateTexturing() = 0;
	virtual void updateShadows() = 0;
	virtual void updateObjects() = 0;
	virtual void updateLighting() = 0;

	virtual VC3 getSunDirection() const = 0;
	virtual void getEditorObjectStates(EditorObjectState &states) const = 0;
	virtual void setEditorObjectStates(const EditorObjectState &states) = 0;

	virtual void visualizeCompletion() = 0;

	virtual void updateGrid(bool force=false) = 0;
	virtual void updateFolds(bool force=false) = 0;
	virtual void updateTriggersVisibility(bool force=false) = 0;
	virtual void updateUnitsVisibility(bool force=false) = 0;

	virtual void updateHelpersVisibility(bool force=false) = 0;
	virtual void updateStaticVisibility(bool force=false) = 0;
	virtual void updateDynamicVisibility(bool force=false) = 0;
	virtual void updateIncompleteVisibility(bool force=false) = 0;
	virtual void updateNoCollisionVisibility(bool force=false) = 0;

	// this "wrapping everything" method does not scale, so just getting direct access to 
	// this... (although that is absolutely horrible too)
	virtual LightMode &getLightMode() = 0;

};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
