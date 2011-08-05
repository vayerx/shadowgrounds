
#include <windows.h>
#include "resource.h"
#include "PhysicsDialogHandler.h"

namespace frozenbyte
{
namespace launcher
{

PhysicsDialogHandler::PhysicsDialogHandler( HWND parent )
{
	dialogData = new DialogData( IDD_PHYSICSDIALOG, parent, DialogData::ATTACH_NONE, this );
}

PhysicsDialogHandler::~PhysicsDialogHandler( )
{
	delete dialogData;
}

void PhysicsDialogHandler::initDialog()
{

	HWND hwnd = dialogData->getHwnd();

	// Load localizations texts.
	setDescriptionText ( GetDlgItem( hwnd, IDC_PSTATIC1));
	setDescriptionText ( GetDlgItem( hwnd, IDC_PSTATIC2));
	setDescriptionText ( GetDlgItem( hwnd, IDC_PCHECK1));
	setDescriptionText ( GetDlgItem( hwnd, IDC_PCHECK2));

	// Load combobox items.
	addComboItems( GetDlgItem( hwnd, IDC_PCOMBO1 ), manager.getOptionNames( "Physics Quality" ), manager.getTheOneInUse( "Physics Quality" ) );

}

void PhysicsDialogHandler::loadOptions( )
{

	HWND hwnd = dialogData->getHwnd();

	setCheckBox( hwnd, IDC_PCHECK1, manager.getTheOneInUse("Physics Hardware") );
	setCheckBox( hwnd, IDC_PCHECK2, manager.getTheOneInUse("MultipleInputDevices") );

	setComboBoxSelection( GetDlgItem( hwnd, IDC_PCOMBO1 ), manager.getTheOneInUse( "Physics Quality" ) );

}

void PhysicsDialogHandler::applyOptions( )
{

	HWND hwnd = dialogData->getHwnd();

	manager.applyOptions( "Physics Hardware", getCheckBoxValue(	hwnd, IDC_PCHECK1 ) );
	manager.applyOptions( "MultipleInputDevices", getCheckBoxValue(	hwnd, IDC_PCHECK2 ) );
	manager.applyOptions( "Physics Quality", getComboBoxSelection( GetDlgItem( hwnd, IDC_PCOMBO1 ) ) );

}

BOOL PhysicsDialogHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	return FALSE;
}

}
}