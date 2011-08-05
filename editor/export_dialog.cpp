// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "export_dialog.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "resource/resource.h"
#include "command_list.h"
#include "icommand.h"
#include "common_dialog.h"
#include "export_options.h"
#include "../util/mod_selector.h"
#include "file_iterator.h"

#include <assert.h>
#include <vector>
#include <io.h>

// TODO: move to some configuration file?
// ...survivor should have a separate editor project, with PROJECT_SURVIVOR defined...
// ...for now, this legacy files check shall do... --jpk
#ifdef LEGACY_FILES
#define PROJECT_SURVIVOR 1
#endif

namespace frozenbyte {
namespace editor {

extern util::ModSelector modSelector;

inline const char *getMissionFolder(Dialog &dialog)
{
#ifdef LEGACY_FILES
	const char *missionFolder = "Data\\Missions\\";
#else
	const char *missionFolder = "data\\mission\\";
#endif

#ifdef PROJECT_SURVIVOR
	int folder = SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSIONFOLDER, CB_GETCURSEL, 0, 0);
	if(folder == 1)
	{
		missionFolder = "Survival\\";
	}
#endif

	return missionFolder;
}

namespace {
	struct SharedData
	{
		bool hasData;
		std::vector<std::string> fileNames;

		std::string fileName;
		bool onlyScripts;

		SharedData()
		:	hasData(false),
			onlyScripts(false)
		{
		}
	};

	class UpdateCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

		void getNames()
		{
			sharedData.fileNames.clear();


			const char *missionFolder = getMissionFolder(dialog);

#ifdef LEGACY_FILES
			FileIterator fileIterator(missionFolder + std::string("*.*"), true);
#else
			FileIterator fileIterator("data\\mission\\*.*", true);
#endif
			std::string fileName = fileIterator.getFileName();
			
			while(!fileName.empty())
			{
				// ignore these mission directories -jpk
#ifdef LEGACY_FILES
				if (fileName  != "Common" 
					&& fileName != "CVS"
					&& fileName != ".svn"
					&& fileName != "MissionMenu"
					&& fileName != "Template")
				{
					sharedData.fileNames.push_back(missionFolder + fileName);
				}
#else
				if (fileName != "common" 
					&& fileName != "CVS"
					&& fileName != ".svn"
					&& fileName != "missionmenu"
					&& fileName != "template")
				{
					sharedData.fileNames.push_back("data\\mission\\" + fileName);
				}
#endif

				fileIterator.next();
				fileName = fileIterator.getFileName();
			}
		}

		void setNames()
		{
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSION_NAMES, LB_RESETCONTENT, 0, 0);

			for(unsigned int i = 0; i < sharedData.fileNames.size(); ++i)
			{
				std::string string = getFileName(sharedData.fileNames[i]);
				//std::string string = sharedData.fileNames[i];

				SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSION_NAMES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
			}
		}

