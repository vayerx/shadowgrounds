// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "dialog_utils.h"
#include "dialog.h"
#include "string_conversions.h"
#include <commctrl.h>

#include <windows.h>

namespace frozenbyte {
namespace editor {

void setDialogItemText(Dialog &dialog, int id, const std::string &text)
{
	SetDlgItemText(dialog.getWindowHandle(), id, text.c_str());
}

void setDialogItemInt(Dialog &dialog, int id, int value)
{
	SetDlgItemInt(dialog.getWindowHandle(), id, value, FALSE);
}

void setDialogItemFloat(Dialog &dialog, int id, float value, int decimals)
{
	std::string string = convertToString<float>(value);

	int pointPosition = string.find_first_of(".");
	if(pointPosition != std::string::npos)
	{
		int end = string.size();
		for(int i = string.size() - 1; i > pointPosition + 1; --i)
		{
			if(string[i] == '0')
				end = i;
			else
				break;
		}

		string = string.substr(0, end);

		//if(pointPosition + decimals < int(string.size()))
		//	string = string.substr(0, pointPosition + decimals + 1);
	}

	setDialogItemText(dialog, id, string);
}

std::string getDialogItemText(Dialog &dialog, int id)
{
	char buffer[256] = { 0 };
	GetDlgItemText(dialog.getWindowHandle(), id, buffer, 255);

	return buffer;
}

int getDialogItemInt(Dialog &dialog, int id)
{
	return GetDlgItemInt(dialog.getWindowHandle(), id, 0, FALSE);
}

float getDialogItemFloat(Dialog &dialog, int id)
{
	std::string string = getDialogItemText(dialog, id);
	return convertFromString<float>(string, 0);
}

void setSliderRange(Dialog &dialog, int id, int min, int max)
{
	SendDlgItemMessage(dialog.getWindowHandle(), id, TBM_SETRANGE, TRUE, MAKELONG(min, max));
}

void setSliderValue(Dialog &dialog, int id, int value)
{
	SendDlgItemMessage(dialog.getWindowHandle(), id, TBM_SETPOS, TRUE, value);
}

int getSliderValue(Dialog &dialog, int id)
{
	return SendDlgItemMessage(dialog.getWindowHandle(), id, TBM_GETPOS, 0, 0);
}

void resetComboBox(Dialog &dialog, int id)
{
	SendDlgItemMessage(dialog.getWindowHandle(), id, CB_RESETCONTENT, 0, 0);
}

void addComboString(Dialog &dialog, int id, const std::string &string)
{
	SendDlgItemMessage(dialog.getWindowHandle(), id, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
}

void setComboIndex(Dialog &dialog, int id, int index)
{
	SendDlgItemMessage(dialog.getWindowHandle(), id, CB_SETCURSEL, index, 0);
}

int getComboIndex(Dialog &dialog, int id)
{
	return SendDlgItemMessage(dialog.getWindowHandle(), id, CB_GETCURSEL, 0, 0);
}

void enableCheck(Dialog &dialog, int id, bool enable)
{
	if(enable)
		CheckDlgButton(dialog.getWindowHandle(), id, BST_CHECKED);
	else
		CheckDlgButton(dialog.getWindowHandle(), id, BST_UNCHECKED);
}

bool isCheckEnabled(Dialog &dialog, int id)
{
	return IsDlgButtonChecked(dialog.getWindowHandle(), id) == BST_CHECKED;
}

void enableDialogItem(Dialog &dialog, int id, bool enableState)
{
	int state = (enableState) ? TRUE : FALSE;
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), id), state);
}

} // end of namespace editor
} // end of namespace frozenbyte
