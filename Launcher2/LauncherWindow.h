
#pragma once

namespace frozenbyte
{
	namespace launcher
	{

		class LauncherWindowImpl;

		class LauncherWindow
		{
		public:

			LauncherWindow( );
			~LauncherWindow( );
		
		private:

			LauncherWindowImpl *impl;

		};
	}
}
