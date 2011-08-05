
#include "precompiled.h"

#include "trackable_types.h"

#include <assert.h>

struct TrackableTypes
{
	const char *typeName;
	TRACKABLE_TYPEID_DATATYPE typeBit;
};

TrackableTypes trackableTypes[TRACKABLE_TYPEID_MAX_TYPES] = {

{ "burnable", TRACKABLE_TYPE_BURNABLE },
{ "leakable", TRACKABLE_TYPE_LEAKABLE }, 
{ "target_for_hostile_ai", TRACKABLE_TYPE_TARGET_FOR_HOSTILE_AI },
{ "target_for_friendly_ai", TRACKABLE_TYPE_TARGET_FOR_FRIENDLY_AI },
{ "net_self", TRACKABLE_TYPE_NET_SELF },
{ "net_other", TRACKABLE_TYPE_NET_OTHER },
{ "reserved_6", (1<<6) },
{ "reserved_7", (1<<7) },
{ "reserved_8", (1<<8) },
{ "reserved_9", (1<<9) },
{ "reserved_10", (1<<10) },
{ "reserved_11", (1<<11) },
{ "reserved_12", (1<<12) },
{ "reserved_13", (1<<13) },
{ "reserved_14", (1<<14) },
{ "reserved_15", (1<<15) },
{ "reserved_16", (1<<16) },
{ "reserved_17", (1<<17) },
{ "reserved_18", (1<<18) },
{ "reserved_19", (1<<19) },
{ "reserved_20", (1<<20) },
{ "reserved_21", (1<<21) },
{ "reserved_22", (1<<22) },
{ "reserved_23", (1<<23) },
{ "reserved_24", (1<<24) },
{ "reserved_25", (1<<25) },
{ "reserved_26", (1<<26) },
{ "reserved_27", (1<<27) },
{ "reserved_28", (1<<28) },
{ "reserved_29", (1<<29) },

};

TRACKABLE_TYPEID_DATATYPE getTrackableTypeIdForName(const char *trackableTypeName)
{
	assert(trackableTypeName != NULL);

	for (int i = 0; i < TRACKABLE_TYPEID_MAX_TYPES; i++)
	{
		if (strcmp(trackableTypes[i].typeName, trackableTypeName) == 0)
		{
			return trackableTypes[i].typeBit;
		}
	}

	return 0;
}

