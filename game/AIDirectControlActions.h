
#ifndef AIDIRECTCONTROLACTIONS_H
#define AIDIRECTCONTROLACTIONS_H

#include "direct_controls.h"
#include <DatatypeDef.h>

class AIDirectControlActions
{
public:
	AIDirectControlActions()
	{
		for (int i = 0; i < DIRECT_CTRL_AMOUNT; i++)
		{
			directControlOn[i] = false;
		}
		aimPosition = VC3(0,0,0);
	}


	AIDirectControlActions& operator= (const AIDirectControlActions &copyFrom)
	{
		for (int i = 0; i < DIRECT_CTRL_AMOUNT; i++)
		{
			this->directControlOn[i] = copyFrom.directControlOn[i];
		}		
		this->aimPosition = copyFrom.aimPosition;
		return *this;
	}


	bool directControlOn[DIRECT_CTRL_AMOUNT];
	VC3 aimPosition;

};

#endif


