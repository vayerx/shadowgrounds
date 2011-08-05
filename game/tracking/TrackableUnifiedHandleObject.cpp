
#include "precompiled.h"

#include "TrackableUnifiedHandleObject.h"
#include "ITrackerObjectType.h"
#include "ITrackerObject.h"
#include "../unified_handle.h"
#include "trackable_types.h"

#include "../Game.h"
#include "../UnifiedHandleManager.h"
#include "../GameUI.h"
#include "../UnitList.h"
#include "../../ui/Terrain.h"
#include "../../system/Logger.h"

// prime numbers are preferrable...
//#define TUHO_POOL_SIZE 7523
#define TUHO_POOL_SIZE 3173

#define UNIFIED_HANDLE_TO_INITIAL_POOL_INDEX(uh) (uh % TUHO_POOL_SIZE)

namespace game
{
namespace tracking
{
	game::Game *tuho_game = NULL;
	TrackableUnifiedHandleObject * tuho_pool = NULL;
	int tuho_pool_used = 0;

	static int TUHOtypeId_data;

	void *TrackableUnifiedHandleObject::typeId = &TUHOtypeId_data;


	TrackableUnifiedHandleObject::TrackableUnifiedHandleObject()
	{
		this->handle = UNIFIED_HANDLE_NONE;
		this->refCount = 0;
		this->trackedByCount = 0;
		for (int i = 0; i < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; i++)
		{
			this->trackedBy[i] = UNIFIED_HANDLE_NONE;
		}
	}


	TrackableUnifiedHandleObject::TrackableUnifiedHandleObject(UnifiedHandle unifiedHandle) 
	{
		this->handle = unifiedHandle; 
		this->refCount = 0;
	}

	TrackableUnifiedHandleObject::~TrackableUnifiedHandleObject()
	{
		assert(this->refCount == 0);
	}


	VC3 TrackableUnifiedHandleObject::getTrackableObjectPosition() const
	{
		// BUGBUG?? assert ei toimi
		// assert(TrackableUnifiedHandleObject::game != NULL);

		if (IS_UNIFIED_HANDLE_UNIT(handle))
		{
			return tuho_game->units->getTrackableUnifiedHandlePosition(handle);
		}
		else if (IS_UNIFIED_HANDLE_TERRAIN_OBJECT(handle))
		{
			return tuho_game->gameUI->getTerrain()->getTrackableUnifiedHandlePosition(handle);
		} else {
			assert(!"TrackableUnifiedHandleObject::getTrackableObjectPosition - Unified handle maps to unsupported object type.");
			return VC3(0,0,0);
		}

	}
 

	QUAT TrackableUnifiedHandleObject::getTrackableObjectRotation() const
	{
		// BUGBUG?? assert ei toimi
		// assert(TrackableUnifiedHandleObject::game != NULL);

		if (IS_UNIFIED_HANDLE_UNIT(handle))
		{
			return tuho_game->units->getTrackableUnifiedHandleRotation(handle);
		}
		else if (IS_UNIFIED_HANDLE_TERRAIN_OBJECT(handle))
		{
			return tuho_game->gameUI->getTerrain()->getTrackableUnifiedHandleRotation(handle);
		} else {
			assert(!"TrackableUnifiedHandleObject::getTrackableObjectRotation - Unified handle maps to unsupported object type.");
			return QUAT();
		}

	}
 

	TRACKABLE_TYPEID_DATATYPE TrackableUnifiedHandleObject::getTrackableTypes() const
	{
// TEMP
return TRACKABLE_TYPE_BURNABLE;
/*
		if (IS_UNIFIED_HANDLE_UNIT(handle))
		{
			// TODO: ...
			return TRACKABLE_TYPE_BURNABLE;
		}
		else if (IS_UNIFIED_HANDLE_TERRAIN_OBJECT(handle))
		{
			// TODO: ...
			return TRACKABLE_TYPE_BURNABLE;
		} else {
			assert(!"TrackableUnifiedHandleObject::getTrackableTypes - Unified handle maps to unsupported object type.");
		}
*/
	}


