// Copyright 2002-2004 Frozenbyte Ltd.

#include "Export_Dialogs.h"
#include "Export_Exporter.h"
#include "Export_Object_Chopper.h"

#include <boost/lexical_cast.hpp>
#include "Resources\Resource.h"
#include <cassert>

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

namespace frozenbyte {
namespace exporter {

// Globals, duh ..
namespace {
	Exporter *exporter = 0;
	HINSTANCE moduleHandle = 0;

	enum ListState { OBJECTS = 0, BONES = 1 };
	ListState listState = OBJECTS;
}

// Helper functions
std::string createInfoText(HWND windowHandle);
void createListItems(HWND windowHandle);
std::string saveFileDialog(HWND windowHandle, const std::string &title, const std::string &fileInfo, const std::string &extension, const std::string &defaultName = "");

// Handle our mighty dialog
BOOL CALLBACK ExportDialog(HWND windowHandle, UINT message, WPARAM wParameter, LPARAM)
{
	assert(exporter);
	assert(moduleHandle);

	if(message == WM_LBUTTONDOWN || message == WM_ACTIVATE || message == WM_NCLBUTTONDOWN)
		KillTimer(windowHandle, 1);

	if(message == WM_INITDIALOG)
	{
		HICON iconHandle = LoadIcon(moduleHandle, MAKEINTRESOURCE(IDI_LOGO));
		PostMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM) iconHandle);

		SendDlgItemMessage(windowHandle, IDC_TRANSFORM_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Pivot as origin"));
		SendDlgItemMessage(windowHandle, IDC_TRANSFORM_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("Pivot as transform"));
		SendDlgItemMessage(windowHandle, IDC_TRANSFORM_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("None"));

		if(exporter->getBones().empty())
			SendDlgItemMessage(windowHandle, IDC_TRANSFORM_TYPE, CB_SETCURSEL, 0, 0);
		else
			SendDlgItemMessage(windowHandle, IDC_TRANSFORM_TYPE, CB_SETCURSEL, 2, 0);
		//if(exporter->getBones().empty())
		//	CheckDlgButton(windowHandle, IDC_PIVOT_ORIGIN, BST_CHECKED);

		CheckDlgButton(windowHandle, IDC_CHECK_WARNINGS, BST_CHECKED);
		CheckDlgButton(windowHandle, IDC_CHECK_SUMMARY, BST_CHECKED);
		CheckDlgButton(windowHandle, IDC_OBJECT_SELECTION, BST_CHECKED);

#ifndef FB_FAST_BUILD
		CheckDlgButton(windowHandle, IDC_VCACHE, BST_CHECKED);
#endif

		//SendDlgItemMessage(windowHandle, IDC_LOD_DETAIL, TBM_SETRANGE, TRUE, MAKELONG(0, 2));

		std::string info = createInfoText(windowHandle);
		SetDlgItemText(windowHandle, IDC_INFO, info.c_str());

		listState = OBJECTS;
		createListItems(windowHandle);

		if(exporter->getBones().size() == 0)
		{
			EnableWindow(GetDlgItem(windowHandle, IDC_BONES), FALSE);
			EnableWindow(GetDlgItem(windowHandle, IDC_ANIMATION), FALSE);
		}

		if(needChop(exporter->getModel().getObjects()))
		{
			//std::string text = std::string("Chop to ") + boost::lexical_cast<std::string> (newObjects) + std::string(" piece(s)");
			std::string text = "Chop to x pieces";
			SetWindowText(GetDlgItem(windowHandle, IDC_CHOP), text.c_str());

			CheckDlgButton(windowHandle, IDC_CHOP, BST_CHECKED);
		}
		else
			EnableWindow(GetDlgItem(windowHandle, IDC_CHOP), FALSE);
	}
	else if(message == WM_TIMER)
	{
		FlashWindow(windowHandle, TRUE);
	}
	else if(message == WM_COMMAND)
	{
		if(wParameter == WM_DESTROY)
		{
			EndDialog(windowHandle, 0);
		}

		else if(wParameter == IDC_MODEL)
		{
			// Collect all objects to save
			std::vector<std::string> objectNames;
	
			// If no object list selected, use all
			if(listState == BONES)
			{
				const std::vector<boost::shared_ptr<Object> > &objects = exporter->getModel().getObjects();
				for(unsigned int i = 0; i < objects.size(); ++i)
					objectNames.push_back(objects[i]->getName());
			}
			else if(listState == OBJECTS)
			{
				// Get all indices
				int objectAmount = exporter->getModel().getObjects().size();
				std::vector<int> indices(objectAmount);

				int objects = SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_GETSELITEMS, objectAmount, reinterpret_cast<long> (&indices[0]));

				// These are in same order on list box
				for(int i = 0; i < objects; ++i)
					objectNames.push_back(exporter->getModel().getObjects()[indices[i]]->getName());
			}

			std::string defaultName = objectNames.size() != 1 || listState != OBJECTS ? "" : objectNames[0];
			std::string fileName = saveFileDialog(windowHandle, "Save to", "Storm3D Model", "s3d", defaultName);

			if(fileName.size() > 0)
			{
				bool copyTextures = false;
				//bool usePivot = false;
				//bool generateLods = false;
				//int lodDetail = SendDlgItemMessage(windowHandle, IDC_LOD_DETAIL, TBM_GETPOS, 0, 0);

				if(IsDlgButtonChecked(windowHandle, IDC_COPYTEXTURES) == BST_CHECKED)
					copyTextures = true;
				//if(IsDlgButtonChecked(windowHandle, IDC_PIVOT_ORIGIN) == BST_CHECKED)
				//	usePivot = true;
				//if(IsDlgButtonChecked(windowHandle, IDC_LODS) == BST_CHECKED)
				//	generateLods = true;

				int transformType = SendDlgItemMessage(windowHandle, IDC_TRANSFORM_TYPE, CB_GETCURSEL, 0, 0);
				bool chop = false;
				if(IsDlgButtonChecked(windowHandle, IDC_CHOP) == BST_CHECKED)
					chop = true;

				bool vcache = false;
				if(IsDlgButtonChecked(windowHandle, IDC_VCACHE) == BST_CHECKED)
					vcache = true;

				//if(exporter->getBoneId() > 0)
				//	chop = false;

				bool hasBones = false;
				if(!exporter->getBones().empty())
					hasBones = true;

				exporter->getModel().saveToFile(fileName, objectNames, exporter->getBoneId(), copyTextures, transformType, chop, vcache, hasBones);
				SetTimer(windowHandle, 1, 500, 0);
			}
		}

