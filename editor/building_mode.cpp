// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "building_mode.h"
#include "terrain_buildings.h"
#include "icommand.h"
#include "command_list.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "gui.h"
#include "common_dialog.h"
#include "storm.h"
#include "storm_model_utils.h"
#include "mouse.h"
#include "ieditor_state.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../util/mod_selector.h"

#include <map>
#include <vector>
#include <istorm3d.h>
#include <istorm3d_model.h>
#include "resource/resource.h"

namespace frozenbyte {
namespace editor {

extern util::ModSelector modSelector;

namespace {
	struct SharedData
	{
		Storm &storm;
		Dialog &dialog;
		IEditorState &editorState;

		TerrainBuildings terrainBuildings;

		int currentMode;
		bool updateList;

		SharedData(Storm &storm_, Gui &gui, IEditorState &editorState_)
		:	storm(storm_),
			dialog(gui.getBuildingsDialog()),
			editorState(editorState_),

			terrainBuildings(storm_, gui.getMouse(), editorState),
			currentMode(0)
		{
			reset();
		}

		void updateDialog()
		{
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_BUILDING_MODELS, LB_RESETCONTENT, 0, 0);

			for(int i = 0; i < terrainBuildings.getModelCount(); ++i)
			{
				std::string model = getFileName(terrainBuildings.getModel(i));
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_BUILDING_MODELS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (model.c_str()));
			}
		}

		void enableProperties(bool enable)
		{
			enableDialogItem(dialog, IDC_CUT_TERRAIN, enable);
		}

		void updateProperties()
		{
			int selection = getSelection();
			if(selection == -1)
			{
				enableProperties(false);
				return;
			}

			enableProperties(true);
			bool cutTerrain = terrainBuildings.hasCutTerrain(selection);
			int cutFlag = cutTerrain ? BST_CHECKED : BST_UNCHECKED;

			CheckDlgButton(dialog.getWindowHandle(), IDC_CUT_TERRAIN, cutFlag);
		}

		void reset()
		{
			terrainBuildings.clear();
			updateDialog();

			CheckDlgButton(dialog.getWindowHandle(), IDC_BUILDING_REMOVEMODE, BST_UNCHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_BUILDING_INSERTMODE, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_HIDE_ROOF, BST_UNCHECKED); 
		}

		int getSelection()
		{
			return SendDlgItemMessage(dialog.getWindowHandle(), IDC_BUILDING_MODELS, LB_GETCURSEL, 0, 0);
		}
	};

	class BuildingCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		BuildingCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BUILDING_MODELS, this);
		}

		void execute(int id)
		{
			sharedData.updateList = true;
		}
	};

	class AddBuildingCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		AddBuildingCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BUILDING_ADD, this);
		}

		void execute(int id)
		{
			modSelector.restoreDir();

			std::vector<std::string> fileNames = getMultipleOpenFileName("s3d", "Data\\Models");
			if(!fileNames.empty())
			{
				for(unsigned int i = 0; i < fileNames.size(); ++i)
				{
					if(!fileNames[i].empty())
					{
						modSelector.fixFileName(fileNames[i]);
						sharedData.terrainBuildings.addModel(fileNames[i]);
					}
				}

				sharedData.updateDialog();
				sharedData.updateList = true;
			}

			modSelector.changeDir();
		}
	};

	class RemoveBuildingCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		RemoveBuildingCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BUILDING_REMOVE, this);
		}

		void execute(int id)
		{
			int selection = sharedData.getSelection();
			if(selection >= 0)
			{
				sharedData.terrainBuildings.removeModel(selection);
				sharedData.updateDialog();
				sharedData.updateList = true;
			}
		}
	};

	class BuildingPropertiesCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		BuildingPropertiesCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_CUT_TERRAIN, this);
		}

		void execute(int id)
		{
			int selection = sharedData.getSelection();
			if(selection >= 0)
			{
				int cutFlag = IsDlgButtonChecked(sharedData.dialog.getWindowHandle(), IDC_CUT_TERRAIN);
				bool cutTerrain = true;
				if(cutFlag == BST_UNCHECKED)
					cutTerrain = false;

				sharedData.terrainBuildings.setCutTerrain(selection, cutTerrain);
			}
		}
	};

	class InsertModeCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		InsertModeCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BUILDING_INSERTMODE, this);
		}

		void execute(int id)
		{
			sharedData.currentMode = 0;
		}
	};

	class RemoveModeCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		RemoveModeCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BUILDING_REMOVEMODE, this);
		}

		void execute(int id)
		{
			sharedData.currentMode = 1;
		}
	};

	class HideCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		HideCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_HIDE_ROOF, this);
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(dialog.getWindowHandle(), IDC_HIDE_ROOF) == BST_CHECKED)
				sharedData.terrainBuildings.hideRoofs(true);
			else
				sharedData.terrainBuildings.hideRoofs(false);
		}
	};

	/* Modes */

	class InsertBuildingMode
	{
		SharedData &sharedData;
		Mouse &mouse;

		boost::shared_ptr<IStorm3D_Model> model;
		float rotation;

		void loadModel()
		{
			int selection = sharedData.getSelection();
			if(selection < 0)
			{
				deleteModel();
				return;
			}

			boost::shared_ptr<IStorm3D_Model> newModel = createEditorModel(*sharedData.storm.storm, sharedData.terrainBuildings.getModel(selection));
			model = newModel;

			rotation = 0;
		}

		void setPosition(Vector &cursor)
		{
			model->SetPosition(cursor);
		}

		void rotateModel(int yDelta)
		{
			rotation = sharedData.storm.unitAligner.getRotation(rotation, yDelta);

			QUAT q;
			q.MakeFromAngles(0, rotation, 0);
			model->SetRotation(q);
		}

		void deleteModel()
		{
			sharedData.storm.scene->RemoveModel(model.get());

			boost::shared_ptr<IStorm3D_Model> emptyModel;
			model = emptyModel;
		}

	public:
		InsertBuildingMode(SharedData &sharedData_, Mouse &mouse_)
		:	sharedData(sharedData_),
			mouse(mouse_)
		{
			rotation = 0;
		}

		~InsertBuildingMode()
		{
		}

		void modeTick()
		{
			if(model)
				sharedData.storm.scene->RemoveModel(model.get());

			if(!model)
				loadModel();
			if(!model)
				return;
			if(!mouse.isInsideWindow())
				return;

			int mouseDelta = mouse.getWheelDelta();
			if(mouseDelta)
				rotateModel(mouseDelta);

			Storm3D_CollisionInfo ci;
			if(!mouse.cursorRayTrace(ci))
				return;

			sharedData.storm.unitAligner.getAlignedPosition(ci.position);

			setPosition(ci.position);
			sharedData.storm.scene->AddModel(model.get());

			if(mouse.hasLeftClicked())
			{
				int index = sharedData.getSelection();

				VC2 position(ci.position.x, ci.position.z);
				sharedData.terrainBuildings.addBuilding(index, position, rotation);

				sharedData.editorState.updateHeightmap();
				sharedData.editorState.updateShadows();
			}
		}

		void update()
		{
			deleteModel();
			loadModel();
		}

		void reset()
		{
			deleteModel();
		}
	};

	class RemoveBuildingMode
	{
		SharedData &sharedData;
		Mouse &mouse;

	public:
		RemoveBuildingMode(SharedData &sharedData_, Mouse &mouse_)
		:	sharedData(sharedData_),
			mouse(mouse_)
		{
		}

		void modeTick()
		{
			if(!mouse.isInsideWindow())
				return;

			Vector p, d;
			Storm3D_CollisionInfo ci;
			float rayLength = 1000.f;
			if(mouse.cursorRayTrace(ci, &p, &d))
				rayLength = p.GetRangeTo(ci.position);

			Building building = sharedData.terrainBuildings.traceActiveCollision(p, d, 1000.f);

			int wheelDelta = mouse.getWheelDelta();
			if(wheelDelta)
			{
				sharedData.terrainBuildings.rotateObject(building, wheelDelta);
				sharedData.editorState.updateHeightmap();
			}

			VC3 pos = sharedData.storm.unitAligner.getMovedPosition(VC3(), *sharedData.storm.scene->GetCamera());
			sharedData.terrainBuildings.moveObject(building, VC2(pos.x, pos.z));

			if(mouse.hasLeftClicked())
			{
				sharedData.terrainBuildings.removeObject(building);
				sharedData.editorState.updateHeightmap();
			}
		}
	};

} // unnamed

