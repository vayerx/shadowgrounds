#include "launch_dialog.h"
#include "launcher_window.h"
#include "window.h"
#include "dialog_data.h"
#include "idlghandler.h"
#include "dlghandlerimpl.h"
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
#include "Resource/resource.h"

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

class LaunchDialogHandler : public DlgHandlerImpl
{
public:
	OptionsValueManager manager;
	LauncherWindow*		lwindow;

	void initDialog( HWND hwnd )
	{
		addComboItems( GetDlgItem( hwnd, IDC_COMBOLANGUAGE ), manager.getOptionNames( "Language" ), manager.getTheOneInUse( "Language" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBORESOLUTION ), manager.getOptionNames( "Resolution" ), manager.getTheOneInUse( "Resolution" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOGFXDETAIL ), manager.getOptionNames( "GraphicsDetail" ), manager.getTheOneInUse( "GraphicsDetail" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOSPEAKERTYPE ), manager.getOptionNames( "SpeakerType" ), manager.getTheOneInUse( "SpeakerType" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOMIXINGRATE ), manager.getOptionNames( "MixingRate" ), manager.getTheOneInUse( "MixingRate" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOPHYSICSDETAIL ),	manager.getOptionNames( "Physics Quality" ),	manager.getTheOneInUse( "Physics Quality" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOCONTROLTYPE ),	manager.getOptionNames( "ClawControlType" ),	manager.getTheOneInUse( "ClawControlType" ) );
	   // IDC_COMBOCONTROLTYPE	
		setCheckBox( hwnd, IDC_CHECKSOUND, manager.getTheOneInUse( "Sounds" ) );
		setCheckBox( hwnd, IDC_CHECKHARDWARE3D, manager.getTheOneInUse( "Hardware3D" ) );
		setCheckBox( hwnd, IDC_CHECKENABLEMENUVIDEO, manager.getTheOneInUse( "EnableMenuVideo" ) );
		setCheckBox( hwnd, IDC_CHECKENABLEMENUVSYNC, manager.getTheOneInUse( "VSync" ) );
		setCheckBox( hwnd, IDC_CHECKEAX, manager.getTheOneInUse( "Eax" ) );

		//SendMessage( hwnd, CB_ADDSTRING, 0, ( LPARAM ) game::getLocaleGuiString( items[ i ].c_str() ) );
		//SendMessage( hwnd, CB_SELECTSTRING, -1, ( LPARAM )game::getLocaleGuiString( select.c_str() ) );

		// Mod stuff
		setDescriptionText( GetDlgItem( hwnd, IDC_MOD_HELP ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_LAUNCHER_MOD_SELECTION ) );
		{
			SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_RESETCONTENT, 0, 0);

#ifdef FB_RU_HAX
			std::wstring nomod = ::game::DHLocaleManager::getInstance()->getWideString(::game::DHLocaleManager::BANK_GUI, "launcher_no_mod");
			SendMessageW( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_ADDSTRING, (WPARAM)0, (LPARAM) nomod.c_str() );

#else
			std::string nomod = game::getLocaleGuiString("launcher_no_mod");
			SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_ADDSTRING, 0, ( LPARAM ) nomod.c_str());
#endif