		else if(wParameter == IDC_BONES)
		{
			std::string fileName = saveFileDialog(windowHandle, "Save to", "Storm3D Bone Structure", "b3d");
			if(fileName.size() > 0)
				exporter->saveBonesToFile(fileName);
		}

		else if(wParameter == IDC_ANIMATION)
		{
			std::vector<std::string> boneNames;

			// If no object list selected, use all
			if(listState == OBJECTS)
			{
				const std::vector<Bone> &bones = exporter->getBones();
				for(unsigned int i = 0; i < bones.size(); ++i)
					boneNames.push_back(bones[i].getName());
			}
			else if(listState == BONES)
			{
				// Get all indices
				int boneAmount = exporter->getBones().size();
				std::vector<int> indices(boneAmount);

				int bones = SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_GETSELITEMS, boneAmount, reinterpret_cast<long> (&indices[0]));
				
				// These are in same order on list box
				for(int i = 0; i < bones; ++i)
					boneNames.push_back(exporter->getBones()[indices[i]].getName());
			}

			std::string fileName = saveFileDialog(windowHandle, "Save to", "Storm3D Bone Animation", "anm");
			if(fileName.size() > 0)
				exporter->saveBoneAnimationToFile(fileName, boneNames);
		}

		else if(wParameter == IDC_OBJECT_SELECTION)
		{
			CheckDlgButton(windowHandle, IDC_OBJECT_SELECTION, BST_CHECKED);
			CheckDlgButton(windowHandle, IDC_BONE_SELECTION, BST_UNCHECKED);
			
			listState = OBJECTS;
			createListItems(windowHandle);
		}
		else if(wParameter == IDC_BONE_SELECTION)
		{
			CheckDlgButton(windowHandle, IDC_OBJECT_SELECTION, BST_UNCHECKED);
			CheckDlgButton(windowHandle, IDC_BONE_SELECTION, BST_CHECKED);
			
			listState = BONES;
			createListItems(windowHandle);
		}

		else switch(wParameter)
		{
			case IDC_CHECK_WARNINGS:
			case IDC_CHECK_SUMMARY:
			case IDC_CHECK_GEOMETRY:
			case IDC_CHECK_BONES:
			case IDC_CHECK_ANIMATION:
			{
				std::string info = createInfoText(windowHandle);
				SetDlgItemText(windowHandle, IDC_INFO, info.c_str());
			}
		}
	}
	
	return FALSE;
}

// Creates dialogs and saves data as needed
void createExportDialog(Exporter *exporter_, HINSTANCE moduleHandle_)
{
	exporter = exporter_;
	moduleHandle = moduleHandle_;

	assert(exporter);
	assert(moduleHandle);

	exporter->validateData();
	exporter->getModel().removeJunctions();
	exporter->getModel().remapIndices();
	DialogBox(moduleHandle, MAKEINTRESOURCE(IDD_EXPORT), 0, (DLGPROC) ExportDialog);
}

