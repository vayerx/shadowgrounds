// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "color_component.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <cassert>

namespace frozenbyte {
namespace editor {
namespace {
	static const char *windowClassName = "color component";
}

struct ColorComponentData
{
	HWND windowHandle;
	HWND parentHandle;

	std::string className;
	WNDCLASSEX windowClass;

	int xPosition;
	int yPosition;
	int xSize;
	int ySize;

	unsigned int color;
	static int instanceCounter;

	ColorComponentData(HWND parentHandle_)
	{
		windowHandle = 0;
		parentHandle = parentHandle_;
		ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

		xPosition = yPosition = 0;
		xSize = ySize = 10;

		color = 0;
		++instanceCounter;
	}

	~ColorComponentData()
	{
		freeResources();
	}

	void setPlacement(int xPosition_, int yPosition_, int xSize_, int ySize_)
	{
		xPosition = xPosition_;
		yPosition = yPosition_;
		xSize = xSize_;
		ySize = ySize_;
	}

	void createWindowClass(int color)
	{
		// Protect against mr. Murphy. We need truly unique name
		className = windowClassName;
		className += boost::lexical_cast<std::string> (instanceCounter);
		className += " ";
		className += boost::lexical_cast<std::string> (color);

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = 0;
		windowClass.lpfnWndProc = &DefWindowProc;
		windowClass.hInstance = GetModuleHandle(0);
		windowClass.hIcon = 0;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = CreateSolidBrush(color);
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = className.c_str();
		windowClass.hIconSm = 0;

		RegisterClassEx(&windowClass);
	}

	void createWindow(int color_)
	{
		color = color_;

		freeResources();
		createWindowClass(color);

		int windowStyle = WS_CHILD|WS_BORDER;
		windowHandle = CreateWindowEx(0, className.c_str(), 0, windowStyle, xPosition, yPosition, xSize, ySize, parentHandle, 0, GetModuleHandle(0), this);
		ShowWindow(windowHandle, SW_SHOW);
	}

	void freeResources()
	{
		if(windowHandle)
		{
			DestroyWindow(windowHandle);
			UnregisterClass(className.c_str(), GetModuleHandle(0));
		}
	}
};

int ColorComponentData::instanceCounter = 0;

ColorComponent::ColorComponent(HWND parentHandle, int xPosition, int yPosition, int xSize, int ySize)
{
	boost::scoped_ptr<ColorComponentData> tempData(new ColorComponentData(parentHandle));
	tempData->setPlacement(xPosition, yPosition, xSize, ySize);
	tempData->createWindow(RGB(255,255,255));

	data.swap(tempData);
}

ColorComponent::~ColorComponent()
{
}

unsigned int ColorComponent::getColor() const
{
	return data->color;
}

void ColorComponent::setColor(unsigned int color)
{
	if(color == data->color)
		return;

	data->createWindow(color);
}

} // end of namespace editor
} // end of namespace frozenbyte
