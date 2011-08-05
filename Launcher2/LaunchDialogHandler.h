
#pragma once

#include "options_value_manager.h"
#include "dlghandlerimpl.h"
#include "dialog_data.h"

namespace frozenbyte
{

	namespace launcher
	{

		class LaunchDialogHandler : public DlgHandlerImpl
		{
			OptionsValueManager manager;
		public:

			 LaunchDialogHandler( HWND parent );
			~LaunchDialogHandler( );

			void updateInfoText( );

			void initDialog( );
			void applyOptions( );
			void loadOptions( );

			static void launchGame( );

			BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		};

	}

}
