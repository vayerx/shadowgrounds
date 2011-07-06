
#include "precompiled.h"

#include "GameJointManager.h"
#include "../unified_handle.h"

// Internal joint index bit masks...
// (32 bit int required)

// (max 64 types)
// (in reality, only 30 used, as 32 bit int bits are used to indicate type flags)
#define JOINT_TYPE_INDEX_MASK ((1<<6)-1)
//#define JOINT_TYPE_INDEX_SHIFT 0
#define JOINT_TYPE_INDEX_MAX_VALUE (64-1)

// (loads of instances within each type, should be 32k?)
// just to be sure, capping to 16k
#define JOINT_INSTANCE_INDEX_MASK (((1<<21)-1) ^ JOINT_TYPE_INDEX_MASK)
#define JOINT_INSTANCE_INDEX_SHIFT 6
#define JOINT_INSTANCE_INDEX_MAX_VALUE (16384-1)

// (about 512 sub-indices for each area - these are for example, rope joint ends)
// just to sure, capping to 256
#define JOINT_INSTANCE_SUB_INDEX_MASK (((1<<30)-1) ^ JOINT_TYPE_INDEX_MASK ^ JOINT_INSTANCE_INDEX_MASK)
#define JOINT_INSTANCE_SUB_INDEX_SHIFT 21
#define JOINT_INSTANCE_SUB_INDEX_MAX_VALUE (256-1)

// (note, last bit reserved for signed bit)


namespace game
{

	class GameJointManagerImpl
	{
	public:
		// instances for each area type...
		// and assigned unified handle as well.
		// (can be used to map internal index -> unified handle with O(1) efficiency)
		std::vector<std::pair<std::vector<UnifiedHandle>, IJoint *> > joints[JOINT_TYPE_MAX_AMOUNT];

		// mapping from (unified handle - base) -> int containing type index and vector index 
		// note, that several
		// (can be used to map unified handle -> internal index with O(1) efficiency)
		std::vector<int> areaIndexByUnifiedHandleOffset;
	};


}

