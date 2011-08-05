
#include <windows.h>
#include "window.h"

#include "../game/DHLocaleManager.h"
#include "options_value_manager.h"
#include "LauncherMainDialog.h"
#include "LauncherWindow.h"

namespace frozenbyte
{
	namespace launcher
	{

		class LauncherWindowImpl
		{
		public:

			Window window;
			LauncherMainDialog dialog;

			LauncherWindowImpl( LauncherWindow *a_LauncherWindow ) :
				window( ::game::getLocaleGuiString( "launcher_window_caption" ) , 0, false, true, 500, 500 ),
				dialog( window, a_LauncherWindow )
			{
				int xs = 0;
				int ys = 0;
				dialog.getSize(xs, ys);
				window.setSize(xs, ys);
	
				dialog.show();

			};
		};


		LauncherWindow::LauncherWindow( ) : impl ( NULL )
		{
			if( !impl )
				impl = new LauncherWindowImpl( this );

		}

		LauncherWindow::~LauncherWindow( )
		{
			if( impl )
				delete impl;
		}


	};
};

