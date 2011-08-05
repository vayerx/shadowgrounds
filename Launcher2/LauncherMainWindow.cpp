
#include <windows.h>
#include "window.h"

#include "LauncherMainDialog.h"
#include "LauncherMainWindow.h"

namespace frozenbyte
{
	namespace launcher
	{

		class LauncherMainWindowImpl
		{
		public:

			Window window;
			LauncherMainDialog dialog;

			LauncherMainWindowImpl( LauncherMainWindow *a_launcherMainWindow ) :
				window( "Shadowgrounds survivor launcher", 0, false, true, 500, 500 ),
				dialog( window, a_launcherMainWindow )
			{
				int xs = 0;
				int ys = 0;
				dialog.getSize(xs, ys);
				window.setSize(xs, ys);
	
				dialog.show();
			};
		};


		LauncherMainWindow::LauncherMainWindow( ) : impl ( NULL )
		{
			if( !impl )
				impl = new LauncherMainWindowImpl( this );

		}

		LauncherMainWindow::~LauncherMainWindow( )
		{
			if( impl )
				delete impl;
		}


	};
};

