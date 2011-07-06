
#include "precompiled.h"

#include "OguiEffectEvent.h"
#include "..\util\Debug_MemoryManager.h"


OguiEffectEvent::OguiEffectEvent(EVENT_TYPE evt, OguiWindow *trigwin)
{
  eventType = evt;
  triggerWindow = trigwin;
}
