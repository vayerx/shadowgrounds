// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

#include "gui.h"
#include "window.h"
#include "dialog.h"
#include "icommand.h"
#include "command_list.h"
#include "mouse.h"
#include "resource/resource.h"

#include <vector>
#include <cassert>

namespace frozenbyte {
namespace editor {
namespace {
	struct StatusDialogs
	{
		Dialog *currentDialog;

		Dialog terrain;
		Dialog scene;
		Dialog objects;
		Dialog buildings;
		Dialog units;
		Dialog decorators;
		Dialog particles;
		Dialog lights;

		StatusDialogs(Window &window)
		:	terrain(IDD_TERRAIN, window, Dialog::ATTACH_BOTTOM),
			scene(IDD_SCENE, window, Dialog::ATTACH_BOTTOM),
			objects(IDD_OBJECTS, window, Dialog::ATTACH_BOTTOM),
			buildings(IDD_BUILDINGS, window, Dialog::ATTACH_BOTTOM),
			units(IDD_UNITS, window, Dialog::ATTACH_BOTTOM),
			decorators(IDD_DECORATORS, window, Dialog::ATTACH_BOTTOM),
			particles(IDD_PARTICLES, window, Dialog::ATTACH_BOTTOM),
			lights(IDD_LIGHTS, window, Dialog::ATTACH_BOTTOM)
		{
			currentDialog = 0;

			initializeDialog(terrain);
			initializeDialog(scene);
			initializeDialog(objects);
			initializeDialog(buildings);
			initializeDialog(units);
			initializeDialog(decorators);
			initializeDialog(particles);
			initializeDialog(lights);
		}

		~StatusDialogs()
		{
		}

		void initializeDialog(Dialog &d)
		{
			d.setPosition(0, 49);
			d.hide();
		}

		void setActiveDialog(Dialog &dialog)
		{
			if(currentDialog == &dialog)
				return;

			if(currentDialog)
				currentDialog->hide();

			dialog.show();
			currentDialog = &dialog;
		}
	};

	class StatusCommand: public ICommand
	{
		StatusDialogs &statusDialogs;
		Dialog &dialog;
		ICommand *menuCommand;

	public:
		StatusCommand(StatusDialogs &statusDialogs_, Dialog &dialog_)
		:	statusDialogs(statusDialogs_),
			dialog(dialog_)
		{
			menuCommand = 0;
		}

		void execute(int id)
		{
			statusDialogs.setActiveDialog(dialog);
			if(menuCommand)
				menuCommand->execute(id);
		}

		void setCommand(ICommand *newCommand)
		{
			menuCommand = newCommand;
		}
	};

	struct StatusCommands
	{
		StatusCommand terrain;
		StatusCommand scene;
		StatusCommand objects;
		StatusCommand buildings;
		StatusCommand units;
		StatusCommand decorators;
		StatusCommand particles;
		StatusCommand lights;

		StatusCommands(StatusDialogs &dialogs, Dialog &menu)
		:	terrain(dialogs, dialogs.terrain),
			scene(dialogs, dialogs.scene),
			objects(dialogs, dialogs.objects),
			buildings(dialogs, dialogs.buildings),
			units(dialogs, dialogs.units),
			decorators(dialogs, dialogs.decorators),
			particles(dialogs, dialogs.particles),
			lights(dialogs, dialogs.lights)
		{
		}

		~StatusCommands()
		{
		}

		void addCommands(CommandList &commandList)
		{
			initializeCommand(terrain, commandList, IDC_MENU_TERRAIN);
			initializeCommand(scene, commandList, IDC_MENU_SCENE);
			initializeCommand(objects, commandList, IDC_MENU_OBJECTS);
			initializeCommand(buildings, commandList, IDC_MENU_BUILDINGS);
			initializeCommand(units, commandList, IDC_MENU_UNIT);
			initializeCommand(decorators, commandList, IDC_MENU_DECORATORS);
			initializeCommand(particles, commandList, IDC_MENU_PARTICLES);
			initializeCommand(lights, commandList, IDC_MENU_LIGHTS);
		}

		void initializeCommand(StatusCommand &command, CommandList &commandList, int resourceId)
		{
			commandList.addCommand(resourceId, &command);
		}
	};

} // end of unnamed namespace

struct GuiData
{
	Window window;

	Dialog menuDialog;
	Dialog renderDialog;

	StatusDialogs statusDialogs;
	StatusCommands statusCommands;

	Mouse mouse;

#ifdef PROJECT_AOV
	GuiData(Gui &gui)
	:	window("AOV Editor", IDI_ICON1, true, false),
		menuDialog(IDD_MENU, window, Dialog::ATTACH_RIGHT),
		renderDialog(IDD_RENDER, window, Dialog::ATTACH_ALL),
#else
	GuiData(Gui &gui)
	:	window("Frozenbyte Editor", IDI_ICON1, true, false),
		menuDialog(IDD_MENU, window, Dialog::ATTACH_RIGHT),
		renderDialog(IDD_RENDER, window, Dialog::ATTACH_ALL),
#endif

