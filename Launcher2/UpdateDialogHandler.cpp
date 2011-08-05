

#include <windows.h>
#include "resource.h"
#include "UpdateDialogHandler.h"

namespace frozenbyte
{
namespace launcher
{

UpdateDialogHandler::UpdateDialogHandler( HWND parent )
{
	dialogData = new DialogData( IDD_UPDATESDIALOG, parent, DialogData::ATTACH_NONE, this );
}

UpdateDialogHandler::~UpdateDialogHandler( )
{
	delete dialogData;
}

void UpdateDialogHandler::initDialog( )
{
	HWND hwnd = dialogData->getHwnd();

	// Load localizations texts.
	setDescriptionText ( GetDlgItem( hwnd, IDC_USTATIC1 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_USTATIC2 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_UBUTTON1 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_UCHECK1 ) );


	// Load combobox items.

}

void UpdateDialogHandler::loadOptions( )
{
	HWND hwnd = dialogData->getHwnd();
//	setCheckBox( hwnd, IDC_UCHECK1, manager.getTheOneInUse("") );
}

void UpdateDialogHandler::applyOptions( )
{

}

BOOL UpdateDialogHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

}
}
