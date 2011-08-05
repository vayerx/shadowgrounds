
#include "precompiled.h"

#include "BlackEdgeWindow.h"

#include "../ogui/Ogui.h"
#include "../util/Debug_MemoryManager.h"


namespace ui
{

	BlackEdgeWindow::BlackEdgeWindow(Ogui *ogui)
	{ 
		this->ogui = ogui;
#ifdef LEGACY_FILES
		win = ogui->CreateSimpleWindow(0, 0, 1024, 128, "Data/GUI/Windows/black_edge.dds");
#else
		win = ogui->CreateSimpleWindow(0, 0, 1024, 128, "data/gui/common/window/black_edge.tga");
#endif
		win->SetUnmovable();
		win->Raise();
	}


	BlackEdgeWindow::~BlackEdgeWindow()
	{
		delete win;
	}


	void BlackEdgeWindow::moveTo(int x, int y)
	{
		win->MoveTo(x, y);
	}


	void BlackEdgeWindow::update()
	{
		win->Raise();
	}

}

