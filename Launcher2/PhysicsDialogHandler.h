
#pragma once

#include "options_value_manager.h"
#include "dlghandlerimpl.h"
#include "dialog_data.h"

namespace frozenbyte
{

	namespace launcher
	{

		class PhysicsDialogHandler : public DlgHandlerImpl
		{
			OptionsValueManager manager;
		public:

			 PhysicsDialogHandler( HWND parent );
			~PhysicsDialogHandler( );

			void initDialog( );
			void applyOptions( );
			void loadOptions( );

			BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		};

	}

}