		statusDialogs(window),
		statusCommands(statusDialogs, menuDialog)
	{
		statusCommands.addCommands(menuDialog.getCommandList());
		statusCommands.addCommands(window.getCommandList());

		menuDialog.setPosition(0,0);

		//renderDialog.setSize(400, 400);
		//renderDialog.setPosition(230, 50);
		renderDialog.setSize(350, 400);
		renderDialog.setPosition(306, 50);

		renderDialog.setMouse(mouse);
		window.setMouse(mouse);
		mouse.setTrackWindow(renderDialog.getWindowHandle());
	}

	~GuiData()
	{
	}
};

Gui::Gui()
{
	boost::scoped_ptr<GuiData> tempData(new GuiData(*this));

	CheckDlgButton(tempData->menuDialog.getWindowHandle(), IDC_MENU_TERRAIN, BST_CHECKED);
	SendMessage(tempData->menuDialog.getWindowHandle(), WM_COMMAND, IDC_MENU_TERRAIN, 0);

	data.swap(tempData);
}

Gui::~Gui()
{
}

void Gui::setMenuCommand(int id, ICommand *command)
{
	if(id == IDC_MENU_TERRAIN)
		data->statusCommands.terrain.setCommand(command);
	else if(id == IDC_MENU_SCENE)
		data->statusCommands.scene.setCommand(command);
	else if(id == IDC_MENU_OBJECTS)
		data->statusCommands.objects.setCommand(command);
	else if(id == IDC_MENU_BUILDINGS)
		data->statusCommands.buildings.setCommand(command);
	else if(id == IDC_MENU_UNIT)
		data->statusCommands.units.setCommand(command);
	//else if(id == IDC_MENU_DECORATORS)
	//	data->statusCommands.decorators.setCommand(command);
	//else if(id == IDC_MENU_PARTICLES)
	//	data->statusCommands.particles.setCommand(command);
	else if(id == IDC_MENU_LIGHTS)
		data->statusCommands.lights.setCommand(command);
	else
	{
		assert(!"Crash boom bang!");
	}
}

void Gui::reset()
{
	HWND menu = data->menuDialog.getWindowHandle();
	CheckDlgButton(menu, IDC_MENU_TERRAIN, BST_CHECKED);
	CheckDlgButton(menu, IDC_MENU_SCENE, BST_UNCHECKED);
	CheckDlgButton(menu, IDC_MENU_OBJECTS, BST_UNCHECKED);
	CheckDlgButton(menu, IDC_MENU_UNIT, BST_UNCHECKED);
	CheckDlgButton(menu, IDC_MENU_SCRIPTS, BST_UNCHECKED);
	CheckDlgButton(menu, IDC_MENU_DECORATORS, BST_UNCHECKED);
	CheckDlgButton(menu, IDC_MENU_LIGHTS, BST_UNCHECKED);

	data->statusDialogs.setActiveDialog(data->statusDialogs.terrain);
}

bool Gui::handleDialogs(MSG &message)
{
	if(IsDialogMessage(data->menuDialog.getWindowHandle(), &message))
		return true;
	if(IsDialogMessage(data->statusDialogs.terrain.getWindowHandle(), &message))
		return true;
	if(IsDialogMessage(data->statusDialogs.scene.getWindowHandle(), &message))
		return true;
	if(IsDialogMessage(data->statusDialogs.objects.getWindowHandle(), &message))
		return true;
	if(IsDialogMessage(data->statusDialogs.buildings.getWindowHandle(), &message))
		return true;
	if(IsDialogMessage(data->statusDialogs.units.getWindowHandle(), &message))
		return true;
	if(IsDialogMessage(data->statusDialogs.decorators.getWindowHandle(), &message))
		return true;

	return false;
}


Mouse &Gui::getMouse()
{
	return data->mouse;
}

Window &Gui::getMainWindow()
{
	return data->window;
}

Dialog &Gui::getRenderDialog()
{
	return data->renderDialog;
}

Dialog &Gui::getMenuDialog()
{
	return data->menuDialog;
}

Dialog &Gui::getTerrainDialog()
{
	return data->statusDialogs.terrain;
}

Dialog &Gui::getSceneDialog()
{
	return data->statusDialogs.scene;
}

Dialog &Gui::getObjectsDialog()
{
	return data->statusDialogs.objects;
}

Dialog &Gui::getBuildingsDialog()
{
	return data->statusDialogs.buildings;
}

Dialog &Gui::getUnitsDialog()
{
	return data->statusDialogs.units;
}

Dialog &Gui::getDecoratorsDialog()
{
	return data->statusDialogs.decorators;
}

Dialog &Gui::getParticleDialog()
{
	return data->statusDialogs.particles;
}

Dialog &Gui::getLightDialog()
{
	return data->statusDialogs.lights;
}

} // end of namespace editor
} // end of namespace frozenbyte
