
#pragma once

#include "options_value_manager.h"
#include "dlghandlerimpl.h"
#include "dialog_data.h"

namespace frozenbyte
{

	namespace launcher
	{

		class UpdateDialogHandler : public DlgHandlerImpl
		{
			OptionsValueManager manager;
		public:

			 UpdateDialogHandler( HWND parent );
			~UpdateDialogHandler( );

			void updateInfoText( HWND );

			void initDialog( );
			void applyOptions( );
			void loadOptions( );

			BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		};

	}

}
