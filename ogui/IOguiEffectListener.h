
#ifndef IOGUIEFFECTLISTENER_H
#define IOGUIEFFECTLISTENER_H

#include "OguiEffectEvent.h"

class IOguiEffectListener
{
public:
	virtual ~IOguiEffectListener() {}
  // all in one
  virtual void EffectEvent(OguiEffectEvent *eve) = 0;
};

#endif
