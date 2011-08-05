// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "hmap_loader.h"
#include "resource/resource.h"
#include <windows.h>
#include <cassert>
#include <fstream>

#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"

#include "../system/Logger.h"

namespace frozenbyte {
namespace editor {
namespace {
	BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		HmapFormat *data = reinterpret_cast<HmapFormat *> (GetWindowLong(windowHandle, GWL_USERDATA));

		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			HmapFormat *data = reinterpret_cast<HmapFormat *> (lParam);

			// Defaults
			SetDlgItemInt(windowHandle, IDC_WIDTH, 512, FALSE);
			SetDlgItemInt(windowHandle, IDC_HEIGHT, 512, FALSE);

			SendDlgItemMessage(windowHandle, IDC_HMAP_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("8-bit integer"));
			SendDlgItemMessage(windowHandle, IDC_HMAP_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("16-bit integer"));
			SendDlgItemMessage(windowHandle, IDC_HMAP_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("24-bit integer"));
			SendDlgItemMessage(windowHandle, IDC_HMAP_TYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("32-bit float"));
			SendDlgItemMessage(windowHandle, IDC_HMAP_TYPE, CB_SETCURSEL, 0, 0);
		}
		else if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY)
				EndDialog(windowHandle, 0);

			if(command == IDC_HMAP_OK)
			{
				data->width = GetDlgItemInt(windowHandle, IDC_WIDTH, 0, FALSE);
				data->height = GetDlgItemInt(windowHandle, IDC_HEIGHT, 0, FALSE);
				data->format = SendDlgItemMessage(windowHandle, IDC_HMAP_TYPE, CB_GETCURSEL, 0, 0);

				EndDialog(windowHandle, 1);
			}
		}

		return 0;
	}

	HmapFormat getFormat()
	{
		HmapFormat format;
		DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_HMAP), 0, DialogHandler, reinterpret_cast<LPARAM> (&format));

		return format;
	}

	void readData(std::vector<unsigned short> *data, const std::string &fileName, HmapFormat &format)
	{
		assert(data);
		std::ifstream file(fileName.c_str(), std::ios::binary);

		for(int y = 0; y < format.height; ++y)
		for(int x = 0; x < format.width; ++x)
		{
			if(!file.is_open())
			{
				Logger::getInstance()->error("hmap_loader - readData - file doesn't exist");
				//throw std::runtime_error("file doesn't exist");
			}
			if(file.fail() || file.eof())
			{
				Logger::getInstance()->error("hmap_loader - readData - error reading file");
				//throw std::runtime_error("error reading file");
			}

			int index = y * format.width + x;
			unsigned short &result = (*data)[index];

			if(format.format == 0) // 8-bit int
			{
				char data = 0;
				file.read(&data, 1);

				unsigned char byte = data;
				result = byte * 256 / 2;
			}
			else if(format.format == 1) // 16-bit int
			{
				file.read(reinterpret_cast<char *> (&result), 2);
				int a = result;
			}
			else if(format.format == 2) // 24-bit int
			{
				char b[3];
				file.read(b, 3);

				result = (b[0] << 16) + (b[1] << 8) + b[2];
			}
			else if(format.format == 3) // 32-bit float (normalized)
			{
				float data = 0;
				file.read(reinterpret_cast<char *> (&data), 4);

				result = static_cast<short> (data * 65535);
			}
			else
			{
				assert(!"Crash boom bang.");
			}
		}
	}
	
	void flipData(std::vector<unsigned short> *data, HmapFormat &format)
	{
		for(int y = 0; y < format.height / 2; ++y)
		for(int x = 0; x < format.width; ++x)
		{
			int p1 = y * format.width + x;
			int p2 = (format.height - y - 1) * format.width + x;
			
			std::swap((*data)[p1], (*data)[p2]);
		}
	}

	void getAllData(std::vector<unsigned short> *data, const std::string &fileName, HmapFormat &format)
	{
		data->resize(format.height * format.width);
		
		readData(data, fileName, format);
		flipData(data, format);
	}

} // end of unnamed namespace

HmapLoader::HmapLoader()
{
}

HmapLoader::HmapLoader(const std::string &fileName_)
{
	format = getFormat();
	fileName = fileName_;

	/*
	try
	{
	*/
		getAllData(&data, fileName, format);
	/*
	}
	catch(std::runtime_error &)
	{
		data = std::vector<unsigned short> ();
		format = HmapFormat();
		
		fileName = "";
	}
	*/
}

HmapLoader::~HmapLoader()
{
}

void HmapLoader::update(unsigned short *buffer)
{
	for(int y = 0; y < format.height; ++y)
	for(int x = 0; x < format.width; ++x)
	{
		int index = y * format.width + x;
		data[index] = buffer[index];
	}
}

void HmapLoader::smooth()
{
	int x = 0;
	int y = 0;

	for(y = 0; y < format.height; ++y)
	for(x = 0; x < format.width - 1; ++x)
	{
		int value1 = data[y * format.width + x];
		int value2 = data[y * format.width + x + 1];

		data[y * format.width + x] = (value1 + value2) / 2;
	}

	for(y = 0; y < format.height - 1; ++y)
	for(x = 0; x < format.width; ++x)
	{
		int value1 = data[y * format.width + x];
		int value2 = data[y * format.width + x + format.width];

		data[y * format.width + x] = (value1 + value2) / 2;
	}
}

std::string HmapLoader::getFileName()
{
	return fileName;
}

std::vector<unsigned short> &HmapLoader::getData()
{
	return data;
}

int HmapLoader::getWidth() const
{
	return format.width;
}

int HmapLoader::getHeight() const
{
	return format.height;
}

filesystem::OutputStream &HmapLoader::writeStream(filesystem::OutputStream &stream) const
{
	stream << fileName;
	stream << format.format << format.width << format.height;

	for(int y = 0; y < format.height; ++y)
	for(int x = 0; x < format.width; ++x)
		stream << data[y * format.width + x];

	return stream;
}

filesystem::InputStream &HmapLoader::readStream(filesystem::InputStream &stream)
{
	stream >> fileName;
	stream >> format.format >> format.width >> format.height;

	data.clear();
	data.resize(format.width * format.height);

	//for(int y = 0; y < format.height; ++y)
	//for(int x = 0; x < format.width; ++x)
	//	stream >> data[y * format.width + x];
	stream.read(&data[0], format.width * format.height);

	//flipData(&data, format);
	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
