#include "launcher_window.h"
#include "window.h"
#include "launch_dialog.h"
#include "advanced_dialog.h"
#include "Resource/resource.h"

namespace frozenbyte {
namespace launcher {

//#ifndef FB_RU_HAX
//	const int			windowWidth = 315;
//	const int			windowHeight = 345;
//#else
	const int			windowWidth = 410;
	const int			windowHeight = 345;
//#endif

const std::string	windowTitle = "Jack Claw Launcher";


///////////////////////////////////////////////////////////////////////////////

class LauncherWindowImpl
{
public:
	LauncherWindowImpl( LauncherWindow* lwindow ) :
	  window( windowTitle, IDI_ICON2 , false, true, windowWidth, windowHeight ),
	  dialog( window, lwindow ),
	  advanced( window, lwindow )
	{
		int xs = 0;
		int ys = 0;
		dialog.getSize(xs, ys);
		window.setSize(xs, ys);

		dialog.show();
	}

	~LauncherWindowImpl() 
	{
	}

	void openAdvanced()
	{	
		dialog.hide();
		advanced.show();
	}

	void closeAdvanced()
	{
		advanced.hide();
		dialog.show();
	}

	Window			window;
	LaunchDialog	dialog;
	AdvancedDialog	advanced;
};

///////////////////////////////////////////////////////////////////////////////

LauncherWindow::LauncherWindow()
{
	impl = new LauncherWindowImpl( this );
}

//=============================================================================

LauncherWindow::~LauncherWindow()
{
	delete impl;
}

///////////////////////////////////////////////////////////////////////////////

void LauncherWindow::openAdvanced()
{
	impl->openAdvanced();
}

//=============================================================================

void LauncherWindow::closeAdvanced()
{
	impl->closeAdvanced();
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace launcher
} // end of namespace frozenbyte