	void TrackableUnifiedHandleObject::setGame(game::Game *game) 
	{ 
		tuho_game = game; 

		if (tuho_pool == NULL)
		{
			tuho_pool = new TrackableUnifiedHandleObject[TUHO_POOL_SIZE];
			tuho_pool_used = 0;
		}
	}


	TrackableUnifiedHandleObject *TrackableUnifiedHandleObject::getInstanceFromPool(int unifiedHandle)
	{
		assert(tuho_pool != NULL);
		assert(unifiedHandle != 0);

		// allow only 3/4 of the pool to be used for now... after that, start failing. 
		// (it would get way too slow at this point anyway)
		// TODO: should increase the pool size once exhausted
		// notice however, that the pool array does not contain pointers, but rather the data itself, so it cannot
		// be just copied to new array - the old references to old array would become invalid after array deletion
		// so, the old array would have to be preserved for a while... -> just make the initial pool size to be 
		// big enough for the largest case scenario.
		if (tuho_pool_used >= TUHO_POOL_SIZE * 3 / 4)
		{
			Logger::getInstance()->error("TrackableUnifiedHandleObject::getInstanceFromPool - Maximum allowed pool size exceeded.");
			assert(!"TrackableUnifiedHandleObject::getInstanceFromPool - Maximum allowed pool size exceeded.");
			return NULL;
		}
		if (tuho_pool_used == TUHO_POOL_SIZE / 2)
		{
			Logger::getInstance()->warning("TrackableUnifiedHandleObject::getInstanceFromPool - Approaching maximum pool size.");
		}

		// NOTE: this is basically a primitive hash table implementation, could just use hash_map for nearly the same result..

		int i = UNIFIED_HANDLE_TO_INITIAL_POOL_INDEX(unifiedHandle);

		while(tuho_pool[i].handle != 0)
		{
			if (tuho_pool[i].handle == unifiedHandle)
				return &tuho_pool[i];

			i+=3;
			i = i % TUHO_POOL_SIZE;
		}

		tuho_pool[i].handle = unifiedHandle;

		// TODO: proper impl
		//tuho_pool[i].refCount++;

		tuho_pool_used++;
		return &tuho_pool[i];
	}


	void TrackableUnifiedHandleObject::cleanupPool()
	{
		if (tuho_pool != NULL)
		{
			delete [] tuho_pool;
			tuho_pool = NULL;
			tuho_pool_used = 0;
		}
	}

	void TrackableUnifiedHandleObject::release()
	{
		// DUH! how stupid is this...
		/*
		int i = handle % TUHO_POOL_SIZE;

		while(tuho_pool[i].handle != 0)
		{
			if (tuho_pool[i].handle == handle)
			{
				assert(tuho_pool[i].refCount > 0);
				tuho_pool[i].refCount--;
				// NOTE: cannot actually remove it from the pool - as that would make a gap.
				// (some other handle may be left behind the gap from it's original modulo calculated position)
				// (and the current implementation does not really handle such situations)
				return;
			}

			i+=3;
			i = i % TUHO_POOL_SIZE;
		}
		// was not in the pool???
		assert(!"TrackableUnifiedHandleObject::release - object did not exist in the pool.");
		*/

		// TODO: proper impl
		//assert(this->refCount > 0);
		//this->refCount--;
	}

	void TrackableUnifiedHandleObject::addTrackedBy(UnifiedHandle tracker)
	{
		this->trackedByCount++;

		for (int i = 0; i < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; i++)
		{
			if (this->trackedBy[i] == UNIFIED_HANDLE_NONE)
			{
				this->trackedBy[i] = tracker;
				return;
			}
		}
		// did not fit in cache.. do nothing? (or overwrite LRU cache entry)
		assert(!"TrackableUnifiedHandleObject::addTrackedBy - did not fit in cache... currently non-cached not supported?");
	}

