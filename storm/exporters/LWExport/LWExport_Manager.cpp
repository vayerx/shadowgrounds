// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "..\Shared\Export_Exporter.h"
#include "LWExport_Manager.h"

#include <lwserver.h>
#include <lwrender.h>
#include <lwgeneric.h>
#include <cassert>

namespace frozenbyte {
namespace exporter {

Manager::Manager(LWLayoutGeneric *layoutGeneric_)
:	global(0),
	layoutGeneric(layoutGeneric_)
{
	exporter = new Exporter();
}

Manager::~Manager()
{
	delete exporter;
}

GlobalFunc *Manager::getGlobal() const
{
	assert(global);
	return global;
}

LWItemInfo *Manager::getItemInfo() const
{
	assert(global);
	return static_cast<LWItemInfo *> (global(LWITEMINFO_GLOBAL, GFUSE_TRANSIENT));
}

LWSurfaceFuncs *Manager::getSurfaceFunctions() const
{
	assert(global);
	return static_cast<LWSurfaceFuncs *> (global(LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT));
}

LWBoneInfo *Manager::getBoneInfo() const
{
	assert(global);
	return static_cast<LWBoneInfo *> (global(LWBONEINFO_GLOBAL, GFUSE_TRANSIENT));
}

LWObjectInfo *Manager::getObjectInfo() const
{
	assert(global);
	return static_cast<LWObjectInfo *> (global(LWOBJECTINFO_GLOBAL, GFUSE_TRANSIENT));
}

LWTextureFuncs *Manager::getTextureFunctions() const
{
	assert(global);
	return static_cast<LWTextureFuncs *> (global(LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT));
}

LWImageList *Manager::getImageList() const
{
	assert(global);
	return static_cast<LWImageList *> (global(LWIMAGELIST_GLOBAL, GFUSE_TRANSIENT));
}

LWSceneInfo *Manager::getSceneInfo() const
{
	assert(global);
	return static_cast<LWSceneInfo *> (global(LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT));
}

LWInterfaceInfo *Manager::getInterfaceInfo() const
{
	assert(global);
	return static_cast<LWInterfaceInfo *> (global(LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT));
}

LWLayoutGeneric *Manager::getLayoutGeneric() const
{
	return layoutGeneric;
}

Exporter *Manager::getExporter()
{
	assert(exporter);
	return exporter;
}

void Manager::setGlobal(GlobalFunc *global_)
{
	global = global_;
}

} // end of namespace export
} // end of namespace frozenbyte
