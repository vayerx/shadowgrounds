// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "editor_state.h"
#include "gui.h"
#include "window.h"
#include "color_picker.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "icommand.h"
#include "command_list.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "storm.h"
#include "camera.h"
#include "terrain_mode.h"
#include "terrain_colormap.h"
#include "terrain_lightmap.h"
#include "scene_mode.h"
#include "object_mode.h"
#include "building_mode.h"
#include "unit_mode.h"
#include "decorator_mode.h"
#include "light_mode.h"

#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../ui/lightmanager.h"
#include <vector>
#include "resource/resource.h"

namespace frozenbyte {
namespace editor {
namespace {
	struct SharedData
	{
		IMode *activeMode;

		SharedData()
		:	activeMode(0)
		{
		}
	};

	class ModeCommand: public ICommand
	{
		SharedData &sharedData;
		IMode *mode;

	public:
		ModeCommand(SharedData &sharedData_, IMode *mode_, Gui &gui, int id)
		:	sharedData(sharedData_),
			mode(mode_)
		{
			gui.setMenuCommand(id, this);
		}

		void execute(int id)
		{
			sharedData.activeMode = mode;
		}
	};
}

struct EditorStateData
{
	Gui &gui;
	SharedData sharedData;
	Camera &camera;
	Storm &storm;

	TerrainColorMap colorMap;
	TerrainLightMap lightMap;

	TerrainMode terrainMode;
	SceneMode sceneMode;
	ObjectMode objectMode;
	BuildingMode buildingMode;
	UnitMode unitMode;
	DecoratorMode decoratorMode;
	LightMode lightMode;

	std::vector<IMode *> modes;

	bool updateHeightmap;
	bool updateTexturing;
	bool updateShadows;
	bool updateObjects;

	bool lastShowGridState;
	bool lastShowFoldsState;
	bool lastShowTriggersState;
	bool lastShowUnitsState;

	bool lastShowHelpersState;
	bool lastShowIncompleteState;
	bool lastShowStaticState;
	bool lastShowDynamicState;
	bool lastShowNoCollisionState;

	ModeCommand terrainCommand;
	ModeCommand sceneCommand;
	ModeCommand objectCommand;
	ModeCommand buildingCommand;
	ModeCommand unitCommand;
	ModeCommand decoratorCommand;
	ModeCommand lightCommand;

	bool flashEnabled;
	int flashTimer;

	EditorStateData(Gui &gui_, Storm &storm_, IEditorState &state, Camera &camera_)
	:	gui(gui_),
		camera(camera_),
		storm(storm_),
		colorMap(storm),
		lightMap(storm),

		terrainMode(gui, storm, state),
		sceneMode(gui, storm, state),
		objectMode(gui, storm, state),
		buildingMode(gui, storm, state),
		unitMode(gui, storm, state),
		decoratorMode(gui, storm, state),
		lightMode(gui, storm, state),

		terrainCommand(sharedData, &terrainMode, gui, IDC_MENU_TERRAIN),
		sceneCommand(sharedData, &sceneMode, gui, IDC_MENU_SCENE),
		objectCommand(sharedData, &objectMode, gui, IDC_MENU_OBJECTS),
		buildingCommand(sharedData, &buildingMode, gui, IDC_MENU_BUILDINGS),
		unitCommand(sharedData, &unitMode, gui, IDC_MENU_UNIT),
		decoratorCommand(sharedData, &decoratorMode, gui, IDC_MENU_DECORATORS),
		lightCommand(sharedData, &lightMode, gui, IDC_MENU_LIGHTS),
		flashEnabled(false),
		flashTimer(0)
	{
		reset();

		lastShowGridState = true;
		lastShowTriggersState = true;
		lastShowHelpersState = true;
		lastShowUnitsState = true;
		lastShowFoldsState = false;

		modes.push_back(&terrainMode);
		modes.push_back(&sceneMode);
		modes.push_back(&objectMode);
		modes.push_back(&buildingMode);
		modes.push_back(&decoratorMode);
		modes.push_back(&unitMode);
		modes.push_back(&lightMode);

		sharedData.activeMode = &terrainMode;
	}

	void enableUpdate()
	{
		enableDialogItem(gui.getMenuDialog(), IDC_MENU_UPDATE, true);
	}

