
#include <windows.h>
#include "resource.h"
#include "ModDialogHandler.h"
#include "LaunchDialogHandler.h"
#include "../game/DHLocaleManager.h"
#include "../util/mod_selector.h"
#include "../game/GameOptionManager.h"


namespace frozenbyte
{
namespace launcher
{
extern util::ModSelector modSelector;


ModDialogHandler::ModDialogHandler( HWND parent )
{
	dialogData = new DialogData( IDD_MODDIALOG, parent, DialogData::ATTACH_NONE, this );
}

ModDialogHandler::~ModDialogHandler( )
{
	delete dialogData;
}

void ModDialogHandler::setActivateButton( bool activate ) 
{
	// active == false => text = "Deactivate" 
	// otherwise text = "Activate"

	HWND hwnd = dialogData->getHwnd();

	if(activate)
	{
		SendMessage( GetDlgItem( hwnd, IDC_MBUTTON1 ), WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)::game::getLocaleGuiString( "launcher_activate" ) );
	}
	else
	{
		SendMessage( GetDlgItem( hwnd, IDC_MBUTTON1 ), WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)::game::getLocaleGuiString( "launcher_deactivate" ) );
	}

}

void ModDialogHandler::loadMods( )
{

	HWND hwnd = dialogData->getHwnd();

	addComboItem( GetDlgItem( hwnd, IDC_MCOMBO1 ), ::game::getLocaleGuiString( "launcher_no_mod" ), true );

	int modAmount = modSelector.getModAmount();
	for(int l = 0; l < modAmount; l++)
	{
		if( modSelector.getActiveIndex() == l)
		{
			setActivateButton( false );
			addComboItem( GetDlgItem( hwnd, IDC_MCOMBO1 ), modSelector.getDescription( l ), true );
		}
		else
			addComboItem( GetDlgItem( hwnd, IDC_MCOMBO1 ), modSelector.getDescription( l ), false);
	}

}

void ModDialogHandler::initDialog( )
{

	HWND hwnd = dialogData->getHwnd();

	// Load localizations texts.
	setDescriptionText ( GetDlgItem( hwnd, IDC_MSTATIC1));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MSTATIC2));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MSTATIC3));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MSTATIC4));

	setDescriptionText ( GetDlgItem( hwnd, IDC_MBUTTON1));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MBUTTON2));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MBUTTON3));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MBUTTON4));
	setDescriptionText ( GetDlgItem( hwnd, IDC_MBUTTON5));

	// Load combobox items.

	loadMods();
}

void ModDialogHandler::loadOptions( )
{

}

void ModDialogHandler::applyOptions( )
{

}

BOOL ModDialogHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	// Selection of mod selection box has changed.
	if( msg == WM_COMMAND && LOWORD(wParam) == IDC_MCOMBO1 && HIWORD(wParam) == CBN_SELCHANGE )
	{
		int index = SendMessage( GetDlgItem( hwnd, IDC_MCOMBO1 ), CB_GETCURSEL, 0, 0) - 1;
		if( index == modSelector.getActiveIndex( ) )
		{
			setActivateButton( false );
		}
		else
			setActivateButton( true );
	}

	// Activate/Deactivate mod -button has been clicked.
	if( msg == WM_COMMAND && LOWORD(wParam) == IDC_MBUTTON1 )
	{
		int index = SendMessage( GetDlgItem( hwnd, IDC_MCOMBO1 ), CB_GETCURSEL, 0, 0) - 1;

		if( index == modSelector.getActiveIndex( ) )
		{
			// Deactivate.
			index = -1;
		}

		modSelector.saveActiveModFile(index);
		modSelector.restoreDir();

		// While we are at it, apply&save changes.
		applyOptions();
		game::GameOptionManager::getInstance()->save();

		// Restart the app.
		PostQuitMessage( 0 );
		char myFileName[ 1024 ];
		GetModuleFileName( NULL, myFileName, 1024);
		ShellExecute( 0, 0, myFileName, 0, 0, SW_NORMAL );

	}

	// Launch game button.
	if( msg == WM_COMMAND && LOWORD(wParam) == IDC_MBUTTON2 )
	{
		applyOptions();
		game::GameOptionManager::getInstance()->save();

		LaunchDialogHandler::launchGame();
	}

	// Launch editor
	if( msg == WM_COMMAND && LOWORD(wParam) == IDC_MBUTTON5 )
	{
		applyOptions();
		game::GameOptionManager::getInstance()->save();
		PostQuitMessage( 0 );
		ShellExecute( 0, 0, "survivor_editor.exe", 0, 0, SW_NORMAL );
	}

	return FALSE;
}

}
}