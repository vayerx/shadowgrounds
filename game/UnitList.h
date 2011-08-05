
#ifndef UNITLIST_H
#define UNITLIST_H

#include "GameObject.h"
#include "IUnitListIterator.h"
#include "gamedefs.h"
#include "unified_handle.h"
#include "tracking/ITrackableUnifiedHandleObjectImplementationManager.h"

#include <DatatypeDef.h>

#define UNITID_LOWEST_POSSIBLE_VALUE UNIFIED_HANDLE_FIRST_UNIT
#define UNITID_HIGHEST_POSSIBLE_VALUE UNIFIED_HANDLE_LAST_UNIT
//#define UNITID_LOWEST_POSSIBLE_VALUE 100000
//#define UNITID_HIGHEST_POSSIBLE_VALUE 9999999

class LinkedList;

namespace game
{
	class Unit;
	class UnitListImpl;

	class UnitList : public GameObject, public game::tracking::ITrackableUnifiedHandleObjectImplementationManager
	{
	public:
		UnitList();
		~UnitList();

		virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;

		int getAllUnitAmount();
		int getOwnedUnitAmount(int player);

		LinkedList *getAllUnits();
		LinkedList *getOwnedUnits(int player);

		void addUnit(Unit *unit);
		void removeUnit(Unit *unit);
		void switchUnitSide(Unit *unit, int side);

		void updateUnitRadius(Unit *unit);

		int getIdForUnit(Unit *unit);
		Unit *getUnitById(int id);

		UnifiedHandle getUnifiedHandle(int unitId) const;
		int unifiedHandleToUnitId(UnifiedHandle unifiedHandle) const;

		Unit *getUnitByIdString(const char *idString);

		IUnitListIterator *getNearbyOwnedUnits(int player, const VC3 &position, float radius);
		IUnitListIterator *getNearbyAllUnits(const VC3 &position, float radius);

		// call this after (or before) every tick
		void updateLists();

		// call this after the map has been recreated
		void recreateLists(const VC2 &size);

		// trackable unified handle interface (ITrackableUnifiedHandleObjectImplementationManager)
		virtual bool doesTrackableUnifiedHandleObjectExist(UnifiedHandle unifiedHandle) const;
		virtual VC3 getTrackableUnifiedHandlePosition(UnifiedHandle unifiedHandle) const;
		virtual QUAT getTrackableUnifiedHandleRotation(UnifiedHandle unifiedHandle) const;
		virtual VC3 getTrackableUnifiedHandleVelocity(UnifiedHandle unifiedHandle) const;
		virtual game::tracking::ITrackableUnifiedHandleObjectIterator *getTrackableUnifiedHandleObjectsFromArea(const VC3 &position, float radius, TRACKABLE_TYPEID_DATATYPE typeMask);
		// ---

	private:
		LinkedList *allUnits;
		LinkedList **ownedUnits;
		int allUnitAmount;
		int ownedUnitAmount[ABS_MAX_PLAYERS];

		UnitListImpl *impl;
	};

}

#endif

