// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LWEXPORT_MANAGER_H
#define INCLUDED_LWEXPORT_MANAGER_H

#ifndef INCLUDED_SINGLETON_H
#include "..\Shared\Export_Singleton.h"
#endif

#ifndef INCLUDED_LWSERVER_H
#define INCLUDED_LWSERVER_H
#include <lwserver.h>
#endif
#ifndef INCLUDED_LWGENERIC_H
#define INCLUDED_LWGENERIC_H
#include <lwgeneric.h>
#endif
#ifndef INCLUDED_LWRENDER_H
#define INCLUDED_LWRENDER_H
#include <lwrender.h>
#endif
#ifndef INCLUDED_LWSURF_H
#define INCLUDED_LWSURF_H
#include <lwsurf.h>
#endif

#ifndef INCLUDED_WINDOWS_H
#define INCLUDED_WINDOWS_H
#include <windows.h>
#endif

namespace frozenbyte {
namespace exporter {

// Forward declarations
class Exporter;

/**  @class Manager
  *  @brief Manages global variables
  *  @author Juha Hiekkamäki
  *  @version 1.0
  *  @date 2001
  */
class Manager: public TSingleton<Manager>
{
	GlobalFunc *global;
	LWLayoutGeneric *layoutGeneric;

	Exporter *exporter;

public:
	explicit Manager(LWLayoutGeneric *layoutGeneric);
	~Manager();

	// LW function pointers
	GlobalFunc *getGlobal() const;

	LWItemInfo *getItemInfo() const;
	LWBoneInfo *getBoneInfo() const;
	LWObjectInfo *getObjectInfo() const;
	LWSurfaceFuncs *getSurfaceFunctions() const;	
	LWTextureFuncs *getTextureFunctions() const;
	LWImageList *getImageList() const;
	
	LWSceneInfo *getSceneInfo() const;
	LWInterfaceInfo *getInterfaceInfo() const;
	LWLayoutGeneric *getLayoutGeneric() const;

	Exporter *getExporter();

	void setGlobal(GlobalFunc *global);
	void setLayoutGeneric(LWLayoutGeneric *layout);
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
