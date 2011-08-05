
#include "precompiled.h"

#include "UnitList.h"

#include "Unit.h"
#include "UnitType.h"
#include "gamedefs.h"
#include "scaledefs.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"

#include <c2_qtree.h>
#include <boost/scoped_ptr.hpp>
#include <vector>

#include "tracking/SimpleTrackableUnifiedHandleObjectIterator.h"
#include "tracking/trackable_types.h"

#include "../util/fb_assert.h"

#include "../util/Debug_MemoryManager.h"

// should be replaced with the unit's actual radius
#define UNIT_QTREE_RADIUS_HACK 2.0f

// this treshold helps us to make sure we don't update unit positions in the qtree
// if the unit is slightly "shaking" due to being squeezed.
#define UNIT_QTREE_UPDATE_TRESHOLD 0.5f

namespace game
{
	typedef Quadtree<Unit> UnitQTree;

	static int unitlist_nextIdNumber = UNITID_LOWEST_POSSIBLE_VALUE;

	class UnitListEntity
	{
		private:
			UnitListEntity()
			{
				entity = NULL;
				lastUpdatePosition = VC3(0,0,0);
			}

			Quadtree<Unit>::Entity *entity;
			VC3 lastUpdatePosition;

			friend class UnitList;
	};


	class UnitListImpl
	{
		private:
			UnitListImpl()
			{
				// nop?
			}

			boost::scoped_ptr<UnitQTree> tree;

		friend class UnitList;
	};

	class NearbyOwnedUnitIterator : public IUnitListIterator
	{
		public:
			virtual ~NearbyOwnedUnitIterator() 
			{
				// nop?
			}

			NearbyOwnedUnitIterator(int player)
			{
				this->player = player;
				atUnit = 0;
			}		

			virtual Unit *iterateNext()
			{
				skipNonOwned();
				if (atUnit >= (int)foundUnits.size())
				{
					fb_assert(!"NearbyOwnedUnitIterator::iterateNext - No more units to iterate.");
					return false;
				}
				
				atUnit++;
				return foundUnits[atUnit-1];
			}

			virtual bool iterateAvailable()
			{
				skipNonOwned();
				if (atUnit < (int)foundUnits.size())
				{
					return true;
				} else {
					return false;
				}
			}

		private:

			void skipNonOwned()
			{
				while (true)
				{
					if (atUnit >= (int)foundUnits.size())
					{
						break;
					}
					if (foundUnits[atUnit]->getOwner() == player)
					{
						break;
					}
					atUnit++;
				}
			}

			std::vector<Unit *> foundUnits;
			int player;
			int atUnit;

			friend class UnitList;
	};


	class NearbyAllUnitIterator : public IUnitListIterator
	{
		public:
			virtual ~NearbyAllUnitIterator() 
			{
				// nop?
			}

			NearbyAllUnitIterator()
			{
				atUnit = 0;
			}		

			virtual Unit *iterateNext()
			{
				if (atUnit >= (int)foundUnits.size())
				{
					fb_assert(!"NearbyAllUnitIterator::iterateNext - No more units to iterate.");
					return false;
				}
				
				atUnit++;
				return foundUnits[atUnit-1];
			}

			virtual bool iterateAvailable()
			{
				if (atUnit < (int)foundUnits.size())
				{
					return true;
				} else {
					return false;
				}
			}

		private:

			std::vector<Unit *> foundUnits;
			int atUnit;

			friend class UnitList;
	};




	UnitList::UnitList()
	{
		allUnits = new LinkedList();
		ownedUnits = new LinkedList *[ABS_MAX_PLAYERS];
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			ownedUnits[i] = new LinkedList();
			ownedUnitAmount[i] = 0;
		}
		allUnitAmount = 0;