std::string createInfoText(HWND windowHandle)
{
	std::string separator = "------------------------------------------------------------------------------------------------------------------------\r\n";
	std::string info;
	
	if(IsDlgButtonChecked(windowHandle, IDC_CHECK_WARNINGS) == BST_CHECKED)
	{
		info += separator;
		info += "Warnings: \r\n";
		info += separator + "\r\n";
		info +=	exporter->getExporterInfo();
		info += "\r\n\r\n";
	}
	if(IsDlgButtonChecked(windowHandle, IDC_CHECK_SUMMARY) == BST_CHECKED)
	{
		info += separator;
		info += "Summary: \r\n";
		info += separator + "\r\n";
		info +=	exporter->getSummaryInfo();
		info += "\r\n\r\n";
	}
	if(IsDlgButtonChecked(windowHandle, IDC_CHECK_MODEL) == BST_CHECKED)
	{
		info += separator;
		info += "Model: \r\n";
		info += separator + "\r\n";
		info +=	exporter->getModelInfo();
		info += "\r\n\r\n";
	}
	if(IsDlgButtonChecked(windowHandle, IDC_CHECK_BONES) == BST_CHECKED)
	{
		info += separator;
		info += "Bones: \r\n";
		info += separator + "\r\n";
		info +=	exporter->getBoneInfo();
		info += "\r\n\r\n";
	}
	if(IsDlgButtonChecked(windowHandle, IDC_CHECK_ANIMATION) == BST_CHECKED)
	{
		info += separator;
		info += "Animation: \r\n";
		info += separator + "\r\n";
		info +=	exporter->getAnimationInfo();
	}

	//info += separator;
	//info += "\t\t\t\tCopyright (c) 2002 Frozenbyte OY.";
	return info;
}

void createListItems(HWND windowHandle)
{
	SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_RESETCONTENT, 0, 0);

	if(listState == OBJECTS)
	{
		const std::vector<boost::shared_ptr<Object> > &objects = exporter->getModel().getObjects();
		for(unsigned int i = 0; i < objects.size(); ++i)
		{
			std::string line = objects[i]->getName();
			
			char number[20] = { 0 };
			_itoa(objects[i]->getMaterialAmount(), number, 10);
			
			line += " (" + std::string(number) + ")";
			const char *str = line.c_str();

			int index = SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_ADDSTRING, 0, reinterpret_cast<long> (str));
			SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_SETSEL, TRUE, index);
		}
	}
	else
	{
		const std::vector<Bone> &bones = exporter->getBones();
		for(unsigned int i = 0; i < bones.size(); ++i)
		{
			const char *str = bones[i].getName().c_str();
			
			int index = SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_ADDSTRING, 0, reinterpret_cast<long> (str));
			SendMessage(GetDlgItem(windowHandle, IDC_LISTBOX), LB_SETSEL, TRUE, index);
		}
	}
}

std::string saveFileDialog(HWND windowHandle, const std::string &title, const std::string &fileInfo, const std::string &extension, const std::string &defaultName)
{
	OPENFILENAME openFileName = { 0 };

	// Store current dir. (otherwise it changes by user's doings)
	char workingDirectory[1000] = { 0 };;
	GetCurrentDirectory(1000, workingDirectory);
	
	char fileName[1000] = { 0 };
	if(!defaultName.empty())
		strcpy(fileName, defaultName.c_str());

	// Hax hax
	for(unsigned int i = 0; i < strlen(fileName); ++i)
	{
		if(fileName[i] == ':')
			fileName[i] = '_';

		if(strcmp(&fileName[i], "_NoCollision") == 0)
			fileName[i] = '\0';
	}

	// Eg. "Storm3D Model (*.s3d)\0*.s3d\0\0"
	std::string fileMask = fileInfo + " (*." + extension + ")\r*." + extension + "\r\r";	
	// String don't like \0 so use \r and change that manually
	char charFileMask[1000];
	strcpy(charFileMask, fileMask.c_str());

	for(unsigned int j = 0; j < fileMask.size(); ++j)
		if(charFileMask[j] == '\r')
			charFileMask[j] = '\0';
	
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = windowHandle;
	openFileName.hInstance = 0;
	openFileName.lpstrFilter = charFileMask;
	openFileName.lpstrCustomFilter = 0;
	openFileName.nMaxCustFilter = 0;
	openFileName.nFilterIndex = 0;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = 1000;
	openFileName.lpstrFileTitle = 0;
	openFileName.lpstrInitialDir = 0;
	openFileName.lpstrTitle = title.c_str();
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	openFileName.nFileOffset = 0;
	openFileName.nFileExtension = 0;
	openFileName.lpstrDefExt = extension.c_str();
	openFileName.lCustData = 0;
	openFileName.lpfnHook = 0;
	openFileName.lpTemplateName = 0;

	std::string result;

	if(GetSaveFileName(&openFileName) == TRUE)
		result = fileName;

	// Restore working dir
	SetCurrentDirectory(workingDirectory);
	return result;
}


} // end of namespace export
} // end of namespace frozenbyte