	void update()
	{
		if(updateHeightmap)
		{
			HeightmapData &heightmapData = terrainMode.loadHeightmap();
		
			terrainMode.setHeightmap();
			objectMode.resetTerrain();
		}

		if(updateHeightmap || updateTexturing || updateShadows)
		{
			sceneMode.setLightning();
			terrainMode.setTexturing();
		}

		objectMode.restoreTerrain();
		for(unsigned int i = 0; i < modes.size(); ++i)
			modes[i]->update();

		decoratorMode.setTerrainLegacy();

		updateHeightmap = false;
		updateTexturing = false;
		updateShadows = false;
		updateObjects = false;

		enableDialogItem(gui.getMenuDialog(), IDC_MENU_UPDATE, false);
	}

	void tick(int ms)
	{
		static int flashFreq = 500;
		if(flashEnabled)
		{
			flashTimer += ms;
			if(flashTimer >= flashFreq)
			{
				FlashWindow(gui.getMainWindow().getWindowHandle(), TRUE);
				flashTimer -= flashFreq;
			}
		}
	}

	void reset()
	{
		updateHeightmap = false;
		updateTexturing = false;
		updateShadows = false;
		updateObjects = false;

		for(unsigned int i = 0; i < modes.size(); ++i)
			modes[i]->reset();
	}
};

EditorState::EditorState(Gui &gui, Storm &storm, Camera &camera)
{
	boost::scoped_ptr<EditorStateData> tempData(new EditorStateData(gui, storm, *this, camera));
	data.swap(tempData);
}

EditorState::~EditorState()
{
}

void EditorState::tick()
{
	int ms = int(getTimeDelta() * 1000.f);

	if(data->sharedData.activeMode)
		data->sharedData.activeMode->tick();

	if(data->storm.lightManager)
	{
		VC3 pos = data->camera.getPosition();
		data->storm.lightManager->update(pos, pos, ms);
	}

	data->decoratorMode.guiTick();
	//data->colorMap.debugRender();
	//data->lightMap.debugRender();
	
	data->tick(ms);
}

void EditorState::update()
{
	data->update();
}

void EditorState::reset()
{
	data->reset();
}

Camera &EditorState::getCamera()
{
	return data->camera;
}

TerrainColorMap &EditorState::getColorMap()
{
	return data->colorMap;
}

TerrainLightMap &EditorState::getLightMap()
{
	return data->lightMap;
}

void EditorState::hideObjects()
{
	data->objectMode.hideObjects();
	data->unitMode.hideObjects();
	data->lightMode.hideObjects();
}

void EditorState::showObjects()
{
	data->objectMode.showObjects();
	data->unitMode.showObjects();
	data->lightMode.showObjects();
}

void EditorState::roofCollision(bool collision)
{
	data->buildingMode.roofCollision(collision);
}

// (crap. i want direct access to this!)
LightMode &EditorState::getLightMode()
{
	return data->lightMode;
}

void EditorState::getLights(std::vector<TerrainLightMap::PointLight> &lights)
{
	data->lightMode.getLights(lights, false);
}

void EditorState::getBuildingLights(std::vector<TerrainLightMap::PointLight> &lights)
{
	data->lightMode.getLights(lights, true);
}

void EditorState::updateHeightmap()
{
	data->updateHeightmap = true;
	data->enableUpdate();
}

void EditorState::updateTexturing()
{
	data->updateTexturing = true;
	data->enableUpdate();
}

void EditorState::updateShadows()
{
	data->updateShadows = true;
	data->enableUpdate();
}

void EditorState::updateObjects()
{
	data->updateObjects = true;
	data->enableUpdate();
}

void EditorState::updateLighting()
{
	// Reset lights on manager?

	data->objectMode.updateLighting();
	data->buildingMode.updateLighting();
	data->unitMode.updateLighting();
}

VC3 EditorState::getSunDirection() const
{
	return data->sceneMode.getSunDirection();
}

void EditorState::getEditorObjectStates(EditorObjectState &states) const
{
	data->objectMode.getEditorObjectStates(states);
}

void EditorState::setEditorObjectStates(const EditorObjectState &states)
{
	data->objectMode.setEditorObjectStates(states);
}

void EditorState::visualizeCompletion()
{
	data->flashEnabled = true;
}

void EditorState::endCompletionVisualization()
{
	data->flashEnabled = false;
}

void EditorState::exportData(const ExportOptions &options) const
{
	Exporter exporter;
	HeightmapData &heightmapData = data->terrainMode.loadHeightmap();

	exporter.getScene().setHeightmap(heightmapData.heightMap, heightmapData.mapSize, heightmapData.realSize);

	for(unsigned int i = 0; i < data->modes.size(); ++i)
		data->modes[i]->doExport(exporter);

	exporter.save(options);
}

filesystem::OutputStream &EditorState::writeStream(filesystem::OutputStream &stream) const
{
	for(unsigned int i = 0; i < data->modes.size(); ++i)
	{
		if(i >= 6)
			break;

		stream << *data->modes[i];
	}

	stream << int(2);
	writeColors(stream);

	stream << data->lightMode;

	return stream;
}

filesystem::InputStream &EditorState::readStream(filesystem::InputStream &stream)
{
	reset();

	for(unsigned int i = 0; i < data->modes.size(); ++i)
	{
		if(i >= 6)
			break;

		stream >> *data->modes[i];
	}

	int version = 0;
	stream >> version;

	readColors(stream);

	if(version > 1)
		stream >> data->lightMode;

	data->updateHeightmap = true;
	data->updateTexturing = true;
	data->updateShadows = true;

	return stream;
}

void EditorState::updateGrid(bool force)
{
	if (force || data->lastShowGridState != data->sceneMode.doesShowGrid())
	{
		data->lastShowGridState = data->sceneMode.doesShowGrid();
		data->unitMode.setGridUnitVisibility(data->lastShowGridState);
	}
}

void EditorState::updateTriggersVisibility(bool force)
{
	if (force || data->lastShowTriggersState != data->sceneMode.doesShowTriggers())
	{
		data->lastShowTriggersState = data->sceneMode.doesShowTriggers();
		data->unitMode.setTriggersVisibility(data->lastShowTriggersState);
	}
}

void EditorState::updateUnitsVisibility(bool force)
{
	if (force || data->lastShowUnitsState != data->sceneMode.doesShowUnits())
	{
		data->lastShowUnitsState = data->sceneMode.doesShowUnits();
		data->unitMode.setUnitsVisibility(data->lastShowUnitsState);
	}
}

void EditorState::updateHelpersVisibility(bool force)
{
	if (force || data->lastShowHelpersState != data->sceneMode.doesShowHelpers())
	{
		data->lastShowHelpersState = data->sceneMode.doesShowHelpers();
		data->objectMode.setHelpersVisibility(data->lastShowHelpersState);
	}
}

void EditorState::updateIncompleteVisibility(bool force)
{
	if (force || data->lastShowIncompleteState != data->sceneMode.doesShowIncomplete())
	{
		data->lastShowIncompleteState = data->sceneMode.doesShowIncomplete();
		data->objectMode.setIncompleteVisibility(data->lastShowIncompleteState);
	}
}

void EditorState::updateStaticVisibility(bool force)
{
	if (force || data->lastShowStaticState != data->sceneMode.doesShowStatic())
	{
		data->lastShowStaticState = data->sceneMode.doesShowStatic();
		data->objectMode.setStaticVisibility(data->lastShowStaticState);
	}
}

void EditorState::updateDynamicVisibility(bool force)
{
	if (force || data->lastShowDynamicState != data->sceneMode.doesShowDynamic())
	{
		data->lastShowDynamicState = data->sceneMode.doesShowDynamic();
		data->objectMode.setDynamicVisibility(data->lastShowDynamicState);
	}
}

void EditorState::updateNoCollisionVisibility(bool force)
{
	if (force || data->lastShowNoCollisionState != data->sceneMode.doesShowNoCollision())
	{
		data->lastShowNoCollisionState = data->sceneMode.doesShowNoCollision();
		data->objectMode.setNoCollisionVisibility(data->lastShowNoCollisionState);
	}
}

void EditorState::updateFolds(bool force)
{
	if (force || data->lastShowFoldsState != data->sceneMode.doesShowFolds())
	{
		data->lastShowFoldsState = data->sceneMode.doesShowFolds();
		// TODO: ...
	}
}

} // end of namespace editor
} // end of namespace frozenbyte
