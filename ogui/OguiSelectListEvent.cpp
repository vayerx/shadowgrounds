
#include "precompiled.h"

#include "OguiSelectListEvent.h"
#include "../util/Debug_MemoryManager.h"

OguiSelectListEvent::OguiSelectListEvent(EVENT_TYPE evt, int curnum, 
  int scrx, int scry, 
  int but, int obut, int selectionnum, char *selval, char *seldesc,
  int scrolly,
  OguiSelectList *trig, OguiWindow *trigwin, void *arg)
{
  eventType = evt;
  cursorNumber = curnum;
  cursorScreenX = scrx;
  cursorScreenY = scry;
  cursorButtonMask = but;
  cursorOldButtonMask = obut;
  
  selectionNumber = selectionnum;
  selectionValue = selval;
  selectionDesc = seldesc;

  scrollY = scrolly;

  triggerSelectList = trig;
  triggerWindow = trigwin;

  extraArgument = arg;
}
