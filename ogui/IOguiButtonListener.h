 
#ifndef IOGUIBUTTONLISTENER_H
#define IOGUIBUTTONLISTENER_H

#include "OguiButtonEvent.h"

class IOguiButtonListener
{
public:
	virtual ~IOguiButtonListener() {}
  // all in one
  virtual void CursorEvent(OguiButtonEvent *eve) = 0;
};

#endif
