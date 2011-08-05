
#include <windows.h>
#include "resource.h"
#include "SoundsDialogHandler.h"

namespace frozenbyte
{
namespace launcher
{

SoundsDialogHandler::SoundsDialogHandler( HWND parent )
{
	dialogData = new DialogData( IDD_SOUNDSDIALOG, parent, DialogData::ATTACH_NONE, this );
}

SoundsDialogHandler::~SoundsDialogHandler( )
{
	delete dialogData;
}

void SoundsDialogHandler::initDialog( )
{

	HWND hwnd = dialogData->getHwnd();

	// Load localizations texts.
	setDescriptionText ( GetDlgItem( hwnd, IDC_SLABEL1 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_SLABEL2 ) );

	setDescriptionText ( GetDlgItem( hwnd, IDC_SCHECK1 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_SCHECK2 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_SCHECK3 ) );

	setDescriptionText ( GetDlgItem( hwnd, IDC_GROUPBOXSOUNDS));

	// Load combobox items.
	addComboItems( GetDlgItem( hwnd, IDC_SCOMBO1 ), manager.getOptionNames( "SpeakerType" ), manager.getTheOneInUse( "SpeakerType" ) );
	addComboItems( GetDlgItem( hwnd, IDC_SCOMBO2 ), manager.getOptionNames( "MixingRate" ), manager.getTheOneInUse( "MixingRate" ) );

}

void SoundsDialogHandler::loadOptions( )
{

	HWND hwnd = dialogData->getHwnd();

	setComboBoxSelection( GetDlgItem( hwnd, IDC_SCOMBO1 ), manager.getTheOneInUse( "SpeakerType" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_SCOMBO2 ), manager.getTheOneInUse( "MixingRate" ) );

	setCheckBox( hwnd, IDC_SCHECK1, manager.getTheOneInUse("Sounds") );
	setCheckBox( hwnd, IDC_SCHECK2, manager.getTheOneInUse("Hardware3D") );
	setCheckBox( hwnd, IDC_SCHECK3, manager.getTheOneInUse("Eax") );

}

void SoundsDialogHandler::applyOptions( )
{
	HWND hwnd = dialogData->getHwnd();

	manager.applyOptions( "SpeakerType", getComboBoxSelection( GetDlgItem( hwnd, IDC_SCOMBO1 ) ) );
	manager.applyOptions( "MixingRate", getComboBoxSelection( GetDlgItem( hwnd, IDC_SCOMBO2 ) ) );

	manager.applyOptions( "Sounds", getCheckBoxValue(					hwnd, IDC_SCHECK1 ) );
	manager.applyOptions( "Hardware3D", getCheckBoxValue(			hwnd, IDC_SCHECK2 ) );
	manager.applyOptions( "Eax", getCheckBoxValue(	hwnd, IDC_SCHECK3 ) );

}

BOOL SoundsDialogHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

}
}