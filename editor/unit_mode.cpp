// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "unit_mode.h"
#include "gui.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "command_list.h"
#include "icommand.h"
#include "unit_scripts.h"
#include "unit_hierarchy.h"
#include "terrain_units.h"
#include "storm.h"
#include "mouse.h"
#include "helper_visualization.h"
#include "string_properties.h"
#include "unit_properties_dialog.h"
#include "terrain_colormap.h"
#include "terrain_lightmap.h"
#include "ieditor_state.h"
#include "../ui/lightmanager.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "resource/resource.h"

#include <istorm3d.h>
#include <istorm3d_model.h>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace boost;

namespace frozenbyte {
namespace editor {
namespace {
	struct SharedData
	{
		Gui &gui;
		Dialog &dialog;
		IEditorState &editorState;

		Storm &storm;
		UnitScripts unitScripts;
		TerrainUnits terrainUnits;
		UnitHierarchy unitHierarchy;

		bool updateUnit;
		UnitHandle activeUnit;
		//std::string activeType;

		int configIndex;
		bool noUpdate;

		HelperVisualization helperVisualization;

		SharedData(Gui &gui_, IEditorState &editorState_, Storm &storm_)
		:	gui(gui_),
			dialog(gui.getUnitsDialog()),
			editorState(editorState_),
			storm(storm_),

			terrainUnits(storm, unitScripts, editorState),
			unitHierarchy(unitScripts),
			helperVisualization(storm)
		{
			noUpdate = false;
			reset();

			unitHierarchy.init();
			updateTypes(0, 0);
		}

		void updateTypes(int primary, int secondary)
		{
			int selectedIndex = SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_LIST, LB_GETCURSEL, 0, 0);
			if(selectedIndex >= 0)
				selectedIndex = unitHierarchy.getUnitScriptIndex(selectedIndex);

			if(!unitHierarchy.setGroups(primary, secondary))
				return;

