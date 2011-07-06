
#include "precompiled.h"

#include "goretypedefs.h"

namespace game
{
  char *goreTypeName[GORETYPE_AMOUNT + 1] = 
	{
		"reserved",
		"slice",
		"partial_slice",
		"explode",
		"partial_explode",
		"melt",
		"burn",
		"grind",
		"electrified",
		"fall",
		"partial_fall",
		"squish",
		"punch",

		"***"
	};

  bool goreTypeRemovesOrigin[GORETYPE_AMOUNT + 1] =
	{
		false,
		true,
		false,
		true,
		false,
		false,
		false,
		true,
		false,
		true,
		false,
		true,
		true,

		(bool)0
	};
}