	public:
		UpdateCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
		}

		void updateDialog()
		{
			getNames();
			std::sort(sharedData.fileNames.begin(), sharedData.fileNames.end());
			setNames();
		}

		void execute(int id)
		{
			updateDialog();
		}
	};

	class InitializationCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		InitializationCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
		}

		void execute(int id)
		{
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSIONFOLDER, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSIONFOLDER, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Campaign"));
#ifdef PROJECT_SURVIVOR
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSIONFOLDER, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Survival"));
#endif
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSIONFOLDER, CB_SETCURSEL, 0, 0);
			UpdateCommand updateCommand(sharedData, dialog);
			updateCommand.execute(id);
		}
	};

	class ExportCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;

	public:
		ExportCommand(SharedData &sharedData_, Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
		}

		void execute(int id)
		{
			int index = SendDlgItemMessage(dialog.getWindowHandle(), IDC_MISSION_NAMES, LB_GETCURSEL, 0, 0);
			if(index >= 0)
			{
				sharedData.fileName = sharedData.fileNames[index];
			}

			sharedData.onlyScripts = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_SCRIPT_ONLY) == BST_CHECKED;
			dialog.hide();
		}
	};

	class CreateNewMissionCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &dialog;
		Dialog &exporter_dialog;

	public:
		CreateNewMissionCommand(SharedData &sharedData_, Dialog &dialog_, Dialog &exporter_dialog_)
		:	exporter_dialog(exporter_dialog_),
		  sharedData(sharedData_),
			dialog(dialog_)
		{
		}

		void execute(int id)
		{
			std::string mission_name = getDialogItemText(dialog, IDC_NEWMISSION_NAME);
			if(mission_name.empty())
			{
				MessageBox(0, "You really need a name for the mission.", "Error!", MB_OK);
				return;
			}
			if(mission_name.find_first_of("\\/.") != std::string::npos)
			{
				MessageBox(0, "You don't need any extra crap, just a name.", "Error!", MB_OK);
				return;
			}

			for(unsigned int i = 0; i < mission_name.length(); ++i)
				mission_name[i] = tolower(mission_name[i]);

			std::string mission_folder = getMissionFolder(exporter_dialog); 
			std::string mission_file = mission_folder + mission_name + "\\" + mission_name + ".dhm";
			if(fileExists(mission_file))
			{
				MessageBox(0, "Mission with that name already exists!", "Error!", MB_OK);
				return;
			}

			// get exe path
			char buffer[1024] = { 0 };
			GetModuleFileName(GetModuleHandle(0), buffer, 1023);
			std::string dir = buffer;
			int end = dir.find_last_of('\\');
			dir = dir.substr(0, end);

#ifdef LEGACY_FILES
			std::string command = "\"" + dir + "\\tools\\missioncreator.exe\" " + mission_folder + " Template " + mission_name;
#else
			std::string command = "\"" + dir + "\\tools\\missioncreator.exe\" " + mission_folder + " template " + mission_name;
#endif
			int ret = system(command.c_str());
			if(ret == 1)
			{
				MessageBox(0, "Creating mission failed!", "Error!", 0);
			}

			// update the mission list
			UpdateCommand uc(sharedData, exporter_dialog);
			uc.execute(id);

			dialog.hide();
		}
	};

	class QueryNewMissionCommand: public ICommand
	{
		SharedData &sharedData;
		Dialog &exporter_dialog;

	public:
		QueryNewMissionCommand(SharedData &sharedData_, Dialog &exporter_dialog_)
		:	sharedData(sharedData_),
			exporter_dialog(exporter_dialog_)
		{
		}

		void execute(int id)
		{
			Dialog dialog(IDD_NEWMISSION);
			CreateNewMissionCommand cnmc(sharedData, dialog, exporter_dialog);
			dialog.getCommandList().addCommand(IDC_NEWMISSION_OK, &cnmc);
			dialog.show();
		}
	};
}

struct ExportDialogData
{
	SharedData sharedData;
};

ExportDialog::ExportDialog()
{
	boost::scoped_ptr<ExportDialogData> tempData(new ExportDialogData());
	data.swap(tempData);
}

ExportDialog::~ExportDialog()
{
}

ExportOptions ExportDialog::show()
{
	Dialog dialog(IDD_EXPORT);

	InitializationCommand initializationCommand(data->sharedData, dialog);
	ExportCommand exportCommand(data->sharedData, dialog);
	UpdateCommand updateCommand(data->sharedData, dialog);
	QueryNewMissionCommand queryMissionCommand(data->sharedData, dialog);
	
	dialog.getCommandList(WM_INITDIALOG).addCommand(WM_INITDIALOG, &initializationCommand);
	dialog.getCommandList().addCommand(IDC_EXPORT, &exportCommand);
	dialog.getCommandList().addCommand(IDC_NEWMISSION, &queryMissionCommand);
	dialog.getCommandList().addCommand(IDC_MISSIONFOLDER, &updateCommand);

	dialog.show();


	ExportOptions options;
	options.fileName = data->sharedData.fileName;

	if(options.fileName.empty())
		return options;

	options.id = makeMissionIdFromFileName(options.fileName);

	options.onlyScripts = data->sharedData.onlyScripts;
	return options;
}

} // end of namespace editor
} // end of namespace frozenbyte
