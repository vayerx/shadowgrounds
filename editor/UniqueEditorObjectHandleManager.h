
#ifndef UNIQUEEDITOROBJECTHANDLEMANAGER_H
#define UNIQUEEDITOROBJECTHANDLEMANAGER_H

#include "UniqueEditorObjectHandle.h"

class UniqueEditorObjectHandleManager
{
public:

	static void init();

	// (kinda error code like stuff to be queried after init)
	static bool wasFirstTimeInit();
	static bool wasLocked();
	static bool wasError();

	static void uninit();

	// the internal handle can be supplied to be used as a part of the new unique handle
	// (if the internal handles are already "quite unique", this will make the generation of the unique 
	// handle more robust)
	static UniqueEditorObjectHandle createNewUniqueHandle(unsigned int internalHandleValue);

	// supplying this conversion probably is a bad idea as that would encourage using this conversion...
	// (which generally would be a bad thing when internalHandles are not really unique,
	// thus allowing mapping of this unique handle to non-unique value that may no longer be mapped to 
	// the valid object)
	//static unsigned int parseInternalHandleValue(UniqueEditorObjectHandle uniqueHandle);

};

#endif
