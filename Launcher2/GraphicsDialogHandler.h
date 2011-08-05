
#pragma once

#include "options_value_manager.h"
#include "dlghandlerimpl.h"
#include "dialog_data.h"

namespace frozenbyte
{

	namespace launcher
	{
		

		class GraphicsDialogHandler : public DlgHandlerImpl
		{
			OptionsValueManager manager;
		public:

			 GraphicsDialogHandler( HWND parent );
			~GraphicsDialogHandler( );

			void initDialog( );
			void applyOptions( );
			void loadOptions( );

			BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		};

	}

}

