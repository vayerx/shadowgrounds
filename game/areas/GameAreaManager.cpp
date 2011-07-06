
#include "precompiled.h"

#include "GameAreaManager.h"
#include "../unified_handle.h"

// Internal area index bit masks...
// (32 bit int required)

// (max 64 types)
// (in reality, only 30 used, as 32 bit int bits are used to indicate type flags)
#define AREA_TYPE_INDEX_MASK ((1<<6)-1)
//#define TYPE_INDEX_SHIFT 0
#define AREA_TYPE_INDEX_MAX_VALUE (64-1)

// (loads of instances within each type, should be 32k?)
// just to be sure, capping to 16k
#define AREA_INSTANCE_INDEX_MASK (((1<<21)-1) ^ AREA_TYPE_INDEX_MASK)
#define AREA_INSTANCE_INDEX_SHIFT 6
#define AREA_INSTANCE_INDEX_MAX_VALUE (16384-1)

// (about 512 sub-indices for each area - these are for example, polygon center and corners)
// just to sure, capping to 256
#define AREA_INSTANCE_SUB_INDEX_MASK (((1<<30)-1) ^ AREA_TYPE_INDEX_MASK ^ AREA_INSTANCE_INDEX_MASK)
#define AREA_INSTANCE_SUB_INDEX_SHIFT 21
#define AREA_INSTANCE_SUB_INDEX_MAX_VALUE (256-1)

// (note, last bit reserved for signed bit)


namespace game
{

	class GameAreaManagerImpl
	{
	public:
		// instances for each area type... 
		// and assigned unified handle as well.
		// note, this is vector of instance indices, not sub indices
		// sub indices are naturally some internal sub indices of the IArea for that instance index
		// (can be used to map internal index -> unified handle with O(1) efficiency)
		// the unified handle vector contains the unified handles for all sub indices (first being the
		// area itself, the rest being corner points or something)
		std::vector<std::pair<std::vector<UnifiedHandle>, IArea *> > areas[AREA_TYPE_MAX_AMOUNT];

		// mapping from (unified handle - base) -> int containing type index and vector index 
		// (can be used to map unified handle -> internal index with O(1) efficiency)
		std::vector<int> areaIndexByUnifiedHandleOffset;
	};


}