			noUpdate = true;

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_PRIMARY, CB_RESETCONTENT, 0, 0);
			for(int p = 0; p < unitHierarchy.getPrimaryAmount(); ++p)
			{
				const std::string &str = unitHierarchy.getPrimaryName(p);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_PRIMARY, CB_ADDSTRING, 0, reinterpret_cast<long> (str.c_str()));
			}
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_PRIMARY, CB_SETCURSEL, primary, 0);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_SECONDARY, CB_RESETCONTENT, 0, 0);
			for(int s = 0; s < unitHierarchy.getSecondaryAmount(primary); ++s)
			{
				const std::string &str = unitHierarchy.getSecondaryName(primary, s);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_SECONDARY, CB_ADDSTRING, 0, reinterpret_cast<long> (str.c_str()));
			}
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_SECONDARY, CB_SETCURSEL, secondary, 0);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_LIST, LB_RESETCONTENT, 0, 0);
			for(int i = 0; i < unitHierarchy.getUnitScriptAmount(); ++i)
			{
				int index = unitHierarchy.getUnitScriptIndex(i);
				const std::string &string = unitScripts.getUnit(index).name;
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_LIST, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
			}

			if(selectedIndex >= 0)
			{
				selectedIndex = unitHierarchy.getUnitListIndex(selectedIndex);
				if(selectedIndex >= 0)
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_LIST, LB_SETCURSEL, selectedIndex, 0);
			}
			if(selectedIndex == -1)
			{
				activeUnit = UnitHandle();
				terrainUnits.setActiveUnit(activeUnit);
			}

			updateUnit = true;
			noUpdate = false;
		}

		void setConfig(UnitScripts::ScriptType type, const std::string &name, int itemId)
		{
			for(int i = 0; i < unitScripts.getScriptCount(type); ++i)
			{
				const std::string &string = unitScripts.getScript(type, i).name;
				if(string == name)
				{
					SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_SETCURSEL, i, 0);
					return;
				}
			}

			SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_SETCURSEL, 0, 0);
		}

		void setScriptConfig(const ScriptConfig &scripts)
		{
			setConfig(UnitScripts::Main, scripts.mainScript, IDC_UNIT_MAIN);
			setConfig(UnitScripts::Spotted, scripts.spottedScript, IDC_UNIT_SPOTTED);
			setConfig(UnitScripts::Alerted, scripts.alertedScript, IDC_UNIT_ALERTED);
			setConfig(UnitScripts::Hit, scripts.hitScript, IDC_UNIT_HIT);
			setConfig(UnitScripts::Hitmiss, scripts.hitmissScript, IDC_UNIT_HITMISS);
			setConfig(UnitScripts::Noise, scripts.noiseScript, IDC_UNIT_NOISE);
			setConfig(UnitScripts::Execute, scripts.executeScript, IDC_UNIT_EXECUTE);
			setConfig(UnitScripts::Special, scripts.specialScript, IDC_UNIT_SPECIAL);
			setConfig(UnitScripts::Pointed, scripts.pointedScript, IDC_UNIT_POINTED);
		}

		void setConfig()
		{
			int index = SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_GETCURSEL, 0, 0);
			if(index == configIndex)
				return;

			ScriptConfig config;

			if(index >= 0)
				config = unitScripts.getScriptConfig(index);

			setScriptConfig(config);
			configIndex = index;
		}

		std::string getConfig(UnitScripts::ScriptType type, int itemId)
		{
			int index = SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_GETCURSEL, 0, 0);
			if(index <= 0)
				return "";

			return unitScripts.getScript(type, index).name;
		}

		void findConfig()
		{
			for(int i = 0; i < unitScripts.getScriptConfigCount(); ++i)
			{
				const ScriptConfig &scripts = unitScripts.getScriptConfig(i);

				if(getConfig(UnitScripts::Main, IDC_UNIT_MAIN) == scripts.mainScript)
				if(getConfig(UnitScripts::Spotted, IDC_UNIT_SPOTTED) == scripts.spottedScript)
				if(getConfig(UnitScripts::Alerted, IDC_UNIT_ALERTED) == scripts.alertedScript)
				if(getConfig(UnitScripts::Hit, IDC_UNIT_HIT) == scripts.hitScript)
				if(getConfig(UnitScripts::Hitmiss, IDC_UNIT_HITMISS) == scripts.hitmissScript)
				if(getConfig(UnitScripts::Noise, IDC_UNIT_NOISE) == scripts.noiseScript)
				if(getConfig(UnitScripts::Execute, IDC_UNIT_EXECUTE) == scripts.executeScript)
				if(getConfig(UnitScripts::Special, IDC_UNIT_SPECIAL) == scripts.specialScript)
				if(getConfig(UnitScripts::Pointed, IDC_UNIT_POINTED) == scripts.pointedScript)
				{
					SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_SETCURSEL, i, 0);
					return;
				}
			}

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_SETCURSEL, -1, 0);
		}

		void setUnit(const UnitHandle &unitHandle)
		{
			activeUnit = unitHandle;
			terrainUnits.setActiveUnit(activeUnit);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_RESETCONTENT, 0, 0);
			helperVisualization.clear();

			if(!activeUnit.hasUnit())
			{
				enableDialogItem(dialog, IDC_UNIT_DELETE, false);
				enableDialogItem(dialog, IDC_UNIT_PROPERTIES, false);
				return;
			}

			ScriptConfig scripts = terrainUnits.getUnitScripts(activeUnit);
			setScriptConfig(scripts);

			std::string unitName = terrainUnits.getUnitName(activeUnit);
			int unitIndex = unitScripts.getUnitIndex(unitName);
			int fooIndex = unitHierarchy.getUnitListIndex(unitIndex);
			if(fooIndex == -1)
			{
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_PRIMARY, CB_SETCURSEL, 0, 0);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE_SECONDARY, CB_SETCURSEL, 0, 0);
				updateTypes(0, 0);
			}
			unitIndex = unitHierarchy.getUnitListIndex(unitIndex);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_LIST, LB_SETCURSEL, unitIndex, 0);

			findConfig();
			enableDialogItem(dialog, IDC_UNIT_DELETE, true);
			enableDialogItem(dialog, IDC_UNIT_PROPERTIES, true);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_SETCURSEL, terrainUnits.getUnitSide(activeUnit), 0);
			updateHelpers();
		}

		void updateHelpers()
		{
			std::string activeText;

			int index = SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETCURSEL, 0, 0);
			if(index >= 0)
			{
				int length = SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETTEXTLEN, index, 0);
				activeText.resize(length + 1);

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETTEXT, index, reinterpret_cast<LPARAM> (&activeText[0]));			
			}
			
			// Clear
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_RESETCONTENT, 0, 0);
			if(!activeUnit.hasUnit())
				return;

			// Set
			UnitHelpers helpers = terrainUnits.getUnitHelpers(activeUnit);
			for(int i = 0; i < helpers.getHelperAmount(); ++i)
			{
				const std::string &string = helpers.getHelperName(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
			}

			// Restore selection
			if(!activeText.empty())
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_SELECTSTRING, 0, reinterpret_cast<LPARAM> (activeText.c_str()));
		}

		void initScriptBox(UnitScripts::ScriptType type, int itemId)
		{
			SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_RESETCONTENT, 0, 0);
			for(int i = 0; i < unitScripts.getScriptCount(type); ++i)
			{
				const std::string &string = unitScripts.getScript(type, i).name;
				SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
			}

			SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_SETCURSEL, 0, 0);
		}

		void initDialog()
		{
			int i = 0;

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_RESETCONTENT, 0, 0);
			for(i = 0; i < unitScripts.getScriptConfigCount(); ++i)
			{
				const std::string &string = unitScripts.getScriptConfig(i).name;
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
			}

			initScriptBox(UnitScripts::Main, IDC_UNIT_MAIN);
			initScriptBox(UnitScripts::Spotted, IDC_UNIT_SPOTTED);
			initScriptBox(UnitScripts::Alerted, IDC_UNIT_ALERTED);
			initScriptBox(UnitScripts::Hit, IDC_UNIT_HIT);
			initScriptBox(UnitScripts::Hitmiss, IDC_UNIT_HITMISS);
			initScriptBox(UnitScripts::Noise, IDC_UNIT_NOISE);
			initScriptBox(UnitScripts::Execute, IDC_UNIT_EXECUTE);
			initScriptBox(UnitScripts::Special, IDC_UNIT_SPECIAL);
			initScriptBox(UnitScripts::Pointed, IDC_UNIT_POINTED);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Hostile"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Neutral"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Ally"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_SETCURSEL, 0, 0);

			std::vector<std::string> types = unitScripts.getTypes();
			for(unsigned int j = 0; j < types.size(); ++j)
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_UNIT_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (types[j].c_str()));
		}

		void findScript(UnitScripts::ScriptType type, int itemId, std::string &result)
		{
			int index = SendDlgItemMessage(dialog.getWindowHandle(), itemId, CB_GETCURSEL, 0, 0);
			if(index < 0)
			{
				result = "";
				return;
			}

			result = unitScripts.getScript(type, index).name;
		}

		ScriptConfig getScripts()
		{
			ScriptConfig result;

			findScript(UnitScripts::Main, IDC_UNIT_MAIN, result.mainScript);
			findScript(UnitScripts::Spotted, IDC_UNIT_SPOTTED, result.spottedScript);
			findScript(UnitScripts::Alerted, IDC_UNIT_ALERTED, result.alertedScript);
			findScript(UnitScripts::Hit, IDC_UNIT_HIT, result.hitScript);
			findScript(UnitScripts::Hitmiss, IDC_UNIT_HITMISS, result.hitmissScript);
			findScript(UnitScripts::Noise, IDC_UNIT_NOISE, result.noiseScript);
			findScript(UnitScripts::Execute, IDC_UNIT_EXECUTE, result.executeScript);
			findScript(UnitScripts::Special, IDC_UNIT_SPECIAL, result.specialScript);
			findScript(UnitScripts::Pointed, IDC_UNIT_POINTED, result.pointedScript);

			return result;
		}

		void setLighting(IStorm3D_Model *model)
		{
			if(!model)
				return;

			VC2 position(model->GetPosition().x, model->GetPosition().z);
			/*
			COL tmpColor = editorState.getColorMap().getColor(position) + editorState.getLightMap().getColor(position);
			VC3 lightPos;
			COL lightCol;
			COL ambient;
			float range = 1.f;

			if(storm.lightManager)
				storm.lightManager->getLighting(tmpColor, model->GetPosition(), lightPos, lightCol, range, ambient);

			model->SetSelfIllumination(ambient);
			model->SetLighting(lightPos, lightCol, range);
			*/

			ui::PointLights lights;
			lights.ambient = editorState.getColorMap().getColor(position) + editorState.getLightMap().getColor(position);
			if(storm.lightManager)
				storm.lightManager->getLighting(model->GetPosition(), lights, ui::getRadius(model), true, true,
					model);

			model->SetSelfIllumination(lights.ambient);
			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				model->SetLighting(i, lights.lightIndices[i]);

			model->useAlwaysDirectional(true);

			if(!storm.onFloor(position))
				model->SetDirectional(editorState.getSunDirection(), 1.f);
			else
				model->SetDirectional(VC3(), 0.f);
		}

		void reset()
		{
			setUnit(UnitHandle());
			configIndex = -1;

			unitScripts.reload();
			terrainUnits.clear();
			initDialog();
		}
	};

	class UnitCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		UnitCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_LIST, this);		
		}

		void execute(int id)
		{
			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_LIST, LB_GETCURSEL, 0, 0);
			if(sharedData.activeUnit.hasUnit())
			{
				if(index >= 0)
				{
					index = sharedData.unitHierarchy.getUnitScriptIndex(index);

					assert(index >= 0);
					sharedData.activeUnit = sharedData.terrainUnits.changeUnitType(sharedData.activeUnit, index);
				}
			}
			else
			{
				if(index >= 0)
				{
					int scriptIndex = sharedData.unitHierarchy.getUnitScriptIndex(index);
					Unit &u = sharedData.terrainUnits.getUnitSettings(scriptIndex);

					if(!u.defaultConfiguration.empty())
					{
						const std::string &str = u.defaultConfiguration;
						const char *f = str.c_str();
						int confIndex = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_FINDSTRINGEXACT, -1, reinterpret_cast<LPARAM> (f));
						if(confIndex >= 0)
							SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_SETCURSEL, confIndex, 0);

						sharedData.noUpdate = true;
						sharedData.setConfig();
						sharedData.noUpdate = false;
					}
				}
			}

			sharedData.updateUnit = true;
		}
	};

	struct HelperCommand: public ICommand
	{
		SharedData &sharedData;
		int currentIndex;

		HelperCommand(SharedData &sharedData_)
		:	sharedData(sharedData_),
			currentIndex(-1)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_HELPERS, this);
		}

		void enableDialogs(bool enable)
		{
			enableDialogItem(sharedData.dialog, IDC_POINT_INSERT, enable);
			enableDialogItem(sharedData.dialog, IDC_POINT_DELETE, enable);
			enableDialogItem(sharedData.dialog, IDC_POINT_RESET, enable);
		}

		void update()
		{
			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETCURSEL, 0, 0);
			if(sharedData.activeUnit.hasUnit())
			{
				UnitHelpers &helpers = sharedData.terrainUnits.getUnitHelpers(sharedData.activeUnit);
				sharedData.helperVisualization.visualize(helpers, index, sharedData.activeUnit.getPosition());
			}

			if(index < 0)
			{
				enableDialogs(false);
				return;
			}
			else
				enableDialogs(true);

			UnitHelpers &helpers = sharedData.terrainUnits.getUnitHelpers(sharedData.activeUnit);
			int pointAmount = helpers.getPointAmount(index);

			if(!helpers.isWayPoint(index) && pointAmount)
			{
				enableDialogItem(sharedData.dialog, IDC_POINT_INSERT, false);
				CheckDlgButton(sharedData.dialog.getWindowHandle(), IDC_POINT_INSERT, BST_UNCHECKED);
			}

			if(pointAmount == 0)
			{
				enableDialogItem(sharedData.dialog, IDC_POINT_DELETE, false);
				enableDialogItem(sharedData.dialog, IDC_POINT_RESET, false);
			}
		}

		void addPoint(const VC3 &position)
		{
			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETCURSEL, 0, 0);
			if(index < 0 || IsDlgButtonChecked(sharedData.dialog.getWindowHandle(), IDC_POINT_INSERT) != BST_CHECKED)
				return;

			UnitHelpers &helpers = sharedData.terrainUnits.getUnitHelpers(sharedData.activeUnit);
			helpers.addPoint(index, VC2(position.x, position.z));

			sharedData.helperVisualization.visualize(helpers, index, sharedData.activeUnit.getPosition());
			update();
		}

		void execute(int id)
		{
			update();
		}
	};


	class ConfigCommand: public ICommand
	{
		SharedData &sharedData;
		HelperCommand &helperCommand;

	public:
		ConfigCommand(SharedData &sharedData_, HelperCommand &helperCommand_)
		:	sharedData(sharedData_),
			helperCommand(helperCommand_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_CONFIGURATION, this);
		}

		void execute(int id)
		{
			if(sharedData.noUpdate)
				return;

			sharedData.noUpdate = true;
			sharedData.setConfig();
			sharedData.noUpdate = false;

			sharedData.terrainUnits.setUnitScripts(sharedData.activeUnit, sharedData.getScripts());
			sharedData.updateHelpers();
			helperCommand.update();
		}
	};

	class ScriptCommand: public ICommand
	{
		SharedData &sharedData;
		HelperCommand &helperCommand;

	public:
		ScriptCommand(SharedData &sharedData_, HelperCommand &helperCommand_)
		:	sharedData(sharedData_),
			helperCommand(helperCommand_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_MAIN, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_SPOTTED, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_ALERTED, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_HIT, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_HITMISS, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_NOISE, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_EXECUTE, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_SPECIAL, this);
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_POINTED, this);
		}

		void execute(int id)
		{
			if(sharedData.noUpdate)
				return;

			sharedData.noUpdate = true;
			sharedData.terrainUnits.setUnitScripts(sharedData.activeUnit, sharedData.getScripts());
			sharedData.findConfig();
			sharedData.noUpdate = false;

			sharedData.updateHelpers();
			helperCommand.update();
		}
	};

	class DeleteCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		DeleteCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_DELETE, this);
		}

		void execute(int id)
		{
			if(!sharedData.activeUnit.hasUnit())
				return;

			sharedData.terrainUnits.removeUnit(sharedData.activeUnit);
			sharedData.setUnit(UnitHandle());
			
			SetFocus(sharedData.gui.getRenderDialog().getWindowHandle());
		}
	};

	class PropertiesCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		PropertiesCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_PROPERTIES, this);
		}

		void execute(int id)
		{
			if(!sharedData.activeUnit.hasUnit())
				return;

			UnitProperties &properties = sharedData.terrainUnits.getUnitProperties(sharedData.activeUnit);
			std::vector<std::string> usedStrings = sharedData.terrainUnits.getUnitUsedProperties(sharedData.activeUnit);
			StringProperties strings = sharedData.terrainUnits.getUnitStringProperties(sharedData.activeUnit);

			UnitPropertiesDialog dialog(properties, usedStrings, strings);
			dialog.execute(id);
		}
	};

	class DeletePointCommand: public ICommand
	{
		SharedData &sharedData;
		HelperCommand &helperCommand;

	public:
		DeletePointCommand(SharedData &sharedData_, HelperCommand &helperCommand_)
		:	sharedData(sharedData_),
			helperCommand(helperCommand_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_POINT_DELETE, this);
		}

		void execute(int id)
		{
			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETCURSEL, 0, 0);
			UnitHelpers &helpers = sharedData.terrainUnits.getUnitHelpers(sharedData.activeUnit);
			
			if(index < 0)
				return;

			int pointAmount = helpers.getPointAmount(index);
			if(!pointAmount)
				return;

			helpers.deletePoint(index, pointAmount - 1);
			helperCommand.update();
		}
	};

	class ResetPointCommand: public ICommand
	{
		SharedData &sharedData;
		HelperCommand &helperCommand;

	public:
		ResetPointCommand(SharedData &sharedData_, HelperCommand &helperCommand_)
		:	sharedData(sharedData_),
			helperCommand(helperCommand_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_POINT_RESET, this);
		}

		void execute(int id)
		{
			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_HELPERS, LB_GETCURSEL, 0, 0);
			UnitHelpers &helpers = sharedData.terrainUnits.getUnitHelpers(sharedData.activeUnit);
			
			if(index < 0)
				return;

			int pointAmount = helpers.getPointAmount(index);
			while(pointAmount)
				helpers.deletePoint(index, --pointAmount);

			helperCommand.update();
		}
	};

	class SideCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		SideCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			sharedData.dialog.getCommandList().addCommand(IDC_UNIT_SIDE, this);
		}

		void execute(int id)
		{
			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_SIDE, CB_GETCURSEL, 0, 0);
			sharedData.terrainUnits.setUnitSide(sharedData.activeUnit, index);
		}
	};

	class MouseTracker
	{
		SharedData &sharedData;
		Mouse &mouse;

		boost::shared_ptr<IStorm3D_Model> model;
		float modelHeight;
		float height;
		float yRotation;

		HelperCommand &helperCommand;

	public:
		MouseTracker(SharedData &sharedData_, Mouse &mouse_, HelperCommand &helperCommand_, SideCommand &sideCommand_)
		:	sharedData(sharedData_), 
			mouse(mouse_),
			helperCommand(helperCommand_),

			modelHeight(0),
			height(0),
			yRotation(0)
		{
		}

		void updateUnit()
		{
			if(model)
				sharedData.storm.scene->RemoveModel(model.get());

			//height = 0;
			//modelHeight = 0;

			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_LIST, LB_GETCURSEL, 0, 0);
			if(index < 0)
			{
				model.reset();
				return;
			}

			index = sharedData.unitHierarchy.getUnitScriptIndex(index);
			model = sharedData.terrainUnits.getUnitModel(index);
			if(model)
				sharedData.updateUnit = false;
		}

		void updateHeight()
		{
			modelHeight = height;
			int direction = 0;
			if(GetKeyState(VK_PRIOR) & 0x80)
				direction = 1;
			if(GetKeyState(VK_NEXT) & 0x80)
				direction = -1;

			modelHeight = sharedData.storm.unitAligner.getHeight(height, direction);
			height = modelHeight;
		}

		void tick()
		{
			if(sharedData.updateUnit)
				updateUnit();

			if(model)
				sharedData.storm.scene->RemoveModel(model.get());
			if(!mouse.isInsideWindow())
				return;

			Vector p, d;
			Storm3D_CollisionInfo ci;
			if(!mouse.cursorRayTrace(ci, &p, &d))
				return;

			UnitHandle unitHandle = sharedData.terrainUnits.traceCursor(p, d, 200.f);
			if(sharedData.activeUnit.hasUnit())
			{
				VC2 pos2 = sharedData.activeUnit.getPosition();
				VC3 pos(pos2.x, 0, pos2.y);
				pos = sharedData.storm.unitAligner.getMovedPosition(pos, *sharedData.storm.scene->GetCamera());

				/*
				if((GetKeyState(VK_CONTROL) & 0x80) && unitHandle.hasUnit())
				{
					pos.x = unitHandle.getPosition().x;
					pos.z = unitHandle.getPosition().y;
				}
				*/

				sharedData.terrainUnits.setHeight(sharedData.activeUnit, modelHeight);
				sharedData.terrainUnits.rotateUnit(sharedData.activeUnit, mouse.getWheelDelta());
				sharedData.terrainUnits.setPosition(sharedData.activeUnit, pos);
			}

			updateHeight();

			if(unitHandle.hasUnit())
			{
				if(mouse.hasLeftClicked())
				{
					if(sharedData.activeUnit == unitHandle)
						sharedData.setUnit(UnitHandle());
					else
					{
						sharedData.setUnit(unitHandle);
						updateUnit();
					}

					height = sharedData.activeUnit.getHeight();
					helperCommand.update();
				}

				if(GetKeyState(VK_DELETE) & 0x80)
				{
					sharedData.terrainUnits.removeUnit(sharedData.activeUnit);
					sharedData.setUnit(UnitHandle());
					SetFocus(sharedData.gui.getRenderDialog().getWindowHandle());
				}

				return;
			}

			if(sharedData.activeUnit.hasUnit() && mouse.hasLeftClicked())
			{
				helperCommand.addPoint(ci.position);
				return;
			}

			if(!model)
				return;

			ci.position.y = sharedData.storm.getHeight(VC2(ci.position.x, ci.position.z)) + modelHeight;
			yRotation = sharedData.storm.unitAligner.getRotation(yRotation, mouse.getWheelDelta(), true);

			int index = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_LIST, LB_GETCURSEL, 0, 0);
			if(index >= 0)
			{
				index = sharedData.unitHierarchy.getUnitScriptIndex(index);
				Unit &u = sharedData.terrainUnits.getUnitSettings(index);
				if(u.disableRotation)
					yRotation = 0;
			}

			model->SetPosition(ci.position);
			QUAT rotation;
			rotation.MakeFromAngles(0, yRotation, 0);
			model->SetRotation(rotation);
			sharedData.setLighting(model.get());

			if(!sharedData.activeUnit.hasUnit())
			{
				sharedData.storm.scene->AddModel(model.get());
				if(mouse.hasLeftClicked())
				{
					UnitHandle handle = sharedData.terrainUnits.addUnit(index, ci.position, yRotation, height);
					Unit unit = sharedData.terrainUnits.getUnit(handle);
					sharedData.terrainUnits.setUnitSide(handle, unit.defaultSide);

					if(!unit.defaultConfiguration.empty())
					{
						const std::string &str = unit.defaultConfiguration;
						const char *f = str.c_str();
						int confIndex = SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_FINDSTRINGEXACT, -1, reinterpret_cast<LPARAM> (f));
						if(confIndex >= 0)
							SendDlgItemMessage(sharedData.dialog.getWindowHandle(), IDC_UNIT_CONFIGURATION, CB_SETCURSEL, confIndex, 0);

						sharedData.noUpdate = true;
						sharedData.setConfig();
						sharedData.noUpdate = false;
					}

					sharedData.terrainUnits.setUnitScripts(handle, sharedData.getScripts());
				}
			}
		}

		void reset()
		{
			model.reset();
		}
	};

	class GroupCommand: public ICommand
	{
		SharedData &data;

	public:
		GroupCommand(SharedData &sharedData)
		:	data(sharedData)
		{
			data.dialog.getCommandList().addCommand(IDC_UNIT_TYPE_PRIMARY, this);
			data.dialog.getCommandList().addCommand(IDC_UNIT_TYPE_SECONDARY, this);
		}

		void execute(int id)
		{
			if(data.noUpdate)
				return;

			int indexP = SendDlgItemMessage(data.dialog.getWindowHandle(), IDC_UNIT_TYPE_PRIMARY, CB_GETCURSEL, 0, 0);
			int indexS = SendDlgItemMessage(data.dialog.getWindowHandle(), IDC_UNIT_TYPE_SECONDARY, CB_GETCURSEL, 0, 0);

			if(indexS >= data.unitHierarchy.getSecondaryAmount(indexP))
			{
				SendDlgItemMessage(data.dialog.getWindowHandle(), IDC_UNIT_TYPE_SECONDARY, CB_SETCURSEL, 0, 0);
				indexS = 0;
			}

			data.updateTypes(indexP, indexS);
		}
	};
}