		this->impl = new UnitListImpl();
	}

	// NOTE, does not delete the units inside this but just the list of them
	UnitList::~UnitList()
	{
		fb_assert(this->impl != NULL);
		delete this->impl;

		while (!allUnits->isEmpty())
		{
			allUnits->popLast();
			allUnitAmount--;
		}
		delete allUnits;
		assert(allUnitAmount == 0);
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			while (!ownedUnits[i]->isEmpty())
			{
				ownedUnits[i]->popLast();
				ownedUnitAmount[i]--;
			}
			delete ownedUnits[i];
		}
		delete [] ownedUnits;
	}

	SaveData *UnitList::getSaveData() const
	{
		// TODO
		return NULL;
	}

	const char *UnitList::getStatusInfo() const
	{
		return "UnitList";
	}

	int UnitList::getAllUnitAmount()
	{
#ifdef _DEBUG
		// TODO, not thread safe! (caller may not be using iterator)
		/*
		int count = 0;
		allUnits->resetIterate();
		while (allUnits->iterateAvailable())
		{
			allUnits->iterateNext();
			count++;
		}
		assert(count == allUnitAmount);
		*/
#endif
		return allUnitAmount;
	}

	int UnitList::getOwnedUnitAmount(int player)
	{
#ifdef _DEBUG
		// TODO, not thread safe! (caller may not be using iterator)
		/*
		int count = 0;
		ownedUnits[player]->resetIterate();
		while (ownedUnits[player]->iterateAvailable())
		{
			ownedUnits[player]->iterateNext();
			count++;
		}
		assert(count == ownedUnitAmount[player]);
		*/
#endif
		return ownedUnitAmount[player];
	}

	LinkedList *UnitList::getAllUnits()
	{
		return allUnits;
	}

	LinkedList *UnitList::getOwnedUnits(int player)
	{
		return ownedUnits[player];
	}

	void UnitList::updateUnitRadius(Unit *unit)
	{
		float radius = UNIT_QTREE_RADIUS_HACK;
		if (unit->getVisualObject() != NULL
			&& unit->getVisualObject()->getStormModel() != NULL)
		{
			float radius2 = unit->getVisualObject()->getStormModel()->GetRadius();
			if (radius2 > radius) radius = radius2;
		}

		//Logger::getInstance()->info(int2str(radius));
		unit->getUnitListEntity()->entity->setRadius(radius);
		unit->setUnitListRadius(radius);
	}

	void UnitList::addUnit(Unit *unit)
	{
		fb_assert(unit != NULL);

		unit->setUnitListEntity(new UnitListEntity());
		if (impl->tree)
		{
			float radius = UNIT_QTREE_RADIUS_HACK;
			// grow the radius for non-boned units only
			// (as boned units may have 15m radius or such because of changing animations)
			if (unit->getUnitType()->getBonesFilename() == NULL)
			{
				if (unit->getVisualObject() != NULL
					&& unit->getVisualObject()->getStormModel() != NULL)
				{
					float radius2 = unit->getVisualObject()->getStormModel()->GetRadius();
					if (radius2 > radius) radius = radius2;
				}
			}
			//Logger::getInstance()->info(int2str(radius));
			unit->getUnitListEntity()->entity = impl->tree->insert(unit, unit->getPosition(), radius);
			unit->setUnitListRadius(radius);
		}

		allUnits->append(unit);
		int own = unit->getOwner();
		if (own != NO_UNIT_OWNER)
		{
			if (own < 0 || own >= ABS_MAX_PLAYERS) abort();
			ownedUnits[own]->append(unit);
			ownedUnitAmount[own]++;
		}
		allUnitAmount++;

		unit->setIdNumber(unitlist_nextIdNumber);

		int failsafecount = 0;
		while (true)
		{
			unitlist_nextIdNumber++;
			if (unitlist_nextIdNumber > UNITID_HIGHEST_POSSIBLE_VALUE)
				unitlist_nextIdNumber = UNITID_LOWEST_POSSIBLE_VALUE;

			bool alreadyTaken = false;
			LinkedListIterator iter(allUnits);
			while (iter.iterateAvailable())
			{
				Unit *u = (Unit *)iter.iterateNext();
				if (u->getIdNumber() == unitlist_nextIdNumber)
				{
					alreadyTaken = true;
					break;
				}
			}

			if (!alreadyTaken)
				break;

			failsafecount++;
			if (failsafecount > (UNITID_HIGHEST_POSSIBLE_VALUE - UNITID_LOWEST_POSSIBLE_VALUE + 1))
			{
				break;
			}
		}
	}

	// does not delete the unit, just removes it from the list
	void UnitList::removeUnit(Unit *unit)
	{
		fb_assert(unit != NULL);

		if (impl->tree)
			impl->tree->erase(unit->getUnitListEntity()->entity);

		delete unit->getUnitListEntity();
		unit->setUnitListEntity(NULL);

		allUnits->remove(unit);
		int own = unit->getOwner();
		if (own != NO_UNIT_OWNER)
		{
			if (own < 0 || own >= ABS_MAX_PLAYERS) abort();
			ownedUnits[own]->remove(unit);
			ownedUnitAmount[own]--;
		} 	 
		allUnitAmount--;
	}

	void UnitList::switchUnitSide(Unit *unit, int side)
	{
		// remove from list
		{
			int own = unit->getOwner();
			if (own != NO_UNIT_OWNER)
			{
				if (own < 0 || own >= ABS_MAX_PLAYERS) abort();
				ownedUnits[own]->remove(unit);
				ownedUnitAmount[own]--;
			}
		}

		// change owner
		unit->setOwner(side);

		// add to list
		{
			int own = unit->getOwner();
			if (own != NO_UNIT_OWNER)
			{
				if (own < 0 || own >= ABS_MAX_PLAYERS) abort();
				ownedUnits[own]->append(unit);
				ownedUnitAmount[own]++;
			}
		}
	}

	int UnitList::getIdForUnit(Unit *unit)
	{
		assert(unit != NULL);

		assert(unit->getIdNumber() != 0);

		return unit->getIdNumber();

		/*
		int i = UNITID_LOWEST_POSSIBLE_VALUE;

		LinkedListIterator iter(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (unit == u) return i;
			i++;
		}

		assert(!"UnitList::getIdForUnit - Unable to solve an id for given unit.");
		return 0;
		*/
	}

	Unit *UnitList::getUnitById(int id)
	{
		// TODO: optimize, need a hash map or something for this!

		LinkedListIterator iter(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->getIdNumber() == id)
				return u;
		}
		Logger::getInstance()->debug("UnitList::getUnitById - Given id did not match any unit.");
		return NULL;

		/*
		int i = UNITID_LOWEST_POSSIBLE_VALUE;

		// TODO: check getAllUnitAmount first before looping.
		// (could save some time, if there was a proper counter for the amount)
		
		LinkedListIterator iter(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (i == id) return u;
			i++;
		}

		Logger::getInstance()->debug("UnitList::getUnitById - Given id did not match any unit.");
		//assert(!"UnitList::getUnitById - Given id did not match any unit.");
		return NULL;
		*/
	}

	Unit *UnitList::getUnitByIdString(const char *idString)
	{
		LinkedListIterator iter(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			const char *tmp = u->getIdString();
			if (tmp != NULL)
			{
				if (strcmp(tmp, idString) == 0)
					return u;
			}
		}

		//assert(!"UnitList::getUnitByIdString - Given id string did not match any unit.");
		return NULL;
	}

	void UnitList::updateLists()
	{
		LinkedListIterator iter(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive())
			{
				UnitListEntity *ent = u->getUnitListEntity();
				if (ent != NULL)
				{
					VC3 pos = u->getPosition();
					VC3 posdiff = pos - ent->lastUpdatePosition;
					if (fabsf(posdiff.x) > UNIT_QTREE_UPDATE_TRESHOLD 
						|| fabsf(posdiff.z) > UNIT_QTREE_UPDATE_TRESHOLD)
					{
						ent->entity->setPosition(pos);
						ent->lastUpdatePosition = pos;
					}
				}
			}
		}
	}

	IUnitListIterator *UnitList::getNearbyOwnedUnits(int player, const VC3 &position, float radius)
	{
		NearbyOwnedUnitIterator *iter = new NearbyOwnedUnitIterator(player);

		impl->tree->collectSphere(iter->foundUnits, position, radius);
		iter->atUnit = 0;

		return iter;
	}

	IUnitListIterator *UnitList::getNearbyAllUnits(const VC3 &position, float radius)
	{
		NearbyAllUnitIterator *iter = new NearbyAllUnitIterator();

		impl->tree->collectSphere(iter->foundUnits, position, radius);

// TEMP: ...
/*
for (int i = 0; i < iter->foundUnits.size(); i++)
{
	Unit *u = iter->foundUnits[i];
	VC3 upos = u->getPosition();
	VC3 diff = upos - position;
	Logger::getInstance()->info(int2str(diff.GetLength()));
	if (diff.GetLength() > radius + 10)
	{
		assert(!"BUG!!!");
		Logger::getInstance()->error(int2str(radius));
		Logger::getInstance()->error("BUG!!!");
	}
}
*/

		iter->atUnit = 0;

		return iter;
	}

	void UnitList::recreateLists(const VC2 &size)
	{
		VC2	mmin(-size.x, -size.y);
		VC2 mmax( size.x,  size.y);

		LinkedListIterator iter(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->getUnitListEntity() != NULL)
			{
				delete u->getUnitListEntity();
				u->setUnitListEntity(NULL);
			}
		}

		impl->tree.reset(new UnitQTree(mmin, mmax));

		iter = LinkedListIterator(allUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			//assert(u->getUnitListEntity() != NULL);
			//delete u->getUnitListEntity();

			float radius = UNIT_QTREE_RADIUS_HACK;
			// grow the radius for non-boned units only
			// (as boned units may have 15m radius or such because of changing animations)
			if (u->getUnitType()->getBonesFilename() == NULL)
			{
				if (u->getVisualObject() != NULL
					&& u->getVisualObject()->getStormModel() != NULL)
				{
					float radius2 = u->getVisualObject()->getStormModel()->GetRadius();
					if (radius2 > radius) radius = radius2;
				}
			}
			u->setUnitListEntity(new UnitListEntity());
			u->getUnitListEntity()->entity = impl->tree->insert(u, u->getPosition(), radius);
			u->getUnitListEntity()->lastUpdatePosition = u->getPosition();
		}

	}

	UnifiedHandle UnitList::getUnifiedHandle(int unitId) const
	{
		// why bother removing the bit, no reason for that... duh.
		//assert(!IS_UNIFIED_HANDLE_UNIT(unitId));
		//assert(IS_UNIFIED_HANDLE_UNIT(unitId | UNIFIED_HANDLE_BIT_UNIT));
		//return (unitId | UNIFIED_HANDLE_BIT_UNIT);

		// the correct implementation...
		assert(VALIDATE_UNIFIED_HANDLE_BITS(unitId));
		assert(IS_UNIFIED_HANDLE_UNIT(unitId));
		return unitId;
	}

	int UnitList::unifiedHandleToUnitId(UnifiedHandle unifiedHandle) const
	{
		// why bother removing the bit, no reason for that... duh.
		//assert(IS_UNIFIED_HANDLE_UNIT(unifiedHandle));
		//return (unifiedHandle ^ UNIFIED_HANDLE_BIT_UNIT);

		// the correct implementation...
		assert(VALIDATE_UNIFIED_HANDLE_BITS(unifiedHandle));
		assert(IS_UNIFIED_HANDLE_UNIT(unifiedHandle));
		return unifiedHandle;
	}

	bool UnitList::doesTrackableUnifiedHandleObjectExist(UnifiedHandle unifiedHandle) const
	{
		UnitList *thism = const_cast<UnitList *>(this);

		if (thism->getUnitById(unifiedHandleToUnitId(unifiedHandle)) != NULL)
			return true;
		else
			return false;
	}

	VC3 UnitList::getTrackableUnifiedHandlePosition(UnifiedHandle unifiedHandle) const
	{
		UnitList *thism = const_cast<UnitList *>(this);

		Unit *u = thism->getUnitById(unifiedHandleToUnitId(unifiedHandle));
		assert(u != NULL);

		return u->getTrackablePosition();
	}

	QUAT UnitList::getTrackableUnifiedHandleRotation(UnifiedHandle unifiedHandle) const
	{
		UnitList *thism = const_cast<UnitList *>(this);

		Unit *u = thism->getUnitById(unifiedHandleToUnitId(unifiedHandle));
		assert(u != NULL);

		VC3 rot = u->getRotation();

		QUAT ret = QUAT(UNIT_ANGLE_TO_RAD(rot.x), UNIT_ANGLE_TO_RAD(rot.y), UNIT_ANGLE_TO_RAD(rot.z));

		return ret;
	}

	VC3 UnitList::getTrackableUnifiedHandleVelocity(UnifiedHandle unifiedHandle) const
	{
		UnitList *thism = const_cast<UnitList *>(this);

		Unit *u = thism->getUnitById(unifiedHandleToUnitId(unifiedHandle));
		assert(u != NULL);

		return u->getVelocity();
	}

	game::tracking::ITrackableUnifiedHandleObjectIterator *UnitList::getTrackableUnifiedHandleObjectsFromArea(const VC3 &position, float radius, TRACKABLE_TYPEID_DATATYPE typeMask)
	{
		game::tracking::SimpleTrackableUnifiedHandleObjectIterator *iter = new game::tracking::SimpleTrackableUnifiedHandleObjectIterator();

		if(typeMask != TRACKABLE_TYPE_BURNABLE)
		{
			Logger::getInstance()->error("UnitList::getTrackableUnifiedHandleObjectsFromArea - Only burnable type supported.");
			assert(!"UnitList::getTrackableUnifiedHandleObjectsFromArea - Only burnable type supported.");
			// or should we return null?
			return iter;
		}

		IUnitListIterator *uli = getNearbyAllUnits(position, radius);
		while (uli->iterateAvailable())
		{
			Unit *u = uli->iterateNext();
			if ((u->getUnitType()->getTrackableTypeMask() & typeMask) != 0)
			{
				iter->addEntry(getUnifiedHandle(getIdForUnit(u)));
			}
		}

		delete uli;

		return iter;
	}

}