struct BuildingModeData
{
	SharedData sharedData;
	Dialog &dialog;

	BuildingCommand buildingCommand;
	AddBuildingCommand addBuildingCommand;
	RemoveBuildingCommand removeBuildingCommand;
	BuildingPropertiesCommand buildingPropertiesCommand;

	InsertModeCommand insertModeCommand;
	RemoveModeCommand removeModeCommand;

	InsertBuildingMode insertBuildingMode;
	RemoveBuildingMode removeBuildingMode;
	HideCommand hideCommand;

	BuildingModeData(Storm &storm, Gui &gui, IEditorState &editorState)
	:	sharedData(storm, gui, editorState),
		dialog(gui.getBuildingsDialog()),

		buildingCommand(sharedData, dialog),
		addBuildingCommand(sharedData, dialog),
		removeBuildingCommand(sharedData, dialog),
		buildingPropertiesCommand(sharedData, dialog),

		insertModeCommand(sharedData, dialog),
		removeModeCommand(sharedData, dialog),

		insertBuildingMode(sharedData, gui.getMouse()),
		removeBuildingMode(sharedData, gui.getMouse()),

		hideCommand(sharedData, dialog)
	{
	}
};

BuildingMode::BuildingMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	boost::scoped_ptr<BuildingModeData> tempData(new BuildingModeData(storm, gui, editorState));
	data.swap(tempData);
}

BuildingMode::~BuildingMode()
{
}

void BuildingMode::tick()
{
	if(data->sharedData.updateList)
	{
		data->sharedData.updateList = false;
		data->sharedData.updateProperties();
		data->insertBuildingMode.update();
	}

	if(data->sharedData.currentMode == 0)
		data->insertBuildingMode.modeTick();
	else
		data->removeBuildingMode.modeTick();
}

void BuildingMode::update()
{
	data->sharedData.terrainBuildings.setToTerrain();
}

void BuildingMode::reset()
{
	data->sharedData.reset();
	data->insertBuildingMode.reset();
}

void BuildingMode::updateLighting()
{
	data->sharedData.terrainBuildings.updateLighting();
}

void BuildingMode::roofCollision(bool collision)
{
	data->sharedData.terrainBuildings.roofCollision(collision);
}

void BuildingMode::doExport(Exporter &exporter) const
{
	data->sharedData.terrainBuildings.doExport(exporter);
}

filesystem::OutputStream &BuildingMode::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(0);
	stream << data->sharedData.terrainBuildings;
	return stream;
}

filesystem::InputStream &BuildingMode::readStream(filesystem::InputStream &stream)
{
	reset();

	int version = 0;
	stream >> version;

	stream >> data->sharedData.terrainBuildings;
	data->sharedData.updateDialog();

	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
