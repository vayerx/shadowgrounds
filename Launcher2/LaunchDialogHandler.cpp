
#include <windows.h>
#include "resource.h"
#include "LaunchDialogHandler.h"
#include "../game/GameOptionManager.h"
#include "../util/mod_selector.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_all.h"
#include "../survivor/version.h"
#include <boost/lexical_cast.hpp>

namespace frozenbyte
{
namespace launcher
{

LaunchDialogHandler::LaunchDialogHandler( HWND parent )
{
	dialogData = new DialogData( IDD_LAUNCHDIALOG, parent, DialogData::ATTACH_NONE, this );
}

LaunchDialogHandler::~LaunchDialogHandler( )
{
	delete dialogData;
}

extern util::ModSelector modSelector;

void LaunchDialogHandler::updateInfoText( ) 
{
	HWND hwnd_text = GetDlgItem( dialogData->getHwnd(), IDC_INFOTEXT );

	int modIndex = modSelector.getActiveIndex();

	std::string text;
	text += get_application_name_string();
	text += "\nversion ";
	text += get_version_string();
	text += "\n";
	text += "Mod: ";
	if(modIndex >= 0)
	{
		std::string modDir = modSelector.getDescription(modIndex);
		text += modDir;
	}
	
	SendMessage( hwnd_text, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)text.c_str() );

}

void LaunchDialogHandler::initDialog( )
{

	// Load localizations.

	HWND dialogHwnd = dialogData->getHwnd();

	setDescriptionText ( GetDlgItem( dialogHwnd, IDC_BUTTON1 ) );
	setDescriptionText ( GetDlgItem( dialogHwnd, IDC_BUTTON2 ) );

	setDescriptionText ( GetDlgItem( dialogHwnd, IDC_LABEL1 ) );
	setDescriptionText ( GetDlgItem( dialogHwnd, IDC_LABEL2 ) );
	setDescriptionText ( GetDlgItem( dialogHwnd, IDC_LABEL3 ) );

	setDescriptionText ( GetDlgItem( dialogHwnd, IDC_GROUPBOX ) );

	// Load combobox members.
	addComboItems( GetDlgItem( dialogHwnd, IDC_LANGCOMBO ), manager.getOptionNames( "Language" ), manager.getTheOneInUse( "Language" ) );
	addComboItems( GetDlgItem( dialogHwnd, IDC_OVERALLCOMBO ), manager.getOptionNames( "Overall Settings" ), manager.getTheOneInUse( "Overall Settings" ) );
	addComboItems( GetDlgItem( dialogHwnd, IDC_RESOLUTIONCOMBO ), manager.getOptionNames( "Resolution" ), manager.getTheOneInUse( "Resolution" ) );

	updateInfoText( );

}


void LaunchDialogHandler::loadOptions( )
{
	HWND dialogHwnd = dialogData->getHwnd();
	setComboBoxSelection( GetDlgItem( dialogHwnd, IDC_LANGCOMBO ), manager.getTheOneInUse( "Language" ) );
	setComboBoxSelection( GetDlgItem( dialogHwnd, IDC_OVERALLCOMBO ), manager.getTheOneInUse( "Overall Settings" ) );
	setComboBoxSelection( GetDlgItem( dialogHwnd, IDC_RESOLUTIONCOMBO ), manager.getTheOneInUse( "Resolution" ) );
	updateInfoText( );
}

void LaunchDialogHandler::applyOptions( )
{
	HWND dialogHwnd = dialogData->getHwnd();
	manager.applyOptions( "Resolution", getComboBoxSelection( GetDlgItem( dialogHwnd, IDC_RESOLUTIONCOMBO ) ) );
	manager.applyOptions( "Overall Settings", getComboBoxSelection( GetDlgItem( dialogHwnd, IDC_OVERALLCOMBO ) ) );
	manager.applyOptions( "Language", getComboBoxSelection( GetDlgItem( dialogHwnd, IDC_LANGCOMBO ) ) );
}


BOOL LaunchDialogHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch ( msg )
	{
	case WM_COMMAND:
		{
			int command = LOWORD(wParam);
			switch( command )
			{
			case IDC_BUTTON1:
				{
					applyOptions( );
					game::GameOptionManager::getInstance()->save();
					//manager.save();
					launchGame();
				}
				break;
			case IDC_LANGCOMBO:
				// Language selection has been changed.
				if(HIWORD(wParam) == CBN_SELCHANGE )
				{
					applyOptions( );
					game::GameOptionManager::getInstance()->save();

					// Restart the app.
					PostQuitMessage( 0 );
					char myFileName[ 1024 ];
					GetModuleFileName( NULL, myFileName, 1024);
					ShellExecute( 0, 0, myFileName, 0, 0, SW_NORMAL );
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

void LaunchDialogHandler::launchGame() 
{
	PostQuitMessage( 0 );

	int modIndex = modSelector.getActiveIndex();
	if(modIndex >= 0)
	{
		modSelector.restoreDir();
		int menuId = game::SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE);
		int speechId = game::SimpleOptions::getInt(DH_OPT_I_SPEECH_LANGUAGE);
		int subtitleId = game::SimpleOptions::getInt(DH_OPT_I_SUBTITLE_LANGUAGE);
		std::string modDir;
		std::string parameters;

		modDir = modSelector.getModDir(modIndex);
		parameters += "-mod=" + modDir;
		parameters += " -menu_language=" + boost::lexical_cast<std::string> (menuId);
		parameters += " -speech_language=" + boost::lexical_cast<std::string> (speechId);
		parameters += " -subtitle_language=" + boost::lexical_cast<std::string> (subtitleId);
		ShellExecute( 0, 0, "survivor.exe", parameters.c_str(), 0, SW_NORMAL );
	}
	else
	{
		ShellExecute( 0, 0, "survivor.exe", 0, 0, SW_NORMAL );
	}
}


}
}