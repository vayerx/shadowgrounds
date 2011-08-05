#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#ifndef __GNUC__
#pragma message("Creating precompiled header (stormtest).")
#endif

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "../util/Debug_MemoryManager.h"

//#include <Storm3D_UI.h>
#include <DatatypeDef.h>

#include "../system/Logger.h"
#include "../convert/str2int.h"
#include "../system/Timer.h"
#include "../util/SimpleParser.h"
#include "../container/LinkedList.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../game/SimpleOptions.h"
#include "../util/ScriptManager.h"
#include "../util/Script.h"
#include "../util/ScriptProcess.h"

#endif

