
#include "precompiled.h"

#include "MovieAspectWindow.h"
#include "BlackEdgeWindow.h"

#include "../util/Debug_MemoryManager.h"

namespace ui
{

	MovieAspectWindow::MovieAspectWindow(Ogui *ogui)
	{
		top = new BlackEdgeWindow(ogui);
		bottom = new BlackEdgeWindow(ogui);
		bottom->moveTo(0, 768 - 128);
	}

	
	MovieAspectWindow::~MovieAspectWindow()
	{
    delete top;
		delete bottom;
	}


	void MovieAspectWindow::update()
	{
		top->update();
		bottom->update();
	}

}


