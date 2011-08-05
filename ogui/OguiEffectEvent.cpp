
#include "precompiled.h"

#include "OguiEffectEvent.h"
#include "../util/Debug_MemoryManager.h"


OguiEffectEvent::OguiEffectEvent(EVENT_TYPE evt, OguiWindow *trigwin)
{
	assert(trigwin != NULL);

  eventType = evt;
  triggerWindow = trigwin;
}


OguiEffectEvent::~OguiEffectEvent()
{
	// not owned
	// don't delete, just poison
	triggerWindow = NULL;
}
