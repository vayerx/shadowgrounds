
#ifndef IOGUISELECTLISTLISTENER_H
#define IOGUISELECTLISTLISTENER_H

#include "OguiSelectListEvent.h"

class IOguiSelectListListener
{
public:
	virtual ~IOguiSelectListListener() {}
  virtual void SelectEvent(OguiSelectListEvent *eve) = 0;
};

#endif
