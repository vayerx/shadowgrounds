
#ifndef OGUIBUTTONEVENT_H
#define OGUIBUTTONEVENT_H

class OguiButton;
class OguiWindow;


class OguiButtonEvent
{
public:

  enum EVENT_TYPE
  {
    EVENT_TYPE_CLICK = 1,  // cursor was released after press at button or wheel was rolled.
    EVENT_TYPE_PRESS = 2,  // cursor was pressed down at button or wheel was rolled
    EVENT_TYPE_OUT = 4,    // cursor press was cancelled by moving out
    EVENT_TYPE_OVER = 8,   // cursor moves to "over the button"
    EVENT_TYPE_LEAVE = 16, // cursor moves out of "over the button"
    EVENT_TYPE_HOLD = 32  // cursor is held down at button (called at every run!)
  };

  // NOTICE: OUT and LEAVE events may have invalid triggerButton data...
  // not sure really...

  EVENT_TYPE eventType;
  int cursorNumber;
  int cursorScreenX;
  int cursorScreenY;
  int cursorRelativeX;
  int cursorRelativeY;
  int cursorButtonMask;
  int cursorOldButtonMask;

  OguiButton *triggerButton;
  OguiWindow *triggerWindow;
  const void *extraArgument;


  OguiButtonEvent(EVENT_TYPE evt, int curnum, int scrx, int scry, int relx, int rely, 
    int but, int obut, OguiButton *trig, OguiWindow *trigwin, const void *arg);
};

#endif
