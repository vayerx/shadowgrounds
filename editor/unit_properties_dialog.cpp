// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "unit_properties_dialog.h"
#include "terrain_units.h"
#include "unit_properties.h"
#include "common_dialog.h"
#include "resource/resource.h"
#include "string_properties.h"
#include <windows.h>
#include <map>

using namespace std;

namespace frozenbyte {
namespace editor {

typedef vector<string> StringList;
typedef map<string, string> StringMap;

struct UnitPropertiesDialog::Data
{
	UnitProperties &properties; 
	const StringList &usedStrings;
	const StringProperties &stringProperties;

	StringMap strings;

	Data(UnitProperties &properties_, const StringList &usedStrings_, const StringProperties &stringProperties_)
	:	properties(properties_),
		usedStrings(usedStrings_),
		stringProperties(stringProperties_)
	{
	}

	void set(HWND window)
	{
		StringList::const_iterator it = usedStrings.begin();
		for(; it != usedStrings.end(); ++it)
		{
			SendDlgItemMessage(window, IDC_UNIT_DATANAME, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (it->c_str()));

			StringMap::iterator i = properties.strings.find(*it);
			if(i != properties.strings.end())
				strings[*it] = i->second;
		}

		SendDlgItemMessage(window, IDC_UNIT_DATANAME, CB_SETCURSEL, 0, 0);

		if(properties.difficulty == UnitProperties::All)
			CheckDlgButton(window, IDC_ALL, BST_CHECKED);
		else if(properties.difficulty == UnitProperties::EasyOnly)
			CheckDlgButton(window, IDC_EASY, BST_CHECKED);
		else if(properties.difficulty == UnitProperties::HardOnly)
			CheckDlgButton(window, IDC_HARD, BST_CHECKED);

		SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("All"));
		SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("A == 0"));
		SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("A == 1"));
		SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("B == 0"));
		SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("B == 1"));
		SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_SETCURSEL, properties.layout, 0);

		setString(window);
	}

	void get(HWND window)
	{
		if(IsDlgButtonChecked(window, IDC_ALL) == BST_CHECKED)
			properties.difficulty = UnitProperties::All;
		else if(IsDlgButtonChecked(window, IDC_EASY) == BST_CHECKED)
			properties.difficulty = UnitProperties::EasyOnly;
		else if(IsDlgButtonChecked(window, IDC_HARD) == BST_CHECKED)
			properties.difficulty = UnitProperties::HardOnly;

		properties.layout = SendDlgItemMessage(window, IDC_UNIT_LAYOUT, CB_GETCURSEL, 0, 0);

		StringMap::iterator it = strings.begin();
		for(; it != strings.end(); ++it)
			properties.strings[it->first] = it->second;
	}

	string getKey(HWND window) const
	{
		int keyIndex = SendDlgItemMessage(window, IDC_UNIT_DATANAME, CB_GETCURSEL, 0, 0);
		if(keyIndex < 0)
			return "";

		int keyLength = SendDlgItemMessage(window, IDC_UNIT_DATANAME, CB_GETLBTEXTLEN, keyIndex, 0) + 1;
		if(keyLength < 2)
			return "";

		string key;
		key.resize(keyLength);

		SendDlgItemMessage(window, IDC_UNIT_DATANAME, CB_GETLBTEXT, keyIndex, reinterpret_cast<LONG> (key.c_str()));
		key.resize(keyLength - 1);

		return key;
	}

	string getData(HWND window) const
	{
		char buffer[256] = { 0 };
		GetDlgItemText(window, IDC_UNIT_DATA, buffer, 255);

		return buffer;
	}

	void setString(HWND window)
	{
		string key = getKey(window);
		string data = strings[key];
		
		SetDlgItemText(window, IDC_UNIT_DATA, data.c_str());

		string def = "No default value defined";
		if(stringProperties.hasDefault(key))
		{
			def = "Default: ";
			def += stringProperties.getDefault(key);
		}

		SetDlgItemText(window, IDC_UNIT_DATA_DEFAULTVALUE, def.c_str());
	}

	void storeString(HWND window)
	{
		string key = getKey(window);
		string data = getData(window);

		if(!key.empty())
			strings[key] = data;
	}

	void openString(HWND window)
	{
		string file = getOpenFileName("*.*");
		if(file.empty())
			return;

		SetDlgItemText(window, IDC_UNIT_DATA, file.c_str());
	}

	static BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);

			Data *data = reinterpret_cast<UnitPropertiesDialog::Data *> (lParam);
			data->set(windowHandle);
		}
		else if(message == WM_COMMAND)
		{
			Data *data = reinterpret_cast<UnitPropertiesDialog::Data *> (GetWindowLong(windowHandle, GWL_USERDATA));

			int command = LOWORD(wParam);
			if(command == WM_DESTROY || command == IDCANCEL)
				EndDialog(windowHandle, 0);

			if(command == IDOK)
			{
				EndDialog(windowHandle, 1);
				data->get(windowHandle);
			}
			else if(command == IDC_UNIT_DATANAME)
				data->setString(windowHandle);
			else if(command == IDC_UNIT_DATA)
				data->storeString(windowHandle);
			else if(command == IDC_UNIT_OPEN)
				data->openString(windowHandle);
		}

		return 0;
	}
};

UnitPropertiesDialog::UnitPropertiesDialog(UnitProperties &properties, const StringList &usedStrings, const StringProperties &stringProperties)
:	data(new Data(properties, usedStrings, stringProperties))
{
}

UnitPropertiesDialog::~UnitPropertiesDialog()
{
}

void UnitPropertiesDialog::execute(int id)
{
	DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_UNIT_PROPERTIES), 0, UnitPropertiesDialog::Data::DialogHandler, reinterpret_cast<LPARAM> (data.get()));
}

} // editor
} // frozenbyte
