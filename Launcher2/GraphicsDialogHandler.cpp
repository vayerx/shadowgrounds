
#include <windows.h>
#include "resource.h"
#include "GraphicsDialogHandler.h"

namespace frozenbyte
{
namespace launcher
{

GraphicsDialogHandler::GraphicsDialogHandler( HWND parent )
{
	dialogData = new DialogData( IDD_GRAPHICSDIALOG, parent, DialogData::ATTACH_NONE, this );
}

GraphicsDialogHandler::~GraphicsDialogHandler( )
{
	delete dialogData;
}

void GraphicsDialogHandler::initDialog( )
{

	HWND hwnd = dialogData->getHwnd();

	// Load localizations texts.
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL1 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL2 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL3 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL4 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL5 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL6 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL7 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL8 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL9 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_LABEL10) );

	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK1 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK2 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK3 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK4 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK5 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK6 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK7 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK8 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK9 ) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK10) );
	setDescriptionText ( GetDlgItem( hwnd, IDC_CHECK11) );

	setDescriptionText ( GetDlgItem( hwnd, IDC_GROUPBOX));

	// Load combobox items.
	addComboItems( GetDlgItem( hwnd, IDC_COMBO1 ), manager.getOptionNames( "Fake Shadows Texture Quality" ), manager.getTheOneInUse( "Fake Shadows Texture Quality" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO2 ), manager.getOptionNames( "Misc" ), manager.getTheOneInUse( "Misc" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO3 ), manager.getOptionNames( "Lighting Level" ), manager.getTheOneInUse( "Lighting Level" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO4 ), manager.getOptionNames( "Anisotropic Filtering" ), manager.getTheOneInUse( "Anisotropic Filtering" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO5 ), manager.getOptionNames( "Lighting Texture Quality" ), manager.getTheOneInUse( "Lighting Texture Quality" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO6 ), manager.getOptionNames( "Antialiasing" ), manager.getTheOneInUse( "Antialiasing" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO7 ), manager.getOptionNames( "Particle Effects Level" ), manager.getTheOneInUse( "Particle Effects Level" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO8 ), manager.getOptionNames( "Shadows Texture Quality" ), manager.getTheOneInUse( "Shadows Texture Quality" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO9 ), manager.getOptionNames( "Shadows Level" ), manager.getTheOneInUse( "Shadows Level" ) );
	addComboItems( GetDlgItem( hwnd, IDC_COMBO10), manager.getOptionNames( "Texture Detail Level" ), manager.getTheOneInUse( "Texture Detail Level" ) );

}

void GraphicsDialogHandler::loadOptions( )
{
	HWND hwnd = dialogData->getHwnd();

	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO1 ), manager.getTheOneInUse( "Fake Shadows Texture Quality" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO2 ), manager.getTheOneInUse( "Misc" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO3 ), manager.getTheOneInUse( "Lighting Level" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO4 ), manager.getTheOneInUse( "Anisotropic Filtering" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO5 ), manager.getTheOneInUse( "Lighting Texture Quality" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO6 ), manager.getTheOneInUse( "Antialiasing" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO7 ), manager.getTheOneInUse( "Particle Effects Level" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO8 ), manager.getTheOneInUse( "Shadows Texture Quality" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO9 ), manager.getTheOneInUse( "Shadows Level" ) );
	setComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO10), manager.getTheOneInUse( "Texture Detail Level" ) );

	setCheckBox( hwnd, IDC_CHECK1, manager.getTheOneInUse("Fullscreen") );
	setCheckBox( hwnd, IDC_CHECK2, manager.getTheOneInUse("Weather Effects") );
	setCheckBox( hwnd, IDC_CHECK3, manager.getTheOneInUse("High Quality Lightmap") );
	setCheckBox( hwnd, IDC_CHECK4, manager.getTheOneInUse("Extra Gamma Effects") );
	setCheckBox( hwnd, IDC_CHECK5, manager.getTheOneInUse("Render Glow") );
	setCheckBox( hwnd, IDC_CHECK6, manager.getTheOneInUse("DistortionEffects") );
	setCheckBox( hwnd, IDC_CHECK7, manager.getTheOneInUse("CorpseDisappear") );
	setCheckBox( hwnd, IDC_CHECK8, manager.getTheOneInUse("High Quality Video") );
	setCheckBox( hwnd, IDC_CHECK9, manager.getTheOneInUse("ResetAfterLoad") );
	setCheckBox( hwnd, IDC_CHECK10,manager.getTheOneInUse("EnableMenuVideo") );
	setCheckBox( hwnd, IDC_CHECK11,manager.getTheOneInUse("VSync") );

}

void GraphicsDialogHandler::applyOptions( )
{
	
	HWND hwnd = dialogData->getHwnd();

	manager.applyOptions( "Fake Shadows Texture Quality", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO1 ) ) );
	manager.applyOptions( "Misc", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO2 ) ) );
	manager.applyOptions( "Lighting Level", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO3 ) ) );
	manager.applyOptions( "Anisotropic Filtering", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO4 ) ) );
	manager.applyOptions( "Lighting Texture Quality", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO5 ) ) );
	manager.applyOptions( "Antialiasing", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO6 ) ) );
	manager.applyOptions( "Particle Effects Level", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO7 ) ) );
	manager.applyOptions( "Shadows Texture Quality", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO8 ) ) );
	manager.applyOptions( "Shadows Level", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO9 ) ) );
	manager.applyOptions( "Texture Detail Level", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBO10) ) );

	manager.applyOptions( "Fullscreen", getCheckBoxValue(					hwnd, IDC_CHECK1 ) );
	manager.applyOptions( "Weather Effects", getCheckBoxValue(			hwnd, IDC_CHECK2 ) );
	manager.applyOptions( "High Quality Lightmap", getCheckBoxValue(	hwnd, IDC_CHECK3 ) );
	manager.applyOptions( "Extra Gamma Effects", getCheckBoxValue(		hwnd, IDC_CHECK4 ) );
	manager.applyOptions( "Render Glow", getCheckBoxValue(				hwnd, IDC_CHECK5 ) );
	manager.applyOptions( "DistortionEffects", getCheckBoxValue(		hwnd, IDC_CHECK6 ) );
	manager.applyOptions( "CorpseDisappear", getCheckBoxValue(			hwnd, IDC_CHECK7 ) );
	manager.applyOptions( "High Quality Video", getCheckBoxValue(		hwnd, IDC_CHECK8 ) );
	manager.applyOptions( "ResetAfterLoad", getCheckBoxValue(			hwnd, IDC_CHECK9 ) );
	manager.applyOptions( "EnableMenuVideo", getCheckBoxValue(			hwnd, IDC_CHECK10) );
	manager.applyOptions( "VSync", getCheckBoxValue(						hwnd, IDC_CHECK11) );

}

BOOL GraphicsDialogHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

}
}