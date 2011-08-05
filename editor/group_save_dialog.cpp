// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "group_save_dialog.h"
#include "group_list.h"
#include "resource/resource.h"
#include <windows.h>

using namespace std;
using namespace boost;

namespace frozenbyte {
namespace editor {
namespace {
	
	struct Strings
	{
		string group;
		string subgroup;
		string name;

		GroupList list;
	};

} // unnamed

struct GroupSaveDialog::Data
{
	Strings strings;
};

namespace {

	string getLabelText(HWND windowHandle, int item)
	{
		string result;
		result.resize(666);
		int length = GetDlgItemText(windowHandle, item , &result[0], 665);
		if(length < 2)
			return std::string();

		result.resize(length);
		return result;
	}

	string getComboText(HWND windowHandle, int item)
	{
		/*
		string result;

		int index = SendDlgItemMessage(windowHandle, item, CB_GETCURSEL, 0, 0);
		if(index < 0)
			return result;

		int length = SendDlgItemMessage(windowHandle, item, CB_GETLBTEXTLEN, index, 0);
		if(length < 2)
			return result;

		result.resize(length + 1);
		SendDlgItemMessage(windowHandle, IDC_GROUP_GROUP, CB_GETLBTEXTLEN, index, LPARAM(&result[0]));
		result.resize(result.size() - 1);

		return result;
		*/

		return getLabelText(windowHandle, item);
	}

	BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		Strings *data = reinterpret_cast<Strings *> (GetWindowLong(windowHandle, GWL_USERDATA));

		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			data = reinterpret_cast<Strings *> (lParam);

			for(int i = 0; i < data->list.getGroupAmount(); ++i)
			{
				const string &str = data->list.getGroupName(i);
				SendDlgItemMessage(windowHandle, IDC_GROUP_GROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str.c_str()));
			}

			SendDlgItemMessage(windowHandle, IDC_GROUP_GROUP, CB_SETCURSEL, 0, 0);
			SendMessage(windowHandle, WM_COMMAND, IDC_GROUP_GROUP, IDC_GROUP_GROUP);
		}
		else if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY || command == IDCANCEL)
				EndDialog(windowHandle, 0);

			if(command == IDOK)
			{
				data->group = getComboText(windowHandle, IDC_GROUP_GROUP);
				data->subgroup = getComboText(windowHandle, IDC_GROUP_SUBGROUP);
				data->name = getLabelText(windowHandle, IDC_GROUP_NAME);

				EndDialog(windowHandle, 1);
			}
			else if(command == IDC_GROUP_GROUP)
			{
				int group = SendDlgItemMessage(windowHandle, IDC_GROUP_GROUP, CB_GETCURSEL, 0, 0);
				SendDlgItemMessage(windowHandle, IDC_GROUP_SUBGROUP, CB_RESETCONTENT, 0, 0);

				if(group >= 0)
				{
					for(int i = 0; i < data->list.getSubgroupAmount(group); ++i)
					{
						const string &str = data->list.getSubgroupName(group, i);
						SendDlgItemMessage(windowHandle, IDC_GROUP_SUBGROUP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (str.c_str()));
					}

					SendDlgItemMessage(windowHandle, IDC_GROUP_SUBGROUP, CB_SETCURSEL, 0, 0);
				}

			}

			if(command == IDC_GROUP_GROUP || command == IDC_GROUP_SUBGROUP || command == IDC_GROUP_NAME)
			{
				string s1 = getComboText(windowHandle, IDC_GROUP_GROUP);
				string s2 = getComboText(windowHandle, IDC_GROUP_SUBGROUP);
				string s3 = getLabelText(windowHandle, IDC_GROUP_NAME);

				if(s1.size() > 2 && s2.size() > 2 && s3.size() > 2)
					EnableWindow(GetDlgItem(windowHandle, IDOK), TRUE);
				else
					EnableWindow(GetDlgItem(windowHandle, IDOK), FALSE);
			}
		}

		return 0;
	}

} // unnamed

GroupSaveDialog::GroupSaveDialog()
{
	scoped_ptr<Data> tempData(new Data());
	data.swap(tempData);
}

GroupSaveDialog::~GroupSaveDialog()
{
}

const std::string &GroupSaveDialog::getGroup()
{
	return data->strings.group;
}

const std::string &GroupSaveDialog::getSubgroup()
{
	return data->strings.subgroup;
}

const std::string &GroupSaveDialog::getName()
{
	return data->strings.name;
}

bool GroupSaveDialog::show()
{
	if(!DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_GROUPSAVE), 0, DialogHandler, reinterpret_cast<LPARAM> (&data->strings)))
		return false;

	return true;
}

} // editor
} // frozenbyte