			for(int i = 0; i < modSelector.getModAmount(); ++i)
			{
				const std::string &desc = modSelector.getDescription(i);
				SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_ADDSTRING, 0, ( LPARAM ) desc.c_str());
			}

			int selection = modSelector.getActiveIndex();
			SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_SETCURSEL, selection + 1, 0);
			/*
			if(selection >= 0)
				SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_SETCURSEL, selection + 1, 0);
			else
				SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_SETCURSEL, 0, 0);
			*/

			/*
			const std::string root = "Mods";

			filesystem::StandardPackage files;
			filesystem::FileList fileList;
			files.findFiles(root, "*.zip", fileList);

			modList.clear();
			modStringList.clear();

			for(int i = 0; i < fileList.getFileAmount(root); ++i)
			{
				const std::string &zipFullName = fileList.getFileName(root, i);
				std::string zipName;
				int index = zipFullName.find_first_of("/\\");
				if(index != zipFullName.npos)
					zipName = zipFullName.substr(index + 1, zipFullName.size() - index - 1);

				filesystem::ZipPackage zip(zipFullName);
				filesystem::InputStream stream = zip.getFile("description.txt");
				if(stream.isEof())
					continue;

				editor::Parser parser;
				stream >> parser;

				const editor::ParserGroup &group = parser.getGlobals();
				if(group.getLineCount() < 1)
					continue;

				std::string name = group.getLine(0);
				if(name.size() < 4)
					continue;

				modList[name] = zipName;
				modStringList.push_back(name);
			}

			std::sort(modStringList.begin(), modStringList.end());

			// FIXME -- get key from locales
			SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_ADDSTRING, 0, ( LPARAM ) "Nothing. Zip. Nada");
			for(unsigned int i = 0; i < modStringList.size(); ++i)
				SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_ADDSTRING, 0, ( LPARAM ) modStringList[i].c_str());
			
			//if no mod selected
				SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_SETCURSEL, 0, 0);
			*/
		}

		// Editor things
		setDescriptionText( GetDlgItem( hwnd, IDC_EDITOR_HELP ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_LAUNCHER_EDITOR ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_EDITOR_MANUAL ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_EDITOR_LAUNCH ) );


		// buttons
		setDescriptionText( GetDlgItem( hwnd, IDOK ) );
		setDescriptionText( GetDlgItem( hwnd, IDCANCEL ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_BUTTONADVANCED ) );
		setDescriptionText( GetDlgItem( hwnd, IDCHECKUPDATES ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_ACTIVATEMOD ) );

		// language
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTION1 ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTION6 ) );
		
		// graphics detail
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICGRAPHICSLEVEL ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTION2 ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTION3 ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKENABLEMENUVIDEO ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKENABLEMENUVSYNC ) );

		// physics
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICPHYSICSLEVEL ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_AGEIAPROPAGANDA ) );


		// sounds
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICSOUNDS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKSOUND ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKHARDWARE3D ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTION4 ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTION5 ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKEAX ) );

		
	}

	void applyOptions( HWND hwnd )
	{
		manager.applyOptions( "Language",		getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOLANGUAGE ) ) );
		manager.applyOptions( "Resolution",		getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBORESOLUTION ) ) );
		manager.applyOptions( "GraphicsDetail", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOGFXDETAIL ) ) );
		manager.applyOptions( "Physics Quality", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOPHYSICSDETAIL ) ) );
		manager.applyOptions( "SpeakerType",	getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOSPEAKERTYPE ) ) );
		manager.applyOptions( "MixingRate",		getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOMIXINGRATE ) ) );
		manager.applyOptions( "ClawControlType", getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOCONTROLTYPE ) ) );


		manager.applyOptions( "Sounds",				getCheckBoxValue( hwnd, IDC_CHECKSOUND ) );
		manager.applyOptions( "Hardware3D",			getCheckBoxValue( hwnd, IDC_CHECKHARDWARE3D ) );
		manager.applyOptions( "EnableMenuVideo",	getCheckBoxValue( hwnd, IDC_CHECKENABLEMENUVIDEO ) );
		manager.applyOptions( "VSync",				getCheckBoxValue( hwnd, IDC_CHECKENABLEMENUVSYNC ) );
		manager.applyOptions( "Eax",				getCheckBoxValue( hwnd, IDC_CHECKEAX ) );
	}


	BOOL handleMessages( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		int index = 0;
		std::string mod;

		int modIndex = modSelector.getActiveIndex();
		int menuId = game::SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE);
		int speechId = game::SimpleOptions::getInt(DH_OPT_I_SPEECH_LANGUAGE);
		int subtitleId = game::SimpleOptions::getInt(DH_OPT_I_SUBTITLE_LANGUAGE);
		std::string modDir;
		std::string parameters;

		switch( msg )
		{
		case WM_INITDIALOG:
			SetFocus( GetDlgItem( hwnd, IDOK ) );
			break; 

		case WM_COMMAND:
		{	
			int command = LOWORD(wParam);
						
			switch( command )
			{
			case IDOK:
				applyOptions( hwnd );
				GameOptionManager::getInstance()->save();
				PostQuitMessage( 0 );

				modSelector.restoreDir();

				//int modIndex = modSelector.getActiveIndex();
				if(modIndex >= 0)
				{
					//int menuId = game::SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE);
					//int speechId = game::SimpleOptions::getInt(DH_OPT_I_SPEECH_LANGUAGE);
					//int subtitleId = game::SimpleOptions::getInt(DH_OPT_I_SUBTITLE_LANGUAGE);
					modDir = modSelector.getModDir(modIndex);

					parameters += "-mod=" + modDir;
					parameters += " -menu_language=" + boost::lexical_cast<std::string> (menuId);
					parameters += " -speech_language=" + boost::lexical_cast<std::string> (speechId);
					parameters += " -subtitle_language=" + boost::lexical_cast<std::string> (subtitleId);

					//MessageBox(0, command.c_str(), "Shit happens", MB_OK);
					ShellExecute( 0, 0, "claw_proto.exe", parameters.c_str(), 0, SW_NORMAL );
				}
				else
				{
					ShellExecute( 0, 0, "claw_proto.exe", 0, 0, SW_NORMAL );
				}

				// manager.save();
				break;

			case IDCHECKUPDATES:
				//applyOptions( hwnd );
				//GameOptionManager::getInstance()->save();
				//ShellExecute( 0, 0, "Updater.exe", 0, 0, SW_NORMAL );
				//system("start Updater.exe");
				modSelector.restoreDir();
				hiddencommand("Updater\\start_updater.bat", false);
				PostQuitMessage( 0 );
				// manager.save();
				break;

			case IDC_ACTIVATEMOD:
				index = SendMessage( GetDlgItem( hwnd, IDC_COMBOMODSELECTION ), CB_GETCURSEL, 0, 0);
				modSelector.saveActiveModFile(index - 1);

				modSelector.restoreDir();

				if(fileExists("Shadowgrounds Launcher.exe"))
					ShellExecute( 0, 0, "Shadowgrounds Launcher.exe", 0, 0, SW_NORMAL );
				else
					ShellExecute( 0, 0, "ShadowgroundsLauncher.exe", 0, 0, SW_NORMAL );

				PostQuitMessage( 0 );
				break;

			case IDC_EDITOR_MANUAL:
				modSelector.restoreDir();

				if(fileExists("Editor Manual.rtf"))
					ShellExecute( 0, 0, "Editor Manual.rtf", 0, 0, SW_NORMAL );
				else
					ShellExecute( 0, 0, "EditorManual.rtf", 0, 0, SW_NORMAL );


				modSelector.changeDir();
				break;

			case IDC_EDITOR_LAUNCH:
				modSelector.restoreDir();
#ifdef STEAM_HAX
				ShellExecute( 0, 0, "steam://run/2505", 0, 0, SW_NORMAL );
#else
				if(fileExists("Shadowgrounds Editor.exe"))
					ShellExecute( 0, 0, "Shadowgrounds Editor.exe", 0, 0, SW_NORMAL );
				else
					ShellExecute( 0, 0, "ShadowgroundsEditor.exe", 0, 0, SW_NORMAL );
#endif
				PostQuitMessage( 0 );
				break;

			case IDCANCEL:
				PostQuitMessage( 0 );
				break;

			case IDC_BUTTONADVANCED:
				applyOptions( hwnd );
				lwindow->openAdvanced();

				break;
			}

			break;
		}

		case WM_SHOWWINDOW:
			manager.load();
			initDialog( hwnd );	
			break;

		};


		return 0;
	}

};

///////////////////////////////////////////////////////////////////////////////

LaunchDialog::LaunchDialog( const Window& parent_window, LauncherWindow* lwindow )
{
	handler = new LaunchDialogHandler;

	handler->manager.load();
	handler->lwindow = lwindow;
	//GameOptionManager::getInstance()->load();

	dialog = new DialogData( IDD_LAUNCHER, parent_window, DialogData::ATTACH_ALL, handler );
	

	// dialog->setDialogHandler( new LaunchDialogHandler );
}

//=============================================================================

LaunchDialog::~LaunchDialog()
{
	
	delete dialog;
	delete handler;
}

///////////////////////////////////////////////////////////////////////////////

void LaunchDialog::getSize(int &x, int &y) const
{
	dialog->getSize(x, y);
}

///////////////////////////////////////////////////////////////////////////////

void LaunchDialog::show()
{
	dialog->show();
}

//=============================================================================

void LaunchDialog::hide()
{
	dialog->hide();
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace launcher
} // end of namespace frozenbyte