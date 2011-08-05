
#include "precompiled.h"

#include "OguiButtonEvent.h"
#include "../util/Debug_MemoryManager.h"


OguiButtonEvent::OguiButtonEvent(EVENT_TYPE evt, int curnum, int scrx, int scry, int relx, int rely, 
  int but, int obut, OguiButton *trig, OguiWindow *trigwin, const void *arg)
{
  eventType = evt;
  cursorNumber = curnum;
  cursorScreenX = scrx;
  cursorScreenY = scry;
  cursorRelativeX = relx;
  cursorRelativeY = rely;
  cursorButtonMask = but;
  cursorOldButtonMask = obut;
  triggerButton = trig;
  triggerWindow = trigwin;
  extraArgument = arg;
}