	void TrackableUnifiedHandleObject::removeTrackedBy(UnifiedHandle tracker)
	{
		assert(this->trackedByCount > 0);
		this->trackedByCount--;

		for (int i = 0; i < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; i++)
		{
			if (this->trackedBy[i] == tracker)
			{
				this->trackedBy[i] = UNIFIED_HANDLE_NONE;
				return;
			}
		}
		// was not in cache? well, do nothing then.
		assert(!"TrackableUnifiedHandleObject::removeTrackedBy - was not in cache... currently non-cached not supported?");
	}

	UnifiedHandle TrackableUnifiedHandleObject::getTrackedByForType(TrackerTypeNumber trackerTypeNumber)
	{
		if (this->trackedByCount == 0)
			return UNIFIED_HANDLE_NONE;

		// this should neven happen? more than this amount of trackers for one object...
		assert(this->trackedByCount < 10);

		int inCacheAmount = 0;
		// is it in cache?
		for (int i = 0; i < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; i++)
		{
			if (this->trackedBy[i] != UNIFIED_HANDLE_NONE)
			{
				inCacheAmount++;
				ITrackerObject *t = tuho_game->objectTracker->getTrackerByUnifiedHandle(this->trackedBy[i]);
				if (t != NULL)
				{
					if (tuho_game->objectTracker->getTrackerTypeNumberForTracker(this->trackedBy[i]) == trackerTypeNumber)
					{
						return this->trackedBy[i];
					}
				} else {
					// the tracker seems to have been deleted... so remove it from our "cache"? (if this happens it's a bug!)
					assert(!"TrackableUnifiedHandleObject::getTrackedByForType - data integrity lost, tracker has been deleted but is still in cache! (bug)");
					return UNIFIED_HANDLE_NONE;
				}
			}
		}

		assert(this->trackedByCount >= inCacheAmount);

		if (this->trackedByCount == inCacheAmount)
		{
			return UNIFIED_HANDLE_NONE;
		}

		if (this->trackedByCount > TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE)
		{
			// it was not in cache, gotta look it up the slow way...
			// change the ObjectTracker::getAllTrackers to return an iterator instead of vector and make it public...
			LOG_ERROR("TrackableUnifiedHandleObject::getTrackedByForType - TODO, need to look for the tracker the non-cached way...");
			assert(!"TrackableUnifiedHandleObject::getTrackedByForType - TODO, need to look for the tracker the non-cached way...");
		}

		return UNIFIED_HANDLE_NONE;
	}

	UnifiedHandle TrackableUnifiedHandleObject::getTrackedByForTrackableTypes(TRACKABLE_TYPEID_DATATYPE trackableTypes)
	{
		if (this->trackedByCount == 0)
			return UNIFIED_HANDLE_NONE;

		// this should neven happen? more than this amount of trackers for one object...
		assert(this->trackedByCount < 10);

		int inCacheAmount = 0;
		// is it in cache?
		for (int i = 0; i < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; i++)
		{
			if (this->trackedBy[i] != UNIFIED_HANDLE_NONE)
			{
				inCacheAmount++;
				ITrackerObject *t = tuho_game->objectTracker->getTrackerByUnifiedHandle(this->trackedBy[i]);
				if (t != NULL)
				{
					if ((t->getType()->getTrackablesTypeOfInterest() & trackableTypes) != 0)
					{
						return this->trackedBy[i];
					}
				} else {
					// the tracker seems to have been deleted... so remove it from our "cache"? (if this happens it's a bug!)
					assert(!"TrackableUnifiedHandleObject::getTrackedByForTrackableType - data integrity lost, tracker has been deleted but is still in cache! (bug)");
					return UNIFIED_HANDLE_NONE;
				}
			}
		}

		assert(this->trackedByCount >= inCacheAmount);

		if (this->trackedByCount == inCacheAmount)
		{
			return UNIFIED_HANDLE_NONE;
		}

		if (this->trackedByCount > TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE)
		{
			// it was not in cache, gotta look it up the slow way...
			// change the ObjectTracker::getAllTrackers to return an iterator instead of vector and make it public...
			LOG_ERROR("TrackableUnifiedHandleObject::getTrackedByForTrackableType - TODO, need to look for the tracker the non-cached way...");
			assert(!"TrackableUnifiedHandleObject::getTrackedByForTrackableType - TODO, need to look for the tracker the non-cached way...");
		}

		return UNIFIED_HANDLE_NONE;
	}

