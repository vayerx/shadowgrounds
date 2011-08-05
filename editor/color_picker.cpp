// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "color_picker.h"
#include "color_component.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"

#include <windows.h>
#include <commctrl.h>
#include "resource/resource.h"

namespace frozenbyte {
namespace editor {
namespace {
	COLORREF customColors[16] = 
	{ 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255), 
		RGB(255,255,255)
	};
} // unnamed

struct ColorPickerData
{
	unsigned char r;
	unsigned char g;
	unsigned char b;

	ColorComponent *colorComponent;
	bool disableUpdate;

	ColorPickerData()
	{
		r = 0;
		g = 0;
		b = 0;

		colorComponent = 0;
		disableUpdate = false;
	}

	~ColorPickerData()
	{
	}

	void updateColor()
	{
		if(colorComponent)
			colorComponent->setColor(RGB(r, g, b));

	}

	static BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		ColorPickerData *data = reinterpret_cast<ColorPickerData *> (GetWindowLong(windowHandle, GWL_USERDATA));

		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			data = reinterpret_cast<ColorPickerData *> (lParam);

			data->disableUpdate = true;
			SendDlgItemMessage(windowHandle, IDC_COLOR_R, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
			SendDlgItemMessage(windowHandle, IDC_COLOR_G, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
			SendDlgItemMessage(windowHandle, IDC_COLOR_B, TBM_SETRANGE, TRUE, MAKELONG(0, 255));

			SendDlgItemMessage(windowHandle, IDC_COLOR_R, TBM_SETPOS, TRUE, data->r);
			SendDlgItemMessage(windowHandle, IDC_COLOR_G, TBM_SETPOS, TRUE, data->g);
			SendDlgItemMessage(windowHandle, IDC_COLOR_B, TBM_SETPOS, TRUE, data->b);

			SetDlgItemInt(windowHandle, IDC_COLOR_RT, data->r, FALSE);
			SetDlgItemInt(windowHandle, IDC_COLOR_GT, data->g, FALSE);
			SetDlgItemInt(windowHandle, IDC_COLOR_BT, data->b, FALSE);

			data->colorComponent = new ColorComponent(windowHandle, 194, 10, 66, 22);
			data->updateColor();
			data->disableUpdate = false;		
		}
		else if(message == WM_NOTIFY)
		{
			LPNMHDR notification = reinterpret_cast<LPNMHDR> (lParam);
			if(notification->code == NM_CUSTOMDRAW && !data->disableUpdate)
			{
				data->r = unsigned char(SendDlgItemMessage(windowHandle, IDC_COLOR_R, TBM_GETPOS, 0, 0));
				data->g = unsigned char(SendDlgItemMessage(windowHandle, IDC_COLOR_G, TBM_GETPOS, 0, 0));
				data->b = unsigned char(SendDlgItemMessage(windowHandle, IDC_COLOR_B, TBM_GETPOS, 0, 0));

				data->disableUpdate = true;
				SetDlgItemInt(windowHandle, IDC_COLOR_RT, data->r, FALSE);
				SetDlgItemInt(windowHandle, IDC_COLOR_GT, data->g, FALSE);
				SetDlgItemInt(windowHandle, IDC_COLOR_BT, data->b, FALSE);
				data->disableUpdate = false;

				data->updateColor();
			}
		}
		else if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY)
			{
				delete data->colorComponent;
				data->colorComponent = 0;

				EndDialog(windowHandle, 0);
			}

			if(command == IDC_CPICK_OK)
			{
				delete data->colorComponent;
				data->colorComponent = 0;

				EndDialog(windowHandle, 1);
			}

			if(command == IDC_COLOR_RT || command == IDC_COLOR_GT || command == IDC_COLOR_BT)
			if(!data->disableUpdate)
			{
				data->r = GetDlgItemInt(windowHandle, IDC_COLOR_RT, 0, FALSE);
				data->g = GetDlgItemInt(windowHandle, IDC_COLOR_GT, 0, FALSE);
				data->b = GetDlgItemInt(windowHandle, IDC_COLOR_BT, 0, FALSE);

				data->disableUpdate = true;
				SendDlgItemMessage(windowHandle, IDC_COLOR_R, TBM_SETPOS, TRUE, data->r);
				SendDlgItemMessage(windowHandle, IDC_COLOR_G, TBM_SETPOS, TRUE, data->g);
				SendDlgItemMessage(windowHandle, IDC_COLOR_B, TBM_SETPOS, TRUE, data->b);
				data->disableUpdate = false;

				data->updateColor();
			}
		}

		return 0;
	}

};

ColorPicker::ColorPicker()
{
	boost::scoped_ptr<ColorPickerData> tempData(new ColorPickerData());
	data.swap(tempData);
}

ColorPicker::~ColorPicker()
{
}

bool ColorPicker::run(int color)
{
	data->r = GetRValue(color);
	data->g = GetGValue(color);
	data->b = GetBValue(color);

	//if(DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_COLORPICK), 0, ColorPickerData::DialogHandler, reinterpret_cast<LPARAM> (data.get())) == 1)
	//	return true;

	CHOOSECOLOR colorData = { 0 };

	colorData.lStructSize = sizeof(CHOOSECOLOR);
	colorData.rgbResult = color;
	colorData.lpCustColors = customColors;
	colorData.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(ChooseColor(&colorData))
	{
		data->r = GetRValue(colorData.rgbResult);
		data->g = GetGValue(colorData.rgbResult);
		data->b = GetBValue(colorData.rgbResult);

		return true;
	}

	return false;
}

int ColorPicker::getColor()
{
	return RGB(data->r, data->g, data->b);
}

void readColors(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;

	if(!version)
		return;

	for(int i = 0; i < 16; ++i)
	{
		unsigned int value = 0;
		stream >> value;

		customColors[i] = value;
	}
}

void writeColors(filesystem::OutputStream &stream)
{
	stream << int(1);

	for(int i = 0; i < 16; ++i)
		stream << unsigned int(customColors[i]);
}

} // end of namespace editor
} // end of namespace frozenbyte
