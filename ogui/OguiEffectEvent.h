
#ifndef OGUIEFFECTEVENT_H
#define OGUIEFFECTEVENT_H


#include <boost/utility.hpp>


class OguiWindow;


class OguiEffectEvent : public boost::noncopyable
{
public:

  enum EVENT_TYPE
  {
    EVENT_TYPE_FADEDOUT = 1,	// window has faded out after a fade-out request
    EVENT_TYPE_FADEDIN = 2,		// window has faded in after a fade-in request
    EVENT_TYPE_MOVEDOUT = 4,	// window has moved out after a move-out request
    EVENT_TYPE_MOVEDIN = 8,		// window has moved in after a move-in request
    EVENT_TYPE_TEXTTYPE = 16,	// buttons texts have been changed
	EVENT_TYPE_TEXTLINE = 32	// a new text has appeared on a button
  };

  EVENT_TYPE eventType;

  // not owned
  OguiWindow *triggerWindow;

  OguiEffectEvent(EVENT_TYPE evt, OguiWindow *trigwin);

  ~OguiEffectEvent();
};

#endif
