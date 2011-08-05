// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "decorator_mode.h"
#include "terrain_decorators.h"
#include "storm.h"
#include "gui.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "common_dialog.h"
#include "ieditor_state.h"
#include "icommand.h"
#include "command_list.h"
#include "string_conversions.h"
#include "../util/parser.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"

#include "resource/resource.h"

namespace frozenbyte {
namespace editor {
namespace {
	struct SharedData
	{
		Gui &gui;
		Storm &storm;
		IEditorState &editorState;
		Dialog &dialog;

		bool noDialogUpdate;

		//ui::TerrainLegacy terrainLegacy;
		std::string legacyName;

		TerrainDecorators terrainDecorators;

		SharedData(Gui &gui_, Storm &storm_, IEditorState &editorState_)
		:	gui(gui_),
			storm(storm_),
			editorState(editorState_),
			dialog(gui.getDecoratorsDialog()),
			terrainDecorators(storm)
		{
			noDialogUpdate = false;
			reset();
		}

		void updateDialog()
		{
			noDialogUpdate = true;

			if(legacyName.empty())
				setDialogItemText(dialog, IDC_LOAD_LEGACY, "Load");
			else
				setDialogItemText(dialog, IDC_LOAD_LEGACY, getFileName(legacyName));

			const std::string &waterName1 = terrainDecorators.getWaterName(0);
			if(waterName1.empty())
				setDialogItemText(dialog, IDC_WATERMODEL_1, "Model 1");
			else
				setDialogItemText(dialog, IDC_WATERMODEL_1, getFileName(waterName1));

			const std::string &waterName2 = terrainDecorators.getWaterName(1);
			if(waterName2.empty())
				setDialogItemText(dialog, IDC_WATERMODEL_2, "Model 2");
			else
				setDialogItemText(dialog, IDC_WATERMODEL_2, getFileName(waterName2));

			noDialogUpdate = false;
		}

		void setDialogWater()
		{
			float height1 = terrainDecorators.getWaterHeight(0);
			float height2 = terrainDecorators.getWaterHeight(0);

			setDialogItemText(gui.getDecoratorsDialog(), IDC_WATERHEIGHT_1, convertToString<float> (height1));
			setDialogItemText(gui.getDecoratorsDialog(), IDC_WATERHEIGHT_2, convertToString<float> (height2));
		}

		void setLegacy(const std::string &fileName)
		{
			if(noDialogUpdate)
				return;

			legacyName = fileName;
			editorState.updateTexturing();
			updateDialog();
		}

		void reset()
		{
			legacyName = "";
			//terrainLegacy.clear();
			terrainDecorators.reset();

			setDialogWater();
			updateDialog();
		}
	};

	class LegacyCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		LegacyCommand(SharedData &sharedData_, Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_LOAD_LEGACY, this);
		}

		void execute(int id)
		{
			std::string fileName = getOpenFileName("txt", "Data\\Missions");
			if(!fileName.empty())
				sharedData.setLegacy(fileName);
		}
	};

	class WaterModelCommand: public ICommand
	{
		SharedData &sharedData;
		int index;

	public:
		WaterModelCommand(SharedData &sharedData_, int index_, Dialog &dialog)
		:	sharedData(sharedData_),
			index(index_)
		{
			if(index == 0)
				dialog.getCommandList().addCommand(IDC_WATERMODEL_1, this);
			else
				dialog.getCommandList().addCommand(IDC_WATERMODEL_2, this);
		}

		void execute(int id)
		{
			std::string fileName = getOpenFileName("s3d", "Data\\Models");
			if(fileName.empty())
				return;

			sharedData.terrainDecorators.setWaterModel(index, fileName);
			sharedData.updateDialog();
		}
	};

	class WaterHeightCommand: public ICommand
	{
		SharedData &sharedData;
		int index;

		Dialog &dialog;

	public:
		WaterHeightCommand(SharedData &sharedData_, int index_, Dialog &dialog_)
		:	sharedData(sharedData_),
			index(index_),
			dialog(dialog_)
		{
			if(index == 0)
				dialog.getCommandList().addCommand(IDC_WATERHEIGHT_1, this);
			else
				dialog.getCommandList().addCommand(IDC_WATERHEIGHT_2, this);
		}

		void execute(int id)
		{
			float height = 0;
			if(index == 0)
				height = convertFromString<float> (getDialogItemText(dialog, IDC_WATERHEIGHT_1), 0);
			else
				height = convertFromString<float> (getDialogItemText(dialog, IDC_WATERHEIGHT_2), 0);

			sharedData.terrainDecorators.setWaterHeight(index, height);
		}
	};
} // unnamed

struct DecoratorModeData
{
	SharedData sharedData;
	LegacyCommand legacyCommand;

	WaterModelCommand waterModelCommand1;
	WaterModelCommand waterModelCommand2;
	WaterHeightCommand waterHeightCommand1;
	WaterHeightCommand waterHeightCommand2;

	DecoratorModeData(Gui &gui, Storm &storm, IEditorState &editorState)
	:	sharedData(gui, storm, editorState),
		legacyCommand(sharedData, gui.getDecoratorsDialog()),

		waterModelCommand1(sharedData, 0, gui.getDecoratorsDialog()),
		waterModelCommand2(sharedData, 1, gui.getDecoratorsDialog()),
		waterHeightCommand1(sharedData, 0, gui.getDecoratorsDialog()),
		waterHeightCommand2(sharedData, 1, gui.getDecoratorsDialog())
	{
	}
};

DecoratorMode::DecoratorMode(Gui &gui, Storm &storm, IEditorState &editorState)
{
	boost::scoped_ptr<DecoratorModeData> tempData(new DecoratorModeData(gui, storm, editorState));
	data.swap(tempData);
}

DecoratorMode::~DecoratorMode()
{
}
	
void DecoratorMode::tick()
{
}

void DecoratorMode::update()
{
}

void DecoratorMode::reset()
{
	data->sharedData.reset();
}

void DecoratorMode::guiTick()
{
	data->sharedData.terrainDecorators.tick();
}

void DecoratorMode::setTerrainLegacy()
{
	if(data->sharedData.legacyName.size() < 3)
		return;

	//Parser::Parser parser(data->sharedData.legacyName.c_str());
	//data->sharedData.terrainLegacy.apply(parser.FindGroup("Paint"));
}

void DecoratorMode::doExport(Exporter &exporter) const
{
}

filesystem::OutputStream &DecoratorMode::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(2);
	stream << data->sharedData.legacyName;
	stream << data->sharedData.terrainDecorators;
	return stream;
}

filesystem::InputStream &DecoratorMode::readStream(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;

	if(version == 0)
		return stream;

	stream >> data->sharedData.legacyName;
	if(version >= 2)
		stream >> data->sharedData.terrainDecorators;

	data->sharedData.setDialogWater();
	data->sharedData.updateDialog();
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
