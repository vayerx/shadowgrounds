
#include "dlghandlerimpl.h"
#include "LauncherMainDialog.h"
#include "LauncherWindow.h"
#include "LaunchDialogHandler.h"
#include "GraphicsDialogHandler.h"
#include "SoundsDialogHandler.h"
#include "PhysicsDialogHandler.h"
#include "ModDialogHandler.h"
#include "UpdateDialogHandler.h"
#include "../Util/UnicodeConverter.h"

#include "window.h"
#include "dialog_data.h"
#include "idlghandler.h"
#include "options_value_manager.h"
#include "../game/GameOptionManager.h"
#include "../game/GameOption.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_all.h"
#include "../game/DHLocaleManager.h"
#include "../util/hiddencommand.h"
#include "../util/mod_selector.h"
#include "../editor/parser.h"
#include "../filesystem/file_list.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include <boost/lexical_cast.hpp>
#include <assert.h>
#include <string>
#include <vector>
#include "resource.h"

#include <commctrl.h>
#include <sys/types.h> 
#include <sys/stat.h>

using namespace game;


namespace frozenbyte {
namespace launcher {
namespace {

	bool fileExists(const char *name)
	{
		struct _stat buffer;
		int result = _stat(name, &buffer);
		if(result != 0)
			return false;

		return true;
	}

} // unnamed

///////////////////////////////////////////////////////////////////////////////

extern util::ModSelector modSelector;

class LauncherMainDialogHandler : public DlgHandlerImpl
{
public:
	OptionsValueManager manager;
	LauncherWindow*		lwindow;
	DlgHandlerImpl ** handlerList;
	int activeTab;

	LauncherMainDialogHandler() :
		activeTab(0)
	{

	}

	void initDialog( )
	{

	}

	void applyOptions( )
	{
	}

	void loadOptions( )
	{
	}


	BOOL handleMessages( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{

		switch( msg )
		{
		case WM_NOTIFY:
			{
				// Tab selection just changed?
				NMHDR hdr = * ( ( NMHDR * ) lParam );
				if( hdr.idFrom == IDC_MAINTABS && hdr.code == TCN_SELCHANGE )
				{
					int tabid = TabCtrl_GetCurSel( GetDlgItem( hwnd, IDC_MAINTABS ) );

					// Apply configuration from old dialog and re-load new ones in case the old selections affected new ones.
					handlerList[activeTab]->applyOptions( );
					handlerList[tabid]->loadOptions( );
					activeTab = tabid;

					// Hide all tab dialogs, then show the selected one.
					for(int l = 0; l < TAB_AMOUNT; l++)
						if(handlerList[l]->dialogData)
							handlerList[l]->dialogData->hide();				
					handlerList[tabid]->dialogData->show();
				}
			}
			break;

		case WM_SHOWWINDOW:
			initDialog( );	
			break;

		};


		return 0;
	}

};

///////////////////////////////////////////////////////////////////////////////

#define ADD_TAB(i, txt)\
	{\
		std::wstring wstr;\
		util::convertToWide( ::game::getLocaleGuiString( txt ), wstr );\
		t.pszText = (LPSTR) wstr.c_str();\
		SendMessage ( GetDlgItem( dialogData->getHwnd(), IDC_MAINTABS ), TCM_INSERTITEMW, i, (LPARAM)&t);\
	}


void LauncherMainDialog::initTabs()
{
	if(!tabsInitialized)
	{
		assert( dialogData );
		tabsInitialized = true;

		TCITEM t;
		t.mask = TCIF_TEXT;
		
		ADD_TAB(0, "launcher_launch_tab");
		ADD_TAB(1, "launcher_graphics");
		ADD_TAB(2, "launcher_sounds");
		ADD_TAB(3, "launcher_physics");
		ADD_TAB(4, "launcher_mods_level_editor");
		ADD_TAB(5, "launcher_updates_news");

	}
}

///////////////////////////////////////////////////////////////////////////////


LauncherMainDialog::LauncherMainDialog( const Window& parent_window, LauncherWindow* lwindow ) :
	tabsInitialized(false)
{
	handler = new LauncherMainDialogHandler;
	handler->lwindow = lwindow;

	optionsValueManager->load();

	// Initialize main dialog + tab control.
	dialogData	= new DialogData( IDD_MAINWINDOW, parent_window.getWindowHandle(), DialogData::ATTACH_ALL, handler );
	handler->dialogData = dialogData;
	tabs			= new DialogData( IDC_MAINTABS, GetDlgItem( parent_window.getWindowHandle(), IDD_MAINWINDOW),  DialogData::ATTACH_ALL, handler );
	int tabs_x = 0, tabs_y = 0;
	int tabs_width = 0, tabs_height = 0;
	initTabs();

	tabs->getPosition( tabs_x, tabs_y );
	tabs->getSize( tabs_width, tabs_height );

	// Sub dialogs' init.
	HWND tabHwnd = GetDlgItem( dialogData->getHwnd(), IDC_MAINTABS );
	handlerList[0] = new LaunchDialogHandler(tabHwnd);
	handlerList[1] = new GraphicsDialogHandler(tabHwnd);
	handlerList[2] = new SoundsDialogHandler(tabHwnd);
	handlerList[3] = new PhysicsDialogHandler(tabHwnd);
	handlerList[4] = new ModDialogHandler(tabHwnd);
	handlerList[5] = new UpdateDialogHandler(tabHwnd);

	handler->handlerList = handlerList;

	for(int l = 0; l < TAB_AMOUNT; l++)
	{
		handlerList[l]->initDialog ( );
		handlerList[l]->loadOptions( );

		// Adjust dialog's position.
		int d_w = 0, d_h = 0;
		handlerList[l]->dialogData->setPosition( 0, 0 );
		handlerList[l]->dialogData->getSize( d_w, d_h );
		RECT rc = { 0, 0, d_w, d_h };
		SendMessage( tabHwnd, TCM_ADJUSTRECT, (WPARAM) FALSE, (LPARAM) &rc );
		handlerList[l]->dialogData->setPosition( rc.left, rc.top );

	}


	handlerList[0]->dialogData->show();

}

//=============================================================================

LauncherMainDialog::~LauncherMainDialog()
{
	for(int l = 0; l < TAB_AMOUNT; l++)
	{
		if(handlerList[l])
			delete handlerList[l];
	}

	delete dialogData;
	delete handler;
}

///////////////////////////////////////////////////////////////////////////////

void LauncherMainDialog::getSize(int &x, int &y) const
{
	dialogData->getSize(x, y);
}

///////////////////////////////////////////////////////////////////////////////

void LauncherMainDialog::show()
{
	dialogData->show();
}

//=============================================================================

void LauncherMainDialog::hide()
{
	dialogData->hide();
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace launcher
} // end of namespace frozenbyte