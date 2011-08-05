#ifndef INC_IOGUISLIDERLISTENER_H
#define INC_IOGUISLIDERLISTENER_H

#include "OguiSliderEvent.h"

class IOguiSliderListener
{
public:
	IOguiSliderListener() { }
	virtual ~IOguiSliderListener() { }

	virtual void sliderEvent( OguiSliderEvent* eve ) = 0;
};


#endif
