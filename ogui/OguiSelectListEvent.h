
#ifndef OGUISELECTLISTEVENT_H
#define OGUISELECTLISTEVENT_H

class OguiSelectList;
class OguiWindow;


class OguiSelectListEvent
{
public:

  enum EVENT_TYPE
  {
    EVENT_TYPE_SELECT = 1,     // user selects an item previously unselected
    EVENT_TYPE_DOUBLESELECT = 2, // user selects an item already selected
    EVENT_TYPE_UNSELECT = 4,   // user unselects an item (if multiselection)
    EVENT_TYPE_SCROLL = 8,     // user scrolled the list up or down
    EVENT_TYPE_CURSOR_OVER = 16,  // cursor moved over an item
    EVENT_TYPE_CURSOR_LEAVE = 32 // cursor moved off an item
  };

  EVENT_TYPE eventType;
  int cursorNumber;
  int cursorScreenX;
  int cursorScreenY;
  //int cursorRelativeX;
  //int cursorRelativeY;
  int cursorButtonMask;
  int cursorOldButtonMask;

  // selection number values start from 0  
  // only valid if event type != EVENT_TYPE_SCROLL, 
  // undefined in case of scroll event (actually -1, but don't rely on it)
  int selectionNumber;
 
  // NULL values may be used for selection values, if all you need is the 
  // selection number. 
  // both selection value and desc are null in case of scroll event
  char *selectionValue;
  char *selectionDesc;

  int scrollY;

  OguiSelectList *triggerSelectList;
  OguiWindow *triggerWindow;
  void *extraArgument;

  OguiSelectListEvent(EVENT_TYPE evt, int curnum, int scrx, int scry, 
    int but, int obut, int selectionnum, char *selval, char *seldesc,
    int scrolly,
    OguiSelectList *trig, OguiWindow *trigwin, void *arg);
};

#endif
