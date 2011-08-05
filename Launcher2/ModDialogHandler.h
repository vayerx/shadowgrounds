

#pragma once

#include "options_value_manager.h"
#include "dlghandlerimpl.h"
#include "dialog_data.h"

namespace frozenbyte
{

	namespace launcher
	{

		class ModDialogHandler : public DlgHandlerImpl
		{
			OptionsValueManager manager;
		public:

			 ModDialogHandler( HWND parent );
			~ModDialogHandler( );

			void initDialog( );
			void applyOptions( );
			void loadOptions( );

			void setActivateButton( bool );	// Changes activatebutton's text.
			void loadMods();

			BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		};

	}

}


