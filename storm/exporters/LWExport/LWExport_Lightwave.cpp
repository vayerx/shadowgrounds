// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Manager.h"
#include "LWExport_Scene.h"
#include "..\Shared\Export_Dialogs.h"

#include <lwserver.h>
#include <lwgeneric.h>
#include <windows.h>

namespace frozenbyte {
namespace exporter {

namespace {
	HINSTANCE moduleHandle = 0;
}

/*
  Exporter function. LW calls this to activate us
*/
extern "C" XCALL_(int) 
Export(long, GlobalFunc *global, LWLayoutGeneric *layoutGeneric, void *)
{
	assert(moduleHandle);

	std::auto_ptr<Manager> manager(new Manager(layoutGeneric));
	manager.get()->setGlobal(global);

	// Get all relevant data (pushes it to managers exporter)
	LWScene().collectData();

	// Export 
	createExportDialog(manager->getExporter(), moduleHandle);	

	// That's all folks!
	return AFUNC_OK;
}


/*
  Lightwave definitions. Identifies our plugin
*/

// Server tags
extern "C"
ServerTagInfo server_tags[] = 
{
#ifdef _DEBUG
	{
		"Frozenbyte DEBUG! Exporter",
		SRVTAG_USERNAME | LANGID_USENGLISH
	},
#elif FB_FAST_BUILD
	{
		"Frozenbyte Fast! Exporter",
		SRVTAG_USERNAME | LANGID_USENGLISH
	},
#else
	{
		"Frozenbyte Exporter",
		SRVTAG_USERNAME | LANGID_USENGLISH
	},
#endif
	{
		"file",
		SRVTAG_CMDGROUP | LANGID_USENGLISH
	},

	{
		"export/storm",
		SRVTAG_MENU | LANGID_USENGLISH
	},

	// Indicates end. Otherwise LW simply crashes, no good :)
	{
		(const char *) NULL
	}
};

// Plugin information  
extern "C"
ServerRecord ServerDesc[] = 
{
	// Our exporter
	{ 
		LWLAYOUTGENERIC_CLASS, 

#ifdef _DEBUG
		"Frozenbyte DEBUG! Exporter", 
#elif FB_FAST_BUILD
		"Frozenbyte Fast! Exporter",
#else
		"Frozenbyte Exporter",
#endif

		(ActivateFunc *) Export,
		server_tags
	},
	
	// Indicates that we don´t have any more plugins
	{ 
		(const char *) NULL 
	}
};


extern "C" 
BOOL WINAPI DllMain(HINSTANCE handle, DWORD, LPVOID)
{
	// Grab instance (for resource usage)
	moduleHandle = handle;
	return TRUE;
}

} // end of namespace export
} // end of namespace frozenbyte
