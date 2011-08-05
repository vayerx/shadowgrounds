
#include "precompiled.h"

#include "direct_controls.h"


static const char *directControlNames[DIRECT_CTRL_AMOUNT + 1] = 
{
	"_invalid",

	"forward",
	"backward",
	"left",
	"right",
	"fire",
	"turn_left",
	"turn_right",
	"fire_secondary",
	"special_move",
	"fire_grenade",

	"***"
};

int getDirectControlIdForName(const char *name)
{
	for (int i = 0; i < DIRECT_CTRL_AMOUNT; i++)
	{
		assert(directControlNames[i] != NULL && strcmp(directControlNames[i], "***") != 0);

		if (strcmp(directControlNames[i], name) == 0)
		{
			return i;
		}
	}

	return 0;
}