struct UnitModeData
{
	Gui &gui;
	Storm &storm;

	SharedData sharedData;
	UnitCommand unitCommand;
	HelperCommand helperCommand;

	ConfigCommand configCommand;
	ScriptCommand scriptCommand;
	DeleteCommand deleteCommand;
	PropertiesCommand propertiesCommand;
	SideCommand sideCommand;

	DeletePointCommand deletePointCommand;
	ResetPointCommand resetPointCommand;
	MouseTracker mouseTracker;
	GroupCommand groupCommand;

	UnitModeData(Gui &gui_, Storm &storm_, IEditorState &editorState)
	:	gui(gui_),
		storm(storm_),

		sharedData(gui, editorState, storm),
		unitCommand(sharedData),
		helperCommand(sharedData),

		configCommand(sharedData, helperCommand),
		scriptCommand(sharedData, helperCommand),
		deleteCommand(sharedData),
		propertiesCommand(sharedData),
		sideCommand(sharedData),

		deletePointCommand(sharedData, helperCommand),
		resetPointCommand(sharedData, helperCommand),
		mouseTracker(sharedData, gui.getMouse(), helperCommand, sideCommand),
		groupCommand(sharedData)
	{
	}
};

UnitMode::UnitMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	boost::scoped_ptr<UnitModeData> tempData(new UnitModeData(gui, storm, editorState));
	data.swap(tempData);
}

