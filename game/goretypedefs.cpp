
#include "precompiled.h"

#include "goretypedefs.h"

namespace game
{
  const char *goreTypeName[GORETYPE_AMOUNT + 1] =
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

		(bool)0
	};
}

