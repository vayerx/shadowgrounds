
#include "precompiled.h"

#include "GameProfilesEnumeration.h"
#include "../util/fb_assert.h"

namespace game
{

	std::string GameProfilesEnumeration::getNextProfile()
	{
		if (currentPos < (int)profileList.size())
		{
			currentPos++;
			return profileList[currentPos - 1];
		} else {
			fb_assert(!"GameProfilesEnumeration::getNextProfile - Attempt to getNextProfile when no more available.");
			return "";
		}
	}


	bool GameProfilesEnumeration::isNextProfileAvailable()
	{
		if (currentPos < (int)profileList.size())
			return true;
		else
			return false;
	}

}

