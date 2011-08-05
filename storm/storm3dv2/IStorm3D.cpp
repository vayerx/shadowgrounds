// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "../../util/Debug_MemoryManager.h"

 

//------------------------------------------------------------------
// IStorm3D::Create_Storm3D_Interface
//------------------------------------------------------------------
IStorm3D *IStorm3D::Create_Storm3D_Interface(bool no_info, frozenbyte::filesystem::FilePackageManager *fileManager, IStorm3D_Logger *logger_)
{
	return new Storm3D(no_info, fileManager, logger_);
}