UnitMode::~UnitMode()
{
}

void UnitMode::tick()
{
	data->mouseTracker.tick();
}

void UnitMode::reset()
{
	data->sharedData.reset();
	data->mouseTracker.reset();
}

void UnitMode::update()
{
	data->sharedData.terrainUnits.setToTerrain();
}

void UnitMode::hideObjects()
{
	data->sharedData.terrainUnits.hideUnits();
}

void UnitMode::showObjects()
{
	data->sharedData.terrainUnits.showUnits();
}

void UnitMode::updateLighting()
{
	data->sharedData.terrainUnits.updateLighting();
}

void UnitMode::setGridUnitVisibility(bool gridVisible)
{
	if (gridVisible)
		data->sharedData.terrainUnits.showGridUnits();
	else
		data->sharedData.terrainUnits.hideGridUnits();
}

void UnitMode::setTriggersVisibility(bool triggersVisible)
{
	if (triggersVisible)
		data->sharedData.terrainUnits.showTriggerUnits();
	else
		data->sharedData.terrainUnits.hideTriggerUnits();
}

void UnitMode::setUnitsVisibility(bool unitsVisible)
{
	if (unitsVisible)
		data->sharedData.terrainUnits.showNormalUnits();
	else
		data->sharedData.terrainUnits.hideNormalUnits();
}

void UnitMode::doExport(Exporter &exporter) const
{
	data->sharedData.terrainUnits.doExport(exporter);
}

filesystem::OutputStream &UnitMode::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(1);
	stream << data->sharedData.terrainUnits;
	return stream;
}

filesystem::InputStream &UnitMode::readStream(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;
	if(version == 0)
		return stream;

	stream >> data->sharedData.terrainUnits;
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
