#include "application.h"
#include <windows.h>
#include "launcher_window.h"

/*
#include "../../editor/window.h"
#include "../../editor/dialog.h"
#include "../../editor/command_list.h"
#include "../../editor/icommand.h"
*/
#include "../filesystem/file_package_manager.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include "../game/GameOptionManager.h"
#include "../game/GameConfigs.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_all.h"

#include "../util/mod_selector.h"
#include "Resource/resource.h"
#include <assert.h>

namespace frozenbyte {	
namespace launcher {

using namespace filesystem;
util::ModSelector modSelector;


///////////////////////////////////////////////////////////////////////////////
/*
namespace {
	class CommandOK : public ICommand
	{
	public:
		void execute( )
		{
			assert( false );
		}
	};

	class CommandCancel : public ICommand
	{
	public:
		void execute( )
		{
			exit( 1 );
		}
	};
}*/

///////////////////////////////////////////////////////////////////////////////

class ApplicationImpl
{
public:
	ApplicationImpl() :
	  quit( false ),
	  window( )
	{
	}

	~ApplicationImpl()
	{
	}

	///////////////////////////////////////////////////////////////////////////

	void run()
	{
		MSG msg;
		while ( GetMessage( &msg, NULL, 0, 0 ) && quit == false ) 
		{
			// IsDialogMessage( dialog.getWindowHandle(), &windowsMessage );
			
			TranslateMessage (&msg); 
			DispatchMessage (&msg); 
		}
		
	}

	///////////////////////////////////////////////////////////////////////////

	bool			quit;
	LauncherWindow	window;

};

///////////////////////////////////////////////////////////////////////////////

Application::Application() 
{
	FilePackageManager &manager = FilePackageManager::getInstance();
	boost::shared_ptr<IFilePackage> standardPackage( new StandardPackage() );
	boost::shared_ptr<IFilePackage> zipPackage1( new ZipPackage( "data1.fbz" ) );
	boost::shared_ptr<IFilePackage> zipPackage2( new ZipPackage( "data2.fbz" ) );
	boost::shared_ptr<IFilePackage> zipPackage3( new ZipPackage( "data3.fbz" ) );
	boost::shared_ptr<IFilePackage> zipPackage4( new ZipPackage( "data4.fbz" ) );

	manager.addPackage( standardPackage, 999 );
	manager.addPackage( zipPackage1, 1 );
	manager.addPackage( zipPackage2, 2 );
	manager.addPackage( zipPackage3, 3 );
	manager.addPackage( zipPackage4, 4 );

	game::GameOptionManager::getInstance()->load();
	int menuId = game::SimpleOptions::getInt(DH_OPT_I_MENU_LANGUAGE);
	int speechId = game::SimpleOptions::getInt(DH_OPT_I_SPEECH_LANGUAGE);
	int subtitleId = game::SimpleOptions::getInt(DH_OPT_I_SUBTITLE_LANGUAGE);

	game::GameOptionManager::cleanInstance();
	game::GameConfigs::cleanInstance();
	modSelector.changeDir();

	game::GameOptionManager::getInstance()->load();
	game::SimpleOptions::setInt(DH_OPT_I_MENU_LANGUAGE, menuId);
	game::SimpleOptions::setInt(DH_OPT_I_SPEECH_LANGUAGE, speechId);
	game::SimpleOptions::setInt(DH_OPT_I_SUBTITLE_LANGUAGE, subtitleId);

	impl = new ApplicationImpl;
}

//=============================================================================

Application::~Application()
{
	delete impl;
	impl = 0;
}

//=============================================================================

void Application::run()
{
	impl->run();
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace launcher

} // end of namespace frozenbyte