	UnifiedHandle TrackableUnifiedHandleObject::getTrackedByWithIndex(int index)
	{
		if (this->trackedByCount == 0)
			return UNIFIED_HANDLE_NONE;

		// this should neven happen? more than this amount of trackers for one object...
		assert(this->trackedByCount < 10);

		int atIndex = 0;
		int inCacheAmount = 0;
		// is it in cache?
		for (int i = 0; i < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; i++)
		{
			if (this->trackedBy[i] != UNIFIED_HANDLE_NONE)
			{
				inCacheAmount++;
				ITrackerObject *t = tuho_game->objectTracker->getTrackerByUnifiedHandle(this->trackedBy[i]);
				if (t != NULL)
				{
					if (atIndex == index)
						return this->trackedBy[i];
					atIndex++;
				} else {
					// the tracker seems to have been deleted... so remove it from our "cache"? (if this happens it's a bug!)
					assert(!"TrackableUnifiedHandleObject::getTrackedByWithIndex - data integrity lost, tracker has been deleted but is still in cache! (bug)");
					return UNIFIED_HANDLE_NONE;
				}
			}
		}

		assert(this->trackedByCount >= inCacheAmount);

		if (this->trackedByCount == inCacheAmount)
		{
			return UNIFIED_HANDLE_NONE;
		}

		if (this->trackedByCount > TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE)
		{
			// it was not in cache, gotta look it up the slow way...
			// change the ObjectTracker::getAllTrackers to return an iterator instead of vector and make it public...
			LOG_ERROR("TrackableUnifiedHandleObject::getTrackedByWithIndex - TODO, need to look for the tracker the non-cached way...");
			assert(!"TrackableUnifiedHandleObject::getTrackedByWithIndex - TODO, need to look for the tracker the non-cached way...");
		}

		return UNIFIED_HANDLE_NONE;
	}


	std::string TrackableUnifiedHandleObject::getStatusInfo()
	{
		std::string ret = "TrackableUnifiedHandleObject:\r\n";

		int trackablesInPool = 0;
		int unitTrackables = 0;
		int toTrackables = 0;
		std::string poolVis = "";

		for (int i = 0; i < TUHO_POOL_SIZE; i++)
		{
			if (tuho_pool[i].handle != 0)
			{
				if (IS_UNIFIED_HANDLE_UNIT(tuho_pool[i].handle))
				{
					unitTrackables++;
				}
				if (IS_UNIFIED_HANDLE_TERRAIN_OBJECT(tuho_pool[i].handle))
				{
					toTrackables++;
				}
				trackablesInPool++;
				int attachCacheAmount = 0;
				for (int j = 0; j < TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE; j++)
				{
					if (tuho_pool[i].trackedBy[j] != UNIFIED_HANDLE_NONE)
					{
						attachCacheAmount++;
					}
				}
				char foo[2];
				if (tuho_pool[i].trackedByCount > attachCacheAmount)
					foo[0] = 'N';
				else
					foo[0] = '0' + attachCacheAmount;
				foo[1] = '\0';
				poolVis += foo;
			} else {
				poolVis += "-";
			}
		}

		assert(tuho_pool_used == trackablesInPool);

		ret += std::string("Unified handle trackables in pool: ") + int2str(trackablesInPool);
		ret += std::string("\r\n");

		ret += std::string("Unified handle trackables pointing to units: ") + int2str(unitTrackables);
		ret += std::string("\r\n");

		ret += std::string("Unified handle trackables pointing to terrain objects: ") + int2str(toTrackables);
		ret += std::string("\r\n");

		ret += std::string("Unified handle trackables pool utilization visualization: ") + poolVis;
		ret += std::string("\r\n");

		return ret;
	}


	bool TrackableUnifiedHandleObject::doesExist() const
	{
		return tuho_game->unifiedHandleManager->doesObjectExist(this->handle);
	}

}
}
