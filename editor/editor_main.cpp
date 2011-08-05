// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "application.h"
#include "../util/mod_selector.h"
#include "../system/Logger.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include <string>
#include <windows.h>

bool forceLowDetail = false;

namespace frozenbyte {
namespace editor {

	util::ModSelector modSelector;

} // editor
} // frozenbyte

using namespace frozenbyte::filesystem;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR param, int)
{
	// editor has its own log file
#ifdef LEGACY_FILES
	Logger::createInstanceForLogfile("editor.log");
#else
	Logger::createInstanceForLogfile("logs/editor.log");
#endif
	Logger::getInstance()->setLogLevel(LOGGER_LEVEL_DEBUG);
	Logger::getInstance()->info("Editor starting up.");

	std::string parameter = param;
	if(parameter.find("-low") != parameter.npos)
		forceLowDetail = true;

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
	frozenbyte::editor::modSelector.changeDir();

	frozenbyte::editor::Application application;
	application.run();

	return 0;
}
